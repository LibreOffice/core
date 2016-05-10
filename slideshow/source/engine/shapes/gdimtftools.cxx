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


#include <tools/diagnose_ex.h>
#include <gdimtftools.hxx>

#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicRenderer.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>

#include <comphelper/uno3.hxx>

#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/animate.hxx>
#include <vcl/graphic.hxx>

#include <unotools/streamwrap.hxx>

#include "tools.hxx"

using namespace ::com::sun::star;


// free support functions
// ======================

namespace slideshow
{
namespace internal
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
bool hasUnsupportedActions( const GDIMetaFile& rMtf )
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
                if( ROP_OVERPAINT ==
                    static_cast<MetaRasterOpAction*>(pCurrAct)->GetRasterOp() )
                {
                    break;
                }
                SAL_FALLTHROUGH;
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
    virtual void SAL_CALL render( const uno::Reference< graphic::XGraphic >& rGraphic ) throw (uno::RuntimeException, std::exception) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            mxGraphic = rGraphic;
        }

    /** Retrieve GDIMetaFile from renderer

        @param bForeignSource
        When true, the source of the metafile might be a
        foreign application. The metafile is checked
        against unsupported content, and, if necessary,
        returned as a pre-rendererd bitmap.
    */
    GDIMetaFile getMtf( bool bForeignSource ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Graphic aGraphic( mxGraphic );

        if( aGraphic.GetType() == GRAPHIC_BITMAP ||
            (bForeignSource &&
             hasUnsupportedActions(aGraphic.GetGDIMetaFile()) ) )
        {
            // wrap bitmap into GDIMetafile
            GDIMetaFile     aMtf;
            ::Point         aEmptyPoint;

            ::BitmapEx      aBmpEx( aGraphic.GetBitmapEx() );

            aMtf.AddAction( new MetaBmpExAction( aEmptyPoint,
                                                 aBmpEx ) );
            aMtf.SetPrefSize( aBmpEx.GetPrefSize() );
            aMtf.SetPrefMapMode( aBmpEx.GetPrefMapMode() );

            return aMtf;
        }
        else
        {
            return aGraphic.GetGDIMetaFile();
        }
    }

private:
    uno::Reference< graphic::XGraphic > mxGraphic;
};

} // anon namespace

