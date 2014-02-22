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
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <cppuhelper/implbase2.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ref.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <boost/scoped_ptr.hpp>
#include <officecfg/Office/Recovery.hxx>

namespace svxdr = ::svx::DocRecovery;
using namespace ::osl;

namespace {

class RecoveryUI : public ::cppu::WeakImplHelper2< css::lang::XServiceInfo        ,
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
        };


    // member
    private:

        /** @short TODO */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** @short TODO */
        Window* m_pParentWindow;

        /** @short TODO */
        RecoveryUI::EJob m_eJob;

        /** @short TODO */
        css::uno::Reference< css::task::XStatusIndicatorFactory > m_xProgressFactory;


    // interface
    public:


        /** @short  TODO */
        RecoveryUI(const css::uno::Reference< css::uno::XComponentContext >& xContext);


        /** @short  TODO */
        virtual ~RecoveryUI();


        // css.lang.XServiceInfo

        virtual OUString SAL_CALL getImplementationName()
            throw(css::uno::RuntimeException);

        virtual sal_Bool SAL_CALL supportsService(const OUString& sServiceName)
            throw(css::uno::RuntimeException);

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw(css::uno::RuntimeException);


        virtual com::sun::star::uno::Any SAL_CALL dispatchWithReturnValue(const css::util::URL& aURL,
                                            const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
            throw(css::uno::RuntimeException);


        // css.frame.XDispatch

        virtual void SAL_CALL dispatch(const css::util::URL&                                  aURL      ,
                                       const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException);
        virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                   const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException);


    // helper
    private:

        EJob impl_classifyJob(const css::util::URL& aURL);

        sal_Bool impl_doEmergencySave();

        void impl_doRecovery();

        void impl_showAllRecoveredDocs();

};

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
    return OUString("com.sun.star.comp.svx.RecoveryUI");
}

sal_Bool SAL_CALL RecoveryUI::supportsService(const OUString& sServiceName)
    throw(css::uno::RuntimeException)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL RecoveryUI::getSupportedServiceNames()
    throw(css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > lServiceNames(1);
    lServiceNames[0] = "com.sun.star.dialog.RecoveryUI";
    return lServiceNames;
}

css::uno::Any SAL_CALL RecoveryUI::dispatchWithReturnValue(const css::util::URL& aURL,
                                                   const css::uno::Sequence< css::beans::PropertyValue >& )
    throw(css::uno::RuntimeException)
{
    // Internally we use VCL ... every call into vcl based code must
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

static OUString GetCrashConfigDir()
{

#if defined(WNT)
    OUString    ustrValue = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/bootstrap.ini:UserInstallation}";
#elif defined(MACOSX)
    OUString    ustrValue = "~";
#else
    OUString    ustrValue = "$SYSUSERCONFIG";
#endif
    rtl::Bootstrap::expandMacros( ustrValue );

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
    OUString aURL = GetCrashConfigDir() + "/" + LCKFILE;
    return aURL;
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
        else if (aURL.Path.equals(RECOVERY_CMDPART_DO_RECOVERY))
            m_eJob = RecoveryUI::E_DO_RECOVERY;
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

    // start the wizard
    xWizard->Execute();

    impl_showAllRecoveredDocs();

    delete pPage3 ;
    delete pPage2 ;
    delete pPage1 ;

    delete_pending_crash();
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

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_svx_RecoveryUI_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new RecoveryUI(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
