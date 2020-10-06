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

#ifndef INCLUDED_VCL_ANIMATE_ANIMATION_HXX
#define INCLUDED_VCL_ANIMATE_ANIMATION_HXX

#include <vcl/dllapi.h>
#include <vcl/timer.hxx>
#include <vcl/animate/AnimationBitmap.hxx>

#define ANIMATION_TIMEOUT_ON_CLICK 2147483647L

class ImplAnimView;

class VCL_DLLPUBLIC Animation
{
public:
    Animation();
    Animation(const Animation& rAnimation);
    ~Animation();

    Animation& operator=(const Animation& rAnimation);
    bool operator==(const Animation& rAnimation) const;
    bool operator!=(const Animation& rAnimation) const { return !(*this == rAnimation); }

    void Clear();

    bool Start(OutputDevice* pOutDev, const Point& rDestPt, const Size& rDestSz, long nExtraData,
               OutputDevice* pFirstFrameOutDev);

    void Stop(const OutputDevice* pOutDev = nullptr, long nExtraData = 0);

    void Draw(OutputDevice* pOutDev, const Point& rDestPt) const;
    void Draw(OutputDevice* pOutDev, const Point& rDestPt, const Size& rDestSz) const;

    bool IsInAnimation() const { return mbIsInAnimation; }
    bool IsTransparent() const;

    const Size& GetDisplaySizePixel() const { return maGlobalSize; }
    void SetDisplaySizePixel(const Size& rSize) { maGlobalSize = rSize; }

    const BitmapEx& GetBitmapEx() const { return maBitmapEx; }
    void SetBitmapEx(const BitmapEx& rBmpEx) { maBitmapEx = rBmpEx; }

    sal_uInt32 GetLoopCount() const { return mnLoopCount; }
    void SetLoopCount(const sal_uInt32 nLoopCount);
    void ResetLoopCount();

    void SetNotifyHdl(const Link<Animation*, void>& rLink) { maNotifyLink = rLink; }
    const Link<Animation*, void>& GetNotifyHdl() const { return maNotifyLink; }

    std::vector<std::unique_ptr<AnimationBitmap>>& GetAnimationFrames() { return maList; }
    size_t Count() const { return maList.size(); }
    bool Insert(const AnimationBitmap& rAnimationBitmap);
    const AnimationBitmap& Get(sal_uInt16 nAnimation) const;
    void Replace(const AnimationBitmap& rNewAnimationBmp, sal_uInt16 nAnimation);

    sal_uLong GetSizeBytes() const;
    BitmapChecksum GetChecksum() const;

public:
    void Convert(BmpConversion eConversion);
    bool ReduceColors(sal_uInt16 nNewColorCount);

    bool Invert();
    void Mirror(BmpMirrorFlags nMirrorFlags);
    void Adjust(short nLuminancePercent, short nContrastPercent, short nChannelRPercent,
                short nChannelGPercent, short nChannelBPercent, double fGamma = 1.0,
                bool bInvert = false);

    friend SvStream& ReadAnimation(SvStream& rIStream, Animation& rAnimation);
    friend SvStream& WriteAnimation(SvStream& rOStream, const Animation& rAnimation);

public:
    SAL_DLLPRIVATE static void ImplIncAnimCount() { mnAnimCount++; }
    SAL_DLLPRIVATE static void ImplDecAnimCount() { mnAnimCount--; }
    SAL_DLLPRIVATE sal_uLong ImplGetCurPos() const { return mnPos; }

private:
    SAL_DLLPRIVATE static sal_uLong mnAnimCount;

    std::vector<std::unique_ptr<AnimationBitmap>> maList;
    std::vector<std::unique_ptr<ImplAnimView>> maViewList;

    Link<Animation*, void> maNotifyLink;
    BitmapEx maBitmapEx;
    Timer maTimer;
    Size maGlobalSize;
    sal_uInt32 mnLoopCount;
    sal_uInt32 mnLoops;
    size_t mnPos;
    bool mbIsInAnimation;
    bool mbLoopTerminated;

    SAL_DLLPRIVATE void ImplRestartTimer(sal_uLong nTimeout);
    DECL_DLLPRIVATE_LINK(ImplTimeoutHdl, Timer*, void);
};

#endif // INCLUDED_VCL_ANIMATE_ANIMATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
