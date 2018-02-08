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

struct ImpSwapInfo
{
    MapMode     maPrefMapMode;
    Size        maPrefSize;
};

class OutputDevice;
class GfxLink;
struct ImpSwapFile;
class GraphicConversionParameters;

class ImpGraphic final
{
    friend class Graphic;

private:

    GDIMetaFile                  maMetaFile;
    BitmapEx                     maEx;
    ImpSwapInfo                  maSwapInfo;
    std::unique_ptr<Animation>   mpAnimation;
    std::shared_ptr<GraphicReader> mpContext;
    std::shared_ptr<ImpSwapFile> mpSwapFile;
    std::unique_ptr<GfxLink>     mpGfxLink;
    GraphicType                  meType;
    mutable sal_uLong            mnSizeBytes;
    bool                         mbSwapOut;
    bool                         mbDummyContext;
    VectorGraphicDataPtr         maVectorGraphicData;
    css::uno::Sequence<sal_Int8> maPdfData;
    OUString msOriginURL;

private:

                        ImpGraphic();
                        ImpGraphic( const ImpGraphic& rImpGraphic );
                        ImpGraphic( ImpGraphic&& rImpGraphic );
                        ImpGraphic( const Bitmap& rBmp );
                        ImpGraphic( const BitmapEx& rBmpEx );
                        ImpGraphic(const VectorGraphicDataPtr& rVectorGraphicDataPtr);
                        ImpGraphic( const Animation& rAnimation );
                        ImpGraphic( const GDIMetaFile& rMtf );
public:
                        ~ImpGraphic();
private:

    ImpGraphic&         operator=( const ImpGraphic& rImpGraphic );
    ImpGraphic&         operator=( ImpGraphic&& rImpGraphic );
    bool                operator==( const ImpGraphic& rImpGraphic ) const;
    bool                operator!=( const ImpGraphic& rImpGraphic ) const { return !( *this == rImpGraphic ); }

    OUString getOriginURL() const
    {
        return msOriginURL;
    }

    void setOriginURL(OUString const & rOriginURL)
    {
        msOriginURL = rOriginURL;
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

    Bitmap              ImplGetBitmap(const GraphicConversionParameters& rParameters) const;
    BitmapEx            ImplGetBitmapEx(const GraphicConversionParameters& rParameters) const;
    /// Gives direct access to the contained BitmapEx.
    const BitmapEx&     ImplGetBitmapExRef() const;
    Animation           ImplGetAnimation() const;
    const GDIMetaFile&  ImplGetGDIMetaFile() const;

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
    void                ImplSwapOutAsLink();
    bool                ImplSwapOut( SvStream* pOStm );

    bool                ImplIsSwapOut() const { return mbSwapOut;}
    bool                ImplIsDummyContext() const { return mbDummyContext; }
    void                ImplSetLink( const GfxLink& );
    GfxLink             ImplGetLink();
    bool                ImplIsLink() const;

    BitmapChecksum      ImplGetChecksum() const;

    bool                ImplExportNative( SvStream& rOStm ) const;

    friend void         WriteImpGraphic(SvStream& rOStm, const ImpGraphic& rImpGraphic);
    friend void         ReadImpGraphic(SvStream& rIStm, ImpGraphic& rImpGraphic);

    const VectorGraphicDataPtr& getVectorGraphicData() const { return maVectorGraphicData; }
};

#endif // INCLUDED_VCL_INC_IMPGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
