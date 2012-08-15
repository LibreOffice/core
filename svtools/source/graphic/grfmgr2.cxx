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


#include <vcl/bmpacc.hxx>
#include <tools/poly.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/metric.hxx>
#include <vcl/animate.hxx>
#include <vcl/alpha.hxx>
#include <vcl/virdev.hxx>
#include "grfcache.hxx"
#include <svtools/grfmgr.hxx>

// -----------
// - defines -
// -----------

#define WATERMARK_LUM_OFFSET        50
#define WATERMARK_CON_OFFSET        -70

// ------------------
// - GraphicManager -
// ------------------

GraphicManager::GraphicManager( sal_uLong nCacheSize, sal_uLong nMaxObjCacheSize ) :
        mpCache( new GraphicCache( nCacheSize, nMaxObjCacheSize ) )
{
}

GraphicManager::~GraphicManager()
{
    for( size_t i = 0, n = maObjList.size(); i < n; ++i )
        maObjList[ i ]->GraphicManagerDestroyed();

    delete mpCache;
}

void GraphicManager::SetMaxCacheSize( sal_uLong nNewCacheSize )
{
    mpCache->SetMaxDisplayCacheSize( nNewCacheSize );
}

void GraphicManager::SetMaxObjCacheSize( sal_uLong nNewMaxObjSize, sal_Bool bDestroyGreaterCached )
{
    mpCache->SetMaxObjDisplayCacheSize( nNewMaxObjSize, bDestroyGreaterCached );
}

void GraphicManager::SetCacheTimeout( sal_uLong nTimeoutSeconds )
{
    mpCache->SetCacheTimeout( nTimeoutSeconds );
}

void GraphicManager::ReleaseFromCache( const GraphicObject& /*rObj*/ )
{
    // !!!
}

sal_Bool GraphicManager::IsInCache( OutputDevice* pOut, const Point& rPt,
                                    const Size& rSz, const GraphicObject& rObj,
                                    const GraphicAttr& rAttr ) const
{
    return mpCache->IsInDisplayCache( pOut, rPt, rSz, rObj, rAttr );
}

sal_Bool GraphicManager::DrawObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                              GraphicObject& rObj, const GraphicAttr& rAttr,
                              const sal_uLong nFlags, sal_Bool& rCached )
{
    Point   aPt( rPt );
    Size    aSz( rSz );
    sal_Bool    bRet = sal_False;

    rCached = sal_False;

    if( ( rObj.GetType() == GRAPHIC_BITMAP ) || ( rObj.GetType() == GRAPHIC_GDIMETAFILE ) )
    {
        // create output and fill cache

        if( rObj.IsAnimated() || ( pOut->GetOutDevType() == OUTDEV_PRINTER ) ||
            ( !( nFlags & GRFMGR_DRAW_NO_SUBSTITUTE ) &&
              ( ( nFlags & GRFMGR_DRAW_SUBSTITUTE ) ||
                !( nFlags & GRFMGR_DRAW_CACHED ) ||
                ( pOut->GetConnectMetaFile() && !pOut->IsOutputEnabled() ) ) ) )
        {
            // simple output of transformed graphic
            const Graphic aGraphic( rObj.GetTransformedGraphic( &rAttr ) );

            if( aGraphic.IsSupportedGraphic() )
            {
                const sal_uInt16 nRot10 = rAttr.GetRotation() % 3600;

                if( nRot10 )
                {
                    Polygon aPoly( Rectangle( aPt, aSz ) );

                    aPoly.Rotate( aPt, nRot10 );
                    const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
                    aPt = aRotBoundRect.TopLeft();
                    aSz = aRotBoundRect.GetSize();
                }

                aGraphic.Draw( pOut, aPt, aSz );
            }

            bRet = sal_True;
        }

        if( !bRet )
        {
            // cached/direct drawing
            if( !mpCache->DrawDisplayCacheObj( pOut, aPt, aSz, rObj, rAttr ) )
                bRet = ImplDraw( pOut, aPt, aSz, rObj, rAttr, nFlags, rCached );
            else
                bRet = rCached = sal_True;
        }
    }

    return bRet;
}

void GraphicManager::ImplRegisterObj( const GraphicObject& rObj, Graphic& rSubstitute,
                                      const rtl::OString* pID, const GraphicObject* pCopyObj )
{
    maObjList.push_back( (GraphicObject*)&rObj );
    mpCache->AddGraphicObject( rObj, rSubstitute, pID, pCopyObj );
}

void GraphicManager::ImplUnregisterObj( const GraphicObject& rObj )
{
    mpCache->ReleaseGraphicObject( rObj );
    for( GraphicObjectList_impl::iterator it = maObjList.begin(); it != maObjList.end(); ++it )
    {
        if ( *it == &rObj ) {
            maObjList.erase( it );
            break;
        }
    }
}

void GraphicManager::ImplGraphicObjectWasSwappedOut( const GraphicObject& rObj )
{
    mpCache->GraphicObjectWasSwappedOut( rObj );
}

rtl::OString GraphicManager::ImplGetUniqueID( const GraphicObject& rObj ) const
{
    return mpCache->GetUniqueID( rObj );
}

sal_Bool GraphicManager::ImplFillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute )
{
    return( mpCache->FillSwappedGraphicObject( rObj, rSubstitute ) );
}

void GraphicManager::ImplGraphicObjectWasSwappedIn( const GraphicObject& rObj )
{
    mpCache->GraphicObjectWasSwappedIn( rObj );
}

sal_Bool GraphicManager::ImplDraw( OutputDevice* pOut, const Point& rPt,
                               const Size& rSz, GraphicObject& rObj,
                               const GraphicAttr& rAttr,
                               const sal_uLong nFlags, sal_Bool& rCached )
{
    const Graphic&  rGraphic = rObj.GetGraphic();
    sal_Bool            bRet = sal_False;

    if( rGraphic.IsSupportedGraphic() && !rGraphic.IsSwapOut() )
    {
        if( GRAPHIC_BITMAP == rGraphic.GetType() )
        {
            const BitmapEx aSrcBmpEx( rGraphic.GetBitmapEx() );

            // #i46805# No point in caching a bitmap that is rendered
            // via RectFill on the OutDev
            if( !(pOut->GetDrawMode() & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP )) &&
                mpCache->IsDisplayCacheable( pOut, rPt, rSz, rObj, rAttr ) )
            {
                BitmapEx aDstBmpEx;

                if( ImplCreateOutput( pOut, rPt, rSz, aSrcBmpEx, rAttr, nFlags, &aDstBmpEx ) )
                {
                    rCached = mpCache->CreateDisplayCacheObj( pOut, rPt, rSz, rObj, rAttr, aDstBmpEx );
                    bRet = sal_True;
                }
            }

            if( !bRet )
                bRet = ImplCreateOutput( pOut, rPt, rSz, aSrcBmpEx, rAttr, nFlags );
        }
        else
        {
            const GDIMetaFile& rSrcMtf = rGraphic.GetGDIMetaFile();

            if( mpCache->IsDisplayCacheable( pOut, rPt, rSz, rObj, rAttr ) )
            {
                GDIMetaFile aDstMtf;
                BitmapEx    aContainedBmpEx;

                if( ImplCreateOutput( pOut, rPt, rSz, rSrcMtf, rAttr, nFlags, aDstMtf, aContainedBmpEx ) )
                {
                    if( !!aContainedBmpEx )
                    {
                        // Use bitmap output method, if metafile basically contains only a single
                        // bitmap (allows caching the resulting pixmap).
                        BitmapEx aDstBmpEx;

                        if( ImplCreateOutput( pOut, rPt, rSz, aContainedBmpEx, rAttr, nFlags, &aDstBmpEx ) )
                        {
                            rCached = mpCache->CreateDisplayCacheObj( pOut, rPt, rSz, rObj, rAttr, aDstBmpEx );
                            bRet = sal_True;
                        }
                    }
                    else
                    {
                        rCached = mpCache->CreateDisplayCacheObj( pOut, rPt, rSz, rObj, rAttr, aDstMtf );
                        bRet = sal_True;
                    }
                }
            }

            if( !bRet )
            {
                const Graphic aGraphic( rObj.GetTransformedGraphic( &rAttr ) );

                if( aGraphic.IsSupportedGraphic() )
                {
                    aGraphic.Draw( pOut, rPt, rSz );
                    bRet = sal_True;
                }
            }
        }
    }

    return bRet;
}

