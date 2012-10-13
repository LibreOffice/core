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

#ifndef _GRFMGR_HXX
#define _GRFMGR_HXX

#include <vcl/graph.hxx>
#include <svtools/svtdllapi.h>

// -----------
// - Defines -
// -----------

#define GRFMGR_DRAW_NOTCACHED               0x00000000UL
#define GRFMGR_DRAW_CACHED                  0x00000001UL
#define GRFMGR_DRAW_SMOOTHSCALE             0x00000002UL
#define GRFMGR_DRAW_USE_DRAWMODE_SETTINGS   0x00000004UL
#define GRFMGR_DRAW_SUBSTITUTE              0x00000008UL
#define GRFMGR_DRAW_NO_SUBSTITUTE           0x00000010UL
#define GRFMGR_DRAW_STANDARD                (GRFMGR_DRAW_CACHED|GRFMGR_DRAW_SMOOTHSCALE)

// --------------------
// - AutoSwap Defines -
// --------------------

#define GRFMGR_AUTOSWAPSTREAM_LINK      ((SvStream*)0x00000000UL)
#define GRFMGR_AUTOSWAPSTREAM_LOADED    ((SvStream*)0xfffffffdUL)
#define GRFMGR_AUTOSWAPSTREAM_TEMP      ((SvStream*)0xfffffffeUL)
#define GRFMGR_AUTOSWAPSTREAM_NONE      ((SvStream*)0xffffffffUL)

// ----------------------
// - Adjustment Defines -
// ----------------------

#define ADJUSTMENT_NONE                 0x00000000UL
#define ADJUSTMENT_DRAWMODE             0x00000001UL
#define ADJUSTMENT_COLORS               0x00000002UL
#define ADJUSTMENT_MIRROR               0x00000004UL
#define ADJUSTMENT_ROTATE               0x00000008UL
#define ADJUSTMENT_TRANSPARENCY         0x00000010UL
#define ADJUSTMENT_ALL                  0xFFFFFFFFUL

// ---------
// - Enums -
// ---------

enum GraphicDrawMode
{
    GRAPHICDRAWMODE_STANDARD = 0,
    GRAPHICDRAWMODE_GREYS = 1,
    GRAPHICDRAWMODE_MONO = 2,
    GRAPHICDRAWMODE_WATERMARK = 3
};

// ------------
// - Forwards -
// ------------

class GraphicManager;
class SvStream;
class BitmapWriteAccess;
class GraphicCache;
class VirtualDevice;
struct GrfSimpleCacheObj;
struct ImplTileInfo;

// ---------------
// - GraphicAttr -
// ---------------

class SVT_DLLPUBLIC GraphicAttr
{
private:

    double          mfGamma;
    sal_uInt32      mnMirrFlags;
    long            mnLeftCrop;
    long            mnTopCrop;
    long            mnRightCrop;
    long            mnBottomCrop;
    sal_uInt16      mnRotate10;
    short           mnContPercent;
    short           mnLumPercent;
    short           mnRPercent;
    short           mnGPercent;
    short           mnBPercent;
    sal_Bool        mbInvert;
    sal_uInt8       mcTransparency;
    GraphicDrawMode meDrawMode;

public:

                    GraphicAttr();
                    ~GraphicAttr();

    sal_Bool        operator==( const GraphicAttr& rAttr ) const;
    sal_Bool        operator!=( const GraphicAttr& rAttr ) const { return !( *this == rAttr ); }

    void            SetDrawMode( GraphicDrawMode eDrawMode ) { meDrawMode = eDrawMode; }
    GraphicDrawMode GetDrawMode() const { return meDrawMode; }

    void            SetMirrorFlags( sal_uLong nMirrFlags ) { mnMirrFlags = nMirrFlags; }
    sal_uLong       GetMirrorFlags() const { return mnMirrFlags; }

    void            SetCrop( long nLeft_100TH_MM, long nTop_100TH_MM, long nRight_100TH_MM, long nBottom_100TH_MM )
                    {
                        mnLeftCrop = nLeft_100TH_MM; mnTopCrop = nTop_100TH_MM;
                        mnRightCrop = nRight_100TH_MM; mnBottomCrop = nBottom_100TH_MM;
                    }
    long            GetLeftCrop() const { return mnLeftCrop; }
    long            GetTopCrop() const { return mnTopCrop; }
    long            GetRightCrop() const { return mnRightCrop; }
    long            GetBottomCrop() const { return mnBottomCrop; }

