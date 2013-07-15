/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FRAMEWORK_JOBS_JOBDISPATCH_HXX_
#define __FRAMEWORK_JOBS_JOBDISPATCH_HXX_

#include <jobs/configaccess.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <macros/debug.hxx>
#include <stdtypes.h>
#include <general.h>

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

#include <unotools/configpaths.hxx>
#include <cppuhelper/implbase4.hxx>
#include <rtl/ustring.hxx>


namespace framework{


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
class JobDispatch : private ThreadHelpBase
                  , public  ::cppu::WeakImplHelper4<
                            css::lang::XServiceInfo
                          , css::lang::XInitialization
                          , css::frame::XDispatchProvider
                          , css::frame::XNotifyingDispatch >      // => XDispatch
{
    //___________________________________
    // member

    private:

        /** reference to the uno service manager */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** reference to the frame, inside which this dispatch is used */
        css::uno::Reference< css::frame::XFrame > m_xFrame;

        /** name of module (writer, impress etc.) the frame is for */
        OUString m_sModuleIdentifier;

    //___________________________________
    // native interface methods

    public:

                  JobDispatch( const css::uno::Reference< css::uno::XComponentContext >& xContext );
         virtual ~JobDispatch(                                                                     );

        void impl_dispatchEvent  ( const OUString&                                            sEvent    ,
                                   const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                   const css::uno::Reference< css::frame::XDispatchResultListener >& xListener );
        void impl_dispatchService( const OUString&                                            sService  ,
                                   const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                   const css::uno::Reference< css::frame::XDispatchResultListener >& xListener );
        void impl_dispatchAlias  ( const OUString&                                            sAlias    ,
                                   const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                   const css::uno::Reference< css::frame::XDispatchResultListener >& xListener );

    //___________________________________
    // uno interface methods

    public:

        // XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        // Xinitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& lArguments ) throw(css::uno::Exception       ,
                                                                                                        css::uno::RuntimeException);

        // XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL                       queryDispatch  ( const css::util::URL&                                       aURL             ,
                                                                                                             const OUString&                                      sTargetFrameName ,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
