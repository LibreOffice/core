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

#ifndef INCLUDED_SVTOOLS_GRFMGR_HXX
#define INCLUDED_SVTOOLS_GRFMGR_HXX

#include <vcl/graph.hxx>
#include <svtools/svtdllapi.h>
#include <o3tl/typed_flags_set.hxx>

enum class GraphicManagerDrawFlags
{
    CACHED                  = 0x01,
    SMOOTHSCALE             = 0x02,
    USE_DRAWMODE_SETTINGS   = 0x04,
    SUBSTITUTE              = 0x08,
    NO_SUBSTITUTE           = 0x10,
    STANDARD                = (CACHED|SMOOTHSCALE),
};
namespace o3tl
{
    template<> struct typed_flags<GraphicManagerDrawFlags> : is_typed_flags<GraphicManagerDrawFlags, 0x1f> {};
}

// AutoSwap defines

#define GRFMGR_AUTOSWAPSTREAM_LINK nullptr
#define GRFMGR_AUTOSWAPSTREAM_LOADED reinterpret_cast<SvStream*>(sal_IntPtr(-3))
#define GRFMGR_AUTOSWAPSTREAM_TEMP   reinterpret_cast<SvStream*>(sal_IntPtr(-2))
#define GRFMGR_AUTOSWAPSTREAM_NONE   reinterpret_cast<SvStream*>(sal_IntPtr(-1))

// Adjustment defines
enum class GraphicAdjustmentFlags
{
    NONE                 = 0x00,
    DRAWMODE             = 0x01,
    COLORS               = 0x02,
    MIRROR               = 0x04,
    ROTATE               = 0x08,
    TRANSPARENCY         = 0x10,
    ALL                  = 0x1f,
};
namespace o3tl
{
    template<> struct typed_flags<GraphicAdjustmentFlags> : is_typed_flags<GraphicAdjustmentFlags, 0x1f> {};
}

enum GraphicDrawMode
{
    GRAPHICDRAWMODE_STANDARD = 0,
    GRAPHICDRAWMODE_GREYS = 1,
    GRAPHICDRAWMODE_MONO = 2,
    GRAPHICDRAWMODE_WATERMARK = 3
};

class GraphicManager;
class SvStream;
class BitmapWriteAccess;
class GraphicCache;
class VirtualDevice;
struct GrfSimpleCacheObj;
struct ImplTileInfo;

class SVT_DLLPUBLIC GraphicAttr
{
private:

    double          mfGamma;
    BmpMirrorFlags  mnMirrFlags;
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
    bool            mbInvert;
    sal_uInt8       mcTransparency;
    GraphicDrawMode meDrawMode;

public:

                    GraphicAttr();
                    ~GraphicAttr();

    bool            operator==( const GraphicAttr& rAttr ) const;
    bool            operator!=( const GraphicAttr& rAttr ) const { return !( *this == rAttr ); }

    void            SetDrawMode( GraphicDrawMode eDrawMode ) { meDrawMode = eDrawMode; }
    GraphicDrawMode GetDrawMode() const { return meDrawMode; }

    void            SetMirrorFlags( BmpMirrorFlags nMirrFlags ) { mnMirrFlags = nMirrFlags; }
    BmpMirrorFlags  GetMirrorFlags() const { return mnMirrFlags; }

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

    void            SetInvert( bool bInvert ) { mbInvert = bInvert; }
    bool            IsInvert() const { return mbInvert; }

    void            SetTransparency( sal_uInt8 cTransparency ) { mcTransparency = cTransparency; }
    sal_uInt8       GetTransparency() const { return mcTransparency; }

    bool            IsSpecialDrawMode() const { return( meDrawMode != GRAPHICDRAWMODE_STANDARD ); }
    bool            IsMirrored() const { return mnMirrFlags != BmpMirrorFlags::NONE; }
    bool            IsCropped() const
                    {
                        return( mnLeftCrop != 0 || mnTopCrop != 0 ||
                                mnRightCrop != 0 || mnBottomCrop != 0 );
                    }
    bool            IsRotated() const { return( ( mnRotate10 % 3600 ) != 0 ); }
    bool            IsTransparent() const { return( mcTransparency > 0 ); }
    bool            IsAdjusted() const
                    {
                        return( mnLumPercent != 0 || mnContPercent != 0 || mnRPercent != 0 ||
                                mnGPercent != 0 || mnBPercent != 0 || mfGamma != 1.0 || mbInvert );
                    }
};

