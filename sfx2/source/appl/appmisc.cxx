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

#include <vcl/canvastools.hxx>
#include <vcl/status.hxx>
#include <vcl/msgbox.hxx>
#include <svl/whiter.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <vcl/graphicfilter.hxx>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/uno/Reference.h>
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
#include <osl/process.h>
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
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

//===================================================================

#define SfxApplication
#include "sfxslots.hxx"

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

SfxModule* SfxApplication::GetModule_Impl()
{
    SfxModule* pModule = SfxModule::GetActiveModule();
    if ( !pModule )
        pModule = SfxModule::GetActiveModule( SfxViewFrame::GetFirst( 0, sal_False ) );
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

sal_Bool  SfxApplication::IsDowning() const { return pAppData_Impl->bDowning; }
SfxDispatcher* SfxApplication::GetAppDispatcher_Impl() { return pAppData_Impl->pAppDispat; }
SfxSlotPool& SfxApplication::GetAppSlotPool_Impl() const { return *pAppData_Impl->pSlotPool; }

bool SfxApplication::loadBrandSvg(const char *pName, BitmapEx &rBitmap, int nWidth)
{
    // Load from disk
    // ---------------------------------------------------------------------
    OUString aBaseName = ( OUString("/") +
                                OUString::createFromAscii( pName ) );
    OUString aSvg( ".svg" );

    rtl_Locale *pLoc = NULL;
    osl_getProcessLocale (&pLoc);
    LanguageTag aLanguageTag( *pLoc);

    OUString uri = OUString::createFromAscii( "$BRAND_BASE_DIR/" LIBO_ETC_FOLDER ) + aBaseName+aSvg;
    rtl::Bootstrap::expandMacros( uri );
    INetURLObject aObj( uri );
    SvgData aSvgData(aObj.PathToFileName());

    // transform into [0,0,width,width*aspect] std dimensions
    // ---------------------------------------------------------------------
    basegfx::B2DRange aRange(aSvgData.getRange());
    const double fAspectRatio(aRange.getWidth()/aRange.getHeight());
    basegfx::B2DHomMatrix aTransform(
        basegfx::tools::createTranslateB2DHomMatrix(
            -aRange.getMinX(),
            -aRange.getMinY()));
    aTransform.scale(
        nWidth / aRange.getWidth(),
        nWidth / fAspectRatio / aRange.getHeight());
    const drawinglayer::primitive2d::Primitive2DReference xTransformRef(
        new drawinglayer::primitive2d::TransformPrimitive2D(
            aTransform,
            aSvgData.getPrimitive2DSequence()));

    // UNO dance to render from drawinglayer
    // ---------------------------------------------------------------------
    uno::Reference< uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());

    try
    {
        const uno::Reference< graphic::XPrimitive2DRenderer > xPrimitive2DRenderer =
            graphic::Primitive2DTools::create( xContext );

        // cancel out rasterize's mm2pixel conversion
        // see fFactor100th_mmToInch in
        // drawinglayer/source/drawinglayeruno/xprimitive2drenderer.cxx
        const double fFakeDPI=2.54 * 1000.0;

        geometry::RealRectangle2D aRealRect(
            0, 0,
            nWidth, nWidth / fAspectRatio);

        const uno::Reference< rendering::XBitmap > xBitmap(
            xPrimitive2DRenderer->rasterize(
                drawinglayer::primitive2d::Primitive2DSequence(&xTransformRef, 1),
                uno::Sequence< beans::PropertyValue >(),
                fFakeDPI,
                fFakeDPI,
                aRealRect,
                500000));

        if(xBitmap.is())
        {
            const uno::Reference< rendering::XIntegerReadOnlyBitmap> xIntBmp(xBitmap, uno::UNO_QUERY_THROW);

            if(xIntBmp.is())
            {
                rBitmap = vcl::unotools::bitmapExFromXBitmap(xIntBmp);
                return true;
            }
        }
    }
    catch(const uno::Exception&)
    {
        OSL_ENSURE(sal_False, "Got no graphic::XPrimitive2DRenderer (!)" );
    }
    return false;
}

/** loads the application logo as used in the impress slideshow pause screen */
BitmapEx SfxApplication::GetApplicationLogo(long nWidth)
{
    BitmapEx aBitmap;
    SfxApplication::loadBrandSvg("flat_logo", aBitmap, nWidth);
    Application::LoadBrandBitmap ("about", aBitmap);
    return aBitmap;
}

SfxChildWinFactArr_Impl::~SfxChildWinFactArr_Impl()
{
    for( const_iterator it = begin(); it != end(); ++it )
        delete *it;
}

SfxMenuCtrlFactArr_Impl::~SfxMenuCtrlFactArr_Impl()
{
    for( const_iterator it = begin(); it != end(); ++it )
        delete *it;
}

SfxStbCtrlFactArr_Impl::~SfxStbCtrlFactArr_Impl()
{
    for( const_iterator it = begin(); it != end(); ++it )
        delete *it;
}

SfxTbxCtrlFactArr_Impl::~SfxTbxCtrlFactArr_Impl()
{
    for( const_iterator it = begin(); it != end(); ++it )
        delete *it;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
