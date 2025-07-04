/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <editeng/postitem.hxx>
#include <IDocumentContentOperations.hxx>

#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <fmtinfmt.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <fmtanchr.hxx>
#include <tools/urlobj.hxx>

#include "swmd.hxx"

int SwMarkdownParser::enter_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata)
{
    SwMarkdownParser* parser = static_cast<SwMarkdownParser*>(userdata);
    switch (type)
    {
        case MD_BLOCK_DOC:
            break;
        case MD_BLOCK_QUOTE:
        {
            parser->AddBlockQuote();
            break;
        }
        case MD_BLOCK_UL:
        {
            parser->StartNumberedBulletList(type);
            break;
        }
        case MD_BLOCK_OL:
        {
            parser->StartNumberedBulletList(type);
            break;
        }
        case MD_BLOCK_LI:
        {
            parser->StartNumberedBulletListItem();
            break;
        }
        case MD_BLOCK_HR:
        {
            parser->AddHR();
            break;
        }
        case MD_BLOCK_H:
        {
            const MD_BLOCK_H_DETAIL* pDetail = static_cast<const MD_BLOCK_H_DETAIL*>(detail);
            parser->StartHeading(pDetail->level);
            break;
        }
        case MD_BLOCK_CODE:
            parser->BeginCodeBlock();
            break;
        case MD_BLOCK_HTML:
            parser->BeginHtmlBlock();
            break;
        case MD_BLOCK_P:
        {
            parser->StartPara();
            break;
        }
        case MD_BLOCK_TABLE:
        {
            const MD_BLOCK_TABLE_DETAIL* pDetail
                = static_cast<const MD_BLOCK_TABLE_DETAIL*>(detail);
            const sal_Int32 nRow = pDetail->body_row_count + pDetail->head_row_count;
            const sal_Int32 nCol = pDetail->col_count;
            parser->StartTable(nRow, nCol);
            break;
        }
        case MD_BLOCK_THEAD:
        case MD_BLOCK_TBODY:
            break;
        case MD_BLOCK_TR:
            parser->StartRow();
            break;
        case MD_BLOCK_TH:
        case MD_BLOCK_TD:
            const MD_BLOCK_TD_DETAIL* pDetail = static_cast<const MD_BLOCK_TD_DETAIL*>(detail);
            parser->StartCell(pDetail->align);
            break;
    }

    return 0;
}

int SwMarkdownParser::leave_block_callback(MD_BLOCKTYPE type, void* /*detail*/, void* userdata)
{
    SwMarkdownParser* parser = static_cast<SwMarkdownParser*>(userdata);

    switch (type)
    {
        case MD_BLOCK_DOC:
            break;
        case MD_BLOCK_QUOTE:
            parser->EndBlockQuote();
            break;
        case MD_BLOCK_UL:
            parser->EndNumberedBulletList();
            break;
        case MD_BLOCK_OL:
            parser->EndNumberedBulletList();
            break;
        case MD_BLOCK_LI:
            parser->EndNumberedBulletListItem();
            break;
        case MD_BLOCK_HR:
        {
            parser->EndHR();
            break;
        }
        case MD_BLOCK_H:
        {
            parser->EndHeading();
            break;
        }
        case MD_BLOCK_CODE:
            parser->EndCodeBlock();
            break;
        case MD_BLOCK_HTML:
        {
            parser->EndHtmlBlock();
            break;
        }
        case MD_BLOCK_P:
        {
            parser->EndPara();
            break;
        }
        case MD_BLOCK_TABLE:
            parser->EndTable();
            break;
        case MD_BLOCK_THEAD:
        case MD_BLOCK_TBODY:
            break;
        case MD_BLOCK_TR:
        case MD_BLOCK_TH:
            break;
        case MD_BLOCK_TD:
            break;
    }
    return 0;
}