class SVT_DLLPUBLIC GraphicObject : public SvDataCopyStream
{
    friend class GraphicManager;
    friend class SdrGrafObj;

private:

    static GraphicManager*  mpGlobalMgr;

    Graphic                 maGraphic;
    GraphicAttr             maAttr;
    Size                    maPrefSize;
    MapMode                 maPrefMapMode;
    sal_uLong               mnSizeBytes;
    GraphicType             meType;
    GraphicManager*         mpMgr;
    OUString                maLink;
    Link<const GraphicObject*, SvStream*> maSwapStreamHdl;
    OUString                maUserData;
    Timer*                  mpSwapOutTimer;
    GrfSimpleCacheObj*      mpSimpleCache;
    sal_uLong               mnAnimationLoopCount;

    // a unique increasing ID to be able to say which data change is older
    sal_uLong               mnDataChangeTimeStamp;

    bool                    mbAutoSwapped   : 1;
    bool                    mbTransparent   : 1;
    bool                    mbAnimated      : 1;
    bool                    mbEPS           : 1;
    bool                    mbIsInSwapIn    : 1;
    bool                    mbIsInSwapOut   : 1;
    bool                    mbAlpha         : 1;

    void                    SVT_DLLPRIVATE ImplConstruct();
    void                    SVT_DLLPRIVATE ImplAssignGraphicData();
    void                    SVT_DLLPRIVATE ImplSetGraphicManager(
                                const GraphicManager* pMgr,
                                const OString* pID = NULL,
                                const GraphicObject* pCopyObj = NULL
                            );
    void                    SVT_DLLPRIVATE ImplAutoSwapIn();
    bool                    SVT_DLLPRIVATE ImplGetCropParams(
                                OutputDevice* pOut,
                                Point& rPt,
                                Size& rSz,
                                const GraphicAttr* pAttr,
                                tools::PolyPolygon& rClipPolyPoly,
                                bool& bRectClipRegion
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
                                GraphicManagerDrawFlags nFlags
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
                                GraphicManagerDrawFlags nFlags,
                                ImplTileInfo& rTileInfo
                            );

    bool SVT_DLLPRIVATE     ImplDrawTiled(
                                OutputDevice* pOut,
                                const Rectangle& rArea,
                                const Size& rSizePixel,
                                const Size& rOffset,
                                const GraphicAttr* pAttr,
                                GraphicManagerDrawFlags nFlags,
                                int nTileCacheSize1D
                            );

    bool SVT_DLLPRIVATE     ImplDrawTiled(
                                OutputDevice& rOut,
                                const Point& rPos,
                                int nNumTilesX,
                                int nNumTilesY,
                                const Size& rTileSize,
                                const GraphicAttr* pAttr,
                                GraphicManagerDrawFlags nFlags
                            );

    void SVT_DLLPRIVATE     ImplTransformBitmap(
                                BitmapEx&           rBmpEx,
                                const GraphicAttr&  rAttr,
                                const Size&         rCropLeftTop,
                                const Size&         rCropRightBottom,
                                const Rectangle&    rCropRect,
                                const Size&         rDstSize,
                                bool                bEnlarge
                            ) const;

                            DECL_LINK_TYPED( ImplAutoSwapOutHdl, Timer*, void );

    // restart SwapOut timer; this is like touching in a cache to reset to the full timeout value
    void SVT_DLLPRIVATE     restartSwapOutTimer() const;

    // Handle evtl. needed AfterDataChanges, needs to be called when new
    // graphic data is swapped in/added to the GraphicManager
    void SVT_DLLPRIVATE     ImplAfterDataChange();
protected:

    void                    GraphicManagerDestroyed();
    SvStream*               GetSwapStream() const;
    void                    SetSwapState();

public:
                            GraphicObject( const GraphicManager* pMgr = NULL );
                            GraphicObject( const Graphic& rGraphic, const GraphicManager* pMgr = NULL );
                            GraphicObject( const GraphicObject& rCacheObj, const GraphicManager* pMgr = NULL );
                            explicit GraphicObject( const OString& rUniqueID, const GraphicManager* pMgr = NULL );
                            virtual ~GraphicObject();

    GraphicObject&          operator=( const GraphicObject& rCacheObj );
    bool                    operator==( const GraphicObject& rCacheObj ) const;
    bool                    operator!=( const GraphicObject& rCacheObj ) const { return !( *this == rCacheObj ); }

