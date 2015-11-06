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
#include <uno/current_context.hxx>

#include "vcl/configsettings.hxx"
#include "vcl/svapp.hxx"
#include "vcl/settings.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/layout.hxx"
#include "vcl/button.hxx"
#include "vcl/dockwin.hxx"
#include "vcl/print.hxx"
#include "vcl/virdev.hxx"
#include "salinst.hxx"
#include "salframe.hxx"
#include "salgdi.hxx"
#include "svdata.hxx"
#include "window.h"
#include "salimestatus.hxx"
#include "salsys.hxx"
#include "svids.hrc"
#include "helpwin.hxx"
#include "../window/scrwnd.hxx"

#include "com/sun/star/accessibility/MSAAService.hpp"

#include "officecfg/Office/Common.hxx"

#include "vcl/opengl/OpenGLContext.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;

namespace
{
    struct private_aImplSVData :
        public rtl::Static<ImplSVData, private_aImplSVData> {};
}

ImplSVData* ImplGetSVData() {
    return &private_aImplSVData::get();
}

SalSystem* ImplGetSalSystem()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpSalSystem )
        pSVData->mpSalSystem = pSVData->mpDefInst->CreateSalSystem();
    return pSVData->mpSalSystem;
}

ImplSVData::ImplSVData()
{
    // init global instance data
    memset( this, 0, sizeof( ImplSVData ) );
    maHelpData.mbAutoHelpId = true;
    maNWFData.maMenuBarHighlightTextColor = Color( COL_TRANSPARENT );
    maNWFData.mbEnableAccel = true;
    maNWFData.mbAutoAccel = false;
}

ImplSVGDIData::~ImplSVGDIData()
{
    // FIXME: deliberately leak any remaining OutputDevice
    // until we have their pGraphics reference counted, doing
    // any disposes so late in shutdown is rather unsafe.
    memset( this, 0, sizeof( ImplSVGDIData ) );
}

void ImplDeInitSVData()
{
    ImplSVData* pSVData = ImplGetSVData();

    // delete global instance data
    if( pSVData->mpSettingsConfigItem )
        delete pSVData->mpSettingsConfigItem;

    if( pSVData->mpDockingManager )
        delete pSVData->mpDockingManager;

    if( pSVData->maCtrlData.mpFieldUnitStrings )
        delete pSVData->maCtrlData.mpFieldUnitStrings, pSVData->maCtrlData.mpFieldUnitStrings = NULL;
    if( pSVData->maCtrlData.mpCleanUnitStrings )
        delete pSVData->maCtrlData.mpCleanUnitStrings, pSVData->maCtrlData.mpCleanUnitStrings = NULL;
    if( pSVData->mpPaperNames )
        delete pSVData->mpPaperNames, pSVData->mpPaperNames = NULL;
}

vcl::Window* ImplGetDefaultWindow()
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
            SAL_INFO( "vcl", "ImplGetDefaultWindow(): No AppWindow" );
            pSVData->mpDefaultWin = VclPtr<WorkWindow>::Create( nullptr, WB_DEFAULTWIN );
            pSVData->mpDefaultWin->SetText( "VCL ImplGetDefaultWindow" );

            // Add a reference to the default context so it never gets deleted
            rtl::Reference<OpenGLContext> pContext = pSVData->mpDefaultWin->GetGraphics()->GetOpenGLContext();
            if( pContext.is() )
                pContext->acquire();
        }
        Application::GetSolarMutex().release();
    }

    return pSVData->mpDefaultWin;
}

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
            SAL_WARN("vcl", "" << pMsg << "\n");
            ScopedVclPtrInstance< MessageDialog > aBox( nullptr, OUString(pMsg, strlen(pMsg), RTL_TEXTENCODING_ASCII_US) );
            aBox->Execute();
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

#ifdef _WIN32
bool HasAtHook();
#endif

#ifdef _WIN32
bool ImplInitAccessBridge()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mxAccessBridge.is() )
    {
        css::uno::Reference< XComponentContext > xContext(comphelper::getProcessComponentContext());

        if (!HasAtHook() && !getenv("SAL_FORCE_IACCESSIBLE2"))
        {
            SAL_INFO("vcl", "Apparently no running AT -> "
                     "not enabling IAccessible2 integration");
        }
        else
        {
            try {
                 pSVData->mxAccessBridge
                     = css::accessibility::MSAAService::create(xContext);
                 SAL_INFO("vcl", "got IAccessible2 bridge");
                 return true;
             } catch (css::uno::DeploymentException & e) {
                 SAL_WARN(
                    "vcl",
                    "got no IAccessible2 bridge" << e.Message);
                 return false;
             }
        }
    }

    return true;
}
#endif

void LocaleConfigurationListener::ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 nHint )
{
    AllSettings::LocaleSettingsChanged( nHint );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
