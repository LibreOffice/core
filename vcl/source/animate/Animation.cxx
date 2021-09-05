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

#include <algorithm>
#include <sal/config.h>

#include <tools/stream.hxx>
#include <tools/GenericTypeSerializer.hxx>
#include <sal/log.hxx>

#include <vcl/animate/Animation.hxx>
#include <vcl/outdev.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/BitmapColorQuantizationFilter.hxx>

#include <animate/AnimationRenderer.hxx>

sal_uLong Animation::mnAnimCount = 0;

Animation::Animation()
    : mnLoopCount(0)
    , mnLoops(0)
    , mnFrameIndex(0)
    , mbIsInAnimation(false)
    , mbLoopTerminated(false)
{
    maTimer.SetInvokeHandler(LINK(this, Animation, ImplTimeoutHdl));
}

Animation::Animation(const Animation& rAnimation)
    : maBitmapEx(rAnimation.maBitmapEx)
    , maGlobalSize(rAnimation.maGlobalSize)
    , mnLoopCount(rAnimation.mnLoopCount)
    , mnFrameIndex(rAnimation.mnFrameIndex)
    , mbIsInAnimation(false)
    , mbLoopTerminated(rAnimation.mbLoopTerminated)
{
    for (auto const& rFrame : rAnimation.maAnimationFrames)
        maAnimationFrames.emplace_back(new AnimationBitmap(*rFrame));

    maTimer.SetInvokeHandler(LINK(this, Animation, ImplTimeoutHdl));
    mnLoops = mbLoopTerminated ? 0 : mnLoopCount;
}

Animation::~Animation()
{
    if (mbIsInAnimation)
        Stop();
}

Animation& Animation::operator=(const Animation& rAnimation)
{
    if (this != &rAnimation)
    {
        Clear();

        for (auto const& i : rAnimation.maAnimationFrames)
            maAnimationFrames.emplace_back(new AnimationBitmap(*i));

        maGlobalSize = rAnimation.maGlobalSize;
        maBitmapEx = rAnimation.maBitmapEx;
        mnLoopCount = rAnimation.mnLoopCount;
        mnFrameIndex = rAnimation.mnFrameIndex;
        mbLoopTerminated = rAnimation.mbLoopTerminated;
        mnLoops = mbLoopTerminated ? 0 : mnLoopCount;
    }
    return *this;
}

bool Animation::operator==(const Animation& rAnimation) const
{
    return maAnimationFrames.size() == rAnimation.maAnimationFrames.size()
           && maBitmapEx == rAnimation.maBitmapEx && maGlobalSize == rAnimation.maGlobalSize
           && std::equal(maAnimationFrames.begin(), maAnimationFrames.end(),
                         rAnimation.maAnimationFrames.begin(),
                         [](const std::unique_ptr<AnimationBitmap>& pAnim1,
                            const std::unique_ptr<AnimationBitmap>& pAnim2) -> bool {
                             return *pAnim1 == *pAnim2;
                         });
}

void Animation::Clear()
{
    maTimer.Stop();
    mbIsInAnimation = false;
    maGlobalSize = Size();
    maBitmapEx.SetEmpty();
    maAnimationFrames.clear();
    maAnimationRenderers.clear();
}

bool Animation::IsTransparent() const
{
    tools::Rectangle aRect{ Point(), maGlobalSize };

    // If some small bitmap needs to be replaced by the background,
    // we need to be transparent, in order to be displayed correctly
    // as the application (?) does not invalidate on non-transparent
    // graphics due to performance reasons.

    return maBitmapEx.IsAlpha()
           || std::any_of(maAnimationFrames.begin(), maAnimationFrames.end(),
                          [&aRect](const std::unique_ptr<AnimationBitmap>& pAnim) -> bool {
                              return pAnim->meDisposal == Disposal::Back
                                     && tools::Rectangle{ pAnim->maPositionPixel,
                                                          pAnim->maSizePixel }
                                            != aRect;
                          });
}