    void            SetRotation( sal_uInt16 nRotate10 ) { mnRotate10 = nRotate10; }
    sal_uInt16      GetRotation() const { return mnRotate10; }

    void            SetLuminance( short nLuminancePercent ) { mnLumPercent = nLuminancePercent; }
    short           GetLuminance() const { return mnLumPercent; }

    void            SetContrast( short nContrastPercent ) { mnContPercent = nContrastPercent; }
    short           GetContrast() const { return mnContPercent; }

    void            SetChannelR( short nChannelRPercent ) { mnRPercent = nChannelRPercent; }
    short           GetChannelR() const { return mnRPercent; }

    void            SetChannelG( short nChannelGPercent ) { mnGPercent = nChannelGPercent; }
    short           GetChannelG() const { return mnGPercent; }

    void            SetChannelB( short nChannelBPercent ) { mnBPercent = nChannelBPercent; }
    short           GetChannelB() const { return mnBPercent; }

    void            SetGamma( double fGamma ) { mfGamma = fGamma; }
    double          GetGamma() const { return mfGamma; }

    void            SetInvert( sal_Bool bInvert ) { mbInvert = bInvert; }
    sal_Bool        IsInvert() const { return mbInvert; }

    void            SetTransparency( sal_uInt8 cTransparency ) { mcTransparency = cTransparency; }
    sal_uInt8       GetTransparency() const { return mcTransparency; }

    sal_Bool        IsSpecialDrawMode() const { return( meDrawMode != GRAPHICDRAWMODE_STANDARD ); }
    sal_Bool        IsMirrored() const { return( mnMirrFlags != 0UL ); }
    sal_Bool        IsCropped() const
                    {
                        return( mnLeftCrop != 0 || mnTopCrop != 0 ||
                                mnRightCrop != 0 || mnBottomCrop != 0 );
                    }
    sal_Bool        IsRotated() const { return( ( mnRotate10 % 3600 ) != 0 ); }
    sal_Bool        IsTransparent() const { return( mcTransparency > 0 ); }
    sal_Bool        IsAdjusted() const
                    {
                        return( mnLumPercent != 0 || mnContPercent != 0 || mnRPercent != 0 ||
                                mnGPercent != 0 || mnBPercent != 0 || mfGamma != 1.0 || mbInvert );
                    }

    friend SvStream& operator<<( SvStream& rOStm, const GraphicAttr& rAttr );
    friend SvStream& operator>>( SvStream& rIStm, GraphicAttr& rAttr );
};

// -----------------
// - GraphicObject -
// -----------------

class SVT_DLLPUBLIC GraphicObject : public SvDataCopyStream
{
    friend class GraphicManager;

private:

    static GraphicManager*  mpGlobalMgr;

    Graphic                 maGraphic;
    GraphicAttr             maAttr;
    Size                    maPrefSize;
    MapMode                 maPrefMapMode;
    sal_uLong               mnSizeBytes;
    GraphicType             meType;
    GraphicManager*         mpMgr;
    String*                 mpLink;
    Link*                   mpSwapStreamHdl;
    String*                 mpUserData;
    Timer*                  mpSwapOutTimer;
    GrfSimpleCacheObj*      mpSimpleCache;
    sal_uLong               mnAnimationLoopCount;
    sal_Bool                mbAutoSwapped   : 1;
    sal_Bool                mbTransparent   : 1;
    sal_Bool                mbAnimated      : 1;
    sal_Bool                mbEPS           : 1;
    sal_Bool                mbIsInSwapIn    : 1;
    sal_Bool                mbIsInSwapOut   : 1;
    sal_Bool                mbAlpha         : 1;
    sal_Bool                mbIsRenderGraphic   : 1;
    sal_Bool                mbHasRenderGraphic  : 1;