sal_Bool GraphicManager::ImplCreateOutput( OutputDevice* pOutputDevice,
                                       const Point& rPoint, const Size& rSize,
                                       const BitmapEx& rBitmapEx, const GraphicAttr& rAttributes,
                                       const sal_uLong nFlags, BitmapEx* pResultBitmapEx )
{
    bool        bRet = false;

    Point       aUnrotatedPointInPixels( pOutputDevice->LogicToPixel( rPoint ) );
    Size        aUnrotatedSizeInPixels(  pOutputDevice->LogicToPixel( rSize ) );

    if( aUnrotatedSizeInPixels.Width() <= 0  || aUnrotatedSizeInPixels.Height() <= 0)
        return false;

    Point       aOutPointInPixels;
    Size        aOutSizeInPixels;
    BitmapEx    aBitmapEx( rBitmapEx );
    int         nRotation = rAttributes.GetRotation() % 3600;

    if( nRotation != 0 )
    {
        Polygon aPoly( Rectangle( rPoint, rSize ) );
        aPoly.Rotate( rPoint, nRotation );
        const Rectangle aRotationBoundRect( aPoly.GetBoundRect() );
        aOutPointInPixels = pOutputDevice->LogicToPixel( aRotationBoundRect.TopLeft() );
        aOutSizeInPixels  = pOutputDevice->LogicToPixel( aRotationBoundRect.GetSize() );
    }
    else
    {
        aOutPointInPixels = aUnrotatedPointInPixels;
        aOutSizeInPixels  = aUnrotatedSizeInPixels;
    }

    Point       aOutPoint;
    Size        aOutSize;

    const Size& rBitmapSizePixels = rBitmapEx.GetSizePixel();
    Rectangle   aCropRectangle(0, 0, 0, 0);

    bool        isHorizontalMirrored = ( rAttributes.GetMirrorFlags() & BMP_MIRROR_HORZ ) != 0;
    bool        isVerticalMirrored   = ( rAttributes.GetMirrorFlags() & BMP_MIRROR_VERT ) != 0;

    // calculate output sizes
    if( true || !pResultBitmapEx )
    {
        Rectangle aBitmapRectangle( aOutPointInPixels, aOutSizeInPixels );
        Rectangle aOutRect( Point(), pOutputDevice->GetOutputSizePixel() );

        if( pOutputDevice->GetOutDevType() == OUTDEV_WINDOW )
        {
            const Region aPaintRegion( ( (Window*) pOutputDevice )->GetPaintRegion() );

            if( !aPaintRegion.IsNull() )
                aOutRect.Intersection( pOutputDevice->LogicToPixel( aPaintRegion.GetBoundRect() ) );
        }
        aOutRect.Intersection( aBitmapRectangle );

        if( !aOutRect.IsEmpty() )
        {
            aOutPoint = pOutputDevice->PixelToLogic( aOutRect.TopLeft() );
            aOutSize =  pOutputDevice->PixelToLogic( aOutRect.GetSize() );

            aCropRectangle = Rectangle(
                aOutRect.Left()   - aBitmapRectangle.Left(),
                aOutRect.Top()    - aBitmapRectangle.Top(),
                aOutRect.Right()  - aBitmapRectangle.Left(),
                aOutRect.Bottom() - aBitmapRectangle.Top() );
        }
    }
    else
    {
        aOutPoint = pOutputDevice->PixelToLogic( aOutPointInPixels );
        aOutSize =  pOutputDevice->PixelToLogic( aOutSizeInPixels );

        aCropRectangle = Rectangle(
            0, 0,
            aOutSizeInPixels.Width()  - 1,
            aOutSizeInPixels.Height() - 1 );
    }


    if( aCropRectangle.GetWidth() <= 0 && aCropRectangle.GetHeight() <= 0 )
        return false;

    // do transformation
    if( !isHorizontalMirrored &&
        !isVerticalMirrored &&
        !nRotation &&
        aOutSizeInPixels == rBitmapSizePixels)
    {
        // simple copy thorugh
        aOutPoint = pOutputDevice->PixelToLogic( aOutPointInPixels );
        aOutSize = pOutputDevice->PixelToLogic( aOutSizeInPixels );
        bRet = true;
    }
    else
    {
        // mirror the image - this should not impact the picture dimenstions
        if( isHorizontalMirrored || isVerticalMirrored )
            bRet = aBitmapEx.Mirror( rAttributes.GetMirrorFlags() );

        // prepare rotation if needed
        if (nRotation != 0)
        {
            Polygon aPoly( Rectangle( Point(), aUnrotatedSizeInPixels) );
            aPoly.Rotate( Point(), nRotation );
            Rectangle aNewBound( aPoly.GetBoundRect() );

            aCropRectangle = Rectangle (
                                aCropRectangle.Left()   + aNewBound.Left(),
                                aCropRectangle.Top()    + aNewBound.Top(),
                                aCropRectangle.Right()  + aNewBound.Left(),
                                aCropRectangle.Bottom() + aNewBound.Top() );
        }

        // calculate scaling factors
        double fScaleX = aUnrotatedSizeInPixels.Width()  / (double) rBitmapSizePixels.Width();
        double fScaleY = aUnrotatedSizeInPixels.Height() / (double) rBitmapSizePixels.Height();

        if( nFlags & GRFMGR_DRAW_SMOOTHSCALE )
        {
            bRet = aBitmapEx.ScaleCropRotate( fScaleX, fScaleY, aCropRectangle, nRotation, COL_TRANSPARENT );
        }
        else
        {
            aCropRectangle = Rectangle (
                                aCropRectangle.Left()   / fScaleX,
                                aCropRectangle.Top()    / fScaleY,
                                aCropRectangle.Right()  / fScaleX,
                                aCropRectangle.Bottom() / fScaleY );

            bRet = aBitmapEx.Crop( aCropRectangle );
            if ( bRet )
                bRet = aBitmapEx.Scale( fScaleX, fScaleY );
        }
    }

    if( bRet )
    {
        // attribute adjustment if neccessary
        if(  rAttributes.IsSpecialDrawMode()
          || rAttributes.IsAdjusted()
          || rAttributes.IsTransparent() )
        {
            ImplAdjust( aBitmapEx, rAttributes, ADJUSTMENT_DRAWMODE | ADJUSTMENT_COLORS | ADJUSTMENT_TRANSPARENCY );
        }

        // OutDev adjustment if neccessary
        if(   pOutputDevice->GetOutDevType() != OUTDEV_PRINTER
          &&  pOutputDevice->GetBitCount() <= 8
          &&  aBitmapEx.GetBitCount() >= 8 )
        {
            aBitmapEx.Dither( BMP_DITHER_MATRIX );
        }
    }

    // create output
    if( bRet )
    {
        if( pResultBitmapEx )
        {
            if( !rAttributes.IsTransparent() && !aBitmapEx.IsAlpha() )
                aBitmapEx = BitmapEx( aBitmapEx.GetBitmap().CreateDisplayBitmap( pOutputDevice ), aBitmapEx.GetMask() );

            *pResultBitmapEx = aBitmapEx;
        }
        pOutputDevice->DrawBitmapEx( aOutPoint, aOutSize, aBitmapEx);
    }

    return bRet;
}

