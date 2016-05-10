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

#include <sal/config.h>

#include <cstdlib>

#include <salhelper/timer.hxx>
#include <svtools/grfmgr.hxx>
#include <tools/debug.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <tools/poly.hxx>
#include <rtl/strbuf.hxx>
#include "grfcache.hxx"
#include <rtl/crc.h>
#include <memory>

#define MAX_BMP_EXTENT  4096

static const char aHexData[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

class GraphicID
{
private:

    sal_uInt32  mnID1;
    sal_uInt32  mnID2;
    sal_uInt32  mnID3;
    BitmapChecksum  mnID4;

public:


                explicit GraphicID( const GraphicObject& rObj );
                ~GraphicID() {}

    bool        operator==( const GraphicID& rID ) const
                {
                    return( rID.mnID1 == mnID1 && rID.mnID2 == mnID2 &&
                            rID.mnID3 == mnID3 && rID.mnID4 == mnID4 );
                }

    OString GetIDString() const;
    bool        IsEmpty() const { return( 0 == mnID4 ); }
};

GraphicID::GraphicID( const GraphicObject& rObj )
{
    const Graphic& rGraphic = rObj.GetGraphic();

    mnID1 = ( (sal_uLong) rGraphic.GetType() ) << 28;

    switch( rGraphic.GetType() )
    {
        case( GRAPHIC_BITMAP ):
        {
            if(rGraphic.getSvgData().get())
            {
                const SvgDataPtr& rSvgDataPtr = rGraphic.getSvgData();
                const basegfx::B2DRange& rRange = rSvgDataPtr->getRange();

                mnID1 |= rSvgDataPtr->getSvgDataArrayLength();
                mnID2 = basegfx::fround(rRange.getWidth());
                mnID3 = basegfx::fround(rRange.getHeight());
                mnID4 = vcl_get_checksum(0, rSvgDataPtr->getSvgDataArray().getConstArray(), rSvgDataPtr->getSvgDataArrayLength());
            }
            else if( rGraphic.IsAnimated() )
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

OString GraphicID::GetIDString() const
{
    OStringBuffer aHexStr;
    sal_Int32 nShift, nIndex = 0;
    aHexStr.setLength(24 + (2 * BITMAP_CHECKSUM_SIZE));

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        aHexStr[nIndex++] = aHexData[ ( mnID1 >> (sal_uInt32) nShift ) & 0xf ];

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        aHexStr[nIndex++] = aHexData[ ( mnID2 >> (sal_uInt32) nShift ) & 0xf ];

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        aHexStr[nIndex++] = aHexData[ ( mnID3 >> (sal_uInt32) nShift ) & 0xf ];

    for( nShift = ( 8 * BITMAP_CHECKSUM_SIZE ) - 4; nShift >= 0; nShift -= 4 )
        aHexStr[nIndex++] = aHexData[ ( mnID4 >> (sal_uInt32) nShift ) & 0xf ];

    return aHexStr.makeStringAndClear();
}

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

    // SvgData support
    SvgDataPtr          maSvgData;

    bool                ImplInit( const GraphicObject& rObj );
    void                ImplFillSubstitute( Graphic& rSubstitute );

public:

                        explicit GraphicCacheEntry( const GraphicObject& rObj );
                        ~GraphicCacheEntry();

    const GraphicID&    GetID() const { return maID; }

    void                AddGraphicObjectReference( const GraphicObject& rObj, Graphic& rSubstitute );
    bool                ReleaseGraphicObjectReference( const GraphicObject& rObj );
    size_t              GetGraphicObjectReferenceCount() { return maGraphicObjectList.size(); }
    bool                HasGraphicObjectReference( const GraphicObject& rObj );

    void                TryToSwapIn();
    void                GraphicObjectWasSwappedOut( const GraphicObject& rObj );
    void                GraphicObjectWasSwappedIn( const GraphicObject& rObj );
};

GraphicCacheEntry::GraphicCacheEntry( const GraphicObject& rObj ) :
    maID            ( rObj ),
    mpBmpEx         ( nullptr ),
    mpMtf           ( nullptr ),
    mpAnimation     ( nullptr ),
    mbSwappedAll    ( true )
{
    mbSwappedAll = !ImplInit( rObj );
    maGraphicObjectList.push_back( const_cast<GraphicObject*>(&rObj) );
}

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

bool GraphicCacheEntry::ImplInit( const GraphicObject& rObj )
{
    bool bRet = false;

    if( !rObj.IsSwappedOut() )
    {
        const Graphic& rGraphic = rObj.GetGraphic();

        if( mpBmpEx )
            delete mpBmpEx, mpBmpEx = nullptr;

        if( mpMtf )
            delete mpMtf, mpMtf = nullptr;

        if( mpAnimation )
            delete mpAnimation, mpAnimation = nullptr;

        switch( rGraphic.GetType() )
        {
            case( GRAPHIC_BITMAP ):
            {
                if(rGraphic.getSvgData().get())
                {
                    maSvgData = rGraphic.getSvgData();
                }
                else if( rGraphic.IsAnimated() )
                {
                    mpAnimation = new Animation( rGraphic.GetAnimation() );
                }
                else
                {
                    mpBmpEx = new BitmapEx( rGraphic.GetBitmapEx() );
                }
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

void GraphicCacheEntry::ImplFillSubstitute( Graphic& rSubstitute )
{
    // create substitute for graphic;
    const Size          aPrefSize( rSubstitute.GetPrefSize() );
    const MapMode       aPrefMapMode( rSubstitute.GetPrefMapMode() );
    const Link<Animation*,void> aAnimationNotifyHdl( rSubstitute.GetAnimationNotifyHdl() );
    const GraphicType   eOldType = rSubstitute.GetType();
    const bool          bDefaultType = ( rSubstitute.GetType() == GRAPHIC_DEFAULT );

    if( rSubstitute.IsLink() && ( GFX_LINK_TYPE_NONE == maGfxLink.GetType() ) )
        maGfxLink = rSubstitute.GetLink();

    if(maSvgData.get())
    {
        rSubstitute = maSvgData;
    }
    else if( mpBmpEx )
    {
        rSubstitute = *mpBmpEx;
    }
    else if( mpAnimation )
    {
        rSubstitute = *mpAnimation;
    }
    else if( mpMtf )
    {
        rSubstitute = *mpMtf;
    }
    else
    {
        rSubstitute.Clear();
    }

    if( eOldType != GRAPHIC_NONE )
    {
        rSubstitute.SetPrefSize( aPrefSize );
        rSubstitute.SetPrefMapMode( aPrefMapMode );
        rSubstitute.SetAnimationNotifyHdl( aAnimationNotifyHdl );
    }

    if( GFX_LINK_TYPE_NONE != maGfxLink.GetType() )
    {
        rSubstitute.SetLink( maGfxLink );
    }

    if( bDefaultType )
    {
        rSubstitute.SetDefaultType();
    }
}

void GraphicCacheEntry::AddGraphicObjectReference( const GraphicObject& rObj, Graphic& rSubstitute )
{
    if( mbSwappedAll )
        mbSwappedAll = !ImplInit( rObj );

    ImplFillSubstitute( rSubstitute );
    maGraphicObjectList.push_back( const_cast<GraphicObject*>(&rObj) );
}

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

bool GraphicCacheEntry::HasGraphicObjectReference( const GraphicObject& rObj )
{
    bool bRet = false;

    for( size_t i = 0, n = maGraphicObjectList.size(); ( i < n ) && !bRet; ++i )
        if( &rObj == maGraphicObjectList[ i ] )
            bRet = true;

    return bRet;
}

void GraphicCacheEntry::TryToSwapIn()
{
    if( mbSwappedAll && !maGraphicObjectList.empty() )
        maGraphicObjectList.front()->FireSwapInRequest();
}

void GraphicCacheEntry::GraphicObjectWasSwappedOut( const GraphicObject& /*rObj*/ )
{
    mbSwappedAll = true;

    for( size_t i = 0, n = maGraphicObjectList.size(); ( i < n ) && mbSwappedAll; ++i )
        if( !maGraphicObjectList[ i ]->IsSwappedOut() )
            mbSwappedAll = false;

    if( mbSwappedAll )
    {
        delete mpBmpEx, mpBmpEx = nullptr;
        delete mpMtf, mpMtf = nullptr;
        delete mpAnimation, mpAnimation = nullptr;

        // #119176# also reset SvgData
        maSvgData.reset();
    }
}

void GraphicCacheEntry::GraphicObjectWasSwappedIn( const GraphicObject& rObj )
{
    if( mbSwappedAll )
        mbSwappedAll = !ImplInit( rObj );
}

class GraphicDisplayCacheEntry
{
private:

    ::salhelper::TTimeValue     maReleaseTime;
    const GraphicCacheEntry*    mpRefCacheEntry;
    GDIMetaFile*                mpMtf;
    BitmapEx*                   mpBmpEx;
    GraphicAttr                 maAttr;
    Size                        maOutSizePix;
    sal_uLong                   mnCacheSize;
    DrawModeFlags               mnOutDevDrawMode;
    sal_uInt16                  mnOutDevBitCount;

    static bool IsCacheableAsBitmap( const GDIMetaFile& rMtf, OutputDevice* pOut, const Size& rSz );

    // Copy assignment is forbidden and not implemented.
    GraphicDisplayCacheEntry (const GraphicDisplayCacheEntry &) = delete;
    GraphicDisplayCacheEntry & operator= (const GraphicDisplayCacheEntry &) = delete;

public:

    static sal_uLong                GetNeededSize( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                               const GraphicObject& rObj, const GraphicAttr& rAttr );

public:

                                GraphicDisplayCacheEntry( const GraphicCacheEntry* pRefCacheEntry,
                                                          OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                                          const BitmapEx& rBmpEx ) :
                                    mpRefCacheEntry( pRefCacheEntry ),
                                    mpMtf( nullptr ), mpBmpEx( new BitmapEx( rBmpEx ) ),
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
                                    mpMtf( new GDIMetaFile( rMtf ) ), mpBmpEx( nullptr ),
                                    maAttr( rAttr ), maOutSizePix( pOut->LogicToPixel( rSz ) ),
                                    mnCacheSize( GetNeededSize( pOut, rPt, rSz, rObj, rAttr ) ),
                                    mnOutDevDrawMode( pOut->GetDrawMode() ),
                                    mnOutDevBitCount( pOut->GetBitCount() )
                                    {
                                    }


                                ~GraphicDisplayCacheEntry();

    sal_uLong                   GetCacheSize() const { return mnCacheSize; }
    const GraphicCacheEntry*    GetReferencedCacheEntry() const { return mpRefCacheEntry; }

    void                        SetReleaseTime( const ::salhelper::TTimeValue& rReleaseTime ) { maReleaseTime = rReleaseTime; }
    const ::salhelper::TTimeValue&    GetReleaseTime() const { return maReleaseTime; }

    bool                        Matches( OutputDevice* pOut, const Point& /*rPtPixel*/, const Size& rSzPixel,
                                         const GraphicCacheEntry* pCacheEntry, const GraphicAttr& rAttr ) const
                                {
                                    // #i46805# Additional match
                                    // criteria: outdev draw mode and
                                    // bit count. One cannot reuse
                                    // this cache object, if it's
                                    // e.g. generated for
                                    // DrawModeFlags::GrayBitmap.
                                    return( ( pCacheEntry == mpRefCacheEntry ) &&
                                            ( maAttr == rAttr ) &&
                                            ( ( maOutSizePix == rSzPixel ) || ( !maOutSizePix.Width() && !maOutSizePix.Height() ) ) &&
                                            ( pOut->GetBitCount() == mnOutDevBitCount ) &&
                                            ( pOut->GetDrawMode() == mnOutDevDrawMode ) );
                                }

    void                        Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz ) const;
};

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
             && std::abs( rDestSize.Width() - rRefSize.Width()) < 5
             && std::abs( rDestSize.Height() - rRefSize.Height()) < 5 )
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
        for( nCurPos = 0, pAct = rOutMtf.FirstAction(); pAct;
             pAct = rOutMtf.NextAction(), nCurPos++ )
        {
            switch( pAct->GetType() )
            {
                case MetaActionType::FONT:
                    // FALLTHROUGH intended
                case MetaActionType::NONE:
                    // FALLTHROUGH intended

                    // OutDev state changes (which don't affect bitmap
                    // output)
                case MetaActionType::LINECOLOR:
                    // FALLTHROUGH intended
                case MetaActionType::FILLCOLOR:
                    // FALLTHROUGH intended
                case MetaActionType::TEXTCOLOR:
                    // FALLTHROUGH intended
                case MetaActionType::TEXTFILLCOLOR:
                    // FALLTHROUGH intended
                case MetaActionType::TEXTALIGN:
                    // FALLTHROUGH intended
                case MetaActionType::TEXTLINECOLOR:
                    // FALLTHROUGH intended
                case MetaActionType::TEXTLINE:
                    // FALLTHROUGH intended
                case MetaActionType::PUSH:
                    // FALLTHROUGH intended
                case MetaActionType::POP:
                    // FALLTHROUGH intended
                case MetaActionType::LAYOUTMODE:
                    // FALLTHROUGH intended
                case MetaActionType::TEXTLANGUAGE:
                    // FALLTHROUGH intended
                case MetaActionType::COMMENT:
                    break;

                    // bitmap output methods
                case MetaActionType::BMP:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpAction* pAction = static_cast<MetaBmpAction*>(pAct);

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

                case MetaActionType::BMPSCALE:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpScaleAction* pAction = static_cast<MetaBmpScaleAction*>(pAct);

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

                case MetaActionType::BMPSCALEPART:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpScalePartAction* pAction = static_cast<MetaBmpScalePartAction*>(pAct);

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

                case MetaActionType::BMPEX:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpExAction* pAction = static_cast<MetaBmpExAction*>(pAct);

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

                case MetaActionType::BMPEXSCALE:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpExScaleAction* pAction = static_cast<MetaBmpExScaleAction*>(pAct);

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

                case MetaActionType::BMPEXSCALEPART:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpExScalePartAction* pAction = static_cast<MetaBmpExScalePartAction*>(pAct);

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
                case MetaActionType::RASTEROP:
                    if( static_cast<MetaRasterOpAction*>(pAct)->GetRasterOp() == ROP_OVERPAINT )
                        break;
                    SAL_FALLTHROUGH;
                case MetaActionType::PIXEL:
                    SAL_FALLTHROUGH;
                case MetaActionType::POINT:
                    SAL_FALLTHROUGH;
                case MetaActionType::LINE:
                    SAL_FALLTHROUGH;
                case MetaActionType::RECT:
                    SAL_FALLTHROUGH;
                case MetaActionType::ROUNDRECT:
                    SAL_FALLTHROUGH;
                case MetaActionType::ELLIPSE:
                    SAL_FALLTHROUGH;
                case MetaActionType::ARC:
                    SAL_FALLTHROUGH;
                case MetaActionType::PIE:
                    SAL_FALLTHROUGH;
                case MetaActionType::CHORD:
                    SAL_FALLTHROUGH;
                case MetaActionType::POLYLINE:
                    SAL_FALLTHROUGH;
                case MetaActionType::POLYGON:
                    SAL_FALLTHROUGH;
                case MetaActionType::POLYPOLYGON:
                    SAL_FALLTHROUGH;

                case MetaActionType::TEXT:
                    SAL_FALLTHROUGH;
                case MetaActionType::TEXTARRAY:
                    SAL_FALLTHROUGH;
                case MetaActionType::STRETCHTEXT:
                    SAL_FALLTHROUGH;
                case MetaActionType::TEXTRECT:
                    SAL_FALLTHROUGH;

                case MetaActionType::MASK:
                    SAL_FALLTHROUGH;
                case MetaActionType::MASKSCALE:
                    SAL_FALLTHROUGH;
                case MetaActionType::MASKSCALEPART:
                    SAL_FALLTHROUGH;

                case MetaActionType::GRADIENT:
                    SAL_FALLTHROUGH;
                case MetaActionType::HATCH:
                    SAL_FALLTHROUGH;
                case MetaActionType::WALLPAPER:
                    SAL_FALLTHROUGH;

                case MetaActionType::Transparent:
                    SAL_FALLTHROUGH;
                case MetaActionType::EPS:
                    SAL_FALLTHROUGH;
                case MetaActionType::FLOATTRANSPARENT:
                    SAL_FALLTHROUGH;
                case MetaActionType::GRADIENTEX:
                    SAL_FALLTHROUGH;

                    // OutDev state changes that _do_ affect bitmap
                    // output
                case MetaActionType::CLIPREGION:
                    SAL_FALLTHROUGH;
                case MetaActionType::ISECTRECTCLIPREGION:
                    SAL_FALLTHROUGH;
                case MetaActionType::ISECTREGIONCLIPREGION:
                    SAL_FALLTHROUGH;
                case MetaActionType::MOVECLIPREGION:
                    SAL_FALLTHROUGH;

                case MetaActionType::MAPMODE:
                    SAL_FALLTHROUGH;
                case MetaActionType::REFPOINT:
                    SAL_FALLTHROUGH;
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

GraphicDisplayCacheEntry::~GraphicDisplayCacheEntry()
{
    delete mpMtf;
    delete mpBmpEx;
}

void GraphicDisplayCacheEntry::Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz ) const
{
    if( mpMtf )
        GraphicManager::ImplDraw( pOut, rPt, rSz, *mpMtf, maAttr );
    else if( mpBmpEx )
    {
        if( maAttr.IsRotated() )
        {
            tools::Polygon aPoly( Rectangle( rPt, rSz ) );

            aPoly.Rotate( rPt, maAttr.GetRotation() % 3600 );
            const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
            pOut->DrawBitmapEx( aRotBoundRect.TopLeft(), aRotBoundRect.GetSize(), *mpBmpEx );
        }
        else
            pOut->DrawBitmapEx( rPt, rSz, *mpBmpEx );
    }
}

GraphicCache::GraphicCache( sal_uLong nDisplayCacheSize, sal_uLong nMaxObjDisplayCacheSize ) :
    maReleaseTimer          ( "GraphicCache maReleaseTimer" ),
    mnReleaseTimeoutSeconds ( 0UL ),
    mnMaxDisplaySize        ( nDisplayCacheSize ),
    mnMaxObjDisplaySize     ( nMaxObjDisplayCacheSize ),
    mnUsedDisplaySize       ( 0UL )
{
    maReleaseTimer.SetTimeoutHdl( LINK( this, GraphicCache, ReleaseTimeoutHdl ) );
    maReleaseTimer.SetTimeout( 10000 );
    maReleaseTimer.Start();
}

GraphicCache::~GraphicCache()
{
    DBG_ASSERT( !maGraphicCache.size(), "GraphicCache::~GraphicCache(): there are some GraphicObjects in cache" );
    DBG_ASSERT( maDisplayCache.empty(), "GraphicCache::~GraphicCache(): there are some GraphicObjects in display cache" );
}

void GraphicCache::AddGraphicObject(
    const GraphicObject& rObj,
    Graphic& rSubstitute,
    const OString* pID,
    const GraphicObject* pCopyObj
)
{
    bool bInserted = false;

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
                    bInserted = true;
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
            std::unique_ptr< GraphicID > apID;

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
                                bInserted = true;
                            }
                        }

                        if( !bInserted )
                        {
                            maGraphicCache.push_back( new GraphicCacheEntry( rObj ) );
                            bInserted = true;
                        }
                    }
                }
                else
                {
                    if( rEntryID == *apID )
                    {
                        (*it)->AddGraphicObjectReference( rObj, rSubstitute );
                        bInserted = true;
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

void GraphicCache::ReleaseGraphicObject( const GraphicObject& rObj )
{
    // Release cached object
    bool    bRemoved = false;
    GraphicCacheEntryList::iterator it = maGraphicCache.begin();
    while (!bRemoved && it != maGraphicCache.end())
    {
        bRemoved = (*it)->ReleaseGraphicObjectReference( rObj );

        if( bRemoved && (0 == (*it)->GetGraphicObjectReferenceCount()) )
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
        else
            ++it;
    }

    DBG_ASSERT( bRemoved, "GraphicCache::ReleaseGraphicObject(...): GraphicObject not found in cache" );
}

void GraphicCache::GraphicObjectWasSwappedOut( const GraphicObject& rObj )
{
    // notify cache that rObj is swapped out (and can thus be pruned
    // from the cache)
    GraphicCacheEntry* pEntry = ImplGetCacheEntry( rObj );

    if( pEntry )
        pEntry->GraphicObjectWasSwappedOut( rObj );
}

void GraphicCache::GraphicObjectWasSwappedIn( const GraphicObject& rObj )
{
    GraphicCacheEntry* pEntry = ImplGetCacheEntry( rObj );

    if( pEntry )
    {
        if( pEntry->GetID().IsEmpty() )
        {
            ReleaseGraphicObject( rObj );
            AddGraphicObject( rObj, (Graphic&) rObj.GetGraphic(), nullptr, nullptr );
        }
        else
            pEntry->GraphicObjectWasSwappedIn( rObj );
    }
}

void GraphicCache::SetMaxDisplayCacheSize( sal_uLong nNewCacheSize )
{
    mnMaxDisplaySize = nNewCacheSize;

    if( GetMaxDisplayCacheSize() < GetUsedDisplayCacheSize() )
        ImplFreeDisplayCacheSpace( GetUsedDisplayCacheSize() - GetMaxDisplayCacheSize() );
}

void GraphicCache::SetMaxObjDisplayCacheSize( sal_uLong nNewMaxObjSize, bool bDestroyGreaterCached )
{
    const bool bDestroy = ( bDestroyGreaterCached && ( nNewMaxObjSize < mnMaxObjDisplaySize ) );

    mnMaxObjDisplaySize = std::min( nNewMaxObjSize, mnMaxDisplaySize );

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

bool GraphicCache::IsDisplayCacheable( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                       const GraphicObject& rObj, const GraphicAttr& rAttr ) const
{
    return( GraphicDisplayCacheEntry::GetNeededSize( pOut, rPt, rSz, rObj, rAttr ) <=
            GetMaxObjDisplayCacheSize() );
}

bool GraphicCache::IsInDisplayCache( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                     const GraphicObject& rObj, const GraphicAttr& rAttr ) const
{
    const Point                 aPtPixel( pOut->LogicToPixel( rPt ) );
    const Size                  aSzPixel( pOut->LogicToPixel( rSz ) );
    const GraphicCacheEntry*    pCacheEntry = const_cast<GraphicCache*>(this)->ImplGetCacheEntry( rObj );
    bool                        bFound = false;

    if( pCacheEntry )
    {
        for( GraphicDisplayCacheEntryList::const_iterator it = maDisplayCache.begin();
             !bFound && ( it != maDisplayCache.end() ); ++it )
        {
            if( (*it)->Matches( pOut, aPtPixel, aSzPixel, pCacheEntry, rAttr ) )
                bFound = true;
        }
    }

    return bFound;
}

OString GraphicCache::GetUniqueID( const GraphicObject& rObj ) const
{
    OString aRet;
    GraphicCacheEntry*  pEntry = const_cast<GraphicCache*>(this)->ImplGetCacheEntry( rObj );

    // ensure that the entry is correctly initialized (it has to be read at least once)
    if( pEntry && pEntry->GetID().IsEmpty() )
    {
        pEntry->TryToSwapIn();
        // do another call to ImplGetCacheEntry in case of modified entry list
        pEntry = const_cast<GraphicCache*>(this)->ImplGetCacheEntry( rObj );
    }

    if( pEntry )
        aRet = pEntry->GetID().GetIDString();

    return aRet;
}

bool GraphicCache::CreateDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                          const BitmapEx& rBmpEx )
{
    const sal_uLong nNeededSize = GraphicDisplayCacheEntry::GetNeededSize( pOut, rPt, rSz, rObj, rAttr );
    bool        bRet = false;

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
        bRet = true;
    }

    return bRet;
}

bool GraphicCache::CreateDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                          const GDIMetaFile& rMtf )
{
    const sal_uLong nNeededSize = GraphicDisplayCacheEntry::GetNeededSize( pOut, rPt, rSz, rObj, rAttr );
    bool        bRet = false;

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
        bRet = true;
    }

    return bRet;
}

bool GraphicCache::DrawDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                        const GraphicObject& rObj, const GraphicAttr& rAttr )
{
    const Point                 aPtPixel( pOut->LogicToPixel( rPt ) );
    const Size                  aSzPixel( pOut->LogicToPixel( rSz ) );
    const GraphicCacheEntry*    pCacheEntry = ImplGetCacheEntry( rObj );
    GraphicDisplayCacheEntry*   pDisplayCacheEntry = nullptr;
    GraphicDisplayCacheEntryList::iterator it = maDisplayCache.begin();
    bool                    bRet = false;

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
            bRet = true;
        }
        else
            ++it;
    }

    if( bRet )
        pDisplayCacheEntry->Draw( pOut, rPt, rSz );

    return bRet;
}

bool GraphicCache::ImplFreeDisplayCacheSpace( sal_uLong nSizeToFree )
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

GraphicCacheEntry* GraphicCache::ImplGetCacheEntry( const GraphicObject& rObj )
{
    GraphicCacheEntry* pRet = nullptr;

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

IMPL_LINK_TYPED( GraphicCache, ReleaseTimeoutHdl, Timer*, pTimer, void )
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
