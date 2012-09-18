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


#include <string.h>

#include <boost/ptr_container/ptr_vector.hpp>

#include <comphelper/string.hxx>

#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <rtl/process.h>
#include "tools/debug.hxx"
#include "tools/resary.hxx"

#include "unotools/fontcfg.hxx"

#include "cppuhelper/implbase1.hxx"

#include "uno/current_context.hxx"

#include "vcl/configsettings.hxx"
#include "vcl/svapp.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/unohelp.hxx"
#include "vcl/button.hxx" // for Button::GetStandardText
#include "vcl/dockwin.hxx"  // for DockingManager
#include "salinst.hxx"
#include "salframe.hxx"
#include "svdata.hxx"
#include "window.h"
#include "salimestatus.hxx"
#include "salsys.hxx"
#include "svids.hrc"

#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/awt/XExtendedToolkit.hpp"
#include "com/sun/star/java/JavaNotConfiguredException.hpp"
#include "com/sun/star/java/JavaVMCreationFailureException.hpp"
#include "com/sun/star/java/MissingJavaRuntimeException.hpp"
#include "com/sun/star/java/JavaDisabledException.hpp"

#include <stdio.h>

namespace {

namespace css = com::sun::star;

}

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;
using ::rtl::OUString;

// =======================================================================

namespace
{
    struct private_aImplSVData :
        public rtl::Static<ImplSVData, private_aImplSVData> {};
}

// static SV-Data
ImplSVData* pImplSVData = NULL;

SalSystem* ImplGetSalSystem()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpSalSystem )
        pSVData->mpSalSystem = pSVData->mpDefInst->CreateSalSystem();
    return pSVData->mpSalSystem;
}


static rtl::OUString ReplaceJavaErrorMessages( const rtl::OUString& rString )
{
    return rString.replaceAll("%OK", Button::GetStandardText(BUTTON_OK)).
            replaceAll("%IGNORE", Button::GetStandardText(BUTTON_IGNORE)).
            replaceAll("%CANCEL", Button::GetStandardText(BUTTON_CANCEL));
}

// =======================================================================

void ImplInitSVData()
{
    pImplSVData = &private_aImplSVData::get();

    // init global instance data
    memset( pImplSVData, 0, sizeof( ImplSVData ) );
    pImplSVData->maHelpData.mbAutoHelpId = sal_True;
    pImplSVData->maNWFData.maMenuBarHighlightTextColor = Color( COL_TRANSPARENT );

    // mark default layout border as unitialized
    pImplSVData->maAppData.mnDefaultLayoutBorder = -1;
}

// -----------------------------------------------------------------------

void ImplDeInitSVData()
{
    ImplSVData* pSVData = ImplGetSVData();

    // delete global instance data
    if( pSVData->mpSettingsConfigItem )
        delete pSVData->mpSettingsConfigItem;

    if( pSVData->mpDockingManager )
        delete pSVData->mpDockingManager;

    if( pSVData->maGDIData.mpDefaultFontConfiguration )
        delete pSVData->maGDIData.mpDefaultFontConfiguration;
    if( pSVData->maGDIData.mpFontSubstConfiguration )
        delete pSVData->maGDIData.mpFontSubstConfiguration;

    if ( pSVData->maAppData.mpMSFTempFileName )
    {
        if ( pSVData->maAppData.mxMSF.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp( pSVData->maAppData.mxMSF, ::com::sun::star::uno::UNO_QUERY );
            xComp->dispose();
            pSVData->maAppData.mxMSF = NULL;
        }

        ::rtl::OUString aFileUrl;
        ::osl::File::getFileURLFromSystemPath( *pSVData->maAppData.mpMSFTempFileName, aFileUrl );
        osl::File::remove( aFileUrl );
        delete pSVData->maAppData.mpMSFTempFileName;
        pSVData->maAppData.mpMSFTempFileName = NULL;
    }

    if( pSVData->maCtrlData.mpFieldUnitStrings )
        delete pSVData->maCtrlData.mpFieldUnitStrings, pSVData->maCtrlData.mpFieldUnitStrings = NULL;
    if( pSVData->maCtrlData.mpCleanUnitStrings )
        delete pSVData->maCtrlData.mpCleanUnitStrings, pSVData->maCtrlData.mpCleanUnitStrings = NULL;
    if( pSVData->mpPaperNames )
        delete pSVData->mpPaperNames, pSVData->mpPaperNames = NULL;
}

