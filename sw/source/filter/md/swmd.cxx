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

#include <osl/diagnose.h>
#include <list.hxx>
#include <numrule.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <fmthdft.hxx>
#include <fltini.hxx>
#include <itabenum.hxx>
#include <fchrfmt.hxx>
#include <swerror.h>
#include <strings.hrc>
#include <mdiexp.hxx>
#include <poolfmt.hxx>
#include <iodetect.hxx>
#include <hintids.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/itemiter.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <fmtinfmt.hxx>
#include <frmatr.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <unotools/securityoptions.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <ndgrf.hxx>
#include <fmtcntnt.hxx>
#include <swtypes.hxx>

#include "swmd.hxx"

namespace
{
bool allowAccessLink(const SwDoc& rDoc)
{
    OUString sReferer;
    SfxObjectShell* sh = rDoc.GetPersist();
    if (sh != nullptr && sh->HasName())
    {
        sReferer = sh->GetMedium()->GetName();
    }
    return !SvtSecurityOptions::isUntrustedReferer(sReferer);
}
}

void SwMarkdownParser::SetNodeNum(sal_uInt8 nLevel)
{
    SwTextNode* pTextNode = m_pPam->GetPointNode().GetTextNode();
    if (!pTextNode)
    {
        SAL_WARN("sw.md", "No Text-Node at PaM-Position");
        return;
    }

    OSL_ENSURE(GetNumInfo().GetNumRule(), "No numbering rule");
    const UIName& rName = GetNumInfo().GetNumRule()->GetName();
    static_cast<SwContentNode*>(pTextNode)->SetAttr(SwNumRuleItem(rName));

    pTextNode->SetAttrListLevel(nLevel);
    pTextNode->SetCountedInList(false);

    // Invalidate NumRule, it may have been set valid because of an EndAction
    GetNumInfo().GetNumRule()->Invalidate();
}

sal_Int32 SwMarkdownParser::StripTrailingLF()
{
    sal_Int32 nStripped = 0;

    const sal_Int32 nLen = m_pPam->GetPoint()->GetContentIndex();
    if (nLen)
    {
        SwTextNode* pTextNd = m_pPam->GetPoint()->GetNode().GetTextNode();

        if (pTextNd)
        {
            sal_Int32 nPos = nLen;
            sal_Int32 nLFCount = 0;
            while (nPos && ('\x0a' == pTextNd->GetText()[--nPos]))
                nLFCount++;

            if (nLFCount)
            {
                if (nLFCount > 2)
                {
                    nLFCount = 2;
                }

                nPos = nLen - nLFCount;
                SwContentIndex nIdx(pTextNd, nPos);
                pTextNd->EraseText(nIdx, nLFCount);
                nStripped = nLFCount;
            }
        }
    }

    return nStripped;
}

bool SwMarkdownParser::AppendTextNode(SwMdAppendMode eMode, bool bUpdateNum)
{
    sal_Int32 nLFStripped = StripTrailingLF();
    if ((AM_NOSPACE == eMode || AM_SOFTNOSPACE == eMode) && nLFStripped > 1)
        eMode = AM_SPACE;

    SwTextNode* pTextNode = (AM_SPACE == eMode || AM_NOSPACE == eMode)
                                ? m_pPam->GetPoint()->GetNode().GetTextNode()
                                : nullptr;

    if (pTextNode)
    {
        const SvxULSpaceItem& rULSpace = pTextNode->SwContentNode::GetAttr(RES_UL_SPACE);

        bool bChange = AM_NOSPACE == eMode ? rULSpace.GetLower() > 0 : rULSpace.GetLower() == 0;

        if (bChange)
        {
            const SvxULSpaceItem& rCollULSpace = pTextNode->GetAnyFormatColl().GetULSpace();

            bool bMayReset
                = AM_NOSPACE == eMode ? rCollULSpace.GetLower() == 0 : rCollULSpace.GetLower() > 0;

            if (bMayReset && rCollULSpace.GetUpper() == rULSpace.GetUpper())
            {
                pTextNode->ResetAttr(RES_UL_SPACE);
            }
            else
            {
                pTextNode->SetAttr(SvxULSpaceItem(
                    rULSpace.GetUpper(), AM_NOSPACE == eMode ? 0 : MD_PARSPACE, RES_UL_SPACE));
            }
        }
    }
    m_bNoParSpace = AM_NOSPACE == eMode || AM_SOFTNOSPACE == eMode;

    bool bRet = m_xDoc->getIDocumentContentOperations().AppendTextNode(*m_pPam->GetPoint());

    if (bUpdateNum)
    {
        if (GetNumInfo().GetDepth())
        {
            sal_uInt8 nLvl = GetNumInfo().GetLevel();
            SetNodeNum(nLvl);
        }
        else
            m_pPam->GetPointNode().GetTextNode()->ResetAttr(RES_PARATR_NUMRULE);
    }

    return bRet;
}

