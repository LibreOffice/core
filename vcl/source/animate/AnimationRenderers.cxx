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
#include <vcl/animate/AnimationRenderer.hxx>
#include <vcl/outdev.hxx>

#include <AnimationData.hxx>

void AnimationRenderers::ClearAnimationRenderers() { maAnimationRenderers.clear(); }

bool AnimationRenderers::CanRepaintRenderers(OutputDevice* pOut, sal_uLong nCallerId,
                                             const Point& rDestPt, const Size& rDestSz)
{
    AnimationRenderer* pRenderer;

    for (size_t i = 0; i < maAnimationRenderers.size(); ++i)
    {
        pRenderer = maAnimationRenderers[i].get();
        if (pRenderer->matches(pOut, nCallerId))
        {
            if (pRenderer->GetOriginPosition() == rDestPt
                && pRenderer->GetSizePx() == pOut->LogicToPixel(rDestSz))
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

bool AnimationRenderers::NoRenderersAreAvailable() { return maAnimationRenderers.empty(); }

std::vector<std::unique_ptr<AnimationData>> AnimationRenderers::CreateAnimationDataItems()
{
    std::vector<std::unique_ptr<AnimationData>> aAnimationDataItems;

    for (auto const& rItem : maAnimationRenderers)
    {
        aAnimationDataItems.emplace_back(rItem->createAnimationData());
    }

    return aAnimationDataItems;
}

void AnimationRenderers::CreateDefaultRenderer(Animation* pAnim, OutputDevice* pOut,
                                               const Point& rDestPt, const Size& rDestSz,
                                               sal_uLong nCallerId, OutputDevice* pFirstFrameOutDev)
{
    maAnimationRenderers.emplace_back(
        pOut->CreateAnimationRenderer(pAnim, pOut, rDestPt, rDestSz, nCallerId, pFirstFrameOutDev));
}

void AnimationRenderers::RemoveAnimationInstance(OutputDevice* pOut, sal_uLong nCallerId)
{
    maAnimationRenderers.erase(
        std::remove_if(maAnimationRenderers.begin(), maAnimationRenderers.end(),
                       [=](const std::unique_ptr<AnimationRenderer>& pAnimView) -> bool {
                           return pAnimView->matches(pOut, nCallerId);
                       }),
        maAnimationRenderers.end());
}

void AnimationRenderers::PopulateRenderers(Animation* pAnim)
{
    AnimationRenderer* pRenderer;

    for (auto& pItem : CreateAnimationDataItems())
    {
        if (!pItem->pAnimationRenderer)
        {
            pRenderer = pItem->pOutDev->CreateAnimationRenderer(
                pAnim, pItem->pOutDev, pItem->aStartOrg, pItem->aStartSize, pItem->nCallerId);

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

void AnimationRenderers::DeleteUnmarkedRenderers()
{
    AnimationRenderer* pRenderer;

    for (size_t i = 0; i < maAnimationRenderers.size();)
    {
        pRenderer = maAnimationRenderers[i].get();
        if (!pRenderer->isMarked())
            maAnimationRenderers.erase(maAnimationRenderers.begin() + i);
    }
}

bool AnimationRenderers::CanResetMarkedRenderers()
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

void AnimationRenderers::PaintRenderers(sal_uLong nFrameIndex)
{
    for (auto& rRenderer : maAnimationRenderers)
    {
        rRenderer->draw(nFrameIndex);
    }
}

void AnimationRenderers::EraseMarkedRenderers()
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