    bool                    HasSwapStreamHdl() const { return maSwapStreamHdl.IsSet(); }
    void                    SetSwapStreamHdl(const Link<const GraphicObject*, SvStream*>& rHdl);

    void                    FireSwapInRequest();
    void                    FireSwapOutRequest();

    GraphicManager&         GetGraphicManager() const { return *mpMgr; }

    bool                    IsCached(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicAttr* pAttr = NULL,
                                GraphicManagerDrawFlags nFlags = GraphicManagerDrawFlags::STANDARD
                            ) const;

    const Graphic&          GetGraphic() const;
    void                    SetGraphic( const Graphic& rGraphic, const GraphicObject* pCopyObj = 0);
    void                    SetGraphic( const Graphic& rGraphic, const OUString& rLink );

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

    bool                    HasLink() const { return !maLink.isEmpty(); }
    void                    SetLink();
    void                    SetLink( const OUString& rLink );
    OUString                GetLink() const { return maLink; }

    bool                    HasUserData() const { return !maUserData.isEmpty(); }
    void                    SetUserData();
    void                    SetUserData( const OUString& rUserData );
    OUString                GetUserData() const { return maUserData; }

    OString                 GetUniqueID() const;

    GraphicType             GetType() const { return meType; }
    const Size&             GetPrefSize() const { return maPrefSize; }
    const MapMode&          GetPrefMapMode() const { return maPrefMapMode; }
    sal_uLong               GetSizeBytes() const { return mnSizeBytes; }
    bool                    IsTransparent() const { return mbTransparent; }
    bool                    IsAnimated() const { return mbAnimated; }
    bool                    IsEPS() const { return mbEPS; }

    Link<Animation*,void>   GetAnimationNotifyHdl() const { return GetGraphic().GetAnimationNotifyHdl(); }

    bool                    SwapOut();
    bool                    SwapOut( SvStream* pOStm );
    bool                    SwapIn();

    bool                    IsInSwapIn() const { return mbIsInSwapIn; }
    bool                    IsInSwapOut() const { return mbIsInSwapOut; }
    bool                    IsSwappedOut() const { return( mbAutoSwapped || maGraphic.IsSwapOut() ); }

    bool                    Draw(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                const GraphicAttr* pAttr = NULL,
                                GraphicManagerDrawFlags nFlags = GraphicManagerDrawFlags::STANDARD
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
    bool                    DrawTiled(
                                OutputDevice* pOut,
                                const Rectangle& rArea,
                                const Size& rSize,
                                const Size& rOffset,
                                const GraphicAttr* pAttr = NULL,
                                GraphicManagerDrawFlags nFlags = GraphicManagerDrawFlags::STANDARD,
                                int nTileCacheSize1D=128
                            );

    bool                    StartAnimation(
                                OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                long nExtraData = 0L,
                                const GraphicAttr* pAttr = NULL,
                                GraphicManagerDrawFlags nFlags = GraphicManagerDrawFlags::STANDARD,
                                OutputDevice* pFirstFrameOutDev = NULL
                            );

    void                    StopAnimation( OutputDevice* pOut = NULL, long nExtraData = 0L );

    static GraphicObject    CreateGraphicObjectFromURL( const OUString &rURL );
    // will inspect an object ( e.g. a control ) for any 'ImageURL'
    // properties and return these in a vector. Note: this implementation
    // will cater for XNameContainer objects and deepinspect any containees
    // if they exist
    static void InspectForGraphicObjectImageURL( const css::uno::Reference< css::uno::XInterface >& rxIf, std::vector< OUString >& rvEmbedImgUrls );

    // create CropScaling information
    // fWidth, fHeight: object size
    // f*Crop: crop values relative to original bitmap size
    basegfx::B2DVector calculateCropScaling(
        double fWidth,
        double fHeight,
        double fLeftCrop,
        double fTopCrop,
        double fRightCrop,
        double fBottomCrop) const;

    // read access
    sal_uLong GetDataChangeTimeStamp() const { return mnDataChangeTimeStamp; }
};

typedef ::std::vector< GraphicObject* > GraphicObjectList_impl;

class SVT_DLLPUBLIC GraphicManager
{
    friend class GraphicObject;
    friend class GraphicDisplayCacheEntry;

private:

    GraphicObjectList_impl  maObjList;
    sal_uLong               mnUsedSize; // currently used memory footprint of all swapped in graphics
    GraphicCache*           mpCache;

