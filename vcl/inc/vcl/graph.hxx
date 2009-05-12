/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: graph.hxx,v $
 * $Revision: 1.4 $
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
    BOOL                operator==( const Graphic& rGraphic ) const;
    BOOL                operator!=( const Graphic& rGraphic ) const;
    BOOL                operator!() const;

    void                Clear();

    GraphicType         GetType() const;
    void                SetDefaultType();
    BOOL                IsSupportedGraphic() const;

    BOOL                IsTransparent() const;
    BOOL                IsAlpha() const;
    BOOL                IsAnimated() const;

    Bitmap              GetBitmap() const;
    Bitmap              GetBitmap( const Size* pSizePixel ) const;
    BitmapEx            GetBitmapEx() const;
    BitmapEx            GetBitmapEx( const Size* pSizePixel ) const;
    Bitmap              GetUnlimitedBitmap( const Size* pSizePixel ) const;
    BitmapEx            GetUnlimitedBitmapEx( const Size* pSizePixel ) const;
    Animation           GetAnimation() const;
    const GDIMetaFile&  GetGDIMetaFile() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > GetXGraphic() const;

    Size                GetPrefSize() const;
    void                SetPrefSize( const Size& rPrefSize );

    MapMode             GetPrefMapMode() const;
    void                SetPrefMapMode( const MapMode& rPrefMapMode );

    Size                GetSizePixel( const OutputDevice* pRefDevice = NULL ) const;

    ULONG               GetSizeBytes() const;

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

    ULONG               GetAnimationLoopCount() const;
    void                ResetAnimationLoopCount();

    List*               GetAnimationInfoList() const;

    ULONG               GetChecksum() const;

public:

    GraphicReader*      GetContext();
    void                SetContext( GraphicReader* pReader );

public:

    static USHORT       GetGraphicsCompressMode( SvStream& rIStm );

    void                SetDocFileName( const String& rName, ULONG nFilePos );
    const String&       GetDocFileName() const;
    ULONG               GetDocFilePos() const;

    BOOL                ReadEmbedded( SvStream& rIStream, BOOL bSwap = FALSE );
    BOOL                WriteEmbedded( SvStream& rOStream );

    BOOL                SwapOut();
    BOOL                SwapOut( SvStream* pOStm );
    BOOL                SwapIn();
    BOOL                SwapIn( SvStream* pIStm );
    BOOL                IsSwapOut() const;

    void                SetLink( const GfxLink& );
    GfxLink             GetLink();
    BOOL                IsLink() const;

    BOOL                ExportNative( SvStream& rOStream ) const;

    friend VCL_DLLPUBLIC SvStream&    operator<<( SvStream& rOStream, const Graphic& rGraphic );
    friend VCL_DLLPUBLIC SvStream&    operator>>( SvStream& rIStream, Graphic& rGraphic );

public:

    virtual void        Load( SvStream& );
    virtual void        Save( SvStream& );
    virtual void        Assign( const SvDataCopyStream& );
};

#endif  // _SV_GRAPH_HXX