sal_uLong Animation::GetSizeBytes() const
{
    sal_uLong nSizeBytes = GetBitmapEx().GetSizeBytes();

    for (auto const& pAnimationBitmap : maAnimationFrames)
    {
        nSizeBytes += pAnimationBitmap->maBitmapEx.GetSizeBytes();
    }

    return nSizeBytes;
}

BitmapChecksum Animation::GetChecksum() const
{
    SVBT32 aBT32;
    BitmapChecksumOctetArray aBCOA;
    BitmapChecksum nCrc = GetBitmapEx().GetChecksum();

    UInt32ToSVBT32(maAnimationFrames.size(), aBT32);
    nCrc = vcl_get_checksum(nCrc, aBT32, 4);

    Int32ToSVBT32(maGlobalSize.Width(), aBT32);
    nCrc = vcl_get_checksum(nCrc, aBT32, 4);

    Int32ToSVBT32(maGlobalSize.Height(), aBT32);
    nCrc = vcl_get_checksum(nCrc, aBT32, 4);

    for (auto const& i : maAnimationFrames)
    {
        BCToBCOA(i->GetChecksum(), aBCOA);
        nCrc = vcl_get_checksum(nCrc, aBCOA, BITMAP_CHECKSUM_SIZE);
    }

    return nCrc;
}

bool Animation::Start(OutputDevice& rOut, const Point& rDestPt, const Size& rDestSz,
                      tools::Long nCallerId, OutputDevice* pFirstFrameOutDev)
{
    bool bRet = false;

    if (!maAnimationFrames.empty())
    {
        if ((rOut.GetOutDevType() == OUTDEV_WINDOW) && !mbLoopTerminated
            && (ANIMATION_TIMEOUT_ON_CLICK != maAnimationFrames[mnFrameIndex]->mnWait))
        {
            bool differs = true;

            auto itAnimView = std::find_if(
                maAnimationRenderers.begin(), maAnimationRenderers.end(),
                [&rOut, nCallerId](const std::unique_ptr<AnimationRenderer>& pAnimView) -> bool {
                    return pAnimView->matches(&rOut, nCallerId);
                });

            if (itAnimView != maAnimationRenderers.end())
            {
                if ((*itAnimView)->getOutPos() == rDestPt
                    && (*itAnimView)->getOutSizePix() == rOut.LogicToPixel(rDestSz))
                {
                    (*itAnimView)->repaint();
                    differs = false;
                }
                else
                    maAnimationRenderers.erase(itAnimView);
            }

            if (maAnimationRenderers.empty())
            {
                maTimer.Stop();
                mbIsInAnimation = false;
                mnFrameIndex = 0;
            }

            if (differs)
                maAnimationRenderers.emplace_back(new AnimationRenderer(
                    this, &rOut, rDestPt, rDestSz, nCallerId, pFirstFrameOutDev));

            if (!mbIsInAnimation)
            {
                RestartTimer(maAnimationFrames[mnFrameIndex]->mnWait);
                mbIsInAnimation = true;
            }
        }
        else
            Draw(rOut, rDestPt, rDestSz);

        bRet = true;
    }

    return bRet;
}

void Animation::Stop(const OutputDevice* pOut, tools::Long nCallerId)
{
    maAnimationRenderers.erase(
        std::remove_if(maAnimationRenderers.begin(), maAnimationRenderers.end(),
                       [=](const std::unique_ptr<AnimationRenderer>& pAnimView) -> bool {
                           return pAnimView->matches(pOut, nCallerId);
                       }),
        maAnimationRenderers.end());

    if (maAnimationRenderers.empty())
    {
        maTimer.Stop();
        mbIsInAnimation = false;
    }
}

void Animation::Draw(OutputDevice& rOut, const Point& rDestPt) const
{
    Draw(rOut, rDestPt, rOut.PixelToLogic(maGlobalSize));
}