int SwMarkdownParser::enter_span_callback(MD_SPANTYPE type, void* detail, void* userdata)
{
    SwMarkdownParser* parser = static_cast<SwMarkdownParser*>(userdata);
    std::unique_ptr<SfxPoolItem> pItem;
    switch (type)
    {
        case MD_SPAN_EM:
            pItem.reset(new SvxPostureItem(ITALIC_NORMAL, RES_CHRATR_POSTURE));
            break;
        case MD_SPAN_STRONG:
            pItem.reset(new SvxWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT));
            break;
        case MD_SPAN_A:
        {
            const MD_SPAN_A_DETAIL* pHyperlinkDetail = static_cast<const MD_SPAN_A_DETAIL*>(detail);
            const MD_ATTRIBUTE& rHref = pHyperlinkDetail->href;
            OUString aURL = rtl::OStringToOUString(std::string_view(rHref.text, rHref.size),
                                                   RTL_TEXTENCODING_UTF8);
            aURL = INetURLObject::GetAbsURL(parser->m_sBaseURL, aURL);
            pItem.reset(new SwFormatINetFormat(aURL, OUString()));
            break;
        }
        case MD_SPAN_IMG:
        {
            const MD_SPAN_IMG_DETAIL* pImgDetail = static_cast<const MD_SPAN_IMG_DETAIL*>(detail);
            const MD_ATTRIBUTE& rSrc = pImgDetail->src;
            const MD_ATTRIBUTE& rTitle = pImgDetail->title;
            OUString aURL = rtl::OStringToOUString(std::string_view(rSrc.text, rSrc.size),
                                                   RTL_TEXTENCODING_UTF8);
            OUString aTitle = rtl::OStringToOUString(std::string_view(rTitle.text, rTitle.size),
                                                     RTL_TEXTENCODING_UTF8);
            parser->InsertImage(aURL, aTitle);
            parser->m_bInsideImage = true;
            break;
        }
        case MD_SPAN_CODE:
            break;
        case MD_SPAN_DEL:
            pItem.reset(new SvxCrossedOutItem(STRIKEOUT_SINGLE, RES_CHRATR_CROSSEDOUT));
            break;
        case MD_SPAN_LATEXMATH:
            break;
        case MD_SPAN_LATEXMATH_DISPLAY:
            break;
        case MD_SPAN_WIKILINK:
        {
            const MD_SPAN_WIKILINK_DETAIL* pWikilink
                = static_cast<const MD_SPAN_WIKILINK_DETAIL*>(detail);
            OUString target = rtl::OStringToOUString(
                std::string_view(pWikilink->target.text, pWikilink->target.size),
                RTL_TEXTENCODING_UTF8);
            OUString aLang = GetAppLanguageTag().getLanguage();
            OUString aURL = "https://" + aLang + ".wikipedia.org/wiki/" + target;
            pItem.reset(new SwFormatINetFormat(aURL, OUString()));
            break;
        }
        case MD_SPAN_U:
            pItem.reset(new SvxUnderlineItem(LINESTYLE_SINGLE, RES_CHRATR_CROSSEDOUT));
            break;
    }
    parser->m_aAttrStack.push_back(std::move(pItem));
    return 0;
}

int SwMarkdownParser::leave_span_callback(MD_SPANTYPE type, void* /*detail*/, void* userdata)
{
    SwMarkdownParser* parser = static_cast<SwMarkdownParser*>(userdata);
    parser->m_aAttrStack.pop_back();
    switch (type)
    {
        case MD_SPAN_EM:
            break;
        case MD_SPAN_STRONG:
            break;
        case MD_SPAN_A:
        {
            break;
        }
        case MD_SPAN_IMG:
            parser->m_bInsideImage = false;
            break;
        case MD_SPAN_CODE:
            break;
        case MD_SPAN_DEL:
            break;
        case MD_SPAN_LATEXMATH:
            break;
        case MD_SPAN_LATEXMATH_DISPLAY:
            break;
        case MD_SPAN_WIKILINK:
            break;
        case MD_SPAN_U:
            break;
    }
    return 0;
}

int SwMarkdownParser::text_callback(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size,
                                    void* userdata)
{
    SwMarkdownParser* parser = static_cast<SwMarkdownParser*>(userdata);

    OUString aText = rtl::OStringToOUString(std::string_view(text, size), RTL_TEXTENCODING_UTF8);

    switch (type)
    {
        case MD_TEXT_CODE:
            if (!parser->m_bInsideImage)
                parser->InsertText(aText);
            break;
        case MD_TEXT_HTML:
            parser->m_htmlData += aText;
            break;
        case MD_TEXT_NORMAL:
            if (!parser->m_bInsideImage)
                parser->InsertText(aText);
            break;
        case MD_TEXT_NULLCHAR:
            break;
        case MD_TEXT_BR:
            parser->m_xDoc->getIDocumentContentOperations().InsertString(*parser->m_pPam,
                                                                         OUString(u'\n'));
            break;
        case MD_TEXT_SOFTBR:
            parser->m_xDoc->getIDocumentContentOperations().InsertString(*parser->m_pPam,
                                                                         OUString(CHAR_HARDBLANK));
            break;
        case MD_TEXT_ENTITY:
            break;
        case MD_TEXT_LATEXMATH:
            break;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
