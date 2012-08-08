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


#include <salhelper/timer.hxx>
#include <tools/debug.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <tools/poly.hxx>
#include <rtl/strbuf.hxx>
#include "grfcache.hxx"

#include <memory>

// -----------
// - Defines -
// -----------

#define RELEASE_TIMEOUT 10000
#define MAX_BMP_EXTENT  4096

// -----------
// - statics -
// -----------

static const char aHexData[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

// -------------
// - GraphicID -
// -------------

class GraphicID
{
private:

    sal_uInt32  mnID1;
    sal_uInt32  mnID2;
    sal_uInt32  mnID3;
    sal_uInt32  mnID4;

                GraphicID();

public:


                GraphicID( const GraphicObject& rObj );
                ~GraphicID() {}

    sal_Bool        operator==( const GraphicID& rID ) const
                {
                    return( rID.mnID1 == mnID1 && rID.mnID2 == mnID2 &&
                            rID.mnID3 == mnID3 && rID.mnID4 == mnID4 );
                }

    rtl::OString GetIDString() const;
    sal_Bool        IsEmpty() const { return( 0 == mnID4 ); }
};

// -----------------------------------------------------------------------------

GraphicID::GraphicID( const GraphicObject& rObj )
{
    const Graphic& rGraphic = rObj.GetGraphic();

    mnID1 = ( (sal_uLong) rGraphic.GetType() ) << 28;

    switch( rGraphic.GetType() )
    {
        case( GRAPHIC_BITMAP ):
        {
            if( rGraphic.IsAnimated() )
            {
                const Animation aAnimation( rGraphic.GetAnimation() );

                mnID1 |= ( aAnimation.Count() & 0x0fffffff );
                mnID2 = aAnimation.GetDisplaySizePixel().Width();
                mnID3 = aAnimation.GetDisplaySizePixel().Height();
                mnID4 = rGraphic.GetChecksum();
            }
            else
            {
                const BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                mnID1 |= ( ( ( (sal_uLong) aBmpEx.GetTransparentType() << 8 ) | ( aBmpEx.IsAlpha() ? 1 : 0 ) ) & 0x0fffffff );
                mnID2 = aBmpEx.GetSizePixel().Width();
                mnID3 = aBmpEx.GetSizePixel().Height();
                mnID4 = rGraphic.GetChecksum();
            }
        }
        break;

        case( GRAPHIC_GDIMETAFILE ):
        {
            const GDIMetaFile& rMtf = rGraphic.GetGDIMetaFile();

            mnID1 |= ( rMtf.GetActionSize() & 0x0fffffff );
            mnID2 = rMtf.GetPrefSize().Width();
            mnID3 = rMtf.GetPrefSize().Height();
            mnID4 = rGraphic.GetChecksum();
        }
        break;

        default:
            mnID2 = mnID3 = mnID4 = 0;
        break;
    }
}

// -----------------------------------------------------------------------------

rtl::OString GraphicID::GetIDString() const
{
    rtl::OStringBuffer aHexStr;
    sal_Int32 nShift, nIndex = 0;
    aHexStr.setLength(32);

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        aHexStr[nIndex++] = aHexData[ ( mnID1 >> (sal_uInt32) nShift ) & 0xf ];

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        aHexStr[nIndex++] = aHexData[ ( mnID2 >> (sal_uInt32) nShift ) & 0xf ];

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        aHexStr[nIndex++] = aHexData[ ( mnID3 >> (sal_uInt32) nShift ) & 0xf ];

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        aHexStr[nIndex++] = aHexData[ ( mnID4 >> (sal_uInt32) nShift ) & 0xf ];

    return aHexStr.makeStringAndClear();
}

// ---------------------
// - GraphicCacheEntry -
// ---------------------

class GraphicCacheEntry
{
private:

    GraphicObjectList_impl  maGraphicObjectList;

    GraphicID           maID;
    GfxLink             maGfxLink;
    BitmapEx*           mpBmpEx;
    GDIMetaFile*        mpMtf;
    Animation*          mpAnimation;
    bool                mbSwappedAll;

    bool                ImplInit( const GraphicObject& rObj );
    void                ImplFillSubstitute( Graphic& rSubstitute );

public:

                        GraphicCacheEntry( const GraphicObject& rObj );
                        ~GraphicCacheEntry();

    const GraphicID&    GetID() const { return maID; }

    void                AddGraphicObjectReference( const GraphicObject& rObj, Graphic& rSubstitute );
    bool                ReleaseGraphicObjectReference( const GraphicObject& rObj );
    size_t              GetGraphicObjectReferenceCount() { return maGraphicObjectList.size(); }
    bool                HasGraphicObjectReference( const GraphicObject& rObj );

    void                TryToSwapIn();
    void                GraphicObjectWasSwappedOut( const GraphicObject& rObj );
    bool                FillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute );
    void                GraphicObjectWasSwappedIn( const GraphicObject& rObj );
};

