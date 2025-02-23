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

#include <rtl/crc.h>
#include <tools/stream.hxx>
#include <tools/GenericTypeSerializer.hxx>
#include <sal/log.hxx>

#include <vcl/animate/Animation.hxx>
#include <vcl/bitmap/BitmapColorQuantizationFilter.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/outdev.hxx>

#include <animate/AnimationRenderer.hxx>

sal_uLong Animation::gAnimationRendererCount = 0;

Animation::Animation()
    : maTimer("vcl::Animation")
    , mnLoopCount(0)
    , mnLoops(0)
    , mnFrameIndex(0)
    , mbIsInAnimation(false)
    , mbLoopTerminated(false)
{
    maTimer.SetInvokeHandler(LINK(this, Animation, ImplTimeoutHdl));
}

Animation::Animation(const Animation& rAnimation)
    : maBitmapEx(rAnimation.maBitmapEx)
    , maTimer("vcl::Animation")
    , maGlobalSize(rAnimation.maGlobalSize)
    , mnLoopCount(rAnimation.mnLoopCount)
    , mnFrameIndex(rAnimation.mnFrameIndex)
    , mbIsInAnimation(false)
    , mbLoopTerminated(rAnimation.mbLoopTerminated)
{
    for (auto const& rFrame : rAnimation.maFrames)
        maFrames.emplace_back(new AnimationFrame(*rFrame));

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

        for (auto const& i : rAnimation.maFrames)
            maFrames.emplace_back(new AnimationFrame(*i));

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
    return maFrames.size() == rAnimation.maFrames.size() && maBitmapEx == rAnimation.maBitmapEx
           && maGlobalSize == rAnimation.maGlobalSize
           && std::equal(maFrames.begin(), maFrames.end(), rAnimation.maFrames.begin(),
                         [](const std::unique_ptr<AnimationFrame>& pAnim1,
                            const std::unique_ptr<AnimationFrame>& pAnim2) -> bool {
                             return *pAnim1 == *pAnim2;
                         });
}

void Animation::Clear()
{
    maTimer.Stop();
    mbIsInAnimation = false;
    maGlobalSize = Size();
    maBitmapEx.SetEmpty();
    maFrames.clear();
    maRenderers.clear();
}

bool Animation::IsTransparent() const
{
    tools::Rectangle aRect{ Point(), maGlobalSize };

    // If some small bitmap needs to be replaced by the background,
    // we need to be transparent, in order to be displayed correctly
    // as the application (?) does not invalidate on non-transparent
    // graphics due to performance reasons.

    return maBitmapEx.IsAlpha()
           || std::any_of(maFrames.begin(), maFrames.end(),
                          [&aRect](const std::unique_ptr<AnimationFrame>& pAnim) -> bool {
                              return pAnim->meDisposal == Disposal::Back
                                     && tools::Rectangle{ pAnim->maPositionPixel,
                                                          pAnim->maSizePixel }
                                            != aRect;
                          });
}

sal_uLong Animation::GetSizeBytes() const
{
    sal_uLong nSizeBytes = GetBitmapEx().GetSizeBytes();

    for (auto const& pAnimationFrame : maFrames)
    {
        nSizeBytes += pAnimationFrame->maBitmapEx.GetSizeBytes();
    }

    return nSizeBytes;
}

BitmapChecksum Animation::GetChecksum() const
{
    SVBT32 aBT32;
    BitmapChecksumOctetArray aBCOA;
    BitmapChecksum nCrc = GetBitmapEx().GetChecksum();

    UInt32ToSVBT32(maFrames.size(), aBT32);
    nCrc = rtl_crc32(nCrc, aBT32, 4);

    Int32ToSVBT32(maGlobalSize.Width(), aBT32);
    nCrc = rtl_crc32(nCrc, aBT32, 4);

    Int32ToSVBT32(maGlobalSize.Height(), aBT32);
    nCrc = rtl_crc32(nCrc, aBT32, 4);

    for (auto const& i : maFrames)
    {
        BCToBCOA(i->GetChecksum(), aBCOA);
        nCrc = rtl_crc32(nCrc, aBCOA, BITMAP_CHECKSUM_SIZE);
    }

    return nCrc;
}