// This function checks whether the bitmap is usable for skipping
// mtf rendering by using just this one bitmap (i.e. in case the metafile
// contains just this one pixmap that covers the entire metafile area).
static BitmapEx checkMetadataBitmap( const BitmapEx& rBmpEx,
                                     Point    rSrcPoint,
                                     Size     rSrcSize,
                                     const Point&    rDestPoint,
                                     const Size&     rDestSize,
                                     const Size&     rRefSize,
                                     bool&           o_rbNonBitmapActionEncountered )
{
// NOTE: If you do changes in this function, change checkMetadataBitmap() in grfcache.cxx too.
    BitmapEx aBmpEx;
    if( rSrcSize == Size())
        rSrcSize = rBmpEx.GetSizePixel();

    if( rDestPoint != Point( 0, 0 ))
    {   // The pixmap in the metafile has an offset (and so would not cover)
        // the entire result -> fall back to mtf rendering.
        o_rbNonBitmapActionEncountered = true;
        return aBmpEx;
    }
    if( rDestSize != rRefSize )
    {   // The pixmap is not fullscale (does not cover the entire metafile area).
        // HACK: The code here should refuse to use the bitmap directly
        // and fall back to mtf rendering, but there seem to be metafiles
        // that do not specify exactly their area (the Windows API requires apps
        // the specify it manually, the rectangle is specified as topleft/bottomright
        // rather than topleft/size [which may be confusing], and the docs
        // on the exact meaning are somewhat confusing as well), so if it turns
        // out this metafile really contains just one bitmap and no other painting,
        // and if the sizes almost match, just use the pixmap (which will be scaled
        // to fit exactly the requested size, so there should not be any actual problem
        // caused by this small difference). This will allow caching of the resulting
        // (scaled) pixmap, which can make a noticeable performance difference.
        if( rBmpEx.GetSizePixel().Width() > 100 && rBmpEx.GetSizePixel().Height() > 100
            && abs( rDestSize.Width() - rRefSize.Width()) < 5
            && abs( rDestSize.Height() - rRefSize.Height()) < 5 )
            ; // ok, assume it's close enough
        else
        {  // fall back to mtf rendering
            o_rbNonBitmapActionEncountered = true;
            return aBmpEx;
        }
    }

    aBmpEx = rBmpEx;

    if( (rSrcPoint.X() != 0 && rSrcPoint.Y() != 0) ||
        rSrcSize != rBmpEx.GetSizePixel() )
    {
        // crop bitmap to given source rectangle (no
        // need to copy and convert the whole bitmap)
        const Rectangle aCropRect( rSrcPoint,
                                   rSrcSize );
        aBmpEx.Crop( aCropRect );
    }

    return aBmpEx;
}

