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


#include <sal/log.hxx>
#include "gdimtftools.hxx"

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicRenderer.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>

#include <comphelper/fileformat.h>

#include <vcl/canvastools.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/animate/Animation.hxx>
#include <vcl/graph.hxx>

#include <tools.hxx>

using namespace ::com::sun::star;


// free support functions
// ======================

namespace slideshow::internal
{
// TODO(E2): Detect the case when svx/drawing layer is not
// in-process, or even not on the same machine, and
// fallback to metafile streaming!

// For fixing #i48102#, have to be a _lot_ more selective
// on which metafiles to convert to bitmaps. The problem
// here is that we _always_ get the shape content as a
// metafile, even if we have a bitmap graphic shape. Thus,
// calling GetBitmapEx on such a Graphic (see below) will
// result in one poorly scaled bitmap into another,
// somewhat arbitrarily sized bitmap.
static bool hasUnsupportedActions( const GDIMetaFile& rMtf )
{
    // search metafile for RasterOp action
    MetaAction* pCurrAct;

    // TODO(Q3): avoid const-cast
    for( pCurrAct = const_cast<GDIMetaFile&>(rMtf).FirstAction();
         pCurrAct;
         pCurrAct = const_cast<GDIMetaFile&>(rMtf).NextAction() )
    {
        switch( pCurrAct->GetType() )
        {
            case MetaActionType::RASTEROP:
                // overpaint is okay - that's the default, anyway
                if( RasterOp::OverPaint ==
                    static_cast<MetaRasterOpAction*>(pCurrAct)->GetRasterOp() )
                {
                    break;
                }
                [[fallthrough]];
            case MetaActionType::MOVECLIPREGION:
            case MetaActionType::REFPOINT:
            case MetaActionType::WALLPAPER:
                return true; // at least one unsupported
                             // action encountered
            default: break;
        }
    }

    return false; // no unsupported action found
}

namespace {

typedef ::cppu::WeakComponentImplHelper< graphic::XGraphicRenderer > DummyRenderer_Base;

class DummyRenderer: public cppu::BaseMutex, public DummyRenderer_Base
{
public:
    DummyRenderer() :
        DummyRenderer_Base( m_aMutex ),
        mxGraphic()
        {
        }

    //---  XGraphicRenderer  -----------------------------------
    virtual void SAL_CALL render( const uno::Reference< graphic::XGraphic >& rGraphic ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            mxGraphic = rGraphic;
        }

