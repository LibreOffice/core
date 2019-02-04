/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <ToxLinkProcessor.hxx>

#include <SwStyleNameMapper.hxx>
#include <ndtxt.hxx>
#include <sal/log.hxx>

#include <stdexcept>

namespace sw {

void
ToxLinkProcessor::StartNewLink(sal_Int32 startPosition, const OUString& characterStyle)
{
    SAL_INFO_IF(m_pStartedLink, "sw.core", "ToxLinkProcessor: LS without LE");
    m_pStartedLink = std::make_unique<StartedLink>(
                startPosition, characterStyle);
}

void
ToxLinkProcessor::CloseLink(sal_Int32 endPosition, const OUString& url)
{
    if (m_pStartedLink == nullptr)
    {
        SAL_INFO("sw.core", "ToxLinkProcessor: LE without LS");
        return;
    }

    if (url.isEmpty()) {
        return;
    }

    std::unique_ptr<ClosedLink> pClosedLink(
            new ClosedLink(url, m_pStartedLink->mStartPosition, endPosition));

    const OUString& characterStyle = m_pStartedLink->mCharacterStyle;
    sal_uInt16 poolId = ObtainPoolId(characterStyle);
    pClosedLink->mINetFormat.SetVisitedFormatAndId(characterStyle, poolId);
    pClosedLink->mINetFormat.SetINetFormatAndId(characterStyle, poolId);

    m_ClosedLinks.push_back(std::move(pClosedLink));
    m_pStartedLink.reset();
}

sal_uInt16
ToxLinkProcessor::ObtainPoolId(const OUString& characterStyle) const
{
    if (characterStyle.isEmpty()) {
        return USHRT_MAX;
    }
    else {
        return SwStyleNameMapper::GetPoolIdFromUIName(characterStyle, SwGetPoolIdFromName::ChrFmt);
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