sal_Bool GraphicManager::ImplCreateOutput( OutputDevice* pOut,
                                       const Point& rPt, const Size& rSz,
                                       const GDIMetaFile& rMtf, const GraphicAttr& rAttr,
                                       const sal_uLong /*nFlags*/, GDIMetaFile& rOutMtf, BitmapEx& rOutBmpEx )
{
    const Size aNewSize( rMtf.GetPrefSize() );

    rOutMtf = rMtf;

    // Count bitmap actions, and flag actions that paint, but
    // are no bitmaps.
    sal_Int32   nNumBitmaps(0);
    bool        bNonBitmapActionEncountered(false);
    if( aNewSize.Width() && aNewSize.Height() && rSz.Width() && rSz.Height() )
    {
        const double fGrfWH = (double) aNewSize.Width() / aNewSize.Height();
        const double fOutWH = (double) rSz.Width() / rSz.Height();

        const double fScaleX = fOutWH / fGrfWH;
        const double fScaleY = 1.0;

        const MapMode rPrefMapMode( rMtf.GetPrefMapMode() );
        const Size rSizePix( pOut->LogicToPixel( aNewSize, rPrefMapMode ) );

// NOTE: If you do changes in this function, check GraphicDisplayCacheEntry::IsCacheableAsBitmap
// in grfcache.cxx too.

        // Determine whether the metafile basically displays
        // a single bitmap (in which case that bitmap is simply used directly
        // instead of playing the metafile). Note that
        // the solution, as implemented here, is quite suboptimal (the
        // cases where a mtf consisting basically of a single bitmap,
        // that fail to pass the test below, are probably frequent). A
        // better solution would involve FSAA, but that's currently
        // expensive, and might trigger bugs on display drivers, if
        // VDevs get bigger than the actual screen.
        sal_uInt32  nCurPos;
        MetaAction* pAct;
        for( nCurPos = 0, pAct = (MetaAction*)rOutMtf.FirstAction(); pAct;
             pAct = (MetaAction*)rOutMtf.NextAction(), nCurPos++ )
        {
            MetaAction* pModAct = NULL;
            switch( pAct->GetType() )
            {
                case META_FONT_ACTION:
                {
                    // taking care of font width default if scaling metafile.
                    MetaFontAction* pA = (MetaFontAction*)pAct;
                    Font aFont( pA->GetFont() );
                    if ( !aFont.GetWidth() )
                    {
                        FontMetric aFontMetric( pOut->GetFontMetric( aFont ) );
                        aFont.SetWidth( aFontMetric.GetWidth() );
                        pModAct = new MetaFontAction( aFont );
                    }
                }
                    // FALLTHROUGH intended
                case META_NULL_ACTION:
                    // FALLTHROUGH intended

                    // OutDev state changes (which don't affect bitmap
                    // output)
                case META_LINECOLOR_ACTION:
                    // FALLTHROUGH intended
                case META_FILLCOLOR_ACTION:
                    // FALLTHROUGH intended
                case META_TEXTCOLOR_ACTION:
                    // FALLTHROUGH intended
                case META_TEXTFILLCOLOR_ACTION:
                    // FALLTHROUGH intended
                case META_TEXTALIGN_ACTION:
                    // FALLTHROUGH intended
                case META_TEXTLINECOLOR_ACTION:
                    // FALLTHROUGH intended
                case META_TEXTLINE_ACTION:
                    // FALLTHROUGH intended
                case META_PUSH_ACTION:
                    // FALLTHROUGH intended
                case META_POP_ACTION:
                    // FALLTHROUGH intended
                case META_LAYOUTMODE_ACTION:
                    // FALLTHROUGH intended
                case META_TEXTLANGUAGE_ACTION:
                    // FALLTHROUGH intended
                case META_COMMENT_ACTION:
                    break;

                    // bitmap output methods
                case META_BMP_ACTION:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpAction* pAction = (MetaBmpAction*)pAct;

                        rOutBmpEx = checkMetadataBitmap(
                            BitmapEx( pAction->GetBitmap()),
                            Point(), Size(),
                            pOut->LogicToPixel( pAction->GetPoint(),
                                                rPrefMapMode ),
                            pAction->GetBitmap().GetSizePixel(),
                            rSizePix,
                            bNonBitmapActionEncountered );
                    }
                    ++nNumBitmaps;
                    break;

                case META_BMPSCALE_ACTION:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpScaleAction* pAction = (MetaBmpScaleAction*)pAct;

                        rOutBmpEx = checkMetadataBitmap(
                            BitmapEx( pAction->GetBitmap()),
                            Point(), Size(),
                            pOut->LogicToPixel( pAction->GetPoint(),
                                                rPrefMapMode ),
                            pOut->LogicToPixel( pAction->GetSize(),
                                                rPrefMapMode ),
                            rSizePix,
                            bNonBitmapActionEncountered );
                    }
                    ++nNumBitmaps;
                    break;

                case META_BMPSCALEPART_ACTION:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpScalePartAction* pAction = (MetaBmpScalePartAction*)pAct;

                        rOutBmpEx = checkMetadataBitmap(
                                                    BitmapEx( pAction->GetBitmap() ),
                                                    pAction->GetSrcPoint(),
                                                    pAction->GetSrcSize(),
                                                    pOut->LogicToPixel( pAction->GetDestPoint(),
                                                                        rPrefMapMode ),
                                                    pOut->LogicToPixel( pAction->GetDestSize(),
                                                                        rPrefMapMode ),
                                                    rSizePix,
                                                    bNonBitmapActionEncountered );
                    }
                    ++nNumBitmaps;
                    break;

                case META_BMPEX_ACTION:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpExAction* pAction = (MetaBmpExAction*)pAct;

                        rOutBmpEx = checkMetadataBitmap(
                            pAction->GetBitmapEx(),
                            Point(), Size(),
                            pOut->LogicToPixel( pAction->GetPoint(),
                                                rPrefMapMode ),
                            pAction->GetBitmapEx().GetSizePixel(),
                            rSizePix,
                            bNonBitmapActionEncountered );
                    }
                    ++nNumBitmaps;
                    break;

                case META_BMPEXSCALE_ACTION:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpExScaleAction* pAction = (MetaBmpExScaleAction*)pAct;

                        rOutBmpEx = checkMetadataBitmap(
                            pAction->GetBitmapEx(),
                            Point(), Size(),
                            pOut->LogicToPixel( pAction->GetPoint(),
                                                rPrefMapMode ),
                            pOut->LogicToPixel( pAction->GetSize(),
                                                rPrefMapMode ),
                            rSizePix,
                            bNonBitmapActionEncountered );
                    }
                    ++nNumBitmaps;
                    break;

                case META_BMPEXSCALEPART_ACTION:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpExScalePartAction* pAction = (MetaBmpExScalePartAction*)pAct;

                        rOutBmpEx = checkMetadataBitmap( pAction->GetBitmapEx(),
                                                    pAction->GetSrcPoint(),
                                                    pAction->GetSrcSize(),
                                                    pOut->LogicToPixel( pAction->GetDestPoint(),
                                                                        rPrefMapMode ),
                                                    pOut->LogicToPixel( pAction->GetDestSize(),
                                                                        rPrefMapMode ),
                                                    rSizePix,
                                                    bNonBitmapActionEncountered );
                    }
                    ++nNumBitmaps;
                    break;

                    // these actions actually output something (that's
                    // different from a bitmap)
                case META_RASTEROP_ACTION:
                    if( ((MetaRasterOpAction*)pAct)->GetRasterOp() == ROP_OVERPAINT )
                        break;
                    // FALLTHROUGH intended
                case META_PIXEL_ACTION:
                    // FALLTHROUGH intended
                case META_POINT_ACTION:
                    // FALLTHROUGH intended
                case META_LINE_ACTION:
                    // FALLTHROUGH intended
                case META_RECT_ACTION:
                    // FALLTHROUGH intended
                case META_ROUNDRECT_ACTION:
                    // FALLTHROUGH intended
                case META_ELLIPSE_ACTION:
                    // FALLTHROUGH intended
                case META_ARC_ACTION:
                    // FALLTHROUGH intended
                case META_PIE_ACTION:
                    // FALLTHROUGH intended
                case META_CHORD_ACTION:
                    // FALLTHROUGH intended
                case META_POLYLINE_ACTION:
                    // FALLTHROUGH intended
                case META_POLYGON_ACTION:
                    // FALLTHROUGH intended
                case META_POLYPOLYGON_ACTION:
                    // FALLTHROUGH intended

                case META_TEXT_ACTION:
                    // FALLTHROUGH intended
                case META_TEXTARRAY_ACTION:
                    // FALLTHROUGH intended
                case META_STRETCHTEXT_ACTION:
                    // FALLTHROUGH intended
                case META_TEXTRECT_ACTION:
                    // FALLTHROUGH intended

                case META_MASK_ACTION:
                    // FALLTHROUGH intended
                case META_MASKSCALE_ACTION:
                    // FALLTHROUGH intended
                case META_MASKSCALEPART_ACTION:
                    // FALLTHROUGH intended

                case META_GRADIENT_ACTION:
                    // FALLTHROUGH intended
                case META_HATCH_ACTION:
                    // FALLTHROUGH intended
                case META_WALLPAPER_ACTION:
                    // FALLTHROUGH intended

                case META_TRANSPARENT_ACTION:
                    // FALLTHROUGH intended
                case META_EPS_ACTION:
                    // FALLTHROUGH intended
                case META_FLOATTRANSPARENT_ACTION:
                    // FALLTHROUGH intended
                case META_GRADIENTEX_ACTION:
                    // FALLTHROUGH intended
                case META_RENDERGRAPHIC_ACTION:
                    // FALLTHROUGH intended

                    // OutDev state changes that _do_ affect bitmap
                    // output
                case META_CLIPREGION_ACTION:
                    // FALLTHROUGH intended
                case META_ISECTRECTCLIPREGION_ACTION:
                    // FALLTHROUGH intended
                case META_ISECTREGIONCLIPREGION_ACTION:
                    // FALLTHROUGH intended
                case META_MOVECLIPREGION_ACTION:
                    // FALLTHROUGH intended

                case META_MAPMODE_ACTION:
                    // FALLTHROUGH intended
                case META_REFPOINT_ACTION:
                    // FALLTHROUGH intended
                default:
                    bNonBitmapActionEncountered = true;
                    break;
            }
            if ( pModAct )
            {
                rOutMtf.ReplaceAction( pModAct, nCurPos );
                pAct->Delete();
            }
            else
            {
                if( pAct->GetRefCount() > 1 )
                {
                    rOutMtf.ReplaceAction( pModAct = pAct->Clone(), nCurPos );
                    pAct->Delete();
                }
                else
                    pModAct = pAct;
            }
            pModAct->Scale( fScaleX, fScaleY );
        }
        rOutMtf.SetPrefSize( Size( FRound( aNewSize.Width() * fScaleX ),
                                   FRound( aNewSize.Height() * fScaleY ) ) );
    }

    if( nNumBitmaps != 1 || bNonBitmapActionEncountered )
    {
        if( rAttr.IsSpecialDrawMode() || rAttr.IsAdjusted() || rAttr.IsMirrored() || rAttr.IsRotated() || rAttr.IsTransparent() )
            ImplAdjust( rOutMtf, rAttr, ADJUSTMENT_ALL );

        ImplDraw( pOut, rPt, rSz, rOutMtf, rAttr );
        rOutBmpEx = BitmapEx();
    }

    return sal_True;
}

