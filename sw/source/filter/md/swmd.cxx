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

#include <boost/property_tree/json_parser/error.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#include <osl/diagnose.h>
#include <numrule.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <fltini.hxx>
#include <fchrfmt.hxx>
#include <swerror.h>
#include <strings.hrc>
#include <mdiexp.hxx>
#include <poolfmt.hxx>
#include <iodetect.hxx>
#include <hintids.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <tools/urlobj.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <fmtinfmt.hxx>
#include <frmatr.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <unotools/securityoptions.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <comphelper/random.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <rtl/uri.hxx>
#include <ndgrf.hxx>
#include <swtypes.hxx>
#include <fmturl.hxx>
#include <formatcontentcontrol.hxx>
#include <docsh.hxx>
#include <unicode/utypes.h>
#include <unicode/ucsdet.h>
#include <rtl/tencinfo.h>

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

    SwTextFormatColl* pColl = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(
        SwPoolFormatId::COLL_HTML_BLOCKQUOTE);

    m_nBlockQuoteDepth++;

    // Set the style on the current paragraph.
    m_xDoc->SetTextFormatColl(*m_pPam, pColl);
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
            = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(SwPoolFormatId::COLL_TEXT);
        m_xDoc->SetTextFormatColl(*m_pPam, pColl);
    }

    m_nBlockQuoteDepth--;
}

void SwMarkdownParser::AddHR()
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);

    SwTextFormatColl* pColl
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(SwPoolFormatId::COLL_HTML_HR);
    m_xDoc->SetTextFormatColl(*m_pPam, pColl);
}

void SwMarkdownParser::EndHR()
{
    AppendTextNode(AM_SPACE);
    SwTextFormatColl* pColl
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(SwPoolFormatId::COLL_TEXT);
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
        SwPoolFormatId::COLL_HEADLINE_BASE + nLvl);
    m_xDoc->SetTextFormatColl(*m_pPam, pColl);
}

