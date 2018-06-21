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
    std::shared_ptr<GfxLink>     mpGfxLink;
    GraphicType                  meType;
    mutable sal_uLong            mnSizeBytes;
    bool                         mbSwapOut;
    bool                         mbDummyContext;
    VectorGraphicDataPtr         maVectorGraphicData;

    /// The PDF stream from which this Graphic is rendered,
    /// as converted (version downgraded) from the original,
    /// which should be in GfxLink.
    std::shared_ptr<css::uno::Sequence<sal_Int8>> mpPdfData;
    OUString msOriginURL;
    GraphicExternalLink          maGraphicExternalLink;

    /// Used with GfxLink and/or PdfData when they store original media
    /// which might be multi-page (PDF, f.e.) and we need to re-render
    /// this Graphic (a page) from the source in GfxLink or PdfData.
    sal_Int32                    mnPageNumber;

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
        return maGraphicExternalLink.msURL;
    }

    void setOriginURL(OUString const & rOriginURL)
    {
        maGraphicExternalLink.msURL = rOriginURL;
    }

    bool hasPdfData() const
    {
        return mpPdfData && mpPdfData->hasElements();
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
    void                ImplSetSharedLink(const std::shared_ptr<GfxLink>& pGfxLink);
    GfxLink             ImplGetLink();
    bool                ImplIsLink() const;

    BitmapChecksum      ImplGetChecksum() const;

    bool                ImplExportNative( SvStream& rOStm ) const;

    friend void         WriteImpGraphic(SvStream& rOStm, const ImpGraphic& rImpGraphic);
    friend void         ReadImpGraphic(SvStream& rIStm, ImpGraphic& rImpGraphic);

    const VectorGraphicDataPtr& getVectorGraphicData() const { return maVectorGraphicData; }

    const std::shared_ptr<css::uno::Sequence<sal_Int8>>& getPdfData() const;

    void setPdfData(const std::shared_ptr<css::uno::Sequence<sal_Int8>>& rPdfData);
};

#endif // INCLUDED_VCL_INC_IMPGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