// -----------------------------------------------------------------------

void ImplDestroySVData()
{
    pImplSVData = NULL;
}

// -----------------------------------------------------------------------

Window* ImplGetDefaultWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpAppWin )
        return pSVData->maWinData.mpAppWin;

    // First test if we already have a default window.
    // Don't only place a single if..else inside solar mutex lockframe
    // because then we might have to wait for the solar mutex what is not neccessary
    // if we already have a default window.

    if ( !pSVData->mpDefaultWin )
    {
        Application::GetSolarMutex().acquire();

        // Test again because the thread who released the solar mutex could have called
        // the same method

        if ( !pSVData->mpDefaultWin && !pSVData->mbDeInit )
        {
            DBG_WARNING( "ImplGetDefaultWindow(): No AppWindow" );
            pSVData->mpDefaultWin = new WorkWindow( 0, WB_DEFAULTWIN );
            pSVData->mpDefaultWin->SetText( OUString( "VCL ImplGetDefaultWindow"  ) );
        }
        Application::GetSolarMutex().release();
    }

    return pSVData->mpDefaultWin;
}

// -----------------------------------------------------------------------

#define VCL_CREATERESMGR_NAME( Name )   #Name

ResMgr* ImplGetResMgr()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpResMgr )
    {
        ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
        pSVData->mpResMgr = ResMgr::SearchCreateResMgr( VCL_CREATERESMGR_NAME( vcl ), aLocale );

        static bool bMessageOnce = false;
        if( !pSVData->mpResMgr && ! bMessageOnce )
        {
            bMessageOnce = true;
            const char* pMsg =
                "Missing vcl resource. This indicates that files vital to localization are missing. "
                "You might have a corrupt installation.";
            fprintf( stderr, "%s\n", pMsg );
            ErrorBox aBox( NULL, WB_OK | WB_DEF_OK, rtl::OUString( pMsg, strlen( pMsg ), RTL_TEXTENCODING_ASCII_US ) );
            aBox.Execute();
        }
    }
    return pSVData->mpResMgr;
}

ResId VclResId( sal_Int32 nId )
{
    ResMgr* pMgr = ImplGetResMgr();
    if( ! pMgr )
        throw std::bad_alloc();

    return ResId( nId, *pMgr );
}

FieldUnitStringList* ImplGetFieldUnits()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->maCtrlData.mpFieldUnitStrings )
    {
        ResMgr* pResMgr = ImplGetResMgr();
        if( pResMgr )
        {
            ResStringArray aUnits( ResId (SV_FUNIT_STRINGS, *pResMgr) );
            sal_uInt32 nUnits = aUnits.Count();
            pSVData->maCtrlData.mpFieldUnitStrings = new FieldUnitStringList();
            pSVData->maCtrlData.mpFieldUnitStrings->reserve( nUnits );
            for( sal_uInt32 i = 0; i < nUnits; i++ )
            {
                std::pair< String, FieldUnit > aElement( aUnits.GetString(i), static_cast<FieldUnit>(aUnits.GetValue(i)) );
                pSVData->maCtrlData.mpFieldUnitStrings->push_back( aElement );
            }
        }
    }
    return pSVData->maCtrlData.mpFieldUnitStrings;
}

FieldUnitStringList* ImplGetCleanedFieldUnits()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->maCtrlData.mpCleanUnitStrings )
    {
        FieldUnitStringList* pUnits = ImplGetFieldUnits();
        if( pUnits )
        {
            size_t nUnits = pUnits->size();
            pSVData->maCtrlData.mpCleanUnitStrings = new FieldUnitStringList();
            pSVData->maCtrlData.mpCleanUnitStrings->reserve( nUnits );
            for( size_t i = 0; i < nUnits; ++i )
            {
                rtl::OUString aUnit( (*pUnits)[i].first );
                aUnit = comphelper::string::remove(aUnit, ' ');
                aUnit = aUnit.toAsciiLowerCase();
                std::pair< String, FieldUnit > aElement( aUnit, (*pUnits)[i].second );
                pSVData->maCtrlData.mpCleanUnitStrings->push_back( aElement );
            }
        }
    }
    return pSVData->maCtrlData.mpCleanUnitStrings;
}