// Quick'n'dirty way: tunnel Graphic (only works for
// in-process slideshow, of course)
bool getMetaFile( const uno::Reference< lang::XComponent >&       xSource,
                  const uno::Reference< drawing::XDrawPage >&     xContainingPage,
                  GDIMetaFile&                                    rMtf,
                  int                                             mtfLoadFlags,
                  const uno::Reference< uno::XComponentContext >& rxContext )
{
    ENSURE_OR_RETURN_FALSE( rxContext.is(),
                       "getMetaFile(): Invalid context" );

    // create dummy XGraphicRenderer, which receives the
    // generated XGraphic from the GraphicExporter

    // TODO(P3): Move creation of DummyRenderer out of the
    // loop! Either by making it static, or transforming
    // the whole thing here into a class.
    DummyRenderer*                              pRenderer( new DummyRenderer() );
    uno::Reference< graphic::XGraphicRenderer > xRenderer( pRenderer );

    // creating the graphic exporter
    uno::Reference< drawing::XGraphicExportFilter > xExporter =
        drawing::GraphicExportFilter::create(rxContext);

    uno::Sequence< beans::PropertyValue > aProps(3);
    aProps[0].Name = "FilterName";
    aProps[0].Value <<= OUString("SVM");

    aProps[1].Name = "GraphicRenderer";
    aProps[1].Value <<= xRenderer;

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
        return false;

    rMtf = pRenderer->getMtf( (mtfLoadFlags & MTF_LOAD_FOREIGN_SOURCE) != 0 );

    // pRenderer is automatically destroyed when xRenderer
    // goes out of scope

    // TODO(E3): Error handling. Exporter might have
    // generated nothing, a bitmap, threw an exception,
    // whatever.
    return true;
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
                              ::std::size_t&                o_rLoopCount,
                              CycleMode&                    o_eCycleMode,
                              const Graphic&                rGraphic )
{
    o_rFrames.clear();

    if( !rGraphic.IsAnimated() )
        return false;

    // some loop invariants
    Animation   aAnimation( rGraphic.GetAnimation() );
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
    ScopedVclPtrInstance< VirtualDevice > pVDevMask;
    pVDevMask->SetOutputSizePixel( aAnimSize );
    pVDevMask->EnableMapMode( false );

    switch( aAnimation.GetCycleMode() )
    {
        case CYCLE_NOT:
            o_rLoopCount = 1;
            o_eCycleMode = CYCLE_LOOP;
            break;

        case CYCLE_FALLBACK:
            // FALLTHROUGH intended
        case CYCLE_NORMAL:
            o_rLoopCount = aAnimation.GetLoopCount();
            o_eCycleMode = CYCLE_LOOP;
            break;

        case CYCLE_REVERS:
            // FALLTHROUGH intended
        case CYCLE_REVERS_FALLBACK:
            o_rLoopCount = aAnimation.GetLoopCount();
            o_eCycleMode = CYCLE_PINGPONGLOOP;
            break;

        default:
            ENSURE_OR_RETURN_FALSE(false,
                              "getAnimationFromGraphic(): Unexpected case" );
            break;
    }

    for( sal_uInt16 i=0, nCount=aAnimation.Count(); i<nCount; ++i )
    {
        const AnimationBitmap& rAnimBmp( aAnimation.Get(i) );
        switch(rAnimBmp.eDisposal)
        {
            case DISPOSE_NOT:
            {
                pVDev->DrawBitmapEx(rAnimBmp.aPosPix,
                                   rAnimBmp.aBmpEx);
                Bitmap aMask = rAnimBmp.aBmpEx.GetMask();

                if( aMask.IsEmpty() )
                {
                    const Rectangle aRect(aEmptyPoint,
                                          pVDevMask->GetOutputSizePixel());
                    const Wallpaper aWallpaper(COL_BLACK);
                    pVDevMask->DrawWallpaper(aRect,
                                            aWallpaper);
                }
                else
                {
                    BitmapEx aTmpMask = BitmapEx(aMask,
                                                 aMask);
                    pVDevMask->DrawBitmapEx(rAnimBmp.aPosPix,
                                           aTmpMask );
                }
                break;
            }

            case DISPOSE_BACK:
            {
                // #i70772# react on no mask
                const Bitmap aMask(rAnimBmp.aBmpEx.GetMask());
                const Bitmap aContent(rAnimBmp.aBmpEx.GetBitmap());

                pVDevMask->Erase();
                pVDev->DrawBitmap(rAnimBmp.aPosPix, aContent);

                if(aMask.IsEmpty())
                {
                    const Rectangle aRect(rAnimBmp.aPosPix, aContent.GetSizePixel());
                    pVDevMask->SetFillColor(COL_BLACK);
                    pVDevMask->SetLineColor();
                    pVDevMask->DrawRect(aRect);
                }
                else
                {
                    pVDevMask->DrawBitmap(rAnimBmp.aPosPix, aMask);
                }
                break;
            }

            case DISPOSE_FULL:
            {
                pVDev->DrawBitmapEx(rAnimBmp.aPosPix,
                                   rAnimBmp.aBmpEx);
                break;
            }

            case DISPOSE_PREVIOUS :
            {
                pVDev->DrawBitmapEx(rAnimBmp.aPosPix,
                                   rAnimBmp.aBmpEx);
                pVDevMask->DrawBitmap(rAnimBmp.aPosPix,
                                     rAnimBmp.aBmpEx.GetMask());
                break;
            }
        }

        // extract current aVDev content into a new animation
        // frame
        GDIMetaFileSharedPtr pMtf( new GDIMetaFile() );
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
        sal_Int32 nWaitTime100thSeconds( rAnimBmp.nWait );
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

        o_rFrames.push_back( MtfAnimationFrame( pMtf,
                                                nWaitTime100thSeconds / 100.0 ) );
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
            if( pAct->GetComment().matchIgnoreAsciiCase( OString("XTEXT") ) )
            {
                if (pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_SCROLLRECT"))
                {
                    o_rScrollRect = vcl::unotools::b2DRectangleFromRectangle(
                        *reinterpret_cast<Rectangle const *>(
                            pAct->GetData() ) );

                    bScrollRectSet = true;
                }
                else if (pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_PAINTRECT") )
                {
                    o_rPaintRect = vcl::unotools::b2DRectangleFromRectangle(
                        *reinterpret_cast<Rectangle const *>(
                            pAct->GetData() ) );

                    bPaintRectSet = true;
                }
            }
        }
    }

    return bScrollRectSet && bPaintRectSet;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