    /** Retrieve GDIMetaFile from renderer

        @param bForeignSource
        When true, the source of the metafile might be a
        foreign application. The metafile is checked
        against unsupported content, and, if necessary,
        returned as a pre-rendered bitmap.
    */
    GDIMetaFileSharedPtr getMtf( bool bForeignSource ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Graphic aGraphic( mxGraphic );

        if( aGraphic.GetType() == GraphicType::Bitmap ||
            (bForeignSource &&
             hasUnsupportedActions(aGraphic.GetGDIMetaFile()) ) )
        {
            // wrap bitmap into GDIMetafile
            GDIMetaFileSharedPtr xMtf = std::make_shared<GDIMetaFile>();

            ::BitmapEx      aBmpEx( aGraphic.GetBitmapEx() );

            xMtf->AddAction( new MetaBmpExAction( Point(),
                                                 aBmpEx ) );
            xMtf->SetPrefSize( aBmpEx.GetPrefSize() );
            xMtf->SetPrefMapMode( aBmpEx.GetPrefMapMode() );

            return xMtf;
        }
        return std::make_shared<GDIMetaFile>(aGraphic.GetGDIMetaFile());
    }

private:
    uno::Reference< graphic::XGraphic > mxGraphic;
};

} // anon namespace

// Quick'n'dirty way: tunnel Graphic (only works for
// in-process slideshow, of course)
GDIMetaFileSharedPtr getMetaFile( const uno::Reference< lang::XComponent >&       xSource,
                                  const uno::Reference< drawing::XDrawPage >&     xContainingPage,
                                  int                                             mtfLoadFlags,
                                  const uno::Reference< uno::XComponentContext >& rxContext )
{
    if (!rxContext.is())
    {
        SAL_WARN("slideshow.opengl", "getMetaFile(): Invalid context" );
        return GDIMetaFileSharedPtr();
    }

    // create dummy XGraphicRenderer, which receives the
    // generated XGraphic from the GraphicExporter

    // TODO(P3): Move creation of DummyRenderer out of the
    // loop! Either by making it static, or transforming
    // the whole thing here into a class.
    rtl::Reference<DummyRenderer> xRenderer( new DummyRenderer() );

    // creating the graphic exporter
    uno::Reference< drawing::XGraphicExportFilter > xExporter =
        drawing::GraphicExportFilter::create(rxContext);

    uno::Sequence< beans::PropertyValue > aProps(3);
    aProps[0].Name = "FilterName";
    aProps[0].Value <<= OUString("SVM");

    aProps[1].Name = "GraphicRenderer";
    aProps[1].Value <<= uno::Reference< graphic::XGraphicRenderer >(xRenderer);

    uno::Sequence< beans::PropertyValue > aFilterData(4);
    aFilterData[0].Name = "ScrollText";
    aFilterData[0].Value <<= ((mtfLoadFlags & MTF_LOAD_SCROLL_TEXT_MTF) != 0);

    aFilterData[1].Name = "ExportOnlyBackground";
    aFilterData[1].Value <<= ((mtfLoadFlags & MTF_LOAD_BACKGROUND_ONLY) != 0);

    aFilterData[2].Name = "Version";
    aFilterData[2].Value <<= static_cast<sal_Int32>( SOFFICE_FILEFORMAT_50 );

    aFilterData[3].Name = "CurrentPage";
    aFilterData[3].Value <<= uno::Reference< uno::XInterface >( xContainingPage,
                                                                uno::UNO_QUERY_THROW );

    aProps[2].Name = "FilterData";
    aProps[2].Value <<= aFilterData;

    xExporter->setSourceDocument( xSource );
    if( !xExporter->filter( aProps ) )
        return GDIMetaFileSharedPtr();

    GDIMetaFileSharedPtr xMtf = xRenderer->getMtf( (mtfLoadFlags & MTF_LOAD_FOREIGN_SOURCE) != 0 );

    // pRenderer is automatically destroyed when xRenderer
    // goes out of scope

    // TODO(E3): Error handling. Exporter might have
    // generated nothing, a bitmap, threw an exception,
    // whatever.
    return xMtf;
}

sal_Int32 getNextActionOffset( MetaAction * pCurrAct )
{
    // Special handling for actions that represent
    // more than one indexable action
    // ===========================================

    switch (pCurrAct->GetType()) {
    case MetaActionType::TEXT: {
        MetaTextAction * pAct = static_cast<MetaTextAction *>(pCurrAct);
        sal_Int32 nLen = std::min(pAct->GetLen(), pAct->GetText().getLength() - pAct->GetIndex());
        return nLen;
    }
    case MetaActionType::TEXTARRAY: {
        MetaTextArrayAction * pAct =
            static_cast<MetaTextArrayAction *>(pCurrAct);
        sal_Int32 nLen = std::min(pAct->GetLen(), pAct->GetText().getLength() - pAct->GetIndex());
        return nLen;
    }
    case MetaActionType::STRETCHTEXT: {
        MetaStretchTextAction * pAct =
            static_cast<MetaStretchTextAction *>(pCurrAct);
        sal_Int32 nLen = std::min(pAct->GetLen(), pAct->GetText().getLength() - pAct->GetIndex());
        return nLen;
    }
    case MetaActionType::FLOATTRANSPARENT: {
        MetaFloatTransparentAction * pAct =
            static_cast<MetaFloatTransparentAction*>(pCurrAct);
        // TODO(F2): Recurse into action metafile
        // (though this is currently not used from the
        // DrawingLayer - shape transparency gradients
        // don't affect shape text)
        return pAct->GetGDIMetaFile().GetActionSize();
    }
    default:
        return 1;
    }
}

bool getAnimationFromGraphic( VectorOfMtfAnimationFrames&   o_rFrames,
                              sal_uInt32&                   o_rLoopCount,
                              const Graphic&                rGraphic )
{
    o_rFrames.clear();

    if( !rGraphic.IsAnimated() )
        return false;

    // some loop invariants
    ::Animation   aAnimation( rGraphic.GetAnimation() );
    const Point aEmptyPoint;
    const Size  aAnimSize( aAnimation.GetDisplaySizePixel() );

    // setup VDev, into which all bitmaps are painted (want to
    // normalize animations to n bitmaps of same size. An Animation,
    // though, can contain bitmaps of varying sizes and different
    // update modes)
    ScopedVclPtrInstance< VirtualDevice > pVDev;
    pVDev->SetOutputSizePixel( aAnimSize );
    pVDev->EnableMapMode( false );

    // setup mask VDev (alpha VDev is currently rather slow)
    ScopedVclPtrInstance<VirtualDevice> pVDevMask(DeviceFormat::BITMASK);
    pVDevMask->SetOutputSizePixel( aAnimSize );
    pVDevMask->EnableMapMode( false );

    o_rLoopCount = aAnimation.GetLoopCount();

    for( sal_uInt16 i=0, nCount=aAnimation.Count(); i<nCount; ++i )
    {
        const AnimationBitmap& rAnimationBitmap( aAnimation.Get(i) );
        switch(rAnimationBitmap.meDisposal)
        {
            case Disposal::Not:
            {
                pVDev->DrawBitmapEx(rAnimationBitmap.maPositionPixel,
                                    rAnimationBitmap.maBitmapEx);
                Bitmap aMask = rAnimationBitmap.maBitmapEx.GetMask();

                if( aMask.IsEmpty() )
                {
                    const tools::Rectangle aRect(aEmptyPoint,
                                          pVDevMask->GetOutputSizePixel());
                    const Wallpaper aWallpaper(COL_BLACK);
                    pVDevMask->DrawWallpaper(aRect,
                                            aWallpaper);
                }
                else
                {
                    BitmapEx aTmpMask(aMask, aMask);
                    pVDevMask->DrawBitmapEx(rAnimationBitmap.maPositionPixel,
                                            aTmpMask );
                }
                break;
            }

            case Disposal::Back:
            {
                // #i70772# react on no mask
                const Bitmap aMask(rAnimationBitmap.maBitmapEx.GetMask());
                const Bitmap & rContent(rAnimationBitmap.maBitmapEx.GetBitmap());

                pVDevMask->Erase();
                pVDev->DrawBitmap(rAnimationBitmap.maPositionPixel, rContent);

                if(aMask.IsEmpty())
                {
                    const tools::Rectangle aRect(rAnimationBitmap.maPositionPixel, rContent.GetSizePixel());
                    pVDevMask->SetFillColor( COL_BLACK);
                    pVDevMask->SetLineColor();
                    pVDevMask->DrawRect(aRect);
                }
                else
                {
                    pVDevMask->DrawBitmap(rAnimationBitmap.maPositionPixel, aMask);
                }
                break;
            }

            case Disposal::Previous :
            {
                pVDev->DrawBitmapEx(rAnimationBitmap.maPositionPixel,
                                    rAnimationBitmap.maBitmapEx);
                pVDevMask->DrawBitmap(rAnimationBitmap.maPositionPixel,
                                      rAnimationBitmap.maBitmapEx.GetMask());
                break;
            }
        }

        // extract current aVDev content into a new animation
        // frame
        GDIMetaFileSharedPtr pMtf = std::make_shared<GDIMetaFile>();
        pMtf->AddAction(
            new MetaBmpExAction( aEmptyPoint,
                                 BitmapEx(
                                     pVDev->GetBitmap(
                                         aEmptyPoint,
                                         aAnimSize ),
                                     pVDevMask->GetBitmap(
                                         aEmptyPoint,
                                         aAnimSize ))));

        // setup mtf dimensions and pref map mode (for
        // simplicity, keep it all in pixel. the metafile
        // renderer scales it down to (1, 1) box anyway)
        pMtf->SetPrefMapMode( MapMode() );
        pMtf->SetPrefSize( aAnimSize );

        // Take care of special value for MultiPage TIFFs. ATM these shall just
        // show their first page for _quite_ some time.
        sal_Int32 nWaitTime100thSeconds(rAnimationBitmap.mnWait);
        if( ANIMATION_TIMEOUT_ON_CLICK == nWaitTime100thSeconds )
        {
            // ATM the huge value would block the timer, so use a long
            // time to show first page (whole day)
            nWaitTime100thSeconds = 100 * 60 * 60 * 24;
        }

        // There are animated GIFs with no WaitTime set. Take 0.1 sec, the
        // same duration that is used by the edit view.
        if( nWaitTime100thSeconds == 0 )
            nWaitTime100thSeconds = 10;

        o_rFrames.emplace_back( pMtf, nWaitTime100thSeconds / 100.0 );
    }

    return !o_rFrames.empty();
}

bool getRectanglesFromScrollMtf( ::basegfx::B2DRectangle&       o_rScrollRect,
                                 ::basegfx::B2DRectangle&       o_rPaintRect,
                                 const GDIMetaFileSharedPtr&    rMtf )
{
    // extract bounds: scroll rect, paint rect
    bool bScrollRectSet(false);
    bool bPaintRectSet(false);

    for ( MetaAction * pCurrAct = rMtf->FirstAction();
          pCurrAct != nullptr; pCurrAct = rMtf->NextAction() )
    {
        if (pCurrAct->GetType() == MetaActionType::COMMENT)
        {
            MetaCommentAction * pAct =
                static_cast<MetaCommentAction *>(pCurrAct);
            // skip comment if not a special XTEXT... comment
            if( pAct->GetComment().matchIgnoreAsciiCase( "XTEXT" ) )
            {
                if (pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_SCROLLRECT"))
                {
                    o_rScrollRect = vcl::unotools::b2DRectangleFromRectangle(
                                        *reinterpret_cast<tools::Rectangle const *>(
                                            pAct->GetData() ));

                    bScrollRectSet = true;
                }
                else if (pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_PAINTRECT") )
                {
                    o_rPaintRect = vcl::unotools::b2DRectangleFromRectangle(
                                        *reinterpret_cast<tools::Rectangle const *>(
                                            pAct->GetData() ));

                    bPaintRectSet = true;
                }
            }
        }
    }

    return bScrollRectSet && bPaintRectSet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
