/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animate.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:47:30 $
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

#ifndef _SV_ANIMATE_HXX
#define _SV_ANIMATE_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

// -----------
// - Defines -
// -----------

#define ANIMATION_TIMEOUT_ON_CLICK 2147483647L

// ---------
// - Enums -
// ---------

enum Disposal
{
    DISPOSE_NOT,
    DISPOSE_BACK,
    DISPOSE_FULL,
    DISPOSE_PREVIOUS
};

enum CycleMode
{
    CYCLE_NOT,
    CYCLE_NORMAL,
    CYCLE_FALLBACK,
    CYCLE_REVERS,
    CYCLE_REVERS_FALLBACK
};

// -------------------
// - AnimationBitmap -
// -------------------

struct VCL_DLLPUBLIC AnimationBitmap
{
    BitmapEx    aBmpEx;
    Point       aPosPix;
    Size        aSizePix;
    long        nWait;
    Disposal    eDisposal;
    BOOL        bUserInput;

                AnimationBitmap() {}
                AnimationBitmap( const BitmapEx& rBmpEx, const Point& rPosPix,
                                 const Size& rSizePix, long _nWait = 0L,
                                 Disposal _eDisposal = DISPOSE_NOT ) :
                            aBmpEx      ( rBmpEx ),
                            aPosPix     ( rPosPix ),
                            aSizePix    ( rSizePix ),
                            nWait       ( _nWait ),
                            eDisposal   ( _eDisposal ),
                            bUserInput  ( FALSE ) {}

    BOOL        operator==( const AnimationBitmap& rAnimBmp ) const
                {
                    return( rAnimBmp.aBmpEx == aBmpEx &&
                            rAnimBmp.aPosPix == aPosPix &&
                            rAnimBmp.aSizePix == aSizePix &&
                            rAnimBmp.nWait == nWait &&
                            rAnimBmp.eDisposal == eDisposal &&
                            rAnimBmp.bUserInput == bUserInput );
                }

    BOOL        operator!=( const AnimationBitmap& rAnimBmp ) const { return !( *this == rAnimBmp ); }

    BOOL        IsEqual( const AnimationBitmap& rAnimBmp ) const
                {
                    return( rAnimBmp.aPosPix == aPosPix &&
                            rAnimBmp.aSizePix == aSizePix &&
                            rAnimBmp.nWait == nWait &&
                            rAnimBmp.eDisposal == eDisposal &&
                            rAnimBmp.bUserInput == bUserInput &&
                            rAnimBmp.aBmpEx.IsEqual( aBmpEx ) );
                }

    ULONG       GetChecksum() const;
};

// -------------------
// - AnimationBitmap -
// -------------------

struct AInfo
{
    Bitmap          aLastSaveBitmap;
    Bitmap          aBackBitmap;
    Rectangle       aClipRect;
    Size            aLastSaveSize;
    Point           aLastSavePoint;
    Point           aStartOrg;
    Size            aStartSize;
    OutputDevice*   pOutDev;
    void*           pViewData;
    long            nExtraData;
    BOOL            bWithSize;
    BOOL            bPause;

                    AInfo() : pOutDev( NULL ),
                              pViewData( NULL ),
                              nExtraData( 0L ),
                              bWithSize( FALSE ),
                              bPause( FALSE ) {}
};

// -------------------
// - AnimationBitmap -
// -------------------

class VCL_DLLPUBLIC Animation
{
    SAL_DLLPRIVATE static ULONG         mnAnimCount;

    List                    maList;
    List                    maAInfoList;
    Link                    maNotifyLink;
    BitmapEx                maBitmapEx;
    Timer                   maTimer;
    Size                    maGlobalSize;
    List*                   mpViewList;
    void*                   mpExtraData;
    long                    mnLoopCount;
    long                    mnLoops;
    long                    mnPos;
    Disposal                meLastDisposal;
    CycleMode               meCycleMode;
    BOOL                    mbFirst;
    BOOL                    mbIsInAnimation;
    BOOL                    mbWithSize;
    BOOL                    mbLoopTerminated;
    BOOL                    mbIsWaiting;

//#if 0 // _SOLAR__PRIVATE

    SAL_DLLPRIVATE void     ImplRestartTimer( ULONG nTimeout );
    DECL_DLLPRIVATE_LINK(   ImplTimeoutHdl, Timer* );

public:

