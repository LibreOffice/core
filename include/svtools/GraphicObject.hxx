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

#ifndef _GRFMGR_HXX
#define _GRFMGR_HXX

#include <rtl/ref.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/graphic/XGraphicObject.hpp>
#include <vcl/graph.hxx>
#include <svtools/svtdllapi.h>

#define GRFMGR_DRAW_NOTCACHED               0x00000000UL
#define GRFMGR_DRAW_CACHED                  0x00000001UL
#define GRFMGR_DRAW_SMOOTHSCALE             0x00000002UL
#define GRFMGR_DRAW_USE_DRAWMODE_SETTINGS   0x00000004UL
#define GRFMGR_DRAW_SUBSTITUTE              0x00000008UL
#define GRFMGR_DRAW_NO_SUBSTITUTE           0x00000010UL
#define GRFMGR_DRAW_STANDARD                (GRFMGR_DRAW_CACHED|GRFMGR_DRAW_SMOOTHSCALE)

// AutoSwap defines

#define GRFMGR_AUTOSWAPSTREAM_LINK      ((SvStream*)0x00000000UL)
#define GRFMGR_AUTOSWAPSTREAM_LOADED    ((SvStream*)0xfffffffdUL)
#define GRFMGR_AUTOSWAPSTREAM_TEMP      ((SvStream*)0xfffffffeUL)
#define GRFMGR_AUTOSWAPSTREAM_NONE      ((SvStream*)0xffffffffUL)

// Adjustment defines

#define ADJUSTMENT_NONE                 0x00000000UL
#define ADJUSTMENT_DRAWMODE             0x00000001UL
#define ADJUSTMENT_COLORS               0x00000002UL
#define ADJUSTMENT_MIRROR               0x00000004UL
#define ADJUSTMENT_ROTATE               0x00000008UL
#define ADJUSTMENT_TRANSPARENCY         0x00000010UL
#define ADJUSTMENT_ALL                  0xFFFFFFFFUL

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

    void            SetInvert( bool bInvert ) { mbInvert = bInvert; }
    bool        IsInvert() const { return mbInvert; }

    void            SetTransparency( sal_uInt8 cTransparency ) { mcTransparency = cTransparency; }
    sal_uInt8       GetTransparency() const { return mcTransparency; }

    bool        IsSpecialDrawMode() const { return( meDrawMode != GRAPHICDRAWMODE_STANDARD ); }
    bool        IsMirrored() const { return( mnMirrFlags != 0UL ); }
    bool        IsCropped() const
                    {
                        return( mnLeftCrop != 0 || mnTopCrop != 0 ||
                                mnRightCrop != 0 || mnBottomCrop != 0 );
                    }
    bool        IsRotated() const { return( ( mnRotate10 % 3600 ) != 0 ); }
    bool        IsTransparent() const { return( mcTransparency > 0 ); }
    bool        IsAdjusted() const
                    {
                        return( mnLumPercent != 0 || mnContPercent != 0 || mnRPercent != 0 ||
                                mnGPercent != 0 || mnBPercent != 0 || mfGamma != 1.0 || mbInvert );
                    }

    friend SvStream& operator<<( SvStream& rOStm, const GraphicAttr& rAttr );
    friend SvStream& operator>>( SvStream& rIStm, GraphicAttr& rAttr );
};

typedef ::cppu::WeakImplHelper1< css::graphic::XGraphicObject > GraphicObject_BASE;

class SVT_DLLPUBLIC GraphicObject : public GraphicObject_BASE
{
    sal_uInt32 nRefCount;
    // XGraphicObject
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL getGraphic()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setGraphic( const css::uno::Reference< css::graphic::XGraphic >& xGraphic )
        throw (css::uno::RuntimeException);
    OUString SAL_CALL getUniqueID() throw (css::uno::RuntimeException);

    GraphicObject( void );
    GraphicObject( const GraphicObject* pGraphicObject);
    GraphicObject( const GraphicObject* pGraphicObject, const Graphic& rGraphic);
    GraphicObject( const Graphic& rGraphic);
    GraphicObject( const GraphicObject& rCacheObj);
    GraphicObject& operator=( const GraphicObject& rCacheObj );

    explicit GraphicObject( const OString& rUniqueID);

public: // only for internal access:
    GraphicObject( css::uno::Sequence< css::uno::Any > const & args,
                   css::uno::Reference< css::uno::XComponentContext > const & xComponentContext )
        throw( css::uno::RuntimeException );
    virtual ~GraphicObject();