void SwMarkdownParser::EndHeading()
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);
    else
        AddParSpace();

    SwTextFormatColl* pColl
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(SwPoolFormatId::COLL_TEXT);
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
        SwPoolFormatId nChrFormatPoolId = SwPoolFormatId::ZERO;
        if (aListType == MD_BLOCK_OL)
        {
            aNumFormat.SetNumberingType(SVX_NUM_ARABIC);
            nChrFormatPoolId = SwPoolFormatId::CHR_NUM_LEVEL;
        }
        else
        {
            if (numfunc::IsDefBulletFontUserDefined())
            {
                aNumFormat.SetBulletFont(&numfunc::GetDefBulletFont());
            }
            aNumFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aNumFormat.SetBulletChar(cBulletChar);
            nChrFormatPoolId = SwPoolFormatId::CHR_BULLET_LEVEL;
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

void SwMarkdownParser::StartNumberedBulletListItem(MD_BLOCK_LI_DETAIL aDetail)
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

        if (aDetail.is_task)
        {
            // Map the task list item to a Writer checkbox content control.
            bool bChecked = (aDetail.task_mark == 'x' || aDetail.task_mark == 'X') ? true : false;
            auto pContentControl = std::make_shared<SwContentControl>(nullptr);
            sal_Int32 nId = comphelper::rng::uniform_uint_distribution(
                1, std::numeric_limits<sal_Int32>::max());
            SwFormatContentControl aContentControl(pContentControl, RES_TXTATR_CONTENTCONTROL);
            pContentControl->SetId(nId);
            pContentControl->SetCheckbox(true);
            pContentControl->SetCheckedState(SwContentControl::CHECKED_STATE);
            pContentControl->SetUncheckedState(SwContentControl::UNCHECKED_STATE);
            pContentControl->SetChecked(bChecked);
            OUString aPlaceholder;
            if (bChecked)
            {
                aPlaceholder = SwContentControl::CHECKED_STATE;
            }
            else
            {
                aPlaceholder = SwContentControl::UNCHECKED_STATE;
            }
            pTextNode->InsertText(aPlaceholder, SwContentIndex(pTextNode, pTextNode->Len()));
            SwPosition aStart(*m_pPam->GetPoint());
            aStart.nContent -= aPlaceholder.getLength();
            SwPosition aEnd(*m_pPam->GetPoint());
            SwPaM aPaM(aStart, aEnd);
            m_xDoc->getIDocumentContentOperations().InsertPoolItem(aPaM, aContentControl);
            pTextNode->InsertText(u" "_ustr, SwContentIndex(pTextNode, pTextNode->Len()));
        }
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
        aNumFormat.SetCharFormat(m_xDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool(
            SwPoolFormatId::CHR_BULLET_LEVEL));
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
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(SwPoolFormatId::COLL_HTML_PRE);
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
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(SwPoolFormatId::COLL_TEXT);
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

void SwMarkdownParser::InsertImage(const MDImage& rImg)
{
    OUString sGrfNm = INetURLObject::GetAbsURL(m_sBaseURL, rImg.url);

    Graphic aGraphic;
    INetURLObject aGraphicURL(sGrfNm);
    if (aGraphicURL.GetProtocol() == INetProtocol::Data)
    {
        // 'data:' URL: read that here, initialize aGraphic and clear sGrfNm.
        std::unique_ptr<SvMemoryStream> pStream = aGraphicURL.getData();
        if (pStream)
        {
            GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
            aGraphic = rFilter.ImportUnloadedGraphic(*pStream);
            sGrfNm.clear();
        }
    }

    if (!sGrfNm.isEmpty())
    {
        aGraphic.SetDefaultType();
    }

    Size aGrfSz(0, 0);
    if (allowAccessLink(*m_xDoc) && !aGraphicURL.IsExoticProtocol() && !sGrfNm.isEmpty())
    {
        GraphicDescriptor aDescriptor(aGraphicURL);
        if (aDescriptor.Detect(true))
            aGrfSz
                = o3tl::convert(aDescriptor.GetSizePixel(), o3tl::Length::px, o3tl::Length::twip);
    }
    else if (aGraphic.GetType() == GraphicType::Bitmap)
    {
        // We have an unloaded graphic in aGraphic, read its size.
        aGrfSz = o3tl::convert(aGraphic.GetSizePixel(), o3tl::Length::px, o3tl::Length::twip);
    }

    tools::Long nWidth = 0;
    tools::Long nHeight = 0;
    if (!aGrfSz.IsEmpty())
    {
        nWidth = aGrfSz.getWidth();
        nHeight = aGrfSz.getHeight();
        if (nWidth > MD_MAX_IMAGE_WIDTH_IN_TWIPS || nHeight > MD_MAX_IMAGE_HEIGHT_IN_TWIPS)
        {
            double fScaleX = static_cast<double>(MD_MAX_IMAGE_WIDTH_IN_TWIPS) / nWidth;
            double fScaleY = static_cast<double>(MD_MAX_IMAGE_HEIGHT_IN_TWIPS) / nHeight;
            double fScale = std::min(fScaleX, fScaleY);
            nWidth = static_cast<tools::Long>(nWidth * fScale);
            nHeight = static_cast<tools::Long>(nHeight * fScale);
        }
        if (nWidth < MD_MIN_IMAGE_WIDTH_IN_TWIPS && nWidth < MD_MIN_IMAGE_HEIGHT_IN_TWIPS)
        {
            nWidth = MD_MIN_IMAGE_WIDTH_IN_TWIPS;
            nHeight = MD_MIN_IMAGE_HEIGHT_IN_TWIPS;
        }
    }
    else
    {
        nWidth = MD_MIN_IMAGE_WIDTH_IN_TWIPS;
        nHeight = MD_MIN_IMAGE_HEIGHT_IN_TWIPS;
    }

    SfxItemSet aFlySet(
        SfxItemSet::makeFixedSfxItemSet<RES_FRMATR_BEGIN, RES_FRMATR_END>(m_xDoc->GetAttrPool()));

    aFlySet.Put(SwFormatAnchor(RndStdIds::FLY_AS_CHAR));
    aFlySet.Put(SwFormatFrameSize(SwFrameSize::Fixed, nWidth, nHeight));
    aFlySet.Put(SwFormatHoriOrient(0, text::HoriOrientation::NONE, text::RelOrientation::CHAR));
    aFlySet.Put(
        SwFormatVertOrient(0, text::VertOrientation::CHAR_CENTER, text::RelOrientation::CHAR));

    if (!rImg.link.isEmpty())
    {
        // Have a link, set that on the image.
        SwFormatURL aFormatURL;
        aFormatURL.SetURL(rImg.link, /*bServerMap=*/false);
        aFlySet.Put(aFormatURL);
    }

    SanitizeAnchor(aFlySet);

    SwFlyFrameFormat* pFlyFormat = m_xDoc->getIDocumentContentOperations().InsertGraphic(
        *m_pPam, sGrfNm, OUString(), &aGraphic, &aFlySet, nullptr, nullptr);

    if (pFlyFormat)
    {
        pFlyFormat->SetObjTitle(rImg.title);
        pFlyFormat->SetObjDescription(rImg.desc);
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
    , m_bNewDoc(bReadNewDoc)
{
    rCursor.DeleteMark();
    m_pPam = &rCursor;
    m_rInput.ResetError();
    m_nFilesize = m_rInput.TellEnd();
    m_rInput.Seek(STREAM_SEEK_TO_BEGIN);
    m_rInput.ResetError();
}

void MarkdownReader::SetupFilterOptions(SwDoc& rDoc)
{
    // See if any import options are provided: if so, collect them into a map.
    if (!m_pMedium)
    {
        return;
    }

    auto pItem = m_pMedium->GetItemSet().GetItem(SID_FILE_FILTEROPTIONS);
    if (!pItem)
    {
        return;
    }

    OUString aFilterOptions = pItem->GetValue();
    if (!aFilterOptions.startsWith("{"))
    {
        return;
    }

    uno::Sequence<beans::PropertyValue> aFilterData;
    try
    {
        std::vector<beans::PropertyValue> aData
            = comphelper::JsonToPropertyValues(aFilterOptions.toUtf8());
        aFilterData = comphelper::containerToSequence(aData);
    }
    catch (const boost::property_tree::json_parser::json_parser_error& e)
    {
        SAL_WARN("sw.md", "failed to parse FilterOptions as JSON: " << e.message());
    }
    comphelper::SequenceAsHashMap aMap(aFilterData);
    OUString aTemplateURL;
    aMap[u"TemplateURL"_ustr] >>= aTemplateURL;
    if (aTemplateURL.isEmpty())
    {
        return;
    }

    // Have a TemplateURL: open it in a new object shell.
    if (!m_pMedium->GetName().isEmpty())
    {
        aTemplateURL = rtl::Uri::convertRelToAbs(m_pMedium->GetName(), aTemplateURL);
    }
    SwDocShell* pDocShell = rDoc.GetDocShell();
    if (!pDocShell)
    {
        return;
    }

    // Go via filter detection so non-ODF templates work, too.
    uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
    uno::Reference<frame::XDesktop2> xComponentLoader = frame::Desktop::create(xContext);
    uno::Sequence<css::beans::PropertyValue> aTemplateArgs = {
        comphelper::makePropertyValue("Hidden", true),
    };
    uno::Reference<lang::XComponent> xTemplateComponent
        = xComponentLoader->loadComponentFromURL(aTemplateURL, u"_blank"_ustr, 0, aTemplateArgs);
    auto pTemplateModel = dynamic_cast<SfxBaseModel*>(xTemplateComponent.get());
    if (!pTemplateModel)
    {
        return;
    }

    SfxObjectShell* pTemplateShell = pTemplateModel->GetObjectShell();
    if (!pTemplateShell)
    {
        return;
    }

    // Copy the styles from the template doc to our document.
    pDocShell->LoadStyles(*pTemplateShell);

    xTemplateComponent->dispose();
}

ErrCodeMsg MarkdownReader::Read(SwDoc& rDoc, const OUString& rBaseURL, SwPaM& rPam, const OUString&)
{
    ErrCode nRet;

    SetupFilterOptions(rDoc);
    SwMarkdownParser parser(rDoc, rPam, *m_pStream, rBaseURL, !m_bInsertMode);
    nRet = parser.CallParser();

    return nRet;
}

ErrCode SwMarkdownParser::CallParser()
{
    // use utf8
    rtl_TextEncoding eSrcEnc = RTL_TEXTENCODING_DONTKNOW;
    m_rInput.StartReadingUnicodeText(eSrcEnc);
    if (m_rInput.good())
    {
        sal_uInt64 nPos = m_rInput.Tell(); //bom size
        {
            std::vector<char> buf(65535); // Arbitrarily chosen 64KiB buffer
            const size_t nSize = m_rInput.ReadBytes(buf.data(), buf.size());
            if (nSize > 0)
            {
                UErrorCode uerr = U_ZERO_ERROR;
                UCharsetDetector* ucd = ucsdet_open(&uerr);
                ucsdet_setText(ucd, buf.data(), nSize, &uerr);
                if (const UCharsetMatch* match = ucsdet_detect(ucd, &uerr))
                {
                    const char* pEncodingName = ucsdet_getName(match, &uerr);

                    if (strcmp("UTF-16LE", pEncodingName) == 0)
                    {
                        eSrcEnc = RTL_TEXTENCODING_UCS2;
                        m_rInput.SetEndian(SvStreamEndian::LITTLE);
                    }
                    else if (strcmp("UTF-16BE", pEncodingName) == 0)
                    {
                        eSrcEnc = RTL_TEXTENCODING_UCS2;
                        m_rInput.SetEndian(SvStreamEndian::BIG);
                    }
                    else
                    {
                        eSrcEnc = rtl_getTextEncodingFromMimeCharset(pEncodingName);
                    }
                }
                ucsdet_close(ucd);
            }
            else
            {
                return ERRCODE_IO_INVALIDLENGTH;
            }
        }

        if (eSrcEnc == RTL_TEXTENCODING_DONTKNOW)
            return ERRCODE_IO_INVALIDCHAR;

        m_rInput.Seek(nPos);
        m_rInput.ResetError();
        m_nFilesize -= nPos;

        OUString sData;
        OString sUtf8Data;

        if (eSrcEnc == RTL_TEXTENCODING_UCS2)
        {
            if (m_nFilesize & 1)
                return ERRCODE_IO_INVALIDCHAR;

            tools::Long nChars = m_nFilesize / 2;
            std::vector<sal_Unicode> aCharData(nChars);

            for (tools::Long n = 0; n < nChars; n++)
            {
                m_rInput.ReadUtf16(aCharData[n]);
            }

            sData = OUString(aCharData.data(), nChars);
            sUtf8Data = OUStringToOString(sData, RTL_TEXTENCODING_UTF8);
        }
        else
        {
            tools::Long nChars = m_nFilesize;
            std::vector<char> aCharData(nChars);
            m_rInput.ReadBytes(aCharData.data(), nChars);
            sData = OUString(aCharData.data(), nChars, eSrcEnc);
            sUtf8Data = OUStringToOString(sData, RTL_TEXTENCODING_UTF8);
        }

        if (sUtf8Data.getLength())
        {
            m_nFilesize = sUtf8Data.getLength();
            m_pArr.reset(new char[m_nFilesize + 1]);
            memcpy(m_pArr.get(), sUtf8Data.getStr(), m_nFilesize);
            //HACK: At least the implementation of md4c 0.5.2 apparently expects the passed-in
            // memory to be null-terminated (it calls e.g. strcspn on it), so pass in an additional
            // byte:
            m_pArr[m_nFilesize] = 0;
        }
        else
        {
            return ERRCODE_IO_INVALIDCHAR;
        }
    }
    else
    {
        return ERRCODE_IO_INVALIDCHAR;
    }

    ::StartProgress(STR_STATSTR_W4WREAD, 0, m_nFilesize, m_xDoc->GetDocShell());

    SwTextFormatColl* pColl
        = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(SwPoolFormatId::COLL_TEXT);
    m_xDoc->SetTextFormatColl(*m_pPam, pColl);

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