void SwMarkdownParser::AddParSpace()
{
    //If it already has ParSpace, return
    if (!m_bNoParSpace)
        return;

    m_bNoParSpace = false;

    SwNodeOffset nNdIdx = m_pPam->GetPoint()->GetNodeIndex() - 1;

    SwTextNode* pTextNode = m_xDoc->GetNodes()[nNdIdx]->GetTextNode();
    if (!pTextNode)
        return;

    SvxULSpaceItem rULSpace = pTextNode->SwContentNode::GetAttr(RES_UL_SPACE);
    if (rULSpace.GetLower())
        return;

    const SvxULSpaceItem& rCollULSpace = pTextNode->GetAnyFormatColl().GetULSpace();
    if (rCollULSpace.GetLower() && rCollULSpace.GetUpper() == rULSpace.GetUpper())
    {
        pTextNode->ResetAttr(RES_UL_SPACE);
    }

    pTextNode->SetAttr(SvxULSpaceItem(rULSpace.GetUpper(), MD_PARSPACE, RES_UL_SPACE));
}

void SwMarkdownParser::AddBlockQuote()
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);
    else
        AddParSpace();

    SwTextFormatColl* pColl
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_HTML_BLOCKQUOTE);

    m_nBlockQuoteDepth++;

    sal_Int32 nBaseLeftIndent = pColl->GetTextLeftMargin().ResolveTextLeft({});
    sal_Int32 nLeftIndent = nBaseLeftIndent + m_nBlockQuoteDepth * nBaseLeftIndent;
    SvxTextLeftMarginItem aLeftMargin(pColl->GetTextLeftMargin());
    aLeftMargin.SetTextLeft(SvxIndentValue::twips(nLeftIndent));
    m_xDoc->getIDocumentContentOperations().InsertPoolItem(*m_pPam, aLeftMargin);
}

void SwMarkdownParser::EndBlockQuote()
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);
    else
        AddParSpace();

    if (m_nBlockQuoteDepth == 0)
    {
        SwTextFormatColl* pColl
            = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_TEXT);
        m_xDoc->SetTextFormatColl(*m_pPam, pColl);
    }

    m_nBlockQuoteDepth--;
}

void SwMarkdownParser::AddHR()
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);

    SwTextFormatColl* pColl
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_HTML_HR);
    m_xDoc->SetTextFormatColl(*m_pPam, pColl);
}

void SwMarkdownParser::EndHR()
{
    AppendTextNode(AM_SPACE);
    SwTextFormatColl* pColl
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_TEXT);
    m_xDoc->SetTextFormatColl(*m_pPam, pColl);
}

void SwMarkdownParser::StartPara()
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);
    else
        AddParSpace();
}

void SwMarkdownParser::EndPara()
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);
    else
        AddParSpace();
}

void SwMarkdownParser::StartHeading(sal_uInt8 nLvl)
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);
    else
        AddParSpace();

    SwTextFormatColl* pColl = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(
        RES_POOLCOLL_HEADLINE_BASE + nLvl);
    m_xDoc->SetTextFormatColl(*m_pPam, pColl);
}

void SwMarkdownParser::EndHeading()
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);
    else
        AddParSpace();

    SwTextFormatColl* pColl
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_TEXT);
    m_xDoc->SetTextFormatColl(*m_pPam, pColl);
}

