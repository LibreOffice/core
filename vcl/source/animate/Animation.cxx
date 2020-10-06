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

#include <impanmvw.hxx>

sal_uLong Animation::mnAnimCount = 0;

Animation::Animation()
    : mnLoopCount(0)
    , mnLoops(0)
    , mnPos(0)
    , mbIsInAnimation(false)
    , mbLoopTerminated(false)
{
    maTimer.SetInvokeHandler(LINK(this, Animation, ImplTimeoutHdl));
}

Animation::Animation(const Animation& rAnimation)
    : maBitmapEx(rAnimation.maBitmapEx)
    , maGlobalSize(rAnimation.maGlobalSize)
    , mnLoopCount(rAnimation.mnLoopCount)
    , mnPos(rAnimation.mnPos)
    , mbIsInAnimation(false)
    , mbLoopTerminated(rAnimation.mbLoopTerminated)
{
    for (auto const& i : rAnimation.maList)
        maList.emplace_back(new AnimationBitmap(*i));

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

        for (auto const& i : rAnimation.maList)
            maList.emplace_back(new AnimationBitmap(*i));

        maGlobalSize = rAnimation.maGlobalSize;
        maBitmapEx = rAnimation.maBitmapEx;
        mnLoopCount = rAnimation.mnLoopCount;
        mnPos = rAnimation.mnPos;
        mbLoopTerminated = rAnimation.mbLoopTerminated;
        mnLoops = mbLoopTerminated ? 0 : mnLoopCount;
    }
    return *this;
}

