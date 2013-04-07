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

#ifndef __FRAMEWORK_JOBS_JOBEXECUTOR_HXX_
#define __FRAMEWORK_JOBS_JOBEXECUTOR_HXX_

#include <jobs/configaccess.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <macros/debug.hxx>
#include <stdtypes.h>
#include <general.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>

#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>


namespace framework{


//_______________________________________
/**
    @short  implements a job executor, which can be triggered from any code
    @descr  It uses the given trigger event to locate any registered job service
            inside the configuration and execute it. Of course it controls the
            liftime of such jobs too.
 */
class JobExecutor : public  css::lang::XTypeProvider
                  , public  css::lang::XServiceInfo
                  , public  css::task::XJobExecutor
                  , public  css::container::XContainerListener // => lang.XEventListener
                  , public  css::document::XEventListener
                  , private ThreadHelpBase
                  , public  ::cppu::OWeakObject
{
    //___________________________________
    // member

    private:

        /** reference to the uno service manager */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** reference to the module info service */
        css::uno::Reference< css::frame::XModuleManager2 > m_xModuleManager;

        /** cached list of all registered event names of cfg for call optimization. */
        OUStringList m_lEvents;

        /** we listen at the configuration for changes at the event list. */
        ConfigAccess m_aConfig;

        /** helper to allow us listen to the configuration without a cyclic dependency */
        com::sun::star::uno::Reference<com::sun::star::container::XContainerListener> m_xConfigListener;

    //___________________________________
    // native interface methods

    public:

                  JobExecutor( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );
         virtual ~JobExecutor(                                                                     );

    //___________________________________
    // uno interface methods

    public:

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // task.XJobExecutor
        virtual void SAL_CALL trigger( const OUString& sEvent ) throw(css::uno::RuntimeException);

        // document.XEventListener
        virtual void SAL_CALL notifyEvent( const css::document::EventObject& aEvent ) throw(css::uno::RuntimeException);

        // container.XContainerListener
        virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& aEvent ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved ( const css::container::ContainerEvent& aEvent ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& aEvent ) throw(css::uno::RuntimeException);

        // lang.XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException);
};

} // namespace framework

#endif // __FRAMEWORK_JOBS_JOBEXECUTOR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
