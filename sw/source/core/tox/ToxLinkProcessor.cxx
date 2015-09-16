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
    m_StartedLinks.push_back(std::unique_ptr<StartedLink>(
                new StartedLink(startPosition, characterStyle)));
}

void
ToxLinkProcessor::CloseLink(sal_Int32 endPosition, const OUString& url)
{
    StartedLink const startedLink( (m_StartedLinks.empty())
        ? StartedLink(0, SW_RES(STR_POOLCHR_TOXJUMP))
        : *m_StartedLinks.back() );
    if (!m_StartedLinks.empty())
    {
        m_StartedLinks.pop_back();
    }

    if (url.isEmpty()) {
        return;
    }

    std::unique_ptr<ClosedLink> pClosedLink(
            new ClosedLink(url, startedLink.mStartPosition, endPosition));

    const OUString& characterStyle = startedLink.mCharacterStyle;
    sal_uInt16 poolId = ObtainPoolId(characterStyle);
    pClosedLink->mINetFormat.SetVisitedFormatAndId(characterStyle, poolId);
    pClosedLink->mINetFormat.SetINetFormatAndId(characterStyle, poolId);

    m_ClosedLinks.push_back(std::move(pClosedLink));
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
    for (auto const& clink : m_ClosedLinks)
    {
        node.InsertItem(clink->mINetFormat, clink->mStartTextPos, clink->mEndTextPos);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