    static rtl::Reference< GraphicObject > Create(void)
    {
        return rtl::Reference< GraphicObject >( new GraphicObject() );
    }
    static rtl::Reference< GraphicObject > Create( const Graphic& rGraphic)
    {
        return rtl::Reference< GraphicObject >( new GraphicObject( rGraphic) );
    }
    static rtl::Reference< GraphicObject > Create( const GraphicObject& rCacheObj)
    {
        return rtl::Reference< GraphicObject >( new GraphicObject( rCacheObj) );
    }
    static rtl::Reference< GraphicObject > Create( const GraphicObject* pCacheObj)
    {
        return rtl::Reference< GraphicObject >( new GraphicObject( pCacheObj) );
    }
    static rtl::Reference< GraphicObject > Create( const rtl::Reference< GraphicObject > &xCacheObj)
    {
        if( xCacheObj.is() )
            return rtl::Reference< GraphicObject >( new GraphicObject( *xCacheObj.get()) );
        else
            return rtl::Reference< GraphicObject >( new GraphicObject());
    }
    static rtl::Reference< GraphicObject > Create( const rtl::Reference< GraphicObject > &xCacheObj, const Graphic& rGraphic)
    {
        if( xCacheObj.is() )
            return rtl::Reference< GraphicObject >( new GraphicObject( xCacheObj.get(), rGraphic) );
        else
            return rtl::Reference< GraphicObject >( new GraphicObject(rGraphic));
    }
    static rtl::Reference< GraphicObject > Create( const OString& rUniqueID)
    {
        return rtl::Reference< GraphicObject >( new GraphicObject( rUniqueID) );
    }

private:
    Graphic             maGraphic;
    GraphicAttr         maAttr;
    Size                maPrefSize;
    MapMode             maPrefMapMode;
    sal_uLong           mnSizeBytes;
    GraphicType         meType;
    OUString*           mpLink;
    Link*               mpSwapStreamHdl;
    OUString*           mpUserData;
    Timer*              mpSwapOutTimer;
    GrfSimpleCacheObj*  mpSimpleCache;
    sal_uLong           mnAnimationLoopCount;
    bool                mbAutoSwapped   : 1;
    bool                mbTransparent   : 1;
    bool                mbAnimated      : 1;
    bool                mbEPS           : 1;
    bool                mbIsInSwapIn    : 1;
    bool                mbIsInSwapOut   : 1;
    bool                mbAlpha         : 1;

    void                SVT_DLLPRIVATE ImplConstruct();
    void                SVT_DLLPRIVATE ImplAssignGraphicData();
    void                SVT_DLLPRIVATE ImplSetGraphicManager( const OString* pID = NULL,
                                                              const GraphicObject* pCopyObj = NULL);
    void                SVT_DLLPRIVATE ImplAutoSwapIn();
    bool                SVT_DLLPRIVATE ImplIsAutoSwapped() const { return mbAutoSwapped; }
    bool                SVT_DLLPRIVATE ImplGetCropParams( OutputDevice* pOut,
                                                          Point& rPoint,
                                                          Size& rSize,
                                                          const GraphicAttr* pAttr,
                                                          PolyPolygon& rClipPolyPoly,
                                                          bool& bRectClipRegion ) const;

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
                                bool            bEnlarge
                            ) const;

                            DECL_LINK( ImplAutoSwapOutHdl, void* );

protected:

    virtual void            GraphicManagerDestroyed();
    virtual SvStream*       GetSwapStream() const;

//    virtual void            Load( SvStream& );
//    virtual void            Save( SvStream& );
//    virtual void            Assign( const SvDataCopyStream& );

public:

                            TYPEINFO();

//    GraphicObject&          operator=( const GraphicObject& rCacheObj );
    bool                operator==( const GraphicObject& rCacheObj ) const;
    bool                operator!=( const GraphicObject& rCacheObj ) const { return !( *this == rCacheObj ); }

    bool                HasSwapStreamHdl() const { return( mpSwapStreamHdl != NULL && mpSwapStreamHdl->IsSet() ); }
    void                SetSwapStreamHdl();
    void                SetSwapStreamHdl( const Link& rHdl, const sal_uLong nSwapOutTimeout = 0UL );
    sal_uLong           GetSwapOutTimeout() const { return( mpSwapOutTimer ? mpSwapOutTimer->GetTimeout() : 0 ); }

    void                FireSwapInRequest();
    void                FireSwapOutRequest();

    bool                IsCached( OutputDevice* pOut,
                                  const Point& rPoint,
                                  const Size& rSize,
                                  const GraphicAttr* pAttr = NULL,
                                  sal_uLong nFlags = GRFMGR_DRAW_STANDARD ) const;
    void                ReleaseFromCache();

    const Graphic&      GetGraphic() const;