void GraphicManager::ImplAdjust( BitmapEx& rBmpEx, const GraphicAttr& rAttr, sal_uLong nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & ADJUSTMENT_DRAWMODE ) && aAttr.IsSpecialDrawMode() )
    {
        switch( aAttr.GetDrawMode() )
        {
            case( GRAPHICDRAWMODE_MONO ):
                rBmpEx.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            break;

            case( GRAPHICDRAWMODE_GREYS ):
                rBmpEx.Convert( BMP_CONVERSION_8BIT_GREYS );
            break;

            case( GRAPHICDRAWMODE_WATERMARK ):
            {
                aAttr.SetLuminance( aAttr.GetLuminance() + WATERMARK_LUM_OFFSET );
                aAttr.SetContrast( aAttr.GetContrast() + WATERMARK_CON_OFFSET );
            }
            break;

            default:
            break;
        }
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_COLORS ) && aAttr.IsAdjusted() )
    {
        rBmpEx.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                       aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                       aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_MIRROR ) && aAttr.IsMirrored() )
    {
        rBmpEx.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_ROTATE ) && aAttr.IsRotated() )
    {
        rBmpEx.Rotate( aAttr.GetRotation(), Color( COL_TRANSPARENT ) );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        AlphaMask   aAlpha;
        sal_uInt8       cTrans = aAttr.GetTransparency();

        if( !rBmpEx.IsTransparent() )
            aAlpha = AlphaMask( rBmpEx.GetSizePixel(), &cTrans );
        else if( !rBmpEx.IsAlpha() )
        {
            aAlpha = rBmpEx.GetMask();
            aAlpha.Replace( 0, cTrans );
        }
        else
        {
            aAlpha = rBmpEx.GetAlpha();
            BitmapWriteAccess* pA = aAlpha.AcquireWriteAccess();

            if( pA )
            {
                sal_uLong       nTrans = cTrans, nNewTrans;
                const long  nWidth = pA->Width(), nHeight = pA->Height();

                if( pA->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                {
                    for( long nY = 0; nY < nHeight; nY++ )
                    {
                        Scanline pAScan = pA->GetScanline( nY );

                        for( long nX = 0; nX < nWidth; nX++ )
                        {
                            nNewTrans = nTrans + *pAScan;
                            *pAScan++ = (sal_uInt8) ( ( nNewTrans & 0xffffff00 ) ? 255 : nNewTrans );
                        }
                    }
                }
                else
                {
                    BitmapColor aAlphaValue( 0 );

                    for( long nY = 0; nY < nHeight; nY++ )
                    {
                        for( long nX = 0; nX < nWidth; nX++ )
                        {
                            nNewTrans = nTrans + pA->GetPixel( nY, nX ).GetIndex();
                            aAlphaValue.SetIndex( (sal_uInt8) ( ( nNewTrans & 0xffffff00 ) ? 255 : nNewTrans ) );
                            pA->SetPixel( nY, nX, aAlphaValue );
                        }
                    }
                }

                aAlpha.ReleaseAccess( pA );
            }
        }

        rBmpEx = BitmapEx( rBmpEx.GetBitmap(), aAlpha );
    }
}

void GraphicManager::ImplAdjust( GDIMetaFile& rMtf, const GraphicAttr& rAttr, sal_uLong nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & ADJUSTMENT_DRAWMODE ) && aAttr.IsSpecialDrawMode() )
    {
        switch( aAttr.GetDrawMode() )
        {
            case( GRAPHICDRAWMODE_MONO ):
                rMtf.Convert( MTF_CONVERSION_1BIT_THRESHOLD );
            break;

            case( GRAPHICDRAWMODE_GREYS ):
                rMtf.Convert( MTF_CONVERSION_8BIT_GREYS );
            break;

            case( GRAPHICDRAWMODE_WATERMARK ):
            {
                aAttr.SetLuminance( aAttr.GetLuminance() + WATERMARK_LUM_OFFSET );
                aAttr.SetContrast( aAttr.GetContrast() + WATERMARK_CON_OFFSET );
            }
            break;

            default:
            break;
        }
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_COLORS ) && aAttr.IsAdjusted() )
    {
        rMtf.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                     aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                     aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_MIRROR ) && aAttr.IsMirrored() )
    {
        rMtf.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_ROTATE ) && aAttr.IsRotated() )
    {
        rMtf.Rotate( aAttr.GetRotation() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        OSL_FAIL( "Missing implementation: Mtf-Transparency" );
    }
}

void GraphicManager::ImplAdjust( Animation& rAnimation, const GraphicAttr& rAttr, sal_uLong nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & ADJUSTMENT_DRAWMODE ) && aAttr.IsSpecialDrawMode() )
    {
        switch( aAttr.GetDrawMode() )
        {
            case( GRAPHICDRAWMODE_MONO ):
                rAnimation.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            break;

            case( GRAPHICDRAWMODE_GREYS ):
                rAnimation.Convert( BMP_CONVERSION_8BIT_GREYS );
            break;

            case( GRAPHICDRAWMODE_WATERMARK ):
            {
                aAttr.SetLuminance( aAttr.GetLuminance() + WATERMARK_LUM_OFFSET );
                aAttr.SetContrast( aAttr.GetContrast() + WATERMARK_CON_OFFSET );
            }
            break;

            default:
            break;
        }
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_COLORS ) && aAttr.IsAdjusted() )
    {
        rAnimation.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                           aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                           aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_MIRROR ) && aAttr.IsMirrored() )
    {
        rAnimation.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_ROTATE ) && aAttr.IsRotated() )
    {
        OSL_FAIL( "Missing implementation: Animation-Rotation" );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        OSL_FAIL( "Missing implementation: Animation-Transparency" );
    }
}

void GraphicManager::ImplDraw( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                               const GDIMetaFile& rMtf, const GraphicAttr& rAttr )
{
       sal_uInt16   nRot10 = rAttr.GetRotation() % 3600;
    Point   aOutPt( rPt );
    Size    aOutSz( rSz );

    if( nRot10 )
    {
        Polygon aPoly( Rectangle( aOutPt, aOutSz ) );

        aPoly.Rotate( aOutPt, nRot10 );
        const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
        aOutPt = aRotBoundRect.TopLeft();
        aOutSz = aRotBoundRect.GetSize();
    }

    pOut->Push( PUSH_CLIPREGION );
    pOut->IntersectClipRegion( Rectangle( aOutPt, aOutSz ) );

    ( (GDIMetaFile&) rMtf ).WindStart();
    ( (GDIMetaFile&) rMtf ).Play( pOut, aOutPt, aOutSz );
    ( (GDIMetaFile&) rMtf ).WindStart();

    pOut->Pop();
}

struct ImplTileInfo
{
    ImplTileInfo() : aTileTopLeft(), aNextTileTopLeft(), aTileSizePixel(), nTilesEmptyX(0), nTilesEmptyY(0) {}

