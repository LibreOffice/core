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

#include "recoveryui.hxx"
#include "docrecovery.hxx"
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <vcl/svapp.hxx>

#include <boost/scoped_ptr.hpp>
#include <officecfg/Office/Recovery.hxx>

namespace svx
{

namespace svxdr = ::svx::DocRecovery;

using namespace ::rtl;
using namespace ::osl;

RecoveryUI::RecoveryUI(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : m_xContext     (xContext                 )
    , m_pParentWindow(0                        )
    , m_eJob         (RecoveryUI::E_JOB_UNKNOWN)
{
}

RecoveryUI::~RecoveryUI()
{
}

OUString SAL_CALL RecoveryUI::getImplementationName()
    throw(css::uno::RuntimeException)
{
    return RecoveryUI::st_getImplementationName();
}

sal_Bool SAL_CALL RecoveryUI::supportsService(const OUString& sServiceName)
    throw(css::uno::RuntimeException)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL RecoveryUI::getSupportedServiceNames()
    throw(css::uno::RuntimeException)
{
    return RecoveryUI::st_getSupportedServiceNames();
}

css::uno::Any SAL_CALL RecoveryUI::dispatchWithReturnValue(const css::util::URL& aURL,
                                                   const css::uno::Sequence< css::beans::PropertyValue >& )
    throw(css::uno::RuntimeException)
{
    // Internaly we use VCL ... every call into vcl based code must
    // be guarded by locking the global solar mutex.
    ::SolarMutexGuard aSolarLock;

    css::uno::Any aRet;
    RecoveryUI::EJob eJob = impl_classifyJob(aURL);
    // TODO think about outside arguments

    switch(eJob)
    {
        case RecoveryUI::E_DO_EMERGENCY_SAVE :
        {
            sal_Bool bRet = impl_doEmergencySave();
            aRet <<= bRet;
            break;
        }

        case RecoveryUI::E_DO_RECOVERY :
            impl_doRecovery();
            break;

        case RecoveryUI::E_DO_CRASHREPORT :
            impl_doCrashReport();
            break;

        default :
            break;
    }

    return aRet;
}

//===============================================
void SAL_CALL RecoveryUI::dispatch(const css::util::URL&                                  aURL      ,
                                   const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
    throw(css::uno::RuntimeException)
{
    // recycle this method :-)
    dispatchWithReturnValue(aURL, lArguments);
}

//===============================================
void SAL_CALL RecoveryUI::addStatusListener(const css::uno::Reference< css::frame::XStatusListener >&, const css::util::URL& ) throw(css::uno::RuntimeException)
{
    // TODO
    OSL_FAIL("RecoveryUI::addStatusListener()\nNot implemented yet!");
}

//===============================================
void SAL_CALL RecoveryUI::removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >&, const css::util::URL& )
    throw(css::uno::RuntimeException)
{
    // TODO
    OSL_FAIL("RecoveryUI::removeStatusListener()\nNot implemented yet!");
}

OUString RecoveryUI::st_getImplementationName()
{
    return OUString("com.sun.star.comp.svx.RecoveryUI");
}

css::uno::Sequence< OUString > RecoveryUI::st_getSupportedServiceNames()
{
    css::uno::Sequence< OUString > lServiceNames(1);
    lServiceNames.getArray() [0] = OUString("com.sun.star.dialog.RecoveryUI");
    return lServiceNames;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL RecoveryUI::st_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    RecoveryUI* pNew = new RecoveryUI(comphelper::getComponentContext(xSMGR));
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::lang::XServiceInfo* >(pNew));
}

static OUString GetCrashConfigDir()
{

#if defined(WNT)
    OUString    ustrValue = OUString("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/bootstrap.ini:UserInstallation}");
#elif defined(MACOSX)
    OUString    ustrValue = OUString("~");
#else
    OUString    ustrValue = OUString("$SYSUSERCONFIG");
#endif
    Bootstrap::expandMacros( ustrValue );

#if defined(WNT)
    ustrValue += "/user/crashdata";
#endif
    return ustrValue;
}

//===============================================

#if defined(WNT)
#define LCKFILE "crashdat.lck"
#else
#define LCKFILE ".crash_report_unsent"
#endif


static OUString GetUnsentURL()
{
    OUString    aURL = GetCrashConfigDir();

    aURL += "/";
    aURL += OUString(  LCKFILE  );

    return aURL;
}

//===============================================

static bool new_crash_pending()
{
    OUString    aUnsentURL = GetUnsentURL();
    File    aFile( aUnsentURL );

    if ( FileBase::E_None == aFile.open( osl_File_OpenFlag_Read ) )
    {
        aFile.close();
        return true;
    }

    return false;
}
//===============================================

static bool delete_pending_crash()
{
    OUString    aUnsentURL = GetUnsentURL();
    return ( FileBase::E_None == File::remove( aUnsentURL ) );
}

RecoveryUI::EJob RecoveryUI::impl_classifyJob(const css::util::URL& aURL)
{
    m_eJob = RecoveryUI::E_JOB_UNKNOWN;
    if (aURL.Protocol.equals(RECOVERY_CMDPART_PROTOCOL))
    {
        if (aURL.Path.equals(RECOVERY_CMDPART_DO_EMERGENCY_SAVE))
            m_eJob = RecoveryUI::E_DO_EMERGENCY_SAVE;
        else
        if (aURL.Path.equals(RECOVERY_CMDPART_DO_RECOVERY))
            m_eJob = RecoveryUI::E_DO_RECOVERY;
        else
        if (aURL.Path.equals(RECOVERY_CMDPART_DO_CRASHREPORT))
            m_eJob = RecoveryUI::E_DO_CRASHREPORT;
    }

    return m_eJob;
}

//===============================================
sal_Bool RecoveryUI::impl_doEmergencySave()
{
    // create core service, which implements the real "emergency save" algorithm.
    svxdr::RecoveryCore* pCore = new svxdr::RecoveryCore(m_xContext, true);
    css::uno::Reference< css::frame::XStatusListener > xCore(pCore);

    // create all needed dialogs for this operation
    // and bind it to the used core service
    svxdr::TabDialog4Recovery* pWizard = new svxdr::TabDialog4Recovery(m_pParentWindow);
    svxdr::IExtendedTabPage*   pPage1  = new svxdr::SaveDialog        (pWizard, pCore );
    pWizard->addTabPage(pPage1);

    // start the wizard
    short nRet = pWizard->Execute();

    delete pPage1 ;
    delete pWizard;

    return (nRet==DLG_RET_OK_AUTOLUNCH);
}

//===============================================
void RecoveryUI::impl_doRecovery()
{
    bool bRecoveryOnly( false );

    bool bCrashRepEnabled(officecfg::Office::Recovery::CrashReporter::Enabled::get(m_xContext));
    bRecoveryOnly = !bCrashRepEnabled;

    // create core service, which implements the real "emergency save" algorithm.
    svxdr::RecoveryCore* pCore = new svxdr::RecoveryCore(m_xContext, false);
    css::uno::Reference< css::frame::XStatusListener > xCore(pCore);

    // create all needed dialogs for this operation
    // and bind it to the used core service
    boost::scoped_ptr<svxdr::TabDialog4Recovery> xWizard(new svxdr::TabDialog4Recovery(m_pParentWindow));
    svxdr::IExtendedTabPage*   pPage1  = new svxdr::RecoveryDialog(xWizard.get(), pCore );
    svxdr::IExtendedTabPage*   pPage2  = 0;
    svxdr::IExtendedTabPage*   pPage3  = 0;

    xWizard->addTabPage(pPage1);
    if ( !bRecoveryOnly && new_crash_pending() )
    {
        pPage2 = new svxdr::ErrorRepWelcomeDialog(xWizard.get());
        pPage3 = new svxdr::ErrorRepSendDialog(xWizard.get());
        xWizard->addTabPage(pPage2);
        xWizard->addTabPage(pPage3);
    }

    // start the wizard
    xWizard->Execute();

    impl_showAllRecoveredDocs();

    delete pPage3 ;
    delete pPage2 ;
    delete pPage1 ;

    delete_pending_crash();
}

//===============================================

void RecoveryUI::impl_doCrashReport()
{
    if ( new_crash_pending() )
    {
        svxdr::TabDialog4Recovery* pWizard = new svxdr::TabDialog4Recovery   (m_pParentWindow   );
        svxdr::IExtendedTabPage*   pPage1  = new svxdr::ErrorRepWelcomeDialog(pWizard, sal_False);
        svxdr::IExtendedTabPage*   pPage2  = new svxdr::ErrorRepSendDialog   (pWizard           );
        pWizard->addTabPage(pPage1);
        pWizard->addTabPage(pPage2);

        // start the wizard
        pWizard->Execute();

        delete pPage2 ;
        delete pPage1 ;
        delete pWizard;

        delete_pending_crash();
    }
}

//===============================================
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

            xWindow->setVisible(sal_True);
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }
    }
}

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
