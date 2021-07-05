/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <SearchResultLocator.hxx>
#include <node.hxx>
#include <drawdoc.hxx>
#include <frame.hxx>
#include <cntfrm.hxx>
#include <viewsh.hxx>
#include <IDocumentLayoutAccess.hxx>

namespace sw
{
LocationResult SearchResultLocator::find(SearchIndexData const& rSearchIndexData)
{
    LocationResult aResult;
    SwNodes const& rNodes = mpDocument->GetNodes();
    if (rSearchIndexData.nNodeIndex >= rNodes.Count())
        return aResult;
    SwNode* pNode = rNodes[rSearchIndexData.nNodeIndex];

    auto* pContentNode = pNode->GetContentNode();
    auto* pShell = mpDocument->getIDocumentLayoutAccess().GetCurrentViewShell();

    if (pContentNode && pShell)
    {
        const SwFrame* pFrame = pContentNode->getLayoutFrame(pShell->GetLayout(), nullptr, nullptr);
        SwRect const& rArea = pFrame->getFrameArea();

        aResult.mbFound = true;
        aResult.maRectangles.emplace_back(rArea.Left(), rArea.Top(), rArea.Left() + rArea.Width(),
                                          rArea.Top() + rArea.Height());
    }

    return aResult;
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