void SwMarkdownParser::StartNumberedBulletList(MD_BLOCKTYPE aListType)
{
    SwMdNumRuleInfo& rInfo = GetNumInfo();

    bool bSpace = (rInfo.GetDepth()) == 0;
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(bSpace ? AM_SPACE : AM_NOSPACE, false);
    else if (bSpace)
        AddParSpace();

    rInfo.IncDepth();
    sal_uInt8 nLevel
        = static_cast<sal_uInt8>((rInfo.GetDepth() <= MAXLEVEL ? rInfo.GetDepth() : MAXLEVEL) - 1);

    if (!rInfo.GetNumRule())
    {
        sal_uInt16 nPos = m_xDoc->MakeNumRule(m_xDoc->GetUniqueNumRuleName());
        rInfo.SetNumRule(m_xDoc->GetNumRuleTable()[nPos]);
    }

    bool bNewNumFormat = rInfo.GetNumRule()->GetNumFormat(nLevel) == nullptr;
    bool bChangeNumFormat = false;

    // Create the default numbering format
    SwNumFormat aNumFormat(rInfo.GetNumRule()->Get(nLevel));
    rInfo.SetNodeStartValue(nLevel);
    if (bNewNumFormat)
    {
        sal_uInt16 nChrFormatPoolId = 0;
        if (aListType == MD_BLOCK_OL)
        {
            aNumFormat.SetNumberingType(SVX_NUM_ARABIC);
            nChrFormatPoolId = RES_POOLCHR_NUM_LEVEL;
        }
        else
        {
            if (numfunc::IsDefBulletFontUserDefined())
            {
                aNumFormat.SetBulletFont(&numfunc::GetDefBulletFont());
            }
            aNumFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aNumFormat.SetBulletChar(cBulletChar);
            nChrFormatPoolId = RES_POOLCHR_BULLET_LEVEL;
        }

        sal_Int32 nAbsLSpace = MD_NUMBER_BULLET_MARGINLEFT;

        sal_Int32 nFirstLineIndent = MD_NUMBER_BULLET_INDENT;
        if (nLevel > 0)
        {
            const SwNumFormat& rPrevNumFormat = rInfo.GetNumRule()->Get(nLevel - 1);
            nAbsLSpace = nAbsLSpace + rPrevNumFormat.GetAbsLSpace();
            nFirstLineIndent = rPrevNumFormat.GetFirstLineOffset();
        }
        aNumFormat.SetAbsLSpace(nAbsLSpace);
        aNumFormat.SetFirstLineOffset(nFirstLineIndent);
        aNumFormat.SetCharFormat(
            m_xDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool(nChrFormatPoolId));

        bChangeNumFormat = true;
    }
    else if (1 != aNumFormat.GetStart())
    {
        // If the layer has already been used, the start value may need to be set hard to the paragraph.
        rInfo.SetNodeStartValue(nLevel, 1);
    }

    {
        sal_uInt8 nLvl = nLevel;
        SetNodeNum(nLvl);
    }

    if (bChangeNumFormat)
    {
        rInfo.GetNumRule()->Set(nLevel, aNumFormat);
        m_xDoc->ChgNumRuleFormats(*rInfo.GetNumRule());
    }
}

