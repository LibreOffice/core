/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: recoveryui.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 12:45:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

//===============================================
// includes

#ifndef _RECOVERYUI_HXX
#include "recoveryui.hxx"
#endif

#ifndef _SVX_DOCRECOVERY_HXX
#include "docrecovery.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif

#ifndef _COMPHELPER_CONFIGURATIONHELPER_HXX_
#include <comphelper/configurationhelper.hxx>
#endif

#include <vcl/svapp.hxx>

//===============================================
// const

#define IMPLEMENTATIONNAME_RECOVERYUI       ::rtl::OUString::createFromAscii("com.sun.star.comp.svx.RecoveryUI")
#define SERVICENAME_RECOVERYUI              ::rtl::OUString::createFromAscii("com.sun.star.dialog.RecoveryUI")

//===============================================
// namespace

namespace svx
{

namespace css   = ::com::sun::star;
namespace svxdr = ::svx::DocRecovery;

using namespace ::rtl;
using namespace ::osl;

//===============================================
RecoveryUI::RecoveryUI(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : m_xSMGR        (xSMGR                    )
    , m_pParentWindow(0                        )
    , m_eJob         (RecoveryUI::E_JOB_UNKNOWN)
{
}

//===============================================
RecoveryUI::~RecoveryUI()
{
}

//===============================================
::rtl::OUString SAL_CALL RecoveryUI::getImplementationName()
    throw(css::uno::RuntimeException)
{
    return RecoveryUI::st_getImplementationName();
}

//===============================================
sal_Bool SAL_CALL RecoveryUI::supportsService(const ::rtl::OUString& sServiceName)
    throw(css::uno::RuntimeException)
{
    const css::uno::Sequence< ::rtl::OUString > lServices = RecoveryUI::st_getSupportedServiceNames();
          sal_Int32                             c         = lServices.getLength();
          sal_Int32                             i         = 0;
    for (i=0; i<c; ++i)
    {
        const ::rtl::OUString& sSupportedService = lServices[i];
        if (sSupportedService.equals(sServiceName))
            return sal_True;
    }
    return sal_False;
}

//===============================================
css::uno::Sequence< ::rtl::OUString > SAL_CALL RecoveryUI::getSupportedServiceNames()
    throw(css::uno::RuntimeException)
{
    return RecoveryUI::st_getSupportedServiceNames();
}

//===============================================
css::uno::Any SAL_CALL RecoveryUI::dispatchWithReturnValue(const css::util::URL& aURL,
                                                   const css::uno::Sequence< css::beans::PropertyValue >& )
    throw(css::uno::RuntimeException)
{
    // Internaly we use VCL ... every call into vcl based code must
    // be guarded by locking the global solar mutex.
    ::vos::OGuard aSolarLock(&Application::GetSolarMutex());

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
    OSL_ENSURE(sal_False, "RecoveryUI::addStatusListener()\nNot implemented yet!");
}

//===============================================
void SAL_CALL RecoveryUI::removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >&, const css::util::URL& )
    throw(css::uno::RuntimeException)
{
    // TODO
    OSL_ENSURE(sal_False, "RecoveryUI::removeStatusListener()\nNot implemented yet!");
}

//===============================================
::rtl::OUString RecoveryUI::st_getImplementationName()
{
    static ::rtl::OUString IMPLEMENTATIONNAME = IMPLEMENTATIONNAME_RECOVERYUI;
    return IMPLEMENTATIONNAME;
}

//===============================================
css::uno::Sequence< ::rtl::OUString > RecoveryUI::st_getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > lServiceNames(1);    lServiceNames.getArray() [0] = SERVICENAME_RECOVERYUI;
    return lServiceNames;
}

//===============================================
css::uno::Reference< css::uno::XInterface > SAL_CALL RecoveryUI::st_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    RecoveryUI* pNew = new RecoveryUI(xSMGR);
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::lang::XServiceInfo* >(pNew));
}

//===============================================

