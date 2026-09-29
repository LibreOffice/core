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

#include <vector>

#include <config_features.h>
#include <wrtsh.hxx>
#include <pam.hxx>
#include <node.hxx>
#include <swtable.hxx>
#include <tblsel.hxx>
#include <ndtxt.hxx>
#include <swcrsr.hxx>
#include <translatehelper.hxx>
#include <o3tl/string_view.hxx>
#include <sal/log.hxx>
#include <rtl/string.h>
#include <shellio.hxx>
#include <vcl/svapp.hxx>
#include <vcl/htmltransferable.hxx>
#include <vcl/transfer.hxx>
#include <swdtflvr.hxx>
#include <linguistic/translate.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <officecfg/Office/Linguistic.hxx>
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
        aResult = aResult.replaceAll("<p"_ostr, "<span"_ostr);
        aResult = aResult.replaceAll("</p>"_ostr, "</span>"_ostr);

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
}

bool TranslateRanges(SwWrtShell& rWrtSh,
                     const std::function<OString(const OString&)>& rTranslate,
                     const bool& rCancelTranslation)
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
        const SwNodeOffset nEndNode = rRange.aEnd.nNode.GetIndex();
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
                const auto aTranslatedOut = rTranslate(aOut);
                if (!aTranslatedOut.isEmpty())
                {
                    SwTranslateHelper::PasteHTMLToPaM(rWrtSh, cursor.get(), aTranslatedOut);
                }
                else
                {
                    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                        nullptr, VclMessageType::Error, VclButtonsType::Ok,
                        SwResId(STR_SWTRANSLATE_ERROR)));
                    xBox->run();
                    bStop = true;
                    break;
                }

                if (xStatusIndicator.is() && nCount)
                    xStatusIndicator->setValue((100 * ++nProgress) / nCount);

                Idle aIdle("TranslateDocumentCancellable aIdle");
                aIdle.SetPriority(TaskPriority::POST_PAINT);
                aIdle.Start();

                rWrtSh.LockView(true);
                while (aIdle.IsActive() && !Application::IsQuit())
                {
                    Application::Yield();
                }
                rWrtSh.LockView(false);
            }
        }
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return true;
}

#if HAVE_FEATURE_CURL
void TranslateDocument(SwWrtShell& rWrtSh, const OString& rTargetLang)
{
    bool bCancel = false;
    TranslateDocumentCancellable(rWrtSh, rTargetLang, bCancel);
}

static bool IsTranslationServiceConfigured(OString* pAPIUrl, OString* pKey)
{
    auto oDeeplAPIUrl = officecfg::Office::Linguistic::Translation::Deepl::ApiURL::get();
    auto oDeeplKey = officecfg::Office::Linguistic::Translation::Deepl::AuthKey::get();
    auto sApiUrlTrimmed = oDeeplAPIUrl ? o3tl::trim(*oDeeplAPIUrl) : std::u16string_view();
    auto sKeyTrimmed = oDeeplKey ? o3tl::trim(*oDeeplKey) : std::u16string_view();
    if (sApiUrlTrimmed.empty() || sKeyTrimmed.empty())
        return false;
    if (pAPIUrl)
        *pAPIUrl = OUStringToOString(sApiUrlTrimmed, RTL_TEXTENCODING_UTF8) + "?tag_handling=html";
    if (pKey)
        *pKey = OUStringToOString(sKeyTrimmed, RTL_TEXTENCODING_UTF8);
    return true;
}

bool IsTranslationServiceConfigured() { return IsTranslationServiceConfigured(nullptr, nullptr); }

bool TranslateDocumentCancellable(SwWrtShell& rWrtSh, const OString& rTargetLang,
                                  const bool& rCancelTranslation)
{
    OString aAPIUrl, aAuthKey;
    if (!IsTranslationServiceConfigured(&aAPIUrl, &aAuthKey))
    {
        SAL_WARN("sw.ui", "TranslateDocumentCancellable: API options are not set");
        return false;
    }

    return TranslateRanges(rWrtSh,
                           [&rTargetLang, &aAPIUrl, &aAuthKey](const OString& rData)
                           { return linguistic::Translate(rTargetLang, aAPIUrl, aAuthKey, rData); },
                           rCancelTranslation);
}
#endif // HAVE_FEATURE_CURL
}