void Animation::Draw(OutputDevice& rOut, const Point& rDestPt, const Size& rDestSz) const
{
    const size_t nCount = maAnimationFrames.size();

    if (!nCount)
        return;

    AnimationBitmap* pObj = maAnimationFrames[std::min(mnFrameIndex, nCount - 1)].get();

    if (rOut.GetConnectMetaFile() || (rOut.GetOutDevType() == OUTDEV_PRINTER))
        maAnimationFrames[0]->maBitmapEx.Draw(&rOut, rDestPt, rDestSz);
    else if (ANIMATION_TIMEOUT_ON_CLICK == pObj->mnWait)
        pObj->maBitmapEx.Draw(&rOut, rDestPt, rDestSz);
    else
    {
        const size_t nOldPos = mnFrameIndex;
        if (mbLoopTerminated)
            const_cast<Animation*>(this)->mnFrameIndex = nCount - 1;

        {
            AnimationRenderer{ const_cast<Animation*>(this), &rOut, rDestPt, rDestSz, 0 };
        }

        const_cast<Animation*>(this)->mnFrameIndex = nOldPos;
    }
}

namespace
{
constexpr sal_uLong constMinTimeout = 2;
}

void Animation::RestartTimer(sal_uLong nTimeout)
{
    maTimer.SetTimeout(std::max(nTimeout, constMinTimeout) * 10);
    maTimer.Start();
}

IMPL_LINK_NOARG(Animation, ImplTimeoutHdl, Timer*, void)
{
    const size_t nAnimCount = maAnimationFrames.size();

    if (nAnimCount)
    {
        bool bIsGloballyPaused = false;

        if (maNotifyLink.IsSet())
        {
            std::vector<std::unique_ptr<AnimationData>> aAnimationDataList;
            // create AnimationData-List
            for (auto const& i : maAnimationRenderers)
                aAnimationDataList.emplace_back(i->createAnimationData());

            maNotifyLink.Call(this);

            // set view state from AnimationData structure
            for (auto& pAnimationData : aAnimationDataList)
            {
                AnimationRenderer* pRenderer = nullptr;
                if (!pAnimationData->pViewData)
                {
                    pRenderer = new AnimationRenderer(
                        this, pAnimationData->pOutDev, pAnimationData->aStartOrg,
                        pAnimationData->aStartSize, pAnimationData->nCallerId);

                    maAnimationRenderers.push_back(std::unique_ptr<AnimationRenderer>(pRenderer));
                }
                else
                    pRenderer = static_cast<AnimationRenderer*>(pAnimationData->pViewData);

                pRenderer->pause(pAnimationData->bPause);
                pRenderer->setMarked(true);
            }

            // delete all unmarked views
            auto removeStart
                = std::remove_if(maAnimationRenderers.begin(), maAnimationRenderers.end(),
                                 [](const auto& pRenderer) { return !pRenderer->isMarked(); });
            maAnimationRenderers.erase(removeStart, maAnimationRenderers.cend());

            // check if every remaining view is paused
            bIsGloballyPaused
                = std::all_of(maAnimationRenderers.cbegin(), maAnimationRenderers.cend(),
                              [](const auto& pRenderer) { return pRenderer->isPause(); });

            // reset marked state
            std::for_each(maAnimationRenderers.cbegin(), maAnimationRenderers.cend(),
                          [](const auto& pRenderer) { pRenderer->setMarked(false); });
        }
        else
        {
            bIsGloballyPaused = false;
        }

        if (maAnimationRenderers.empty())
            Stop();
        else if (bIsGloballyPaused)
            RestartTimer(10);
        else
        {
            AnimationBitmap* pStepBmp = (++mnFrameIndex < maAnimationFrames.size())
                                            ? maAnimationFrames[mnFrameIndex].get()
                                            : nullptr;

            if (!pStepBmp)
            {
                if (mnLoops == 1)
                {
                    Stop();
                    mbLoopTerminated = true;
                    mnFrameIndex = nAnimCount - 1;
                    maBitmapEx = maAnimationFrames[mnFrameIndex]->maBitmapEx;
                    return;
                }
                else
                {
                    if (mnLoops)
                        mnLoops--;

                    mnFrameIndex = 0;
                    pStepBmp = maAnimationFrames[mnFrameIndex].get();
                }
            }

            // Paint all views.
            std::for_each(maAnimationRenderers.cbegin(), maAnimationRenderers.cend(),
                          [this](const auto& pRenderer) { pRenderer->draw(mnFrameIndex); });
            /*
             * If a view is marked, remove the view, because
             * area of output lies out of display area of window.
             * Mark state is set from view itself.
             */
            auto removeStart
                = std::remove_if(maAnimationRenderers.begin(), maAnimationRenderers.end(),
                                 [](const auto& pRenderer) { return pRenderer->isMarked(); });
            maAnimationRenderers.erase(removeStart, maAnimationRenderers.cend());

            // stop or restart timer
            if (maAnimationRenderers.empty())
                Stop();
            else
                RestartTimer(pStepBmp->mnWait);
        }
    }
    else
        Stop();
}

