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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <vcl/status.hxx>
#include <vcl/msgbox.hxx>
#include <svl/whiter.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svtools/filter.hxx>
#include <unotools/internaloptions.hxx>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/uno/Reference.h>
#include <tools/config.hxx>
#include <tools/rcid.h>
#include <osl/mutex.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/frame/XDesktop.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <framework/menuconfiguration.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/moduleoptions.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>

#include "sfx2/sfxresid.hxx"
#include <sfx2/app.hxx>
#include "appdata.hxx"
#include "arrdecl.hxx"
#include <sfx2/tbxctrl.hxx>
#include "sfx2/stbitem.hxx"
#include <sfx2/mnuitem.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "workwin.hxx"
#include "intro.hxx"
#include <sfx2/fcontnr.hxx>
#include "sfxlocal.hrc"
#include <sfx2/sfx.hrc>
#include "app.hrc"
#include <sfx2/templdlg.hxx>
#include <sfx2/module.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/viewfrm.hxx>
#include "openflag.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/objface.hxx>
#include "helper.hxx"   // SfxContentHelper::Kill()

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

//===================================================================

SV_IMPL_PTRARR( SfxTbxCtrlFactArr_Impl, SfxTbxCtrlFactory* );
SV_IMPL_PTRARR( SfxStbCtrlFactArr_Impl, SfxStbCtrlFactory* );
SV_IMPL_PTRARR( SfxMenuCtrlFactArr_Impl, SfxMenuCtrlFactory* );
SV_IMPL_PTRARR( SfxChildWinFactArr_Impl, SfxChildWinFactory* );
SV_IMPL_PTRARR( SfxModuleArr_Impl, SfxModule* );

//===================================================================

#define SfxApplication
#include "sfxslots.hxx"

class SfxSpecialConfigError_Impl
{
    String aError;

public:

    SfxSpecialConfigError_Impl( const String& rStr );
    DECL_LINK( TimerHdl, Timer*);
};


SfxSpecialConfigError_Impl::SfxSpecialConfigError_Impl( const String& rStr ) :
    aError( rStr )
{
    Timer *pTimer = new Timer;
    pTimer->SetTimeoutHdl( LINK(this, SfxSpecialConfigError_Impl, TimerHdl) );
    pTimer->SetTimeout( 0 );
    pTimer->Start();
}

IMPL_LINK( SfxSpecialConfigError_Impl, TimerHdl, Timer*, pTimer )
{
    delete pTimer;
    ErrorBox( 0, WinBits( WB_OK ) , aError ).Execute();
    delete this;
    SFX_APP()->GetAppDispatcher_Impl()->Execute( SID_QUITAPP );
    return 0L;
}

//====================================================================

#define SFX_ITEMTYPE_STATBAR             4

SFX_IMPL_INTERFACE(SfxApplication,SfxShell,SfxResId(RID_DESKTOP))
{
    SFX_STATUSBAR_REGISTRATION(SfxResId(SFX_ITEMTYPE_STATBAR));
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_0);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_1);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_2);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_3);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_4);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_5);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_6);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_7);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_8);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_9);
}

//--------------------------------------------------------------------
SfxProgress* SfxApplication::GetProgress() const

/*  [Description]

    Returns the running SfxProgress for the entire application or 0 if
    none is running for the entire application.

    [Cross-reference]

    <SfxProgress::GetActiveProgress(SfxViewFrame*)>
    <SfxViewFrame::GetProgress()const>
*/

{
    return pAppData_Impl->pProgress;
}

//------------------------------------------------------------------------

SvUShorts* SfxApplication::GetDisabledSlotList_Impl()
{
    sal_Bool bError = sal_False;
    SvUShorts* pList = pAppData_Impl->pDisabledSlotList;
    if ( !pList )
    {
        // Is there a slot file?
        INetURLObject aUserObj( SvtPathOptions().GetUserConfigPath() );
        aUserObj.insertName( DEFINE_CONST_UNICODE( "slots.cfg" ) );
        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aUserObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READ );
        if ( !pStream || pStream->GetError() == ERRCODE_IO_NOTEXISTS )
        {
            delete pStream;
            INetURLObject aObj( SvtPathOptions().GetConfigPath() );
            aObj.insertName( DEFINE_CONST_UNICODE( "slots.cfg" ) );
            pStream = ::utl::UcbStreamHelper::CreateStream( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READ );
        }

        sal_Bool bSlotsEnabled = SvtInternalOptions().SlotCFGEnabled();
        sal_Bool bSlots = ( pStream && !pStream->GetError() );
        if( bSlots && bSlotsEnabled )
        {
            // Read Slot file
            String aTitle;
            pStream->ReadByteString(aTitle);
            if ( aTitle.CompareToAscii("SfxSlotFile" ) == COMPARE_EQUAL )
            {
                sal_uInt16 nCount;
                (*pStream) >> nCount;
                pList = pAppData_Impl->pDisabledSlotList =
                        new SvUShorts( nCount < 255 ? (sal_Int8) nCount : 255, 255 );

                sal_uInt16 nSlot;
                for ( sal_uInt16 n=0; n<nCount; n++ )
                {
                    (*pStream) >> nSlot;
                    pList->Insert( nSlot, n );
                }

                pStream->ReadByteString(aTitle);
                if ( aTitle.CompareToAscii("END" ) != COMPARE_EQUAL || pStream->GetError() )
                {
                    // Read failed
                    DELETEZ( pList );
                    bError = sal_True;
                }
            }
            else
            {
                // Stream detection failure
                bError = sal_True;
            }
        }
        else if ( bSlots != bSlotsEnabled )
        {
            // If no slot list entry, then no slot file shall exist
            bError = sal_True;
        }

        delete pStream;
    }
    else if ( pList == (SvUShorts*) -1L )
    {
        return NULL;
    }

    if ( !pList )
        pAppData_Impl->pDisabledSlotList = (SvUShorts*) -1L;

    if ( bError )
    {
        // If an entry slot is present, but no or faulty slot file, or a slot
        // file, but no slot entry, then this is considered to be a
        // misconfiguration
        new SfxSpecialConfigError_Impl( String( SfxResId( RID_SPECIALCONFIG_ERROR ) ) );
    }

       return pList;
}