bool Animation::Start(OutputDevice& rOut, const Point& rDestPt, const Size& rDestSz,
                      tools::Long nRendererId, OutputDevice* pFirstFrameOutDev)
{
    if (maFrames.empty())
        return false;

    if (rOut.CanAnimate() && !mbLoopTerminated
        && (ANIMATION_TIMEOUT_ON_CLICK != maFrames[mnFrameIndex]->mnWait))
    {
        bool differs = true;

        auto itAnimView = std::find_if(
            maRenderers.begin(), maRenderers.end(),
            [&rOut, nRendererId](const std::unique_ptr<AnimationRenderer>& pRenderer) -> bool {
                return pRenderer->matches(&rOut, nRendererId);
            });

        if (itAnimView != maRenderers.end())
        {
            if ((*itAnimView)->getOriginPosition() == rDestPt
                && (*itAnimView)->getOutSizePix() == rOut.LogicToPixel(rDestSz))
            {
                (*itAnimView)->repaint();
                differs = false;
            }
            else
            {
                maRenderers.erase(itAnimView);
            }
        }

        if (maRenderers.empty())
        {
            maTimer.Stop();
            mbIsInAnimation = false;
            mnFrameIndex = 0;
        }

        if (differs)
            maRenderers.emplace_back(new AnimationRenderer(this, &rOut, rDestPt, rDestSz,
                                                           nRendererId, pFirstFrameOutDev));

        if (!mbIsInAnimation)
        {
            ImplRestartTimer(maFrames[mnFrameIndex]->mnWait);
            mbIsInAnimation = true;
        }
    }
    else
    {
        Draw(rOut, rDestPt, rDestSz);
    }

    return true;
}