bool Animation::Insert(const AnimationBitmap& rStepBmp)
{
    bool bRet = false;

    if (!IsInAnimation())
    {
        tools::Rectangle aGlobalRect(Point(), maGlobalSize);

        maGlobalSize
            = aGlobalRect.Union(tools::Rectangle(rStepBmp.maPositionPixel, rStepBmp.maSizePixel))
                  .GetSize();
        maAnimationFrames.emplace_back(new AnimationBitmap(rStepBmp));

        // As a start, we make the first BitmapEx the replacement BitmapEx
        if (maAnimationFrames.size() == 1)
            maBitmapEx = rStepBmp.maBitmapEx;

        bRet = true;
    }

    return bRet;
}

const AnimationBitmap& Animation::Get(sal_uInt16 nAnimation) const
{
    SAL_WARN_IF((nAnimation >= maAnimationFrames.size()), "vcl", "No object at this position");
    return *maAnimationFrames[nAnimation];
}

void Animation::Replace(const AnimationBitmap& rNewAnimationBitmap, sal_uInt16 nAnimation)
{
    SAL_WARN_IF((nAnimation >= maAnimationFrames.size()), "vcl", "No object at this position");

    maAnimationFrames[nAnimation].reset(new AnimationBitmap(rNewAnimationBitmap));

    // If we insert at first position we also need to
    // update the replacement BitmapEx
    if ((!nAnimation && (!mbLoopTerminated || (maAnimationFrames.size() == 1)))
        || ((nAnimation == maAnimationFrames.size() - 1) && mbLoopTerminated))
    {
        maBitmapEx = rNewAnimationBitmap.maBitmapEx;
    }
}

void Animation::SetLoopCount(const sal_uInt32 nLoopCount)
{
    mnLoopCount = nLoopCount;
    ResetLoopCount();
}

void Animation::ResetLoopCount()
{
    mnLoops = mnLoopCount;
    mbLoopTerminated = false;
}

void Animation::Convert(BmpConversion eConversion)
{
    SAL_WARN_IF(IsInAnimation(), "vcl", "Animation modified while it is animated");

    bool bRet;

    if (!IsInAnimation() && !maAnimationFrames.empty())
    {
        bRet = true;

        for (size_t i = 0, n = maAnimationFrames.size(); (i < n) && bRet; ++i)
            bRet = maAnimationFrames[i]->maBitmapEx.Convert(eConversion);

        maBitmapEx.Convert(eConversion);
    }
}

bool Animation::ReduceColors(sal_uInt16 nNewColorCount)
{
    SAL_WARN_IF(IsInAnimation(), "vcl", "Animation modified while it is animated");

    bool bRet;

    if (!IsInAnimation() && !maAnimationFrames.empty())
    {
        bRet = true;

        for (size_t i = 0, n = maAnimationFrames.size(); (i < n) && bRet; ++i)
        {
            bRet = BitmapFilter::Filter(maAnimationFrames[i]->maBitmapEx,
                                        BitmapColorQuantizationFilter(nNewColorCount));
        }

        BitmapFilter::Filter(maBitmapEx, BitmapColorQuantizationFilter(nNewColorCount));
    }
    else
    {
        bRet = false;
    }

    return bRet;
}

