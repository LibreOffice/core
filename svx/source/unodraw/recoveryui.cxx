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

#include <config_folders.h>

#include <docrecovery.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <officecfg/Office/Recovery.hxx>

namespace svxdr = svx::DocRecovery;
using namespace ::osl;

namespace {

class RecoveryUI : public ::cppu::WeakImplHelper< css::lang::XServiceInfo        ,
                                                   css::frame::XSynchronousDispatch > // => XDispatch!
{

    // const, types, etcpp.
    private:

        /** @short TODO */
        enum EJob
        {
            E_JOB_UNKNOWN,
            E_DO_EMERGENCY_SAVE,
            E_DO_RECOVERY,
            E_DO_BRINGTOFRONT,
        };


    // member
    private:

        /** @short TODO */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** @short TODO */
        weld::Window* m_pParentWindow;

        /** @short TODO */
        RecoveryUI::EJob m_eJob;

        // Active dialog
        weld::Dialog* m_pDialog;

    // interface
    public:


        /** @short  TODO */
        explicit RecoveryUI(const css::uno::Reference< css::uno::XComponentContext >& xContext);

        // css.lang.XServiceInfo

        virtual OUString SAL_CALL getImplementationName() override;

        virtual sal_Bool SAL_CALL supportsService(const OUString& sServiceName) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;


        virtual css::uno::Any SAL_CALL dispatchWithReturnValue(const css::util::URL& aURL,
                                            const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) override;

        void SetActiveDialog(weld::Dialog* pDialog)
        {
            m_pDialog = pDialog;
        }

    // helper
    private:

        EJob impl_classifyJob(const css::util::URL& aURL);

        bool impl_doEmergencySave();

        bool impl_doRecovery();

        void impl_showAllRecoveredDocs();

        bool impl_doBringToFront();
};

RecoveryUI::RecoveryUI(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : m_xContext(xContext)
    , m_pParentWindow(nullptr)
    , m_eJob(RecoveryUI::E_JOB_UNKNOWN)
    , m_pDialog(nullptr)
{
}

OUString SAL_CALL RecoveryUI::getImplementationName()
{
    return OUString("com.sun.star.comp.svx.RecoveryUI");
}

sal_Bool SAL_CALL RecoveryUI::supportsService(const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL RecoveryUI::getSupportedServiceNames()
{
    return { "com.sun.star.dialog.RecoveryUI" };
}

css::uno::Any SAL_CALL RecoveryUI::dispatchWithReturnValue(const css::util::URL& aURL,
                                                   const css::uno::Sequence< css::beans::PropertyValue >& )
{
    // Internally we use VCL ... every call into vcl based code must
    // be guarded by locking the global solar mutex.
    ::SolarMutexGuard aSolarLock;

    css::uno::Any aRet;
    RecoveryUI::EJob eJob = impl_classifyJob(aURL);
    // TODO think about outside arguments

    switch(eJob)
    {
        case RecoveryUI::E_DO_EMERGENCY_SAVE:
        {
            bool bRet = impl_doEmergencySave();
            aRet <<= bRet;
            break;
        }

        case RecoveryUI::E_DO_RECOVERY:
        {
            bool bRet = impl_doRecovery();
            aRet <<= bRet;
            break;
        }

        case RecoveryUI::E_DO_BRINGTOFRONT:
        {
            bool bRet = impl_doBringToFront();
            aRet <<= bRet;
            break;
        }

        default:
        {
            aRet <<= false;
            break;
        }
    }

    return aRet;
}


OUString GetCrashConfigDir()
{

#if defined(_WIN32)
    OUString    ustrValue = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/bootstrap.ini:UserInstallation}";
#elif defined(MACOSX)
    OUString    ustrValue = "~";
#else
    OUString    ustrValue = "$SYSUSERCONFIG";
#endif
    rtl::Bootstrap::expandMacros( ustrValue );

#if defined(_WIN32)
    ustrValue += "/user/crashdata";
#endif
    return ustrValue;
}


#if defined(_WIN32)
#define LCKFILE "crashdat.lck"
#else
#define LCKFILE ".crash_report_unsent"
#endif


OUString GetUnsentURL()
{
    OUString aURL = GetCrashConfigDir() + "/" LCKFILE;
    return aURL;
}


bool delete_pending_crash()
{
    OUString    aUnsentURL = GetUnsentURL();
    return ( FileBase::E_None == File::remove( aUnsentURL ) );
}

RecoveryUI::EJob RecoveryUI::impl_classifyJob(const css::util::URL& aURL)
{
    m_eJob = RecoveryUI::E_JOB_UNKNOWN;
    if (aURL.Protocol == RECOVERY_CMDPART_PROTOCOL)
    {
        if (aURL.Path == RECOVERY_CMDPART_DO_EMERGENCY_SAVE)
            m_eJob = RecoveryUI::E_DO_EMERGENCY_SAVE;
        else if (aURL.Path == RECOVERY_CMDPART_DO_RECOVERY)
            m_eJob = RecoveryUI::E_DO_RECOVERY;
        else if (aURL.Path == RECOVERY_CMDPART_DO_BRINGTOFRONT)
            m_eJob = RecoveryUI::E_DO_BRINGTOFRONT;
    }

    return m_eJob;
}

struct DialogReleaseGuard
{
    RecoveryUI& m_rRecoveryUI;

    DialogReleaseGuard(RecoveryUI& rRecoveryUI, weld::Dialog* p)
        : m_rRecoveryUI(rRecoveryUI)
    {
        m_rRecoveryUI.SetActiveDialog(p);
    }
    ~DialogReleaseGuard()
    {
        m_rRecoveryUI.SetActiveDialog(nullptr);
    }
};

bool RecoveryUI::impl_doEmergencySave()
{
    // create core service, which implements the real "emergency save" algorithm.
    rtl::Reference<svxdr::RecoveryCore> pCore = new svxdr::RecoveryCore(m_xContext, true);

    // create dialog for this operation and bind it to the used core service
    std::unique_ptr<svxdr::SaveDialog> xDialog(new svxdr::SaveDialog(m_pParentWindow, pCore.get()));
    DialogReleaseGuard dialogReleaseGuard(*this, xDialog->getDialog());

    // start the dialog
    short nRet = xDialog->run();
    return (nRet==DLG_RET_OK_AUTOLUNCH);
}

bool RecoveryUI::impl_doRecovery()
{
    // create core service, which implements the real "emergency save" algorithm.
    rtl::Reference<svxdr::RecoveryCore> pCore = new svxdr::RecoveryCore(m_xContext, false);

    // create all needed dialogs for this operation
    // and bind it to the used core service
    std::unique_ptr<svxdr::RecoveryDialog> xDialog(new svxdr::RecoveryDialog(m_pParentWindow, pCore.get()));
    DialogReleaseGuard dialogReleaseGuard(*this, xDialog->getDialog());

    // start the dialog
    short nRet = xDialog->run();

    impl_showAllRecoveredDocs();

    delete_pending_crash();

    return nRet != RET_CANCEL;
}

void RecoveryUI::impl_showAllRecoveredDocs()
{
    css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( m_xContext );

    css::uno::Reference< css::container::XIndexAccess > xTaskContainer(
        xDesktop->getFrames(),
        css::uno::UNO_QUERY_THROW);

    sal_Int32 c = xTaskContainer->getCount();
    sal_Int32 i = 0;
    for (i=0; i<c; ++i)
    {
        try
        {
            css::uno::Reference< css::frame::XFrame > xTask;
            xTaskContainer->getByIndex(i) >>= xTask;
            if (!xTask.is())
                continue;

            css::uno::Reference< css::awt::XWindow > xWindow = xTask->getContainerWindow();
            if (!xWindow.is())
                continue;

            xWindow->setVisible(true);
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }
    }
}

bool RecoveryUI::impl_doBringToFront()
{
    if (!m_pDialog || !m_pDialog->get_visible())
        return false;
    m_pDialog->present();
    return true;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_RecoveryUI_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new RecoveryUI(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
