/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include <com/sun/star/oooimprovement/XCore.hpp>

#include "oooimprovecore_module.hxx"
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/oooimprovement/XCoreController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/componentmodule.hxx>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/uieventslogger.hxx>
#include <cppuhelper/implbase3.hxx>
#include <svx/svxdlg.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <sfx2/app.hxx>
#include <svx/dialogs.hrc>
#include <sfx2/sfxsids.hrc>

using namespace ::com::sun::star::oooimprovement;
using ::com::sun::star::frame::XTerminateListener;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::uno::XInterface;
using ::comphelper::UiEventsLogger;
using ::rtl::OUString;

// declaration
namespace oooimprovecore
{
    class Core : public ::cppu::WeakImplHelper3<XCore,XServiceInfo,XTerminateListener>
    {
        public:
            // XServiceInfo - static version
            static OUString SAL_CALL getImplementationName_static();
            static Sequence<OUString> SAL_CALL getSupportedServiceNames_static();
            static Reference<XInterface> Create(const Reference<XComponentContext>& context );

        protected:
            Core(const Reference<XComponentContext>&);
            virtual ~Core();

            // XCore
            virtual sal_Int32 SAL_CALL getSessionLogEventCount() throw(RuntimeException);
            virtual sal_Bool SAL_CALL getUiEventsLoggerEnabled() throw(RuntimeException);
            virtual void SAL_CALL inviteUser() throw(RuntimeException);

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
            virtual sal_Bool SAL_CALL supportsService(const OUString& service_name) throw(RuntimeException);
            virtual Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(RuntimeException);

            // XTerminateListener
            virtual void SAL_CALL queryTermination(const EventObject&) throw(RuntimeException);
            virtual void SAL_CALL notifyTermination(const EventObject&) throw(RuntimeException);

            // XEventListener
            virtual void SAL_CALL disposing(const EventObject&) throw(RuntimeException);
    };
}


// implementation
namespace oooimprovecore
{

    Core::Core(const Reference<XComponentContext>&)
    { }

    Core::~Core()
    { }

    sal_Int32 SAL_CALL Core::getSessionLogEventCount() throw(RuntimeException)
    { return UiEventsLogger::getSessionLogEventCount(); }

    sal_Bool SAL_CALL Core::getUiEventsLoggerEnabled() throw(RuntimeException)
    { return UiEventsLogger::isEnabled(); }

    void SAL_CALL Core::inviteUser() throw(RuntimeException)
    {
        Reference<XMultiServiceFactory> xServiceFactory = ::comphelper::getProcessServiceFactory();

        OUString help_url;
        Reference<XCoreController> core_c(
            xServiceFactory->createInstance(OUString::createFromAscii("com.sun.star.oooimprovement.CoreController")),
            UNO_QUERY);
        if(core_c.is())
            ::comphelper::ConfigurationHelper::readDirectKey(
                xServiceFactory,
                OUString::createFromAscii("/org.openoffice.Office.OOoImprovement.Settings"),
                OUString::createFromAscii("Participation"),
                OUString::createFromAscii("HelpUrl"),
                ::comphelper::ConfigurationHelper::E_READONLY) >>= help_url;
        else
            help_url = OUString::createFromAscii("http://www.openoffice.org");
        {
            SolarMutexGuard aGuard;
            SfxAllItemSet aSet( SFX_APP()->GetPool() );
            aSet.Put( SfxStringItem( SID_CURRENT_URL, help_url ) );
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            if ( pFact )
            {
                SfxAbstractDialog *pDlg = pFact->CreateSfxDialog( NULL, aSet, 0, RID_SVXPAGE_IMPROVEMENT );
                pDlg->Execute();
                delete pDlg;
            }
        }
    }

    sal_Bool SAL_CALL Core::supportsService(const OUString& service_name) throw(RuntimeException)
    {
        const Sequence<OUString> service_names(getSupportedServiceNames());
        for (sal_Int32 idx = service_names.getLength()-1; idx>=0; --idx)
            if(service_name == service_names[idx]) return sal_True;
        return sal_False;
    }

    OUString SAL_CALL Core::getImplementationName() throw(RuntimeException)
    { return getImplementationName_static(); }

    Sequence<OUString> SAL_CALL Core::getSupportedServiceNames() throw(RuntimeException)
    { return getSupportedServiceNames_static(); }

    OUString SAL_CALL Core::getImplementationName_static()
    { return OUString::createFromAscii("com.sun.star.comp.extensions.oooimprovecore.Core"); }

    Sequence<OUString> SAL_CALL Core::getSupportedServiceNames_static()
    {
        Sequence<OUString> aServiceNames(1);
        aServiceNames[0] = OUString::createFromAscii("com.sun.star.oooimprovement.Core");
        return aServiceNames;
    }

    void Core::queryTermination(const EventObject&) throw(RuntimeException)
    { }

    void Core::notifyTermination(const EventObject&) throw(RuntimeException)
    {
        UiEventsLogger::disposing();
    }

    void Core::disposing(const EventObject&) throw(RuntimeException)
    { }

    Reference<XInterface> Core::Create(const Reference<XComponentContext>& context)
    { return *(new Core(context)); }

    void createRegistryInfo_Core()
    {
        static OAutoRegistration<Core> auto_reg;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