void Animation::Stop(const OutputDevice* pOut, tools::Long nRendererId)
{
    std::erase_if(maRenderers, [=](const std::unique_ptr<AnimationRenderer>& pRenderer) -> bool {
        return pRenderer->matches(pOut, nRendererId);
    });

    if (maRenderers.empty())
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
    const size_t nCount = maFrames.size();

    if (!nCount)
        return;

    AnimationFrame* pObj = maFrames[std::min(mnFrameIndex, nCount - 1)].get();

    if (rOut.GetConnectMetaFile() || (rOut.GetOutDevType() == OUTDEV_PRINTER))
    {
        maFrames[0]->maBitmapEx.Draw(&rOut, rDestPt, rDestSz);
    }
    else if (ANIMATION_TIMEOUT_ON_CLICK == pObj->mnWait)
    {
        pObj->maBitmapEx.Draw(&rOut, rDestPt, rDestSz);
    }
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

void Animation::ImplRestartTimer(sal_uLong nTimeout)
{
    maTimer.SetTimeout(std::max(nTimeout, constMinTimeout) * 10);
    maTimer.Start();
}

std::vector<std::unique_ptr<AnimationData>> Animation::CreateAnimationDataItems()
{
    std::vector<std::unique_ptr<AnimationData>> aDataItems;

    for (auto const& rItem : maRenderers)
    {
        aDataItems.emplace_back(rItem->createAnimationData());
    }

    return aDataItems;
}

void Animation::PopulateRenderers()
{
    for (auto& pDataItem : CreateAnimationDataItems())
    {
        AnimationRenderer* pRenderer = nullptr;
        if (!pDataItem->mpRendererData)
        {
            pRenderer = new AnimationRenderer(this, pDataItem->mpRenderContext,
                                              pDataItem->maOriginStartPt, pDataItem->maStartSize,
                                              pDataItem->mnRendererId);

            maRenderers.push_back(std::unique_ptr<AnimationRenderer>(pRenderer));
        }
        else
        {
            pRenderer = pDataItem->mpRendererData;
        }

        pRenderer->pause(pDataItem->mbIsPaused);
        pRenderer->setMarked(true);
    }
}

void Animation::RenderNextFrameInAllRenderers()
{
    AnimationFrame* pCurrentFrameBmp
        = (++mnFrameIndex < maFrames.size()) ? maFrames[mnFrameIndex].get() : nullptr;

    if (!pCurrentFrameBmp)
    {
        if (mnLoops == 1)
        {
            Stop();
            mbLoopTerminated = true;
            mnFrameIndex = maFrames.size() - 1;
            maBitmapEx = maFrames[mnFrameIndex]->maBitmapEx;
            return;
        }
        else
        {
            if (mnLoops)
                mnLoops--;

            mnFrameIndex = 0;
            pCurrentFrameBmp = maFrames[mnFrameIndex].get();
        }
    }

    // Paint all views.
    std::for_each(maRenderers.cbegin(), maRenderers.cend(),
                  [this](const auto& pRenderer) { pRenderer->draw(mnFrameIndex); });
    /*
     * If a view is marked, remove the view, because
     * area of output lies out of display area of window.
     * Mark state is set from view itself.
     */
    std::erase_if(maRenderers, [](const auto& pRenderer) { return pRenderer->isMarked(); });

    // stop or restart timer
    if (maRenderers.empty())
        Stop();
    else
        ImplRestartTimer(pCurrentFrameBmp->mnWait);
}

void Animation::PruneMarkedRenderers()
{
    // delete all unmarked views
    std::erase_if(maRenderers, [](const auto& pRenderer) { return !pRenderer->isMarked(); });

    // reset marked state
    std::for_each(maRenderers.cbegin(), maRenderers.cend(),
                  [](const auto& pRenderer) { pRenderer->setMarked(false); });
}

bool Animation::IsAnyRendererActive()
{
    return std::any_of(maRenderers.cbegin(), maRenderers.cend(),
                       [](const auto& pRenderer) { return !pRenderer->isPaused(); });
}

IMPL_LINK_NOARG(Animation, ImplTimeoutHdl, Timer*, void)
{
    const size_t nAnimCount = maFrames.size();

    if (!nAnimCount)
    {
        Stop();
        return;
    }

    bool bIsAnyRendererActive = true;

    if (maNotifyLink.IsSet())
    {
        maNotifyLink.Call(this);
        PopulateRenderers();
        PruneMarkedRenderers();
        bIsAnyRendererActive = IsAnyRendererActive();
    }

    if (maRenderers.empty())
        Stop();
    else if (!bIsAnyRendererActive)
        ImplRestartTimer(10);
    else
        RenderNextFrameInAllRenderers();
}

bool Animation::Insert(const AnimationFrame& rStepBmp)
{
    if (IsInAnimation())
        return false;

    tools::Rectangle aGlobalRect(Point(), maGlobalSize);

    maGlobalSize
        = aGlobalRect.Union(tools::Rectangle(rStepBmp.maPositionPixel, rStepBmp.maSizePixel))
              .GetSize();
    maFrames.emplace_back(new AnimationFrame(rStepBmp));

    // As a start, we make the first BitmapEx the replacement BitmapEx
    if (maFrames.size() == 1)
        maBitmapEx = rStepBmp.maBitmapEx;

    return true;
}

const AnimationFrame& Animation::Get(sal_uInt16 nAnimation) const
{
    SAL_WARN_IF((nAnimation >= maFrames.size()), "vcl", "No object at this position");
    return *maFrames[nAnimation];
}

void Animation::Replace(const AnimationFrame& rNewAnimationFrame, sal_uInt16 nAnimation)
{
    SAL_WARN_IF((nAnimation >= maFrames.size()), "vcl", "No object at this position");

    maFrames[nAnimation].reset(new AnimationFrame(rNewAnimationFrame));

    // If we insert at first position we also need to
    // update the replacement BitmapEx
    if ((!nAnimation && (!mbLoopTerminated || (maFrames.size() == 1)))
        || ((nAnimation == maFrames.size() - 1) && mbLoopTerminated))
    {
        maBitmapEx = rNewAnimationFrame.maBitmapEx;
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

    if (IsInAnimation() || maFrames.empty())
        return;

    bool bRet = true;

    for (size_t i = 0, n = maFrames.size(); (i < n) && bRet; ++i)
    {
        bRet = maFrames[i]->maBitmapEx.Convert(eConversion);
    }

    maBitmapEx.Convert(eConversion);
}

bool Animation::ReduceColors(sal_uInt16 nNewColorCount)
{
    SAL_WARN_IF(IsInAnimation(), "vcl", "Animation modified while it is animated");

    if (IsInAnimation() || maFrames.empty())
        return false;

    bool bRet = true;

    for (size_t i = 0, n = maFrames.size(); (i < n) && bRet; ++i)
    {
        bRet = BitmapFilter::Filter(maFrames[i]->maBitmapEx,
                                    BitmapColorQuantizationFilter(nNewColorCount));
    }

    BitmapFilter::Filter(maBitmapEx, BitmapColorQuantizationFilter(nNewColorCount));

    return bRet;
}

bool Animation::Invert()
{
    SAL_WARN_IF(IsInAnimation(), "vcl", "Animation modified while it is animated");

    if (IsInAnimation() || maFrames.empty())
        return false;

    bool bRet = true;

    for (size_t i = 0, n = maFrames.size(); (i < n) && bRet; ++i)
    {
        bRet = maFrames[i]->maBitmapEx.Invert();
    }

    maBitmapEx.Invert();

    return bRet;
}

void Animation::Mirror(BmpMirrorFlags nMirrorFlags)
{
    SAL_WARN_IF(IsInAnimation(), "vcl", "Animation modified while it is animated");

    if (IsInAnimation() || maFrames.empty())
        return;

    if (nMirrorFlags == BmpMirrorFlags::NONE)
        return;

    bool bRet = true;

    for (size_t i = 0, n = maFrames.size(); (i < n) && bRet; ++i)
    {
        AnimationFrame* pCurrentFrameBmp = maFrames[i].get();
        bRet = pCurrentFrameBmp->maBitmapEx.Mirror(nMirrorFlags);
        if (bRet)
        {
            if (nMirrorFlags & BmpMirrorFlags::Horizontal)
                pCurrentFrameBmp->maPositionPixel.setX(maGlobalSize.Width()
                                                       - pCurrentFrameBmp->maPositionPixel.X()
                                                       - pCurrentFrameBmp->maSizePixel.Width());

            if (nMirrorFlags & BmpMirrorFlags::Vertical)
                pCurrentFrameBmp->maPositionPixel.setY(maGlobalSize.Height()
                                                       - pCurrentFrameBmp->maPositionPixel.Y()
                                                       - pCurrentFrameBmp->maSizePixel.Height());
        }
    }

    maBitmapEx.Mirror(nMirrorFlags);
}

void Animation::Adjust(short nLuminancePercent, short nContrastPercent, short nChannelRPercent,
                       short nChannelGPercent, short nChannelBPercent, double fGamma, bool bInvert)
{
    SAL_WARN_IF(IsInAnimation(), "vcl", "Animation modified while it is animated");

    if (IsInAnimation() || maFrames.empty())
        return;

    bool bRet = true;

    for (size_t i = 0, n = maFrames.size(); (i < n) && bRet; ++i)
    {
        bRet = maFrames[i]->maBitmapEx.Adjust(nLuminancePercent, nContrastPercent, nChannelRPercent,
                                              nChannelGPercent, nChannelBPercent, fGamma, bInvert);
    }

    maBitmapEx.Adjust(nLuminancePercent, nContrastPercent, nChannelRPercent, nChannelGPercent,
                      nChannelBPercent, fGamma, bInvert);
}

SvStream& WriteAnimation(SvStream& rOStm, const Animation& rAnimation)
{
    const sal_uInt16 nCount = rAnimation.Count();

    if (!nCount)
        return rOStm;

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
        const AnimationFrame& rAnimationFrame = rAnimation.Get(i);
        const sal_uInt16 nRest = nCount - i - 1;

        // Write AnimationFrame
        WriteDIBBitmapEx(rAnimationFrame.maBitmapEx, rOStm);
        tools::GenericTypeSerializer aSerializer(rOStm);
        aSerializer.writePoint(rAnimationFrame.maPositionPixel);
        aSerializer.writeSize(rAnimationFrame.maSizePixel);
        aSerializer.writeSize(rAnimation.maGlobalSize);
        rOStm.WriteUInt16((ANIMATION_TIMEOUT_ON_CLICK == rAnimationFrame.mnWait)
                              ? 65535
                              : rAnimationFrame.mnWait);
        rOStm.WriteUInt16(static_cast<sal_uInt16>(rAnimationFrame.meDisposal));
        rOStm.WriteBool(rAnimationFrame.mbUserInput);
        rOStm.WriteUInt32(rAnimation.mnLoopCount);
        rOStm.WriteUInt32(nDummy32); // Unused
        rOStm.WriteUInt32(nDummy32); // Unused
        rOStm.WriteUInt32(nDummy32); // Unused
        write_uInt16_lenPrefixed_uInt8s_FromOString(rOStm, ""); // dummy
        rOStm.WriteUInt16(nRest); // Count of remaining structures
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
    // we can start reading the AnimationFrames right away
    if ((nAnimMagic1 == 0x5344414e) && (nAnimMagic2 == 0x494d4931) && !rIStm.GetError())
    {
        bReadAnimations = true;
    }
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

    // Read AnimationFrames
    if (bReadAnimations)
    {
        AnimationFrame aAnimationFrame;
        sal_uInt32 nTmp32;
        sal_uInt16 nTmp16;
        bool cTmp;

        do
        {
            ReadDIBBitmapEx(aAnimationFrame.maBitmapEx, rIStm);
            tools::GenericTypeSerializer aSerializer(rIStm);
            aSerializer.readPoint(aAnimationFrame.maPositionPixel);
            aSerializer.readSize(aAnimationFrame.maSizePixel);
            aSerializer.readSize(rAnimation.maGlobalSize);
            rIStm.ReadUInt16(nTmp16);
            aAnimationFrame.mnWait = ((65535 == nTmp16) ? ANIMATION_TIMEOUT_ON_CLICK : nTmp16);
            rIStm.ReadUInt16(nTmp16);
            aAnimationFrame.meDisposal = static_cast<Disposal>(nTmp16);
            rIStm.ReadCharAsBool(cTmp);
            aAnimationFrame.mbUserInput = cTmp;
            rIStm.ReadUInt32(rAnimation.mnLoopCount);
            rIStm.ReadUInt32(nTmp32); // Unused
            rIStm.ReadUInt32(nTmp32); // Unused
            rIStm.ReadUInt32(nTmp32); // Unused
            read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm); // Unused
            rIStm.ReadUInt16(nTmp16); // The rest to read

            rAnimation.Insert(aAnimationFrame);
        } while (nTmp16 && !rIStm.GetError());

        rAnimation.ResetLoopCount();
    }

    rIStm.SetEndian(nOldFormat);

    return rIStm;
}

AnimationData::AnimationData()
    : mpRenderContext(nullptr)
    , mpRendererData(nullptr)
    , mnRendererId(0)
    , mbIsPaused(false)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
