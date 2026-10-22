/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include <wrtsh.hxx>
#include <view.hxx>
#include <pam.hxx>
#include <node.hxx>
#include <swtable.hxx>
#include <tblsel.hxx>
#include <ndtxt.hxx>
#include <swatrset.hxx>
#include <swcrsr.hxx>
#include <doc.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentUndoRedo.hxx>
#include <hintids.hxx>
#include <translatehelper.hxx>
#include <comphelper/scopeguard.hxx>
#include <editeng/langitem.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <sal/log.hxx>
#include <svl/itemiter.hxx>
#include <svl/itemset.hxx>
#include <svl/languageoptions.hxx>
#include <rtl/string.h>
#include <shellio.hxx>
#include <vcl/kit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/htmltransferable.hxx>
#include <vcl/transfer.hxx>
#include <swdtflvr.hxx>
#include <linguistic/translate.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <strings.hrc>

using namespace css;
using namespace ::cpo;

namespace SwTranslateHelper
{
OString ExportPaMToHTML(SwPaM* pCursor)
{
    SolarMutexGuard gMutex;
    OString aResult;
    WriterRef xWrt;
    GetHTMLWriter(u"NoLineLimit,SkipHeaderFooter,SkipHeaderFooterContent,NoPrettyPrint", OUString(), xWrt);
    if (pCursor != nullptr)
    {
        SvMemoryStream aMemoryStream;
        SwWriter aWriter(aMemoryStream, *pCursor);
        ErrCodeMsg nError = aWriter.Write(xWrt);
        if (nError.IsError())
        {
            SAL_WARN("sw.ui", "ExportPaMToHTML: failed to export selection to HTML " << nError);
            return {};
        }
        aResult
            = OString(static_cast<const char*>(aMemoryStream.GetData()), aMemoryStream.GetSize());
        // The blocks of footnotes and endnotes come after the text: they are
        // re-created on paste, and their paragraph tags give them the note's
        // style, so they stay as they are.
        sal_Int32 nNotes = aResult.getLength();
        for (const OString& rBlock : { "<div id=\"sdfootnote"_ostr, "<div id=\"sdendnote"_ostr })
        {
            const sal_Int32 nPos = aResult.indexOf(rBlock);
            if (nPos != -1 && nPos < nNotes)
                nNotes = nPos;
        }
        const OString aNotes = aResult.copy(nNotes);
        aResult = aResult.copy(0, nNotes);
        // The text itself goes in as inline text, so it replaces what the
        // paragraph had rather than splitting the paragraph.
        aResult = aResult.replaceAll("<p"_ostr, "<span"_ostr);
        aResult = aResult.replaceAll("</p>"_ostr, "</span>"_ostr);
        // A heading is a block element too: pasted as one, it would split the
        // paragraph and leave the original heading empty. The paragraph style
        // stays on the node, so the translation can go in as inline text.
        for (int nLevel = 1; nLevel <= 6; ++nLevel)
        {
            const OString aOpen = "<h" + OString::number(nLevel);
            const OString aClose = "</h" + OString::number(nLevel) + ">";
            aResult = aResult.replaceAll(aOpen, "<span"_ostr);
            aResult = aResult.replaceAll(aClose, "</span>"_ostr);
        }
        aResult += aNotes;

        // HTML has for that <br> and <p> also does new line
        aResult = aResult.replaceAll("<ul>"_ostr, ""_ostr);
        aResult = aResult.replaceAll("</ul>"_ostr, ""_ostr);
        aResult = aResult.replaceAll("<ol>"_ostr, ""_ostr);
        aResult = aResult.replaceAll("</ol>"_ostr, ""_ostr);
        aResult = aResult.replaceAll("\n"_ostr, ""_ostr).trim();
        return aResult;
    }
    return {};
}

void PasteHTMLToPaM(SwWrtShell& rWrtSh, const SwPaM* pCursor, const OString& rData)
{
    SolarMutexGuard gMutex;
    rtl::Reference<vcl::unohelper::HtmlTransferable> pHtmlTransferable
        = new vcl::unohelper::HtmlTransferable(rData);
    if (pHtmlTransferable.is())
    {
        TransferableDataHelper aDataHelper(pHtmlTransferable);
        if (aDataHelper.GetXTransferable().is()
            && SwTransferable::IsPasteSpecial(rWrtSh, aDataHelper))
        {
            // Pasting with a table box selection active would not delete the
            // cell content (PasteData skips it in table mode) and would insert
            // the data into every selected box instead of replacing the given
            // range: collapse it first.
            if (rWrtSh.IsTableMode())
                rWrtSh.ClearMark();
            rWrtSh.SetSelection(*pCursor);
            SwTransferable::Paste(rWrtSh, aDataHelper);
            rWrtSh.KillSelection(nullptr, false);
        }
    }
}

BulkUpdateGuard::BulkUpdateGuard(SwWrtShell& rWrtSh, SwUndoId eUndoId)
    : m_rWrtSh(rWrtSh)
    , m_eUndoId(eUndoId)
{
    m_rWrtSh.GetDoc()->GetIDocumentUndoRedo().StartUndo(m_eUndoId, nullptr);
    m_rWrtSh.StartAction();
    m_rWrtSh.Push();
}

BulkUpdateGuard::~BulkUpdateGuard()
{
    m_rWrtSh.Pop(SwCursorShell::PopMode::DeleteCurrent);
    m_rWrtSh.EndAction();
    m_rWrtSh.GetDoc()->GetIDocumentUndoRedo().EndUndo(m_eUndoId, nullptr);
}

void GetTranslationNodeRange(SwWrtShell& rWrtSh, SwNodeOffset& rStartNode, SwNodeOffset& rEndNode)
{
    auto const& rNodes = rWrtSh.GetNodes();
    // The node array also holds the header/footer/footnote sections, ahead of the body.
    // Clamp to the body so a document-wide translate, or a selection that a caller failed
    // to normalize into the body, never walks into and overwrites that other content.
    const SwNodeOffset nBodyStart = rNodes.GetEndOfExtras().GetIndex();
    const SwNodeOffset nBodyEnd = rNodes.GetEndOfContent().GetIndex();

    bool bHasSelection = rWrtSh.HasSelection();
    if (!bHasSelection)
    {
        rStartNode = nBodyStart;
        rEndNode = nBodyEnd;
        return;
    }

    auto pCurrentPam = rWrtSh.GetCursor();
    // iteration will start top to bottom
    pCurrentPam->Normalize();
    SwPosition aPoint = *pCurrentPam->GetPoint();
    SwPosition aMark = *pCurrentPam->GetMark();
    rStartNode = std::max(aPoint.nNode.GetIndex(), nBodyStart);
    rEndNode = std::min(aMark.nNode.GetIndex(), nBodyEnd);
}

namespace
{
/// A range of the document to translate: the content between aStart and aEnd.
/// The positions are registered, so they keep pointing at the same content
/// even if translating an earlier range inserts or deletes nodes.
struct TranslateRange
{
    SwPosition aStart;
    SwPosition aEnd;
};

/// Sets eLanguage on the text from nStartContent in node nStartNode up to nCharsAfter
/// characters before the end of node nEndNode.
void SetLanguageOfRange(SwWrtShell& rWrtSh, SwNodeOffset nStartNode, sal_Int32 nStartContent,
                        SwNodeOffset nEndNode, sal_Int32 nCharsAfter, LanguageType eLanguage)
{
    const SwNodes& rNodes = rWrtSh.GetNodes();
    if (nEndNode < nStartNode || nEndNode >= rNodes.Count())
        return;
    SwTextNode* pStartNode = rNodes[nStartNode]->GetTextNode();
    SwTextNode* pEndNode = rNodes[nEndNode]->GetTextNode();
    if (!pStartNode || !pEndNode || nStartContent > pStartNode->Len())
        return;
    const sal_Int32 nEndContent = pEndNode->Len() - nCharsAfter;
    if (nEndContent < 0 || (nStartNode == nEndNode && nEndContent <= nStartContent))
        return;

    // The attribute of the language's script: a Western, Asian or Complex one.
    const TypedWhichId<SvxLanguageItem> nWhich = GetWhichOfScript(
        RES_CHRATR_LANGUAGE, SvtLanguageOptions::GetI18NScriptTypeOfLanguage(eLanguage));
    const SwPaM aPaM(*pStartNode, nStartContent, *pEndNode, nEndContent);
    rWrtSh.GetDoc()->getIDocumentContentOperations().InsertPoolItem(
        aPaM, SvxLanguageItem(eLanguage, nWhich));
}
}

bool TranslateRanges(SwWrtShell& rWrtSh, const TranslateFunc& rTranslate,
                     const bool& rCancelTranslation, LanguageType eTargetLanguage)
{
    SwCursor* pCurrentPam = rWrtSh.GetCursor();
    const bool bTableMode = rWrtSh.IsTableMode();
    const bool bHasSelection = rWrtSh.HasSelection();

    if (bHasSelection)
    {
        // iteration will start top to bottom
        pCurrentPam->Normalize();
    }

    // Collect the ranges to translate.
    std::vector<TranslateRange> aRanges;
    if (bTableMode)
    {
        // A table box selection is a list of selected boxes on the table
        // cursor. Looking only at the current PaM would translate the first
        // selected box only (and pasting would then insert that single result
        // into every selected box), so expand each box to the range of its
        // content and translate the boxes separately. The selection lives on
        // the table cursor (model side), so this does not depend on layout.
        for (const SwTableBox* pBox : rWrtSh.GetTableCursor()->GetSelectedBoxes())
        {
            const SwNode* pSttNd = pBox->GetSttNd();
            if (!pSttNd)
                continue;

            // First content node inside the box: the node after the box start
            // node; GoNextSection skips over nested section starts.
            SwNodeIndex aSttIdx(*pSttNd, 1);
            SwContentNode* pContent
                = aSttIdx.GetNode().IsContentNode()
                      ? aSttIdx.GetNode().GetContentNode()
                      : SwNodes::GoNextSection(&aSttIdx, true, false);
            if (!pContent)
                continue;

            // Last content node inside the box: the node before the box end
            // node; GoPrevSection skips over nested section ends.
            SwNodeIndex aEndIdx(*pSttNd->EndOfSectionNode(), -1);
            SwContentNode* pLast
                = aEndIdx.GetNode().IsContentNode()
                      ? aEndIdx.GetNode().GetContentNode()
                      : SwNodes::GoPrevSection(&aEndIdx, true, false);
            if (!pLast || pLast->GetIndex() < pContent->GetIndex())
                pLast = pContent;

            SwPosition aStart(*pContent, 0);
            SwPosition aEnd(*pLast, pLast->Len());

            aRanges.push_back({ aStart, aEnd });
        }
        // Leave table mode, so that pasting the translation replaces the cell
        // content instead of inserting it into every selected box.
        rWrtSh.ClearMark();
    }
    else if (bHasSelection)
    {
        auto [pStart, pEnd] = pCurrentPam->StartEnd(); // SwPosition*
        aRanges.push_back({ *pStart, *pEnd });
    }
    else
    {
        // Use the body-only range so a document-wide translate never walks into
        // the header/footer/footnote sections (cool#6098): the node array also
        // holds those ahead of the body.
        SwNodeOffset nStartNode, nEndNode;
        GetTranslationNodeRange(rWrtSh, nStartNode, nEndNode);
        SwNodes& rNodes = rWrtSh.GetDoc()->GetNodes();
        const SwNodeIndex aStartIdx(rNodes, nStartNode);
        const SwNodeIndex aEndIdx(rNodes, nEndNode);
        aRanges.push_back({ SwPosition(aStartIdx), SwPosition(aEndIdx) });
        // Note: can't use SwPosition(SwNodeIndex(...)) — the rvalue overload
        // of SwPosition's ctor (SwPosition(SwNodeIndex&&)) is deleted.
    }

    auto const& pNodes = rWrtSh.GetNodes();

    sal_Int32 nCount(0);
    sal_Int32 nProgress(0);

    for (const TranslateRange& rRange : aRanges)
    {
        for (SwNodeOffset n(rRange.aStart.nNode.GetIndex());
             n <= rRange.aEnd.nNode.GetIndex(); ++n)
        {
            if (pNodes[n] && pNodes[n]->IsTextNode())
            {
                if (pNodes[n]->GetTextNode()->GetText().isEmpty())
                    continue;
                nCount++;
            }
        }
    }

    SfxViewFrame* pFrame = SfxViewFrame::Current();
    uno::Reference<frame::XFrame> xFrame(pFrame ? pFrame->GetFrame().GetFrameInterface() : nullptr);
    uno::Reference<task::XStatusIndicatorFactory> xProgressFactory(xFrame, uno::UNO_QUERY);
    uno::Reference<task::XStatusIndicator> xStatusIndicator;

    if (xProgressFactory.is())
    {
        xStatusIndicator = xProgressFactory->createStatusIndicator();
    }

    if (xStatusIndicator.is())
        xStatusIndicator->start(SwResId(STR_STATSTR_SWTRANSLATE), nCount);

    bool bStop = false;
    for (const TranslateRange& rRange : aRanges)
    {
        const SwNodeOffset nStartNode = rRange.aStart.nNode.GetIndex();
        // The end position is registered, so it follows the content when a
        // pasted translation changes the number of nodes before it.
        SwNodeOffset nEndNode = rRange.aEnd.nNode.GetIndex();
        for (SwNodeOffset n(nStartNode); !bStop && n <= nEndNode; ++n)
        {
            if (rCancelTranslation)
                break;

            if (n >= rWrtSh.GetNodes().Count())
                break;

            if (!pNodes[n])
                break;

            SwNode* pNode = pNodes[n];
            if (pNode->IsTextNode())
            {
                if (pNode->GetTextNode()->GetText().isEmpty())
                    continue;
                auto cursor
                    = Writer::NewUnoCursor(*rWrtSh.GetDoc(), pNode->GetIndex(), pNode->GetIndex());

                // set edges (start, end) for nodes inside the selection.
                if (bHasSelection)
                {
                    if (nStartNode == nEndNode)
                    {
                        cursor->SetMark();
                        cursor->GetPoint()->nContent = rRange.aStart.nContent;
                        cursor->GetMark()->nContent = rRange.aEnd.nContent;
                    }
                    else if (n == nStartNode)
                    {
                        cursor->SetMark();
                        cursor->GetPoint()->nContent = rRange.aStart.nContent;
                    }
                    else if (n == nEndNode)
                    {
                        cursor->SetMark();
                        cursor->GetMark()->nContent = rRange.aEnd.nContent;
                        cursor->GetPoint()->nContent = 0;
                    }
                }

                const auto aOut = SwTranslateHelper::ExportPaMToHTML(cursor.get());
                OString aError;
                const auto aTranslatedOut = rTranslate(aOut, aError);
                if (!aTranslatedOut.isEmpty())
                {
                    // The translation replaces the range and the text after the range stays,
                    // so the end of the translation is found by counting back from there:
                    // the paste can change the number of paragraphs.
                    const sal_Int32 nStartContent = cursor->Start()->GetContentIndex();
                    const SwNodeOffset nNodesAfter = pNodes.Count() - n;
                    const sal_Int32 nCharsAfter
                        = pNode->GetTextNode()->Len() - cursor->End()->GetContentIndex();
                    SwTextFormatColl* pColl = pNode->GetTextNode()->GetTextColl();
                    std::optional<SwAttrSet> oNodeAttrs;
                    if (const SwAttrSet* pNodeAttrs = pNode->GetTextNode()->GetpSwAttrSet())
                        oNodeAttrs.emplace(*pNodeAttrs);
                    // The paste re-creates the paragraph's footnotes, whose
                    // sections sit ahead of the body: the paragraph's index
                    // moves by as many nodes as those sections grew or shrank.
                    const SwNodeOffset nExtrasBefore = pNodes.GetEndOfExtras().GetIndex();
                    // One undo step for the paragraph: its translation and its language.
                    rWrtSh.StartUndo(SwUndoId::PASTE_CLIPBOARD);
                    SwTranslateHelper::PasteHTMLToPaM(rWrtSh, cursor.get(), aTranslatedOut);
                    const SwNodeOffset nFirstPasted
                        = n + (pNodes.GetEndOfExtras().GetIndex() - nExtrasBefore);
                    const SwNodeOffset nLastPasted = pNodes.Count() - nNodesAfter;
                    // The HTML import treats text without a block element as
                    // a paragraph of its own: it sets its default paragraph
                    // style, and what the markup says about the paragraph, its
                    // spacing, alignment, font and list, as hard attributes.
                    // Put the paragraph's own style and attributes back.
                    SwDoc& rDoc = *rWrtSh.GetDoc();
                    for (SwNodeOffset i = nFirstPasted; i <= nLastPasted; ++i)
                    {
                        SwTextNode* pPasted = pNodes[i]->GetTextNode();
                        if (!pPasted)
                            continue;
                        const SwPaM aPasted(*pPasted, 0, *pPasted, pPasted->Len());
                        if (pPasted->GetTextColl() != pColl)
                            rDoc.SetTextFormatColl(aPasted, pColl, false);
                        // Character formatting that covers the whole paragraph
                        // ends up on the paragraph as well. Where it says more
                        // than the paragraph style does, it is the translation's
                        // own, bold text for instance, and stays.
                        SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END - 1> aOwnCharAttrs(
                            rDoc.GetAttrPool());
                        if (const SwAttrSet* pPastedAttrs = pPasted->GetpSwAttrSet())
                        {
                            for (SfxItemIter aIter(*pPastedAttrs); !aIter.IsAtEnd(); aIter.Next())
                            {
                                const SfxPoolItem* pItem = aIter.GetCurItem();
                                const sal_uInt16 nWhich = pItem->Which();
                                if (nWhich < RES_CHRATR_BEGIN || nWhich >= RES_CHRATR_END)
                                    continue;
                                if (!(pColl->GetAttrSet().Get(nWhich) == *pItem))
                                    aOwnCharAttrs.Put(*pItem);
                            }
                            if (pPastedAttrs->Count())
                                rDoc.ResetAttrs(aPasted, false, {});
                        }
                        if (oNodeAttrs && oNodeAttrs->Count())
                            rDoc.getIDocumentContentOperations().InsertItemSet(aPasted, *oNodeAttrs);
                        if (aOwnCharAttrs.Count())
                            rDoc.getIDocumentContentOperations().InsertItemSet(aPasted, aOwnCharAttrs);
                    }
                    if (eTargetLanguage != LANGUAGE_DONTKNOW)
                        SetLanguageOfRange(rWrtSh, nFirstPasted, nStartContent, nLastPasted,
                                           nCharsAfter, eTargetLanguage);
                    rWrtSh.EndUndo(SwUndoId::PASTE_CLIPBOARD);
                    // Continue after the pasted translation, however many
                    // nodes it took.
                    n = nLastPasted;
                    nEndNode = rRange.aEnd.nNode.GetIndex();
                }
                else
                {
                    // Close the progress indicator first: in the LOK case it is a
                    // dialog of its own, which would cover the error.
                    if (xStatusIndicator.is())
                    {
                        xStatusIndicator->end();
                        xStatusIndicator.clear();
                    }
                    // Parent the dialog to the frame: a parentless dialog has no
                    // notifier in the LOK case, so it would never reach the client.
                    std::shared_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                        rWrtSh.GetView().GetFrameWeld(), VclMessageType::Error,
                        VclButtonsType::Ok, SwResId(STR_SWTRANSLATE_ERROR)));
                    if (!aError.isEmpty())
                        xBox->set_secondary_text(OStringToOUString(aError, RTL_TEXTENCODING_UTF8));
                    xBox->runAsync(xBox, [](sal_uInt32) {});
                    bStop = true;
                    break;
                }

                if (xStatusIndicator.is() && nCount)
                    xStatusIndicator->setValue((100 * ++nProgress) / nCount);

                Idle aIdle("TranslateDocumentCancellable aIdle");
                aIdle.SetPriority(TaskPriority::POST_PAINT);
                aIdle.Start();

                rWrtSh.LockView(true);
                {
                    // Spinning the event loop here is deliberate, so the progress is painted
                    // and a click on Cancel is seen: tell the kit host poll loop not to take
                    // this re-entry for a non-async dialog.
                    vcl::kit::pushExpectedReentry();
                    comphelper::ScopeGuard aPopReentry([] { vcl::kit::popExpectedReentry(); });
                    while (aIdle.IsActive() && !Application::IsQuit())
                    {
                        Application::Yield();
                    }
                }
                rWrtSh.LockView(false);
            }
        }
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return !bStop;
}