    SAL_DLLPRIVATE static void  ImplIncAnimCount() { mnAnimCount++; }
    SAL_DLLPRIVATE static void  ImplDecAnimCount() { mnAnimCount--; }
    SAL_DLLPRIVATE ULONG        ImplGetCurPos() const { return mnPos; }

//#endif

public:
                            Animation();
                            Animation( const Animation& rAnimation );
                            ~Animation();

    Animation&              operator=( const Animation& rAnimation );
    BOOL                    operator==( const Animation& rAnimation ) const;
    BOOL                    operator!=( const Animation& rAnimation ) const { return !(*this==rAnimation); }

    BOOL                    IsEqual( const Animation& rAnimation ) const;

    BOOL                    IsEmpty() const;
    void                    SetEmpty();

    void                    Clear();

    BOOL                    Start( OutputDevice* pOutDev, const Point& rDestPt, long nExtraData = 0,
                                   OutputDevice* pFirstFrameOutDev = NULL );
    BOOL                    Start( OutputDevice* pOutDev, const Point& rDestPt, const Size& rDestSz, long nExtraData = 0,
                                   OutputDevice* pFirstFrameOutDev = NULL );
    void                    Stop( OutputDevice* pOutDev = NULL, long nExtraData = 0 );

    void                    Draw( OutputDevice* pOutDev, const Point& rDestPt ) const;
    void                    Draw( OutputDevice* pOutDev, const Point& rDestPt, const Size& rDestSz ) const;

    BOOL                    IsInAnimation() const { return mbIsInAnimation; }
    BOOL                    IsTransparent() const;
    BOOL                    IsTerminated() const { return mbLoopTerminated; }

    const Size&             GetDisplaySizePixel() const { return maGlobalSize; }
    void                    SetDisplaySizePixel( const Size& rSize ) { maGlobalSize = rSize; }

    const BitmapEx&         GetBitmapEx() const { return maBitmapEx; }
    void                    SetBitmapEx( const BitmapEx& rBmpEx ) { maBitmapEx = rBmpEx; }

    ULONG                   GetLoopCount() const { return mnLoopCount; }
    void                    SetLoopCount( const ULONG nLoopCount );
    void                    ResetLoopCount();

    void                    SetCycleMode( CycleMode eMode );
    CycleMode               GetCycleMode() const { return meCycleMode; }

    void                    SetNotifyHdl( const Link& rLink ) { maNotifyLink = rLink; }
    const Link&             GetNotifyHdl() const { return maNotifyLink; }

    USHORT                  Count() const { return (USHORT) maList.Count(); }
    BOOL                    Insert( const AnimationBitmap& rAnimationBitmap );
    const AnimationBitmap&  Get( USHORT nAnimation ) const;
    void                    Replace( const AnimationBitmap& rNewAnimationBmp, USHORT nAnimation );

    List*                   GetAInfoList() { return &maAInfoList; }
    ULONG                   GetSizeBytes() const;
    ULONG                   GetChecksum() const;

public:

    BOOL                    Convert( BmpConversion eConversion );
    BOOL                    ReduceColors( USHORT nNewColorCount,
                                          BmpReduce eReduce = BMP_REDUCE_SIMPLE );
    BOOL                    Invert();
    BOOL                    Mirror( ULONG nMirrorFlags );
    BOOL                    Dither( ULONG nDitherFlags = BMP_DITHER_MATRIX );
    BOOL                    Adjust( short nLuminancePercent = 0,
                                    short nContrastPercent = 0,
                                    short nChannelRPercent = 0,
                                    short nChannelGPercent = 0,
                                    short nChannelBPercent = 0,
                                    double fGamma = 1.0,
                                    BOOL bInvert = FALSE );
    BOOL                    Filter( BmpFilter eFilter,
                                    const BmpFilterParam* pFilterParam = NULL,
                                    const Link* pProgress = NULL );

    friend VCL_DLLPUBLIC SvStream&      operator>>( SvStream& rIStream, Animation& rAnimation );
    friend VCL_DLLPUBLIC SvStream&      operator<<( SvStream& rOStream, const Animation& rAnimation );
};

#endif // _SV_ANIMATE_HXX