    Point aTileTopLeft;     // top, left position of the rendered tile
    Point aNextTileTopLeft; // top, left position for next recursion
                            // level's tile
    Size  aTileSizePixel;   // size of the generated tile (might
                            // differ from
                            // aNextTileTopLeft-aTileTopLeft, because
                            // this is nExponent*prevTileSize. The
                            // generated tile is always nExponent
                            // times the previous tile, such that it
                            // can be used in the next stage. The
                            // required area coverage is often
                            // less. The extraneous area covered is
                            // later overwritten by the next stage)
    int   nTilesEmptyX;     // number of original tiles empty right of
                            // this tile. This counts from
                            // aNextTileTopLeft, i.e. the additional
                            // area covered by aTileSizePixel is not
                            // considered here. This is for
                            // unification purposes, as the iterative
                            // calculation of the next level's empty
                            // tiles has to be based on this value.
    int   nTilesEmptyY;     // as above, for Y
};


bool GraphicObject::ImplRenderTempTile( VirtualDevice& rVDev, int nExponent,
                                        int nNumTilesX, int nNumTilesY,
                                        const Size& rTileSizePixel,
                                        const GraphicAttr* pAttr, sal_uLong nFlags )
{
    if( nExponent <= 1 )
        return false;

    // determine MSB factor
    int nMSBFactor( 1 );
    while( nNumTilesX / nMSBFactor != 0 ||
           nNumTilesY / nMSBFactor != 0 )
    {
        nMSBFactor *= nExponent;
    }

    // one less
    nMSBFactor /= nExponent;

    ImplTileInfo aTileInfo;

    // #105229# Switch off mapping (converting to logic and back to
    // pixel might cause roundoff errors)
    sal_Bool bOldMap( rVDev.IsMapModeEnabled() );
    rVDev.EnableMapMode( sal_False );

    bool bRet( ImplRenderTileRecursive( rVDev, nExponent, nMSBFactor, nNumTilesX, nNumTilesY,
                                        nNumTilesX, nNumTilesY, rTileSizePixel, pAttr, nFlags, aTileInfo ) );

    rVDev.EnableMapMode( bOldMap );

    return bRet;
}

// define for debug drawings
//#define DBG_TEST

// see header comment. this works similar to base conversion of a
// number, i.e. if the exponent is 10, then the number for every tile
// size is given by the decimal place of the corresponding decimal
// representation.
bool GraphicObject::ImplRenderTileRecursive( VirtualDevice& rVDev, int nExponent, int nMSBFactor,
                                             int nNumOrigTilesX, int nNumOrigTilesY,
                                             int nRemainderTilesX, int nRemainderTilesY,
                                             const Size& rTileSizePixel, const GraphicAttr* pAttr,
                                             sal_uLong nFlags, ImplTileInfo& rTileInfo )
{
    // gets loaded with our tile bitmap
    GraphicObject aTmpGraphic;

    // stores a flag that renders the zero'th tile position
    // (i.e. (0,0)+rCurrPos) only if we're at the bottom of the
    // recursion stack. All other position already have that tile
    // rendered, because the lower levels painted their generated tile
    // there.
    bool bNoFirstTileDraw( false );

    // what's left when we're done with our tile size
    const int nNewRemainderX( nRemainderTilesX % nMSBFactor );
    const int nNewRemainderY( nRemainderTilesY % nMSBFactor );

    // gets filled out from the recursive call with info of what's
    // been generated
    ImplTileInfo aTileInfo;

    // current output position while drawing
    Point aCurrPos;
    int nX, nY;

    // check for recursion's end condition: LSB place reached?
    if( nMSBFactor == 1 )
    {
        aTmpGraphic = *this;

        // set initial tile size -> orig size
        aTileInfo.aTileSizePixel = rTileSizePixel;
        aTileInfo.nTilesEmptyX = nNumOrigTilesX;
        aTileInfo.nTilesEmptyY = nNumOrigTilesY;
    }
    else if( ImplRenderTileRecursive( rVDev, nExponent, nMSBFactor/nExponent,
                                      nNumOrigTilesX, nNumOrigTilesY,
                                      nNewRemainderX, nNewRemainderY,
                                      rTileSizePixel, pAttr, nFlags, aTileInfo ) )
    {
        // extract generated tile -> see comment on the first loop below
        BitmapEx aTileBitmap( rVDev.GetBitmap( aTileInfo.aTileTopLeft, aTileInfo.aTileSizePixel ) );

        aTmpGraphic = GraphicObject( aTileBitmap );

        // fill stripes left over from upstream levels:
        //
        //  x0000
        //  0
        //  0
        //  0
        //  0
        //
        // where x denotes the place filled by our recursive predecessors

        // check whether we have to fill stripes here. Although not
        // obvious, there is one case where we can skip this step: if
        // the previous recursion level (the one who filled our
        // aTileInfo) had zero area to fill, then there are no white
        // stripes left, naturally. This happens if the digit
        // associated to that level has a zero, and can be checked via
        // aTileTopLeft==aNextTileTopLeft.
        if( aTileInfo.aTileTopLeft != aTileInfo.aNextTileTopLeft )
        {
            // now fill one row from aTileInfo.aNextTileTopLeft.X() all
            // the way to the right
            aCurrPos.X() = aTileInfo.aNextTileTopLeft.X();
            aCurrPos.Y() = aTileInfo.aTileTopLeft.Y();
            for( nX=0; nX < aTileInfo.nTilesEmptyX; nX += nMSBFactor )
            {
                if( !aTmpGraphic.Draw( &rVDev, aCurrPos, aTileInfo.aTileSizePixel, pAttr, nFlags ) )
                    return false;

                aCurrPos.X() += aTileInfo.aTileSizePixel.Width();
            }

#ifdef DBG_TEST
//          rVDev.SetFillColor( COL_WHITE );
            rVDev.SetFillColor();
            rVDev.SetLineColor( Color( 255 * nExponent / nMSBFactor, 255 - 255 * nExponent / nMSBFactor, 128 - 255 * nExponent / nMSBFactor ) );
            rVDev.DrawEllipse( Rectangle(aTileInfo.aNextTileTopLeft.X(), aTileInfo.aTileTopLeft.Y(),
                                         aTileInfo.aNextTileTopLeft.X() - 1 + (aTileInfo.nTilesEmptyX/nMSBFactor)*aTileInfo.aTileSizePixel.Width(),
                                         aTileInfo.aTileTopLeft.Y() + aTileInfo.aTileSizePixel.Height() - 1) );
#endif

            // now fill one column from aTileInfo.aNextTileTopLeft.Y() all
            // the way to the bottom
            aCurrPos.X() = aTileInfo.aTileTopLeft.X();
            aCurrPos.Y() = aTileInfo.aNextTileTopLeft.Y();
            for( nY=0; nY < aTileInfo.nTilesEmptyY; nY += nMSBFactor )
            {
                if( !aTmpGraphic.Draw( &rVDev, aCurrPos, aTileInfo.aTileSizePixel, pAttr, nFlags ) )
                    return false;

                aCurrPos.Y() += aTileInfo.aTileSizePixel.Height();
            }

#ifdef DBG_TEST
            rVDev.DrawEllipse( Rectangle(aTileInfo.aTileTopLeft.X(), aTileInfo.aNextTileTopLeft.Y(),
                                         aTileInfo.aTileTopLeft.X() + aTileInfo.aTileSizePixel.Width() - 1,
                                         aTileInfo.aNextTileTopLeft.Y() - 1 + (aTileInfo.nTilesEmptyY/nMSBFactor)*aTileInfo.aTileSizePixel.Height()) );
#endif
        }
        else
        {
            // Thought that aTileInfo.aNextTileTopLeft tile has always
            // been drawn already, but that's wrong: typically,
            // _parts_ of that tile have been drawn, since the
            // previous level generated the tile there. But when
            // aTileInfo.aNextTileTopLeft!=aTileInfo.aTileTopLeft, the
            // difference between these two values is missing in the
            // lower right corner of this first tile. So, can do that
            // only here.
            bNoFirstTileDraw = true;
        }
    }
    else
    {
        return false;
    }

    // calc number of original tiles in our drawing area without
    // remainder
    nRemainderTilesX -= nNewRemainderX;
    nRemainderTilesY -= nNewRemainderY;

    // fill tile info for calling method
    rTileInfo.aTileTopLeft     = aTileInfo.aNextTileTopLeft;
    rTileInfo.aNextTileTopLeft = Point( rTileInfo.aTileTopLeft.X() + rTileSizePixel.Width()*nRemainderTilesX,
                                        rTileInfo.aTileTopLeft.Y() + rTileSizePixel.Height()*nRemainderTilesY );
    rTileInfo.aTileSizePixel   = Size( rTileSizePixel.Width()*nMSBFactor*nExponent,
                                       rTileSizePixel.Height()*nMSBFactor*nExponent );
    rTileInfo.nTilesEmptyX     = aTileInfo.nTilesEmptyX - nRemainderTilesX;
    rTileInfo.nTilesEmptyY     = aTileInfo.nTilesEmptyY - nRemainderTilesY;

    // init output position
    aCurrPos = aTileInfo.aNextTileTopLeft;

    // fill our drawing area. Fill possibly more, to create the next
    // bigger tile size -> see bitmap extraction above. This does no
    // harm, since everything right or below our actual area is
    // overdrawn by our caller. Just in case we're in the last level,
    // we don't draw beyond the right or bottom border.
    for( nY=0; nY < aTileInfo.nTilesEmptyY && nY < nExponent*nMSBFactor; nY += nMSBFactor )
    {
        aCurrPos.X() = aTileInfo.aNextTileTopLeft.X();

        for( nX=0; nX < aTileInfo.nTilesEmptyX && nX < nExponent*nMSBFactor; nX += nMSBFactor )
        {
            if( bNoFirstTileDraw )
                bNoFirstTileDraw = false; // don't draw first tile position
            else if( !aTmpGraphic.Draw( &rVDev, aCurrPos, aTileInfo.aTileSizePixel, pAttr, nFlags ) )
                return false;

            aCurrPos.X() += aTileInfo.aTileSizePixel.Width();
        }

        aCurrPos.Y() += aTileInfo.aTileSizePixel.Height();
    }

#ifdef DBG_TEST
//  rVDev.SetFillColor( COL_WHITE );
    rVDev.SetFillColor();
    rVDev.SetLineColor( Color( 255 * nExponent / nMSBFactor, 255 - 255 * nExponent / nMSBFactor, 128 - 255 * nExponent / nMSBFactor ) );
    rVDev.DrawRect( Rectangle((rTileInfo.aTileTopLeft.X())*rTileSizePixel.Width(),
                              (rTileInfo.aTileTopLeft.Y())*rTileSizePixel.Height(),
                              (rTileInfo.aNextTileTopLeft.X())*rTileSizePixel.Width()-1,
                              (rTileInfo.aNextTileTopLeft.Y())*rTileSizePixel.Height()-1) );
#endif

    return true;
}

