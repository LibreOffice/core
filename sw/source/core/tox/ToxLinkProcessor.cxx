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

#include <boost/foreach.hpp>
#include <stdexcept>

namespace sw {

void
ToxLinkProcessor::StartNewLink(sal_Int32 startPosition, const OUString& characterStyle)
{
    mStartedLinks.push_back(StartedLink(startPosition, characterStyle));
}

void
ToxLinkProcessor::CloseLink(sal_Int32 endPosition, const OUString& url)
{
    if (mStartedLinks.empty()) {
        throw std::runtime_error("ToxLinkProcessor: More calls for CloseLink() than open links exist.");
    }
    StartedLink startedLink = mStartedLinks.back();
    mStartedLinks.pop_back();

    if (url.isEmpty()) {
        return;
    }

    ClosedLink closedLink(url, startedLink.mStartPosition, endPosition);

    const OUString& characterStyle = startedLink.mCharacterStyle;
    sal_uInt16 poolId = ObtainPoolId(characterStyle);
    closedLink.mINetFmt.SetVisitedFmtAndId(characterStyle, poolId);
    closedLink.mINetFmt.SetINetFmtAndId(characterStyle, poolId);

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
ToxLinkProcessor::InsertLinkAttributes(SwTxtNode& node)
{
    BOOST_FOREACH(ClosedLink& clink, mClosedLinks) {
        node.InsertItem(clink.mINetFmt, clink.mStartTextPos, clink.mEndTextPos);
    }
}

}

