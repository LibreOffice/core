/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */



#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <gdimtftools.hxx>

#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicRenderer.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>

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
#include <vcl/animate.hxx>
#include <vcl/graph.hxx>

#include <unotools/streamwrap.hxx>

#include "tools.hxx"

using namespace ::com::sun::star;





namespace slideshow
{
namespace internal
{











bool hasUnsupportedActions( const GDIMetaFile& rMtf )
{
    
    MetaAction* pCurrAct;

    
    for( pCurrAct = const_cast<GDIMetaFile&>(rMtf).FirstAction();
         pCurrAct;
         pCurrAct = const_cast<GDIMetaFile&>(rMtf).NextAction() )
    {
        switch( pCurrAct->GetType() )
        {
            case META_RASTEROP_ACTION:
                
                if( ROP_OVERPAINT ==
                    static_cast<MetaRasterOpAction*>(pCurrAct)->GetRasterOp() )
                {
                    break;
                }
                
            case META_MOVECLIPREGION_ACTION:
                
            case META_REFPOINT_ACTION:
                
            case META_WALLPAPER_ACTION:
                return true; 
                             
        }
    }

    return false; 
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

} 



bool getMetaFile( const uno::Reference< lang::XComponent >&       xSource,
                  const uno::Reference< drawing::XDrawPage >&     xContainingPage,
                  GDIMetaFile&                                    rMtf,
                  int                                             mtfLoadFlags,
                  const uno::Reference< uno::XComponentContext >& rxContext )
{
    ENSURE_OR_RETURN_FALSE( rxContext.is(),
                       "getMetaFile(): Invalid context" );

    
    

    
    
    
    DummyRenderer*                              pRenderer( new DummyRenderer() );
    uno::Reference< graphic::XGraphicRenderer > xRenderer( pRenderer );

    
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

    
    

    
    
    
    return true;
}

sal_Int32 getNextActionOffset( MetaAction * pCurrAct )
{
    
    
    

    switch (pCurrAct->GetType()) {
    case META_TEXT_ACTION: {
        MetaTextAction * pAct = static_cast<MetaTextAction *>(pCurrAct);
        sal_Int32 nLen = std::min(pAct->GetLen(), pAct->GetText().getLength() - pAct->GetIndex());
        return nLen;
    }
    case META_TEXTARRAY_ACTION: {
        MetaTextArrayAction * pAct =
            static_cast<MetaTextArrayAction *>(pCurrAct);
        sal_Int32 nLen = std::min(pAct->GetLen(), pAct->GetText().getLength() - pAct->GetIndex());
        return nLen;
    }
    case META_STRETCHTEXT_ACTION: {
        MetaStretchTextAction * pAct =
            static_cast<MetaStretchTextAction *>(pCurrAct);
        sal_Int32 nLen = std::min(pAct->GetLen(), pAct->GetText().getLength() - pAct->GetIndex());
        return nLen;
    }
    case META_FLOATTRANSPARENT_ACTION: {
        MetaFloatTransparentAction * pAct =
            static_cast<MetaFloatTransparentAction*>(pCurrAct);
        
        
        
        
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

    
    Animation   aAnimation( rGraphic.GetAnimation() );
    const Point aEmptyPoint;
    const Size  aAnimSize( aAnimation.GetDisplaySizePixel() );

    
    
    
    
    VirtualDevice aVDev;
    aVDev.SetOutputSizePixel( aAnimSize );
    aVDev.EnableMapMode( false );

    
    VirtualDevice aVDevMask;
    aVDevMask.SetOutputSizePixel( aAnimSize );
    aVDevMask.EnableMapMode( false );

    switch( aAnimation.GetCycleMode() )
    {
        case CYCLE_NOT:
            o_rLoopCount = 1;
            o_eCycleMode = CYCLE_LOOP;
            break;

        case CYCLE_FALLBACK:
            
        case CYCLE_NORMAL:
            o_rLoopCount = aAnimation.GetLoopCount();
            o_eCycleMode = CYCLE_LOOP;
            break;

        case CYCLE_REVERS:
            
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

        
        
        
        pMtf->SetPrefMapMode( MapMode() );
        pMtf->SetPrefSize( aAnimSize );

        
        
        sal_Int32 nWaitTime100thSeconds( rAnimBmp.nWait );
        if( ANIMATION_TIMEOUT_ON_CLICK == nWaitTime100thSeconds )
        {
            
            
            nWaitTime100thSeconds = 100 * 60 * 60 * 24;
        }

        
        
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
    
    bool bScrollRectSet(false);
    bool bPaintRectSet(false);

    for ( MetaAction * pCurrAct = rMtf->FirstAction();
          pCurrAct != 0; pCurrAct = rMtf->NextAction() )
    {
        if (pCurrAct->GetType() == META_COMMENT_ACTION)
        {
            MetaCommentAction * pAct =
                static_cast<MetaCommentAction *>(pCurrAct);
            
            if( pAct->GetComment().matchIgnoreAsciiCase( OString("XTEXT"), 0 ) )
            {
                if (pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_SCROLLRECT"))
                {
                    o_rScrollRect = ::vcl::unotools::b2DRectangleFromRectangle(
                        *reinterpret_cast<Rectangle const *>(
                            pAct->GetData() ) );

                    bScrollRectSet = true;
                }
                else if (pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_PAINTRECT") )
                {
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