    void                    SVT_DLLPRIVATE ImplConstruct();
    void                    SVT_DLLPRIVATE ImplAssignGraphicData();
    void                    SVT_DLLPRIVATE ImplSetGraphicManager(
                                const GraphicManager* pMgr,
                                const rtl::OString* pID = NULL,
                                const GraphicObject* pCopyObj = NULL
                            );
    void                    SVT_DLLPRIVATE ImplAutoSwapIn();
    sal_Bool                SVT_DLLPRIVATE ImplIsAutoSwapped() const { return mbAutoSwapped; }
    sal_Bool                SVT_DLLPRIVATE ImplGetCropParams(
                                OutputDevice* pOut,
                                Point& rPt,
                                Size& rSz,
                                const GraphicAttr* pAttr,
                                PolyPolygon& rClipPolyPoly,
                                sal_Bool& bRectClipRegion
                            ) const;

    /** Render a given number of tiles in an optimized way

        This method recursively subdivides the tile rendering problem
        in smaller parts, i.e. rendering output size x with few tiles
        of size y, which in turn are generated from the original
        bitmap in a recursive fashion. The subdivision size can be
        controlled by the exponent argument, which specifies the
        minimal number of smaller tiles used in one recursion
        step. The resulting tile size is given as the integer number
        of repetitions of the original bitmap along x and y. As the
        exponent need not necessarily divide these numbers without
        remainder, the repetition counts are effectively converted to
        base-exponent numbers, where each place denotes the number of
        times the corresponding tile size is rendered.

        @param rVDev
        Virtual device to render everything into

        @param nExponent
        Number of repetitions per subdivision step, _must_ be greater than 1

        @param nNumTilesX
        Number of original tiles to generate in x direction

        @param nNumTilesY
        Number of original tiles to generate in y direction

        @param rTileSizePixel
        Size in pixel of the original tile bitmap to render it in

        @param pAttr
        Graphic attributes to be used for rendering

        @param nFlags
        Graphic flags to be used for rendering

        @param rCurrPos
        Current output point for this recursion level (should start with (0,0))

        @return true, if everything was successfully rendered.
    */
    bool SVT_DLLPRIVATE     ImplRenderTempTile(
                                VirtualDevice& rVDev,
                                int nExponent,
                                int nNumTilesX,
                                int nNumTilesY,
                                const Size& rTileSizePixel,
                                const GraphicAttr* pAttr,
                                sal_uLong nFlags
                            );

    /// internally called by ImplRenderTempTile()
    bool SVT_DLLPRIVATE     ImplRenderTileRecursive(
                                VirtualDevice& rVDev,
                                int nExponent,
                                int nMSBFactor,
                                int nNumOrigTilesX,
                                int nNumOrigTilesY,
                                int nRemainderTilesX,
                                int nRemainderTilesY,
                                const Size& rTileSizePixel,
                                const GraphicAttr* pAttr,
                                sal_uLong nFlags,
                                ImplTileInfo& rTileInfo
                            );

    bool SVT_DLLPRIVATE     ImplDrawTiled(
                                OutputDevice* pOut,
                                const Rectangle& rArea,
                                const Size& rSizePixel,
                                const Size& rOffset,
                                const GraphicAttr* pAttr,
                                sal_uLong nFlags,
                                int nTileCacheSize1D
                            );

    bool SVT_DLLPRIVATE     ImplDrawTiled(
                                OutputDevice& rOut,
                                const Point& rPos,
                                int nNumTilesX,
                                int nNumTilesY,
                                const Size& rTileSize,
                                const GraphicAttr* pAttr,
                                sal_uLong nFlags
                            );

    void SVT_DLLPRIVATE     ImplTransformBitmap(
                                BitmapEx&           rBmpEx,
                                const GraphicAttr&  rAttr,
                                const Size&         rCropLeftTop,
                                const Size&         rCropRightBottom,
                                const Rectangle&    rCropRect,
                                const Size&         rDstSize,
                                sal_Bool            bEnlarge
                            ) const;

                            DECL_LINK( ImplAutoSwapOutHdl, void* );

protected:

    virtual void            GraphicManagerDestroyed();
    virtual SvStream*       GetSwapStream() const;

