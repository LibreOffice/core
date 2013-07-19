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

#ifndef _SV_GRAPH_HXX
#define _SV_GRAPH_HXX

#include <tools/stream.hxx>
#include <vcl/dllapi.h>
#include <tools/solar.h>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/animate.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.h>
#include <vcl/gfxlink.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <vcl/svgdata.hxx>

namespace com { namespace sun { namespace star { namespace graphic { class XGraphic;} } } }

// -----------
// - Graphic -
// -----------

class   ImpGraphic;
class   OutputDevice;
class   Font;
class   GfxLink;

class VCL_DLLPUBLIC GraphicConversionParameters
{
private:
    Size            maSizePixel;            // default is (0,0)

    // bitfield
    unsigned        mbUnlimitedSize : 1;    // default is false
    unsigned        mbAntiAliase : 1;       // default is false
    unsigned        mbSnapHorVerLines : 1;  // default is false
    unsigned        mbScaleHighQuality : 1; // default is false

public:
    GraphicConversionParameters(
        const Size& rSizePixel = Size(),
        bool bUnlimitedSize = false,
        bool bAntiAliase = false,
        bool bSnapHorVerLines = false,
        bool bScaleHighQuality = false)
    :   maSizePixel(rSizePixel),
        mbUnlimitedSize(bUnlimitedSize),
        mbAntiAliase(bAntiAliase),
        mbSnapHorVerLines(bSnapHorVerLines),
        mbScaleHighQuality(bScaleHighQuality)
    {
    }

    // data read access
    const Size getSizePixel() const { return maSizePixel; }
    bool getUnlimitedSize() const { return mbUnlimitedSize; }
    bool getAntiAliase() const { return mbAntiAliase; }
    bool getSnapHorVerLines() const { return mbSnapHorVerLines; }
    bool getScaleHighQuality() const { return mbScaleHighQuality; }
};

class VCL_DLLPUBLIC Graphic : public SvDataCopyStream
{
private:

    ImpGraphic*         mpImpGraphic;

public:

    SAL_DLLPRIVATE void         ImplTestRefCount();
    SAL_DLLPRIVATE ImpGraphic*  ImplGetImpGraphic() const { return mpImpGraphic; }

public:

                        TYPEINFO();

                        Graphic();
                        Graphic( const Graphic& rGraphic );
                        Graphic( const Bitmap& rBmp );
                        Graphic( const BitmapEx& rBmpEx );
                        Graphic( const SvgDataPtr& rSvgDataPtr );
                        Graphic( const Animation& rAnimation );
                        Graphic( const GDIMetaFile& rMtf );
                        Graphic( const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rxGraphic );
    virtual             ~Graphic();

    Graphic&            operator=( const Graphic& rGraphic );
    sal_Bool                operator==( const Graphic& rGraphic ) const;
    sal_Bool                operator!=( const Graphic& rGraphic ) const;
    sal_Bool                operator!() const;

    void                Clear();

    GraphicType         GetType() const;
    void                SetDefaultType();
    sal_Bool                IsSupportedGraphic() const;

    sal_Bool            IsTransparent() const;
    sal_Bool            IsAlpha() const;
    sal_Bool            IsAnimated() const;
    sal_Bool            IsEPS() const;

    // #i102089# Access of Bitmap potentially will have to rasterconvert the Graphic
    // if it is a MetaFile. To be able to control this conversion it is necessary to
    // allow giving parameters which control AntiAliasing and LineSnapping of the
    // MetaFile when played. Defaults will use a no-AAed, not snapped conversion as
    // before.
    Bitmap              GetBitmap(const GraphicConversionParameters& rParameters = GraphicConversionParameters()) const;
    BitmapEx            GetBitmapEx(const GraphicConversionParameters& rParameters = GraphicConversionParameters()) const;

    Animation               GetAnimation() const;
    const GDIMetaFile&      GetGDIMetaFile() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > GetXGraphic() const;

    Size                GetPrefSize() const;
    void                SetPrefSize( const Size& rPrefSize );

    MapMode             GetPrefMapMode() const;
    void                SetPrefMapMode( const MapMode& rPrefMapMode );

    Size                GetSizePixel( const OutputDevice* pRefDevice = NULL ) const;

    sal_uLong               GetSizeBytes() const;

    void                Draw( OutputDevice* pOutDev,
                              const Point& rDestPt ) const;
    void                Draw( OutputDevice* pOutDev,
                              const Point& rDestPt,
                              const Size& rDestSize ) const;
    static void         DrawEx( OutputDevice* pOutDev, const OUString& rText,
                              Font& rFont, const BitmapEx& rBitmap,
                              const Point& rDestPt, const Size& rDestSize );

    void                StartAnimation( OutputDevice* pOutDev,
                                        const Point& rDestPt,
                                        const Size& rDestSize,
                                        long nExtraData = 0L,
                                        OutputDevice* pFirstFrameOutDev = NULL );
    void                StopAnimation( OutputDevice* pOutputDevice = NULL,
                                       long nExtraData = 0L );

    void                SetAnimationNotifyHdl( const Link& rLink );
    Link                GetAnimationNotifyHdl() const;

    sal_uLong               GetAnimationLoopCount() const;

    sal_uLong               GetChecksum() const;

public:

    GraphicReader*      GetContext();
    void                SetContext( GraphicReader* pReader );

public:

    void                SetDocFileName( const OUString& rName, sal_uLong nFilePos );
    const OUString&       GetDocFileName() const;
    sal_uLong               GetDocFilePos() const;

    sal_Bool                SwapOut();
    sal_Bool                SwapOut( SvStream* pOStm );
    sal_Bool                SwapIn();
    sal_Bool                SwapIn( SvStream* pIStm );
    sal_Bool                IsSwapOut() const;

    void                SetLink( const GfxLink& );
    GfxLink             GetLink() const;
    sal_Bool                IsLink() const;

    sal_Bool                ExportNative( SvStream& rOStream ) const;

    friend VCL_DLLPUBLIC SvStream&    operator<<( SvStream& rOStream, const Graphic& rGraphic );
    friend VCL_DLLPUBLIC SvStream&    operator>>( SvStream& rIStream, Graphic& rGraphic );

public:

    virtual void        Load( SvStream& );
    virtual void        Save( SvStream& );
    virtual void        Assign( const SvDataCopyStream& );

    const SvgDataPtr&   getSvgData() const;
};

#endif  // _SV_GRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
