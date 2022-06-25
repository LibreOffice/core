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

#include <sal/config.h>

#include <vcl/animate/Animation.hxx>
#include <vcl/outdev.hxx>

#include <animate/AnimationRenderer.hxx>

#include <algorithm>

void Animation::ClearAnimationRenderers() { maRenderers.clear(); }

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
        if (!pDataItem->pRendererData)
        {
            pRenderer = new AnimationRenderer(this, pDataItem->pOutDev, pDataItem->aStartOrg,
                                              pDataItem->aStartSize, pDataItem->nRendererId);

            maRenderers.push_back(std::unique_ptr<AnimationRenderer>(pRenderer));
        }
        else
        {
            pRenderer = static_cast<AnimationRenderer*>(pDataItem->pRendererData);
        }

        pRenderer->pause(pDataItem->bPause);
        pRenderer->setMarked(true);
    }
}

void Animation::RenderNextFrameInAllRenderers()
{
    AnimationBitmap* pCurrentFrameBmp
        = (++mnFrameIndex < maFrames.size()) ? maFrames[mnFrameIndex].get() : nullptr;

    if (!pCurrentFrameBmp)
    {
        if (mnLoops == 1)
        {
            Stop();
            mbLoopTerminated = true;
            mnFrameIndex = mnAnimCount - 1;
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
    auto removeStart = std::remove_if(maRenderers.begin(), maRenderers.end(),
                                      [](const auto& pRenderer) { return pRenderer->isMarked(); });
    maRenderers.erase(removeStart, maRenderers.cend());

    // stop or restart timer
    if (maRenderers.empty())
        Stop();
    else
        ImplRestartTimer(pCurrentFrameBmp->mnWait);
}

void Animation::PruneMarkedRenderers()
{
    // delete all unmarked views
    auto removeStart = std::remove_if(maRenderers.begin(), maRenderers.end(),
                                      [](const auto& pRenderer) { return !pRenderer->isMarked(); });
    maRenderers.erase(removeStart, maRenderers.cend());

    // reset marked state
    std::for_each(maRenderers.cbegin(), maRenderers.cend(),
                  [](const auto& pRenderer) { pRenderer->setMarked(false); });
}

bool Animation::AllRenderersPaused()
{
    return !std::any_of(maRenderers.cbegin(), maRenderers.cend(),
                        [](const auto& pRenderer) { return !pRenderer->isPause(); });
}

bool Animation::Repaint(OutputDevice& rOut, tools::Long nRendererId, Point const& rDestPt,
                        Size const& rDestSz)
{
    bool bRepainted = false;

    auto itRenderer = std::find_if(
        maRenderers.begin(), maRenderers.end(),
        [&rOut, nRendererId](const std::unique_ptr<AnimationRenderer>& pRenderer) -> bool {
            return pRenderer->matches(&rOut, nRendererId);
        });

    if (itRenderer != maRenderers.end())
    {
        if ((*itRenderer)->getOutPos() == rDestPt
            && (*itRenderer)->getOutSizePix() == rOut.LogicToPixel(rDestSz))
        {
            (*itRenderer)->repaint();
            bRepainted = true;
        }
        else
        {
            maRenderers.erase(itRenderer);
        }
    }

    return bRepainted;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