SfxModule* SfxApplication::GetModule_Impl()
{
    SfxModule* pModule = SfxModule::GetActiveModule();
    if ( !pModule )
        pModule = SfxModule::GetActiveModule( SfxViewFrame::GetFirst( sal_False ) );
    if( pModule )
        return pModule;
    else
    {
        OSL_FAIL( "No module!" );
        return NULL;
    }
}

ISfxTemplateCommon* SfxApplication::GetCurrentTemplateCommon( SfxBindings& rBindings )
{
    if( pAppData_Impl->pTemplateCommon )
        return pAppData_Impl->pTemplateCommon;
    SfxChildWindow *pChild = rBindings.GetWorkWindow_Impl()->GetChildWindow_Impl(
        SfxTemplateDialogWrapper::GetChildWindowId() );
    if ( pChild )
        return ((SfxTemplateDialog*) pChild->GetWindow())->GetISfxTemplateCommon();
    return 0;
}

SfxResourceManager& SfxApplication::GetResourceManager() const { return *pAppData_Impl->pResMgr; }
sal_Bool  SfxApplication::IsDowning() const { return pAppData_Impl->bDowning; }
SfxDispatcher* SfxApplication::GetAppDispatcher_Impl() { return pAppData_Impl->pAppDispat; }
SfxSlotPool& SfxApplication::GetAppSlotPool_Impl() const { return *pAppData_Impl->pSlotPool; }

static bool impl_loadBitmap(
    const rtl::OUString &rPath, const rtl::OUString &rBmpFileName,
    Image &rLogo )
{
    rtl::OUString uri( rPath );
    rtl::Bootstrap::expandMacros( uri );
    INetURLObject aObj( uri );
    aObj.insertName( rBmpFileName );
    SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );
    if ( !aStrm.GetError() )
    {
        // Use graphic class to also support more graphic formats (bmp,png,...)
        Graphic aGraphic;

        GraphicFilter* pGF = GraphicFilter::GetGraphicFilter();
        pGF->ImportGraphic( aGraphic, String(), aStrm, GRFILTER_FORMAT_DONTKNOW );

        // Default case, we load the intro bitmap from a seperate file
        // (e.g. staroffice_intro.bmp or starsuite_intro.bmp)
        BitmapEx aBmp = aGraphic.GetBitmapEx();
        rLogo = Image( aBmp );
        return true;
    }
    return false;
}

/** loads the application logo as used in the about dialog and impress slideshow pause screen */
Image SfxApplication::GetApplicationLogo()
{
    Image aAppLogo;

    rtl::OUString aAbouts;
    bool bLoaded = false;
    sal_Int32 nIndex = 0;
    do
    {
        bLoaded = impl_loadBitmap(
            rtl::OUString::createFromAscii( "$BRAND_BASE_DIR/program" ),
            aAbouts.getToken( 0, ',', nIndex ), aAppLogo );
    }
    while ( !bLoaded && ( nIndex >= 0 ) );

    // fallback to "about.bmp"
    if ( !bLoaded )
    {
        bLoaded = impl_loadBitmap(
            rtl::OUString::createFromAscii( "$BRAND_BASE_DIR/program/edition" ),
            rtl::OUString::createFromAscii( "about.png" ), aAppLogo );
        if ( !bLoaded )
            bLoaded = impl_loadBitmap(
                rtl::OUString::createFromAscii( "$BRAND_BASE_DIR/program/edition" ),
                rtl::OUString::createFromAscii( "about.bmp" ), aAppLogo );
    }

    if ( !bLoaded )
    {
        bLoaded = impl_loadBitmap(
            rtl::OUString::createFromAscii( "$BRAND_BASE_DIR/program" ),
            rtl::OUString::createFromAscii( "about.png" ), aAppLogo );
        if ( !bLoaded )
            bLoaded = impl_loadBitmap(
                rtl::OUString::createFromAscii( "$BRAND_BASE_DIR/program" ),
                rtl::OUString::createFromAscii( "about.bmp" ), aAppLogo );
    }

    return aAppLogo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