bool Animation::Invert()
{
    SAL_WARN_IF(IsInAnimation(), "vcl", "Animation modified while it is animated");

    bool bRet;

    if (!IsInAnimation() && !maAnimationFrames.empty())
    {
        bRet = true;

        for (size_t i = 0, n = maAnimationFrames.size(); (i < n) && bRet; ++i)
            bRet = maAnimationFrames[i]->maBitmapEx.Invert();

        maBitmapEx.Invert();
    }
    else
        bRet = false;

    return bRet;
}

void Animation::Mirror(BmpMirrorFlags nMirrorFlags)
{
    SAL_WARN_IF(IsInAnimation(), "vcl", "Animation modified while it is animated");

    bool bRet;

    if (IsInAnimation() || maAnimationFrames.empty())
        return;

    bRet = true;

    if (nMirrorFlags == BmpMirrorFlags::NONE)
        return;

    for (size_t i = 0, n = maAnimationFrames.size(); (i < n) && bRet; ++i)
    {
        AnimationBitmap* pStepBmp = maAnimationFrames[i].get();
        bRet = pStepBmp->maBitmapEx.Mirror(nMirrorFlags);
        if (bRet)
        {
            if (nMirrorFlags & BmpMirrorFlags::Horizontal)
                pStepBmp->maPositionPixel.setX(maGlobalSize.Width() - pStepBmp->maPositionPixel.X()
                                               - pStepBmp->maSizePixel.Width());

            if (nMirrorFlags & BmpMirrorFlags::Vertical)
                pStepBmp->maPositionPixel.setY(maGlobalSize.Height() - pStepBmp->maPositionPixel.Y()
                                               - pStepBmp->maSizePixel.Height());
        }
    }

    maBitmapEx.Mirror(nMirrorFlags);
}

void Animation::Adjust(short nLuminancePercent, short nContrastPercent, short nChannelRPercent,
                       short nChannelGPercent, short nChannelBPercent, double fGamma, bool bInvert)
{
    SAL_WARN_IF(IsInAnimation(), "vcl", "Animation modified while it is animated");

    bool bRet;

    if (IsInAnimation() || maAnimationFrames.empty())
        return;

    bRet = true;

    for (size_t i = 0, n = maAnimationFrames.size(); (i < n) && bRet; ++i)
    {
        bRet = maAnimationFrames[i]->maBitmapEx.Adjust(nLuminancePercent, nContrastPercent,
                                                       nChannelRPercent, nChannelGPercent,
                                                       nChannelBPercent, fGamma, bInvert);
    }

    maBitmapEx.Adjust(nLuminancePercent, nContrastPercent, nChannelRPercent, nChannelGPercent,
                      nChannelBPercent, fGamma, bInvert);
}

SvStream& WriteAnimation(SvStream& rOStm, const Animation& rAnimation)
{
    const sal_uInt16 nCount = rAnimation.Count();

    if (nCount)
    {
        const sal_uInt32 nDummy32 = 0;

        // If no BitmapEx was set we write the first Bitmap of
        // the Animation
        if (rAnimation.GetBitmapEx().GetBitmap().IsEmpty())
            WriteDIBBitmapEx(rAnimation.Get(0).maBitmapEx, rOStm);
        else
            WriteDIBBitmapEx(rAnimation.GetBitmapEx(), rOStm);

        // Write identifier ( SDANIMA1 )
        rOStm.WriteUInt32(0x5344414e).WriteUInt32(0x494d4931);

        for (sal_uInt16 i = 0; i < nCount; i++)
        {
            const AnimationBitmap& rAnimationBitmap = rAnimation.Get(i);
            const sal_uInt16 nRest = nCount - i - 1;

            // Write AnimationBitmap
            WriteDIBBitmapEx(rAnimationBitmap.maBitmapEx, rOStm);
            tools::GenericTypeSerializer aSerializer(rOStm);
            aSerializer.writePoint(rAnimationBitmap.maPositionPixel);
            aSerializer.writeSize(rAnimationBitmap.maSizePixel);
            aSerializer.writeSize(rAnimation.maGlobalSize);
            rOStm.WriteUInt16((ANIMATION_TIMEOUT_ON_CLICK == rAnimationBitmap.mnWait)
                                  ? 65535
                                  : rAnimationBitmap.mnWait);
            rOStm.WriteUInt16(static_cast<sal_uInt16>(rAnimationBitmap.meDisposal));
            rOStm.WriteBool(rAnimationBitmap.mbUserInput);
            rOStm.WriteUInt32(rAnimation.mnLoopCount);
            rOStm.WriteUInt32(nDummy32); // Unused
            rOStm.WriteUInt32(nDummy32); // Unused
            rOStm.WriteUInt32(nDummy32); // Unused
            write_uInt16_lenPrefixed_uInt8s_FromOString(rOStm, ""); // dummy
            rOStm.WriteUInt16(nRest); // Count of remaining structures
        }
    }

    return rOStm;
}