    virtual void            Load( SvStream& );
    virtual void            Save( SvStream& );
    virtual void            Assign( const SvDataCopyStream& );

public:

                            TYPEINFO();

                            GraphicObject( const GraphicManager* pMgr = NULL );
                            GraphicObject( const Graphic& rGraphic, const GraphicManager* pMgr = NULL );
                            GraphicObject( const GraphicObject& rCacheObj, const GraphicManager* pMgr = NULL );
                            explicit GraphicObject( const rtl::OString& rUniqueID, const GraphicManager* pMgr = NULL );
                            ~GraphicObject();

    GraphicObject&          operator=( const GraphicObject& rCacheObj );
    sal_Bool                operator==( const GraphicObject& rCacheObj ) const;
    sal_Bool                operator!=( const GraphicObject& rCacheObj ) const { return !( *this == rCacheObj ); }

    sal_Bool                HasSwapStreamHdl() const { return( mpSwapStreamHdl != NULL && mpSwapStreamHdl->IsSet() ); }
    void                    SetSwapStreamHdl();
    void                    SetSwapStreamHdl( const Link& rHdl, const sal_uLong nSwapOutTimeout = 0UL );
    sal_uLong               GetSwapOutTimeout() const { return( mpSwapOutTimer ? mpSwapOutTimer->GetTimeout() : 0 ); }

    void                    FireSwapInRequest();
    void                    FireSwapOutRequest();

    GraphicManager&         GetGraphicManager() const { return *mpMgr; }

    sal_Bool                IsCached(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicAttr* pAttr = NULL,
                                sal_uLong nFlags = GRFMGR_DRAW_STANDARD
                            ) const;
    void                    ReleaseFromCache();

    const Graphic&          GetGraphic() const;
    void                    SetGraphic( const Graphic& rGraphic, const GraphicObject* pCopyObj = 0);
    void                    SetGraphic( const Graphic& rGraphic, const String& rLink );

    /** Get graphic transformed according to given attributes

        This method returns a Graphic transformed, cropped and scaled
        to the given parameters, ready to be rendered to printer or
        display. The returned graphic has the same visual appearance
        as if it had been drawn via GraphicObject::Draw() to a
        specific output device.

        @param rDestSize
        Desired output size in logical coordinates. The mapmode to
        interpret these logical coordinates in is given by the second
        parameter, rDestMap.

        @param rDestMap
        Mapmode the output should be interpreted in. This is used to
        interpret rDestSize, to set the appropriate PrefMapMode on the
        returned Graphic, and to deal correctly with metafile graphics.

        @param rAttr
        Graphic attributes used to transform the graphic. This
        includes cropping, rotation, mirroring, and various color
        adjustment parameters.

        @return the readily transformed Graphic
     */
    Graphic                 GetTransformedGraphic(
                                const Size& rDestSize,
                                const MapMode& rDestMap,
                                const GraphicAttr& rAttr
                            ) const;
    Graphic                 GetTransformedGraphic( const GraphicAttr* pAttr = NULL ) const; // TODO: Change to Impl

    void                    SetAttr( const GraphicAttr& rAttr );
    const GraphicAttr&      GetAttr() const { return maAttr; }

    sal_Bool                HasLink() const { return( mpLink != NULL && mpLink->Len() > 0 ); }
    void                    SetLink();
    void                    SetLink( const String& rLink );
    String                  GetLink() const;

    sal_Bool                HasUserData() const { return( mpUserData != NULL && mpUserData->Len() > 0 ); }
    void                    SetUserData();
    void                    SetUserData( const String& rUserData );
    String                  GetUserData() const;

    rtl::OString            GetUniqueID() const;

    GraphicType             GetType() const { return meType; }
    const Size&             GetPrefSize() const { return maPrefSize; }
    const MapMode&          GetPrefMapMode() const { return maPrefMapMode; }
    sal_uLong               GetSizeBytes() const { return mnSizeBytes; }
    sal_Bool                IsTransparent() const { return mbTransparent; }
    sal_Bool                IsAlpha() const { return mbAlpha; }
    sal_Bool                IsAnimated() const { return mbAnimated; }
    sal_Bool                IsEPS() const { return mbEPS; }
    sal_Bool                IsRenderGraphic() const { return mbIsRenderGraphic; }
    sal_Bool                HasRenderGraphic() const { return mbHasRenderGraphic; }