bool Animation::operator==(const Animation& rAnimation) const
{
    return maList.size() == rAnimation.maList.size() && maBitmapEx == rAnimation.maBitmapEx
           && maGlobalSize == rAnimation.maGlobalSize
           && std::equal(maList.begin(), maList.end(), rAnimation.maList.begin(),
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
    maList.clear();
    maViewList.clear();
}

bool Animation::IsTransparent() const
{
    tools::Rectangle aRect{ Point(), maGlobalSize };

    // If some small bitmap needs to be replaced by the background,
    // we need to be transparent, in order to be displayed correctly
    // as the application (?) does not invalidate on non-transparent
    // graphics due to performance reasons.

    return maBitmapEx.IsTransparent()
           || std::any_of(maList.begin(), maList.end(),
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

    for (auto const& pAnimationBitmap : maList)
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

    UInt32ToSVBT32(maList.size(), aBT32);
    nCrc = vcl_get_checksum(nCrc, aBT32, 4);

    Int32ToSVBT32(maGlobalSize.Width(), aBT32);
    nCrc = vcl_get_checksum(nCrc, aBT32, 4);

    Int32ToSVBT32(maGlobalSize.Height(), aBT32);
    nCrc = vcl_get_checksum(nCrc, aBT32, 4);

    for (auto const& i : maList)
    {
        BCToBCOA(i->GetChecksum(), aBCOA);
        nCrc = vcl_get_checksum(nCrc, aBCOA, BITMAP_CHECKSUM_SIZE);
    }

    return nCrc;
}

bool Animation::Start(OutputDevice* pOut, const Point& rDestPt, const Size& rDestSz,
                      long nExtraData, OutputDevice* pFirstFrameOutDev)
{
    bool bRet = false;

    if (!maList.empty())
    {
        if ((pOut->GetOutDevType() == OUTDEV_WINDOW) && !mbLoopTerminated
            && (ANIMATION_TIMEOUT_ON_CLICK != maList[mnPos]->mnWait))
        {
            bool differs = true;

            auto itAnimView = std::find_if(
                maViewList.begin(), maViewList.end(),
                [pOut, nExtraData](const std::unique_ptr<ImplAnimView>& pAnimView) -> bool {
                    return pAnimView->matches(pOut, nExtraData);
                });

            if (itAnimView != maViewList.end())
            {
                if ((*itAnimView)->getOutPos() == rDestPt
                    && (*itAnimView)->getOutSizePix() == pOut->LogicToPixel(rDestSz))
                {
                    (*itAnimView)->repaint();
                    differs = false;
                }
                else
                    maViewList.erase(itAnimView);
            }

            if (maViewList.empty())
            {
                maTimer.Stop();
                mbIsInAnimation = false;
                mnPos = 0;
            }

            if (differs)
                maViewList.emplace_back(
                    new ImplAnimView(this, pOut, rDestPt, rDestSz, nExtraData, pFirstFrameOutDev));

            if (!mbIsInAnimation)
            {
                ImplRestartTimer(maList[mnPos]->mnWait);
                mbIsInAnimation = true;
            }
        }
        else
            Draw(pOut, rDestPt, rDestSz);

        bRet = true;
    }

    return bRet;
}

void Animation::Stop(const OutputDevice* pOut, long nExtraData)
{
    maViewList.erase(std::remove_if(maViewList.begin(), maViewList.end(),
                                    [=](const std::unique_ptr<ImplAnimView>& pAnimView) -> bool {
                                        return pAnimView->matches(pOut, nExtraData);
                                    }),
                     maViewList.end());

    if (maViewList.empty())
    {
        maTimer.Stop();
        mbIsInAnimation = false;
    }
}

void Animation::Draw(OutputDevice* pOut, const Point& rDestPt) const
{
    Draw(pOut, rDestPt, pOut->PixelToLogic(maGlobalSize));
}

void Animation::Draw(OutputDevice* pOut, const Point& rDestPt, const Size& rDestSz) const
{
    const size_t nCount = maList.size();

    if (!nCount)
        return;

    AnimationBitmap* pObj = maList[std::min(mnPos, nCount - 1)].get();

    if (pOut->GetConnectMetaFile() || (pOut->GetOutDevType() == OUTDEV_PRINTER))
        maList[0]->maBitmapEx.Draw(pOut, rDestPt, rDestSz);
    else if (ANIMATION_TIMEOUT_ON_CLICK == pObj->mnWait)
        pObj->maBitmapEx.Draw(pOut, rDestPt, rDestSz);
    else
    {
        const size_t nOldPos = mnPos;
        if (mbLoopTerminated)
            const_cast<Animation*>(this)->mnPos = nCount - 1;

        {
            ImplAnimView{ const_cast<Animation*>(this), pOut, rDestPt, rDestSz, 0 };
        }

        const_cast<Animation*>(this)->mnPos = nOldPos;
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

IMPL_LINK_NOARG(Animation, ImplTimeoutHdl, Timer*, void)
{
    const size_t nAnimCount = maList.size();

    if (nAnimCount)
    {
        bool bGlobalPause = false;

        if (maNotifyLink.IsSet())
        {
            std::vector<std::unique_ptr<AInfo>> aAInfoList;
            // create AInfo-List
            for (auto const& i : maViewList)
                aAInfoList.emplace_back(i->createAInfo());

            maNotifyLink.Call(this);

            // set view state from AInfo structure
            for (auto& pAInfo : aAInfoList)
            {
                ImplAnimView* pView = nullptr;
                if (!pAInfo->pViewData)
                {
                    pView = new ImplAnimView(this, pAInfo->pOutDev, pAInfo->aStartOrg,
                                             pAInfo->aStartSize, pAInfo->nExtraData);

                    maViewList.push_back(std::unique_ptr<ImplAnimView>(pView));
                }
                else
                    pView = static_cast<ImplAnimView*>(pAInfo->pViewData);

                pView->pause(pAInfo->bPause);
                pView->setMarked(true);
            }

            // delete all unmarked views
            auto removeStart = std::remove_if(maViewList.begin(), maViewList.end(),
                                              [](const auto& pView) { return !pView->isMarked(); });
            maViewList.erase(removeStart, maViewList.cend());

            // check if every remaining view is paused
            bGlobalPause = std::all_of(maViewList.cbegin(), maViewList.cend(),
                                       [](const auto& pView) { return pView->isPause(); });

            // reset marked state
            std::for_each(maViewList.cbegin(), maViewList.cend(),
                          [](const auto& pView) { pView->setMarked(false); });
        }

        if (maViewList.empty())
            Stop();
        else if (bGlobalPause)
            ImplRestartTimer(10);
        else
        {
            AnimationBitmap* pStepBmp = (++mnPos < maList.size()) ? maList[mnPos].get() : nullptr;

            if (!pStepBmp)
            {
                if (mnLoops == 1)
                {
                    Stop();
                    mbLoopTerminated = true;
                    mnPos = nAnimCount - 1;
                    maBitmapEx = maList[mnPos]->maBitmapEx;
                    return;
                }
                else
                {
                    if (mnLoops)
                        mnLoops--;

                    mnPos = 0;
                    pStepBmp = maList[mnPos].get();
                }
            }

            // Paint all views.
            std::for_each(maViewList.cbegin(), maViewList.cend(),
                          [this](const auto& pView) { pView->draw(mnPos); });
            /*
             * If a view is marked, remove the view, because
             * area of output lies out of display area of window.
             * Mark state is set from view itself.
             */
            auto removeStart = std::remove_if(maViewList.begin(), maViewList.end(),
                                              [](const auto& pView) { return pView->isMarked(); });
            maViewList.erase(removeStart, maViewList.cend());

            // stop or restart timer
            if (maViewList.empty())
                Stop();
            else
                ImplRestartTimer(pStepBmp->mnWait);
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
        maList.emplace_back(new AnimationBitmap(rStepBmp));

        // As a start, we make the first BitmapEx the replacement BitmapEx
        if (maList.size() == 1)
            maBitmapEx = rStepBmp.maBitmapEx;

        bRet = true;
    }

    return bRet;
}

const AnimationBitmap& Animation::Get(sal_uInt16 nAnimation) const
{
    SAL_WARN_IF((nAnimation >= maList.size()), "vcl", "No object at this position");
    return *maList[nAnimation];
}

void Animation::Replace(const AnimationBitmap& rNewAnimationBitmap, sal_uInt16 nAnimation)
{
    SAL_WARN_IF((nAnimation >= maList.size()), "vcl", "No object at this position");

    maList[nAnimation].reset(new AnimationBitmap(rNewAnimationBitmap));

    // If we insert at first position we also need to
    // update the replacement BitmapEx
    if ((!nAnimation && (!mbLoopTerminated || (maList.size() == 1)))
        || ((nAnimation == maList.size() - 1) && mbLoopTerminated))
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

    if (!IsInAnimation() && !maList.empty())
    {
        bRet = true;

        for (size_t i = 0, n = maList.size(); (i < n) && bRet; ++i)
            bRet = maList[i]->maBitmapEx.Convert(eConversion);

        maBitmapEx.Convert(eConversion);
    }
}

bool Animation::ReduceColors(sal_uInt16 nNewColorCount)
{
    SAL_WARN_IF(IsInAnimation(), "vcl", "Animation modified while it is animated");

    bool bRet;

    if (!IsInAnimation() && !maList.empty())
    {
        bRet = true;

        for (size_t i = 0, n = maList.size(); (i < n) && bRet; ++i)
        {
            bRet = BitmapFilter::Filter(maList[i]->maBitmapEx,
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

    if (!IsInAnimation() && !maList.empty())
    {
        bRet = true;

        for (size_t i = 0, n = maList.size(); (i < n) && bRet; ++i)
            bRet = maList[i]->maBitmapEx.Invert();

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

    if (IsInAnimation() || maList.empty())
        return;

    bRet = true;

    if (nMirrorFlags == BmpMirrorFlags::NONE)
        return;

    for (size_t i = 0, n = maList.size(); (i < n) && bRet; ++i)
    {
        AnimationBitmap* pStepBmp = maList[i].get();
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

    if (IsInAnimation() || maList.empty())
        return;

    bRet = true;

    for (size_t i = 0, n = maList.size(); (i < n) && bRet; ++i)
    {
        bRet = maList[i]->maBitmapEx.Adjust(nLuminancePercent, nContrastPercent, nChannelRPercent,
                                            nChannelGPercent, nChannelBPercent, fGamma, bInvert);
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
        if (!rAnimation.GetBitmapEx().GetBitmap())
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
            write_uInt16_lenPrefixed_uInt8s_FromOString(rOStm, OString()); // dummy
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

AInfo::AInfo()
    : pOutDev(nullptr)
    , pViewData(nullptr)
    , nExtraData(0)
    , bPause(false)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