static OUString GetCrashConfigDir()
{

#if defined(WNT) || defined(OS2)
    OUString    ustrValue = OUString::createFromAscii("${$BRAND_BASE_DIR/program/bootstrap.ini:UserInstallation}");
#else
    OUString    ustrValue = OUString::createFromAscii("$SYSUSERCONFIG");
#endif
    Bootstrap::expandMacros( ustrValue );

#if defined(WNT) || defined(OS2)
    ustrValue += OUString::createFromAscii("/user/crashdata");
#endif
    return ustrValue;
}

//===============================================

#if defined(WNT) || defined(OS2)
#define LCKFILE "crashdat.lck"
#else
#define LCKFILE ".crash_report_unsent"
#endif


static OUString GetUnsentURL()
{
    OUString    aURL = GetCrashConfigDir();

    aURL += OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) );
    aURL += OUString( RTL_CONSTASCII_USTRINGPARAM( LCKFILE ) );

    return aURL;
}

//===============================================

static bool new_crash_pending()
{
    OUString    aUnsentURL = GetUnsentURL();
    File    aFile( aUnsentURL );

    if ( FileBase::E_None == aFile.open( OpenFlag_Read ) )
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
    svxdr::RecoveryCore* pCore = new svxdr::RecoveryCore(m_xSMGR, sal_True);
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
    sal_Bool bRecoveryOnly( sal_False );

    ::rtl::OUString CFG_PACKAGE_RECOVERY( RTL_CONSTASCII_USTRINGPARAM  ( "org.openoffice.Office.Recovery/" ));
    ::rtl::OUString CFG_PATH_CRASHREPORTER( RTL_CONSTASCII_USTRINGPARAM( "CrashReporter"                 ));
    ::rtl::OUString CFG_ENTRY_ENABLED( RTL_CONSTASCII_USTRINGPARAM     ( "Enabled"                       ));

    sal_Bool bCrashRepEnabled( sal_True );
    css::uno::Any aVal = ::comphelper::ConfigurationHelper::readDirectKey(
                                m_xSMGR,
                                CFG_PACKAGE_RECOVERY,
                                CFG_PATH_CRASHREPORTER,
                                CFG_ENTRY_ENABLED,
                                ::comphelper::ConfigurationHelper::E_READONLY);
    aVal >>= bCrashRepEnabled;
    bRecoveryOnly = !bCrashRepEnabled;

    // create core service, which implements the real "emergency save" algorithm.
    svxdr::RecoveryCore* pCore = new svxdr::RecoveryCore(m_xSMGR, sal_False);
    css::uno::Reference< css::frame::XStatusListener > xCore(pCore);

    // create all needed dialogs for this operation
    // and bind it to the used core service
    svxdr::TabDialog4Recovery* pWizard = new svxdr::TabDialog4Recovery   (m_pParentWindow);
    svxdr::IExtendedTabPage*   pPage1  = new svxdr::RecoveryDialog       (pWizard, pCore );
    svxdr::IExtendedTabPage*   pPage2  = 0;
    svxdr::IExtendedTabPage*   pPage3  = 0;

    pWizard->addTabPage(pPage1);
    if ( !bRecoveryOnly && new_crash_pending() )
    {
        pPage2 = new svxdr::ErrorRepWelcomeDialog(pWizard        );
        pPage3 = new svxdr::ErrorRepSendDialog   (pWizard        );
        pWizard->addTabPage(pPage2);
        pWizard->addTabPage(pPage3);
    }

    // start the wizard
    pWizard->Execute();

    impl_showAllRecoveredDocs();

    delete pPage3 ;
    delete pPage2 ;
    delete pPage1 ;
    delete pWizard;

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
    css::uno::Reference< css::frame::XFramesSupplier > xDesktop(
        m_xSMGR->createInstance(SERVICENAME_DESKTOP),
        css::uno::UNO_QUERY_THROW);

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
        catch(const css::uno::RuntimeException& exRun)
            { throw exRun; }
        catch(const css::uno::Exception&)
            { continue; }
    }
}

} // namespace svx
