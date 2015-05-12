/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ToxLinkProcessor.hxx"

#include "SwStyleNameMapper.hxx"
#include "ndtxt.hxx"
#include <poolfmt.hrc>

#include <stdexcept>

namespace sw {

void
ToxLinkProcessor::StartNewLink(sal_Int32 startPosition, const OUString& characterStyle)
{
    mStartedLinks.push_back(new StartedLink(startPosition, characterStyle));
}

void
ToxLinkProcessor::CloseLink(sal_Int32 endPosition, const OUString& url)
{
    StartedLink const startedLink( (mStartedLinks.empty())
        ? StartedLink(0, SW_RES(STR_POOLCHR_TOXJUMP))
        : mStartedLinks.back() );
    if (!mStartedLinks.empty())
    {
        mStartedLinks.pop_back();
    }

    if (url.isEmpty()) {
        return;
    }

    ClosedLink* closedLink = new ClosedLink(url, startedLink.mStartPosition, endPosition);

    const OUString& characterStyle = startedLink.mCharacterStyle;
    sal_uInt16 poolId = ObtainPoolId(characterStyle);
    closedLink->mINetFormat.SetVisitedFormatAndId(characterStyle, poolId);
    closedLink->mINetFormat.SetINetFormatAndId(characterStyle, poolId);

    mClosedLinks.push_back(closedLink);
}

sal_uInt16
ToxLinkProcessor::ObtainPoolId(const OUString& characterStyle) const
{
    if (characterStyle.isEmpty()) {
        return USHRT_MAX;
    }
    else {
        return SwStyleNameMapper::GetPoolIdFromUIName(characterStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
    }
}


void
ToxLinkProcessor::InsertLinkAttributes(SwTextNode& node)
{
    for (ClosedLink& clink : mClosedLinks) {
        node.InsertItem(clink.mINetFormat, clink.mStartTextPos, clink.mEndTextPos);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
