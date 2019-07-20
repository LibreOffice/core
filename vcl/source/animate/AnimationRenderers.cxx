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

#include <vcl/animate/Animation.hxx>
#include <vcl/outdev.hxx>

#include <AnimationData.hxx>
#include <AnimationRenderer.hxx>

void Animation::ClearAnimationRenderers() { maAnimationRenderers.clear(); }

bool Animation::CanRepaintRenderers(OutputDevice* pOut, sal_uLong nCallerId, const Point& rDestPt,
                                    const Size& rDestSz)
{
    AnimationRenderer* pRenderer;

    for (size_t i = 0; i < maAnimationRenderers.size(); ++i)
    {
        pRenderer = maAnimationRenderers[i].get();
        if (pRenderer->matches(pOut, nCallerId))
        {
            if (pRenderer->getOutPos() == rDestPt
                && pRenderer->getOutSizePix() == pOut->LogicToPixel(rDestSz))
            {
                pRenderer->repaint();
                return true;
            }
            else
            {
                maAnimationRenderers.erase(maAnimationRenderers.begin() + i);
                return false;
            }
        }
    }

    return false;
}

bool Animation::NoRenderersAreAvailable() { return maAnimationRenderers.empty(); }

std::vector<std::unique_ptr<AnimationData>> Animation::CreateAnimationDataItems()
{
    std::vector<std::unique_ptr<AnimationData>> aAnimationDataItems;

    for (auto const& rItem : maAnimationRenderers)
    {
        aAnimationDataItems.emplace_back(rItem->createAnimationData());
    }

    return aAnimationDataItems;
}

void Animation::PopulateRenderers()
{
    AnimationRenderer* pRenderer;

    for (auto& pItem : CreateAnimationDataItems())
    {
        if (!pItem->pAnimationRenderer)
        {
            pRenderer = new AnimationRenderer(this, pItem->pOutDev, pItem->aStartOrg,
                                              pItem->aStartSize, pItem->nCallerId);

            maAnimationRenderers.push_back(std::unique_ptr<AnimationRenderer>(pRenderer));
        }
        else
        {
            pRenderer = static_cast<AnimationRenderer*>(pItem->pAnimationRenderer);
        }

        pRenderer->pause(pItem->bPause);
        pRenderer->setMarked(true);
    }
}

void Animation::DeleteUnmarkedRenderers()
{
    AnimationRenderer* pRenderer;

    for (size_t i = 0; i < maAnimationRenderers.size();)
    {
        pRenderer = maAnimationRenderers[i].get();
        if (!pRenderer->isMarked())
            maAnimationRenderers.erase(maAnimationRenderers.begin() + i);
    }
}

bool Animation::CanResetMarkedRenderers()
{
    bool bIsGloballyPaused = true;

    for (auto& rItem : maAnimationRenderers)
    {
        if (!rItem->isPause())
            bIsGloballyPaused = false;

        rItem->setMarked(false);
    }

    return bIsGloballyPaused;
}

bool Animation::IsTimeoutSetup() { return maTimeoutNotifier.IsSet(); }

bool Animation::CanSendTimeout()
{
    if (IsTimeoutSetup())
    {
        maTimeoutNotifier.Call(this);
        PopulateRenderers();
        DeleteUnmarkedRenderers();
        return CanResetMarkedRenderers();
    }

    return false;
}

void Animation::PaintRenderers()
{
    for (auto& rRenderer : maAnimationRenderers)
    {
        rRenderer->draw(mnFrameIndex);
    }
}

void Animation::EraseMarkedRenderers()
{
    for (size_t i = 0; i < maAnimationRenderers.size();)
    {
        AnimationRenderer* pRenderer = maAnimationRenderers[i].get();

        if (pRenderer->isMarked())
            maAnimationRenderers.erase(maAnimationRenderers.begin() + i);
        else
            i++;
    }
}

AnimationBitmap* Animation::GetNextFrameBitmap()
{
    const size_t nAnimCount = maAnimationFrames.size();

    bool bIsFrameAtEnd = mnFrameIndex >= maAnimationFrames.size();
    mnFrameIndex++;

    AnimationBitmap* pCurrentFrameBmp
        = bIsFrameAtEnd ? nullptr : maAnimationFrames[mnFrameIndex].get();

    if (!pCurrentFrameBmp)
    {
        if (mnLoops == 1)
        {
            Stop();
            mbLoopTerminated = true;
            mnFrameIndex = nAnimCount - 1;
            maBitmapEx = maAnimationFrames[mnFrameIndex]->maBitmapEx;
        }
        else
        {
            if (mnLoops)
                mnLoops--;

            mnFrameIndex = 0;
            pCurrentFrameBmp = maAnimationFrames[mnFrameIndex].get();
        }
    }

    return pCurrentFrameBmp;
}

void Animation::RenderNextFrame()
{
    AnimationBitmap* pCurrentFrameBmp = GetNextFrameBitmap();
    if (pCurrentFrameBmp)
    {
        PaintRenderers();
        EraseMarkedRenderers();

        // stop or restart timer
        if (maAnimationRenderers.empty())
            Stop();
        else
            RestartTimer(pCurrentFrameBmp->mnWait);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