SvStream& ReadAnimation(SvStream& rIStm, Animation& rAnimation)
{
    sal_uLong nStmPos;
    sal_uInt32 nAnimMagic1, nAnimMagic2;
    SvStreamEndian nOldFormat = rIStm.GetEndian();
    bool bReadAnimations = false;

    rIStm.SetEndian(SvStreamEndian::LITTLE);
    nStmPos = rIStm.Tell();
    rIStm.ReadUInt32(nAnimMagic1).ReadUInt32(nAnimMagic2);

    rAnimation.Clear();

    // If the BitmapEx at the beginning have already been read (by Graphic)
    // we can start reading the AnimationBitmaps right away
    if ((nAnimMagic1 == 0x5344414e) && (nAnimMagic2 == 0x494d4931) && !rIStm.GetError())
        bReadAnimations = true;
    // Else, we try reading the Bitmap(-Ex)
    else
    {
        rIStm.Seek(nStmPos);
        ReadDIBBitmapEx(rAnimation.maBitmapEx, rIStm);
        nStmPos = rIStm.Tell();
        rIStm.ReadUInt32(nAnimMagic1).ReadUInt32(nAnimMagic2);

        if ((nAnimMagic1 == 0x5344414e) && (nAnimMagic2 == 0x494d4931) && !rIStm.GetError())
            bReadAnimations = true;
        else
            rIStm.Seek(nStmPos);
    }

    // Read AnimationBitmaps
    if (bReadAnimations)
    {
        AnimationBitmap aAnimationBitmap;
        sal_uInt32 nTmp32;
        sal_uInt16 nTmp16;
        bool cTmp;

        do
        {
            ReadDIBBitmapEx(aAnimationBitmap.maBitmapEx, rIStm);
            tools::GenericTypeSerializer aSerializer(rIStm);
            aSerializer.readPoint(aAnimationBitmap.maPositionPixel);
            aSerializer.readSize(aAnimationBitmap.maSizePixel);
            aSerializer.readSize(rAnimation.maGlobalSize);
            rIStm.ReadUInt16(nTmp16);
            aAnimationBitmap.mnWait = ((65535 == nTmp16) ? ANIMATION_TIMEOUT_ON_CLICK : nTmp16);
            rIStm.ReadUInt16(nTmp16);
            aAnimationBitmap.meDisposal = static_cast<Disposal>(nTmp16);
            rIStm.ReadCharAsBool(cTmp);
            aAnimationBitmap.mbUserInput = cTmp;
            rIStm.ReadUInt32(rAnimation.mnLoopCount);
            rIStm.ReadUInt32(nTmp32); // Unused
            rIStm.ReadUInt32(nTmp32); // Unused
            rIStm.ReadUInt32(nTmp32); // Unused
            read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm); // Unused
            rIStm.ReadUInt16(nTmp16); // The rest to read

            rAnimation.Insert(aAnimationBitmap);
        } while (nTmp16 && !rIStm.GetError());

        rAnimation.ResetLoopCount();
    }

    rIStm.SetEndian(nOldFormat);

    return rIStm;
}

AnimationData::AnimationData()
    : pOutDev(nullptr)
    , pViewData(nullptr)
    , nCallerId(0)
    , bPause(false)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