bool GraphicObject::ImplDrawTiled( OutputDevice* pOut, const Rectangle& rArea, const Size& rSizePixel,
                                   const Size& rOffset, const GraphicAttr* pAttr, sal_uLong nFlags, int nTileCacheSize1D )
{
    // how many tiles to generate per recursion step
    enum{ SubdivisionExponent=2 };

    const MapMode   aOutMapMode( pOut->GetMapMode() );
    const MapMode   aMapMode( aOutMapMode.GetMapUnit(), Point(), aOutMapMode.GetScaleX(), aOutMapMode.GetScaleY() );
    bool            bRet( false );

    // #i42643# Casting to Int64, to avoid integer overflow for
    // huge-DPI output devices
    if( GetGraphic().GetType() == GRAPHIC_BITMAP &&
        static_cast<sal_Int64>(rSizePixel.Width()) * rSizePixel.Height() <
        static_cast<sal_Int64>(nTileCacheSize1D)*nTileCacheSize1D )
    {
        // First combine very small bitmaps into a larger tile
        // ===================================================

        VirtualDevice   aVDev;
        const int       nNumTilesInCacheX( (nTileCacheSize1D + rSizePixel.Width()-1) / rSizePixel.Width() );
        const int       nNumTilesInCacheY( (nTileCacheSize1D + rSizePixel.Height()-1) / rSizePixel.Height() );

        aVDev.SetOutputSizePixel( Size( nNumTilesInCacheX*rSizePixel.Width(),
                                        nNumTilesInCacheY*rSizePixel.Height() ) );
        aVDev.SetMapMode( aMapMode );

        // draw bitmap content
        if( ImplRenderTempTile( aVDev, SubdivisionExponent, nNumTilesInCacheX,
                                nNumTilesInCacheY, rSizePixel, pAttr, nFlags ) )
        {
            BitmapEx aTileBitmap( aVDev.GetBitmap( Point(0,0), aVDev.GetOutputSize() ) );

            // draw alpha content, if any
            if( IsTransparent() )
            {
                GraphicObject aAlphaGraphic;

                if( GetGraphic().IsAlpha() )
                    aAlphaGraphic.SetGraphic( GetGraphic().GetBitmapEx().GetAlpha().GetBitmap() );
                else
                    aAlphaGraphic.SetGraphic( GetGraphic().GetBitmapEx().GetMask() );

                if( aAlphaGraphic.ImplRenderTempTile( aVDev, SubdivisionExponent, nNumTilesInCacheX,
                                                      nNumTilesInCacheY, rSizePixel, pAttr, nFlags ) )
                {
                    // Combine bitmap and alpha/mask
                    if( GetGraphic().IsAlpha() )
                        aTileBitmap = BitmapEx( aTileBitmap.GetBitmap(),
                                                AlphaMask( aVDev.GetBitmap( Point(0,0), aVDev.GetOutputSize() ) ) );
                    else
                        aTileBitmap = BitmapEx( aTileBitmap.GetBitmap(),
                                                aVDev.GetBitmap( Point(0,0), aVDev.GetOutputSize() ).CreateMask( Color(COL_WHITE) ) );
                }
            }

            // paint generated tile
            GraphicObject aTmpGraphic( aTileBitmap );
            bRet = aTmpGraphic.ImplDrawTiled( pOut, rArea,
                                              aTileBitmap.GetSizePixel(),
                                              rOffset, pAttr, nFlags, nTileCacheSize1D );
        }
    }
    else
    {
        const Size      aOutOffset( pOut->LogicToPixel( rOffset, aOutMapMode ) );
        const Rectangle aOutArea( pOut->LogicToPixel( rArea, aOutMapMode ) );

        // number of invisible (because out-of-area) tiles
        int nInvisibleTilesX;
        int nInvisibleTilesY;

        // round towards -infty for negative offset
        if( aOutOffset.Width() < 0 )
            nInvisibleTilesX = (aOutOffset.Width() - rSizePixel.Width() + 1) / rSizePixel.Width();
        else
            nInvisibleTilesX = aOutOffset.Width() / rSizePixel.Width();

        // round towards -infty for negative offset
        if( aOutOffset.Height() < 0 )
            nInvisibleTilesY = (aOutOffset.Height() - rSizePixel.Height() + 1) / rSizePixel.Height();
        else
            nInvisibleTilesY = aOutOffset.Height() / rSizePixel.Height();

        // origin from where to 'virtually' start drawing in pixel
        const Point aOutOrigin( pOut->LogicToPixel( Point( rArea.Left() - rOffset.Width(),
                                                           rArea.Top() - rOffset.Height() ) ) );
        // position in pixel from where to really start output
        const Point aOutStart( aOutOrigin.X() + nInvisibleTilesX*rSizePixel.Width(),
                               aOutOrigin.Y() + nInvisibleTilesY*rSizePixel.Height() );

        pOut->Push( PUSH_CLIPREGION );
        pOut->IntersectClipRegion( rArea );

        // Paint all tiles
        // ===============

        bRet = ImplDrawTiled( *pOut, aOutStart,
                              (aOutArea.GetWidth() + aOutArea.Left() - aOutStart.X() + rSizePixel.Width() - 1) / rSizePixel.Width(),
                              (aOutArea.GetHeight() + aOutArea.Top() - aOutStart.Y() + rSizePixel.Height() - 1) / rSizePixel.Height(),
                              rSizePixel, pAttr, nFlags );

        pOut->Pop();
    }

    return bRet;
}

