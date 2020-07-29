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

#ifndef INCLUDED_VCL_INC_IMPGRAPH_HXX
#define INCLUDED_VCL_INC_IMPGRAPH_HXX

#include <vcl/GraphicExternalLink.hxx>
#include "graphic/Manager.hxx"

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
struct ImpSwapFile;
class GraphicConversionParameters;
class ImpGraphic;

class GraphicID
{
private:
    sal_uInt32  mnID1;
    sal_uInt32  mnID2;
    sal_uInt32  mnID3;
    BitmapChecksum  mnID4;

public:
    GraphicID(ImpGraphic const & rGraphic);

    bool operator==(const GraphicID& rID) const
    {
        return rID.mnID1 == mnID1 && rID.mnID2 == mnID2 &&
               rID.mnID3 == mnID3 && rID.mnID4 == mnID4;
    }

    OString getIDString() const;
};

class ImpGraphic final
{
    friend class Graphic;
    friend class GraphicID;
    friend class vcl::graphic::Manager;

private:

    GDIMetaFile                  maMetaFile;
    BitmapEx                     maEx;
    ImpSwapInfo                  maSwapInfo;
    std::unique_ptr<Animation>   mpAnimation;
    std::shared_ptr<GraphicReader> mpContext;
    std::shared_ptr<ImpSwapFile> mpSwapFile;
    std::shared_ptr<GfxLink>     mpGfxLink;
    GraphicType                  meType;
    mutable sal_uLong            mnSizeBytes;
    bool                         mbSwapOut;
    bool                         mbDummyContext;
    VectorGraphicDataPtr         maVectorGraphicData;
    // cache checksum computation
    mutable BitmapChecksum       mnChecksum = 0;

    std::unique_ptr<GraphicID>   mpGraphicID;
    GraphicExternalLink          maGraphicExternalLink;

    std::chrono::high_resolution_clock::time_point maLastUsed;
    bool mbPrepared;

public:
    ImpGraphic();
    ImpGraphic( const ImpGraphic& rImpGraphic );
    ImpGraphic( ImpGraphic&& rImpGraphic );
    ImpGraphic( const GraphicExternalLink& rExternalLink);
    ImpGraphic( const Bitmap& rBmp );
    ImpGraphic( const BitmapEx& rBmpEx );
    ImpGraphic(const VectorGraphicDataPtr& rVectorGraphicDataPtr);
    ImpGraphic( const Animation& rAnimation );
    ImpGraphic( const GDIMetaFile& rMtf );
    ~ImpGraphic();

    void ImplSetPrepared(bool bAnimated, Size* pSizeHint);

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

    void                ImplCreateSwapInfo();
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
                                            long nExtraData,
                                            OutputDevice* pFirstFrameOutDev );
    void                ImplStopAnimation( OutputDevice* pOutputDevice,
                                           long nExtraData );

    void                ImplSetAnimationNotifyHdl( const Link<Animation*,void>& rLink );
    Link<Animation*,void> ImplGetAnimationNotifyHdl() const;

    sal_uInt32          ImplGetAnimationLoopCount() const;

private:

    std::shared_ptr<GraphicReader>& ImplGetContext() { return mpContext;}
    void                ImplSetContext( const std::shared_ptr<GraphicReader>& pReader );
    void                ImplSetDummyContext( bool value ) { mbDummyContext = value; }
    bool                ImplReadEmbedded( SvStream& rIStream );
    bool                ImplWriteEmbedded( SvStream& rOStream );

    bool                ImplSwapIn();
    bool                ImplSwapIn( SvStream* pIStm );

    bool                ImplSwapOut();
    bool                ImplSwapOut( SvStream* pOStm );

    bool                ImplIsSwapOut() const { return mbSwapOut;}
    bool                ImplIsDummyContext() const { return mbDummyContext; }
    void                ImplSetLink( const std::shared_ptr<GfxLink>& );
    std::shared_ptr<GfxLink> ImplGetSharedGfxLink() const;
    GfxLink             ImplGetLink();
    bool                ImplIsLink() const;

    BitmapChecksum      ImplGetChecksum() const;

    bool                ImplExportNative( SvStream& rOStm ) const;

    friend void         WriteImpGraphic(SvStream& rOStm, const ImpGraphic& rImpGraphic);
    friend void         ReadImpGraphic(SvStream& rIStm, ImpGraphic& rImpGraphic);

    const VectorGraphicDataPtr& getVectorGraphicData() const;

    bool ensureAvailable () const;

    bool loadPrepared();

    sal_Int32 getPageNumber() const;
};

#endif // INCLUDED_VCL_INC_IMPGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