DockingManager* ImplGetDockingManager()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpDockingManager )
        pSVData->mpDockingManager = new DockingManager();

    return pSVData->mpDockingManager;
}

class AccessBridgeCurrentContext: public cppu::WeakImplHelper1< com::sun::star::uno::XCurrentContext >
{
public:
    AccessBridgeCurrentContext(
        const com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > &context ) :
        m_prevContext( context ) {}

    // XCurrentContext
    virtual com::sun::star::uno::Any SAL_CALL getValueByName( const rtl::OUString& Name )
        throw (com::sun::star::uno::RuntimeException);
private:
    com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > m_prevContext;
};

com::sun::star::uno::Any AccessBridgeCurrentContext::getValueByName( const rtl::OUString & Name )
    throw (com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any ret;
    if ( Name == "java-vm.interaction-handler" )
    {
        // Currently, for accessbility no interaction handler shall be offered.
        // There may be introduced later on a handler using native toolkits
        // jbu->obr: Instantiate here your interaction handler
    }
    else if( m_prevContext.is() )
    {
        ret = m_prevContext->getValueByName( Name );
    }
    return ret;
}


bool ImplInitAccessBridge(sal_Bool bAllowCancel, sal_Bool &rCancelled)
{
    rCancelled = sal_False;

    bool bErrorMessage = true;

    // Note:
    // if bAllowCancel is sal_True we were called from application startup
    //  where we will disable any Java errorboxes and show our own accessibility dialog if Java throws an exception
    // if bAllowCancel is sal_False we were called from Tools->Options
    //  where we will see Java errorboxes, se we do not show our dialogs in addition to Java's

    try
    {
        bool bSuccess = true;

        // No error messages when env var is set ..
        static const char* pEnv = getenv("SAL_ACCESSIBILITY_ENABLED" );
        if( pEnv && *pEnv )
        {
            bErrorMessage = false;
        }

        ImplSVData* pSVData = ImplGetSVData();
        if( ! pSVData->mxAccessBridge.is() )
        {
            css::uno::Reference< XMultiServiceFactory > xFactory(vcl::unohelper::GetMultiServiceFactory());

            if( xFactory.is() )
            {
                css::uno::Reference< XExtendedToolkit > xToolkit =
                    css::uno::Reference< XExtendedToolkit >(Application::GetVCLToolkit(), UNO_QUERY);

                Sequence< Any > arguments(1);
                arguments[0] = makeAny(xToolkit);

                // Disable default java error messages on startup, because they were probably unreadable
                // for a disabled user. Use native message boxes which are accessible without java support.
                // No need to do this when activated by Tools-Options dialog ..
                if( bAllowCancel )
                {
                    // customize the java-not-available-interaction-handler entry within the
                    // current context when called at startup.
                    com::sun::star::uno::ContextLayer layer(
                        new AccessBridgeCurrentContext( com::sun::star::uno::getCurrentContext() ) );

                    pSVData->mxAccessBridge = xFactory->createInstanceWithArguments(
                            OUString("com.sun.star.accessibility.AccessBridge"),
                            arguments
                        );
                }
                else
                {
                    pSVData->mxAccessBridge = xFactory->createInstanceWithArguments(
                            OUString("com.sun.star.accessibility.AccessBridge"),
                            arguments
                        );
                }

                if( !pSVData->mxAccessBridge.is() )
                    bSuccess = false;
            }
        }

        return bSuccess;
    }
    catch (const ::com::sun::star::java::JavaNotConfiguredException&)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && bAllowCancel && pResMgr )
        {
            rtl::OUString aTitle(ResId(SV_ACCESSERROR_JAVA_NOT_CONFIGURED, *pResMgr).toString());
            rtl::OUStringBuffer aMessage((ResId(SV_ACCESSERROR_JAVA_MSG, *pResMgr)).toString());

            aMessage.append(' ').append(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr).toString());

            int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                aTitle,
                ReplaceJavaErrorMessages(aMessage.makeStringAndClear()),
                SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, true);

            // Do not change the setting in case the user chooses to cancel
            if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                rCancelled = sal_True;
        }

        return false;
    }
    catch (const ::com::sun::star::java::JavaVMCreationFailureException&)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && bAllowCancel && pResMgr )
        {
            rtl::OUString aTitle(ResId(SV_ACCESSERROR_FAULTY_JAVA, *pResMgr).toString());
            rtl::OUStringBuffer aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, *pResMgr).toString());

            aMessage.append(' ').append(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr).toString());

            int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                aTitle,
                ReplaceJavaErrorMessages(aMessage.makeStringAndClear()),
                SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, true);

            // Do not change the setting in case the user chooses to cancel
            if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                rCancelled = sal_True;
        }

        return false;
    }
    catch (const ::com::sun::star::java::MissingJavaRuntimeException&)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && bAllowCancel && pResMgr )
        {
            rtl::OUString aTitle(ResId(SV_ACCESSERROR_MISSING_JAVA, *pResMgr).toString());
            rtl::OUStringBuffer aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, *pResMgr).toString());

            aMessage.append(' ').append(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr).toString());

            int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                aTitle,
                ReplaceJavaErrorMessages(aMessage.makeStringAndClear()),
                SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, true);

            // Do not change the setting in case the user chooses to cancel
            if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                rCancelled = sal_True;
        }

        return false;
    }
    catch (const ::com::sun::star::java::JavaDisabledException&)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && bAllowCancel && pResMgr )
        {
            rtl::OUString aTitle(ResId(SV_ACCESSERROR_JAVA_DISABLED, *pResMgr).toString());
            rtl::OUStringBuffer aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, *pResMgr).toString());

            aMessage.append(' ').append(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr).toString());

            int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                aTitle,
                ReplaceJavaErrorMessages(aMessage.makeStringAndClear()),
                SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, true);

            // Do not change the setting in case the user chooses to cancel
            if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                rCancelled = sal_True;
        }

        return false;
    }
    catch (const ::com::sun::star::uno::RuntimeException& e)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && pResMgr )
        {
            rtl::OUString aTitle;
            rtl::OUStringBuffer aMessage(ResId(SV_ACCESSERROR_BRIDGE_MSG, *pResMgr).toString());

            if( 0 == e.Message.compareTo(::rtl::OUString("ClassNotFound"), 13) )
            {
                aTitle = ResId(SV_ACCESSERROR_MISSING_BRIDGE, *pResMgr).toString();
            }
            else if( 0 == e.Message.compareTo(::rtl::OUString("NoSuchMethod"), 12) )
            {
                aTitle = ResId(SV_ACCESSERROR_WRONG_VERSION, *pResMgr).toString();
            }

            if (!aTitle.isEmpty())
            {
                if( bAllowCancel )
                {
                    // Something went wrong initializing the Java AccessBridge (on Windows) during the
                    // startup. Since the office will be probably unusable for a disabled user, we offer
                    // to terminate directly.
                    aMessage.append(' ').append(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr).toString());

                    int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                        aTitle,
                        ReplaceJavaErrorMessages(aMessage.makeStringAndClear()),
                        SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                        SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, true);

                    // Do not change the setting in case the user chooses to cancel
                    if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                        rCancelled = sal_True;
                }
                else
                {
                    // The user tried to activate accessibility support using Tools-Options dialog,
                    // so we don't offer to terminate here !
                    ImplGetSalSystem()->ShowNativeMessageBox(
                        aTitle,
                        ReplaceJavaErrorMessages(aMessage.makeStringAndClear()),
                        SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK,
                        SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK, true);
                }
            }
        }

        return false;
    }
    catch (...)
    {
        return false;
    }
}

// -----------------------------------------------------------------------

Window* ImplFindWindow( const SalFrame* pFrame, Point& rSalFramePos )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pFrameWindow = pSVData->maWinData.mpFirstFrame;
    while ( pFrameWindow )
    {
        if ( pFrameWindow->ImplGetFrame() == pFrame )
        {
            Window* pWindow = pFrameWindow->ImplFindWindow( rSalFramePos );
            if ( !pWindow )
                pWindow = pFrameWindow->ImplGetWindow();
            rSalFramePos = pWindow->ImplFrameToOutput( rSalFramePos );
            return pWindow;
        }
        pFrameWindow = pFrameWindow->ImplGetFrameData()->mpNextFrame;
    }

    return NULL;
}

void LocaleConfigurationListener::ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 nHint )
{
    AllSettings::LocaleSettingsChanged( nHint );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
