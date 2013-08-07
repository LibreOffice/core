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

#include <string.h>
#include <boost/ptr_container/ptr_vector.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <osl/mutex.hxx>
#include <rtl/process.h>
#include <tools/debug.hxx>
#include <tools/resary.hxx>
#include <tools/gen.hxx>
#include <unotools/fontcfg.hxx>
#include <cppuhelper/implbase1.hxx>
#include <uno/current_context.hxx>

#include "vcl/configsettings.hxx"
#include "vcl/svapp.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/button.hxx" // for Button::GetStandardText
#include "vcl/dockwin.hxx"  // for DockingManager
#include "salinst.hxx"
#include "salframe.hxx"
#include "svdata.hxx"
#include "window.h"
#include "salimestatus.hxx"
#include "salsys.hxx"
#include "svids.hrc"

#include "com/sun/star/accessibility/AccessBridge.hpp"
#include "com/sun/star/awt/XExtendedToolkit.hpp"
#include "com/sun/star/java/JavaNotConfiguredException.hpp"
#include "com/sun/star/java/JavaVMCreationFailureException.hpp"
#include "com/sun/star/java/MissingJavaRuntimeException.hpp"
#include "com/sun/star/java/JavaDisabledException.hpp"

#include <stdio.h>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;

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


static OUString ReplaceJavaErrorMessages( const OUString& rString )
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
    // because then we might have to wait for the solar mutex what is not necessary
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

ResMgr* ImplGetResMgr()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpResMgr )
    {
        LanguageTag aLocale( Application::GetSettings().GetUILanguageTag());
        pSVData->mpResMgr = ResMgr::SearchCreateResMgr( "vcl", aLocale );

        static bool bMessageOnce = false;
        if( !pSVData->mpResMgr && ! bMessageOnce )
        {
            bMessageOnce = true;
            const char* pMsg =
                "Missing vcl resource. This indicates that files vital to localization are missing. "
                "You might have a corrupt installation.";
            fprintf( stderr, "%s\n", pMsg );
            ErrorBox aBox( NULL, WB_OK | WB_DEF_OK, OUString( pMsg, strlen( pMsg ), RTL_TEXTENCODING_ASCII_US ) );
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
                std::pair< OUString, FieldUnit > aElement( aUnits.GetString(i), static_cast<FieldUnit>(aUnits.GetValue(i)) );
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
                OUString aUnit( (*pUnits)[i].first );
                aUnit = comphelper::string::remove(aUnit, ' ');
                aUnit = aUnit.toAsciiLowerCase();
                std::pair< OUString, FieldUnit > aElement( aUnit, (*pUnits)[i].second );
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

BlendFrameCache* ImplGetBlendFrameCache()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpBlendFrameCache)
        pSVData->mpBlendFrameCache= new BlendFrameCache();

    return pSVData->mpBlendFrameCache;
}

class AccessBridgeCurrentContext: public cppu::WeakImplHelper1< com::sun::star::uno::XCurrentContext >
{
public:
    AccessBridgeCurrentContext(
        const com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > &context ) :
        m_prevContext( context ) {}

    // XCurrentContext
    virtual com::sun::star::uno::Any SAL_CALL getValueByName( const OUString& Name )
        throw (com::sun::star::uno::RuntimeException);
private:
    com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > m_prevContext;
};

com::sun::star::uno::Any AccessBridgeCurrentContext::getValueByName( const OUString & Name )
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


