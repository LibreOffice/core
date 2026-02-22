/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <config_features.h>
#include <wrtsh.hxx>
#include <pam.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <translatehelper.hxx>
#include <o3tl/string_view.hxx>
#include <sal/log.hxx>
#include <rtl/string.h>
#include <shellio.hxx>
#include <vcl/svapp.hxx>
#include <vcl/htmltransferable.hxx>
#include <vcl/transfer.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <swdtflvr.hxx>
#include <linguistic/translate.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <officecfg/Office/Linguistic.hxx>
#include <strings.hrc>

namespace SwTranslateHelper
{
OString ExportPaMToHTML(SwPaM* pCursor)
{
    SolarMutexGuard gMutex;
    OString aResult;
    WriterRef xWrt;
    GetHTMLWriter(u"NoLineLimit,SkipHeaderFooter,NoPrettyPrint", OUString(), xWrt);
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
            rWrtSh.SetSelection(*pCursor);
            SwTransferable::Paste(rWrtSh, aDataHelper);
            rWrtSh.KillSelection(nullptr, false);
        }
    }
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

    auto m_pCurrentPam = rWrtSh.GetCursor();
    bool bHasSelection = rWrtSh.HasSelection();

    if (bHasSelection)
    {
        // iteration will start top to bottom
        m_pCurrentPam->Normalize();
    }

    auto const& pNodes = rWrtSh.GetNodes();
    SwPosition aPoint = *m_pCurrentPam->GetPoint();
    SwPosition aMark = *m_pCurrentPam->GetMark();
    auto startNode = bHasSelection ? aPoint.nNode.GetIndex() : SwNodeOffset(0);
    auto endNode = bHasSelection ? aMark.nNode.GetIndex() : pNodes.Count() - 1;

    sal_Int32 nCount(0);
    sal_Int32 nProgress(0);

    for (SwNodeOffset n(startNode); n <= endNode; ++n)
    {
        if (pNodes[n] && pNodes[n]->IsTextNode())
        {
            if (pNodes[n]->GetTextNode()->GetText().isEmpty())
                continue;
            nCount++;
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

    for (SwNodeOffset n(startNode); n <= endNode; ++n)
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
                if (startNode == endNode)
                {
                    cursor->SetMark();
                    cursor->GetPoint()->nContent = aPoint.nContent;
                    cursor->GetMark()->nContent = aMark.nContent;
                }
                else if (n == startNode)
                {
                    cursor->SetMark();
                    cursor->GetPoint()->nContent = aPoint.nContent;
                }
                else if (n == endNode)
                {
                    cursor->SetMark();
                    cursor->GetMark()->nContent = aMark.nContent;
                    cursor->GetPoint()->nContent = 0;
                }
            }

            const auto aOut = SwTranslateHelper::ExportPaMToHTML(cursor.get());
            const auto aTranslatedOut = linguistic::Translate(rTargetLang, aAPIUrl, aAuthKey, aOut);
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

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return true;
}
#endif // HAVE_FEATURE_CURL
}