void SwMarkdownParser::EndNumberedBulletList()
{
    SwMdNumRuleInfo& rInfo = GetNumInfo();

    // A new paragraph needs to be created, when
    // - the current one isn't empty (it contains text or paragraph-bound objects)
    // - the current one is numbered
    bool bAppend = m_pPam->GetPoint()->GetContentIndex() > 0;
    if (!bAppend)
    {
        SwTextNode* pTextNode = m_pPam->GetPointNode().GetTextNode();

        bAppend = (pTextNode && !pTextNode->IsOutline() && pTextNode->IsCountedInList());
    }

    bool bSpace = (rInfo.GetDepth()) == 1;
    if (bAppend)
        AppendTextNode(bSpace ? AM_SPACE : AM_NOSPACE, false);
    else if (bSpace)
        AddParSpace();

    if (rInfo.GetDepth() > 0)
    {
        rInfo.DecDepth();
        if (!rInfo.GetDepth())
        {
            // The formats not yet modified are now modified, to ease editing
            const SwNumFormat* pRefNumFormat = nullptr;
            bool bChanged = false;
            for (sal_uInt16 i = 0; i < MAXLEVEL; i++)
            {
                const SwNumFormat* pNumFormat = rInfo.GetNumRule()->GetNumFormat(i);
                if (pNumFormat)
                {
                    pRefNumFormat = pNumFormat;
                }
                else if (pRefNumFormat)
                {
                    SwNumFormat aNumFormat(rInfo.GetNumRule()->Get(i));
                    aNumFormat.SetNumberingType(pRefNumFormat->GetNumberingType() != SVX_NUM_BITMAP
                                                    ? pRefNumFormat->GetNumberingType()
                                                    : SVX_NUM_CHAR_SPECIAL);
                    if (SVX_NUM_CHAR_SPECIAL == aNumFormat.GetNumberingType())
                    {
                        if (numfunc::IsDefBulletFontUserDefined())
                        {
                            aNumFormat.SetBulletFont(&numfunc::GetDefBulletFont());
                        }
                        aNumFormat.SetBulletChar(cBulletChar);
                    }
                    aNumFormat.SetAbsLSpace((i + 1) * MD_NUMBER_BULLET_MARGINLEFT);
                    aNumFormat.SetFirstLineOffset(MD_NUMBER_BULLET_INDENT);
                    aNumFormat.SetCharFormat(pRefNumFormat->GetCharFormat());
                    rInfo.GetNumRule()->Set(i, aNumFormat);
                    bChanged = true;
                }
            }
            if (bChanged)
                m_xDoc->ChgNumRuleFormats(*rInfo.GetNumRule());

            // On the last append, the NumRule item and NodeNum object were copied.
            // Now we need to delete them. ResetAttr deletes the NodeNum object as well
            if (SwTextNode* pTextNode = m_pPam->GetPointNode().GetTextNode())
                pTextNode->ResetAttr(RES_PARATR_NUMRULE);

            rInfo.Clear();
        }
        else
        {
            // the next paragraph not numbered first
            SetNodeNum(rInfo.GetLevel());
        }
    }
}

void SwMarkdownParser::StartNumberedBulletListItem()
{
    sal_uInt8 nLevel = GetNumInfo().GetLevel();
    sal_uInt16 nStart = GetNumInfo().GetNodeStartValue(nLevel);

    GetNumInfo().SetNodeStartValue(nLevel);

    // create a new paragraph
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_NOSPACE, false);
    m_bNoParSpace = false; // no space in <LI>!

    SwTextNode* pTextNode = m_pPam->GetPointNode().GetTextNode();
    if (!pTextNode)
    {
        SAL_WARN("sw.md", "No Text-Node at PaM-Position");
        return;
    }

    UIName aNumRuleName;
    if (GetNumInfo().GetNumRule())
    {
        aNumRuleName = GetNumInfo().GetNumRule()->GetName();
    }
    else
    {
        aNumRuleName = m_xDoc->GetUniqueNumRuleName();
        SwNumRule aNumRule(aNumRuleName, SvxNumberFormat::LABEL_WIDTH_AND_POSITION);
        SwNumFormat aNumFormat(aNumRule.Get(0));
        if (numfunc::IsDefBulletFontUserDefined())
        {
            aNumFormat.SetBulletFont(&numfunc::GetDefBulletFont());
        }
        aNumFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
        aNumFormat.SetBulletChar(cBulletChar);
        aNumFormat.SetCharFormat(
            m_xDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool(RES_POOLCHR_BULLET_LEVEL));
        aNumFormat.SetFirstLineOffset(MD_NUMBER_BULLET_INDENT);
        aNumRule.Set(0, aNumFormat);

        m_xDoc->MakeNumRule(aNumRuleName, &aNumRule);
    }

    static_cast<SwContentNode*>(pTextNode)->SetAttr(SwNumRuleItem(aNumRuleName));
    pTextNode->SetAttrListLevel(nLevel);

    if (nLevel < MAXLEVEL)
    {
        pTextNode->SetCountedInList(true);
    }

    if (nStart != USHRT_MAX)
    {
        pTextNode->SetListRestart(true);
        pTextNode->SetAttrListRestartValue(nStart);
    }

    if (GetNumInfo().GetNumRule())
        GetNumInfo().GetNumRule()->Invalidate();
}

void SwMarkdownParser::EndNumberedBulletListItem()
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);
}

void SwMarkdownParser::BeginHtmlBlock()
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);
    else
        AddParSpace();
}

