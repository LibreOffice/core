/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_ANIMATE_HXX
#define _SV_ANIMATE_HXX

#include <vcl/dllapi.h>
#include <vcl/timer.hxx>
#include <vcl/bitmapex.hxx>

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
    sal_Bool        bUserInput;

                AnimationBitmap() {}
                AnimationBitmap( const BitmapEx& rBmpEx, const Point& rPosPix,
                                 const Size& rSizePix, long _nWait = 0L,
                                 Disposal _eDisposal = DISPOSE_NOT ) :
                            aBmpEx      ( rBmpEx ),
                            aPosPix     ( rPosPix ),
                            aSizePix    ( rSizePix ),
                            nWait       ( _nWait ),
                            eDisposal   ( _eDisposal ),
                            bUserInput  ( sal_False ) {}

    sal_Bool        operator==( const AnimationBitmap& rAnimBmp ) const
                {
                    return( rAnimBmp.aBmpEx == aBmpEx &&
                            rAnimBmp.aPosPix == aPosPix &&
                            rAnimBmp.aSizePix == aSizePix &&
                            rAnimBmp.nWait == nWait &&
                            rAnimBmp.eDisposal == eDisposal &&
                            rAnimBmp.bUserInput == bUserInput );
                }

    sal_Bool        operator!=( const AnimationBitmap& rAnimBmp ) const { return !( *this == rAnimBmp ); }

    sal_Bool        IsEqual( const AnimationBitmap& rAnimBmp ) const
                {
                    return( rAnimBmp.aPosPix == aPosPix &&
                            rAnimBmp.aSizePix == aSizePix &&
                            rAnimBmp.nWait == nWait &&
                            rAnimBmp.eDisposal == eDisposal &&
                            rAnimBmp.bUserInput == bUserInput &&
                            rAnimBmp.aBmpEx.IsEqual( aBmpEx ) );
                }

    sal_uLong       GetChecksum() const;
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
    sal_Bool            bWithSize;
    sal_Bool            bPause;

                    AInfo() : pOutDev( NULL ),
                              pViewData( NULL ),
                              nExtraData( 0L ),
                              bWithSize( sal_False ),
                              bPause( sal_False ) {}
};

// -------------------
// - AnimationBitmap -
// -------------------

class VCL_DLLPUBLIC Animation
{
    SAL_DLLPRIVATE static sal_uLong         mnAnimCount;

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
    sal_Bool                    mbFirst;
    sal_Bool                    mbIsInAnimation;
    sal_Bool                    mbWithSize;
    sal_Bool                    mbLoopTerminated;
    sal_Bool                    mbIsWaiting;

//#if 0 // _SOLAR__PRIVATE

    SAL_DLLPRIVATE void     ImplRestartTimer( sal_uLong nTimeout );
    DECL_DLLPRIVATE_LINK(   ImplTimeoutHdl, Timer* );

public:

    SAL_DLLPRIVATE static void  ImplIncAnimCount() { mnAnimCount++; }
    SAL_DLLPRIVATE static void  ImplDecAnimCount() { mnAnimCount--; }
    SAL_DLLPRIVATE sal_uLong        ImplGetCurPos() const { return mnPos; }

//#endif

public:
                            Animation();
                            Animation( const Animation& rAnimation );
                            ~Animation();

    Animation&              operator=( const Animation& rAnimation );
    sal_Bool                    operator==( const Animation& rAnimation ) const;
    sal_Bool                    operator!=( const Animation& rAnimation ) const { return !(*this==rAnimation); }

    sal_Bool                    IsEqual( const Animation& rAnimation ) const;

    sal_Bool                    IsEmpty() const;
    void                    SetEmpty();

    void                    Clear();

    sal_Bool                    Start( OutputDevice* pOutDev, const Point& rDestPt, long nExtraData = 0,
                                   OutputDevice* pFirstFrameOutDev = NULL );
    sal_Bool                    Start( OutputDevice* pOutDev, const Point& rDestPt, const Size& rDestSz, long nExtraData = 0,
                                   OutputDevice* pFirstFrameOutDev = NULL );
    void                    Stop( OutputDevice* pOutDev = NULL, long nExtraData = 0 );

    void                    Draw( OutputDevice* pOutDev, const Point& rDestPt ) const;
    void                    Draw( OutputDevice* pOutDev, const Point& rDestPt, const Size& rDestSz ) const;

    sal_Bool                    IsInAnimation() const { return mbIsInAnimation; }
    sal_Bool                    IsTransparent() const;
    sal_Bool                    IsTerminated() const { return mbLoopTerminated; }

    const Size&             GetDisplaySizePixel() const { return maGlobalSize; }
    void                    SetDisplaySizePixel( const Size& rSize ) { maGlobalSize = rSize; }

    const BitmapEx&         GetBitmapEx() const { return maBitmapEx; }
    void                    SetBitmapEx( const BitmapEx& rBmpEx ) { maBitmapEx = rBmpEx; }

    sal_uLong                   GetLoopCount() const { return mnLoopCount; }
    void                    SetLoopCount( const sal_uLong nLoopCount );
    void                    ResetLoopCount();

    void                    SetCycleMode( CycleMode eMode );
    CycleMode               GetCycleMode() const { return meCycleMode; }

    void                    SetNotifyHdl( const Link& rLink ) { maNotifyLink = rLink; }
    const Link&             GetNotifyHdl() const { return maNotifyLink; }

    sal_uInt16                  Count() const { return (sal_uInt16) maList.Count(); }
    sal_Bool                    Insert( const AnimationBitmap& rAnimationBitmap );
    const AnimationBitmap&  Get( sal_uInt16 nAnimation ) const;
    void                    Replace( const AnimationBitmap& rNewAnimationBmp, sal_uInt16 nAnimation );

    List*                   GetAInfoList() { return &maAInfoList; }
    sal_uLong                   GetSizeBytes() const;
    sal_uLong                   GetChecksum() const;

public:

    sal_Bool                    Convert( BmpConversion eConversion );
    sal_Bool                    ReduceColors( sal_uInt16 nNewColorCount,
                                          BmpReduce eReduce = BMP_REDUCE_SIMPLE );
    sal_Bool                    Invert();
    sal_Bool                    Mirror( sal_uLong nMirrorFlags );
    sal_Bool                    Dither( sal_uLong nDitherFlags = BMP_DITHER_MATRIX );
    sal_Bool                    Adjust( short nLuminancePercent = 0,
                                    short nContrastPercent = 0,
                                    short nChannelRPercent = 0,
                                    short nChannelGPercent = 0,
                                    short nChannelBPercent = 0,
                                    double fGamma = 1.0,
                                    sal_Bool bInvert = sal_False );
    sal_Bool                    Filter( BmpFilter eFilter,
                                    const BmpFilterParam* pFilterParam = NULL,
                                    const Link* pProgress = NULL );

    friend VCL_DLLPUBLIC SvStream&      operator>>( SvStream& rIStream, Animation& rAnimation );
    friend VCL_DLLPUBLIC SvStream&      operator<<( SvStream& rOStream, const Animation& rAnimation );
};

#endif // _SV_ANIMATE_HXX
