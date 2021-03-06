#pragma once

#include "StdAfx.h"

#define __NOT -4, TEXT("<NOT")
#define TON__ -4, TEXT("NOT>")
#define __AND -4, TEXT("<AND")
#define DNA__ -4, TEXT("AND>")
#define __OR -4, TEXT("<OR")
#define RO__ -4, TEXT("OR>")
#define _QE_ -4, TEXT("=")
#define _NOTEQ_ -4, TEXT("!=")

namespace GwaArx
{
	namespace Util
	{
		namespace _aced_get
		{
			/****************************************************************
			type argument [SentryFunc]:
				1. must be a callable type with a signature as bellow: 
					[any_type] SentryFunc( const AcDbObjectId & )
				3. where [any_type] can participate in bool context, e.g.
					if(any_type), or if(!any_type) ...
				to denote the SentryFunc invocation is successful or not.
				4. if any_type is tested to be true, then [any_type] is returned 
				by COPY.				
			****************************************************************/
			template<typename SentryFunc>
			typename boost::result_of<SentryFunc(const AcDbObjectId &)>::type
				acedPick( SentryFunc sentry , ACHAR * szPrompt, ads_point ptPick = NULL )
			{
				using namespace GwaArx::Util;

				// pull auto variables declarations out of loop body for performance consideration
				ads_name sname;
				ads_point pt;	
				boost::shared_ptr<AcDbText> pObj;

				// keep prompting user to pick a valid object until:
				//		1. user press [Ctrl + C] or [Esc] to cancel.
				//      2. user picked a wanted object.
				do 
				{
					// if the user misses the target (i.e. kicks on nothing), acedEntSel 
					// returns RTERROR, which therefore should neither be treated as an exception
					// nor be thrown out.
					// so DO NOT use RtCall() here!
					switch(acedEntSel(szPrompt, sname, pt))
					{
					case RTERROR: 		// most likely, user missed target, retry.	
						continue; 
					case RTCAN:			// user want to cancel.
						throw UserCanceled();			
					}

					AcDbObjectId id;
					acdbGetObjectId(id, sname);					

					BOOST_AUTO(ret, sentry(id));
					if(!ret)
					{
						acutPrintf(TEXT("\n无效的对象."));
						continue;
					}

					// return pick point if required.
					if (ptPick)
					{
						ads_point_set(pt, ptPick);
					}

					return ret;					 	
				} while (1);
			} // acedPick		
			
			/****************************************************************
			type argument [Wanted]:
			1. is the class of the object you finally want.
			2. should have a typedef Wanted::shared_ptr_type as 
				boost::shared_ptr<Wanted>
			3. should have a static member function named newInstance, which 
			actually behave as a factory method and retuning 
				a empty Wanted::shared_ptr_type() denoting failure.
				a valid Wanted::shared_ptr_type() denoting success.
			type argument [Predicator]
			1. just as its name implies, it acts as a predicate.
			2. return true to denote "passed".
			****************************************************************/
			template<typename Wanted>
			typename Wanted::shared_ptr_type acedPickA( 
				ACHAR * szPrompt,								// prompt string
				ads_point ptPick = NULL,						// pointer where user picked the object
				boost::function<bool (typename Wanted::shared_ptr_type)> Pred	// default parameter
				= boost::function<bool (typename Wanted::shared_ptr_type)>())	//  —— a empty function object
			{
				typename Wanted::shared_ptr_type sp = acedPick(
					&typename Wanted::newInstance, szPrompt);

				if (!sp || Pred.empty())
				{
					return sp;
				}
				
				if (!Pred(sp))
				{
					return typename Wanted::shared_ptr_type();
				}
				else
				{
					return sp;
				}				
			}						

			class CSSGet
			{
			public:
				typedef boost::function<bool( ads_name )> sentry_type;
				//constructors
			public:
				CSSGet();			

			public:				
				CSSGet & setPrompt	( const std::wstring & prompt );
				CSSGet & setStr		( const std::wstring & str );
				CSSGet & setPt1		( void * pt1 );				
				CSSGet & setPt2		( void * pt2 );				
				CSSGet & setFilter	( struct resbuf * filter );
				CSSGet & setSS		( ads_name ss );
				CSSGet & setSentry	( sentry_type sentry);				

			public:
				void operator()( void );

			private:
				std::wstring	m_prompt;
				std::wstring	m_str;
				void			*m_pt1;
				void			*m_pt2;
				struct resbuf	*m_filter;
				sentry_type		m_sentry;
				ads_namep		m_ss;
			};				
		} // namespace _aced_get
	} // namespace Util
} // namespace GwaArx