bool GraphicObject::ImplDrawTiled( OutputDevice& rOut, const Point& rPosPixel,
                                   int nNumTilesX, int nNumTilesY,
                                   const Size& rTileSizePixel, const GraphicAttr* pAttr, sal_uLong nFlags )
{
    Point   aCurrPos( rPosPixel );
    Size    aTileSizeLogic( rOut.PixelToLogic( rTileSizePixel ) );
    int     nX, nY;

    // #107607# Use logical coordinates for metafile playing, too
    bool    bDrawInPixel( rOut.GetConnectMetaFile() == NULL && GRAPHIC_BITMAP == GetType() );
    bool    bRet = false;

    // #105229# Switch off mapping (converting to logic and back to
    // pixel might cause roundoff errors)
    bool bOldMap( rOut.IsMapModeEnabled() );

    if( bDrawInPixel )
        rOut.EnableMapMode( sal_False );

    for( nY=0; nY < nNumTilesY; ++nY )
    {
        aCurrPos.X() = rPosPixel.X();

        for( nX=0; nX < nNumTilesX; ++nX )
        {
            // #105229# work with pixel coordinates here, mapping is disabled!
            // #104004# don't disable mapping for metafile recordings
            // #108412# don't quit the loop if one draw fails

            // update return value. This method should return true, if
            // at least one of the looped Draws succeeded.
            bRet |= Draw( &rOut,
                          bDrawInPixel ? aCurrPos : rOut.PixelToLogic( aCurrPos ),
                          bDrawInPixel ? rTileSizePixel : aTileSizeLogic,
                          pAttr, nFlags );

            aCurrPos.X() += rTileSizePixel.Width();
        }

        aCurrPos.Y() += rTileSizePixel.Height();
    }

    if( bDrawInPixel )
        rOut.EnableMapMode( bOldMap );

    return bRet;
}

void GraphicObject::ImplTransformBitmap( BitmapEx&          rBmpEx,
                                         const GraphicAttr& rAttr,
                                         const Size&        rCropLeftTop,
                                         const Size&        rCropRightBottom,
                                         const Rectangle&   rCropRect,
                                         const Size&        rDstSize,
                                         sal_Bool               bEnlarge ) const
{
    // #107947# Extracted from svdograf.cxx

    // #104115# Crop the bitmap
    if( rAttr.IsCropped() )
    {
        rBmpEx.Crop( rCropRect );

        // #104115# Negative crop sizes mean: enlarge bitmap and pad
        if( bEnlarge && (
            rCropLeftTop.Width() < 0 ||
            rCropLeftTop.Height() < 0 ||
            rCropRightBottom.Width() < 0 ||
            rCropRightBottom.Height() < 0 ) )
        {
            Size aBmpSize( rBmpEx.GetSizePixel() );
            sal_Int32 nPadLeft( rCropLeftTop.Width() < 0 ? -rCropLeftTop.Width() : 0 );
            sal_Int32 nPadTop( rCropLeftTop.Height() < 0 ? -rCropLeftTop.Height() : 0 );
            sal_Int32 nPadTotalWidth( aBmpSize.Width() + nPadLeft + (rCropRightBottom.Width() < 0 ? -rCropRightBottom.Width() : 0) );
            sal_Int32 nPadTotalHeight( aBmpSize.Height() + nPadTop + (rCropRightBottom.Height() < 0 ? -rCropRightBottom.Height() : 0) );

            BitmapEx aBmpEx2;

            if( rBmpEx.IsTransparent() )
            {
                if( rBmpEx.IsAlpha() )
                    aBmpEx2 = BitmapEx( rBmpEx.GetBitmap(), rBmpEx.GetAlpha() );
                else
                    aBmpEx2 = BitmapEx( rBmpEx.GetBitmap(), rBmpEx.GetMask() );
            }
            else
            {
                // #104115# Generate mask bitmap and init to zero
                Bitmap aMask( aBmpSize, 1 );
                aMask.Erase( Color(0,0,0) );

                // #104115# Always generate transparent bitmap, we need the border transparent
                aBmpEx2 = BitmapEx( rBmpEx.GetBitmap(), aMask );

                // #104115# Add opaque mask to source bitmap, otherwise the destination remains transparent
                rBmpEx = aBmpEx2;
            }

            aBmpEx2.SetSizePixel( Size(nPadTotalWidth, nPadTotalHeight) );
            aBmpEx2.Erase( Color(0xFF,0,0,0) );
            aBmpEx2.CopyPixel( Rectangle( Point(nPadLeft, nPadTop), aBmpSize ), Rectangle( Point(0, 0), aBmpSize ), &rBmpEx );
            rBmpEx = aBmpEx2;
        }
    }

    const Size  aSizePixel( rBmpEx.GetSizePixel() );

    if( rAttr.GetRotation() != 0 && !IsAnimated() )
    {
        if( aSizePixel.Width() && aSizePixel.Height() && rDstSize.Width() && rDstSize.Height() )
        {
            double fSrcWH = (double) aSizePixel.Width() / aSizePixel.Height();
            double fDstWH = (double) rDstSize.Width() / rDstSize.Height();
            double fScaleX = 1.0, fScaleY = 1.0;

            // always choose scaling to shrink bitmap
            if( fSrcWH < fDstWH )
                fScaleY = aSizePixel.Width() / ( fDstWH * aSizePixel.Height() );
            else
                fScaleX = fDstWH * aSizePixel.Height() / aSizePixel.Width();

            rBmpEx.Scale( fScaleX, fScaleY );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