void SwMarkdownParser::InsertHtmlData()
{
    OString aData = rtl::OUStringToOString(m_htmlData, RTL_TEXTENCODING_UTF8);
    SvMemoryStream aStream(const_cast<char*>(aData.getStr()), aData.getLength(), StreamMode::READ);
    SwReader aReader(aStream, OUString(), OUString(), *m_pPam);
    aReader.Read(*ReadHTML);
}

void SwMarkdownParser::EndHtmlBlock()
{
    InsertHtmlData();
    m_htmlData.clear();
}

void SwMarkdownParser::BeginCodeBlock()
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);
    else
        AddParSpace();

    SwTextFormatColl* pColl
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_HTML_PRE);
    m_xDoc->SetTextFormatColl(*m_pPam, pColl);

    SvxBrushItem aBrushItem(COL_CODE_BLOCK, RES_BACKGROUND);
    m_xDoc->getIDocumentContentOperations().InsertPoolItem(*m_pPam, aBrushItem);
}

void SwMarkdownParser::EndCodeBlock()
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);
    else
        AddParSpace();

    SwTextFormatColl* pColl
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_TEXT);
    m_xDoc->SetTextFormatColl(*m_pPam, pColl);
    ClearAttrs();
}

void SwMarkdownParser::InsertText(OUString& aStr)
{
    SwContentNode* pCnd = m_pPam->GetPointContentNode();
    sal_Int32 nStartPos = m_pPam->GetPoint()->GetContentIndex();

    m_xDoc->getIDocumentContentOperations().InsertString(*m_pPam, aStr);

    SwPaM aAttrPam(*m_pPam->GetPoint());
    aAttrPam.SetMark();
    aAttrPam.GetMark()->Assign(*pCnd, nStartPos);

    if (!m_aAttrStack.empty())
    {
        SetAttrs(aAttrPam);
        ClearAttrs();
    }
}

void SwMarkdownParser::SetAttrs(SwPaM& rRange)
{
    for (const auto& pItem : m_aAttrStack)
    {
        if (pItem)
        {
            if (rRange.HasMark() && (*rRange.GetMark() != *rRange.GetPoint()))
            {
                m_xDoc->getIDocumentContentOperations().InsertPoolItem(rRange, *pItem);
            }
        }
    }
}

void SwMarkdownParser::ClearAttrs() { m_xDoc->ResetAttrs(*m_pPam, true); }

void SwMarkdownParser::InsertImage(const OUString& aURL, const OUString& rTitle)
{
    OUString sGrfNm = INetURLObject::GetAbsURL(m_sBaseURL, aURL);

    Graphic aGraphic;
    INetURLObject aGraphicURL(sGrfNm);

    if (!sGrfNm.isEmpty())
    {
        aGraphic.SetDefaultType();
    }

    Size aGrfSz(0, 0);
    if (allowAccessLink(*m_xDoc) && !aGraphicURL.IsExoticProtocol())
    {
        GraphicDescriptor aDescriptor(aGraphicURL);
        if (aDescriptor.Detect(true))
            aGrfSz
                = o3tl::convert(aDescriptor.GetSizePixel(), o3tl::Length::px, o3tl::Length::twip);
    }

    tools::Long nWidth = aGrfSz.getWidth();
    tools::Long nHeight = aGrfSz.getHeight();

    if (nWidth > 0 && nHeight > 0)
    {
        if (nWidth > MD_MAX_IMAGE_WIDTH_IN_TWIPS || nHeight > MD_MAX_IMAGE_HEIGH_IN_TWIPS)
        {
            double fScaleX = static_cast<double>(MD_MAX_IMAGE_WIDTH_IN_TWIPS) / nWidth;
            double fScaleY = static_cast<double>(MD_MAX_IMAGE_HEIGH_IN_TWIPS) / nHeight;
            double fScale = std::min(fScaleX, fScaleY);

            nWidth = static_cast<tools::Long>(nWidth * fScale);
            nHeight = static_cast<tools::Long>(nHeight * fScale);
        }
    }

    if (nWidth < MINFLY || nHeight < MINFLY)
    {
        nWidth = MINFLY;
        nHeight = MINFLY;
    }

    SfxItemSet aFlySet(
        SfxItemSet::makeFixedSfxItemSet<RES_FRM_SIZE, RES_VERT_ORIENT, RES_HORI_ORIENT, RES_ANCHOR>(
            m_xDoc->GetAttrPool()));

    aFlySet.Put(SwFormatAnchor(RndStdIds::FLY_AS_CHAR));
    aFlySet.Put(SwFormatFrameSize(SwFrameSize::Fixed, nWidth, nHeight));
    aFlySet.Put(SwFormatHoriOrient(0, text::HoriOrientation::NONE, text::RelOrientation::CHAR));
    aFlySet.Put(
        SwFormatVertOrient(0, text::VertOrientation::CHAR_CENTER, text::RelOrientation::CHAR));

    SanitizeAnchor(aFlySet);

    SwFlyFrameFormat* pFlyFormat = m_xDoc->getIDocumentContentOperations().InsertGraphic(
        *m_pPam, sGrfNm, OUString(), &aGraphic, &aFlySet, nullptr, nullptr);

    SwGrfNode* pGrfNd = m_xDoc->GetNodes()[pFlyFormat->GetContent().GetContentIdx()->GetIndex() + 1]
                            ->GetGrfNode();

    if (pGrfNd && !rTitle.isEmpty())
    {
        pGrfNd->SetTitle(rTitle);
    }

    m_bNoParSpace = true;
}

