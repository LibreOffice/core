/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SlideLink.hxx>

#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

#include <comphelper/kit.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <svl/undo.hxx>
#include <tools/json_writer.hxx>
#include <tools/urlobj.hxx>

#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <strings.hrc>

namespace sd
{
namespace
{
/// The scheme a source document is named under.
constexpr OUString gSourceScheme = u"vnd.collabora.slide-source:"_ustr;

/// Whether rUrl names a file on this machine.
bool isLocalFile(std::u16string_view rUrl)
{
    const INetURLObject aUrl(rUrl);
    return aUrl.GetProtocol() == INetProtocol::File && aUrl.GetHost().isEmpty();
}

/** The pages of rDoc linked to rReference, by their index in the standard page list, in document
    order.
*/
std::vector<sal_uInt16> getLinkedPages(const SdDrawDocument& rDoc, std::u16string_view rReference)
{
    std::vector<sal_uInt16> aPages;
    for (sal_uInt16 nIndex = 0, nCount = rDoc.GetSdPageCount(PageKind::Standard); nIndex < nCount;
         ++nIndex)
    {
        const SdPage* pPage = rDoc.GetSdPage(nIndex, PageKind::Standard);
        if (pPage && std::u16string_view(pPage->GetFileName()) == rReference
            && !pPage->GetBookmarkName().isEmpty())
            aPages.push_back(nIndex);
    }
    return aPages;
}

}

OUString SlideLink::MakeSourceReference(const OUString& rSourceName)
{
    const OUString aEscaped = rtl::Uri::encode(rSourceName, rtl_UriCharClassPchar,
                                               rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8);
    return gSourceScheme + rtl::Uri::decode(aEscaped, rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8);
}

OUString SlideLink::GetSourceName(std::u16string_view rReference)
{
    if (!o3tl::starts_with(rReference, std::u16string_view(gSourceScheme)))
        return OUString();

    return rtl::Uri::decode(OUString(rReference.substr(gSourceScheme.getLength())),
                            rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
}

OUString SlideLink::GetSourceFile(const SdDrawDocument& rDoc, const OUString& rReference)
{
    const OUString aStagedFile = rDoc.GetStagedLinkSourceFile(rReference);
    if (!aStagedFile.isEmpty())
        return aStagedFile;

    // A source document is named rather than addressed, so its pages come from the file staged for
    // it and from nothing else.
    if (rReference.startsWith(gSourceScheme))
        return OUString();

    // A document served by a kit holds no capability to reach anything but this machine.
    if (comphelper::COKit::isActive() && !isLocalFile(rReference))
        return OUString();

    return rReference;
}

void SlideLink::WriteLinks(const SdDrawDocument& rDoc, tools::JsonWriter& rJsonWriter)
{
    // The pages of one source, by the reference they record, in the order the sources first appear.
    std::vector<std::pair<OUString, std::vector<const SdPage*>>> aSources;

    for (sal_uInt16 nIndex = 0, nCount = rDoc.GetSdPageCount(PageKind::Standard); nIndex < nCount;
         ++nIndex)
    {
        const SdPage* pPage = rDoc.GetSdPage(nIndex, PageKind::Standard);
        if (!pPage || pPage->GetBookmarkName().isEmpty()
            || GetSourceName(pPage->GetFileName()).isEmpty())
            continue;

        auto it = std::find_if(aSources.begin(), aSources.end(),
                               [pPage](const auto& rSource)
                               { return rSource.first == pPage->GetFileName(); });
        if (it == aSources.end())
        {
            aSources.push_back({ pPage->GetFileName(), { pPage } });
            continue;
        }
        it->second.push_back(pPage);
    }

    auto aLinksArray = rJsonWriter.startArray("links");
    for (const auto& rSource : aSources)
    {
        auto aSourceNode = rJsonWriter.startStruct();
        rJsonWriter.put("source", GetSourceName(rSource.first));
        auto aSlidesArray = rJsonWriter.startArray("slides");
        for (const SdPage* pPage : rSource.second)
        {
            auto aSlideNode = rJsonWriter.startStruct();
            rJsonWriter.put("part", pPage->GetGuid().getString());
            rJsonWriter.put("name", pPage->GetBookmarkName());
        }
    }
}

sal_Int32 SlideLink::Refresh(SdDrawDocument& rDoc, const OUString& rSourceName,
                             const OUString& rFileUrl)
{
    const OUString aReference = MakeSourceReference(rSourceName);
    const std::vector<sal_uInt16> aLinkedPages = getLinkedPages(rDoc, aReference);
    if (aLinkedPages.empty())
        return -1;

    // The pages come from a file on this machine, so that a refresh needs no network.
    if (!isLocalFile(rFileUrl))
    {
        SAL_WARN("sd", "slide link refresh: the pages are not staged in a file on this machine");
        return -1;
    }

    // A file staged under a name used before holds other pages this time, so the document reads the
    // file it is given now.
    rDoc.CloseBookmarkDoc();
    SdDrawDocument* pSourceDoc = rDoc.OpenBookmarkDoc(rFileUrl, /*bNoDialogs=*/true);
    if (!pSourceDoc)
        return -1;

    // The file stays with the document, so that a later update of its links reads this same file.
    rDoc.SetStagedLinkSourceFile(aReference, rFileUrl);

    // The pages the file holds a slide for, by their index in the standard page list. A page whose
    // slide the file has lost keeps the content it holds.
    std::vector<sal_uInt16> aPages;
    for (sal_uInt16 nIndex : aLinkedPages)
    {
        const SdPage* pPage = rDoc.GetSdPage(nIndex, PageKind::Standard);
        if (!pPage)
            continue;

        // A page of that name in the master pages is a design rather than a slide, so the file
        // holds no slide to read.
        bool bIsMasterPage = false;
        const OUString aSourcePage = pPage->GetBookmarkName();
        if (pSourceDoc->GetPageByName(aSourcePage, bIsMasterPage) == SDRPAGE_NOTFOUND
            || bIsMasterPage)
        {
            SAL_WARN("sd", "slide link refresh: no slide named " << aSourcePage << " in the file");
            continue;
        }

        aPages.push_back(nIndex);
    }

    SfxUndoManager* pUndoManager = rDoc.beginUndoAction(SdResId(STR_UNDO_UPDATE_LINKED_SLIDES));

    sal_Int32 nRefreshed = 0;
    // One resolution reads a run of slides into the pages from the position of its first one, so
    // the pages that sit next to each other are resolved together. Every resolution of its own
    // copies the styles of the file and sweeps the master pages of the whole document again.
    for (size_t nFirst = 0; nFirst < aPages.size();)
    {
        size_t nPast = nFirst + 1;
        while (nPast < aPages.size() && aPages[nPast] == aPages[nPast - 1] + 1)
            ++nPast;

        std::vector<OUString> aSourcePages;
        std::vector<OString> aPageIds;
        for (size_t nPos = nFirst; nPos < nPast; ++nPos)
        {
            // The positions were collected before any of the resolutions above, each of which
            // reads slides into the document, so a position holds a page as far as the document
            // still reaches.
            const SdPage* pPage = rDoc.GetSdPage(aPages[nPos], PageKind::Standard);
            if (!pPage)
                break;

            aSourcePages.push_back(pPage->GetBookmarkName());
            aPageIds.push_back(pPage->GetGuid().getString());
        }

        if (aSourcePages.empty())
        {
            nFirst = nPast;
            continue;
        }

        const sal_uInt16 nInsertPos
            = rDoc.GetSdPage(aPages[nFirst], PageKind::Standard)->GetPageNum();
        rDoc.ResolvePageLinks(aSourcePages, nInsertPos, /*bNoDialogs=*/true, /*bCopy=*/true,
                              aReference);

        // A resolution puts the slide it read in place of the page at its position, and a page read
        // from a file holds an identifier of its own, so the identifier a position holds now says
        // whether that page was refreshed.
        for (size_t nPos = 0; nPos < aPageIds.size(); ++nPos)
        {
            const SdPage* pPage = rDoc.GetSdPage(aPages[nFirst + nPos], PageKind::Standard);
            if (pPage && pPage->GetGuid().getString() != aPageIds[nPos])
                ++nRefreshed;
        }

        nFirst = nPast;
    }

    rDoc.endUndoAction(/*bUndo=*/false, pUndoManager);
    rDoc.CloseBookmarkDoc();

    return nRefreshed;
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
