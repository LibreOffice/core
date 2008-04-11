/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impgraph.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_IMPGRAPH_HXX
#define _SV_IMPGRAPH_HXX

#include <tools/urlobj.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/animate.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.h>

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
    String              maDocFileURLStr;
    ULONG               mnDocFilePos;
    mutable ULONG       mnSizeBytes;
    ULONG               mnRefCount;
    BOOL                mbSwapOut;
    BOOL                mbSwapUnderway;

private:

                        ImpGraphic();
                        ImpGraphic( const ImpGraphic& rImpGraphic );
                        ImpGraphic( const Bitmap& rBmp );
                        ImpGraphic( const BitmapEx& rBmpEx );
                        ImpGraphic( const Animation& rAnimation );
                        ImpGraphic( const GDIMetaFile& rMtf );
    virtual             ~ImpGraphic();

    ImpGraphic&         operator=( const ImpGraphic& rImpGraphic );
    BOOL                operator==( const ImpGraphic& rImpGraphic ) const;
    BOOL                operator!=( const ImpGraphic& rImpGraphic ) const { return !( *this == rImpGraphic ); }

    void                ImplClearGraphics( BOOL bCreateSwapInfo );
    void                ImplClear();

    GraphicType         ImplGetType() const;
    void                ImplSetDefaultType();
    BOOL                ImplIsSupportedGraphic() const;

    BOOL                ImplIsTransparent() const;
    BOOL                ImplIsAlpha() const;
    BOOL                ImplIsAnimated() const;

    Bitmap              ImplGetBitmap( const Size* pSizePixel, BOOL bUnlimited ) const;
    BitmapEx            ImplGetBitmapEx( const Size* pSizePixel, BOOL bUnlimited ) const;
    Animation           ImplGetAnimation() const;
    const GDIMetaFile&  ImplGetGDIMetaFile() const;

    Size                ImplGetPrefSize() const;
    void                ImplSetPrefSize( const Size& rPrefSize );

    MapMode             ImplGetPrefMapMode() const;
    void                ImplSetPrefMapMode( const MapMode& rPrefMapMode );

    ULONG               ImplGetSizeBytes() const;

    void                ImplDraw( OutputDevice* pOutDev,
                                  const Point& rDestPt ) const;
    void                ImplDraw( OutputDevice* pOutDev,
                                  const Point& rDestPt,
                                  const Size& rDestSize ) const;

    void                ImplStartAnimation( OutputDevice* pOutDev,
                                            const Point& rDestPt,
                                            long nExtraData = 0,
                                            OutputDevice* pFirstFrameOutDev = NULL );
    void                ImplStartAnimation( OutputDevice* pOutDev,
                                            const Point& rDestPt,
                                            const Size& rDestSize,
                                            long nExtraData = 0,
                                            OutputDevice* pFirstFrameOutDev = NULL );
    void                ImplStopAnimation( OutputDevice* pOutputDevice = NULL,
                                           long nExtraData = 0 );

    void                ImplSetAnimationNotifyHdl( const Link& rLink );
    Link                ImplGetAnimationNotifyHdl() const;

    ULONG               ImplGetAnimationLoopCount() const;
    void                ImplResetAnimationLoopCount();

    List*               ImplGetAnimationInfoList() const;

private:

    GraphicReader*      ImplGetContext();
    void                ImplSetContext( GraphicReader* pReader );

private:

    void                ImplSetDocFileName( const String& rName, ULONG nFilePos );
    const String&       ImplGetDocFileName() const;
    ULONG               ImplGetDocFilePos() const;

    BOOL                ImplReadEmbedded( SvStream& rIStream, BOOL bSwap = FALSE );
    BOOL                ImplWriteEmbedded( SvStream& rOStream );

    BOOL                ImplSwapIn();
    BOOL                ImplSwapIn( SvStream* pIStm );

    BOOL                ImplSwapOut();
    BOOL                ImplSwapOut( SvStream* pOStm );

    BOOL                ImplIsSwapOut() const;

    void                ImplSetLink( const GfxLink& );
    GfxLink             ImplGetLink();
    BOOL                ImplIsLink() const;

    ULONG               ImplGetChecksum() const;

    BOOL                ImplExportNative( SvStream& rOStm ) const;

    friend SvStream&    operator<<( SvStream& rOStm, const ImpGraphic& rImpGraphic );
    friend SvStream&    operator>>( SvStream& rIStm, ImpGraphic& rImpGraphic );
};

#endif // _SV_IMPGRAPH_HXX