void SwMarkdownParser::RegisterTable(MDTable* pTable) { m_aTables.push_back(pTable); }

void SwMarkdownParser::DeRegisterTable(MDTable* pTable) { std::erase(m_aTables, pTable); }

SwMarkdownParser::SwMarkdownParser(SwDoc& rD, SwPaM& rCursor, SvStream& rIn, OUString aBaseURL,
                                   bool bReadNewDoc)
    : m_xDoc(&rD)
    , m_rInput(rIn)
    // , m_pMedium(&rMedium)
    , m_pNumRuleInfo(new SwMdNumRuleInfo)
    , m_sBaseURL(std::move(aBaseURL))
    , m_nBlockQuoteDepth(-1)
    , m_bNewDoc(bReadNewDoc)
{
    rCursor.DeleteMark();
    m_pPam = &rCursor;
    m_rInput.ResetError();
    m_nFilesize = m_rInput.TellEnd();
    m_rInput.Seek(STREAM_SEEK_TO_BEGIN);
    m_rInput.ResetError();
    m_pArr.reset(new char[m_nFilesize + 1]);
}

ErrCodeMsg MarkdownReader::Read(SwDoc& rDoc, const OUString& rBaseURL, SwPaM& rPam, const OUString&)
{
    ErrCode nRet;

    SwMarkdownParser parser(rDoc, rPam, *m_pStream, rBaseURL, !m_bInsertMode);
    nRet = parser.CallParser();

    return nRet;
}

ErrCode SwMarkdownParser::CallParser()
{
    ::StartProgress(STR_STATSTR_W4WREAD, 0, m_nFilesize, m_xDoc->GetDocShell());

    SwTextFormatColl* pColl
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_TEXT);
    m_xDoc->SetTextFormatColl(*m_pPam, pColl);
    m_rInput.ReadBytes(m_pArr.get(), m_nFilesize);
    m_pArr[m_nFilesize] = '\0';

    ErrCode nRet;

    MD_PARSER parser = { 0,
                         MD_DIALECT_GITHUB | MD_FLAG_WIKILINKS,
                         enter_block_callback,
                         leave_block_callback,
                         enter_span_callback,
                         leave_span_callback,
                         text_callback,
                         nullptr,
                         nullptr };

    int result = md_parse(m_pArr.get(), m_nFilesize, &parser, static_cast<void*>(this));

    if (result != 0)
    {
        nRet = ERRCODE_IO_GENERAL;
    }

    ::EndProgress(m_xDoc->GetDocShell());
    return nRet;
}

SwMarkdownParser::~SwMarkdownParser()
{
    m_pArr.reset();
    m_pNumRuleInfo.reset();
    m_xDoc.clear();
}

//static
void SwMarkdownParser::SanitizeAnchor(SfxItemSet& rFrameItemSet)
{
    const SwFormatAnchor& rAnch = rFrameItemSet.Get(RES_ANCHOR);
    if (SwNode* pAnchorNode = rAnch.GetAnchorNode())
    {
        if (pAnchorNode->IsEndNode())
        {
            SAL_WARN("sw.md", "Invalid EndNode Anchor");
            rFrameItemSet.ClearItem(RES_ANCHOR);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