// -----------------------------------------------------------------------------

GraphicCacheEntry::GraphicCacheEntry( const GraphicObject& rObj ) :
    maID            ( rObj ),
    mpBmpEx         ( NULL ),
    mpMtf           ( NULL ),
    mpAnimation     ( NULL ),
    mbSwappedAll    ( !ImplInit( rObj ) )
{
    maGraphicObjectList.push_back( (GraphicObject*)&rObj );
}

// -----------------------------------------------------------------------------

GraphicCacheEntry::~GraphicCacheEntry()
{
    DBG_ASSERT(
        maGraphicObjectList.empty(),
        "GraphicCacheEntry::~GraphicCacheEntry(): Not all GraphicObjects are removed from this entry"
    );

    delete mpBmpEx;
    delete mpMtf;
    delete mpAnimation;
}

// -----------------------------------------------------------------------------

bool GraphicCacheEntry::ImplInit( const GraphicObject& rObj )
{
    bool bRet = false;

    if( !rObj.IsSwappedOut() )
    {
        const Graphic& rGraphic = rObj.GetGraphic();

        if( mpBmpEx )
            delete mpBmpEx, mpBmpEx = NULL;

        if( mpMtf )
            delete mpMtf, mpMtf = NULL;

        if( mpAnimation )
            delete mpAnimation, mpAnimation = NULL;

        switch( rGraphic.GetType() )
        {
            case( GRAPHIC_BITMAP ):
            {
                if( rGraphic.IsAnimated() )
                    mpAnimation = new Animation( rGraphic.GetAnimation() );
                else
                    mpBmpEx = new BitmapEx( rGraphic.GetBitmapEx() );
            }
            break;

            case( GRAPHIC_GDIMETAFILE ):
            {
                mpMtf = new GDIMetaFile( rGraphic.GetGDIMetaFile() );
            }
            break;

            default:
                DBG_ASSERT( GetID().IsEmpty(), "GraphicCacheEntry::ImplInit: Could not initialize graphic! (=>KA)" );
            break;
        }

        if( rGraphic.IsLink() )
            maGfxLink = ( (Graphic&) rGraphic ).GetLink();
        else
            maGfxLink = GfxLink();

        bRet = true;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void GraphicCacheEntry::ImplFillSubstitute( Graphic& rSubstitute )
{
    // create substitute for graphic;
    const Size          aPrefSize( rSubstitute.GetPrefSize() );
    const MapMode       aPrefMapMode( rSubstitute.GetPrefMapMode() );
    const Link          aAnimationNotifyHdl( rSubstitute.GetAnimationNotifyHdl() );
    const String        aDocFileName( rSubstitute.GetDocFileName() );
    const sal_uLong     nDocFilePos = rSubstitute.GetDocFilePos();
    const GraphicType   eOldType = rSubstitute.GetType();
    const bool          bDefaultType = ( rSubstitute.GetType() == GRAPHIC_DEFAULT );

    if( rSubstitute.IsLink() && ( GFX_LINK_TYPE_NONE == maGfxLink.GetType() ) )
        maGfxLink = rSubstitute.GetLink();

    if( mpBmpEx )
        rSubstitute = *mpBmpEx;
    else if( mpAnimation )
        rSubstitute = *mpAnimation;
    else if( mpMtf )
        rSubstitute = *mpMtf;
    else
        rSubstitute.Clear();

    if( eOldType != GRAPHIC_NONE )
    {
        rSubstitute.SetPrefSize( aPrefSize );
        rSubstitute.SetPrefMapMode( aPrefMapMode );
        rSubstitute.SetAnimationNotifyHdl( aAnimationNotifyHdl );
        rSubstitute.SetDocFileName( aDocFileName, nDocFilePos );
    }

    if( GFX_LINK_TYPE_NONE != maGfxLink.GetType() )
        rSubstitute.SetLink( maGfxLink );

    if( bDefaultType )
        rSubstitute.SetDefaultType();
}

// -----------------------------------------------------------------------------

void GraphicCacheEntry::AddGraphicObjectReference( const GraphicObject& rObj, Graphic& rSubstitute )
{
    if( mbSwappedAll )
        mbSwappedAll = !ImplInit( rObj );

    ImplFillSubstitute( rSubstitute );
    maGraphicObjectList.push_back( (GraphicObject*) &rObj );
}

// -----------------------------------------------------------------------------

bool GraphicCacheEntry::ReleaseGraphicObjectReference( const GraphicObject& rObj )
{
    for(
        GraphicObjectList_impl::iterator it = maGraphicObjectList.begin();
        it != maGraphicObjectList.end();
        ++it
    ) {
        if( &rObj == *it )
        {
            maGraphicObjectList.erase( it );
            return true;
        }
    }

    return false;
}

// -----------------------------------------------------------------------------

bool GraphicCacheEntry::HasGraphicObjectReference( const GraphicObject& rObj )
{
    bool bRet = false;

    for( size_t i = 0, n = maGraphicObjectList.size(); ( i < n ) && !bRet; ++i )
        if( &rObj == maGraphicObjectList[ i ] )
            bRet = true;

    return bRet;
}

// -----------------------------------------------------------------------------

void GraphicCacheEntry::TryToSwapIn()
{
    if( mbSwappedAll && !maGraphicObjectList.empty() )
        maGraphicObjectList.front()->FireSwapInRequest();
}

// -----------------------------------------------------------------------------

void GraphicCacheEntry::GraphicObjectWasSwappedOut( const GraphicObject& /*rObj*/ )
{
    mbSwappedAll = true;

    for( size_t i = 0, n = maGraphicObjectList.size(); ( i < n ) && mbSwappedAll; ++i )
        if( !maGraphicObjectList[ i ]->IsSwappedOut() )
            mbSwappedAll = false;

    if( mbSwappedAll )
    {
        delete mpBmpEx, mpBmpEx = NULL;
        mpMtf = NULL; // No need to delete it as it has already been dereferenced
        delete mpAnimation, mpAnimation = NULL;
    }
}

// -----------------------------------------------------------------------------

bool GraphicCacheEntry::FillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute )
{
    bool bRet = false;

    if( !mbSwappedAll && rObj.IsSwappedOut() )
    {
        ImplFillSubstitute( rSubstitute );
        bRet = true;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void GraphicCacheEntry::GraphicObjectWasSwappedIn( const GraphicObject& rObj )
{
    if( mbSwappedAll )
        mbSwappedAll = !ImplInit( rObj );
}

// ----------------------------
// - GraphicDisplayCacheEntry -
// ----------------------------

class GraphicDisplayCacheEntry
{
private:

    ::salhelper::TTimeValue     maReleaseTime;
    const GraphicCacheEntry*    mpRefCacheEntry;
    GDIMetaFile*                mpMtf;
    BitmapEx*                   mpBmpEx;
    GraphicAttr                 maAttr;
    Size                        maOutSizePix;
    sal_uLong                       mnCacheSize;
    sal_uLong                       mnOutDevDrawMode;
    sal_uInt16                      mnOutDevBitCount;

    static bool IsCacheableAsBitmap( const GDIMetaFile& rMtf, OutputDevice* pOut, const Size& rSz );

public:

    static sal_uLong                GetNeededSize( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                               const GraphicObject& rObj, const GraphicAttr& rAttr );

public:

                                GraphicDisplayCacheEntry( const GraphicCacheEntry* pRefCacheEntry,
                                                          OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                                          const BitmapEx& rBmpEx ) :
                                    mpRefCacheEntry( pRefCacheEntry ),
                                    mpMtf( NULL ), mpBmpEx( new BitmapEx( rBmpEx ) ),
                                    maAttr( rAttr ), maOutSizePix( pOut->LogicToPixel( rSz ) ),
                                    mnCacheSize( GetNeededSize( pOut, rPt, rSz, rObj, rAttr ) ),
                                    mnOutDevDrawMode( pOut->GetDrawMode() ),
                                    mnOutDevBitCount( pOut->GetBitCount() )
                                    {
                                    }

                                GraphicDisplayCacheEntry( const GraphicCacheEntry* pRefCacheEntry,
                                                          OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                                          const GDIMetaFile& rMtf ) :
                                    mpRefCacheEntry( pRefCacheEntry ),
                                    mpMtf( new GDIMetaFile( rMtf ) ), mpBmpEx( NULL ),
                                    maAttr( rAttr ), maOutSizePix( pOut->LogicToPixel( rSz ) ),
                                    mnCacheSize( GetNeededSize( pOut, rPt, rSz, rObj, rAttr ) ),
                                    mnOutDevDrawMode( pOut->GetDrawMode() ),
                                    mnOutDevBitCount( pOut->GetBitCount() )
                                    {
                                    }


                                ~GraphicDisplayCacheEntry();

    const GraphicAttr&          GetAttr() const { return maAttr; }
    const Size&                 GetOutputSizePixel() const { return maOutSizePix; }
    sal_uLong                   GetCacheSize() const { return mnCacheSize; }
    const GraphicCacheEntry*    GetReferencedCacheEntry() const { return mpRefCacheEntry; }
    sal_uLong                   GetOutDevDrawMode() const { return mnOutDevDrawMode; }
    sal_uInt16              GetOutDevBitCount() const { return mnOutDevBitCount; }

    void                        SetReleaseTime( const ::salhelper::TTimeValue& rReleaseTime ) { maReleaseTime = rReleaseTime; }
    const ::salhelper::TTimeValue&    GetReleaseTime() const { return maReleaseTime; }

    sal_Bool                        Matches( OutputDevice* pOut, const Point& /*rPtPixel*/, const Size& rSzPixel,
                                         const GraphicCacheEntry* pCacheEntry, const GraphicAttr& rAttr ) const
                                {
                                    // #i46805# Additional match
                                    // criteria: outdev draw mode and
                                    // bit count. One cannot reuse
                                    // this cache object, if it's
                                    // e.g. generated for
                                    // DRAWMODE_GRAYBITMAP.
                                    return( ( pCacheEntry == mpRefCacheEntry ) &&
                                            ( maAttr == rAttr ) &&
                                            ( ( maOutSizePix == rSzPixel ) || ( !maOutSizePix.Width() && !maOutSizePix.Height() ) ) &&
                                            ( pOut->GetBitCount() == mnOutDevBitCount ) &&
                                            ( pOut->GetDrawMode() == mnOutDevDrawMode ) );
                                }

    void                        Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz ) const;
};

// -----------------------------------------------------------------------------

// This whole function is based on checkMetadataBitmap() from grfmgr2.cxx, see that one for details.
// If you do changes here, change the original function too.
static void checkMetadataBitmap( const BitmapEx& rBmpEx,
                                 Point    /*rSrcPoint*/,
                                 Size     rSrcSize,
                                 const Point&    rDestPoint,
                                 const Size&     rDestSize,
                                 const Size&     rRefSize,
                                 bool&           o_rbNonBitmapActionEncountered )
{
    if( rSrcSize == Size())
        rSrcSize = rBmpEx.GetSizePixel();

    if( rDestPoint != Point( 0, 0 ))
    {
        o_rbNonBitmapActionEncountered = true;
        return;
    }
    if( rDestSize != rRefSize )
    {    if( rBmpEx.GetSizePixel().Width() > 100 && rBmpEx.GetSizePixel().Height() > 100
            && abs( rDestSize.Width() - rRefSize.Width()) < 5
            && abs( rDestSize.Height() - rRefSize.Height()) < 5 )
            ; // ok, assume it's close enough
        else
        {  // fall back to mtf rendering
            o_rbNonBitmapActionEncountered = true;
            return;
        }
    }
}

// This function is based on GraphicManager::ImplCreateOutput(), in fact it mostly copies
// it, the difference is that this one does not create anything, it only checks if
// ImplCreateOutput() would use the optimization of using the single bitmap.
// If you do changes here, change the original function too.
bool GraphicDisplayCacheEntry::IsCacheableAsBitmap( const GDIMetaFile& rMtf,
    OutputDevice* pOut, const Size& rSz )
{
    const Size aNewSize( rMtf.GetPrefSize() );
    GDIMetaFile rOutMtf = rMtf;

    // Count bitmap actions, and flag actions that paint, but
    // are no bitmaps.
    sal_Int32   nNumBitmaps(0);
    bool        bNonBitmapActionEncountered(false);
    if( aNewSize.Width() && aNewSize.Height() && rSz.Width() && rSz.Height() )
    {
        const MapMode rPrefMapMode( rMtf.GetPrefMapMode() );
        const Size rSizePix( pOut->LogicToPixel( aNewSize, rPrefMapMode ) );

        sal_uInt32  nCurPos;
        MetaAction* pAct;
        for( nCurPos = 0, pAct = (MetaAction*)rOutMtf.FirstAction(); pAct;
             pAct = (MetaAction*)rOutMtf.NextAction(), nCurPos++ )
        {
            switch( pAct->GetType() )
            {
                case META_FONT_ACTION:
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

                        checkMetadataBitmap(
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

                        checkMetadataBitmap(
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

                        checkMetadataBitmap(        BitmapEx( pAction->GetBitmap() ),
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

                        checkMetadataBitmap(
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

                        checkMetadataBitmap(
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

                        checkMetadataBitmap( pAction->GetBitmapEx(),
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
        }
    }
    return nNumBitmaps == 1 && !bNonBitmapActionEncountered;
}

sal_uLong GraphicDisplayCacheEntry::GetNeededSize( OutputDevice* pOut, const Point& /*rPt*/, const Size& rSz,
                                               const GraphicObject& rObj, const GraphicAttr& rAttr )
{
    const Graphic&      rGraphic = rObj.GetGraphic();
    const GraphicType   eType = rGraphic.GetType();

    bool canCacheAsBitmap = false;
    if( GRAPHIC_BITMAP == eType )
        canCacheAsBitmap = true;
    else if( GRAPHIC_GDIMETAFILE == eType )
        canCacheAsBitmap = IsCacheableAsBitmap( rGraphic.GetGDIMetaFile(), pOut, rSz );
    else
        return 0;
    if( canCacheAsBitmap )
    {
        const Size aOutSizePix( pOut->LogicToPixel( rSz ) );
        const long nBitCount = pOut->GetBitCount();

        if( ( aOutSizePix.Width() > MAX_BMP_EXTENT ) ||
            ( aOutSizePix.Height() > MAX_BMP_EXTENT ) )
        {
            return ULONG_MAX;
        }
        else if( nBitCount )
        {
            sal_uLong nNeededSize = aOutSizePix.Width() * aOutSizePix.Height() * nBitCount / 8;
            if( rObj.IsTransparent() || ( rAttr.GetRotation() % 3600 ) )
                nNeededSize += nNeededSize / nBitCount;
            return nNeededSize;
        }
        else
        {
            OSL_FAIL( "GraphicDisplayCacheEntry::GetNeededSize(): pOut->GetBitCount() == 0" );
            return 256000;
        }
    }
    else
        return rGraphic.GetSizeBytes();
}

// -----------------------------------------------------------------------------

GraphicDisplayCacheEntry::~GraphicDisplayCacheEntry()
{
    if( mpMtf )
        delete mpMtf;

    if( mpBmpEx )
        delete mpBmpEx;
}

// -----------------------------------------------------------------------------

void GraphicDisplayCacheEntry::Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz ) const
{
    if( mpMtf )
        GraphicManager::ImplDraw( pOut, rPt, rSz, *mpMtf, maAttr );
    else if( mpBmpEx )
    {
        if( maAttr.IsRotated() )
        {
            Polygon aPoly( Rectangle( rPt, rSz ) );

            aPoly.Rotate( rPt, maAttr.GetRotation() % 3600 );
            const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
            pOut->DrawBitmapEx( aRotBoundRect.TopLeft(), aRotBoundRect.GetSize(), *mpBmpEx );
        }
        else
            pOut->DrawBitmapEx( rPt, rSz, *mpBmpEx );
    }
}

// -----------------------
// - GraphicCache -
// -----------------------

GraphicCache::GraphicCache( GraphicManager& rMgr, sal_uLong nDisplayCacheSize, sal_uLong nMaxObjDisplayCacheSize ) :
    mrMgr                   ( rMgr ),
    mnReleaseTimeoutSeconds ( 0UL ),
    mnMaxDisplaySize        ( nDisplayCacheSize ),
    mnMaxObjDisplaySize     ( nMaxObjDisplayCacheSize ),
    mnUsedDisplaySize       ( 0UL )
{
    maReleaseTimer.SetTimeoutHdl( LINK( this, GraphicCache, ReleaseTimeoutHdl ) );
    maReleaseTimer.SetTimeout( RELEASE_TIMEOUT );
    maReleaseTimer.Start();
}

// -----------------------------------------------------------------------------

GraphicCache::~GraphicCache()
{
    DBG_ASSERT( !maGraphicCache.size(), "GraphicCache::~GraphicCache(): there are some GraphicObjects in cache" );
    DBG_ASSERT( maDisplayCache.empty(), "GraphicCache::~GraphicCache(): there are some GraphicObjects in display cache" );
}

// -----------------------------------------------------------------------------

void GraphicCache::AddGraphicObject(
    const GraphicObject& rObj,
    Graphic& rSubstitute,
    const rtl::OString* pID,
    const GraphicObject* pCopyObj
)
{
    sal_Bool bInserted = sal_False;

    if(  !rObj.IsSwappedOut()
      && (  pID
         || (    pCopyObj
            && ( pCopyObj->GetType() != GRAPHIC_NONE )
            )
         || ( rObj.GetType() != GRAPHIC_NONE )
         )
      )
    {
        if( pCopyObj
          && !maGraphicCache.empty()
        )
        {
            GraphicCacheEntryList::iterator it = maGraphicCache.begin();
            while(  !bInserted
                 && ( it != maGraphicCache.end() )
                 )
            {
                if( (*it)->HasGraphicObjectReference( *pCopyObj ) )
                {
                    (*it)->AddGraphicObjectReference( rObj, rSubstitute );
                    bInserted = sal_True;
                }
                else
                {
                    ++it;
                }
            }
        }

        if( !bInserted )
        {
            GraphicCacheEntryList::iterator it = maGraphicCache.begin();
            ::std::auto_ptr< GraphicID > apID;

            if( !pID )
            {
                apID.reset( new GraphicID( rObj ) );
            }

            while(  !bInserted
                 && ( it != maGraphicCache.end() )
                 )
            {
                const GraphicID& rEntryID = (*it)->GetID();

                if( pID )
                {
                    if( rEntryID.GetIDString() == *pID )
                    {
                        (*it)->TryToSwapIn();

                        // since pEntry->TryToSwapIn can modify our current list, we have to
                        // iterate from beginning to add a reference to the appropriate
                        // CacheEntry object; after this, quickly jump out of the outer iteration
                        for( GraphicCacheEntryList::iterator jt = maGraphicCache.begin();
                             !bInserted && jt != maGraphicCache.end();
                             ++jt
                        )
                        {
                            const GraphicID& rID = (*jt)->GetID();

                            if( rID.GetIDString() == *pID )
                            {
                                (*jt)->AddGraphicObjectReference( rObj, rSubstitute );
                                bInserted = sal_True;
                            }
                        }

                        if( !bInserted )
                        {
                            maGraphicCache.push_back( new GraphicCacheEntry( rObj ) );
                            bInserted = sal_True;
                        }
                    }
                }
                else
                {
                    if( rEntryID == *apID )
                    {
                        (*it)->AddGraphicObjectReference( rObj, rSubstitute );
                        bInserted = sal_True;
                    }
                }

                if( !bInserted )
                    ++it;
            }
        }
    }

    if( !bInserted )
        maGraphicCache.push_back( new GraphicCacheEntry( rObj ) );
}

// -----------------------------------------------------------------------------

void GraphicCache::ReleaseGraphicObject( const GraphicObject& rObj )
{
    // Release cached object
    bool    bRemoved = false;
    GraphicCacheEntryList::iterator it = maGraphicCache.begin();
    while (!bRemoved && it != maGraphicCache.end())
    {
        bRemoved = (*it)->ReleaseGraphicObjectReference( rObj );

        if( bRemoved )
        {
            if( 0 == (*it)->GetGraphicObjectReferenceCount() )
            {
                // if graphic cache entry has no more references,
                // the corresponding display cache object can be removed
                GraphicDisplayCacheEntryList::iterator it2 = maDisplayCache.begin();
                while( it2 != maDisplayCache.end() )
                {
                    GraphicDisplayCacheEntry* pDisplayEntry = *it2;
                    if( pDisplayEntry->GetReferencedCacheEntry() == *it )
                    {
                        mnUsedDisplaySize -= pDisplayEntry->GetCacheSize();
                        it2 = maDisplayCache.erase( it2 );
                        delete pDisplayEntry;
                    }
                    else
                        ++it2;
                }

                // delete graphic cache entry
                delete *it;
                it = maGraphicCache.erase( it );
            }
        }
        else
            ++it;
    }

    DBG_ASSERT( bRemoved, "GraphicCache::ReleaseGraphicObject(...): GraphicObject not found in cache" );
}

// -----------------------------------------------------------------------------

void GraphicCache::GraphicObjectWasSwappedOut( const GraphicObject& rObj )
{
    // notify cache that rObj is swapped out (and can thus be pruned
    // from the cache)
    GraphicCacheEntry* pEntry = ImplGetCacheEntry( rObj );

    if( pEntry )
        pEntry->GraphicObjectWasSwappedOut( rObj );
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::FillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute )
{
    GraphicCacheEntry* pEntry = ImplGetCacheEntry( rObj );

    if( !pEntry )
        return sal_False;

    return pEntry->FillSwappedGraphicObject( rObj, rSubstitute );
}

// -----------------------------------------------------------------------------

void GraphicCache::GraphicObjectWasSwappedIn( const GraphicObject& rObj )
{
    GraphicCacheEntry* pEntry = ImplGetCacheEntry( rObj );

    if( pEntry )
    {
        if( pEntry->GetID().IsEmpty() )
        {
            ReleaseGraphicObject( rObj );
            AddGraphicObject( rObj, (Graphic&) rObj.GetGraphic(), NULL, NULL );
        }
        else
            pEntry->GraphicObjectWasSwappedIn( rObj );
    }
}

// -----------------------------------------------------------------------------

void GraphicCache::SetMaxDisplayCacheSize( sal_uLong nNewCacheSize )
{
    mnMaxDisplaySize = nNewCacheSize;

    if( GetMaxDisplayCacheSize() < GetUsedDisplayCacheSize() )
        ImplFreeDisplayCacheSpace( GetUsedDisplayCacheSize() - GetMaxDisplayCacheSize() );
}

// -----------------------------------------------------------------------------

void GraphicCache::SetMaxObjDisplayCacheSize( sal_uLong nNewMaxObjSize, sal_Bool bDestroyGreaterCached )
{
    const sal_Bool bDestroy = ( bDestroyGreaterCached && ( nNewMaxObjSize < mnMaxObjDisplaySize ) );

    mnMaxObjDisplaySize = Min( nNewMaxObjSize, mnMaxDisplaySize );

    if( bDestroy )
    {
        GraphicDisplayCacheEntryList::iterator it = maDisplayCache.begin();
        while( it != maDisplayCache.end() )
        {
            GraphicDisplayCacheEntry* pCacheObj = *it;
            if( pCacheObj->GetCacheSize() > mnMaxObjDisplaySize )
            {
                mnUsedDisplaySize -= pCacheObj->GetCacheSize();
                it = maDisplayCache.erase( it );
                delete pCacheObj;
            }
            else
                ++it;
        }
    }
}

// -----------------------------------------------------------------------------

void GraphicCache::SetCacheTimeout( sal_uLong nTimeoutSeconds )
{
    if( mnReleaseTimeoutSeconds != nTimeoutSeconds )
    {
        ::salhelper::TTimeValue           aReleaseTime;

        if( ( mnReleaseTimeoutSeconds = nTimeoutSeconds ) != 0 )
        {
            osl_getSystemTime( &aReleaseTime );
            aReleaseTime.addTime( ::salhelper::TTimeValue( nTimeoutSeconds, 0 ) );
        }

        for( GraphicDisplayCacheEntryList::const_iterator it = maDisplayCache.begin();
             it != maDisplayCache.end(); ++it )
        {
            (*it)->SetReleaseTime( aReleaseTime );
        }
    }
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::IsDisplayCacheable( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                       const GraphicObject& rObj, const GraphicAttr& rAttr ) const
{
    return( GraphicDisplayCacheEntry::GetNeededSize( pOut, rPt, rSz, rObj, rAttr ) <=
            GetMaxObjDisplayCacheSize() );
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::IsInDisplayCache( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                     const GraphicObject& rObj, const GraphicAttr& rAttr ) const
{
    const Point                 aPtPixel( pOut->LogicToPixel( rPt ) );
    const Size                  aSzPixel( pOut->LogicToPixel( rSz ) );
    const GraphicCacheEntry*    pCacheEntry = ( (GraphicCache*) this )->ImplGetCacheEntry( rObj );
    sal_Bool                        bFound = sal_False;

    if( pCacheEntry )
    {
        for( GraphicDisplayCacheEntryList::const_iterator it = maDisplayCache.begin();
             !bFound && ( it != maDisplayCache.end() ); ++it )
        {
            if( (*it)->Matches( pOut, aPtPixel, aSzPixel, pCacheEntry, rAttr ) )
                bFound = sal_True;
        }
    }

    return bFound;
}

// -----------------------------------------------------------------------------

rtl::OString GraphicCache::GetUniqueID( const GraphicObject& rObj ) const
{
    rtl::OString aRet;
    GraphicCacheEntry*  pEntry = ( (GraphicCache*) this )->ImplGetCacheEntry( rObj );

    // ensure that the entry is correctly initialized (it has to be read at least once)
    if( pEntry && pEntry->GetID().IsEmpty() )
        pEntry->TryToSwapIn();

    // do another call to ImplGetCacheEntry in case of modified entry list
    pEntry = ( (GraphicCache*) this )->ImplGetCacheEntry( rObj );

    if( pEntry )
        aRet = pEntry->GetID().GetIDString();

    return aRet;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::CreateDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                          const BitmapEx& rBmpEx )
{
    const sal_uLong nNeededSize = GraphicDisplayCacheEntry::GetNeededSize( pOut, rPt, rSz, rObj, rAttr );
    sal_Bool        bRet = sal_False;

    if( nNeededSize <= GetMaxObjDisplayCacheSize() )
    {
        if( nNeededSize > GetFreeDisplayCacheSize() )
            ImplFreeDisplayCacheSpace( nNeededSize - GetFreeDisplayCacheSize() );

        GraphicDisplayCacheEntry* pNewEntry = new GraphicDisplayCacheEntry( ImplGetCacheEntry( rObj ),
                                                                            pOut, rPt, rSz, rObj, rAttr, rBmpEx );

        if( GetCacheTimeout() )
        {
            ::salhelper::TTimeValue aReleaseTime;

            osl_getSystemTime( &aReleaseTime );
            aReleaseTime.addTime( ::salhelper::TTimeValue( GetCacheTimeout(), 0 ) );
            pNewEntry->SetReleaseTime( aReleaseTime );
        }

        maDisplayCache.push_back( pNewEntry );
        mnUsedDisplaySize += pNewEntry->GetCacheSize();
        bRet = sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::CreateDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                          const GDIMetaFile& rMtf )
{
    const sal_uLong nNeededSize = GraphicDisplayCacheEntry::GetNeededSize( pOut, rPt, rSz, rObj, rAttr );
    sal_Bool        bRet = sal_False;

    if( nNeededSize <= GetMaxObjDisplayCacheSize() )
    {
        if( nNeededSize > GetFreeDisplayCacheSize() )
            ImplFreeDisplayCacheSpace( nNeededSize - GetFreeDisplayCacheSize() );

        GraphicDisplayCacheEntry* pNewEntry = new GraphicDisplayCacheEntry( ImplGetCacheEntry( rObj ),
                                                                            pOut, rPt, rSz, rObj, rAttr, rMtf );

        if( GetCacheTimeout() )
        {
            ::salhelper::TTimeValue aReleaseTime;

            osl_getSystemTime( &aReleaseTime );
            aReleaseTime.addTime( ::salhelper::TTimeValue( GetCacheTimeout(), 0 ) );
            pNewEntry->SetReleaseTime( aReleaseTime );
        }

        maDisplayCache.push_back( pNewEntry );
        mnUsedDisplaySize += pNewEntry->GetCacheSize();
        bRet = sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::DrawDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                        const GraphicObject& rObj, const GraphicAttr& rAttr )
{
    const Point                 aPtPixel( pOut->LogicToPixel( rPt ) );
    const Size                  aSzPixel( pOut->LogicToPixel( rSz ) );
    const GraphicCacheEntry*    pCacheEntry = ImplGetCacheEntry( rObj );
    GraphicDisplayCacheEntry*   pDisplayCacheEntry = NULL;
    GraphicDisplayCacheEntryList::iterator it = maDisplayCache.begin();
    sal_Bool                    bRet = sal_False;

    while( !bRet && it != maDisplayCache.end() )
    {
        pDisplayCacheEntry = *it;
        if( pDisplayCacheEntry->Matches( pOut, aPtPixel, aSzPixel, pCacheEntry, rAttr ) )
        {
            ::salhelper::TTimeValue aReleaseTime;

            // put found object at last used position
            it = maDisplayCache.erase( it );
            maDisplayCache.push_back( pDisplayCacheEntry );

            if( GetCacheTimeout() )
            {
                osl_getSystemTime( &aReleaseTime );
                aReleaseTime.addTime( ::salhelper::TTimeValue( GetCacheTimeout(), 0 ) );
            }

            pDisplayCacheEntry->SetReleaseTime( aReleaseTime );
            bRet = sal_True;
        }
        else
            ++it;
    }

    if( bRet )
        pDisplayCacheEntry->Draw( pOut, rPt, rSz );

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::ImplFreeDisplayCacheSpace( sal_uLong nSizeToFree )
{
    sal_uLong nFreedSize = 0UL;

    if( nSizeToFree )
    {
        GraphicDisplayCacheEntryList::iterator it = maDisplayCache.begin();

        if( nSizeToFree > mnUsedDisplaySize )
            nSizeToFree = mnUsedDisplaySize;

        while( it != maDisplayCache.end() )
        {
            GraphicDisplayCacheEntry* pCacheObj = *it;

            nFreedSize += pCacheObj->GetCacheSize();
            mnUsedDisplaySize -= pCacheObj->GetCacheSize();
            it = maDisplayCache.erase( it );
            delete pCacheObj;

            if( nFreedSize >= nSizeToFree )
                break;
        }
    }

    return( nFreedSize >= nSizeToFree );
}

// -----------------------------------------------------------------------------

GraphicCacheEntry* GraphicCache::ImplGetCacheEntry( const GraphicObject& rObj )
{
    GraphicCacheEntry* pRet = NULL;

    for(
        GraphicCacheEntryList::iterator it = maGraphicCache.begin();
        !pRet && it != maGraphicCache.end();
        ++it
    ) {
        if( (*it)->HasGraphicObjectReference( rObj ) ) {
            pRet = *it;
        }
    }

    return pRet;
}

// -----------------------------------------------------------------------------

IMPL_LINK( GraphicCache, ReleaseTimeoutHdl, Timer*, pTimer )
{
    pTimer->Stop();

    ::salhelper::TTimeValue           aCurTime;
    GraphicDisplayCacheEntryList::iterator it = maDisplayCache.begin();

    osl_getSystemTime( &aCurTime );

    while( it != maDisplayCache.end() )
    {
        GraphicDisplayCacheEntry*   pDisplayEntry = *it;
        const ::salhelper::TTimeValue& rReleaseTime = pDisplayEntry->GetReleaseTime();

        if( !rReleaseTime.isEmpty() && ( rReleaseTime < aCurTime ) )
        {
            mnUsedDisplaySize -= pDisplayEntry->GetCacheSize();
            it = maDisplayCache.erase( it );
            delete pDisplayEntry;
        }
        else
            ++it;
    }

    pTimer->Start();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