    Link                    GetAnimationNotifyHdl() const { return maGraphic.GetAnimationNotifyHdl(); }

    sal_Bool                SwapOut();
    sal_Bool                SwapOut( SvStream* pOStm );
    sal_Bool                SwapIn();
    sal_Bool                SwapIn( SvStream* pIStm );

    sal_Bool                IsInSwapIn() const { return mbIsInSwapIn; }
    sal_Bool                IsInSwapOut() const { return mbIsInSwapOut; }
    sal_Bool                IsInSwap() const { return( mbIsInSwapOut || mbIsInSwapOut ); }
    sal_Bool                IsSwappedOut() const { return( mbAutoSwapped || maGraphic.IsSwapOut() ); }
    void                    SetSwapState();

    bool                    Draw(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicAttr* pAttr = NULL,
                                sal_uLong nFlags = GRFMGR_DRAW_STANDARD
                            );

    sal_Bool                DrawWithPDFHandling(
                                OutputDevice& rOutDev,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicAttr* pGrfAttr = NULL,
                                const sal_uLong nFlags = GRFMGR_DRAW_STANDARD
                            );

    /** Draw the graphic repeatedly into the given output rectangle

        @param pOut
        OutputDevice where the rendering should take place

        @param rArea
        The output area that is filled with tiled instances of this graphic

        @param rSize
        The actual size of a single tile

        @param rOffset
        Offset from the left, top position of rArea, where to start
        the tiling. The upper left corner of the graphic tilings will
        virtually start at this position. Concretely, only that many
        tiles are drawn to completely fill the given output area.

        @param pAttr
        Optional GraphicAttr

        @param nFlags
        Optional rendering flags

        @param nTileCacheSize1D
        Optional dimension of the generated cache tiles. The pOut sees
        a number of tile draws, which have approximately
        nTileCacheSize1D times nTileCacheSize1D bitmap sizes if the
        tile bitmap is smaller. Otherwise, the tile is drawn as
        is. This is useful if e.g. you want only a few, very large
        bitmap drawings appear on the outdev.

        @return sal_True, if drawing completed successfully
     */
    sal_Bool                DrawTiled(
                                OutputDevice* pOut,
                                const Rectangle& rArea,
                                const Size& rSize,
                                const Size& rOffset,
                                const GraphicAttr* pAttr = NULL,
                                sal_uLong nFlags = GRFMGR_DRAW_STANDARD,
                                int nTileCacheSize1D=128
                            );

    sal_Bool                StartAnimation(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                long nExtraData = 0L,
                                const GraphicAttr* pAttr = NULL,
                                sal_uLong nFlags = GRFMGR_DRAW_STANDARD,
                                OutputDevice* pFirstFrameOutDev = NULL
                            );

    void                    StopAnimation( OutputDevice* pOut = NULL, long nExtraData = 0L );

    friend SvStream&        operator<<( SvStream& rOStm, const GraphicObject& rGraphicObj );
    friend SvStream&        operator>>( SvStream& rIStm, GraphicObject& rGraphicObj );

    static GraphicObject    CreateGraphicObjectFromURL( const ::rtl::OUString &rURL );
    // will inspect an object ( e.g. a control ) for any 'ImageURL'
    // properties and return these in a vector. Note: this implementation
    // will cater for XNameContainer objects and deepinspect any containees
    // if they exist
    static void InspectForGraphicObjectImageURL( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIf, std::vector< rtl::OUString >& rvEmbedImgUrls );
};

// ------------------
// - GraphicManager -
// ------------------

typedef ::std::vector< GraphicObject* > GraphicObjectList_impl;

class SVT_DLLPUBLIC GraphicManager
{
    friend class GraphicObject;
    friend class GraphicDisplayCacheEntry;

private:

    GraphicObjectList_impl  maObjList;
    GraphicCache*           mpCache;

                        GraphicManager( const GraphicManager& ) {}
    GraphicManager&     operator=( const GraphicManager& ) { return *this; }