                        GraphicManager( const GraphicManager& ) {}
    GraphicManager&     operator=( const GraphicManager& ) { return *this; }

    bool SVT_DLLPRIVATE ImplDraw(
                            OutputDevice* pOut,
                            const Point& rPt,
                            const Size& rSz,
                            GraphicObject& rObj,
                            const GraphicAttr& rAttr,
                            const GraphicManagerDrawFlags nFlags,
                            bool& rCached
                        );

    static bool SVT_DLLPRIVATE ImplCreateOutput(
                            OutputDevice* pOut,
                            const Point& rPt,
                            const Size& rSz,
                            const BitmapEx& rBmpEx,
                            const GraphicAttr& rAttr,
                            const GraphicManagerDrawFlags nFlags,
                            BitmapEx* pBmpEx = NULL
                        );
    static bool SVT_DLLPRIVATE ImplCreateOutput(
                            OutputDevice* pOut,
                            const Point& rPt,
                            const Size& rSz,
                            const GDIMetaFile& rMtf,
                            const GraphicAttr& rAttr,
                            const GraphicManagerDrawFlags nFlags,
                            GDIMetaFile& rOutMtf,
                            BitmapEx& rOutBmpEx
                        );

    static void SVT_DLLPRIVATE ImplAdjust(
                            BitmapEx& rBmpEx,
                            const GraphicAttr& rAttr,
                            GraphicAdjustmentFlags nAdjustmentFlags
                        );
    static void SVT_DLLPRIVATE ImplAdjust(
                            GDIMetaFile& rMtf,
                            const GraphicAttr& rAttr,
                            GraphicAdjustmentFlags nAdjustmentFlags
                        );
    static void SVT_DLLPRIVATE ImplAdjust(
                            Animation& rAnimation,
                            const GraphicAttr& rAttr,
                            GraphicAdjustmentFlags nAdjustmentFlags
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
                            const OString* pID = NULL,
                            const GraphicObject* pCopyObj = NULL
                        );
    void SVT_DLLPRIVATE ImplUnregisterObj( const GraphicObject& rObj );
    inline bool SVT_DLLPRIVATE ImplHasObjects() const { return !maObjList.empty(); }

                    // Only used in swap case by GraphicObject
    void SVT_DLLPRIVATE ImplGraphicObjectWasSwappedOut( const GraphicObject& rObj );
    void SVT_DLLPRIVATE ImplGraphicObjectWasSwappedIn( const GraphicObject& rObj );

    OString SVT_DLLPRIVATE ImplGetUniqueID( const GraphicObject& rObj ) const;

    // This method allows to check memory footprint for all currently swapped in GraphicObjects on this GraphicManager
    // which are based on Bitmaps. This is needed on 32Bit systems and only does something on those systems. The problem
    // to solve is that normally the SwapOut is timer-driven, but even with short timer settings there are situations
    // where this does not trigger - or in other words: A maximum limitation for GraphicManagers was not in place before.
    // For 32Bit systems this leads to situations where graphics will be missing. This method will actively swap out
    // the longest swapped in graphics until a maximum memory boundary (derived from user settings in tools/options/memory)
    // is no longer exceeded
    void SVT_DLLPRIVATE ImplCheckSizeOfSwappedInGraphics(const GraphicObject* pGraphicToIgnore);
public:

                        GraphicManager( sal_uLong nCacheSize = 10000000UL, sal_uLong nMaxObjCacheSize = 2400000UL );
                        ~GraphicManager();

    void                SetMaxCacheSize( sal_uLong nNewCacheSize );
    sal_uLong           GetMaxCacheSize() const;

    void                SetMaxObjCacheSize(
                            sal_uLong nNewMaxObjSize,
                            bool bDestroyGreaterCached = false
                        );

    void                SetCacheTimeout( sal_uLong nTimeoutSeconds );

    bool                IsInCache(
                            OutputDevice* pOut,
                            const Point& rPt,
                            const Size& rSz,
                            const GraphicObject& rObj,
                            const GraphicAttr& rAttr
                        ) const;

    bool                DrawObj(
                            OutputDevice* pOut,
                            const Point& rPt,
                            const Size& rSz,
                            GraphicObject& rObj,
                            const GraphicAttr& rAttr,
                            const GraphicManagerDrawFlags nFlags,
                            bool& rCached
                        );
};

#endif // INCLUDED_SVTOOLS_GRFMGR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
