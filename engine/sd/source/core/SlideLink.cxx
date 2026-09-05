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
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include <comphelper/kit.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/undo.hxx>
#include <tools/json_writer.hxx>
#include <tools/urlobj.hxx>

#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <sdundo.hxx>
#include <strings.hrc>
#include <unopage.hxx>

namespace sd
{
namespace
{
/// The scheme a source document is named under.
constexpr OUString gSourceScheme = u"vnd.collabora.slide-source:"_ustr;

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

/// Whether rPage records a slide of a source document: the name of that slide, its identifier, or
/// both. A page that records neither is a page of this document alone.
bool recordsSourceSlide(const SdPage& rPage)
{
    return !rPage.GetBookmarkName().isEmpty() || !rPage.GetSourcePageGuid().isEmpty();
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
            && recordsSourceSlide(*pPage))
            aPages.push_back(nIndex);
    }
    return aPages;
}

/// The slide of rSourceDoc that rPage is linked to, or nothing when the file holds no slide for it.
const SdPage* findSourcePage(const SdDrawDocument& rSourceDoc, const SdPage& rPage)
{
    const OUString& rSourceGuid = rPage.GetSourcePageGuid();
    const OUString& rSourceName = rPage.GetBookmarkName();
    const sal_uInt16 nCount = rSourceDoc.GetSdPageCount(PageKind::Standard);

    if (!rSourceGuid.isEmpty())
    {
        for (sal_uInt16 nIndex = 0; nIndex < nCount; ++nIndex)
        {
            const SdPage* pSourcePage = rSourceDoc.GetSdPage(nIndex, PageKind::Standard);
            if (pSourcePage && pSourcePage->GetGuid().getOUString() == rSourceGuid)
                return pSourcePage;
        }
    }

    // A page linked to the position of a slide reads the slide standing there, whatever that
    // slide is named now.
    const sal_Int32 nPosition = SlideLink::GetNamedPosition(rSourceName);
    if (nPosition > 0)
        return nPosition <= nCount ? rSourceDoc.GetSdPage(static_cast<sal_uInt16>(nPosition - 1),
                                                          PageKind::Standard)
                                   : nullptr;

    if (!rSourceName.isEmpty())
    {
        for (sal_uInt16 nIndex = 0; nIndex < nCount; ++nIndex)
        {
            const SdPage* pSourcePage = rSourceDoc.GetSdPage(nIndex, PageKind::Standard);
            if (pSourcePage && pSourcePage->GetName() == rSourceName)
                return pSourcePage;
        }
    }

    return nullptr;
}

/// The undo manager of rDoc, or nothing for a document that is served without one.
SfxUndoManager* getUndoManager(const SdDrawDocument& rDoc)
{
    ::sd::DrawDocShell* pDocShell = rDoc.GetDocSh();
    return pDocShell ? pDocShell->GetUndoManager() : nullptr;
}

/// Tells every view of rDoc that the pages linked to a source document are not what they were.
void notifyLinksChanged(const SdDrawDocument& rDoc)
{
    const SfxObjectShell* pDocShell = rDoc.GetDocSh();
    if (!pDocShell)
        return;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst(false);
    while (pViewShell)
    {
        if (pViewShell->GetObjectShell() == pDocShell)
            pViewShell->viewCallback(COKitCallbackType::SLIDE_LINKS_CHANGED, OString());
        pViewShell = SfxViewShell::GetNext(*pViewShell, false);
    }
}

/// Puts the source document and the source page of one page as they are given, link and all.
void setPageLink(SdPage& rPage, const OUString& rReference, const OUString& rSourcePage)
{
    rPage.DisconnectLink();
    rPage.SetFileName(rReference);
    rPage.SetBookmarkName(rSourcePage);
    rPage.ConnectLink();
}

/// Gives a page back the source document it recorded before that source was taken off.
class UndoSlideLinkBreak final : public SdUndoAction
{
public:
    UndoSlideLinkBreak(SdDrawDocument& rDoc, SdPage& rPage)
        : SdUndoAction(rDoc)
        , mpPage(&rPage)
        , maReference(rPage.GetFileName())
        , maSourcePage(rPage.GetBookmarkName())
    {
        SetComment(SdResId(STR_UNDO_BREAK_SLIDE_LINK));
    }

    virtual void Undo() override
    {
        setPageLink(*mpPage, maReference, maSourcePage);
        notifyLinksChanged(mrDoc);
    }

    virtual void Redo() override
    {
        setPageLink(*mpPage, OUString(), OUString());
        notifyLinksChanged(mrDoc);
    }

private:
    SdPage* mpPage;
    OUString maReference;
    OUString maSourcePage;
};

/// Takes the source document off rPage, recording an undo action for it. Reports whether it did.
bool breakPageLink(SdDrawDocument& rDoc, SdPage& rPage)
{
    // The page names a source document, which is the reference this feature writes and the one a
    // refresh reads. A page linked to a file by its path belongs to the older linked-page feature
    // and keeps what it records.
    if (SlideLink::GetSourceName(rPage.GetFileName()).isEmpty())
        return false;

    // The action reads the source off the page, so it is made while the page still records one.
    std::unique_ptr<SfxUndoAction> pUndoAction = std::make_unique<UndoSlideLinkBreak>(rDoc, rPage);

    setPageLink(rPage, OUString(), OUString());

    if (SfxUndoManager* pUndoManager = getUndoManager(rDoc))
        pUndoManager->AddUndoAction(std::move(pUndoAction));

    rDoc.SetChanged();
    notifyLinksChanged(rDoc);
    return true;
}
}

