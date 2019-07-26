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

#include <AnimationRenderer.hxx>
#include <AnimationRenderers.hxx>
#include <AnimationData.hxx>

void AnimationRenderers::ClearAnimationRenderers() { maRenderers.clear(); }

bool AnimationRenderers::RepaintRenderers(OutputDevice* pOut, sal_uLong nCallerId,
                                          const Point& rDestPt, const Size& rDestSz)
{
    AnimationRenderer* pRenderer;

    for (size_t i = 0; i < maRenderers.size(); ++i)
    {
        pRenderer = maRenderers[i].get();
        if (pRenderer->Matches(pOut, nCallerId))
        {
            if (pRenderer->GetOriginPosition() == rDestPt
                && pRenderer->GetSizePx() == pOut->LogicToPixel(rDestSz))
            {
                pRenderer->Repaint();
                return true;
            }
            else
            {
                maRenderers.erase(maRenderers.begin() + i);
                return false;
            }
        }
    }

    return false;
}

bool AnimationRenderers::NoRenderersAreAvailable() { return maRenderers.empty(); }

std::vector<std::unique_ptr<AnimationData>> AnimationRenderers::CreateAnimationDataItems()
{
    std::vector<std::unique_ptr<AnimationData>> aAnimationDataItems;

    for (auto const& rItem : maRenderers)
    {
        aAnimationDataItems.emplace_back(rItem->CreateAnimationData());
    }

    return aAnimationDataItems;
}

void AnimationRenderers::CreateDefaultRenderer(Animation* pAnim, OutputDevice* pOut,
                                               const Point& rDestPt, const Size& rDestSz,
                                               sal_uLong nCallerId, OutputDevice* pFirstFrameOutDev)
{
    maRenderers.emplace_back(
        pOut->CreateAnimationRenderer(pAnim, pOut, rDestPt, rDestSz, nCallerId, pFirstFrameOutDev));
}

void AnimationRenderers::RemoveAnimationInstance(OutputDevice* pOut, sal_uLong nCallerId)
{
    maRenderers.erase(
        std::remove_if(maRenderers.begin(), maRenderers.end(),
                       [=](const std::unique_ptr<AnimationRenderer>& pAnimView) -> bool {
                           return pAnimView->Matches(pOut, nCallerId);
                       }),
        maRenderers.end());
}

void AnimationRenderers::PopulateRenderers(Animation* pAnim)
{
    AnimationRenderer* pRenderer;

    for (auto& pItem : CreateAnimationDataItems())
    {
        if (!pItem->mpAnimationRenderer)
        {
            pRenderer = pItem->mpRenderContext->CreateAnimationRenderer(
                pAnim, pItem->mpRenderContext, pItem->maOriginStartPt, pItem->maStartSize,
                pItem->mnCallerId);

            maRenderers.push_back(std::unique_ptr<AnimationRenderer>(pRenderer));
        }
        else
        {
            pRenderer = static_cast<AnimationRenderer*>(pItem->mpAnimationRenderer);
        }

        pRenderer->Pause(pItem->mbIsPaused);
        pRenderer->SetMarked(true);
    }
}

void AnimationRenderers::DeleteUnmarkedRenderers()
{
    AnimationRenderer* pRenderer;

    for (size_t i = 0; i < maRenderers.size();)
    {
        pRenderer = maRenderers[i].get();
        if (!pRenderer->IsMarked())
            maRenderers.erase(maRenderers.begin() + i);
    }
}

bool AnimationRenderers::ResetMarkedRenderers()
{
    bool bIsGloballyPaused = true;

    for (auto& rItem : maRenderers)
    {
        if (!rItem->IsPaused())
            bIsGloballyPaused = false;

        rItem->SetMarked(false);
    }

    return bIsGloballyPaused;
}

void AnimationRenderers::PaintRenderers(sal_uLong nFrameIndex)
{
    for (auto& rRenderer : maRenderers)
    {
        rRenderer->Draw(nFrameIndex);
    }
}

void AnimationRenderers::EraseMarkedRenderers()
{
    for (size_t i = 0; i < maRenderers.size();)
    {
        AnimationRenderer* pRenderer = maRenderers[i].get();

        if (pRenderer->IsMarked())
            maRenderers.erase(maRenderers.begin() + i);
        else
            i++;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