//    void                    SetGraphic( const Graphic& rGraphic, const GraphicObject* pCopyObj = 0);
//    void                    SetGraphic( const Graphic& rGraphic, const String& rLink );

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
    Graphic             GetTransformedGraphic( const Size& rDestSize,
                                               const MapMode& rDestMap,
                                               const GraphicAttr& rAttr ) const;
    Graphic             GetTransformedGraphic( const GraphicAttr* pAttr = NULL ) const; // TODO: Change to Impl

    void                SetAttr( const GraphicAttr& rAttr );
    const GraphicAttr&  GetAttr() const { return maAttr; }

    bool                HasLink() const { return( mpLink != NULL && !mpLink->isEmpty() ); }
    void                SetLink();
    void                SetLink( const OUString& rLink );
    OUString            GetLink() const;

    bool                HasUserData() const { return( mpUserData != NULL && !mpUserData->isEmpty()); }
    void                SetUserData();
    void                SetUserData( const OUString& rUserData );
    OUString            GetUserData() const;

    OString             GetUniqueID() const;

    GraphicType         GetType() const { return meType; }
    const Size&         GetPrefSize() const { return maPrefSize; }
    const MapMode&      GetPrefMapMode() const { return maPrefMapMode; }
    sal_uLong           GetSizeBytes() const { return mnSizeBytes; }
    bool                IsTransparent() const { return mbTransparent; }
    bool                IsAlpha() const { return mbAlpha; }
    bool                IsAnimated() const { return mbAnimated; }
    bool                IsEPS() const { return mbEPS; }

    Link                GetAnimationNotifyHdl() const { return maGraphic.GetAnimationNotifyHdl(); }

    bool                SwapOut();
    bool                SwapOut( SvStream* pOStm );
    bool                SwapIn();

    bool                IsInSwapIn() const { return mbIsInSwapIn; }
    bool                IsInSwapOut() const { return mbIsInSwapOut; }
    bool                IsInSwap() const { return( mbIsInSwapOut || mbIsInSwapOut ); }
    bool                IsSwappedOut() const { return( mbAutoSwapped || maGraphic.IsSwapOut() ); }
    void                SetSwapState();

    bool                Draw( OutputDevice* pOut,
                              const Point& rPoint,
                              const Size& rSize,
                              const GraphicAttr* pAttr = NULL,
                              sal_uLong nFlags = GRFMGR_DRAW_STANDARD );

    bool                DrawWithPDFHandling( OutputDevice& rOutDev,
                                             const Point& rPoint,
                                             const Size& rSize,
                                             const GraphicAttr* pGrfAttr = NULL,
                                             const sal_uLong nFlags = GRFMGR_DRAW_STANDARD );

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
    bool        DrawTiled( OutputDevice* pOut,
                           const Rectangle& rArea,
                           const Size& rSize,
                           const Size& rOffset,
                           const GraphicAttr* pAttr = NULL,
                           sal_uLong nFlags = GRFMGR_DRAW_STANDARD,
                           int nTileCacheSize1D=128 );

    bool        StartAnimation( OutputDevice* pOut,
                                const Point& rPt,
                                const Size& rSz,
                                long nExtraData = 0L,
                                const GraphicAttr* pAttr = NULL,
                                sal_uLong nFlags = GRFMGR_DRAW_STANDARD,
                                OutputDevice* pFirstFrameOutDev = NULL );

    void        StopAnimation( OutputDevice* pOut = NULL, long nExtraData = 0L );

//    friend SvStream&        operator<<( SvStream& rOStm, const GraphicObject& rGraphicObj );
//    friend SvStream&        operator>>( SvStream& rIStm, GraphicObject& rGraphicObj );

    static rtl::Reference<GraphicObject> CreateGraphicObjectFromURL( const OUString &rURL );
    // will inspect an object ( e.g. a control ) for any 'ImageURL'
    // properties and return these in a vector. Note: this implementation
    // will cater for XNameContainer objects and deepinspect any containees
    // if they exist
    static void InspectForGraphicObjectImageURL( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIf, std::vector< OUString >& rvEmbedImgUrls );

    // create CropScaling information
    // fWidth, fHeight: object size
    // f*Crop: crop values relative to original bitmap size
    basegfx::B2DVector calculateCropScaling( double fWidth,
                                             double fHeight,
                                             double fLeftCrop,
                                             double fTopCrop,
                                             double fRightCrop,
                                             double fBottomCrop) const;
};

#endif // _GRFMGR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
