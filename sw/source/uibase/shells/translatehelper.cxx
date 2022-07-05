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
#include <wrtsh.hxx>
#include <pam.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <translatehelper.hxx>
#include <sal/log.hxx>
#include <rtl/string.h>
#include <shellio.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>
#include <curl/curl.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <vcl/htmltransferable.hxx>
#include <vcl/transfer.hxx>
#include <swdtflvr.hxx>
#include <linguistic/translate.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <strings.hrc>

namespace SwTranslateHelper
{
OString ExportPaMToHTML(SwPaM* pCursor, bool bReplacePTag)
{
    SolarMutexGuard gMutex;
    OString aResult;
    WriterRef xWrt;
    GetHTMLWriter(OUString("NoLineLimit,SkipHeaderFooter"), OUString(), xWrt);
    if (pCursor != nullptr)
    {
        SvMemoryStream aMemoryStream;
        SwWriter aWriter(aMemoryStream, *pCursor);
        ErrCode nError = aWriter.Write(xWrt);
        if (nError.IsError())
        {
            SAL_WARN("translatehelper", "failed to export selection to HTML");
            return {};
        }
        aResult
            = OString(static_cast<const char*>(aMemoryStream.GetData()), aMemoryStream.GetSize());
        if (bReplacePTag)
        {
            aResult = aResult.replaceAll("<p", "<span");
            aResult = aResult.replaceAll("</p>", "</span>");
        }
        return aResult;
    }
    return {};
}

void PasteHTMLToPaM(SwWrtShell& rWrtSh, SwPaM* pCursor, const OString& rData, bool bSetSelection)
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
            if (bSetSelection)
            {
                rWrtSh.SetSelection(*pCursor);
            }
            SwTransferable::Paste(rWrtSh, aDataHelper);
            rWrtSh.KillSelection(nullptr, false);
        }
    }
}

void TranslateDocument(SwWrtShell& rWrtSh, const TranslateAPIConfig& rConfig)
{
    bool bCancel = false;
    TranslateDocumentCancellable(rWrtSh, rConfig, bCancel);
}

void TranslateDocumentCancellable(SwWrtShell& rWrtSh, const TranslateAPIConfig& rConfig,
                                  bool& rCancelTranslation)
{
    auto m_pCurrentPam = rWrtSh.GetCursor();
    bool bHasSelection = rWrtSh.HasSelection();

    if (bHasSelection)
    {
        // iteration will start top to bottom
        if (m_pCurrentPam->GetPoint()->nNode > m_pCurrentPam->GetMark()->nNode)
            m_pCurrentPam->Exchange();
    }

    auto const& pNodes = rWrtSh.GetNodes();
    auto pPoint = SwPosition(*m_pCurrentPam->GetPoint());
    auto pMark = SwPosition(*m_pCurrentPam->GetMark());
    auto startNode = bHasSelection ? pPoint.nNode.GetIndex() : SwNodeOffset(0);
    auto endNode = bHasSelection ? pMark.nNode.GetIndex() : pNodes.Count() - 1;

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
    uno::Reference<frame::XFrame> xFrame = pFrame->GetFrame().GetFrameInterface();
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
                    cursor->GetPoint()->nContent = pPoint.nContent;
                    cursor->GetMark()->nContent = pMark.nContent;
                }
                else if (n == startNode)
                {
                    cursor->SetMark();
                    cursor->GetPoint()->nContent = std::min(pPoint.nContent, pMark.nContent);
                }
                else if (n == endNode)
                {
                    cursor->SetMark();
                    cursor->GetMark()->nContent = pMark.nContent;
                    cursor->GetPoint()->nContent = 0;
                }
            }

            const auto aOut = SwTranslateHelper::ExportPaMToHTML(cursor.get(), true);
            const auto aTranslatedOut = linguistic::Translate(
                rConfig.m_xTargetLanguage, rConfig.m_xAPIUrl, rConfig.m_xAuthKey, aOut);
            SwTranslateHelper::PasteHTMLToPaM(rWrtSh, cursor.get(), aTranslatedOut, true);

            if (xStatusIndicator.is())
                xStatusIndicator->setValue((100 * ++nProgress) / nCount);

            Idle aIdle("ProgressBar::SetValue aIdle");
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
}
}