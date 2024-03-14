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

#pragma once

#include <memory>
#include <string_view>
#include <vcl/graph.hxx>
#include <vcl/dllapi.h>
#include <o3tl/typed_flags_set.hxx>

#include <vcl/GraphicAttributes.hxx>

namespace com::sun::star::graphic { class XGraphic; }

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

class VirtualDevice;
struct GrfSimpleCacheObj;
struct ImplTileInfo;

class VCL_DLLPUBLIC GraphicObject
{
    friend class SdrGrafObj;

private:
    Graphic                 maGraphic;
    GraphicAttr             maAttr;
    OUString                maUserData;
    std::unique_ptr<GrfSimpleCacheObj> mxSimpleCache;

    bool                    SAL_DLLPRIVATE ImplGetCropParams(
                                const OutputDevice& rOut,
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
    bool SAL_DLLPRIVATE     ImplRenderTempTile(
                                VirtualDevice& rVDev,
                                int nNumTilesX,
                                int nNumTilesY,
                                const Size& rTileSizePixel,
                                const GraphicAttr* pAttr
                            );

    /// internally called by ImplRenderTempTile()
    bool SAL_DLLPRIVATE     ImplRenderTileRecursive(
                                VirtualDevice& rVDev,
                                int nExponent,
                                int nMSBFactor,
                                int nNumOrigTilesX,
                                int nNumOrigTilesY,
                                int nRemainderTilesX,
                                int nRemainderTilesY,
                                const Size& rTileSizePixel,
                                const GraphicAttr* pAttr,
                                ImplTileInfo& rTileInfo
                            );

    bool SAL_DLLPRIVATE     ImplDrawTiled(
                                OutputDevice& rOut,
                                const tools::Rectangle& rArea,
                                const Size& rSizePixel,
                                const Size& rOffset,
                                const GraphicAttr* pAttr,
                                int nTileCacheSize1D
                            );

    bool SAL_DLLPRIVATE     ImplDrawTiled(
                                OutputDevice& rOut,
                                const Point& rPos,
                                int nNumTilesX,
                                int nNumTilesY,
                                const Size& rTileSize,
                                const GraphicAttr* pAttr
                            ) const;

    void SAL_DLLPRIVATE     ImplTransformBitmap(
                                BitmapEx&           rBmpEx,
                                const GraphicAttr&  rAttr,
                                const Size&         rCropLeftTop,
                                const Size&         rCropRightBottom,
                                const tools::Rectangle&    rCropRect,
                                const Size&         rDstSize,
                                bool                bEnlarge
                            ) const;

public:
                            GraphicObject();
                            GraphicObject( Graphic aGraphic );
                            GraphicObject( const GraphicObject& rCacheObj );
                            ~GraphicObject();

    GraphicObject&          operator=( const GraphicObject& rCacheObj );
    bool                    operator==( const GraphicObject& rCacheObj ) const;
    bool                    operator!=( const GraphicObject& rCacheObj ) const { return !( *this == rCacheObj ); }

    const Graphic&          GetGraphic() const;
    void                    SetGraphic( const Graphic& rGraphic);

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
    Graphic                 GetTransformedGraphic( const GraphicAttr* pAttr ) const; // TODO: Change to Impl

    void                    SetAttr( const GraphicAttr& rAttr );
    const GraphicAttr&      GetAttr() const { return maAttr; }

    bool                    HasUserData() const { return !maUserData.isEmpty(); }
    void                    SetUserData();
    void                    SetUserData( const OUString& rUserData );
    const OUString&         GetUserData() const { return maUserData; }

    OString                 GetUniqueID() const;

    GraphicType             GetType() const;
    Size                    GetPrefSize() const;
    MapMode                 GetPrefMapMode() const;
    bool                    IsTransparent() const;
    bool                    IsAnimated() const;
    bool                    IsEPS() const;

    bool                    Draw(OutputDevice& rOut,
                                 const Point& rPt,
                                 const Size& rSz,
                                 const GraphicAttr* pAttr = nullptr) const;

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

        @param nFlags
        Optional rendering flags

        @param nTileCacheSize1D
        Optional dimension of the generated cache tiles. The pOut sees
        a number of tile draws, which have approximately
        nTileCacheSize1D times nTileCacheSize1D bitmap sizes if the
        tile bitmap is smaller. Otherwise, the tile is drawn as
        is. This is useful if e.g. you want only a few, very large
        bitmap drawings appear on the outdev.
     */
    void                    DrawTiled(
                                OutputDevice& rOut,
                                const tools::Rectangle& rArea,
                                const Size& rSize,
                                const Size& rOffset,
                                int nTileCacheSize1D=128
                            );

    bool                    StartAnimation(
                                OutputDevice& rOut,
                                const Point& rPt,
                                const Size& rSz,
                                tools::Long nExtraData = 0,
                                OutputDevice* pFirstFrameOutDev = nullptr
                            );

    void                    StopAnimation( const OutputDevice* pOut = nullptr, tools::Long nExtraData = 0 );

    static bool isGraphicObjectUniqueIdURL(std::u16string_view rURL);

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
};

namespace vcl::graphic
{

// Will search an object ( e.g. a control ) for any 'ImageURL' or 'Graphic'
// properties and return graphics from the properties in a vector. ImageURL
// will be loaded from the URL.
//
// Note: this implementation will cater for XNameContainer objects and deep inspect any containers
// if they exist

VCL_DLLPUBLIC void SearchForGraphics(css::uno::Reference<css::uno::XInterface> const & rxInterface,
                                     std::vector<css::uno::Reference<css::graphic::XGraphic>> & raGraphicList);


} // end namespace vcl::graphic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
