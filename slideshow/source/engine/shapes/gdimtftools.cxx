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
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <gdimtftools.hxx>

#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicRenderer.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>

#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>

#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/animate.hxx>
#include <vcl/graph.hxx>

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
            case META_RASTEROP_ACTION:
                // overpaint is okay - that's the default, anyway
                if( ROP_OVERPAINT ==
                    static_cast<MetaRasterOpAction*>(pCurrAct)->GetRasterOp() )
                {
                    break;
                }
                // FALLTHROUGH intended
            case META_MOVECLIPREGION_ACTION:
                // FALLTHROUGH intended
            case META_REFPOINT_ACTION:
                // FALLTHROUGH intended
            case META_WALLPAPER_ACTION:
                return true; // at least one unsupported
                             // action encountered
        }
    }

    return false; // no unsupported action found
}

namespace {

typedef ::cppu::WeakComponentImplHelper1< graphic::XGraphicRenderer > DummyRenderer_Base;

class DummyRenderer :
        public DummyRenderer_Base,
        public cppu::BaseMutex
{
public:
    DummyRenderer() :
        DummyRenderer_Base( m_aMutex ),
        mxGraphic()
        {
        }

    //---  XGraphicRenderer  -----------------------------------
    virtual void SAL_CALL render( const uno::Reference< graphic::XGraphic >& rGraphic ) throw (uno::RuntimeException)
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

    // -> stuff that into UnoGraphicExporter.
    uno::Reference<lang::XMultiComponentFactory> xFactory(
        rxContext->getServiceManager() );

    OSL_ENSURE( xFactory.is(), "### no UNO?!" );
    if( !xFactory.is() )
        return false;

    // creating the graphic exporter
    uno::Reference< document::XExporter > xExporter(
        xFactory->createInstanceWithContext(
            OUSTR("com.sun.star.drawing.GraphicExportFilter"),
            rxContext),
        uno::UNO_QUERY );
    uno::Reference< document::XFilter > xFilter( xExporter, uno::UNO_QUERY );

    OSL_ENSURE( xExporter.is() && xFilter.is(), "### no graphic exporter?!" );
    if( !xExporter.is() || !xFilter.is() )
        return false;

    uno::Sequence< beans::PropertyValue > aProps(3);
    aProps[0].Name = OUSTR("FilterName");
    aProps[0].Value <<= OUSTR("SVM");

    aProps[1].Name = OUSTR("GraphicRenderer");
    aProps[1].Value <<= xRenderer;

    uno::Sequence< beans::PropertyValue > aFilterData(5);
    aFilterData[0].Name = OUSTR("VerboseComments");
    aFilterData[0].Value <<= ((mtfLoadFlags & MTF_LOAD_VERBOSE_COMMENTS) != 0);

    aFilterData[1].Name = OUSTR("ScrollText");
    aFilterData[1].Value <<= ((mtfLoadFlags & MTF_LOAD_SCROLL_TEXT_MTF) != 0);

    aFilterData[2].Name = OUSTR("ExportOnlyBackground");
    aFilterData[2].Value <<= ((mtfLoadFlags & MTF_LOAD_BACKGROUND_ONLY) != 0);

    aFilterData[3].Name = OUSTR("Version");
    aFilterData[3].Value <<= static_cast<sal_Int32>( SOFFICE_FILEFORMAT_50 );

    aFilterData[4].Name = OUSTR("CurrentPage");
    aFilterData[4].Value <<= uno::Reference< uno::XInterface >( xContainingPage,
                                                                uno::UNO_QUERY_THROW );

    aProps[2].Name = OUSTR("FilterData");
    aProps[2].Value <<= aFilterData;

    xExporter->setSourceDocument( xSource );
    if( !xFilter->filter( aProps ) )
        return false;

    rMtf = pRenderer->getMtf( (mtfLoadFlags & MTF_LOAD_FOREIGN_SOURCE) != 0 );

    // pRenderer is automatically destroyed when xRenderer
    // goes out of scope

    // TODO(E3): Error handling. Exporter might have
    // generated nothing, a bitmap, threw an exception,
    // whatever.
    return true;
}

void removeTextActions( GDIMetaFile& rMtf )
{
    // search metafile for text output
    MetaAction* pCurrAct;

    int nActionIndex(0);
    pCurrAct = rMtf.FirstAction();
    while( pCurrAct )
    {
        switch( pCurrAct->GetType() )
        {
        case META_TEXTCOLOR_ACTION:
        case META_TEXTFILLCOLOR_ACTION:
        case META_TEXTLINECOLOR_ACTION:
        case META_TEXTALIGN_ACTION:
        case META_FONT_ACTION:
        case META_LAYOUTMODE_ACTION:
        case META_TEXT_ACTION:
        case META_TEXTARRAY_ACTION:
        case META_TEXTRECT_ACTION:
        case META_STRETCHTEXT_ACTION:
        case META_TEXTLINE_ACTION:
        {
            // remove every text-related actions
            pCurrAct = rMtf.NextAction();

            rMtf.RemoveAction( nActionIndex );
            break;
        }

        default:
            pCurrAct = rMtf.NextAction();
            ++nActionIndex;
            break;
        }
    }
}

sal_Int32 getNextActionOffset( MetaAction * pCurrAct )
{
    // Special handling for actions that represent
    // more than one indexable action
    // ===========================================

    switch (pCurrAct->GetType()) {
    case META_TEXT_ACTION: {
        MetaTextAction * pAct = static_cast<MetaTextAction *>(pCurrAct);
        return (pAct->GetLen() == (sal_uInt16)STRING_LEN
                ? pAct->GetText().Len() - pAct->GetIndex() : pAct->GetLen());
    }
    case META_TEXTARRAY_ACTION: {
        MetaTextArrayAction * pAct =
            static_cast<MetaTextArrayAction *>(pCurrAct);
        return (pAct->GetLen() == (sal_uInt16)STRING_LEN
                ? pAct->GetText().Len() - pAct->GetIndex() : pAct->GetLen());
    }
    case META_STRETCHTEXT_ACTION: {
        MetaStretchTextAction * pAct =
            static_cast<MetaStretchTextAction *>(pCurrAct);
        return (pAct->GetLen() == (sal_uInt16)STRING_LEN
                ? pAct->GetText().Len() - pAct->GetIndex() : pAct->GetLen());
    }
    case META_FLOATTRANSPARENT_ACTION: {
        MetaFloatTransparentAction * pAct =
            static_cast<MetaFloatTransparentAction*>(pCurrAct);
        // TODO(F2): Recurse into action metafile
        // (though this is currently not used from the
        // DrawingLayer - shape transparency gradients
        // don't affect shape text)
        return pAct->GetGDIMetaFile().GetActionCount();
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
    VirtualDevice aVDev;
    aVDev.SetOutputSizePixel( aAnimSize );
    aVDev.EnableMapMode( sal_False );

    // setup mask VDev (alpha VDev is currently rather slow)
    VirtualDevice aVDevMask;
    aVDevMask.SetOutputSizePixel( aAnimSize );
    aVDevMask.EnableMapMode( sal_False );

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
                aVDev.DrawBitmapEx(rAnimBmp.aPosPix,
                                   rAnimBmp.aBmpEx);
                Bitmap aMask = rAnimBmp.aBmpEx.GetMask();

                if( aMask.IsEmpty() )
                {
                    const Point aEmpty;
                    const Rectangle aRect(aEmptyPoint,
                                          aVDevMask.GetOutputSizePixel());
                    const Wallpaper aWallpaper(COL_BLACK);
                    aVDevMask.DrawWallpaper(aRect,
                                            aWallpaper);
                }
                else
                {
                    BitmapEx aTmpMask = BitmapEx(aMask,
                                                 aMask);
                    aVDevMask.DrawBitmapEx(rAnimBmp.aPosPix,
                                           aTmpMask );
                }
                break;
            }

            case DISPOSE_BACK:
            {
                // #i70772# react on no mask
                const Bitmap aMask(rAnimBmp.aBmpEx.GetMask());
                const Bitmap aContent(rAnimBmp.aBmpEx.GetBitmap());

                aVDevMask.Erase();
                aVDev.DrawBitmap(rAnimBmp.aPosPix, aContent);

                if(aMask.IsEmpty())
                {
                    const Rectangle aRect(rAnimBmp.aPosPix, aContent.GetSizePixel());
                    aVDevMask.SetFillColor(COL_BLACK);
                    aVDevMask.SetLineColor();
                    aVDevMask.DrawRect(aRect);
                }
                else
                {
                    aVDevMask.DrawBitmap(rAnimBmp.aPosPix, aMask);
                }
                break;
            }

            case DISPOSE_FULL:
            {
                aVDev.DrawBitmapEx(rAnimBmp.aPosPix,
                                   rAnimBmp.aBmpEx);
                break;
            }

            case DISPOSE_PREVIOUS :
            {
                aVDev.DrawBitmapEx(rAnimBmp.aPosPix,
                                   rAnimBmp.aBmpEx);
                aVDevMask.DrawBitmap(rAnimBmp.aPosPix,
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
                                     aVDev.GetBitmap(
                                         aEmptyPoint,
                                         aAnimSize ),
                                     aVDevMask.GetBitmap(
                                         aEmptyPoint,
                                         aAnimSize ))));

        // setup mtf dimensions and pref map mode (for
        // simplicity, keep it all in pixel. the metafile
        // renderer scales it down to (1, 1) box anyway)
        pMtf->SetPrefMapMode( MapMode() );
        pMtf->SetPrefSize( aAnimSize );

        // #115934#
        // Take care of special value for MultiPage TIFFs. ATM these shall just
        // show their first page for _quite_ some time.
        sal_Int32 nWaitTime100thSeconds( rAnimBmp.nWait );
        if( ANIMATION_TIMEOUT_ON_CLICK == nWaitTime100thSeconds )
        {
            // ATM the huge value would block the timer, so use a long
            // time to show first page (whole day)
            nWaitTime100thSeconds = 100 * 60 * 60 * 24;
        }

        // There are animated GIFs with no WaitTime set. Take 1 sec, then.
        if( nWaitTime100thSeconds == 0 )
            nWaitTime100thSeconds = 100;

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
          pCurrAct != 0; pCurrAct = rMtf->NextAction() )
    {
        if (pCurrAct->GetType() == META_COMMENT_ACTION)
        {
            MetaCommentAction * pAct =
                static_cast<MetaCommentAction *>(pCurrAct);
            // skip comment if not a special XTEXT comment
            if (pAct->GetComment().CompareIgnoreCaseToAscii(
                    RTL_CONSTASCII_STRINGPARAM("XTEXT") ) == COMPARE_EQUAL)
            {
                if (pAct->GetComment().CompareIgnoreCaseToAscii(
                        "XTEXT_SCROLLRECT" ) == COMPARE_EQUAL) {
                    o_rScrollRect = ::vcl::unotools::b2DRectangleFromRectangle(
                        *reinterpret_cast<Rectangle const *>(
                            pAct->GetData() ) );

                    bScrollRectSet = true;
                }
                else if (pAct->GetComment().CompareIgnoreCaseToAscii(
                             "XTEXT_PAINTRECT" ) == COMPARE_EQUAL) {
                    o_rPaintRect = ::vcl::unotools::b2DRectangleFromRectangle(
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

