/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_GRAPH_HXX
#define _SV_GRAPH_HXX

#include <tools/stream.hxx>
#include <vcl/dllapi.h>
#include <vcl/sv.h>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/animate.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/rendergraphic.hxx>
#include <vcl/graph.h>
#include <vcl/gfxlink.hxx>
#include <com/sun/star/uno/Reference.hxx>

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

public:
    GraphicConversionParameters(
        const Size& rSizePixel = Size(),
        bool bUnlimitedSize = false,
        bool bAntiAliase = false,
        bool bSnapHorVerLines = false)
    :   maSizePixel(rSizePixel),
        mbUnlimitedSize(bUnlimitedSize),
        mbAntiAliase(bAntiAliase),
        mbSnapHorVerLines(bSnapHorVerLines)
    {
    }

    // data read access
    const Size getSizePixel() const { return maSizePixel; }
    bool getUnlimitedSize() const { return mbUnlimitedSize; }
    bool getAntiAliase() const { return mbAntiAliase; }
    bool getSnapHorVerLines() const { return mbSnapHorVerLines; }
};

class VCL_DLLPUBLIC Graphic : public SvDataCopyStream
{
private:

    ImpGraphic*         mpImpGraphic;

//#if 0 // _SOLAR__PRIVATE

public:

    SAL_DLLPRIVATE void         ImplTestRefCount();
    SAL_DLLPRIVATE ImpGraphic*  ImplGetImpGraphic() const { return mpImpGraphic; }

//#endif

public:

                        TYPEINFO();

                        Graphic();
                        Graphic( const Graphic& rGraphic );
                        Graphic( const Bitmap& rBmp );
                        Graphic( const BitmapEx& rBmpEx );
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
    sal_Bool            IsRenderGraphic() const;
    sal_Bool            HasRenderGraphic() const;

    // #i102089# Access of Bitmap potentially will have to rasterconvert the Graphic
    // if it is a MetaFile. To be able to control this conversion it is necessary to
    // allow giving parameters which control AntiAliasing and LineSnapping of the
    // MetaFile when played. Defaults will use a no-AAed, not snapped conversion as
    // before.
    Bitmap              GetBitmap(const GraphicConversionParameters& rParameters = GraphicConversionParameters()) const;
    BitmapEx            GetBitmapEx(const GraphicConversionParameters& rParameters = GraphicConversionParameters()) const;

    Animation               GetAnimation() const;
    const GDIMetaFile&      GetGDIMetaFile() const;
    ::vcl::RenderGraphic    GetRenderGraphic() const;

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
    static void         Draw( OutputDevice* pOutDev, const String& rText,
                              Font& rFont, const Bitmap& rBitmap,
                              const Point& rDestPt, const Size& rDestSize );
    static void         DrawEx( OutputDevice* pOutDev, const String& rText,
                              Font& rFont, const BitmapEx& rBitmap,
                              const Point& rDestPt, const Size& rDestSize );

    void                StartAnimation( OutputDevice* pOutDev,
                                        const Point& rDestPt,
                                        long nExtraData = 0L,
                                        OutputDevice* pFirstFrameOutDev = NULL );
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
    void                ResetAnimationLoopCount();

    List*               GetAnimationInfoList() const;

    sal_uLong               GetChecksum() const;

public:

    GraphicReader*      GetContext();
    void                SetContext( GraphicReader* pReader );

public:

    static sal_uInt16       GetGraphicsCompressMode( SvStream& rIStm );

    void                SetDocFileName( const String& rName, sal_uLong nFilePos );
    const String&       GetDocFileName() const;
    sal_uLong               GetDocFilePos() const;

    sal_Bool                ReadEmbedded( SvStream& rIStream, sal_Bool bSwap = sal_False );
    sal_Bool                WriteEmbedded( SvStream& rOStream );

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
};

#endif  // _SV_GRAPH_HXX
