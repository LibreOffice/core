/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <com/sun/star/frame/XModuleManager.hpp>

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
        css::uno::Reference< css::frame::XModuleManager > m_xModuleManager;

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
        virtual void SAL_CALL trigger( const ::rtl::OUString& sEvent ) throw(css::uno::RuntimeException);

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
