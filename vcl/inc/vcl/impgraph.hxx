/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impgraph.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:00:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_IMPGRAPH_HXX
#define _SV_IMPGRAPH_HXX

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _SV_ANIMATE_HXX
#include <vcl/animate.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif
#ifndef _SV_GRAPH_H
#include <vcl/graph.h>
#endif

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
