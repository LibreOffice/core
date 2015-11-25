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

#ifndef INCLUDED_VCL_ANIMATE_HXX
#define INCLUDED_VCL_ANIMATE_HXX

#include <vcl/dllapi.h>
#include <vcl/timer.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/vclptr.hxx>

#define ANIMATION_TIMEOUT_ON_CLICK 2147483647L

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

struct VCL_DLLPUBLIC AnimationBitmap
{
    BitmapEx        aBmpEx;
    Point           aPosPix;
    Size            aSizePix;
    long            nWait;
    Disposal        eDisposal;
    bool            bUserInput;

                    AnimationBitmap()
                        : nWait(0)
                        , eDisposal(DISPOSE_NOT)
                        , bUserInput(false)
                    {}

                    AnimationBitmap(
                        const BitmapEx& rBmpEx,
                        const Point& rPosPix,
                        const Size& rSizePix,
                        long _nWait = 0L,
                        Disposal _eDisposal = DISPOSE_NOT
                    ) :
                        aBmpEx      ( rBmpEx ),
                        aPosPix     ( rPosPix ),
                        aSizePix    ( rSizePix ),
                        nWait       ( _nWait ),
                        eDisposal   ( _eDisposal ),
                        bUserInput  ( false )
                    {}

    bool            operator==( const AnimationBitmap& rAnimBmp ) const
                        {
                            return( rAnimBmp.aBmpEx == aBmpEx &&
                                    rAnimBmp.aPosPix == aPosPix &&
                                    rAnimBmp.aSizePix == aSizePix &&
                                    rAnimBmp.nWait == nWait &&
                                    rAnimBmp.eDisposal == eDisposal &&
                                    rAnimBmp.bUserInput == bUserInput );
                        }

    bool            operator!=( const AnimationBitmap& rAnimBmp ) const
                        { return !( *this == rAnimBmp ); }


    BitmapChecksum  GetChecksum() const;
};

struct AInfo
{
    Bitmap          aLastSaveBitmap;
    Bitmap          aBackBitmap;
    Point           aStartOrg;
    Size            aStartSize;
    VclPtr<OutputDevice>   pOutDev;
    void*           pViewData;
    long            nExtraData;
    bool            bPause;

    AInfo();
};

class ImplAnimView;

class VCL_DLLPUBLIC Animation
{
public:
                    Animation();
                    Animation( const Animation& rAnimation );
                    ~Animation();

    Animation&      operator=( const Animation& rAnimation );
    bool            operator==( const Animation& rAnimation ) const;
    bool            operator!=( const Animation& rAnimation ) const
                        { return !(*this==rAnimation); }

    void            Clear();

    bool            Start(
                        OutputDevice* pOutDev,
                        const Point& rDestPt,
                        const Size& rDestSz,
                        long nExtraData = 0,
                        OutputDevice* pFirstFrameOutDev = nullptr);

    void            Stop( OutputDevice* pOutDev = nullptr, long nExtraData = 0 );

    void            Draw( OutputDevice* pOutDev, const Point& rDestPt ) const;
    void            Draw( OutputDevice* pOutDev, const Point& rDestPt, const Size& rDestSz ) const;

    bool            IsInAnimation() const { return mbIsInAnimation; }
    bool            IsTransparent() const;

    const Size&     GetDisplaySizePixel() const { return maGlobalSize; }
    void            SetDisplaySizePixel( const Size& rSize ) { maGlobalSize = rSize; }

    const BitmapEx& GetBitmapEx() const { return maBitmapEx; }
    void            SetBitmapEx( const BitmapEx& rBmpEx ) { maBitmapEx = rBmpEx; }

    sal_uLong       GetLoopCount() const { return mnLoopCount; }
    void            SetLoopCount( const sal_uLong nLoopCount );
    void            ResetLoopCount();

    CycleMode       GetCycleMode() const { return meCycleMode; }

    void            SetNotifyHdl( const Link<Animation*,void>& rLink ) { maNotifyLink = rLink; }
    const Link<Animation*,void>& GetNotifyHdl() const { return maNotifyLink; }

    size_t          Count() const { return maList.size(); }
    bool            Insert( const AnimationBitmap& rAnimationBitmap );
    const AnimationBitmap&
                    Get( sal_uInt16 nAnimation ) const;
    void            Replace( const AnimationBitmap& rNewAnimationBmp, sal_uInt16 nAnimation );

    sal_uLong       GetSizeBytes() const;
    BitmapChecksum  GetChecksum() const;

public:

    bool            Convert( BmpConversion eConversion );
    bool            ReduceColors(
                        sal_uInt16 nNewColorCount,
                        BmpReduce eReduce = BMP_REDUCE_SIMPLE );

    bool            Invert();
    bool            Mirror( BmpMirrorFlags nMirrorFlags );
    bool            Adjust(
                        short nLuminancePercent = 0,
                        short nContrastPercent = 0,
                        short nChannelRPercent = 0,
                        short nChannelGPercent = 0,
                        short nChannelBPercent = 0,
                        double fGamma = 1.0,
                        bool bInvert = false );

    bool            Filter(
                        BmpFilter eFilter,
                        const BmpFilterParam* pFilterParam = nullptr );

    friend VCL_DLLPUBLIC SvStream& ReadAnimation( SvStream& rIStream, Animation& rAnimation );
    friend VCL_DLLPUBLIC SvStream& WriteAnimation( SvStream& rOStream, const Animation& rAnimation );

public:

    SAL_DLLPRIVATE static void
                    ImplIncAnimCount() { mnAnimCount++; }
    SAL_DLLPRIVATE static void
                    ImplDecAnimCount() { mnAnimCount--; }
    SAL_DLLPRIVATE sal_uLong
                    ImplGetCurPos() const { return mnPos; }

private:
    SAL_DLLPRIVATE static sal_uLong mnAnimCount;

    std::vector< AnimationBitmap* > maList;
    std::vector< ImplAnimView* >    maViewList;

    Link<Animation*,void> maNotifyLink;
    BitmapEx        maBitmapEx;
    Timer           maTimer;
    Size            maGlobalSize;
    long            mnLoopCount;
    long            mnLoops;
    size_t          mnPos;
    CycleMode       meCycleMode;
    bool            mbIsInAnimation;
    bool            mbLoopTerminated;
    bool            mbIsWaiting;

    SAL_DLLPRIVATE void ImplRestartTimer( sal_uLong nTimeout );
    DECL_DLLPRIVATE_LINK_TYPED( ImplTimeoutHdl, Timer*, void );

};

#endif // INCLUDED_VCL_ANIMATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
