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

#include <vcl/dllapi.h>
#include <vcl/GraphicExternalLink.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include "graphic/Manager.hxx"
#include "graphic/GraphicID.hxx"
#include <optional>

struct ImpSwapInfo
{
    MapMode     maPrefMapMode;
    Size        maPrefSize;
    Size        maSizePixel;

    bool mbIsAnimated;
    bool mbIsEPS;
    bool mbIsTransparent;
    bool mbIsAlpha;

    sal_uInt32 mnAnimationLoopCount;
    sal_Int32 mnPageIndex;
};

class OutputDevice;
class GfxLink;
class ImpSwapFile;
class GraphicConversionParameters;
class ImpGraphic;

enum class GraphicContentType : sal_Int32
{
    Bitmap,
    Animation,
    Vector
};

class VCL_DLLPUBLIC ImpGraphic final
{
    friend class Graphic;
    friend class GraphicID;
    friend class vcl::graphic::Manager;

private:

    GDIMetaFile                  maMetaFile;
    BitmapEx                     maBitmapEx;
    /// If maBitmapEx is empty, this preferred size will be set on it when it gets initialized.
    Size                         maExPrefSize;
    ImpSwapInfo                  maSwapInfo;
    std::unique_ptr<Animation>   mpAnimation;
    std::shared_ptr<GraphicReader> mpContext;
    std::shared_ptr<ImpSwapFile> mpSwapFile;
    std::shared_ptr<GfxLink>     mpGfxLink;
    GraphicType                  meType;
    mutable sal_uInt32           mnSizeBytes;
    bool                         mbSwapOut;
    bool                         mbDummyContext;
    std::shared_ptr<VectorGraphicData> maVectorGraphicData;
    // cache checksum computation
    mutable BitmapChecksum       mnChecksum = 0;

    std::optional<GraphicID>     mxGraphicID;
    GraphicExternalLink          maGraphicExternalLink;

    std::chrono::high_resolution_clock::time_point maLastUsed;
    bool mbPrepared;

public:
    ImpGraphic();
    ImpGraphic( const ImpGraphic& rImpGraphic );
    ImpGraphic( ImpGraphic&& rImpGraphic ) noexcept;
    ImpGraphic( const GraphicExternalLink& rExternalLink);
    ImpGraphic(std::shared_ptr<GfxLink> const& rGfxLink, sal_Int32 nPageIndex = 0);
    ImpGraphic( const BitmapEx& rBmpEx );
    ImpGraphic(const std::shared_ptr<VectorGraphicData>& rVectorGraphicDataPtr);
    ImpGraphic( const Animation& rAnimation );
    ImpGraphic( const GDIMetaFile& rMtf );
    ~ImpGraphic();

    void setPrepared(bool bAnimated, const Size* pSizeHint);

private:

    ImpGraphic&         operator=( const ImpGraphic& rImpGraphic );
    ImpGraphic&         operator=( ImpGraphic&& rImpGraphic );
    bool                operator==( const ImpGraphic& rImpGraphic ) const;
    bool                operator!=( const ImpGraphic& rImpGraphic ) const { return !( *this == rImpGraphic ); }

    OUString const & getOriginURL() const
    {
        return maGraphicExternalLink.msURL;
    }

    void setOriginURL(OUString const & rOriginURL)
    {
        maGraphicExternalLink.msURL = rOriginURL;
    }

    OString getUniqueID()
    {
        if (!mxGraphicID)
            mxGraphicID.emplace(*this);
        return mxGraphicID->getIDString();
    }

    void createSwapInfo();
    void restoreFromSwapInfo();

    void                clearGraphics();
    void                clear();

    GraphicType         getType() const { return meType;}
    void                setDefaultType();
    bool                isSupportedGraphic() const;

    bool                isTransparent() const;
    bool                isAlpha() const;
    bool                isAnimated() const;
    bool                isEPS() const;

    bool isAvailable() const;
    bool makeAvailable();

    Bitmap              getBitmap(const GraphicConversionParameters& rParameters) const;
    BitmapEx            getBitmapEx(const GraphicConversionParameters& rParameters) const;
    /// Gives direct access to the contained BitmapEx.
    const BitmapEx&     getBitmapExRef() const;
    Animation           getAnimation() const;
    const GDIMetaFile&  getGDIMetaFile() const;

    Size                getSizePixel() const;

    Size                getPrefSize() const;
    void                setPrefSize( const Size& rPrefSize );

    MapMode             getPrefMapMode() const;
    void                setPrefMapMode( const MapMode& rPrefMapMode );

    sal_uInt32          getSizeBytes() const;

    void                draw(OutputDevice& rOutDev, const Point& rDestPt) const;
    void                draw(OutputDevice& rOutDev, const Point& rDestPt,
                             const Size& rDestSize) const;

    void                startAnimation(OutputDevice& rOutDev,
                                       const Point& rDestPt,
                                       const Size& rDestSize,
                                       tools::Long nExtraData,
                                       OutputDevice* pFirstFrameOutDev);
    void                stopAnimation( const OutputDevice* pOutputDevice,
                                           tools::Long nExtraData );

    void                setAnimationNotifyHdl( const Link<Animation*,void>& rLink );
    Link<Animation*,void> getAnimationNotifyHdl() const;

    sal_uInt32          getAnimationLoopCount() const;

private:
    // swapping methods
    bool swapInFromStream(SvStream& rStream);
    bool swapInGraphic(SvStream& rStream);

    bool swapInContent(SvStream& rStream);
    bool swapOutContent(SvStream& rStream);
    bool swapOutGraphic(SvStream& rStream);
    // end swapping

    std::shared_ptr<GraphicReader>& getContext() { return mpContext;}
    void                setContext( const std::shared_ptr<GraphicReader>& pReader );
    void                setDummyContext( bool value ) { mbDummyContext = value; }
    bool                isDummyContext() const { return mbDummyContext; }
    void                setGfxLink( const std::shared_ptr<GfxLink>& );
    const std::shared_ptr<GfxLink> & getSharedGfxLink() const;
    GfxLink             getGfxLink() const;
    bool                isGfxLink() const;

    BitmapChecksum      getChecksum() const;

    const std::shared_ptr<VectorGraphicData>& getVectorGraphicData() const;

    /// Gets the bitmap replacement for a vector graphic.
    BitmapEx getVectorGraphicReplacement() const;

    bool ensureAvailable () const;

    sal_Int32 getPageNumber() const;

    // Set the pref size, but don't force swap-in
    void setValuesForPrefSize(const Size& rPrefSize);
    // Set the pref map mode, but don't force swap-in
    void setValuesForPrefMapMod(const MapMode& rPrefMapMode);

public:
    void resetChecksum() { mnChecksum = 0; }
    bool swapIn();
    bool swapOut();
    bool isSwappedOut() const { return mbSwapOut; }
    OUString getSwapFileURL() const;
    // public only because of use in GraphicFilter
    void updateFromLoadedGraphic(const ImpGraphic* graphic);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