OUString SlideLink::MakeSourceReference(const OUString& rSourceName)
{
    const OUString aEscaped = rtl::Uri::encode(rSourceName, rtl_UriCharClassPchar,
                                               rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8);
    return gSourceScheme + rtl::Uri::decode(aEscaped, rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8);
}

OUString SlideLink::OriginReference() { return gOriginReference; }

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

OUString SlideLink::GetRefreshedSlideName(const SdPage& rPage, const OUString& rReadName)
{
    const OUString& rRecordedName = rPage.GetBookmarkName();
    if (rRecordedName.isEmpty())
        return OUString();

    if (rPage.GetSourcePageGuid().isEmpty() && GetNamedPosition(rRecordedName) > 0)
        return rRecordedName;

    return rReadName;
}

OUString SlideLink::GetSourceSlideName(const SdDrawDocument& rSourceDoc, const SdPage& rPage)
{
    const SdPage* pSourcePage = findSourcePage(rSourceDoc, rPage);
    return pSourcePage ? pSourcePage->GetName() : OUString();
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
        if (!pPage || !recordsSourceSlide(*pPage)
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
            rJsonWriter.put("sourceGuid", pPage->GetSourcePageGuid());
            rJsonWriter.put("lastModifiedTime", pPage->GetSourceModifiedTime());
        }
    }
}

sal_Int32 SlideLink::Refresh(SdDrawDocument& rDoc, const OUString& rSourceName,
                             const OUString& rFileUrl, const OUString& rLastModifiedTime,
                             std::vector<OString>* pNotUpdated)
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

    std::vector<sal_uInt16> aPages;
    std::vector<OUString> aSourceNames;
    std::vector<OUString> aSourceGuids;
    std::vector<OUString> aRecordedNames;
    for (sal_uInt16 nIndex : aLinkedPages)
    {
        const SdPage* pPage = rDoc.GetSdPage(nIndex, PageKind::Standard);
        if (!pPage)
            continue;

        const OUString aSourceName = GetSourceSlideName(*pSourceDoc, *pPage);
        if (aSourceName.isEmpty())
        {
            SAL_WARN("sd", "slide link refresh: the file holds no slide "
                               << pPage->GetSourcePageGuid() << " and none named "
                               << pPage->GetBookmarkName());
            if (pNotUpdated)
                pNotUpdated->push_back(pPage->GetGuid().getString());
            continue;
        }

        aPages.push_back(nIndex);
        aSourceNames.push_back(aSourceName);
        aSourceGuids.push_back(pPage->GetSourcePageGuid());
        aRecordedNames.push_back(GetRefreshedSlideName(*pPage, aSourceName));
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

            aSourcePages.push_back(aSourceNames[nPos]);
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
        // whether that page was refreshed. A refreshed page records the slide it was read from and
        // the time its source was last modified now, so that its content, the slide it names and
        // that time all agree again.
        for (size_t nPos = 0; nPos < aPageIds.size(); ++nPos)
        {
            SdPage* pPage = rDoc.GetSdPage(aPages[nFirst + nPos], PageKind::Standard);
            if (pPage && pPage->GetGuid().getString() != aPageIds[nPos])
            {
                pPage->SetBookmarkName(aRecordedNames[nFirst + nPos]);
                pPage->SetSourcePageGuid(aSourceGuids[nFirst + nPos]);
                pPage->SetSourceModifiedTime(rLastModifiedTime);
                ++nRefreshed;
            }
        }

        nFirst = nPast;
    }

    rDoc.endUndoAction(/*bUndo=*/false, pUndoManager);
    rDoc.CloseBookmarkDoc();

    return nRefreshed;
}

bool SlideLink::Break(SdDrawDocument& rDoc, sal_Int32 nIndex)
{
    if (nIndex < 0 || nIndex >= rDoc.GetSdPageCount(PageKind::Standard))
        return false;

    SdPage* pPage = rDoc.GetSdPage(static_cast<sal_uInt16>(nIndex), PageKind::Standard);
    if (!pPage)
        return false;

    return breakPageLink(rDoc, *pPage);
}

void SlideLink::BreakOnEdit(SdDrawDocument& rDoc, sal_Int32 nIndex)
{
    if (rDoc.ArePageLinksKept())
        return;

    SfxUndoManager* pUndoManager = getUndoManager(rDoc);
    if (!pUndoManager)
        return;

    // The source comes off as part of the edit that changed the page, so there has to be an undo
    // step in hand to record that in. A change outside a step is the engine's own work on the
    // page: an undo or a redo putting it back the way it was, a document being read, a linked
    // graphic given fresh content, an embedded object reporting the size it wants.
    if (!pUndoManager->IsUndoEnabled() || !pUndoManager->IsInListAction())
        return;

    Break(rDoc, nIndex);
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
