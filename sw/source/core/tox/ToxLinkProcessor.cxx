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
#include <rtl/uri.hxx>

namespace sw {

void
ToxLinkProcessor::StartNewLink(sal_Int32 startPosition, const OUString& characterStyle)
{
    SAL_INFO_IF(m_oStartedLink, "sw.core", "ToxLinkProcessor: LS without LE");
    m_oStartedLink.emplace(startPosition, characterStyle);
}

void ToxLinkProcessor::CloseLink(sal_Int32 endPosition, const OUString& url, const OUString& sAltText, bool bRelative)
{
    if (!m_oStartedLink)
    {
        SAL_INFO("sw.core", "ToxLinkProcessor: LE without LS");
        return;
    }

    if (url.isEmpty()) {
        return;
    }

    OUString uri;
    if (bRelative)
    {
        // url contains '|' which must be encoded; also in some cases contains
        // arbitrary strings that need to be encoded
        assert(url[0] == '#'); // all links are internal
        uri = "#"
              + rtl::Uri::encode(url.copy(1), rtl_UriCharClassUricNoSlash,
                                 rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8);
    }
    else
    {
        uri = url;
    }

    std::unique_ptr<ClosedLink> pClosedLink(
            new ClosedLink(uri, m_oStartedLink->mStartPosition, endPosition));

    const OUString& characterStyle = m_oStartedLink->mCharacterStyle;
    sal_uInt16 poolId = ObtainPoolId(characterStyle);
    pClosedLink->mINetFormat.SetVisitedFormatAndId(characterStyle, poolId);
    pClosedLink->mINetFormat.SetINetFormatAndId(characterStyle, poolId);
    pClosedLink->mINetFormat.SetName(sAltText);

    m_ClosedLinks.push_back(std::move(pClosedLink));
    m_oStartedLink.reset();
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