    sal_Bool SVT_DLLPRIVATE ImplDraw(
                            OutputDevice* pOut,
                            const Point& rPt,
                            const Size& rSz,
                            GraphicObject& rObj,
                            const GraphicAttr& rAttr,
                            const sal_uLong nFlags,
                            sal_Bool& rCached
                        );

    sal_Bool SVT_DLLPRIVATE ImplCreateOutput(
                            OutputDevice* pOut,
                            const Point& rPt,
                            const Size& rSz,
                            const BitmapEx& rBmpEx,
                            const GraphicAttr& rAttr,
                            const sal_uLong nFlags,
                            BitmapEx* pBmpEx = NULL
                        );
    sal_Bool SVT_DLLPRIVATE ImplCreateOutput(
                            OutputDevice* pOut,
                            const Point& rPt,
                            const Size& rSz,
                            const GDIMetaFile& rMtf,
                            const GraphicAttr& rAttr,
                            const sal_uLong nFlags,
                            GDIMetaFile& rOutMtf,
                            BitmapEx& rOutBmpEx
                        );

    static void SVT_DLLPRIVATE ImplAdjust(
                            BitmapEx& rBmpEx,
                            const GraphicAttr& rAttr,
                            sal_uLong nAdjustmentFlags
                        );
    static void SVT_DLLPRIVATE ImplAdjust(
                            GDIMetaFile& rMtf,
                            const GraphicAttr& rAttr,
                            sal_uLong nAdjustmentFlags
                        );
    static void SVT_DLLPRIVATE ImplAdjust(
                            Animation& rAnimation,
                            const GraphicAttr& rAttr,
                            sal_uLong nAdjustmentFlags
                        );

    static void SVT_DLLPRIVATE ImplDraw(
                            OutputDevice* pOut,
                            const Point& rPt,
                            const Size& rSz,
                            const GDIMetaFile& rMtf,
                            const GraphicAttr& rAttr
                        );

                    // Only used by GraphicObject's Ctor's and Dtor's
    void SVT_DLLPRIVATE ImplRegisterObj(
                            const GraphicObject& rObj,
                            Graphic& rSubstitute,
                            const rtl::OString* pID = NULL,
                            const GraphicObject* pCopyObj = NULL
                        );
    void SVT_DLLPRIVATE ImplUnregisterObj( const GraphicObject& rObj );
    inline sal_Bool SVT_DLLPRIVATE ImplHasObjects() const { return !maObjList.empty(); }

                    // Only used in swap case by GraphicObject
    void SVT_DLLPRIVATE ImplGraphicObjectWasSwappedOut( const GraphicObject& rObj );
    sal_Bool SVT_DLLPRIVATE ImplFillSwappedGraphicObject(
                            const GraphicObject& rObj,
                            Graphic& rSubstitute
                        );
    void SVT_DLLPRIVATE ImplGraphicObjectWasSwappedIn( const GraphicObject& rObj );

    rtl::OString SVT_DLLPRIVATE ImplGetUniqueID( const GraphicObject& rObj ) const;

public:

                        GraphicManager( sal_uLong nCacheSize = 10000000UL, sal_uLong nMaxObjCacheSize = 2400000UL );
                        ~GraphicManager();

    void                SetMaxCacheSize( sal_uLong nNewCacheSize );

    void                SetMaxObjCacheSize(
                            sal_uLong nNewMaxObjSize,
                            sal_Bool bDestroyGreaterCached = sal_False
                        );

    void                SetCacheTimeout( sal_uLong nTimeoutSeconds );

    void                ReleaseFromCache( const GraphicObject& rObj );

    sal_Bool            IsInCache(
                            OutputDevice* pOut,
                            const Point& rPt,
                            const Size& rSz,
                            const GraphicObject& rObj,
                            const GraphicAttr& rAttr
                        ) const;

    sal_Bool            DrawObj(
                            OutputDevice* pOut,
                            const Point& rPt,
                            const Size& rSz,
                            GraphicObject& rObj,
                            const GraphicAttr& rAttr,
                            const sal_uLong nFlags,
                            sal_Bool& rCached
                        );
};

#endif // _GRFMGR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
