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
    mutable sal_uLong            mnSizeBytes;
    bool                         mbSwapOut;
    bool                         mbDummyContext;
    std::shared_ptr<VectorGraphicData> maVectorGraphicData;
    // cache checksum computation
    mutable BitmapChecksum       mnChecksum = 0;

    std::unique_ptr<GraphicID>   mpGraphicID;
    GraphicExternalLink          maGraphicExternalLink;

    std::chrono::high_resolution_clock::time_point maLastUsed;
    bool mbPrepared;

public:
    ImpGraphic();
    ImpGraphic( const ImpGraphic& rImpGraphic );
    ImpGraphic( ImpGraphic&& rImpGraphic ) noexcept;
    ImpGraphic( const GraphicExternalLink& rExternalLink);
    ImpGraphic( const BitmapEx& rBmpEx );
    ImpGraphic(const std::shared_ptr<VectorGraphicData>& rVectorGraphicDataPtr);
    ImpGraphic( const Animation& rAnimation );
    ImpGraphic( const GDIMetaFile& rMtf );
    ~ImpGraphic();

    void ImplSetPrepared(bool bAnimated, const Size* pSizeHint);

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
        if (!mpGraphicID)
            mpGraphicID.reset(new GraphicID(*this));
        return mpGraphicID->getIDString();
    }

    void createSwapInfo();
    void restoreFromSwapInfo();

    void                ImplClearGraphics();
    void                ImplClear();

    GraphicType         ImplGetType() const { return meType;}
    void                ImplSetDefaultType();
    bool                ImplIsSupportedGraphic() const;

    bool                ImplIsTransparent() const;
    bool                ImplIsAlpha() const;
    bool                ImplIsAnimated() const;
    bool                ImplIsEPS() const;

    bool isAvailable() const;
    bool makeAvailable();

    Bitmap              ImplGetBitmap(const GraphicConversionParameters& rParameters) const;
    BitmapEx            ImplGetBitmapEx(const GraphicConversionParameters& rParameters) const;
    /// Gives direct access to the contained BitmapEx.
    const BitmapEx&     ImplGetBitmapExRef() const;
    Animation           ImplGetAnimation() const;
    const GDIMetaFile&  ImplGetGDIMetaFile() const;

    Size                ImplGetSizePixel() const;

    Size                ImplGetPrefSize() const;
    void                ImplSetPrefSize( const Size& rPrefSize );

    MapMode             ImplGetPrefMapMode() const;
    void                ImplSetPrefMapMode( const MapMode& rPrefMapMode );

    sal_uLong           ImplGetSizeBytes() const;

    void                ImplDraw( OutputDevice* pOutDev,
                                  const Point& rDestPt ) const;
    void                ImplDraw( OutputDevice* pOutDev,
                                  const Point& rDestPt,
                                  const Size& rDestSize ) const;

    void                ImplStartAnimation( OutputDevice* pOutDev,
                                            const Point& rDestPt,
                                            const Size& rDestSize,
                                            tools::Long nExtraData,
                                            OutputDevice* pFirstFrameOutDev );
    void                ImplStopAnimation( const OutputDevice* pOutputDevice,
                                           tools::Long nExtraData );

    void                ImplSetAnimationNotifyHdl( const Link<Animation*,void>& rLink );
    Link<Animation*,void> ImplGetAnimationNotifyHdl() const;

    sal_uInt32          ImplGetAnimationLoopCount() const;

private:
    // swapping methods
    bool swapInFromStream(SvStream& rStream);
    bool swapInGraphic(SvStream& rStream);

    bool swapInContent(SvStream& rStream);
    bool swapOutContent(SvStream& rStream);
    bool swapOutGraphic(SvStream& rStream);
    // end swapping

    std::shared_ptr<GraphicReader>& ImplGetContext() { return mpContext;}
    void                ImplSetContext( const std::shared_ptr<GraphicReader>& pReader );
    void                ImplSetDummyContext( bool value ) { mbDummyContext = value; }
    bool                ImplIsDummyContext() const { return mbDummyContext; }
    void                ImplSetLink( const std::shared_ptr<GfxLink>& );
    std::shared_ptr<GfxLink> ImplGetSharedGfxLink() const;
    GfxLink             ImplGetLink();
    bool                ImplIsLink() const;

    BitmapChecksum      ImplGetChecksum() const;

    bool                ImplExportNative( SvStream& rOStm ) const;

    friend void         WriteImpGraphic(SvStream& rOStm, const ImpGraphic& rImpGraphic);
    friend void         ReadImpGraphic(SvStream& rIStm, ImpGraphic& rImpGraphic);

    const std::shared_ptr<VectorGraphicData>& getVectorGraphicData() const;

    /// Gets the bitmap replacement for a vector graphic.
    BitmapEx getVectorGraphicReplacement() const;

    bool ensureAvailable () const;

    bool loadPrepared();

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
    void updateFromLoadedGraphic(ImpGraphic* graphic);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
