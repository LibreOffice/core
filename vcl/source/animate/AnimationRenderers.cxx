/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <vcl/animate/Animation.hxx>
#include <vcl/outdev.hxx>

#include <animate/AnimationRenderer.hxx>

#include <algorithm>

std::vector<std::unique_ptr<AnimationData>> Animation::CreateAnimationDataItems()
{
    std::vector<std::unique_ptr<AnimationData>> aDataItems;

    for (auto const& rItem : maRenderers)
    {
        aDataItems.emplace_back(rItem->CreateAnimationData());
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
            pRenderer = static_cast<AnimationRenderer*>(pDataItem->mpRendererData);
        }

        pRenderer->Pause(pDataItem->mbIsPaused);
        pRenderer->SetMarked(true);
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
                  [this](const auto& pRenderer) { pRenderer->Draw(mnFrameIndex); });
    /*
     * If a view is marked, remove the view, because
     * area of output lies out of display area of window.
     * Mark state is set from view itself.
     */
    auto removeStart = std::remove_if(maRenderers.begin(), maRenderers.end(),
                                      [](const auto& pRenderer) { return pRenderer->IsMarked(); });
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
                                      [](const auto& pRenderer) { return !pRenderer->IsMarked(); });
    maRenderers.erase(removeStart, maRenderers.cend());

    // reset marked state
    std::for_each(maRenderers.cbegin(), maRenderers.cend(),
                  [](const auto& pRenderer) { pRenderer->SetMarked(false); });
}

bool Animation::AllRenderersPaused()
{
    return !std::any_of(maRenderers.cbegin(), maRenderers.cend(),
                        [](const auto& pRenderer) { return !pRenderer->IsPaused(); });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
