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

#include <comphelper/kit.hxx>
#include <o3tl/string_view.hxx>
#include <svl/undo.hxx>
#include <tools/Guid.hxx>
#include <tools/urlobj.hxx>

#include <DrawDocShell.hxx>
#include <SlideLinkManager.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <unopage.hxx>

namespace sd
{
namespace
{
/// The scheme every reference of our own is under, none of which names a file.
constexpr OUString gOwnScheme = u"vnd.collabora."_ustr;

/// The reference the pages of a written presentation record, which names no document.
constexpr OUString gOriginReference = u"vnd.collabora.slide-origin:self"_ustr;

/// The name ODF keeps for a slide of no name of its own, before the position of the slide.
constexpr OUString gPositionNamePrefix = u"page"_ustr;

/// Whether rUrl names a file on this machine.
bool isLocalFile(std::u16string_view rUrl)
{
    const INetURLObject aUrl(rUrl);
    return aUrl.GetProtocol() == INetProtocol::File && aUrl.GetHost().isEmpty();
}
}

OUString SlideLink::OriginReference() { return gOriginReference; }

bool SlideLink::IsOwnReference(std::u16string_view rReference)
{
    return o3tl::starts_with(rReference, std::u16string_view(gOwnScheme));
}

OUString SlideLink::GetOriginPage(const SdPage& rPage)
{
    if (rPage.GetFileName() != gOriginReference)
        return OUString();

    return rPage.GetBookmarkName();
}

OUString SlideLink::MakePositionName(sal_Int32 nPosition)
{
    const OUString aApiName = gPositionNamePrefix + OUString::number(nPosition);
    return SdDrawPage::getUiNameFromPageApiName(aApiName);
}

sal_Int32 SlideLink::GetNamedPosition(const OUString& rName)
{
    // A name of a slide's own is kept as it is, and a name of a position is the prefix ODF keeps
    // for one and the number of the position.
    const OUString aApiName = SdDrawPage::getPageApiNameFromUiName(rName);
    if (aApiName == rName || !aApiName.startsWith(gPositionNamePrefix))
        return 0;

    const std::u16string_view aPosition = aApiName.subView(gPositionNamePrefix.getLength());
    return aPosition.find_first_not_of(u"0123456789") == std::u16string_view::npos
               ? o3tl::toInt32(aPosition)
               : 0;
}

OUString SlideLink::NormaliseGuid(std::u16string_view rGuid)
{
    const tools::Guid aGuid(OUStringToOString(rGuid, RTL_TEXTENCODING_ASCII_US));
    return aGuid.isEmpty() ? OUString() : aGuid.getOUString();
}

bool SlideLink::IsLocalFile(std::u16string_view rUrl) { return isLocalFile(rUrl); }

bool SlideLink::IsReadableSourceString(std::u16string_view rValue)
{
    constexpr size_t nMaxLength = 128;
    return rValue.size() <= nMaxLength
           && std::none_of(rValue.begin(), rValue.end(), [](sal_Unicode c) { return c < 0x20; });
}

bool SlideLink::IsReadablePersistentLink(std::u16string_view rLink)
{
    constexpr size_t nMaxLength = 1024;
    return !rLink.empty() && rLink.size() <= nMaxLength
           && std::none_of(rLink.begin(), rLink.end(), [](sal_Unicode c) { return c < 0x20; });
}

sal_Int32 SlideLink::GetPageIndex(const SdDrawDocument& rDoc, std::u16string_view rGuid)
{
    if (rGuid.empty())
        return -1;

    for (sal_uInt16 nIndex = 0, nCount = rDoc.GetSdPageCount(PageKind::Standard); nIndex < nCount;
         ++nIndex)
    {
        const SdPage* pPage = rDoc.GetSdPage(nIndex, PageKind::Standard);
        if (pPage && pPage->GetGuid().getOUString() == rGuid)
            return nIndex;
    }
    return -1;
}

OUString SlideLink::GetSourceFile(std::u16string_view rFileName)
{
    if (IsOwnReference(rFileName))
        return OUString();

    // A document served by a kit holds no capability to reach anything but this machine.
    if (comphelper::COKit::isActive() && !isLocalFile(rFileName))
        return OUString();

    return OUString(rFileName);
}

void SlideLink::ClearPageReference(SdPage& rPage)
{
    rPage.DisconnectLink();
    rPage.SetFileName(OUString());
    rPage.SetBookmarkName(OUString());
}

sal_Int32 SlideLink::InsertPages(SdDrawDocument& rDoc, SdDrawDocument& rSource,
                                 const std::vector<OUString>& rNames,
                                 const InsertBookmarkOptions& rOptions, sal_uInt16 nInsertPosition)
{
    // The standard page of slide S sits at page position 2*S+1.
    const sal_uInt16 nSlidesBefore = rDoc.GetSdPageCount(PageKind::Standard);
    const sal_uInt16 nFirstSlide
        = nInsertPosition == 0xFFFF ? nSlidesBefore : (nInsertPosition - 1) / 2;
    if (!rDoc.InsertFileAsPage(rNames, /*pExchangeList=*/nullptr, rOptions, nInsertPosition,
                               rSource.GetDocSh(), /*oScaleObjects=*/true))
        return -1;

    const sal_uInt16 nInsertedCount = rDoc.GetSdPageCount(PageKind::Standard) - nSlidesBefore;
    for (sal_uInt16 nSlide = nFirstSlide; nSlide < nFirstSlide + nInsertedCount; ++nSlide)
    {
        if (SdPage* pPage = rDoc.GetSdPage(nSlide, PageKind::Standard))
            ClearPageReference(*pPage);
    }
    return nInsertedCount;
}

void SlideLink::BreakOnEdit(SdDrawDocument& rDoc, sal_Int32 nIndex)
{
    if (rDoc.ArePageLinksKept())
        return;

    ::sd::DrawDocShell* pDocShell = rDoc.GetDocSh();
    SfxUndoManager* pUndoManager = pDocShell ? pDocShell->GetUndoManager() : nullptr;
    if (!pUndoManager)
        return;

    // The link comes off as part of the edit that changed the page, so there has to be an undo
    // step in hand to record that in. A change outside a step is the engine's own work on the
    // page: an undo or a redo putting it back the way it was, a document being read, a linked
    // graphic given fresh content, an embedded object reporting the size it wants.
    if (!pUndoManager->IsUndoEnabled() || !pUndoManager->IsInListAction())
        return;

    if (nIndex < 0 || nIndex >= rDoc.GetSdPageCount(PageKind::Standard))
        return;

    const SdPage* pPage = rDoc.GetSdPage(static_cast<sal_uInt16>(nIndex), PageKind::Standard);
    if (!pPage)
        return;

    rDoc.GetSlideLinkManager().Break(SlideLinkKind::Slide, pPage->GetGuid().getOUString(),
                                     /*bDeep=*/false);
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
