/*************************************************************************
 *
 *  $RCSfile: job.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2002-10-11 13:41:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_JOBS_JOBDISPATCH_HXX_
#define __FRAMEWORK_JOBS_JOBDISPATCH_HXX_

//_______________________________________
// my own includes

#ifndef __FRAMEWORK_CONFIG_CONFIGACCESS_HXX_
#include <jobs/configaccess.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_______________________________________
// interface includes

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XNOTIFYINGDISPATCH_HPP_
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRESULTLISTENER_HPP_
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XJOBLISTENER_HPP_
#include <com/sun/star/task/XJobListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTEVENT_HPP_
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#endif

//_______________________________________
// other includes

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#include <queue>

//_______________________________________
// namespace

namespace framework{

//_______________________________________
// public const

//_______________________________________
/**
    @short  implements a dispatch object for jobs
    @descr  Such dispatch object will be used by the JobHandler if
            an URL "vnd.sun.star.job:alias=<name>" occures.
            Then an instance of this class will be created and used.
            This new instance will be called within his method
            dispatch() or dispatchWithNotification() for executing the
            real job. We do it, control the life cycle of this internal
            wrapped job and inform any interested listener if it finish.

    @see    JobHandler
 */
class JobDispatch : public  css::lang::XTypeProvider
                  , public  css::frame::XNotifyingDispatch // => XDispatch
                  , public  css::task::XJobListener
                  , public  css::frame::XTerminateListener
                  , private ThreadHelpBase
                  , public  ::cppu::OWeakObject
{
    //___________________________________
    // types

    struct JobData
    {
        JobData( const ::rtl::OUString& sAlias )
            : m_sAlias( sAlias )
        {}

        /**
            the alias name of this job
            Is used as entry of configuration set for job registration, to find all
            neccessary properties of it..
         */
        ::rtl::OUString m_sAlias;

        /**
            the uno implementation name of this job
            It's readed from the configuration. Don't set it from outside!
         */
        ::rtl::OUString m_sService;

        /**
            job specific configuration items ... unknown for us!
            It's readed from the configuration. Don't set it from outside!
         */
        css::uno::Sequence< css::beans::NamedValue > m_lArguments;

        /**
            time stamp of the admin layer for reactivation of a deactivated job
            It's readed from the configuration. Don't set it from outside!
         */
        DateTime m_aAdminTime;

        /**
            time stamp of the user layer to deactivate it after finishing his work
            It's readed from the configuration. Don't set it from outside!
         */
        DateTime m_aUserTime;

        /**
            if the admin time stamp is newer then the user time ... this job is ready for execute
            It's set by comparing the two timetamp values m_aAdminTime & m_aUserTime.
         */
        sal_Bool m_bActive;

        /**
            mark it as an asyncronoues or synchronoues job
            It's readed from the configuration. Don't set it from outside!
         */
        sal_Bool m_bAsync;
    };

    struct ExecuteInfo
    {
        /** This (possible) listener is interested on the result of this executed job. */
        css::uno::Reference< css::frame::XDispatchResultListener > m_xResultListener;
    };

    struct DispatchParameter
    {
        DispatchParameter( const css::util::URL&                                             aURL      ,
                           const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                           const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ,
                                 JobDispatch*                                                pOwner    )
        {
            m_xOwnerHold = css::uno::Reference< css::uno::XInterface >( static_cast< ::cppu::OWeakObject* >(pOwner), css::uno::UNO_QUERY );
            m_aURL       = aURL     ;
            m_lArgs      = lArgs    ;
            m_xListener  = xListener;
        }

        DispatchParameter( const DispatchParameter& aCopy )
        {
            m_xOwnerHold = aCopy.m_xOwnerHold;
            m_aURL       = aCopy.m_aURL      ;
            m_lArgs      = aCopy.m_lArgs     ;
            m_xListener  = aCopy.m_xListener ;
        }

        void operator=( const DispatchParameter& aCopy )
        {
            m_xOwnerHold = aCopy.m_xOwnerHold;
            m_aURL       = aCopy.m_aURL      ;
            m_lArgs      = aCopy.m_lArgs     ;
            m_xListener  = aCopy.m_xListener ;
        }

        /** the dispatch URL */
        css::util::URL m_aURL;

        /** the dispatch arguments */
        css::uno::Sequence< css::beans::PropertyValue > m_lArgs;

        /** an interested listener for dispatch results */
        css::uno::Reference< css::frame::XDispatchResultListener > m_xListener;

        /**
            every instance of this struct indicates an outstanding dispatch request
            We have to work till all this jobs are done. So we hold the outside
            JobDispatch object alive by using references.
            So the JobDispatch can die be ref count ...
         */
        css::uno::Reference< css::uno::XInterface > m_xOwnerHold;
    };

    //___________________________________
    // member

    private:

        /** reference to the uno service manager */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /**
            knows anything about the internal wrapped job
            and is filled from the configuration access.
         */
        JobData m_aJobData;

        /**
            a list of all outstanding dispatch requests
         */
        ::std::queue< DispatchParameter > m_lDispatchQueue;

        /**
            hold the currently running job service alive (important for async jobs, wich leave the scope of the start method)
            Normaly they should hold itself alive ... but if some implementations don't do it - we do it.
         */
        css::uno::Reference< css::uno::XInterface > m_xJob;

        /**
            hold the opened configuration access alive
            Because we need this instance in different methods,
            we transport it as a member.
         */
        ConfigAccess m_aConfig;

    //___________________________________
    // native interface methods

    public:

                  JobDispatch( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR  ,
                               const ::rtl::OUString&                                        sAlias );
         virtual ~JobDispatch();

    //___________________________________
    // uno interface methods

    public:

        // XInterface, XTypeProvider
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        // XNotifyingDispatch
        virtual void SAL_CALL dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                        const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                                        const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw(css::uno::RuntimeException);
        // XDispatch
        virtual void SAL_CALL dispatch            ( const css::util::URL&                                     aURL      ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&    lArgs     ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL addStatusListener   ( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException);
        // XJobListener
        virtual void SAL_CALL jobFinished( const css::uno::Reference< css::task::XAsyncJob >& xJob    ,
                                           const css::uno::Any&                               aResult ) throw(css::uno::RuntimeException);
        // XTerminateListener
        virtual void SAL_CALL queryTermination ( const css::lang::EventObject& aEvent ) throw(css::frame::TerminationVetoException,
                                                                                              css::uno::RuntimeException          );
        virtual void SAL_CALL notifyTermination( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException          );
        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException);

    //___________________________________
    // internal methods

    private:

        void impl_startJob          (                                                );
        void impl_finishJob         ( const css::uno::Any&                   aResult );
        void impl_readConfig        (                                                );
        void impl_saveArguments     (                                                );
        void impl_deactivateJob     (                                                );
        void impl_sendDispatchResult( const css::frame::DispatchResultEvent& aResult );
};

} // namespace framework

#endif // __FRAMEWORK_JOBS_JOBDISPATCH_HXX_