void TranslateDocument(SwWrtShell& rWrtSh, const OString& rTargetLang)
{
    bool bCancel = false;
    TranslateDocumentCancellable(rWrtSh, rTargetLang, bCancel);
}

bool IsTranslationServiceConfigured() { return linguistic::getActiveTranslationProvider() != nullptr; }

std::vector<OString> GetTargetLanguages()
{
    std::shared_ptr<linguistic::TranslationProvider> pProvider
        = linguistic::getActiveTranslationProvider();
    if (!pProvider)
        return {};
    return pProvider->getTargetLanguages();
}

/// The language of text translated to rTargetLang, a tag like "de" or "en-GB".
static LanguageType GetTargetLanguageType(std::string_view rTargetLang)
{
    const LanguageTag aTag(OStringToOUString(rTargetLang, RTL_TEXTENCODING_ASCII_US));
    // A bare language like "de" stands for its default locale, de-DE. A language without
    // a known locale falls back to en-US instead, so it is taken as it is.
    LanguageTag aFallbackTag(aTag);
    aFallbackTag.makeFallback();
    if (aFallbackTag.getLanguage() != aTag.getLanguage())
        return aTag.getLanguageType();
    const LanguageType eType = aFallbackTag.getLanguageType();
    // zh-Hant has no default locale to fall back to, unlike zh-Hans with zh-CN: it gets the
    // locale the language list shows it under.
    if (eType == LANGUAGE_CHINESE_TRADITIONAL_LSO)
        return LANGUAGE_CHINESE_TRADITIONAL;
    return eType;
}

bool TranslateDocumentCancellable(SwWrtShell& rWrtSh, const OString& rTargetLang,
                                  const bool& rCancelTranslation)
{
    std::shared_ptr<linguistic::TranslationProvider> pProvider
        = linguistic::getActiveTranslationProvider();
    if (!pProvider)
    {
        SAL_WARN("sw.ui", "TranslateDocumentCancellable: no translation provider is set up");
        return false;
    }

    return TranslateRanges(
        rWrtSh,
        [&rTargetLang, &pProvider, &rCancelTranslation](const OString& rData, OString& rError)
        {
            linguistic::TranslateResult aResult
                = pProvider->translate(rData, rTargetLang, rCancelTranslation);
            rError = aResult.aError;
            return aResult.aText;
        },
        rCancelTranslation, GetTargetLanguageType(rTargetLang));
}
}
