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
#include <ucbhelper/content.hxx>

#include <vcl/canvastools.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>
#include <com/sun/star/uno/Reference.h>
#include <unotools/configmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include <rtl/bootstrap.hxx>
#include <svl/stritem.hxx>
#include <tools/urlobj.hxx>

#include <sfx2/app.hxx>
#include <appdata.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

#define ShellClass_SfxApplication
#include <sfxslots.hxx>

SFX_IMPL_INTERFACE(SfxApplication,SfxShell)

void SfxApplication::InitInterface_Impl()
{
    GetStaticInterface()->RegisterStatusBar(StatusBarId::GenericStatusBar);

    GetStaticInterface()->RegisterChildWindow(SID_DOCKWIN_0);
    GetStaticInterface()->RegisterChildWindow(SID_DOCKWIN_1);
    GetStaticInterface()->RegisterChildWindow(SID_DOCKWIN_2);
    GetStaticInterface()->RegisterChildWindow(SID_DOCKWIN_3);
    GetStaticInterface()->RegisterChildWindow(SID_DOCKWIN_4);
    GetStaticInterface()->RegisterChildWindow(SID_DOCKWIN_5);
    GetStaticInterface()->RegisterChildWindow(SID_DOCKWIN_6);
    GetStaticInterface()->RegisterChildWindow(SID_DOCKWIN_7);
    GetStaticInterface()->RegisterChildWindow(SID_DOCKWIN_8);
    GetStaticInterface()->RegisterChildWindow(SID_DOCKWIN_9);
}

/** Returns the running SfxProgress for the entire application or 0 if
    none is running for the entire application.

    [Cross-reference]

    <SfxProgress::GetActiveProgress(SfxViewFrame*)>
    <SfxViewFrame::GetProgress()const>
*/
SfxProgress* SfxApplication::GetProgress() const
{
    return pImpl->pProgress;
}

SfxModule* SfxApplication::GetModule_Impl()
{
    SfxModule* pModule = SfxModule::GetActiveModule();
    if ( !pModule )
        pModule = SfxModule::GetActiveModule( SfxViewFrame::GetFirst( nullptr, false ) );
    if( pModule )
        return pModule;
    else
    {
        OSL_FAIL( "No module!" );
        return nullptr;
    }
}

bool  SfxApplication::IsDowning() const { return pImpl->bDowning; }
SfxDispatcher* SfxApplication::GetAppDispatcher_Impl() { return &*pImpl->pAppDispat; }
SfxSlotPool& SfxApplication::GetAppSlotPool_Impl() const { return *pImpl->pSlotPool; }

static bool FileExists( const INetURLObject& rURL )
{
    bool bRet = false;

    if( rURL.GetProtocol() != INetProtocol::NotValid )
    {
        try
        {
            ::ucbhelper::Content  aCnt( rURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
            OUString aTitle;

            aCnt.getPropertyValue("Title") >>= aTitle;
            bRet = ( !aTitle.isEmpty() );
        }
        catch(const Exception&)
        {
            return false;
        }
    }

    return bRet;
}

bool SfxApplication::loadBrandSvg(const char *pName, BitmapEx &rBitmap, int nWidth)
{
    // Load from disk

    OUString aBaseName = "/" + OUString::createFromAscii( pName );

    OUString uri = "$BRAND_BASE_DIR/" LIBO_ETC_FOLDER + aBaseName + ".svg";
    rtl::Bootstrap::expandMacros( uri );

    INetURLObject aObj( uri );
    if ( !FileExists(aObj) )
        return false;

    VectorGraphicData aVectorGraphicData(aObj.PathToFileName(), VectorGraphicDataType::Svg);

    // transform into [0,0,width,width*aspect] std dimensions

    basegfx::B2DRange aRange(aVectorGraphicData.getRange());
    const double fAspectRatio(
        aRange.getHeight() == 0.0 ? 1.0 : aRange.getWidth()/aRange.getHeight());
    basegfx::B2DHomMatrix aTransform(
        basegfx::utils::createTranslateB2DHomMatrix(
            -aRange.getMinX(),
            -aRange.getMinY()));
    aTransform.scale(
        aRange.getWidth() == 0.0 ? 1.0 : nWidth / aRange.getWidth(),
        (aRange.getHeight() == 0.0
         ? 1.0 : nWidth / fAspectRatio / aRange.getHeight()));
    const drawinglayer::primitive2d::Primitive2DReference xTransformRef(
        new drawinglayer::primitive2d::TransformPrimitive2D(
            aTransform,
            aVectorGraphicData.getPrimitive2DSequence()));

    // UNO dance to render from drawinglayer

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
            rBitmap = vcl::unotools::bitmapExFromXBitmap(xIntBmp);
            return true;
        }
    }
    catch(const uno::Exception&)
    {
        OSL_ENSURE(false, "Got no graphic::XPrimitive2DRenderer (!)" );
    }
    return false;
}

/** loads the application logo as used in the impress slideshow pause screen */
BitmapEx SfxApplication::GetApplicationLogo(tools::Long nWidth)
{
    BitmapEx aBitmap;
    SfxApplication::loadBrandSvg("shell/about", aBitmap, nWidth);
    return aBitmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
