/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: jobdispatch.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_JOBS_JOBDISPATCH_HXX_
#define __FRAMEWORK_JOBS_JOBDISPATCH_HXX_

//_______________________________________
// my own includes

#include <jobs/configaccess.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <macros/debug.hxx>
#include <stdtypes.h>
#include <general.h>

//_______________________________________
// interface includes
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/task/XJobListener.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

//_______________________________________
// other includes
#include <unotools/configpathes.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

//_______________________________________
// namespace

namespace framework{

//_______________________________________
// public const

//_______________________________________
/**
    @short  implements a dispatch object for jobs
    @descr  Such dispatch object will be used by the generic dispatch mechanism if
            an URL "vnd.sun.star.job:alias=<name>" occures.
            Then an instance of this class will be created and used.
            This new instance will be called within his method
            dispatch() or dispatchWithNotification() for executing the
            real job. We do it, control the life cycle of this internal
            wrapped job and inform any interested listener if it finish.
 */
class JobDispatch : public  css::lang::XTypeProvider
                  , public  css::lang::XServiceInfo
                  , public  css::lang::XInitialization
                  , public  css::frame::XDispatchProvider
                  , public  css::frame::XNotifyingDispatch      // => XDispatch
                  , private ThreadHelpBase
                  , public  ::cppu::OWeakObject
{
    //___________________________________
    // member

    private:

        /** reference to the uno service manager */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** reference to the frame, inside which this dispatch is used */
        css::uno::Reference< css::frame::XFrame > m_xFrame;

    //___________________________________
    // native interface methods

    public:

                  JobDispatch( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );
         virtual ~JobDispatch(                                                                     );

        void impl_dispatchEvent  ( const ::rtl::OUString&                                            sEvent    ,
                                   const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                   const css::uno::Reference< css::frame::XDispatchResultListener >& xListener );
        void impl_dispatchService( const ::rtl::OUString&                                            sService  ,
                                   const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                   const css::uno::Reference< css::frame::XDispatchResultListener >& xListener );
        void impl_dispatchAlias  ( const ::rtl::OUString&                                            sAlias    ,
                                   const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                   const css::uno::Reference< css::frame::XDispatchResultListener >& xListener );

    //___________________________________
    // uno interface methods

    public:

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // Xinitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& lArguments ) throw(css::uno::Exception       ,
                                                                                                        css::uno::RuntimeException);

        // XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL                       queryDispatch  ( const css::util::URL&                                       aURL             ,
                                                                                                             const ::rtl::OUString&                                      sTargetFrameName ,
                                                                                                                   sal_Int32                                             nSearchFlags     ) throw(css::uno::RuntimeException);
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor      ) throw(css::uno::RuntimeException);

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
};

} // namespace framework

#endif // __FRAMEWORK_JOBS_JOBDISPATCH_HXX_