bool ImplInitAccessBridge(bool bAllowCancel, bool &rCancelled)
{
    rCancelled = false;

    bool bErrorMessage = true;

    // Note:
    // if bAllowCancel is sal_True we were called from application startup
    //  where we will disable any Java errorboxes and show our own accessibility dialog if Java throws an exception
    // if bAllowCancel is sal_False we were called from Tools->Options
    //  where we will see Java errorboxes, se we do not show our dialogs in addition to Java's

    try
    {
        // No error messages when env var is set ..
        static const char* pEnv = getenv("SAL_ACCESSIBILITY_ENABLED" );
        if( pEnv && *pEnv )
        {
            bErrorMessage = false;
        }

        ImplSVData* pSVData = ImplGetSVData();
        if( ! pSVData->mxAccessBridge.is() )
        {
            css::uno::Reference< XComponentContext > xContext(comphelper::getProcessComponentContext());

            css::uno::Reference< XExtendedToolkit > xToolkit =
                css::uno::Reference< XExtendedToolkit >(Application::GetVCLToolkit(), UNO_QUERY);

            // Disable default java error messages on startup, because they were probably unreadable
            // for a disabled user. Use native message boxes which are accessible without java support.
            // No need to do this when activated by Tools-Options dialog ..
            if( bAllowCancel )
            {
                // customize the java-not-available-interaction-handler entry within the
                // current context when called at startup.
                com::sun::star::uno::ContextLayer layer(
                    new AccessBridgeCurrentContext( com::sun::star::uno::getCurrentContext() ) );

                pSVData->mxAccessBridge
                    = css::accessibility::AccessBridge::createWithToolkit(
                        xContext, xToolkit);
            }
            else
            {
                pSVData->mxAccessBridge
                    = css::accessibility::AccessBridge::createWithToolkit(
                        xContext, xToolkit);
            }
        }

        return true;
    }
    catch (const ::com::sun::star::java::JavaNotConfiguredException&)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && bAllowCancel && pResMgr )
        {
            OUString aTitle(ResId(SV_ACCESSERROR_JAVA_NOT_CONFIGURED, *pResMgr).toString());
            OUStringBuffer aMessage((ResId(SV_ACCESSERROR_JAVA_MSG, *pResMgr)).toString());

            aMessage.append(' ').append(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr).toString());

            int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                aTitle,
                ReplaceJavaErrorMessages(aMessage.makeStringAndClear()),
                SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, true);

            // Do not change the setting in case the user chooses to cancel
            if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                rCancelled = true;
        }

        return false;
    }
    catch (const ::com::sun::star::java::JavaVMCreationFailureException&)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && bAllowCancel && pResMgr )
        {
            OUString aTitle(ResId(SV_ACCESSERROR_FAULTY_JAVA, *pResMgr).toString());
            OUStringBuffer aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, *pResMgr).toString());

            aMessage.append(' ').append(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr).toString());

            int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                aTitle,
                ReplaceJavaErrorMessages(aMessage.makeStringAndClear()),
                SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, true);

            // Do not change the setting in case the user chooses to cancel
            if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                rCancelled = true;
        }

        return false;
    }
    catch (const ::com::sun::star::java::MissingJavaRuntimeException&)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && bAllowCancel && pResMgr )
        {
            OUString aTitle(ResId(SV_ACCESSERROR_MISSING_JAVA, *pResMgr).toString());
            OUStringBuffer aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, *pResMgr).toString());

            aMessage.append(' ').append(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr).toString());

            int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                aTitle,
                ReplaceJavaErrorMessages(aMessage.makeStringAndClear()),
                SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, true);

            // Do not change the setting in case the user chooses to cancel
            if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                rCancelled = true;
        }

        return false;
    }
    catch (const ::com::sun::star::java::JavaDisabledException&)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && bAllowCancel && pResMgr )
        {
            OUString aTitle(ResId(SV_ACCESSERROR_JAVA_DISABLED, *pResMgr).toString());
            OUStringBuffer aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, *pResMgr).toString());

            aMessage.append(' ').append(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr).toString());

            int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                aTitle,
                ReplaceJavaErrorMessages(aMessage.makeStringAndClear()),
                SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, true);

            // Do not change the setting in case the user chooses to cancel
            if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                rCancelled = true;
        }

        return false;
    }
    catch (const ::com::sun::star::uno::RuntimeException& e)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && pResMgr )
        {
            OUString aTitle;
            OUStringBuffer aMessage(ResId(SV_ACCESSERROR_BRIDGE_MSG, *pResMgr).toString());

            if( e.Message.startsWith("ClassNotFound") )
            {
                aTitle = ResId(SV_ACCESSERROR_MISSING_BRIDGE, *pResMgr).toString();
            }
            else if( e.Message.startsWith("NoSuchMethod") )
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

Window* ImplFindWindow( const SalFrame* pFrame, ::Point& rSalFramePos )
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
