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

#ifndef _SV_IMPGRAPH_HXX
#define _SV_IMPGRAPH_HXX

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/animate.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.h>
#include <vcl/svgdata.hxx>

// ---------------
// - ImpSwapInfo -
// ---------------

struct ImpSwapInfo
{
    MapMode     maPrefMapMode;
    Size        maPrefSize;
};

// --------------
// - ImpGraphic -
// --------------

class   OutputDevice;
class   GfxLink;
struct  ImpSwapFile;
class GraphicConversionParameters;

class ImpGraphic
{
    friend class Graphic;

private:

    GDIMetaFile         maMetaFile;
    BitmapEx            maEx;
    ImpSwapInfo         maSwapInfo;
    Animation*          mpAnimation;
    GraphicReader*      mpContext;
    ImpSwapFile*        mpSwapFile;
    GfxLink*            mpGfxLink;
    GraphicType         meType;
    OUString            maDocFileURLStr;
    sal_uLong           mnDocFilePos;
    mutable sal_uLong   mnSizeBytes;
    sal_uLong           mnRefCount;
    sal_Bool            mbSwapOut;
    sal_Bool            mbSwapUnderway;

    // SvgData support
    SvgDataPtr          maSvgData;

private:

                        ImpGraphic();
                        ImpGraphic( const ImpGraphic& rImpGraphic );
                        ImpGraphic( const Bitmap& rBmp );
                        ImpGraphic( const BitmapEx& rBmpEx );
                        ImpGraphic(const SvgDataPtr& rSvgDataPtr);
                        ImpGraphic( const Animation& rAnimation );
                        ImpGraphic( const GDIMetaFile& rMtf );
    virtual             ~ImpGraphic();

    ImpGraphic&         operator=( const ImpGraphic& rImpGraphic );
    sal_Bool                operator==( const ImpGraphic& rImpGraphic ) const;
    sal_Bool                operator!=( const ImpGraphic& rImpGraphic ) const { return !( *this == rImpGraphic ); }

    void                ImplClearGraphics( sal_Bool bCreateSwapInfo );
    void                ImplClear();

    GraphicType         ImplGetType() const;
    void                ImplSetDefaultType();
    sal_Bool                ImplIsSupportedGraphic() const;

    sal_Bool            ImplIsTransparent() const;
    sal_Bool            ImplIsAlpha() const;
    sal_Bool            ImplIsAnimated() const;
    sal_Bool            ImplIsEPS() const;

    Bitmap                  ImplGetBitmap(const GraphicConversionParameters& rParameters) const;
    BitmapEx                ImplGetBitmapEx(const GraphicConversionParameters& rParameters) const;
    Animation               ImplGetAnimation() const;
    const GDIMetaFile&      ImplGetGDIMetaFile() const;


    Size                ImplGetPrefSize() const;
    void                ImplSetPrefSize( const Size& rPrefSize );

    MapMode             ImplGetPrefMapMode() const;
    void                ImplSetPrefMapMode( const MapMode& rPrefMapMode );

    sal_uLong               ImplGetSizeBytes() const;

    void                ImplDraw( OutputDevice* pOutDev,
                                  const Point& rDestPt ) const;
    void                ImplDraw( OutputDevice* pOutDev,
                                  const Point& rDestPt,
                                  const Size& rDestSize ) const;

    void                ImplStartAnimation( OutputDevice* pOutDev,
                                            const Point& rDestPt,
                                            const Size& rDestSize,
                                            long nExtraData = 0,
                                            OutputDevice* pFirstFrameOutDev = NULL );
    void                ImplStopAnimation( OutputDevice* pOutputDevice = NULL,
                                           long nExtraData = 0 );

    void                ImplSetAnimationNotifyHdl( const Link& rLink );
    Link                ImplGetAnimationNotifyHdl() const;

    sal_uLong               ImplGetAnimationLoopCount() const;

private:

    GraphicReader*      ImplGetContext();
    void                ImplSetContext( GraphicReader* pReader );

private:

    void                ImplSetDocFileName( const OUString& rName, sal_uLong nFilePos );
    const OUString&     ImplGetDocFileName() const;
    sal_uLong               ImplGetDocFilePos() const;

    sal_Bool                ImplReadEmbedded( SvStream& rIStream, sal_Bool bSwap = sal_False );
    sal_Bool                ImplWriteEmbedded( SvStream& rOStream );

    sal_Bool                ImplSwapIn();
    sal_Bool                ImplSwapIn( SvStream* pIStm );

    sal_Bool                ImplSwapOut();
    sal_Bool                ImplSwapOut( SvStream* pOStm );

    sal_Bool                ImplIsSwapOut() const;

    void                ImplSetLink( const GfxLink& );
    GfxLink             ImplGetLink();
    sal_Bool                ImplIsLink() const;

    sal_uLong               ImplGetChecksum() const;

    sal_Bool                ImplExportNative( SvStream& rOStm ) const;

    friend SvStream&    operator<<( SvStream& rOStm, const ImpGraphic& rImpGraphic );
    friend SvStream&    operator>>( SvStream& rIStm, ImpGraphic& rImpGraphic );

    // SvgData support
    const SvgDataPtr& getSvgData() const;
};

#endif // _SV_IMPGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
