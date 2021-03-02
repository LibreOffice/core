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

#include "rtfattributeoutput.hxx"
#include <memory>
#include <cstring>
#include "rtfsdrexport.hxx"
#include "writerwordglue.hxx"
#include "ww8par.hxx"
#include <fmtcntnt.hxx>
#include <rtl/tencinfo.h>
#include <sal/log.hxx>
#include <sot/exchange.hxx>
#include <svtools/rtfkeywd.hxx>
#include <tools/UnitConversion.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/opaqitem.hxx>
#include <svx/svdouno.hxx>
#include <filter/msfilter/rtfutil.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <docufld.hxx>
#include <fmtclds.hxx>
#include <fmtrowsplt.hxx>
#include <fmtline.hxx>
#include <fmtanchr.hxx>
#include <ftninfo.hxx>
#include <htmltbl.hxx>
#include <ndgrf.hxx>
#include <pagedesc.hxx>
#include <swmodule.hxx>
#include <txtftn.hxx>
#include <txtinet.hxx>
#include <grfatr.hxx>
#include <ndole.hxx>
#include <lineinfo.hxx>
#include <redline.hxx>
#include <rtf.hxx>
#include <IDocumentSettingAccess.hxx>
#include <vcl/cvtgrf.hxx>
#include <oox/mathml/export.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <svl/grabbagitem.hxx>
#include <frmatr.hxx>
#include <swtable.hxx>
#include "rtfexport.hxx"

using namespace ::com::sun::star;
using namespace sw::util;

static OString OutTBLBorderLine(RtfExport const& rExport, const editeng::SvxBorderLine* pLine,
                                const char* pStr)
{
    OStringBuffer aRet;
    if (pLine && !pLine->isEmpty())
    {
        aRet.append(pStr);
        // single line
        switch (pLine->GetBorderLineStyle())
        {
            case SvxBorderLineStyle::SOLID:
            {
                if (DEF_LINE_WIDTH_0 == pLine->GetWidth())
                    aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRHAIR);
                else
                    aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRS);
            }
            break;
            case SvxBorderLineStyle::DOTTED:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRDOT);
                break;
            case SvxBorderLineStyle::DASHED:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRDASH);
                break;
            case SvxBorderLineStyle::DOUBLE:
            case SvxBorderLineStyle::DOUBLE_THIN:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRDB);
                break;
            case SvxBorderLineStyle::THINTHICK_SMALLGAP:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTNTHSG);
                break;
            case SvxBorderLineStyle::THINTHICK_MEDIUMGAP:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTNTHMG);
                break;
            case SvxBorderLineStyle::THINTHICK_LARGEGAP:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTNTHLG);
                break;
            case SvxBorderLineStyle::THICKTHIN_SMALLGAP:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTHTNSG);
                break;
            case SvxBorderLineStyle::THICKTHIN_MEDIUMGAP:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTHTNMG);
                break;
            case SvxBorderLineStyle::THICKTHIN_LARGEGAP:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTHTNLG);
                break;
            case SvxBorderLineStyle::EMBOSSED:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDREMBOSS);
                break;
            case SvxBorderLineStyle::ENGRAVED:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRENGRAVE);
                break;
            case SvxBorderLineStyle::OUTSET:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDROUTSET);
                break;
            case SvxBorderLineStyle::INSET:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRINSET);
                break;
            case SvxBorderLineStyle::FINE_DASHED:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRDASHSM);
                break;
            case SvxBorderLineStyle::DASH_DOT:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRDASHD);
                break;
            case SvxBorderLineStyle::DASH_DOT_DOT:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRDASHDD);
                break;
            case SvxBorderLineStyle::NONE:
            default:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRNONE);
                break;
        }

        double const fConverted(
            ::editeng::ConvertBorderWidthToWord(pLine->GetBorderLineStyle(), pLine->GetWidth()));
        if (255 >= pLine->GetWidth()) // That value comes from RTF specs
        {
            aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRW).append(static_cast<sal_Int32>(fConverted));
        }
        else
        {
            // use \brdrth to double the value range...
            aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTH OOO_STRING_SVTOOLS_RTF_BRDRW);
            aRet.append(static_cast<sal_Int32>(fConverted) / 2);
        }

        aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRCF);
        aRet.append(static_cast<sal_Int32>(rExport.GetColor(pLine->GetColor())));
    }
    return aRet.makeStringAndClear();
}

static OString OutBorderLine(RtfExport const& rExport, const editeng::SvxBorderLine* pLine,
                             const char* pStr, sal_uInt16 nDist,
                             SvxShadowLocation eShadowLocation = SvxShadowLocation::NONE)
{
    OStringBuffer aRet;
    aRet.append(OutTBLBorderLine(rExport, pLine, pStr));
    aRet.append(OOO_STRING_SVTOOLS_RTF_BRSP);
    aRet.append(static_cast<sal_Int32>(nDist));
    if (eShadowLocation == SvxShadowLocation::BottomRight)
        aRet.append(LO_STRING_SVTOOLS_RTF_BRDRSH);
    return aRet.makeStringAndClear();
}

void RtfAttributeOutput::RTLAndCJKState(bool bIsRTL, sal_uInt16 nScript)
{
    m_bIsRTL = bIsRTL;
    m_nScript = nScript;
    m_bControlLtrRtl = true;
}

void RtfAttributeOutput::StartParagraph(ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo)
{
    if (m_bIsBeforeFirstParagraph && m_rExport.m_nTextTyp != TXT_HDFT)
        m_bIsBeforeFirstParagraph = false;

    // Output table/table row/table cell starts if needed
    if (pTextNodeInfo)
    {
        sal_uInt32 nRow = pTextNodeInfo->getRow();
        sal_uInt32 nCell = pTextNodeInfo->getCell();

        // New cell/row?
        if (m_nTableDepth > 0 && !m_bTableCellOpen)
        {
            ww8::WW8TableNodeInfoInner::Pointer_t pDeepInner(
                pTextNodeInfo->getInnerForDepth(m_nTableDepth));
            OSL_ENSURE(pDeepInner, "TableNodeInfoInner not found");
            // Make sure we always start a row between ending one and starting a cell.
            // In case of subtables, we may not get the first cell.
            if (pDeepInner && (pDeepInner->getCell() == 0 || m_bTableRowEnded))
            {
                StartTableRow(pDeepInner);
            }

            StartTableCell();
        }

        // Again, if depth was incremented, start a new table even if we skipped the first cell.
        if ((nRow == 0 && nCell == 0) || (m_nTableDepth == 0 && pTextNodeInfo->getDepth()))
        {
            // Do we have to start the table?
            // [If we are at the right depth already, it means that we
            // continue the table cell]
            sal_uInt32 nCurrentDepth = pTextNodeInfo->getDepth();

            if (nCurrentDepth > m_nTableDepth)
            {
                // Start all the tables that begin here
                for (sal_uInt32 nDepth = m_nTableDepth + 1; nDepth <= pTextNodeInfo->getDepth();
                     ++nDepth)
                {
                    ww8::WW8TableNodeInfoInner::Pointer_t pInner(
                        pTextNodeInfo->getInnerForDepth(nDepth));

                    m_bLastTable = (nDepth == pTextNodeInfo->getDepth());
                    StartTable();
                    StartTableRow(pInner);
                    StartTableCell();
                }

                m_nTableDepth = nCurrentDepth;
            }
        }
    }

    OSL_ENSURE(m_aRun.getLength() == 0, "m_aRun is not empty");
}

void RtfAttributeOutput::EndParagraph(ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner)
{
    bool bLastPara = false;
    if (m_rExport.m_nTextTyp == TXT_FTN || m_rExport.m_nTextTyp == TXT_EDN
        || m_rExport.m_rDoc.IsClipBoard())
    {
        // We're ending a paragraph that is the last paragraph of a footnote or endnote, or of clipboard.
        bLastPara
            = m_rExport.GetCurrentNodeIndex()
              && m_rExport.GetCurrentNodeIndex() == m_rExport.m_pCurPam->End()->nNode.GetIndex();
    }

    FinishTableRowCell(pTextNodeInfoInner);

    RtfStringBuffer aParagraph;

    aParagraph.appendAndClear(m_aRun);
    aParagraph->append(m_aAfterRuns.makeStringAndClear());
    if (m_bTableAfterCell)
        m_bTableAfterCell = false;
    else
    {
        aParagraph->append(SAL_NEWLINE_STRING);
        // RTF_PAR at the end of the footnote or clipboard, would cause an additional empty paragraph.
        if (!bLastPara)
        {
            aParagraph->append(OOO_STRING_SVTOOLS_RTF_PAR);
            aParagraph->append(' ');
        }
    }
    if (m_nColBreakNeeded)
    {
        aParagraph->append(OOO_STRING_SVTOOLS_RTF_COLUMN);
        m_nColBreakNeeded = false;
    }

    if (!m_bBufferSectionHeaders)
        aParagraph.makeStringAndClear(this);
    else
        m_aSectionHeaders.append(aParagraph.makeStringAndClear());
}

void RtfAttributeOutput::EmptyParagraph()
{
    m_rExport.Strm()
        .WriteCharPtr(SAL_NEWLINE_STRING)
        .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PAR)
        .WriteChar(' ');
}

void RtfAttributeOutput::SectionBreaks(const SwNode& rNode)
{
    SwNodeIndex aNextIndex(rNode, 1);
    if (rNode.IsTextNode())
    {
        OSL_ENSURE(m_aStyles.getLength() == 0, "m_aStyles is not empty");

        // output page/section breaks
        m_rExport.Strm().WriteOString(m_aSectionBreaks.makeStringAndClear());
        m_bBufferSectionBreaks = true;

        // output section headers / footers
        if (!m_bBufferSectionHeaders)
            m_rExport.Strm().WriteOString(m_aSectionHeaders.makeStringAndClear());

        if (aNextIndex.GetNode().IsTextNode())
        {
            const SwTextNode* pTextNode = static_cast<SwTextNode*>(&aNextIndex.GetNode());
            m_rExport.OutputSectionBreaks(pTextNode->GetpSwAttrSet(), *pTextNode);
            // Save the current page description for now, so later we will be able to access the previous one.
            m_pPrevPageDesc = pTextNode->FindPageDesc();
        }
        else if (aNextIndex.GetNode().IsTableNode())
        {
            const SwTableNode* pTableNode = static_cast<SwTableNode*>(&aNextIndex.GetNode());
            const SwFrameFormat* pFormat = pTableNode->GetTable().GetFrameFormat();
            m_rExport.OutputSectionBreaks(&(pFormat->GetAttrSet()), *pTableNode);
        }
        m_bBufferSectionBreaks = false;
    }
    else if (rNode.IsEndNode())
    {
        // End of something: make sure that it's the end of a table.
        assert(rNode.StartOfSectionNode()->IsTableNode());
        if (aNextIndex.GetNode().IsTextNode())
        {
            // Handle section break between a table and a text node following it.
            const SwTextNode* pTextNode = aNextIndex.GetNode().GetTextNode();
            m_rExport.OutputSectionBreaks(pTextNode->GetpSwAttrSet(), *pTextNode);
        }
    }
}

void RtfAttributeOutput::StartParagraphProperties()
{
    OStringBuffer aPar;
    if (!m_rExport.GetRTFFlySyntax())
    {
        aPar.append(OOO_STRING_SVTOOLS_RTF_PARD);
        aPar.append(OOO_STRING_SVTOOLS_RTF_PLAIN);
        aPar.append(' ');
    }
    if (!m_bBufferSectionHeaders)
        m_rExport.Strm().WriteOString(aPar.makeStringAndClear());
    else
        m_aSectionHeaders.append(aPar.makeStringAndClear());
}

void RtfAttributeOutput::EndParagraphProperties(
    const SfxItemSet& /*rParagraphMarkerProperties*/, const SwRedlineData* /*pRedlineData*/,
    const SwRedlineData* /*pRedlineParagraphMarkerDeleted*/,
    const SwRedlineData* /*pRedlineParagraphMarkerInserted*/)
{
    const OString aProperties = MoveCharacterProperties(true);
    m_rExport.Strm().WriteOString(aProperties);
}

void RtfAttributeOutput::StartRun(const SwRedlineData* pRedlineData, sal_Int32 /*nPos*/,
                                  bool bSingleEmptyRun)
{
    SAL_INFO("sw.rtf", __func__ << ", bSingleEmptyRun: " << bSingleEmptyRun);

    m_bInRun = true;
    m_bSingleEmptyRun = bSingleEmptyRun;
    if (!m_bSingleEmptyRun)
        m_aRun->append('{');

    // if there is some redlining in the document, output it
    Redline(pRedlineData);

    OSL_ENSURE(m_aRunText.getLength() == 0, "m_aRunText is not empty");
}

void RtfAttributeOutput::EndRun(const SwTextNode* /*pNode*/, sal_Int32 /*nPos*/, bool /*bLastRun*/)
{
    m_aRun->append(SAL_NEWLINE_STRING);
    m_aRun.appendAndClear(m_aRunText);

    if (m_bInRuby)
    {
        m_aRun->append(")}}{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " {}}}");
        m_bInRuby = false;
    }

    if (!m_bSingleEmptyRun && m_bInRun)
        m_aRun->append('}');
    m_bInRun = false;
}

void RtfAttributeOutput::StartRunProperties()
{
    OSL_ENSURE(m_aStyles.getLength() == 0, "m_aStyles is not empty");
}

void RtfAttributeOutput::EndRunProperties(const SwRedlineData* /*pRedlineData*/)
{
    const OString aProperties = MoveCharacterProperties(true);
    m_aRun->append(aProperties.getStr());
}

OString RtfAttributeOutput::MoveCharacterProperties(bool aAutoWriteRtlLtr)
{
    const OString aAssocHich = m_aStylesAssocHich.makeStringAndClear();
    const OString aAssocDbch = m_aStylesAssocDbch.makeStringAndClear();
    const OString aAssocRtlch = m_aStylesAssocRtlch.makeStringAndClear();
    const OString aAssocLtrch = m_aStylesAssocLtrch.makeStringAndClear();
    const OString aNormal = m_aStyles.makeStringAndClear();
    OStringBuffer aBuf;

    if (aAutoWriteRtlLtr && !m_bControlLtrRtl)
    {
        m_bControlLtrRtl = !aAssocRtlch.isEmpty();
        m_bIsRTL = false;
        m_nScript = i18n::ScriptType::LATIN;
    }

    if (m_bIsRTL)
    {
        if (!aAssocRtlch.isEmpty())
        {
            aBuf.append(OOO_STRING_SVTOOLS_RTF_LTRCH)
                .append(aAssocLtrch)
                .append(' ')
                .append(OOO_STRING_SVTOOLS_RTF_RTLCH)
                .append(aAssocRtlch);
        }
    }
    else
    {
        if (!aAssocRtlch.isEmpty())
        {
            aBuf.append(OOO_STRING_SVTOOLS_RTF_RTLCH)
                .append(aAssocRtlch)
                .append(' ')
                .append(OOO_STRING_SVTOOLS_RTF_LTRCH)
                .append(aAssocLtrch);
        }
        if (!aAssocHich.isEmpty())
        {
            aBuf.append(OOO_STRING_SVTOOLS_RTF_HICH).append(aAssocHich);
        }
        if (!aNormal.isEmpty())
        {
            aBuf.append(OOO_STRING_SVTOOLS_RTF_LOCH).append(aNormal);
        }
        if (!aAssocDbch.isEmpty())
        {
            aBuf.append(OOO_STRING_SVTOOLS_RTF_DBCH).append(aAssocDbch);
        }
    }

    if (m_bControlLtrRtl)
    {
        m_bControlLtrRtl = false;

        switch (m_nScript)
        {
            case i18n::ScriptType::LATIN:
                aBuf.append(OOO_STRING_SVTOOLS_RTF_LOCH);
                break;
            case i18n::ScriptType::ASIAN:
                aBuf.append(OOO_STRING_SVTOOLS_RTF_DBCH);
                break;
            case i18n::ScriptType::COMPLEX:
                /* noop */
            default:
                /* should not happen? */
                break;
        }
    }

    return aBuf.makeStringAndClear();
}

void RtfAttributeOutput::RunText(const OUString& rText, rtl_TextEncoding /*eCharSet*/)
{
    SAL_INFO("sw.rtf", __func__ << ", rText: " << rText);
    RawText(rText, m_rExport.GetCurrentEncoding());
}

OStringBuffer& RtfAttributeOutput::RunText() { return m_aRunText.getLastBuffer(); }

void RtfAttributeOutput::RawText(const OUString& rText, rtl_TextEncoding eCharSet)
{
    m_aRunText->append(msfilter::rtfutil::OutString(rText, eCharSet));
}

void RtfAttributeOutput::StartRuby(const SwTextNode& rNode, sal_Int32 /*nPos*/,
                                   const SwFormatRuby& rRuby)
{
    WW8Ruby aWW8Ruby(rNode, rRuby, GetExport());
    OUString aStr(FieldString(ww::eEQ) + "\\* jc");
    aStr += OUString::number(aWW8Ruby.GetJC()) + " \\* \"Font:";
    aStr += aWW8Ruby.GetFontFamily() + "\" \\* hps";
    aStr += OUString::number((aWW8Ruby.GetRubyHeight() + 5) / 10) + " \\o";
    if (aWW8Ruby.GetDirective())
    {
        aStr += "\\a" + OUStringChar(aWW8Ruby.GetDirective());
    }
    aStr += "(\\s\\up " + OUString::number((aWW8Ruby.GetBaseHeight() + 10) / 20 - 1) + "(";
    m_rExport.OutputField(nullptr, ww::eEQ, aStr, FieldFlags::Start | FieldFlags::CmdStart);
    aStr = rRuby.GetText() + "),";
    m_rExport.OutputField(nullptr, ww::eEQ, aStr, FieldFlags::NONE);
    m_bInRuby = true;
}

void RtfAttributeOutput::EndRuby(const SwTextNode& /*rNode*/, sal_Int32 /*nPos*/) {}

bool RtfAttributeOutput::StartURL(const OUString& rUrl, const OUString& rTarget)
{
    m_aURLs.push(rUrl);
    // Ignore hyperlink without a URL.
    if (!rUrl.isEmpty())
    {
        m_aRun->append('{');
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_FIELD);
        m_aRun->append('{');
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_IGNORE);
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_FLDINST);
        m_aRun->append(" HYPERLINK ");

        m_aRun->append("\"");
        m_aRun->append(msfilter::rtfutil::OutString(rUrl, m_rExport.GetCurrentEncoding()));
        m_aRun->append("\" ");

        if (!rTarget.isEmpty())
        {
            m_aRun->append("\\\\t \"");
            m_aRun->append(msfilter::rtfutil::OutString(rTarget, m_rExport.GetCurrentEncoding()));
            m_aRun->append("\" ");
        }

        m_aRun->append("}");
        m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " {");
    }
    return true;
}

bool RtfAttributeOutput::EndURL(bool const isAtEndOfParagraph)
{
    if (m_aURLs.empty())
    {
        return true;
    }

    const OUString& rURL = m_aURLs.top();
    if (!rURL.isEmpty())
    {
        // UGLY: usually EndRun is called earlier, but there is an extra
        // call to OutAttrWithRange() when at the end of the paragraph,
        // so in that special case the output needs to be appended to the
        // new run's text instead of the previous run
        if (isAtEndOfParagraph)
        {
            // close the fldrslt group
            m_aRunText->append("}}");
            // close the field group
            m_aRunText->append('}');
        }
        else
        {
            // close the fldrslt group
            m_aRun->append("}}");
            // close the field group
            m_aRun->append('}');
        }
    }
    m_aURLs.pop();
    return true;
}

void RtfAttributeOutput::FieldVanish(const OUString& /*rText*/, ww::eField /*eType*/)
{
    SAL_INFO("sw.rtf", "TODO: " << __func__);
}

void RtfAttributeOutput::Redline(const SwRedlineData* pRedline)
{
    if (!pRedline)
        return;

    if (pRedline->GetType() == RedlineType::Insert)
    {
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_REVISED);
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_REVAUTH);
        m_aRun->append(static_cast<sal_Int32>(
            m_rExport.GetRedline(SW_MOD()->GetRedlineAuthor(pRedline->GetAuthor()))));
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_REVDTTM);
    }
    else if (pRedline->GetType() == RedlineType::Delete)
    {
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_DELETED);
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_REVAUTHDEL);
        m_aRun->append(static_cast<sal_Int32>(
            m_rExport.GetRedline(SW_MOD()->GetRedlineAuthor(pRedline->GetAuthor()))));
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_REVDTTMDEL);
    }
    m_aRun->append(static_cast<sal_Int32>(sw::ms::DateTime2DTTM(pRedline->GetTimeStamp())));
    m_aRun->append(' ');
}

void RtfAttributeOutput::FormatDrop(const SwTextNode& /*rNode*/,
                                    const SwFormatDrop& /*rSwFormatDrop*/, sal_uInt16 /*nStyle*/,
                                    ww8::WW8TableNodeInfo::Pointer_t /*pTextNodeInfo*/,
                                    ww8::WW8TableNodeInfoInner::Pointer_t /*pTextNodeInfoInner*/)
{
    SAL_INFO("sw.rtf", "TODO: " << __func__);
}

void RtfAttributeOutput::ParagraphStyle(sal_uInt16 nStyle)
{
    OString* pStyle = m_rExport.GetStyle(nStyle);
    OStringBuffer aStyle;
    aStyle.append(OOO_STRING_SVTOOLS_RTF_S);
    aStyle.append(static_cast<sal_Int32>(nStyle));
    if (pStyle)
        aStyle.append(pStyle->getStr());
    if (!m_bBufferSectionHeaders)
        m_rExport.Strm().WriteOString(aStyle.makeStringAndClear());
    else
        m_aSectionHeaders.append(aStyle.makeStringAndClear());
}

void RtfAttributeOutput::TableInfoCell(
    ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/)
{
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_INTBL);
    if (m_nTableDepth > 1)
    {
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ITAP);
        m_aStyles.append(static_cast<sal_Int32>(m_nTableDepth));
    }
    m_bWroteCellInfo = true;
}

void RtfAttributeOutput::TableInfoRow(ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfo*/)
{
    /* noop */
}

void RtfAttributeOutput::TableDefinition(
    ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner)
{
    InitTableHelper(pTableTextNodeInfoInner);

    const SwTable* pTable = pTableTextNodeInfoInner->getTable();
    SwFrameFormat* pFormat = pTable->GetFrameFormat();

    m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_TROWD);
    TableOrientation(pTableTextNodeInfoInner);
    TableBidi(pTableTextNodeInfoInner);
    TableHeight(pTableTextNodeInfoInner);
    TableCanSplit(pTableTextNodeInfoInner);

    // Cell margins
    const SvxBoxItem& rBox = pFormat->GetBox();
    static const SvxBoxItemLine aBorders[] = { SvxBoxItemLine::TOP, SvxBoxItemLine::LEFT,
                                               SvxBoxItemLine::BOTTOM, SvxBoxItemLine::RIGHT };

    static const char* aRowPadNames[]
        = { OOO_STRING_SVTOOLS_RTF_TRPADDT, OOO_STRING_SVTOOLS_RTF_TRPADDL,
            OOO_STRING_SVTOOLS_RTF_TRPADDB, OOO_STRING_SVTOOLS_RTF_TRPADDR };

    static const char* aRowPadUnits[]
        = { OOO_STRING_SVTOOLS_RTF_TRPADDFT, OOO_STRING_SVTOOLS_RTF_TRPADDFL,
            OOO_STRING_SVTOOLS_RTF_TRPADDFB, OOO_STRING_SVTOOLS_RTF_TRPADDFR };

    for (int i = 0; i < 4; ++i)
    {
        m_aRowDefs.append(aRowPadUnits[i]);
        m_aRowDefs.append(sal_Int32(3));
        m_aRowDefs.append(aRowPadNames[i]);
        m_aRowDefs.append(static_cast<sal_Int32>(rBox.GetDistance(aBorders[i])));
    }

    // The cell-dependent properties
    const double fWidthRatio = m_pTableWrt->GetAbsWidthRatio();
    const SwWriteTableRows& aRows = m_pTableWrt->GetRows();
    SwWriteTableRow* pRow = aRows[pTableTextNodeInfoInner->getRow()].get();
    SwTwips nSz = 0;

    // Not using m_nTableDepth, which is not yet incremented here.
    sal_uInt32 nCurrentDepth = pTableTextNodeInfoInner->getDepth();
    m_aCells[nCurrentDepth] = pRow->GetCells().size();
    for (sal_uInt32 i = 0; i < m_aCells[nCurrentDepth]; i++)
    {
        const SwWriteTableCell* const pCell = pRow->GetCells()[i].get();
        const SwFrameFormat* pCellFormat = pCell->GetBox()->GetFrameFormat();

        pTableTextNodeInfoInner->setCell(i);
        TableCellProperties(pTableTextNodeInfoInner);

        // Right boundary: this can't be in TableCellProperties as the old
        // value of nSz is needed.
        nSz += pCellFormat->GetFrameSize().GetWidth();
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CELLX);
        m_aRowDefs.append(static_cast<sal_Int32>(pFormat->GetLRSpace().GetLeft()
                                                 + rtl::math::round(nSz * fWidthRatio)));
    }
}

void RtfAttributeOutput::TableDefaultBorders(
    ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner)
{
    /*
     * The function name is a bit misleading: given that we write borders
     * before each row, we just have borders, not default ones. Additionally,
     * this function actually writes borders for a specific cell only and is
     * called for each cell.
     */

    const SwWriteTableRows& aRows = m_pTableWrt->GetRows();
    SwWriteTableRow* pRow = aRows[pTableTextNodeInfoInner->getRow()].get();
    const SwWriteTableCell* const pCell
        = pRow->GetCells()[pTableTextNodeInfoInner->getCell()].get();
    const SwFrameFormat* pCellFormat = pCell->GetBox()->GetFrameFormat();
    const SfxPoolItem* pItem;
    if (!pCellFormat->GetAttrSet().HasItem(RES_BOX, &pItem))
        return;

    auto& rBox = static_cast<const SvxBoxItem&>(*pItem);
    static const SvxBoxItemLine aBorders[] = { SvxBoxItemLine::TOP, SvxBoxItemLine::LEFT,
                                               SvxBoxItemLine::BOTTOM, SvxBoxItemLine::RIGHT };
    static const char* aBorderNames[]
        = { OOO_STRING_SVTOOLS_RTF_CLBRDRT, OOO_STRING_SVTOOLS_RTF_CLBRDRL,
            OOO_STRING_SVTOOLS_RTF_CLBRDRB, OOO_STRING_SVTOOLS_RTF_CLBRDRR };
    //Yes left and top are swapped with each other for cell padding! Because
    //that's what the thundering annoying rtf export/import word xp does.
    static const char* aCellPadNames[]
        = { OOO_STRING_SVTOOLS_RTF_CLPADL, OOO_STRING_SVTOOLS_RTF_CLPADT,
            OOO_STRING_SVTOOLS_RTF_CLPADB, OOO_STRING_SVTOOLS_RTF_CLPADR };
    static const char* aCellPadUnits[]
        = { OOO_STRING_SVTOOLS_RTF_CLPADFL, OOO_STRING_SVTOOLS_RTF_CLPADFT,
            OOO_STRING_SVTOOLS_RTF_CLPADFB, OOO_STRING_SVTOOLS_RTF_CLPADFR };
    for (int i = 0; i < 4; ++i)
    {
        if (const editeng::SvxBorderLine* pLn = rBox.GetLine(aBorders[i]))
            m_aRowDefs.append(OutTBLBorderLine(m_rExport, pLn, aBorderNames[i]));
        if (rBox.GetDistance(aBorders[i]))
        {
            m_aRowDefs.append(aCellPadUnits[i]);
            m_aRowDefs.append(sal_Int32(3));
            m_aRowDefs.append(aCellPadNames[i]);
            m_aRowDefs.append(static_cast<sal_Int32>(rBox.GetDistance(aBorders[i])));
        }
    }
}

void RtfAttributeOutput::TableBackgrounds(
    ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner)
{
    const SwTable* pTable = pTableTextNodeInfoInner->getTable();
    const SwTableBox* pTableBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine* pTableLine = pTableBox->GetUpper();

    Color aColor = COL_AUTO;
    auto pTableColorProp
        = pTable->GetFrameFormat()->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    if (pTableColorProp)
        aColor = pTableColorProp->GetColor();

    auto pRowColorProp
        = pTableLine->GetFrameFormat()->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    if (pRowColorProp && pRowColorProp->GetColor() != COL_AUTO)
        aColor = pRowColorProp->GetColor();

    const SwWriteTableRows& aRows = m_pTableWrt->GetRows();
    SwWriteTableRow* pRow = aRows[pTableTextNodeInfoInner->getRow()].get();
    const SwWriteTableCell* const pCell
        = pRow->GetCells()[pTableTextNodeInfoInner->getCell()].get();
    const SwFrameFormat* pCellFormat = pCell->GetBox()->GetFrameFormat();
    const SfxPoolItem* pItem;
    if (pCellFormat->GetAttrSet().HasItem(RES_BACKGROUND, &pItem))
    {
        auto& rBack = static_cast<const SvxBrushItem&>(*pItem);
        if (rBack.GetColor() != COL_AUTO)
            aColor = rBack.GetColor();
    }

    if (!aColor.GetTransparency())
    {
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLCBPAT);
        m_aRowDefs.append(static_cast<sal_Int32>(m_rExport.GetColor(aColor)));
    }
}

void RtfAttributeOutput::TableRowRedline(
    ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/)
{
}

void RtfAttributeOutput::TableCellRedline(
    ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/)
{
}

void RtfAttributeOutput::TableHeight(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner)
{
    const SwTableBox* pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine* pTabLine = pTabBox->GetUpper();
    const SwFrameFormat* pLineFormat = pTabLine->GetFrameFormat();
    const SwFormatFrameSize& rLSz = pLineFormat->GetFrameSize();

    if (!(SwFrameSize::Variable != rLSz.GetHeightSizeType() && rLSz.GetHeight()))
        return;

    sal_Int32 nHeight = 0;

    switch (rLSz.GetHeightSizeType())
    {
        case SwFrameSize::Fixed:
            nHeight = -rLSz.GetHeight();
            break;
        case SwFrameSize::Minimum:
            nHeight = rLSz.GetHeight();
            break;
        default:
            break;
    }

    if (nHeight)
    {
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_TRRH);
        m_aRowDefs.append(nHeight);
    }
}

void RtfAttributeOutput::TableCanSplit(
    ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner)
{
    const SwTableBox* pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine* pTabLine = pTabBox->GetUpper();
    const SwFrameFormat* pLineFormat = pTabLine->GetFrameFormat();
    const SwFormatRowSplit& rSplittable = pLineFormat->GetRowSplit();

    // The rtf default is to allow a row to break
    if (!rSplittable.GetValue())
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_TRKEEP);
}

void RtfAttributeOutput::TableBidi(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner)
{
    const SwTable* pTable = pTableTextNodeInfoInner->getTable();
    const SwFrameFormat* pFrameFormat = pTable->GetFrameFormat();

    if (m_rExport.TrueFrameDirection(*pFrameFormat) != SvxFrameDirection::Horizontal_RL_TB)
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_LTRROW);
    else
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_RTLROW);
}

void RtfAttributeOutput::TableVerticalCell(
    ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner)
{
    const SwWriteTableRows& aRows = m_pTableWrt->GetRows();
    SwWriteTableRow* pRow = aRows[pTableTextNodeInfoInner->getRow()].get();
    const SwWriteTableCell* const pCell
        = pRow->GetCells()[pTableTextNodeInfoInner->getCell()].get();
    const SwFrameFormat* pCellFormat = pCell->GetBox()->GetFrameFormat();

    // Text direction.
    if (SvxFrameDirection::Vertical_RL_TB == m_rExport.TrueFrameDirection(*pCellFormat))
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLTXTBRL);
    else if (SvxFrameDirection::Vertical_LR_BT == m_rExport.TrueFrameDirection(*pCellFormat))
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLTXBTLR);

    const SfxPoolItem* pItem;

    // vertical merges
    if (pCell->GetRowSpan() > 1)
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLVMGF);
    else if (pCell->GetRowSpan() == 0)
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLVMRG);

    // vertical alignment
    if (!pCellFormat->GetAttrSet().HasItem(RES_VERT_ORIENT, &pItem))
        return;

    switch (static_cast<const SwFormatVertOrient*>(pItem)->GetVertOrient())
    {
        case text::VertOrientation::CENTER:
            m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLVERTALC);
            break;
        case text::VertOrientation::BOTTOM:
            m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLVERTALB);
            break;
        default:
            m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLVERTALT);
            break;
    }
}

void RtfAttributeOutput::TableNodeInfoInner(ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner)
{
    // This is called when the nested table ends in a cell, and there's no
    // paragraph behind that; so we must check for the ends of cell, rows,
    // and tables
    FinishTableRowCell(pNodeInfoInner);
}

void RtfAttributeOutput::TableOrientation(
    ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner)
{
    const SwTable* pTable = pTableTextNodeInfoInner->getTable();
    SwFrameFormat* pFormat = pTable->GetFrameFormat();

    OStringBuffer aTableAdjust(OOO_STRING_SVTOOLS_RTF_TRQL);
    switch (pFormat->GetHoriOrient().GetHoriOrient())
    {
        case text::HoriOrientation::CENTER:
            aTableAdjust.setLength(0);
            aTableAdjust.append(OOO_STRING_SVTOOLS_RTF_TRQC);
            break;
        case text::HoriOrientation::RIGHT:
            aTableAdjust.setLength(0);
            aTableAdjust.append(OOO_STRING_SVTOOLS_RTF_TRQR);
            break;
        case text::HoriOrientation::NONE:
        case text::HoriOrientation::LEFT_AND_WIDTH:
            aTableAdjust.append(OOO_STRING_SVTOOLS_RTF_TRLEFT);
            aTableAdjust.append(static_cast<sal_Int32>(pFormat->GetLRSpace().GetLeft()));
            break;
        default:
            break;
    }

    m_aRowDefs.append(aTableAdjust.makeStringAndClear());
}

void RtfAttributeOutput::TableSpacing(
    ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/)
{
    SAL_INFO("sw.rtf", "TODO: " << __func__);
}

void RtfAttributeOutput::TableRowEnd(sal_uInt32 /*nDepth*/) { /* noop, see EndTableRow() */}

/*
 * Our private table methods.
 */

void RtfAttributeOutput::InitTableHelper(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pTableTextNodeInfoInner)
{
    const SwTable* pTable = pTableTextNodeInfoInner->getTable();
    if (m_pTableWrt && pTable == m_pTableWrt->GetTable())
        return;

    tools::Long nPageSize = 0;
    bool bRelBoxSize = false;

    // Create the SwWriteTable instance to use col spans
    GetTablePageSize(pTableTextNodeInfoInner.get(), nPageSize, bRelBoxSize);

    const SwFrameFormat* pFormat = pTable->GetFrameFormat();
    const sal_uInt32 nTableSz = pFormat->GetFrameSize().GetWidth();

    const SwHTMLTableLayout* pLayout = pTable->GetHTMLTableLayout();
    if (pLayout && pLayout->IsExportable())
        m_pTableWrt = std::make_unique<SwWriteTable>(pTable, pLayout);
    else
        m_pTableWrt = std::make_unique<SwWriteTable>(pTable, pTable->GetTabLines(), nPageSize,
                                                     nTableSz, false);
}

void RtfAttributeOutput::StartTable()
{
    // To trigger calling InitTableHelper()
    m_pTableWrt.reset();
}

void RtfAttributeOutput::StartTableRow(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pTableTextNodeInfoInner)
{
    sal_uInt32 nCurrentDepth = pTableTextNodeInfoInner->getDepth();
    SAL_INFO("sw.rtf", __func__ << ", (depth is " << nCurrentDepth << ")");
    m_bTableRowEnded = false;

    TableDefinition(pTableTextNodeInfoInner);

    if (!m_bLastTable)
        m_aTables.push_back(m_aRowDefs.makeStringAndClear());

    // We'll write the table definition for nested tables later
    if (nCurrentDepth > 1)
        return;
    // Empty the previous row closing buffer before starting the new one,
    // necessary for subtables.
    m_rExport.Strm().WriteOString(m_aAfterRuns.makeStringAndClear());
    m_rExport.Strm().WriteOString(m_aRowDefs.makeStringAndClear());
}

void RtfAttributeOutput::StartTableCell() { m_bTableCellOpen = true; }

void RtfAttributeOutput::TableCellProperties(
    const ww8::WW8TableNodeInfoInner::Pointer_t& pTableTextNodeInfoInner)
{
    TableDefaultBorders(pTableTextNodeInfoInner);
    TableBackgrounds(pTableTextNodeInfoInner);
    TableVerticalCell(pTableTextNodeInfoInner);
}

void RtfAttributeOutput::EndTableCell()
{
    SAL_INFO("sw.rtf", __func__ << ", (depth is " << m_nTableDepth << ")");

    if (!m_bWroteCellInfo)
    {
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_INTBL);
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_ITAP);
        m_aAfterRuns.append(static_cast<sal_Int32>(m_nTableDepth));
    }
    if (m_nTableDepth > 1)
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_NESTCELL);
    else
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_CELL);

    m_bTableCellOpen = false;
    m_bTableAfterCell = true;
    m_bWroteCellInfo = false;
    if (m_aCells[m_nTableDepth] > 0)
        m_aCells[m_nTableDepth]--;
}

void RtfAttributeOutput::EndTableRow()
{
    SAL_INFO("sw.rtf", __func__ << ", (depth is " << m_nTableDepth << ")");

    // Trying to end the row without writing the required number of cells? Fill with empty ones.
    for (sal_uInt32 i = 0; i < m_aCells[m_nTableDepth]; i++)
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_CELL);

    if (m_nTableDepth > 1)
    {
        m_aAfterRuns.append(
            "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_NESTTABLEPROPRS);
        if (!m_aRowDefs.isEmpty())
            m_aAfterRuns.append(m_aRowDefs.makeStringAndClear());
        else if (!m_aTables.empty())
        {
            m_aAfterRuns.append(m_aTables.back());
            m_aTables.pop_back();
        }
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_NESTROW
                            "}"
                            "{" OOO_STRING_SVTOOLS_RTF_NONESTTABLES OOO_STRING_SVTOOLS_RTF_PAR "}");
    }
    else
    {
        if (!m_aTables.empty())
        {
            m_aAfterRuns.append(m_aTables.back());
            m_aTables.pop_back();
        }
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_ROW).append(OOO_STRING_SVTOOLS_RTF_PARD);
    }
    m_bTableRowEnded = true;
}

void RtfAttributeOutput::EndTable()
{
    if (m_nTableDepth > 0)
    {
        m_nTableDepth--;
        m_pTableWrt.reset();
    }

    // We closed the table; if it is a nested table, the cell that contains it
    // still continues
    m_bTableCellOpen = true;

    // Cleans the table helper
    m_pTableWrt.reset();
}

void RtfAttributeOutput::FinishTableRowCell(const ww8::WW8TableNodeInfoInner::Pointer_t& pInner)
{
    if (!pInner)
        return;

    // Where are we in the table
    sal_uInt32 nRow = pInner->getRow();

    const SwTable* pTable = pInner->getTable();
    const SwTableLines& rLines = pTable->GetTabLines();
    sal_uInt16 nLinesCount = rLines.size();

    if (pInner->isEndOfCell())
        EndTableCell();

    // This is a line end
    if (pInner->isEndOfLine())
        EndTableRow();

    // This is the end of the table
    if (pInner->isEndOfLine() && (nRow + 1) == nLinesCount)
        EndTable();
}

void RtfAttributeOutput::StartStyles()
{
    m_rExport.Strm()
        .WriteCharPtr(SAL_NEWLINE_STRING)
        .WriteChar('{')
        .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_COLORTBL);
    m_rExport.OutColorTable();
    OSL_ENSURE(m_aStylesheet.getLength() == 0, "m_aStylesheet is not empty");
    m_aStylesheet.append(SAL_NEWLINE_STRING);
    m_aStylesheet.append('{');
    m_aStylesheet.append(OOO_STRING_SVTOOLS_RTF_STYLESHEET);
}

void RtfAttributeOutput::EndStyles(sal_uInt16 /*nNumberOfStyles*/)
{
    m_rExport.Strm().WriteChar('}');
    m_rExport.Strm().WriteOString(m_aStylesheet.makeStringAndClear());
    m_rExport.Strm().WriteChar('}');
}

void RtfAttributeOutput::DefaultStyle() { /* noop, the default style is always 0 in RTF */}

void RtfAttributeOutput::StartStyle(const OUString& rName, StyleType eType, sal_uInt16 nBase,
                                    sal_uInt16 nNext, sal_uInt16 /*nWwId*/, sal_uInt16 nId,
                                    bool bAutoUpdate)
{
    SAL_INFO("sw.rtf", __func__ << ", rName = '" << rName << "'");

    m_aStylesheet.append('{');
    if (eType == STYLE_TYPE_PARA)
        m_aStylesheet.append(OOO_STRING_SVTOOLS_RTF_S);
    else
        m_aStylesheet.append(OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_CS);
    m_aStylesheet.append(static_cast<sal_Int32>(nId));

    if (nBase != 0x0FFF)
    {
        m_aStylesheet.append(OOO_STRING_SVTOOLS_RTF_SBASEDON);
        m_aStylesheet.append(static_cast<sal_Int32>(nBase));
    }

    m_aStylesheet.append(OOO_STRING_SVTOOLS_RTF_SNEXT);
    m_aStylesheet.append(static_cast<sal_Int32>(nNext));

    if (bAutoUpdate)
        m_aStylesheet.append(OOO_STRING_SVTOOLS_RTF_SAUTOUPD);

    m_rStyleName = rName;
    m_nStyleId = nId;
}

void RtfAttributeOutput::EndStyle()
{
    OString aStyles = MoveCharacterProperties();
    m_rExport.InsStyle(m_nStyleId, aStyles);
    m_aStylesheet.append(aStyles);
    m_aStylesheet.append(' ');
    m_aStylesheet.append(
        msfilter::rtfutil::OutString(m_rStyleName, m_rExport.GetCurrentEncoding()));
    m_aStylesheet.append(";}");
    m_aStylesheet.append(SAL_NEWLINE_STRING);
}

void RtfAttributeOutput::StartStyleProperties(bool /*bParProp*/, sal_uInt16 /*nStyle*/)
{
    /* noop */
}

void RtfAttributeOutput::EndStyleProperties(bool /*bParProp*/) { /* noop */}

void RtfAttributeOutput::OutlineNumbering(sal_uInt8 nLvl)
{
    if (nLvl >= WW8ListManager::nMaxLevel)
        nLvl = WW8ListManager::nMaxLevel - 1;

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ILVL);
    m_aStyles.append(static_cast<sal_Int32>(nLvl));
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_OUTLINELEVEL);
    m_aStyles.append(static_cast<sal_Int32>(nLvl));
}

void RtfAttributeOutput::PageBreakBefore(bool bBreak)
{
    if (bBreak)
    {
        m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PAGEBB);
    }
}

void RtfAttributeOutput::SectionBreak(sal_uInt8 nC, bool /*bBreakAfter*/,
                                      const WW8_SepInfo* pSectionInfo)
{
    switch (nC)
    {
        case msword::ColumnBreak:
            m_nColBreakNeeded = true;
            break;
        case msword::PageBreak:
            if (pSectionInfo)
                m_rExport.SectionProperties(*pSectionInfo);
            break;
    }
}

void RtfAttributeOutput::StartSection()
{
    if (m_bIsBeforeFirstParagraph)
        return;

    m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_SECT OOO_STRING_SVTOOLS_RTF_SECTD);
    if (!m_bBufferSectionBreaks)
        m_rExport.Strm().WriteOString(m_aSectionBreaks.makeStringAndClear());
}

void RtfAttributeOutput::EndSection()
{
    /*
     * noop, \sect must go to StartSection or Word won't notice multiple
     * columns...
     */
}

void RtfAttributeOutput::SectionFormProtection(bool bProtected)
{
    m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_SECTUNLOCKED);
    m_aSectionBreaks.append(static_cast<sal_Int32>(!bProtected));
}

void RtfAttributeOutput::SectionLineNumbering(sal_uLong nRestartNo,
                                              const SwLineNumberInfo& rLnNumInfo)
{
    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LINEMOD);
    m_rExport.OutLong(rLnNumInfo.GetCountBy());
    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LINEX);
    m_rExport.OutLong(rLnNumInfo.GetPosFromLeft());
    if (!rLnNumInfo.IsRestartEachPage())
        m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LINECONT);

    if (nRestartNo > 0)
    {
        m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LINESTARTS);
        m_rExport.OutLong(nRestartNo);
    }
}

void RtfAttributeOutput::SectionTitlePage()
{
    /*
     * noop, handled in RtfExport::WriteHeaderFooter()
     */
}

void RtfAttributeOutput::SectionPageBorders(const SwFrameFormat* pFormat,
                                            const SwFrameFormat* /*pFirstPageFormat*/)
{
    const SvxBoxItem& rBox = pFormat->GetBox();
    const editeng::SvxBorderLine* pLine = rBox.GetTop();
    if (pLine)
        m_aSectionBreaks.append(OutBorderLine(m_rExport, pLine, OOO_STRING_SVTOOLS_RTF_PGBRDRT,
                                              rBox.GetDistance(SvxBoxItemLine::TOP)));
    pLine = rBox.GetBottom();
    if (pLine)
        m_aSectionBreaks.append(OutBorderLine(m_rExport, pLine, OOO_STRING_SVTOOLS_RTF_PGBRDRB,
                                              rBox.GetDistance(SvxBoxItemLine::BOTTOM)));
    pLine = rBox.GetLeft();
    if (pLine)
        m_aSectionBreaks.append(OutBorderLine(m_rExport, pLine, OOO_STRING_SVTOOLS_RTF_PGBRDRL,
                                              rBox.GetDistance(SvxBoxItemLine::LEFT)));
    pLine = rBox.GetRight();
    if (pLine)
        m_aSectionBreaks.append(OutBorderLine(m_rExport, pLine, OOO_STRING_SVTOOLS_RTF_PGBRDRR,
                                              rBox.GetDistance(SvxBoxItemLine::RIGHT)));
}

void RtfAttributeOutput::SectionBiDi(bool bBiDi)
{
    m_rExport.Strm().WriteCharPtr(bBiDi ? OOO_STRING_SVTOOLS_RTF_RTLSECT
                                        : OOO_STRING_SVTOOLS_RTF_LTRSECT);
}

void RtfAttributeOutput::SectionPageNumbering(sal_uInt16 nNumType,
                                              const ::std::optional<sal_uInt16>& oPageRestartNumber)
{
    if (oPageRestartNumber)
    {
        m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_PGNSTARTS);
        m_aSectionBreaks.append(static_cast<sal_Int32>(*oPageRestartNumber));
        m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_PGNRESTART);
    }

    const char* pStr = nullptr;
    switch (nNumType)
    {
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:
            pStr = OOO_STRING_SVTOOLS_RTF_PGNUCLTR;
            break;
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:
            pStr = OOO_STRING_SVTOOLS_RTF_PGNLCLTR;
            break;
        case SVX_NUM_ROMAN_UPPER:
            pStr = OOO_STRING_SVTOOLS_RTF_PGNUCRM;
            break;
        case SVX_NUM_ROMAN_LOWER:
            pStr = OOO_STRING_SVTOOLS_RTF_PGNLCRM;
            break;

        case SVX_NUM_ARABIC:
            pStr = OOO_STRING_SVTOOLS_RTF_PGNDEC;
            break;
    }
    if (pStr)
        m_aSectionBreaks.append(pStr);
}

void RtfAttributeOutput::SectionType(sal_uInt8 nBreakCode)
{
    SAL_INFO("sw.rtf", __func__ << ", nBreakCode = " << int(nBreakCode));

    /*
     * break code:   0 No break, 1 New column
     * 2 New page, 3 Even page, 4 Odd page
     */
    const char* sType = nullptr;
    switch (nBreakCode)
    {
        case 1:
            sType = OOO_STRING_SVTOOLS_RTF_SBKCOL;
            break;
        case 2:
            sType = OOO_STRING_SVTOOLS_RTF_SBKPAGE;
            break;
        case 3:
            sType = OOO_STRING_SVTOOLS_RTF_SBKEVEN;
            break;
        case 4:
            sType = OOO_STRING_SVTOOLS_RTF_SBKODD;
            break;
        default:
            sType = OOO_STRING_SVTOOLS_RTF_SBKNONE;
            break;
    }
    m_aSectionBreaks.append(sType);
    if (!m_bBufferSectionBreaks)
        m_rExport.Strm().WriteOString(m_aSectionBreaks.makeStringAndClear());
}

void RtfAttributeOutput::SectFootnoteEndnotePr()
{
    WriteFootnoteEndnotePr(true, m_rExport.m_rDoc.GetFootnoteInfo());
    WriteFootnoteEndnotePr(false, m_rExport.m_rDoc.GetEndNoteInfo());
}

void RtfAttributeOutput::WriteFootnoteEndnotePr(bool bFootnote, const SwEndNoteInfo& rInfo)
{
    const char* pOut = nullptr;

    if (bFootnote)
    {
        switch (rInfo.m_aFormat.GetNumberingType())
        {
            default:
                pOut = OOO_STRING_SVTOOLS_RTF_SFTNNAR;
                break;
            case SVX_NUM_CHARS_LOWER_LETTER:
            case SVX_NUM_CHARS_LOWER_LETTER_N:
                pOut = OOO_STRING_SVTOOLS_RTF_SFTNNALC;
                break;
            case SVX_NUM_CHARS_UPPER_LETTER:
            case SVX_NUM_CHARS_UPPER_LETTER_N:
                pOut = OOO_STRING_SVTOOLS_RTF_SFTNNAUC;
                break;
            case SVX_NUM_ROMAN_LOWER:
                pOut = OOO_STRING_SVTOOLS_RTF_SFTNNRLC;
                break;
            case SVX_NUM_ROMAN_UPPER:
                pOut = OOO_STRING_SVTOOLS_RTF_SFTNNRUC;
                break;
            case SVX_NUM_SYMBOL_CHICAGO:
                pOut = OOO_STRING_SVTOOLS_RTF_SFTNNCHI;
                break;
        }
    }
    else
    {
        switch (rInfo.m_aFormat.GetNumberingType())
        {
            default:
                pOut = OOO_STRING_SVTOOLS_RTF_SAFTNNAR;
                break;
            case SVX_NUM_CHARS_LOWER_LETTER:
            case SVX_NUM_CHARS_LOWER_LETTER_N:
                pOut = OOO_STRING_SVTOOLS_RTF_SAFTNNALC;
                break;
            case SVX_NUM_CHARS_UPPER_LETTER:
            case SVX_NUM_CHARS_UPPER_LETTER_N:
                pOut = OOO_STRING_SVTOOLS_RTF_SAFTNNAUC;
                break;
            case SVX_NUM_ROMAN_LOWER:
                pOut = OOO_STRING_SVTOOLS_RTF_SAFTNNRLC;
                break;
            case SVX_NUM_ROMAN_UPPER:
                pOut = OOO_STRING_SVTOOLS_RTF_SAFTNNRUC;
                break;
            case SVX_NUM_SYMBOL_CHICAGO:
                pOut = OOO_STRING_SVTOOLS_RTF_SAFTNNCHI;
                break;
        }
    }

    m_aSectionBreaks.append(pOut);

    if (!m_bBufferSectionBreaks)
    {
        m_rExport.Strm().WriteOString(m_aSectionBreaks.makeStringAndClear());
    }
}

void RtfAttributeOutput::NumberingDefinition(sal_uInt16 nId, const SwNumRule& /*rRule*/)
{
    m_rExport.Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LISTOVERRIDE);
    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LISTID);
    m_rExport.OutULong(nId);
    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LISTOVERRIDECOUNT).WriteChar('0');
    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LS);
    m_rExport.OutULong(nId).WriteChar('}');
}

void RtfAttributeOutput::StartAbstractNumbering(sal_uInt16 nId)
{
    m_rExport.Strm()
        .WriteChar('{')
        .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LIST)
        .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LISTTEMPLATEID);
    m_rExport.OutULong(nId);
    m_nListId = nId;
}

void RtfAttributeOutput::EndAbstractNumbering()
{
    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LISTID);
    m_rExport.OutULong(m_nListId).WriteChar('}').WriteCharPtr(SAL_NEWLINE_STRING);
}

void RtfAttributeOutput::NumberingLevel(sal_uInt8 nLevel, sal_uInt16 nStart,
                                        sal_uInt16 nNumberingType, SvxAdjust eAdjust,
                                        const sal_uInt8* pNumLvlPos, sal_uInt8 nFollow,
                                        const wwFont* pFont, const SfxItemSet* pOutSet,
                                        sal_Int16 nIndentAt, sal_Int16 nFirstLineIndex,
                                        sal_Int16 /*nListTabPos*/, const OUString& rNumberingString,
                                        const SvxBrushItem* pBrush)
{
    m_rExport.Strm().WriteCharPtr(SAL_NEWLINE_STRING);
    if (nLevel > 8) // RTF knows only 9 levels
        m_rExport.Strm()
            .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_IGNORE)
            .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SOUTLVL);

    m_rExport.Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LISTLEVEL);

    sal_uInt16 nVal = 0;
    switch (nNumberingType)
    {
        case SVX_NUM_ROMAN_UPPER:
            nVal = 1;
            break;
        case SVX_NUM_ROMAN_LOWER:
            nVal = 2;
            break;
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:
            nVal = 3;
            break;
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:
            nVal = 4;
            break;
        case SVX_NUM_FULL_WIDTH_ARABIC:
            nVal = 14;
            break;
        case SVX_NUM_CIRCLE_NUMBER:
            nVal = 18;
            break;
        case SVX_NUM_NUMBER_LOWER_ZH:
            nVal = 35;
            if (pOutSet)
            {
                const SvxLanguageItem& rLang = pOutSet->Get(RES_CHRATR_CJK_LANGUAGE);
                if (rLang.GetLanguage() == LANGUAGE_CHINESE_SIMPLIFIED)
                {
                    nVal = 39;
                }
            }
            break;
        case SVX_NUM_NUMBER_UPPER_ZH:
            nVal = 38;
            break;
        case SVX_NUM_NUMBER_UPPER_ZH_TW:
            nVal = 34;
            break;
        case SVX_NUM_TIAN_GAN_ZH:
            nVal = 30;
            break;
        case SVX_NUM_DI_ZI_ZH:
            nVal = 31;
            break;
        case SVX_NUM_NUMBER_TRADITIONAL_JA:
            nVal = 16;
            break;
        case SVX_NUM_AIU_FULLWIDTH_JA:
            nVal = 20;
            break;
        case SVX_NUM_AIU_HALFWIDTH_JA:
            nVal = 12;
            break;
        case SVX_NUM_IROHA_FULLWIDTH_JA:
            nVal = 21;
            break;
        case SVX_NUM_IROHA_HALFWIDTH_JA:
            nVal = 13;
            break;
        case style::NumberingType::HANGUL_SYLLABLE_KO:
            nVal = 24;
            break; // ganada
        case style::NumberingType::HANGUL_JAMO_KO:
            nVal = 25;
            break; // chosung
        case style::NumberingType::HANGUL_CIRCLED_SYLLABLE_KO:
            nVal = 24;
            break;
        case style::NumberingType::HANGUL_CIRCLED_JAMO_KO:
            nVal = 25;
            break;
        case style::NumberingType::NUMBER_HANGUL_KO:
            nVal = 41;
            break;
        case style::NumberingType::NUMBER_UPPER_KO:
            nVal = 44;
            break;

        case SVX_NUM_BITMAP:
        case SVX_NUM_CHAR_SPECIAL:
            nVal = 23;
            break;
        case SVX_NUM_NUMBER_NONE:
            nVal = 255;
            break;
        case SVX_NUM_ARABIC_ZERO:
            nVal = 22;
            break;
    }
    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LEVELNFC);
    m_rExport.OutULong(nVal);

    switch (eAdjust)
    {
        case SvxAdjust::Center:
            nVal = 1;
            break;
        case SvxAdjust::Right:
            nVal = 2;
            break;
        default:
            nVal = 0;
            break;
    }
    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LEVELJC);
    m_rExport.OutULong(nVal);

    // bullet
    if (nNumberingType == SVX_NUM_BITMAP && pBrush)
    {
        int nIndex = m_rExport.GetGrfIndex(*pBrush);
        if (nIndex != -1)
        {
            m_rExport.Strm().WriteCharPtr(LO_STRING_SVTOOLS_RTF_LEVELPICTURE);
            m_rExport.OutULong(nIndex);
        }
    }

    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LEVELSTARTAT);
    m_rExport.OutULong(nStart);

    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LEVELFOLLOW);
    m_rExport.OutULong(nFollow);

    // leveltext group
    m_rExport.Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LEVELTEXT).WriteChar(' ');

    if (SVX_NUM_CHAR_SPECIAL == nNumberingType || SVX_NUM_BITMAP == nNumberingType)
    {
        m_rExport.Strm().WriteCharPtr("\\'01");
        sal_Unicode cChar = rNumberingString[0];
        m_rExport.Strm().WriteCharPtr("\\u");
        m_rExport.OutULong(cChar);
        m_rExport.Strm().WriteCharPtr(" ?");
    }
    else
    {
        m_rExport.Strm().WriteCharPtr("\\'").WriteCharPtr(
            msfilter::rtfutil::OutHex(rNumberingString.getLength(), 2).getStr());
        m_rExport.Strm().WriteCharPtr(msfilter::rtfutil::OutString(rNumberingString,
                                                                   m_rExport.GetDefaultEncoding(),
                                                                   /*bUnicode =*/false)
                                          .getStr());
    }

    m_rExport.Strm().WriteCharPtr(";}");

    // write the levelnumbers
    m_rExport.Strm().WriteCharPtr("{").WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LEVELNUMBERS);
    for (sal_uInt8 i = 0; i <= nLevel && pNumLvlPos[i]; ++i)
    {
        m_rExport.Strm().WriteCharPtr("\\'").WriteCharPtr(
            msfilter::rtfutil::OutHex(pNumLvlPos[i], 2).getStr());
    }
    m_rExport.Strm().WriteCharPtr(";}");

    if (pOutSet)
    {
        if (pFont)
        {
            m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_F);
            m_rExport.OutULong(m_rExport.m_aFontHelper.GetId(*pFont));
        }
        m_rExport.OutputItemSet(*pOutSet, false, true, i18n::ScriptType::LATIN,
                                m_rExport.m_bExportModeRTF);
        const OString aProperties = MoveCharacterProperties(true);
        m_rExport.Strm().WriteOString(aProperties);
    }

    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_FI);
    m_rExport.OutLong(nFirstLineIndex).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LI);
    m_rExport.OutLong(nIndentAt);

    m_rExport.Strm().WriteChar('}');
    if (nLevel > 8)
        m_rExport.Strm().WriteChar('}');
}

void RtfAttributeOutput::WriteField_Impl(const SwField* const pField, ww::eField /*eType*/,
                                         const OUString& rFieldCmd, FieldFlags nMode)
{
    // If there are no field instructions, don't export it as a field.
    bool bHasInstructions = !rFieldCmd.isEmpty();
    if (FieldFlags::All == nMode)
    {
        if (bHasInstructions)
        {
            m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_FIELD);
            m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FLDINST
                               " ");
            m_aRunText->append(
                msfilter::rtfutil::OutString(rFieldCmd, m_rExport.GetCurrentEncoding()));
            m_aRunText->append("}{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " ");
        }
        if (pField)
            m_aRunText->append(msfilter::rtfutil::OutString(pField->ExpandField(true, nullptr),
                                                            m_rExport.GetDefaultEncoding()));
        if (bHasInstructions)
            m_aRunText->append("}}");
    }
    else
    {
        if (nMode & FieldFlags::CmdStart)
        {
            m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_FIELD);
            m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FLDINST
                               // paragraph break closes group so open another one "inside" to
                               " {"); // prevent leaving the field instruction
        }
        if (bHasInstructions)
            m_aRunText->append(
                msfilter::rtfutil::OutString(rFieldCmd, m_rExport.GetCurrentEncoding()));
        if (nMode & FieldFlags::CmdEnd)
        {
            m_aRunText->append("}}{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " {");
        }
        if (nMode & FieldFlags::Close)
        {
            m_aRunText->append("}}}");
        }
    }
}

void RtfAttributeOutput::WriteBookmarks_Impl(std::vector<OUString>& rStarts,
                                             std::vector<OUString>& rEnds)
{
    for (const auto& rStart : rStarts)
    {
        m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_BKMKSTART " ");
        m_aRun->append(msfilter::rtfutil::OutString(rStart, m_rExport.GetCurrentEncoding()));
        m_aRun->append('}');
    }
    rStarts.clear();

    for (const auto& rEnd : rEnds)
    {
        m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_BKMKEND " ");
        m_aRun->append(msfilter::rtfutil::OutString(rEnd, m_rExport.GetCurrentEncoding()));
        m_aRun->append('}');
    }
    rEnds.clear();
}

void RtfAttributeOutput::WriteAnnotationMarks_Impl(std::vector<OUString>& rStarts,
                                                   std::vector<OUString>& rEnds)
{
    for (const auto& rStart : rStarts)
    {
        OString rName = OUStringToOString(rStart, RTL_TEXTENCODING_UTF8);

        // Output the annotation mark
        const sal_Int32 nId = m_nNextAnnotationMarkId++;
        m_rOpenedAnnotationMarksIds[rName] = nId;
        m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ATRFSTART " ");
        m_aRun->append(OString::number(nId).getStr());
        m_aRun->append('}');
    }
    rStarts.clear();

    for (const auto& rEnd : rEnds)
    {
        OString rName = OUStringToOString(rEnd, RTL_TEXTENCODING_UTF8);

        // Get the id of the annotation mark
        auto it = m_rOpenedAnnotationMarksIds.find(rName);
        if (it != m_rOpenedAnnotationMarksIds.end())
        {
            const sal_Int32 nId = it->second;
            m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ATRFEND " ");
            m_aRun->append(OString::number(nId).getStr());
            m_aRun->append('}');
            m_rOpenedAnnotationMarksIds.erase(rName);

            if (m_aPostitFields.find(nId) != m_aPostitFields.end())
            {
                m_aRunText->append("{");
                m_nCurrentAnnotationMarkId = nId;
                PostitField(m_aPostitFields[nId]);
                m_nCurrentAnnotationMarkId = -1;
                m_aRunText->append("}");
            }
        }
    }
    rEnds.clear();
}

void RtfAttributeOutput::WriteHeaderFooter_Impl(const SwFrameFormat& rFormat, bool bHeader,
                                                const char* pStr, bool bTitlepg)
{
    OStringBuffer aSectionBreaks = m_aSectionBreaks;
    m_aSectionBreaks.setLength(0);
    RtfStringBuffer aRun = m_aRun;
    m_aRun.clear();

    m_aSectionHeaders.append(bHeader ? OOO_STRING_SVTOOLS_RTF_HEADERY
                                     : OOO_STRING_SVTOOLS_RTF_FOOTERY);
    m_aSectionHeaders.append(
        static_cast<sal_Int32>(m_rExport.m_pCurrentPageDesc->GetMaster().GetULSpace().GetUpper()));
    if (bTitlepg)
        m_aSectionHeaders.append(OOO_STRING_SVTOOLS_RTF_TITLEPG);
    m_aSectionHeaders.append('{');
    m_aSectionHeaders.append(pStr);
    m_bBufferSectionHeaders = true;
    m_rExport.WriteHeaderFooterText(rFormat, bHeader);
    m_bBufferSectionHeaders = false;
    m_aSectionHeaders.append('}');

    m_aSectionBreaks = aSectionBreaks;
    m_aRun = aRun;
}

namespace
{
void lcl_TextFrameShadow(std::vector<std::pair<OString, OString>>& rFlyProperties,
                         const SwFrameFormat& rFrameFormat)
{
    const SvxShadowItem& aShadowItem = rFrameFormat.GetShadow();
    if (aShadowItem.GetLocation() == SvxShadowLocation::NONE)
        return;

    rFlyProperties.push_back(std::make_pair<OString, OString>("fShadow", OString::number(1)));

    const Color& rColor = aShadowItem.GetColor();
    // We in fact need RGB to BGR, but the transformation is symmetric.
    rFlyProperties.push_back(std::make_pair<OString, OString>(
        "shadowColor", OString::number(wwUtility::RGBToBGR(rColor))));

    // Twips -> points -> EMUs -- hacky, the intermediate step hides rounding errors on roundtrip.
    OString aShadowWidth = OString::number(sal_Int32(aShadowItem.GetWidth() / 20) * 12700);
    OString aOffsetX;
    OString aOffsetY;
    switch (aShadowItem.GetLocation())
    {
        case SvxShadowLocation::TopLeft:
            aOffsetX = "-" + aShadowWidth;
            aOffsetY = "-" + aShadowWidth;
            break;
        case SvxShadowLocation::TopRight:
            aOffsetX = aShadowWidth;
            aOffsetY = "-" + aShadowWidth;
            break;
        case SvxShadowLocation::BottomLeft:
            aOffsetX = "-" + aShadowWidth;
            aOffsetY = aShadowWidth;
            break;
        case SvxShadowLocation::BottomRight:
            aOffsetX = aShadowWidth;
            aOffsetY = aShadowWidth;
            break;
        case SvxShadowLocation::NONE:
        case SvxShadowLocation::End:
            break;
    }
    if (!aOffsetX.isEmpty())
        rFlyProperties.emplace_back("shadowOffsetX", aOffsetX);
    if (!aOffsetY.isEmpty())
        rFlyProperties.emplace_back("shadowOffsetY", aOffsetY);
}

void lcl_TextFrameRelativeSize(std::vector<std::pair<OString, OString>>& rFlyProperties,
                               const SwFrameFormat& rFrameFormat)
{
    const SwFormatFrameSize& rSize = rFrameFormat.GetFrameSize();

    // Relative size of the Text Frame.
    const sal_uInt8 nWidthPercent = rSize.GetWidthPercent();
    if (nWidthPercent && nWidthPercent != SwFormatFrameSize::SYNCED)
    {
        rFlyProperties.push_back(
            std::make_pair<OString, OString>("pctHoriz", OString::number(nWidthPercent * 10)));

        OString aRelation;
        switch (rSize.GetWidthPercentRelation())
        {
            case text::RelOrientation::PAGE_FRAME:
                aRelation = "1"; // page
                break;
            default:
                aRelation = "0"; // margin
                break;
        }
        rFlyProperties.emplace_back(std::make_pair("sizerelh", aRelation));
    }
    const sal_uInt8 nHeightPercent = rSize.GetHeightPercent();
    if (!(nHeightPercent && nHeightPercent != SwFormatFrameSize::SYNCED))
        return;

    rFlyProperties.push_back(
        std::make_pair<OString, OString>("pctVert", OString::number(nHeightPercent * 10)));

    OString aRelation;
    switch (rSize.GetHeightPercentRelation())
    {
        case text::RelOrientation::PAGE_FRAME:
            aRelation = "1"; // page
            break;
        default:
            aRelation = "0"; // margin
            break;
    }
    rFlyProperties.emplace_back(std::make_pair("sizerelv", aRelation));
}
}

void RtfAttributeOutput::writeTextFrame(const ww8::Frame& rFrame, bool bTextBox)
{
    RtfStringBuffer aRunText;
    if (bTextBox)
    {
        m_rExport.setStream();
        aRunText = m_aRunText;
        m_aRunText.clear();
    }

    m_rExport.Strm().WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_SHPTXT);

    {
        // Save table state, in case the inner text also contains a table.
        ww8::WW8TableInfo::Pointer_t pTableInfoOrig = m_rExport.m_pTableInfo;
        m_rExport.m_pTableInfo = std::make_shared<ww8::WW8TableInfo>();
        std::unique_ptr<SwWriteTable> pTableWrt(std::move(m_pTableWrt));
        sal_uInt32 nTableDepth = m_nTableDepth;

        m_nTableDepth = 0;
        /*
         * Save m_aRun as we should not lose the opening brace.
         * OTOH, just drop the contents of m_aRunText in case something
         * would be there, causing a problem later.
         */
        OString aSave = m_aRun.makeStringAndClear();
        // Also back m_bInRun and m_bSingleEmptyRun up.
        bool bInRunOrig = m_bInRun;
        m_bInRun = false;
        bool bSingleEmptyRunOrig = m_bSingleEmptyRun;
        m_bSingleEmptyRun = false;
        m_rExport.SetRTFFlySyntax(true);

        const SwFrameFormat& rFrameFormat = rFrame.GetFrameFormat();
        const SwNodeIndex* pNodeIndex = rFrameFormat.GetContent().GetContentIdx();
        sal_uLong nStt = pNodeIndex ? pNodeIndex->GetIndex() + 1 : 0;
        sal_uLong nEnd = pNodeIndex ? pNodeIndex->GetNode().EndOfSectionIndex() : 0;
        m_rExport.SaveData(nStt, nEnd);
        m_rExport.m_pParentFrame = &rFrame;
        m_rExport.WriteText();
        m_rExport.RestoreData();

        m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PARD);
        m_rExport.SetRTFFlySyntax(false);
        m_aRun->append(aSave);
        m_aRunText.clear();
        m_bInRun = bInRunOrig;
        m_bSingleEmptyRun = bSingleEmptyRunOrig;

        // Restore table state.
        m_rExport.m_pTableInfo = pTableInfoOrig;
        m_pTableWrt = std::move(pTableWrt);
        m_nTableDepth = nTableDepth;
    }

    m_rExport.m_pParentFrame = nullptr;

    m_rExport.Strm().WriteChar('}'); // shptxt

    if (bTextBox)
    {
        m_aRunText = aRunText;
        m_aRunText->append(m_rExport.getStream());
        m_rExport.resetStream();
    }
}

/** save the current run state around exporting things that contain paragraphs
    themselves like text frames.
    TODO: probably more things need to be saved?
 */
class SaveRunState
{
private:
    RtfAttributeOutput& m_rRtf;
    RtfStringBuffer m_Run;
    RtfStringBuffer m_RunText;
    bool const m_bSingleEmptyRun;
    bool const m_bInRun;

public:
    explicit SaveRunState(RtfAttributeOutput& rRtf)
        : m_rRtf(rRtf)
        , m_Run(std::move(rRtf.m_aRun))
        , m_RunText(std::move(rRtf.m_aRunText))
        , m_bSingleEmptyRun(rRtf.m_bSingleEmptyRun)
        , m_bInRun(rRtf.m_bInRun)
    {
        m_rRtf.m_rExport.setStream();
    }
    ~SaveRunState()
    {
        m_rRtf.m_aRun = std::move(m_Run);
        m_rRtf.m_aRunText = std::move(m_RunText);
        m_rRtf.m_bSingleEmptyRun = m_bSingleEmptyRun;
        m_rRtf.m_bInRun = m_bInRun;

        m_rRtf.m_aRunText->append(m_rRtf.m_rExport.getStream());
        m_rRtf.m_rExport.resetStream();
    }
};

void RtfAttributeOutput::OutputFlyFrame_Impl(const ww8::Frame& rFrame, const Point& /*rNdTopLeft*/)
{
    const SwNode* pNode = rFrame.GetContent();
    const SwGrfNode* pGrfNode = pNode ? pNode->GetGrfNode() : nullptr;

    switch (rFrame.GetWriterType())
    {
        case ww8::Frame::eTextBox:
        {
            // If this is a TextBox of a shape, then ignore: it's handled in RtfSdrExport::StartShape().
            if (RtfSdrExport::isTextBox(rFrame.GetFrameFormat()))
                break;

            SaveRunState const saved(*this);

            m_rExport.m_pParentFrame = &rFrame;

            m_rExport.Strm().WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_SHP);
            m_rExport.Strm().WriteCharPtr(
                "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_SHPINST);

            // Shape properties.
            m_aFlyProperties.push_back(std::make_pair<OString, OString>(
                "shapeType", OString::number(ESCHER_ShpInst_TextBox)));

            // When a frame has some low height, but automatically expanded due
            // to lots of contents, this size contains the real size.
            const Size aSize = rFrame.GetSize();
            m_pFlyFrameSize = &aSize;

            m_rExport.m_bOutFlyFrameAttrs = true;
            m_rExport.SetRTFFlySyntax(true);
            m_rExport.OutputFormat(rFrame.GetFrameFormat(), false, false, true);

            // Write ZOrder.
            if (const SdrObject* pObject = rFrame.GetFrameFormat().FindRealSdrObject())
            {
                m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SHPZ);
                m_rExport.OutULong(pObject->GetOrdNum());
            }

            m_rExport.Strm().WriteOString(m_aRunText.makeStringAndClear());
            m_rExport.Strm().WriteOString(m_aStyles.makeStringAndClear());
            m_rExport.m_bOutFlyFrameAttrs = false;
            m_rExport.SetRTFFlySyntax(false);
            m_pFlyFrameSize = nullptr;

            const SwFrameFormat& rFrameFormat = rFrame.GetFrameFormat();
            lcl_TextFrameShadow(m_aFlyProperties, rFrameFormat);
            lcl_TextFrameRelativeSize(m_aFlyProperties, rFrameFormat);

            for (const std::pair<OString, OString>& rPair : m_aFlyProperties)
            {
                m_rExport.Strm().WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_SP "{");
                m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SN " ");
                m_rExport.Strm().WriteOString(rPair.first);
                m_rExport.Strm().WriteCharPtr("}{" OOO_STRING_SVTOOLS_RTF_SV " ");
                m_rExport.Strm().WriteOString(rPair.second);
                m_rExport.Strm().WriteCharPtr("}}");
            }
            m_aFlyProperties.clear();

            writeTextFrame(rFrame);

            m_rExport.Strm().WriteChar('}'); // shpinst
            m_rExport.Strm().WriteChar('}'); // shp

            m_rExport.Strm().WriteCharPtr(SAL_NEWLINE_STRING);
        }
        break;
        case ww8::Frame::eGraphic:
            if (pGrfNode)
            {
                m_aRunText.append(dynamic_cast<const SwFlyFrameFormat*>(&rFrame.GetFrameFormat()),
                                  pGrfNode);
            }
            else if (!rFrame.IsInline())
            {
                m_rExport.m_pParentFrame = &rFrame;
                m_rExport.SetRTFFlySyntax(true);
                m_rExport.OutputFormat(rFrame.GetFrameFormat(), false, false, true);
                m_rExport.SetRTFFlySyntax(false);
                m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE);
                m_rExport.OutputFormat(rFrame.GetFrameFormat(), false, false, true);
                m_aRunText->append('}');
                m_rExport.m_pParentFrame = nullptr;
            }
            break;
        case ww8::Frame::eDrawing:
        {
            const SdrObject* pSdrObj = rFrame.GetFrameFormat().FindRealSdrObject();
            if (pSdrObj)
            {
                m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_FIELD "{");
                m_aRunText->append(OOO_STRING_SVTOOLS_RTF_IGNORE);
                m_aRunText->append(OOO_STRING_SVTOOLS_RTF_FLDINST);
                m_aRunText->append(" SHAPE ");
                m_aRunText->append("}"
                                   "{" OOO_STRING_SVTOOLS_RTF_FLDRSLT);

                m_rExport.SdrExporter().AddSdrObject(*pSdrObj);

                m_aRunText->append('}');
                m_aRunText->append('}');
            }
        }
        break;
        case ww8::Frame::eFormControl:
        {
            const SwFrameFormat& rFrameFormat = rFrame.GetFrameFormat();
            const SdrObject* pObject = rFrameFormat.FindRealSdrObject();

            m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_FIELD);
            m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FLDINST);

            if (pObject && pObject->GetObjInventor() == SdrInventor::FmForm)
            {
                if (auto pFormObj = dynamic_cast<const SdrUnoObj*>(pObject))
                {
                    const uno::Reference<awt::XControlModel>& xControlModel
                        = pFormObj->GetUnoControlModel();
                    uno::Reference<lang::XServiceInfo> xInfo(xControlModel, uno::UNO_QUERY);
                    if (xInfo.is())
                    {
                        uno::Reference<beans::XPropertySet> xPropSet(xControlModel, uno::UNO_QUERY);
                        uno::Reference<beans::XPropertySetInfo> xPropSetInfo
                            = xPropSet->getPropertySetInfo();
                        OUString sName;
                        if (xInfo->supportsService("com.sun.star.form.component.CheckBox"))
                        {
                            m_aRun->append(OUStringToOString(FieldString(ww::eFORMCHECKBOX),
                                                             m_rExport.GetCurrentEncoding()));
                            m_aRun->append(
                                "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FORMFIELD
                                "{");
                            m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFTYPE "1"); // 1 = checkbox
                            // checkbox size in half points, this seems to be always 20
                            m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFHPS "20");

                            OUString aStr;
                            sName = "Name";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aStr;
                                m_aRun->append(
                                    "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFNAME
                                    " ");
                                m_aRun->append(
                                    OUStringToOString(aStr, m_rExport.GetCurrentEncoding()));
                                m_aRun->append('}');
                            }

                            sName = "HelpText";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aStr;
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFOWNHELP);
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE
                                                   OOO_STRING_SVTOOLS_RTF_FFHELPTEXT " ");
                                m_aRun->append(
                                    OUStringToOString(aStr, m_rExport.GetCurrentEncoding()));
                                m_aRun->append('}');
                            }

                            sName = "HelpF1Text";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aStr;
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFOWNSTAT);
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE
                                                   OOO_STRING_SVTOOLS_RTF_FFSTATTEXT " ");
                                m_aRun->append(
                                    OUStringToOString(aStr, m_rExport.GetCurrentEncoding()));
                                m_aRun->append('}');
                            }

                            sal_Int16 nTemp = 0;
                            xPropSet->getPropertyValue("DefaultState") >>= nTemp;
                            m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFDEFRES);
                            m_aRun->append(static_cast<sal_Int32>(nTemp));
                            xPropSet->getPropertyValue("State") >>= nTemp;
                            m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFRES);
                            m_aRun->append(static_cast<sal_Int32>(nTemp));

                            m_aRun->append("}}");

                            // field result is empty, ffres already contains the form result
                            m_aRun->append("}{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " ");
                        }
                        else if (xInfo->supportsService("com.sun.star.form.component.TextField"))
                        {
                            OStringBuffer aBuf;
                            OString aStr;
                            OUString aTmp;
                            const char* pStr;

                            m_aRun->append(OUStringToOString(FieldString(ww::eFORMTEXT),
                                                             m_rExport.GetCurrentEncoding()));
                            m_aRun->append(
                                "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_DATAFIELD
                                " ");
                            for (int i = 0; i < 8; i++)
                                aBuf.append(char(0x00));
                            xPropSet->getPropertyValue("Name") >>= aTmp;
                            aStr = OUStringToOString(aTmp, m_rExport.GetCurrentEncoding());
                            aBuf.append(static_cast<char>(aStr.getLength()));
                            aBuf.append(aStr);
                            aBuf.append(char(0x00));
                            xPropSet->getPropertyValue("DefaultText") >>= aTmp;
                            aStr = OUStringToOString(aTmp, m_rExport.GetCurrentEncoding());
                            aBuf.append(static_cast<char>(aStr.getLength()));
                            aBuf.append(aStr);
                            for (int i = 0; i < 11; i++)
                                aBuf.append(char(0x00));
                            aStr = aBuf.makeStringAndClear();
                            pStr = aStr.getStr();
                            for (int i = 0; i < aStr.getLength(); i++, pStr++)
                                m_aRun->append(msfilter::rtfutil::OutHex(*pStr, 2));
                            m_aRun->append('}');
                            m_aRun->append("}{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " ");
                            xPropSet->getPropertyValue("Text") >>= aTmp;
                            m_aRun->append(OUStringToOString(aTmp, m_rExport.GetCurrentEncoding()));
                            m_aRun->append('}');
                            m_aRun->append(
                                "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FORMFIELD
                                "{");
                            sName = "HelpText";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aTmp;
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFOWNHELP);
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE
                                                   OOO_STRING_SVTOOLS_RTF_FFHELPTEXT " ");
                                m_aRun->append(
                                    OUStringToOString(aTmp, m_rExport.GetCurrentEncoding()));
                                m_aRun->append('}');
                            }

                            sName = "HelpF1Text";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aTmp;
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFOWNSTAT);
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE
                                                   OOO_STRING_SVTOOLS_RTF_FFSTATTEXT " ");
                                m_aRun->append(
                                    OUStringToOString(aTmp, m_rExport.GetCurrentEncoding()));
                                m_aRun->append('}');
                            }
                            m_aRun->append("}");
                        }
                        else if (xInfo->supportsService("com.sun.star.form.component.ListBox"))
                        {
                            OUString aStr;
                            uno::Sequence<sal_Int16> aIntSeq;
                            uno::Sequence<OUString> aStrSeq;

                            m_aRun->append(OUStringToOString(FieldString(ww::eFORMDROPDOWN),
                                                             m_rExport.GetCurrentEncoding()));
                            m_aRun->append(
                                "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FORMFIELD
                                "{");
                            m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFTYPE "2"); // 2 = list
                            m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFHASLISTBOX);

                            xPropSet->getPropertyValue("DefaultSelection") >>= aIntSeq;
                            if (aIntSeq.hasElements())
                            {
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFDEFRES);
                                // a dropdown list can have only one 'selected item by default'
                                m_aRun->append(static_cast<sal_Int32>(aIntSeq[0]));
                            }

                            xPropSet->getPropertyValue("SelectedItems") >>= aIntSeq;
                            if (aIntSeq.hasElements())
                            {
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFRES);
                                // a dropdown list can have only one 'currently selected item'
                                m_aRun->append(static_cast<sal_Int32>(aIntSeq[0]));
                            }

                            sName = "Name";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aStr;
                                m_aRun->append(
                                    "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFNAME
                                    " ");
                                m_aRun->append(
                                    OUStringToOString(aStr, m_rExport.GetCurrentEncoding()));
                                m_aRun->append('}');
                            }

                            sName = "HelpText";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aStr;
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFOWNHELP);
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE
                                                   OOO_STRING_SVTOOLS_RTF_FFHELPTEXT " ");
                                m_aRun->append(
                                    OUStringToOString(aStr, m_rExport.GetCurrentEncoding()));
                                m_aRun->append('}');
                            }

                            sName = "HelpF1Text";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aStr;
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFOWNSTAT);
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE
                                                   OOO_STRING_SVTOOLS_RTF_FFSTATTEXT " ");
                                m_aRun->append(
                                    OUStringToOString(aStr, m_rExport.GetCurrentEncoding()));
                                m_aRun->append('}');
                            }

                            xPropSet->getPropertyValue("StringItemList") >>= aStrSeq;
                            for (const auto& rStr : std::as_const(aStrSeq))
                                m_aRun
                                    ->append(
                                        "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFL
                                        " ")
                                    .append(OUStringToOString(rStr, m_rExport.GetCurrentEncoding()))
                                    .append('}');

                            m_aRun->append("}}");

                            // field result is empty, ffres already contains the form result
                            m_aRun->append("}{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " ");
                        }
                        else
                            SAL_INFO("sw.rtf", __func__ << " unhandled form control: '"
                                                        << xInfo->getImplementationName() << "'");
                        m_aRun->append('}');
                    }
                }
            }

            m_aRun->append('}');
        }
        break;
        case ww8::Frame::eOle:
        {
            const SwFrameFormat& rFrameFormat = rFrame.GetFrameFormat();
            const SdrObject* pSdrObj = rFrameFormat.FindRealSdrObject();
            if (pSdrObj)
            {
                SwNodeIndex aIdx(*rFrameFormat.GetContent().GetContentIdx(), 1);
                SwOLENode& rOLENd = *aIdx.GetNode().GetOLENode();
                FlyFrameOLE(dynamic_cast<const SwFlyFrameFormat*>(&rFrameFormat), rOLENd,
                            rFrame.GetLayoutSize());
            }
        }
        break;
        default:
            SAL_INFO("sw.rtf", __func__ << ": unknown type ("
                                        << static_cast<int>(rFrame.GetWriterType()) << ")");
            break;
    }
}

void RtfAttributeOutput::CharCaseMap(const SvxCaseMapItem& rCaseMap)
{
    switch (rCaseMap.GetValue())
    {
        case SvxCaseMap::SmallCaps:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SCAPS);
            break;
        case SvxCaseMap::Uppercase:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CAPS);
            break;
        default: // Something that rtf does not support
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SCAPS);
            m_aStyles.append(sal_Int32(0));
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CAPS);
            m_aStyles.append(sal_Int32(0));
            break;
    }
}

void RtfAttributeOutput::CharColor(const SvxColorItem& rColor)
{
    const Color aColor(rColor.GetValue());

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CF);
    m_aStyles.append(static_cast<sal_Int32>(m_rExport.GetColor(aColor)));
}

void RtfAttributeOutput::CharContour(const SvxContourItem& rContour)
{
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_OUTL);
    if (!rContour.GetValue())
        m_aStyles.append(sal_Int32(0));
}

void RtfAttributeOutput::CharCrossedOut(const SvxCrossedOutItem& rCrossedOut)
{
    switch (rCrossedOut.GetStrikeout())
    {
        case STRIKEOUT_NONE:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_STRIKE);
            m_aStyles.append(sal_Int32(0));
            break;
        case STRIKEOUT_DOUBLE:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_STRIKED);
            m_aStyles.append(sal_Int32(1));
            break;
        default:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_STRIKE);
            break;
    }
}

void RtfAttributeOutput::CharEscapement(const SvxEscapementItem& rEscapement)
{
    short nEsc = rEscapement.GetEsc();
    short nProp = rEscapement.GetProportionalHeight();
    sal_Int32 nProp100 = nProp * 100;
    if (DFLT_ESC_PROP == nProp || nProp < 1 || nProp > 100)
    {
        if (DFLT_ESC_SUB == nEsc || DFLT_ESC_AUTO_SUB == nEsc)
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SUB);
        else if (DFLT_ESC_SUPER == nEsc || DFLT_ESC_AUTO_SUPER == nEsc)
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SUPER);
        return;
    }
    if (DFLT_ESC_AUTO_SUPER == nEsc)
    {
        nEsc = .8 * (100 - nProp);
        ++nProp100; // A 1 afterwards means 'automatic' according to editeng/rtf/rtfitem.cxx
    }
    else if (DFLT_ESC_AUTO_SUB == nEsc)
    {
        nEsc = .2 * -(100 - nProp);
        ++nProp100;
    }

    const char* pUpDn;

    double fHeight = m_rExport.GetItem(RES_CHRATR_FONTSIZE).GetHeight();

    if (0 < nEsc)
        pUpDn = OOO_STRING_SVTOOLS_RTF_UP;
    else if (0 > nEsc)
    {
        pUpDn = OOO_STRING_SVTOOLS_RTF_DN;
        fHeight = -fHeight;
    }
    else
        return;

    m_aStyles.append('{');
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_IGNORE);
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_UPDNPROP);
    m_aStyles.append(nProp100);
    m_aStyles.append('}');
    m_aStyles.append(pUpDn);

    /*
     * Calculate the act. FontSize and the percentage of the displacement;
     * RTF file expects half points, while internally it's in twips.
     * Formally :            (FontSize * 1/20 ) pts         x * 2
     *                    -----------------------  = ------------
     *                      100%                       Escapement
     */
    m_aStyles.append(static_cast<sal_Int32>(round(fHeight * nEsc / 1000)));
}

void RtfAttributeOutput::CharFont(const SvxFontItem& rFont)
{
    // Insert \loch in MoveCharacterProperties
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_F);
    m_aStyles.append(static_cast<sal_Int32>(m_rExport.m_aFontHelper.GetId(rFont)));

    // Insert \hich in MoveCharacterProperties
    m_aStylesAssocHich.append(OOO_STRING_SVTOOLS_RTF_AF);
    m_aStylesAssocHich.append(static_cast<sal_Int32>(m_rExport.m_aFontHelper.GetId(rFont)));

    // FIXME: this may be a tad expensive... but the charset needs to be
    // consistent with what wwFont::WriteRtf() does
    sw::util::FontMapExport aTmp(rFont.GetFamilyName());
    sal_uInt8 nWindowsCharset = sw::ms::rtl_TextEncodingToWinCharsetRTF(
        aTmp.msPrimary, aTmp.msSecondary, rFont.GetCharSet());
    m_rExport.SetCurrentEncoding(rtl_getTextEncodingFromWindowsCharset(nWindowsCharset));
    if (m_rExport.GetCurrentEncoding() == RTL_TEXTENCODING_DONTKNOW)
        m_rExport.SetCurrentEncoding(m_rExport.GetDefaultEncoding());
}

void RtfAttributeOutput::CharFontSize(const SvxFontHeightItem& rFontSize)
{
    switch (rFontSize.Which())
    {
        case RES_CHRATR_FONTSIZE:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_FS);
            m_aStyles.append(static_cast<sal_Int32>(rFontSize.GetHeight() / 10));
            break;
        case RES_CHRATR_CJK_FONTSIZE:
            m_aStylesAssocDbch.append(OOO_STRING_SVTOOLS_RTF_AFS);
            m_aStylesAssocDbch.append(static_cast<sal_Int32>(rFontSize.GetHeight() / 10));
            break;
        case RES_CHRATR_CTL_FONTSIZE:
            m_aStylesAssocRtlch.append(OOO_STRING_SVTOOLS_RTF_AFS);
            m_aStylesAssocRtlch.append(static_cast<sal_Int32>(rFontSize.GetHeight() / 10));
            break;
    }
}

void RtfAttributeOutput::CharKerning(const SvxKerningItem& rKerning)
{
    // in quarter points then in twips
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_EXPND);
    m_aStyles.append(static_cast<sal_Int32>(rKerning.GetValue() / 5));
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_EXPNDTW);
    m_aStyles.append(static_cast<sal_Int32>(rKerning.GetValue()));
}

void RtfAttributeOutput::CharLanguage(const SvxLanguageItem& rLanguage)
{
    switch (rLanguage.Which())
    {
        case RES_CHRATR_LANGUAGE:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_LANG);
            m_aStyles.append(
                static_cast<sal_Int32>(static_cast<sal_uInt16>(rLanguage.GetLanguage())));
            m_aStylesAssocLtrch.append(OOO_STRING_SVTOOLS_RTF_LANG);
            m_aStylesAssocLtrch.append(
                static_cast<sal_Int32>(static_cast<sal_uInt16>(rLanguage.GetLanguage())));
            break;
        case RES_CHRATR_CJK_LANGUAGE:
            m_aStylesAssocDbch.append(OOO_STRING_SVTOOLS_RTF_LANGFE);
            m_aStylesAssocDbch.append(
                static_cast<sal_Int32>(static_cast<sal_uInt16>(rLanguage.GetLanguage())));
            m_aStylesAssocLtrch.append(OOO_STRING_SVTOOLS_RTF_LANGFE);
            m_aStylesAssocLtrch.append(
                static_cast<sal_Int32>(static_cast<sal_uInt16>(rLanguage.GetLanguage())));
            break;
        case RES_CHRATR_CTL_LANGUAGE:
            m_aStylesAssocRtlch.append(OOO_STRING_SVTOOLS_RTF_ALANG);
            m_aStylesAssocRtlch.append(
                static_cast<sal_Int32>(static_cast<sal_uInt16>(rLanguage.GetLanguage())));
            break;
    }
}

void RtfAttributeOutput::CharPosture(const SvxPostureItem& rPosture)
{
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_I);
    if (rPosture.GetPosture() == ITALIC_NONE)
        m_aStyles.append(sal_Int32(0));
}

void RtfAttributeOutput::CharShadow(const SvxShadowedItem& rShadow)
{
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SHAD);
    if (!rShadow.GetValue())
        m_aStyles.append(sal_Int32(0));
}

void RtfAttributeOutput::CharUnderline(const SvxUnderlineItem& rUnderline)
{
    const char* pStr = nullptr;
    const SfxPoolItem* pItem = m_rExport.HasItem(RES_CHRATR_WORDLINEMODE);
    bool bWord = false;
    if (pItem)
        bWord = static_cast<const SvxWordLineModeItem*>(pItem)->GetValue();
    switch (rUnderline.GetLineStyle())
    {
        case LINESTYLE_SINGLE:
            pStr = bWord ? OOO_STRING_SVTOOLS_RTF_ULW : OOO_STRING_SVTOOLS_RTF_UL;
            break;
        case LINESTYLE_DOUBLE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULDB;
            break;
        case LINESTYLE_NONE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULNONE;
            break;
        case LINESTYLE_DOTTED:
            pStr = OOO_STRING_SVTOOLS_RTF_ULD;
            break;
        case LINESTYLE_DASH:
            pStr = OOO_STRING_SVTOOLS_RTF_ULDASH;
            break;
        case LINESTYLE_DASHDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_ULDASHD;
            break;
        case LINESTYLE_DASHDOTDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_ULDASHDD;
            break;
        case LINESTYLE_BOLD:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTH;
            break;
        case LINESTYLE_WAVE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULWAVE;
            break;
        case LINESTYLE_BOLDDOTTED:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHD;
            break;
        case LINESTYLE_BOLDDASH:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHDASH;
            break;
        case LINESTYLE_LONGDASH:
            pStr = OOO_STRING_SVTOOLS_RTF_ULLDASH;
            break;
        case LINESTYLE_BOLDLONGDASH:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHLDASH;
            break;
        case LINESTYLE_BOLDDASHDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHDASHD;
            break;
        case LINESTYLE_BOLDDASHDOTDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHDASHDD;
            break;
        case LINESTYLE_BOLDWAVE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULHWAVE;
            break;
        case LINESTYLE_DOUBLEWAVE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULULDBWAVE;
            break;
        default:
            break;
    }

    if (pStr)
    {
        m_aStyles.append(pStr);
        // NEEDSWORK looks like here rUnderline.GetColor() is always black,
        // even if the color in the odt is for example green...
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ULC);
        m_aStyles.append(static_cast<sal_Int32>(m_rExport.GetColor(rUnderline.GetColor())));
    }
}

void RtfAttributeOutput::CharWeight(const SvxWeightItem& rWeight)
{
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_B);
    if (rWeight.GetWeight() != WEIGHT_BOLD)
        m_aStyles.append(sal_Int32(0));
}

void RtfAttributeOutput::CharAutoKern(const SvxAutoKernItem& rAutoKern)
{
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_KERNING);
    m_aStyles.append(static_cast<sal_Int32>(rAutoKern.GetValue() ? 1 : 0));
}

void RtfAttributeOutput::CharAnimatedText(const SvxBlinkItem& rBlink)
{
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ANIMTEXT);
    m_aStyles.append(static_cast<sal_Int32>(rBlink.GetValue() ? 2 : 0));
}

void RtfAttributeOutput::CharBackground(const SvxBrushItem& rBrush)
{
    if (!rBrush.GetColor().GetTransparency())
    {
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CHCBPAT);
        m_aStyles.append(static_cast<sal_Int32>(m_rExport.GetColor(rBrush.GetColor())));
    }
}

void RtfAttributeOutput::CharFontCJK(const SvxFontItem& rFont)
{
    // Insert \dbch in MoveCharacterProperties
    m_aStylesAssocDbch.append(OOO_STRING_SVTOOLS_RTF_AF);
    m_aStylesAssocDbch.append(static_cast<sal_Int32>(m_rExport.m_aFontHelper.GetId(rFont)));
}

void RtfAttributeOutput::CharFontSizeCJK(const SvxFontHeightItem& rFontSize)
{
    CharFontSize(rFontSize);
}

void RtfAttributeOutput::CharLanguageCJK(const SvxLanguageItem& rLanguageItem)
{
    CharLanguage(rLanguageItem);
}

void RtfAttributeOutput::CharPostureCJK(const SvxPostureItem& rPosture)
{
    m_aStylesAssocDbch.append(OOO_STRING_SVTOOLS_RTF_I);
    if (rPosture.GetPosture() == ITALIC_NONE)
        m_aStylesAssocDbch.append(sal_Int32(0));
}

void RtfAttributeOutput::CharWeightCJK(const SvxWeightItem& rWeight)
{
    m_aStylesAssocDbch.append(OOO_STRING_SVTOOLS_RTF_AB);
    if (rWeight.GetWeight() != WEIGHT_BOLD)
        m_aStylesAssocDbch.append(sal_Int32(0));
}

void RtfAttributeOutput::CharFontCTL(const SvxFontItem& rFont)
{
    // Insert \rtlch in MoveCharacterProperties
    m_aStylesAssocRtlch.append(OOO_STRING_SVTOOLS_RTF_AF);
    m_aStylesAssocRtlch.append(static_cast<sal_Int32>(m_rExport.m_aFontHelper.GetId(rFont)));
}

void RtfAttributeOutput::CharFontSizeCTL(const SvxFontHeightItem& rFontSize)
{
    CharFontSize(rFontSize);
}

void RtfAttributeOutput::CharLanguageCTL(const SvxLanguageItem& rLanguageItem)
{
    CharLanguage(rLanguageItem);
}

void RtfAttributeOutput::CharPostureCTL(const SvxPostureItem& rPosture)
{
    m_aStylesAssocRtlch.append(OOO_STRING_SVTOOLS_RTF_AI);
    if (rPosture.GetPosture() == ITALIC_NONE)
        m_aStylesAssocRtlch.append(sal_Int32(0));
}

void RtfAttributeOutput::CharWeightCTL(const SvxWeightItem& rWeight)
{
    m_aStylesAssocRtlch.append(OOO_STRING_SVTOOLS_RTF_AB);
    if (rWeight.GetWeight() != WEIGHT_BOLD)
        m_aStylesAssocRtlch.append(sal_Int32(0));
}

void RtfAttributeOutput::CharBidiRTL(const SfxPoolItem& /*rItem*/) {}

void RtfAttributeOutput::CharIdctHint(const SfxPoolItem& /*rItem*/) {}

void RtfAttributeOutput::CharRotate(const SvxCharRotateItem& rRotate)
{
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_HORZVERT);
    m_aStyles.append(static_cast<sal_Int32>(rRotate.IsFitToLine() ? 1 : 0));
}

void RtfAttributeOutput::CharEmphasisMark(const SvxEmphasisMarkItem& rEmphasisMark)
{
    FontEmphasisMark v = rEmphasisMark.GetEmphasisMark();
    if (v == FontEmphasisMark::NONE)
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ACCNONE);
    else if (v == (FontEmphasisMark::Dot | FontEmphasisMark::PosAbove))
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ACCDOT);
    else if (v == (FontEmphasisMark::Accent | FontEmphasisMark::PosAbove))
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ACCCOMMA);
    else if (v == (FontEmphasisMark::Circle | FontEmphasisMark::PosAbove))
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ACCCIRCLE);
    else if (v == (FontEmphasisMark::Dot | FontEmphasisMark::PosBelow))
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ACCUNDERDOT);
}

void RtfAttributeOutput::CharTwoLines(const SvxTwoLinesItem& rTwoLines)
{
    if (!rTwoLines.GetValue())
        return;

    sal_Unicode cStart = rTwoLines.GetStartBracket();
    sal_Unicode cEnd = rTwoLines.GetEndBracket();

    sal_uInt16 nType;
    if (!cStart && !cEnd)
        nType = 0;
    else if ('{' == cStart || '}' == cEnd)
        nType = 4;
    else if ('<' == cStart || '>' == cEnd)
        nType = 3;
    else if ('[' == cStart || ']' == cEnd)
        nType = 2;
    else // all other kind of brackets
        nType = 1;

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_TWOINONE);
    m_aStyles.append(static_cast<sal_Int32>(nType));
}

void RtfAttributeOutput::CharScaleWidth(const SvxCharScaleWidthItem& rScaleWidth)
{
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CHARSCALEX);
    m_aStyles.append(static_cast<sal_Int32>(rScaleWidth.GetValue()));
}

void RtfAttributeOutput::CharRelief(const SvxCharReliefItem& rRelief)
{
    const char* pStr;
    switch (rRelief.GetValue())
    {
        case FontRelief::Embossed:
            pStr = OOO_STRING_SVTOOLS_RTF_EMBO;
            break;
        case FontRelief::Engraved:
            pStr = OOO_STRING_SVTOOLS_RTF_IMPR;
            break;
        default:
            pStr = nullptr;
            break;
    }

    if (pStr)
        m_aStyles.append(pStr);
}

void RtfAttributeOutput::CharHidden(const SvxCharHiddenItem& rHidden)
{
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_V);
    if (!rHidden.GetValue())
        m_aStyles.append(sal_Int32(0));
}

void RtfAttributeOutput::CharBorder(const editeng::SvxBorderLine* pAllBorder,
                                    const sal_uInt16 nDist, const bool bShadow)
{
    m_aStyles.append(
        OutBorderLine(m_rExport, pAllBorder, OOO_STRING_SVTOOLS_RTF_CHBRDR, nDist,
                      bShadow ? SvxShadowLocation::BottomRight : SvxShadowLocation::NONE));
}

void RtfAttributeOutput::CharHighlight(const SvxBrushItem& rBrush)
{
    if (!rBrush.GetColor().GetTransparency())
    {
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_HIGHLIGHT);
        m_aStyles.append(static_cast<sal_Int32>(msfilter::util::TransColToIco(rBrush.GetColor())));
    }
}

void RtfAttributeOutput::TextINetFormat(const SwFormatINetFormat& rURL)
{
    if (rURL.GetValue().isEmpty())
        return;

    const SwCharFormat* pFormat;
    const SwTextINetFormat* pTextAtr = rURL.GetTextINetFormat();

    if (pTextAtr && nullptr != (pFormat = pTextAtr->GetCharFormat()))
    {
        sal_uInt16 nStyle = m_rExport.GetId(pFormat);
        OString* pString = m_rExport.GetStyle(nStyle);
        if (pString)
            m_aStyles.append(*pString);
    }
}

void RtfAttributeOutput::TextCharFormat(const SwFormatCharFormat& rCharFormat)
{
    sal_uInt16 nStyle = m_rExport.GetId(rCharFormat.GetCharFormat());
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CS);
    m_aStyles.append(static_cast<sal_Int32>(nStyle));
    OString* pString = m_rExport.GetStyle(nStyle);
    if (pString)
        m_aStyles.append(*pString);
}

void RtfAttributeOutput::WriteTextFootnoteNumStr(const SwFormatFootnote& rFootnote)
{
    if (rFootnote.GetNumStr().isEmpty())
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_CHFTN);
    else
        m_aRun->append(
            msfilter::rtfutil::OutString(rFootnote.GetNumStr(), m_rExport.GetCurrentEncoding()));
}

void RtfAttributeOutput::TextFootnote_Impl(const SwFormatFootnote& rFootnote)
{
    SAL_INFO("sw.rtf", __func__ << " start");

    m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_SUPER " ");
    EndRunProperties(nullptr);
    m_aRun->append(' ');
    WriteTextFootnoteNumStr(rFootnote);
    m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FOOTNOTE);
    if (rFootnote.IsEndNote() || m_rExport.m_rDoc.GetFootnoteInfo().m_ePos == FTNPOS_CHAPTER)
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_FTNALT);
    m_aRun->append(' ');
    WriteTextFootnoteNumStr(rFootnote);

    /*
     * The footnote contains a whole paragraph, so we have to:
     * 1) Reset, then later restore the contents of our run buffer and run state.
     * 2) Buffer the output of the whole paragraph, as we do so for section headers already.
     */
    const SwNodeIndex* pIndex = rFootnote.GetTextFootnote()->GetStartNode();
    RtfStringBuffer aRun = m_aRun;
    m_aRun.clear();
    bool bInRunOrig = m_bInRun;
    m_bInRun = false;
    bool bSingleEmptyRunOrig = m_bSingleEmptyRun;
    m_bSingleEmptyRun = false;
    m_bBufferSectionHeaders = true;
    m_rExport.WriteSpecialText(pIndex->GetIndex() + 1, pIndex->GetNode().EndOfSectionIndex(),
                               !rFootnote.IsEndNote() ? TXT_FTN : TXT_EDN);
    m_bBufferSectionHeaders = false;
    m_bInRun = bInRunOrig;
    m_bSingleEmptyRun = bSingleEmptyRunOrig;
    m_aRun = aRun;
    m_aRun->append(m_aSectionHeaders.makeStringAndClear());

    m_aRun->append("}");
    m_aRun->append("}");

    SAL_INFO("sw.rtf", __func__ << " end");
}

void RtfAttributeOutput::ParaLineSpacing_Impl(short nSpace, short nMulti)
{
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SL);
    m_aStyles.append(static_cast<sal_Int32>(nSpace));
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SLMULT);
    m_aStyles.append(static_cast<sal_Int32>(nMulti));
}

void RtfAttributeOutput::ParaAdjust(const SvxAdjustItem& rAdjust)
{
    switch (rAdjust.GetAdjust())
    {
        case SvxAdjust::Left:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_QL);
            break;
        case SvxAdjust::Right:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_QR);
            break;
        case SvxAdjust::BlockLine:
        case SvxAdjust::Block:
            if (rAdjust.GetLastBlock() == SvxAdjust::Block)
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_QD);
            else
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_QJ);
            break;
        case SvxAdjust::Center:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_QC);
            break;
        default:
            break;
    }
}

void RtfAttributeOutput::ParaSplit(const SvxFormatSplitItem& rSplit)
{
    if (!rSplit.GetValue())
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_KEEP);
}

void RtfAttributeOutput::ParaWidows(const SvxWidowsItem& rWidows)
{
    if (rWidows.GetValue())
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_WIDCTLPAR);
    else
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_NOWIDCTLPAR);
}

void RtfAttributeOutput::ParaTabStop(const SvxTabStopItem& rTabStop)
{
    tools::Long nOffset = 0;
    // Tabs are absolute by default.
    if (m_rExport.m_rDoc.getIDocumentSettingAccess().get(
            DocumentSettingId::TABS_RELATIVE_TO_INDENT))
        nOffset = m_rExport.GetItem(RES_LR_SPACE).GetTextLeft();

    for (sal_uInt16 n = 0; n < rTabStop.Count(); n++)
    {
        const SvxTabStop& rTS = rTabStop[n];
        if (SvxTabAdjust::Default != rTS.GetAdjustment())
        {
            const char* pFill = nullptr;
            switch (rTS.GetFill())
            {
                case cDfltFillChar:
                    break;

                case '.':
                    pFill = OOO_STRING_SVTOOLS_RTF_TLDOT;
                    break;
                case '_':
                    pFill = OOO_STRING_SVTOOLS_RTF_TLUL;
                    break;
                case '-':
                    pFill = OOO_STRING_SVTOOLS_RTF_TLTH;
                    break;
                case '=':
                    pFill = OOO_STRING_SVTOOLS_RTF_TLEQ;
                    break;
                default:
                    break;
            }
            if (pFill)
                m_aStyles.append(pFill);

            const char* pAdjStr = nullptr;
            switch (rTS.GetAdjustment())
            {
                case SvxTabAdjust::Right:
                    pAdjStr = OOO_STRING_SVTOOLS_RTF_TQR;
                    break;
                case SvxTabAdjust::Decimal:
                    pAdjStr = OOO_STRING_SVTOOLS_RTF_TQDEC;
                    break;
                case SvxTabAdjust::Center:
                    pAdjStr = OOO_STRING_SVTOOLS_RTF_TQC;
                    break;
                default:
                    break;
            }
            if (pAdjStr)
                m_aStyles.append(pAdjStr);
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_TX);
            m_aStyles.append(static_cast<sal_Int32>(rTS.GetTabPos() + nOffset));
        }
        else
        {
            m_aTabStop.append(OOO_STRING_SVTOOLS_RTF_DEFTAB);
            m_aTabStop.append(rTabStop[0].GetTabPos());
        }
    }
}

void RtfAttributeOutput::ParaHyphenZone(const SvxHyphenZoneItem& rHyphenZone)
{
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_HYPHPAR);
    m_aStyles.append(sal_Int32(rHyphenZone.IsHyphen()));
}

void RtfAttributeOutput::ParaNumRule_Impl(const SwTextNode* pTextNd, sal_Int32 nLvl,
                                          sal_Int32 nNumId)
{
    if (USHRT_MAX == nNumId || 0 == nNumId || nullptr == pTextNd)
        return;

    const SwNumRule* pRule = pTextNd->GetNumRule();

    if (!pRule || !pTextNd->IsInList())
        return;

    SAL_WARN_IF(pTextNd->GetActualListLevel() < 0 || pTextNd->GetActualListLevel() >= MAXLEVEL,
                "sw.rtf", "text node does not have valid list level");

    const SwNumFormat* pFormat = pRule->GetNumFormat(nLvl);
    if (!pFormat)
        pFormat = &pRule->Get(nLvl);

    const SfxItemSet& rNdSet = pTextNd->GetSwAttrSet();

    m_aStyles.append('{');
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_LISTTEXT);
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_PARD);
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_PLAIN);
    m_aStyles.append(' ');

    SvxLRSpaceItem aLR(rNdSet.Get(RES_LR_SPACE));
    aLR.SetTextLeft(aLR.GetTextLeft() + pFormat->GetIndentAt());
    aLR.SetTextFirstLineOffset(pFormat->GetFirstLineOffset()); //TODO: overflow

    sal_uInt16 nStyle = m_rExport.GetId(pFormat->GetCharFormat());
    OString* pString = m_rExport.GetStyle(nStyle);
    if (pString)
        m_aStyles.append(*pString);

    {
        OUString sText;
        if (SVX_NUM_CHAR_SPECIAL == pFormat->GetNumberingType()
            || SVX_NUM_BITMAP == pFormat->GetNumberingType())
        {
            sal_UCS4 cBullet = pFormat->GetBulletChar();
            sText = OUString(&cBullet, 1);
        }
        else
            sText = pTextNd->GetNumString();

        if (!sText.isEmpty())
        {
            m_aStyles.append(' ');
            m_aStyles.append(msfilter::rtfutil::OutString(sText, m_rExport.GetDefaultEncoding()));
        }

        if (OUTLINE_RULE != pRule->GetRuleType())
        {
            if (!sText.isEmpty())
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_TAB);
            m_aStyles.append('}');
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ILVL);
            if (nLvl > 8) // RTF knows only 9 levels
            {
                m_aStyles.append(sal_Int32(8));
                m_aStyles.append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_SOUTLVL);
                m_aStyles.append(nLvl);
                m_aStyles.append('}');
            }
            else
                m_aStyles.append(nLvl);
        }
        else
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_TAB "}");
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_LS);
        m_aStyles.append(static_cast<sal_Int32>(m_rExport.GetNumberingId(*pRule)) + 1);
        m_aStyles.append(' ');
    }
    FormatLRSpace(aLR);
}

void RtfAttributeOutput::ParaScriptSpace(const SfxBoolItem& rScriptSpace)
{
    if (!rScriptSpace.GetValue())
        return;

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ASPALPHA);
}

void RtfAttributeOutput::ParaHangingPunctuation(const SfxBoolItem& /*rItem*/)
{
    SAL_INFO("sw.rtf", "TODO: " << __func__);
}

void RtfAttributeOutput::ParaForbiddenRules(const SfxBoolItem& /*rItem*/)
{
    SAL_INFO("sw.rtf", "TODO: " << __func__);
}

void RtfAttributeOutput::ParaVerticalAlign(const SvxParaVertAlignItem& rAlign)
{
    const char* pStr;
    switch (rAlign.GetValue())
    {
        case SvxParaVertAlignItem::Align::Top:
            pStr = OOO_STRING_SVTOOLS_RTF_FAHANG;
            break;
        case SvxParaVertAlignItem::Align::Bottom:
            pStr = OOO_STRING_SVTOOLS_RTF_FAVAR;
            break;
        case SvxParaVertAlignItem::Align::Center:
            pStr = OOO_STRING_SVTOOLS_RTF_FACENTER;
            break;
        case SvxParaVertAlignItem::Align::Baseline:
            pStr = OOO_STRING_SVTOOLS_RTF_FAROMAN;
            break;

        default:
            pStr = OOO_STRING_SVTOOLS_RTF_FAAUTO;
            break;
    }
    m_aStyles.append(pStr);
}

void RtfAttributeOutput::ParaSnapToGrid(const SvxParaGridItem& /*rGrid*/)
{
    SAL_INFO("sw.rtf", "TODO: " << __func__);
}

void RtfAttributeOutput::FormatFrameSize(const SwFormatFrameSize& rSize)
{
    if (m_rExport.m_bOutPageDescs)
    {
        m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_PGWSXN);
        m_aSectionBreaks.append(static_cast<sal_Int32>(rSize.GetWidth()));
        m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_PGHSXN);
        m_aSectionBreaks.append(static_cast<sal_Int32>(rSize.GetHeight()));
        if (!m_bBufferSectionBreaks)
            m_rExport.Strm().WriteOString(m_aSectionBreaks.makeStringAndClear());
    }
}

void RtfAttributeOutput::FormatPaperBin(const SvxPaperBinItem& /*rItem*/)
{
    SAL_INFO("sw.rtf", "TODO: " << __func__);
}

void RtfAttributeOutput::FormatLRSpace(const SvxLRSpaceItem& rLRSpace)
{
    if (!m_rExport.m_bOutFlyFrameAttrs)
    {
        if (m_rExport.m_bOutPageDescs)
        {
            if (rLRSpace.GetLeft())
            {
                m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_MARGLSXN);
                m_aSectionBreaks.append(static_cast<sal_Int32>(rLRSpace.GetLeft()));
            }
            if (rLRSpace.GetRight())
            {
                m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_MARGRSXN);
                m_aSectionBreaks.append(static_cast<sal_Int32>(rLRSpace.GetRight()));
            }
            if (!m_bBufferSectionBreaks)
                m_rExport.Strm().WriteOString(m_aSectionBreaks.makeStringAndClear());
        }
        else
        {
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_LI);
            m_aStyles.append(static_cast<sal_Int32>(rLRSpace.GetTextLeft()));
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_RI);
            m_aStyles.append(static_cast<sal_Int32>(rLRSpace.GetRight()));
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_LIN);
            m_aStyles.append(static_cast<sal_Int32>(rLRSpace.GetTextLeft()));
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_RIN);
            m_aStyles.append(static_cast<sal_Int32>(rLRSpace.GetRight()));
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_FI);
            m_aStyles.append(static_cast<sal_Int32>(rLRSpace.GetTextFirstLineOffset()));
        }
    }
    else if (m_rExport.GetRTFFlySyntax())
    {
        // Wrap: top and bottom spacing, convert from twips to EMUs.
        m_aFlyProperties.push_back(std::make_pair<OString, OString>(
            "dxWrapDistLeft", OString::number(rLRSpace.GetLeft() * 635)));
        m_aFlyProperties.push_back(std::make_pair<OString, OString>(
            "dxWrapDistRight", OString::number(rLRSpace.GetRight() * 635)));
    }
}

void RtfAttributeOutput::FormatULSpace(const SvxULSpaceItem& rULSpace)
{
    if (!m_rExport.m_bOutFlyFrameAttrs)
    {
        if (m_rExport.m_bOutPageDescs)
        {
            OSL_ENSURE(m_rExport.GetCurItemSet(), "Impossible");
            if (!m_rExport.GetCurItemSet())
                return;

            // If we export a follow page format, then our doc model has
            // separate header/footer distances for the first page and the
            // follow pages, but Word can have only a single distance. In case
            // the two values differ, work with the value from the first page
            // format to be in sync with the import.
            sw::util::HdFtDistanceGlue aDistances(m_rExport.GetFirstPageItemSet()
                                                      ? *m_rExport.GetFirstPageItemSet()
                                                      : *m_rExport.GetCurItemSet());

            if (aDistances.dyaTop)
            {
                m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_MARGTSXN);
                m_aSectionBreaks.append(static_cast<sal_Int32>(aDistances.dyaTop));
            }
            if (aDistances.HasHeader())
            {
                m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_HEADERY);
                m_aSectionBreaks.append(static_cast<sal_Int32>(aDistances.dyaHdrTop));
            }

            if (aDistances.dyaBottom)
            {
                m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_MARGBSXN);
                m_aSectionBreaks.append(static_cast<sal_Int32>(aDistances.dyaBottom));
            }
            if (aDistances.HasFooter())
            {
                m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_FOOTERY);
                m_aSectionBreaks.append(static_cast<sal_Int32>(aDistances.dyaHdrBottom));
            }
            if (!m_bBufferSectionBreaks)
                m_rExport.Strm().WriteOString(m_aSectionBreaks.makeStringAndClear());
        }
        else
        {
            // Spacing before.
            if (m_bParaBeforeAutoSpacing && m_nParaBeforeSpacing == rULSpace.GetUpper())
                m_aStyles.append(LO_STRING_SVTOOLS_RTF_SBAUTO "1");
            else if (m_bParaBeforeAutoSpacing && m_nParaBeforeSpacing == -1)
            {
                m_aStyles.append(LO_STRING_SVTOOLS_RTF_SBAUTO "0");
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SB);
                m_aStyles.append(static_cast<sal_Int32>(rULSpace.GetUpper()));
            }
            else
            {
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SB);
                m_aStyles.append(static_cast<sal_Int32>(rULSpace.GetUpper()));
            }
            m_bParaBeforeAutoSpacing = false;

            // Spacing after.
            if (m_bParaAfterAutoSpacing && m_nParaAfterSpacing == rULSpace.GetLower())
                m_aStyles.append(LO_STRING_SVTOOLS_RTF_SAAUTO "1");
            else if (m_bParaAfterAutoSpacing && m_nParaAfterSpacing == -1)
            {
                m_aStyles.append(LO_STRING_SVTOOLS_RTF_SAAUTO "0");
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SA);
                m_aStyles.append(static_cast<sal_Int32>(rULSpace.GetLower()));
            }
            else
            {
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SA);
                m_aStyles.append(static_cast<sal_Int32>(rULSpace.GetLower()));
            }
            m_bParaAfterAutoSpacing = false;

            // Contextual spacing.
            if (rULSpace.GetContext())
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CONTEXTUALSPACE);
        }
    }
    else if (m_rExport.GetRTFFlySyntax())
    {
        // Wrap: top and bottom spacing, convert from twips to EMUs.
        m_aFlyProperties.push_back(std::make_pair<OString, OString>(
            "dyWrapDistTop", OString::number(rULSpace.GetUpper() * 635)));
        m_aFlyProperties.push_back(std::make_pair<OString, OString>(
            "dyWrapDistBottom", OString::number(rULSpace.GetLower() * 635)));
    }
}

void RtfAttributeOutput::FormatSurround(const SwFormatSurround& rSurround)
{
    if (m_rExport.m_bOutFlyFrameAttrs && !m_rExport.GetRTFFlySyntax())
    {
        css::text::WrapTextMode eSurround = rSurround.GetSurround();
        bool bGold = css::text::WrapTextMode_DYNAMIC == eSurround;
        if (bGold)
            eSurround = css::text::WrapTextMode_PARALLEL;
        RTFSurround aMC(bGold, static_cast<sal_uInt8>(eSurround));
        m_aRunText->append(OOO_STRING_SVTOOLS_RTF_FLYMAINCNT);
        m_aRunText->append(static_cast<sal_Int32>(aMC.GetValue()));
    }
    else if (m_rExport.m_bOutFlyFrameAttrs && m_rExport.GetRTFFlySyntax())
    {
        // See DocxSdrExport::startDMLAnchorInline() for SwFormatSurround -> WR / WRK mappings.
        sal_Int32 nWr = -1;
        std::optional<sal_Int32> oWrk;
        switch (rSurround.GetValue())
        {
            case css::text::WrapTextMode_NONE:
                nWr = 1; // top and bottom
                break;
            case css::text::WrapTextMode_THROUGH:
                nWr = 3; // none
                break;
            case css::text::WrapTextMode_PARALLEL:
                nWr = 2; // around
                oWrk = 0; // both sides
                break;
            case css::text::WrapTextMode_DYNAMIC:
            default:
                nWr = 2; // around
                oWrk = 3; // largest
                break;
        }

        if (rSurround.IsContour())
            nWr = 4; // tight

        m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SHPWR);
        m_rExport.OutLong(nWr);
        if (oWrk)
        {
            m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SHPWRK);
            m_rExport.OutLong(*oWrk);
        }
    }
}

void RtfAttributeOutput::FormatVertOrientation(const SwFormatVertOrient& rFlyVert)
{
    if (!(m_rExport.m_bOutFlyFrameAttrs && m_rExport.GetRTFFlySyntax()))
        return;

    switch (rFlyVert.GetRelationOrient())
    {
        case text::RelOrientation::PAGE_FRAME:
            m_aFlyProperties.push_back(
                std::make_pair<OString, OString>("posrelv", OString::number(1)));
            break;
        default:
            m_aFlyProperties.push_back(
                std::make_pair<OString, OString>("posrelv", OString::number(2)));
            m_rExport.Strm()
                .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SHPBYPARA)
                .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SHPBYIGNORE);
            break;
    }

    switch (rFlyVert.GetVertOrient())
    {
        case text::VertOrientation::TOP:
        case text::VertOrientation::LINE_TOP:
            m_aFlyProperties.push_back(
                std::make_pair<OString, OString>("posv", OString::number(1)));
            break;
        case text::VertOrientation::BOTTOM:
        case text::VertOrientation::LINE_BOTTOM:
            m_aFlyProperties.push_back(
                std::make_pair<OString, OString>("posv", OString::number(3)));
            break;
        case text::VertOrientation::CENTER:
        case text::VertOrientation::LINE_CENTER:
            m_aFlyProperties.push_back(
                std::make_pair<OString, OString>("posv", OString::number(2)));
            break;
        default:
            break;
    }

    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SHPTOP);
    m_rExport.OutLong(rFlyVert.GetPos());
    if (m_pFlyFrameSize)
    {
        m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SHPBOTTOM);
        m_rExport.OutLong(rFlyVert.GetPos() + m_pFlyFrameSize->Height());
    }
}

void RtfAttributeOutput::FormatHorizOrientation(const SwFormatHoriOrient& rFlyHori)
{
    if (!(m_rExport.m_bOutFlyFrameAttrs && m_rExport.GetRTFFlySyntax()))
        return;

    switch (rFlyHori.GetRelationOrient())
    {
        case text::RelOrientation::PAGE_FRAME:
            m_aFlyProperties.push_back(
                std::make_pair<OString, OString>("posrelh", OString::number(1)));
            break;
        default:
            m_aFlyProperties.push_back(
                std::make_pair<OString, OString>("posrelh", OString::number(2)));
            m_rExport.Strm()
                .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SHPBXCOLUMN)
                .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SHPBXIGNORE);
            break;
    }

    switch (rFlyHori.GetHoriOrient())
    {
        case text::HoriOrientation::LEFT:
            m_aFlyProperties.push_back(
                std::make_pair<OString, OString>("posh", OString::number(1)));
            break;
        case text::HoriOrientation::CENTER:
            m_aFlyProperties.push_back(
                std::make_pair<OString, OString>("posh", OString::number(2)));
            break;
        case text::HoriOrientation::RIGHT:
            m_aFlyProperties.push_back(
                std::make_pair<OString, OString>("posh", OString::number(3)));
            break;
        default:
            break;
    }

    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SHPLEFT);
    m_rExport.OutLong(rFlyHori.GetPos());
    if (m_pFlyFrameSize)
    {
        m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SHPRIGHT);
        m_rExport.OutLong(rFlyHori.GetPos() + m_pFlyFrameSize->Width());
    }
}

void RtfAttributeOutput::FormatAnchor(const SwFormatAnchor& rAnchor)
{
    if (m_rExport.GetRTFFlySyntax())
        return;

    RndStdIds eId = rAnchor.GetAnchorId();
    m_aRunText->append(OOO_STRING_SVTOOLS_RTF_FLYANCHOR);
    m_aRunText->append(static_cast<sal_Int32>(eId));
    switch (eId)
    {
        case RndStdIds::FLY_AT_PAGE:
            m_aRunText->append(OOO_STRING_SVTOOLS_RTF_FLYPAGE);
            m_aRunText->append(static_cast<sal_Int32>(rAnchor.GetPageNum()));
            break;
        case RndStdIds::FLY_AT_PARA:
        case RndStdIds::FLY_AS_CHAR:
            m_aRunText->append(OOO_STRING_SVTOOLS_RTF_FLYCNTNT);
            break;
        default:
            break;
    }
}

void RtfAttributeOutput::FormatBackground(const SvxBrushItem& rBrush)
{
    if (m_rExport.GetRTFFlySyntax())
    {
        const Color& rColor = rBrush.GetColor();
        // We in fact need RGB to BGR, but the transformation is symmetric.
        m_aFlyProperties.push_back(std::make_pair<OString, OString>(
            "fillColor", OString::number(wwUtility::RGBToBGR(rColor))));
    }
    else if (!rBrush.GetColor().GetTransparency())
    {
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CBPAT);
        m_aStyles.append(static_cast<sal_Int32>(m_rExport.GetColor(rBrush.GetColor())));
    }
}

void RtfAttributeOutput::FormatFillStyle(const XFillStyleItem& rFillStyle)
{
    m_oFillStyle = rFillStyle.GetValue();
}

void RtfAttributeOutput::FormatFillGradient(const XFillGradientItem& rFillGradient)
{
    if (*m_oFillStyle != drawing::FillStyle_GRADIENT)
        return;

    m_aFlyProperties.push_back(std::make_pair<OString, OString>(
        "fillType", OString::number(7))); // Shade using the fillAngle

    const XGradient& rGradient = rFillGradient.GetGradientValue();
    const Color& rStartColor = rGradient.GetStartColor();
    m_aFlyProperties.push_back(std::make_pair<OString, OString>(
        "fillBackColor", OString::number(wwUtility::RGBToBGR(rStartColor))));

    const Color& rEndColor = rGradient.GetEndColor();
    m_aFlyProperties.push_back(std::make_pair<OString, OString>(
        "fillColor", OString::number(wwUtility::RGBToBGR(rEndColor))));

    switch (rGradient.GetGradientStyle())
    {
        case css::awt::GradientStyle_LINEAR:
            break;
        case css::awt::GradientStyle_AXIAL:
            m_aFlyProperties.push_back(
                std::make_pair<OString, OString>("fillFocus", OString::number(50)));
            break;
        case css::awt::GradientStyle_RADIAL:
        case css::awt::GradientStyle_ELLIPTICAL:
        case css::awt::GradientStyle_SQUARE:
        case css::awt::GradientStyle_RECT:
        default:
            break;
    }
}

void RtfAttributeOutput::FormatBox(const SvxBoxItem& rBox)
{
    static const SvxBoxItemLine aBorders[] = { SvxBoxItemLine::TOP, SvxBoxItemLine::LEFT,
                                               SvxBoxItemLine::BOTTOM, SvxBoxItemLine::RIGHT };
    static const char* aBorderNames[]
        = { OOO_STRING_SVTOOLS_RTF_BRDRT, OOO_STRING_SVTOOLS_RTF_BRDRL,
            OOO_STRING_SVTOOLS_RTF_BRDRB, OOO_STRING_SVTOOLS_RTF_BRDRR };

    sal_uInt16 const nDist = rBox.GetSmallestDistance();

    if (m_rExport.GetRTFFlySyntax())
    {
        // Borders: spacing to contents, convert from twips to EMUs.
        m_aFlyProperties.push_back(std::make_pair<OString, OString>(
            "dxTextLeft", OString::number(rBox.GetDistance(SvxBoxItemLine::LEFT) * 635)));
        m_aFlyProperties.push_back(std::make_pair<OString, OString>(
            "dyTextTop", OString::number(rBox.GetDistance(SvxBoxItemLine::TOP) * 635)));
        m_aFlyProperties.push_back(std::make_pair<OString, OString>(
            "dxTextRight", OString::number(rBox.GetDistance(SvxBoxItemLine::RIGHT) * 635)));
        m_aFlyProperties.push_back(std::make_pair<OString, OString>(
            "dyTextBottom", OString::number(rBox.GetDistance(SvxBoxItemLine::BOTTOM) * 635)));

        const editeng::SvxBorderLine* pLeft = rBox.GetLine(SvxBoxItemLine::LEFT);
        const editeng::SvxBorderLine* pRight = rBox.GetLine(SvxBoxItemLine::RIGHT);
        const editeng::SvxBorderLine* pTop = rBox.GetLine(SvxBoxItemLine::TOP);
        const editeng::SvxBorderLine* pBottom = rBox.GetLine(SvxBoxItemLine::BOTTOM);
        if (pLeft && pRight && pTop && pBottom && *pLeft == *pRight && *pLeft == *pTop
            && *pLeft == *pBottom)
        {
            const Color& rColor = pTop->GetColor();
            // We in fact need RGB to BGR, but the transformation is symmetric.
            m_aFlyProperties.push_back(std::make_pair<OString, OString>(
                "lineColor", OString::number(wwUtility::RGBToBGR(rColor))));

            if (pTop->GetBorderLineStyle() != SvxBorderLineStyle::NONE)
            {
                double const fConverted(editeng::ConvertBorderWidthToWord(
                    pTop->GetBorderLineStyle(), pTop->GetWidth()));
                sal_Int32 nWidth = fConverted * 635; // Twips -> EMUs
                m_aFlyProperties.push_back(
                    std::make_pair<OString, OString>("lineWidth", OString::number(nWidth)));
            }
            else
                // No border: no line.
                m_aFlyProperties.push_back(std::make_pair<OString, OString>("fLine", "0"));
        }

        return;
    }

    if (rBox.GetTop() && rBox.GetBottom() && rBox.GetLeft() && rBox.GetRight()
        && *rBox.GetTop() == *rBox.GetBottom() && *rBox.GetTop() == *rBox.GetLeft()
        && *rBox.GetTop() == *rBox.GetRight() && nDist == rBox.GetDistance(SvxBoxItemLine::TOP)
        && nDist == rBox.GetDistance(SvxBoxItemLine::LEFT)
        && nDist == rBox.GetDistance(SvxBoxItemLine::BOTTOM)
        && nDist == rBox.GetDistance(SvxBoxItemLine::RIGHT))
        m_aSectionBreaks.append(
            OutBorderLine(m_rExport, rBox.GetTop(), OOO_STRING_SVTOOLS_RTF_BOX, nDist));
    else
    {
        SvxShadowLocation eShadowLocation = SvxShadowLocation::NONE;
        if (const SfxPoolItem* pItem = GetExport().HasItem(RES_SHADOW))
            eShadowLocation = static_cast<const SvxShadowItem*>(pItem)->GetLocation();

        const SvxBoxItemLine* pBrd = aBorders;
        const char** pBrdNms = aBorderNames;
        for (int i = 0; i < 4; ++i, ++pBrd, ++pBrdNms)
        {
            if (const editeng::SvxBorderLine* pLn = rBox.GetLine(*pBrd))
            {
                m_aSectionBreaks.append(OutBorderLine(m_rExport, pLn, *pBrdNms,
                                                      rBox.GetDistance(*pBrd), eShadowLocation));
            }
        }
    }

    if (!m_bBufferSectionBreaks)
        m_aStyles.append(m_aSectionBreaks.makeStringAndClear());
}

void RtfAttributeOutput::FormatColumns_Impl(sal_uInt16 nCols, const SwFormatCol& rCol, bool bEven,
                                            SwTwips nPageSize)
{
    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_COLS);
    m_rExport.OutLong(nCols);

    if (rCol.GetLineAdj() != COLADJ_NONE)
        m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LINEBETCOL);

    if (bEven)
    {
        m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_COLSX);
        m_rExport.OutLong(rCol.GetGutterWidth(true));
    }
    else
    {
        const SwColumns& rColumns = rCol.GetColumns();
        for (sal_uInt16 n = 0; n < nCols;)
        {
            m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_COLNO);
            m_rExport.OutLong(n + 1);

            m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_COLW);
            m_rExport.OutLong(rCol.CalcPrtColWidth(n, nPageSize));

            if (++n != nCols)
            {
                m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_COLSR);
                m_rExport.OutLong(rColumns[n - 1].GetRight() + rColumns[n].GetLeft());
            }
        }
    }
}

void RtfAttributeOutput::FormatKeep(const SvxFormatKeepItem& rItem)
{
    if (rItem.GetValue())
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_KEEPN);
}

void RtfAttributeOutput::FormatTextGrid(const SwTextGridItem& /*rGrid*/)
{
    SAL_INFO("sw.rtf", "TODO: " << __func__);
}

void RtfAttributeOutput::FormatLineNumbering(const SwFormatLineNumber& rNumbering)
{
    if (!rNumbering.IsCount())
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_NOLINE);
}

void RtfAttributeOutput::FormatFrameDirection(const SvxFrameDirectionItem& rDirection)
{
    SvxFrameDirection nDir = rDirection.GetValue();
    if (nDir == SvxFrameDirection::Environment)
        nDir = GetExport().GetDefaultFrameDirection();

    if (m_rExport.m_bOutPageDescs)
    {
        if (nDir == SvxFrameDirection::Vertical_RL_TB)
        {
            m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_STEXTFLOW);
            m_aSectionBreaks.append(static_cast<sal_Int32>(1));
            if (!m_bBufferSectionBreaks)
                m_rExport.Strm().WriteOString(m_aSectionBreaks.makeStringAndClear());
        }
        return;
    }

    if (m_rExport.GetRTFFlySyntax())
    {
        if (nDir == SvxFrameDirection::Vertical_RL_TB)
        {
            // Top to bottom non-ASCII font
            m_aFlyProperties.push_back(std::make_pair<OString, OString>("txflTextFlow", "3"));
        }
        else if (rDirection.GetValue() == SvxFrameDirection::Vertical_LR_BT)
        {
            // Bottom to top non-ASCII font
            m_aFlyProperties.push_back(std::make_pair<OString, OString>("txflTextFlow", "2"));
        }
        return;
    }

    if (nDir == SvxFrameDirection::Horizontal_RL_TB)
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_RTLPAR);
    else
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_LTRPAR);
}

void RtfAttributeOutput::ParaGrabBag(const SfxGrabBagItem& rItem)
{
    const std::map<OUString, css::uno::Any>& rMap = rItem.GetGrabBag();
    for (const auto& rValue : rMap)
    {
        if (rValue.first == "ParaTopMarginBeforeAutoSpacing")
        {
            m_bParaBeforeAutoSpacing = true;
            rValue.second >>= m_nParaBeforeSpacing;
            m_nParaBeforeSpacing = convertMm100ToTwip(m_nParaBeforeSpacing);
        }
        else if (rValue.first == "ParaBottomMarginAfterAutoSpacing")
        {
            m_bParaAfterAutoSpacing = true;
            rValue.second >>= m_nParaAfterSpacing;
            m_nParaAfterSpacing = convertMm100ToTwip(m_nParaAfterSpacing);
        }
    }
}

void RtfAttributeOutput::CharGrabBag(const SfxGrabBagItem& /*rItem*/) {}

void RtfAttributeOutput::ParaOutlineLevel(const SfxUInt16Item& /*rItem*/) {}

void RtfAttributeOutput::WriteExpand(const SwField* pField)
{
    OUString sCmd; // for optional Parameters
    switch (pField->GetTyp()->Which())
    {
        //#i119803# Export user field for RTF filter
        case SwFieldIds::User:
            sCmd = pField->GetTyp()->GetName();
            m_rExport.OutputField(pField, ww::eNONE, sCmd);
            break;
        default:
            m_rExport.OutputField(pField, ww::eUNKNOWN, sCmd);
            break;
    }
}

void RtfAttributeOutput::RefField(const SwField& /*rField*/, const OUString& /*rRef*/)
{
    SAL_INFO("sw.rtf", "TODO: " << __func__);
}

void RtfAttributeOutput::HiddenField(const SwField& /*rField*/)
{
    SAL_INFO("sw.rtf", "TODO: " << __func__);
}

void RtfAttributeOutput::SetField(const SwField& /*rField*/, ww::eField /*eType*/,
                                  const OUString& /*rCmd*/)
{
    SAL_INFO("sw.rtf", "TODO: " << __func__);
}

void RtfAttributeOutput::PostitField(const SwField* pField)
{
    const SwPostItField& rPField = *static_cast<const SwPostItField*>(pField);

    OString aName = OUStringToOString(rPField.GetName(), RTL_TEXTENCODING_UTF8);
    auto it = m_rOpenedAnnotationMarksIds.find(aName);
    if (it != m_rOpenedAnnotationMarksIds.end())
    {
        // In case this field is inside annotation marks, we want to write the
        // annotation itself after the annotation mark is closed, not here.
        m_aPostitFields[it->second] = &rPField;
        return;
    }

    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ATNID " ");
    m_aRunText->append(OUStringToOString(rPField.GetInitials(), m_rExport.GetCurrentEncoding()));
    m_aRunText->append("}");
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ATNAUTHOR " ");
    m_aRunText->append(OUStringToOString(rPField.GetPar1(), m_rExport.GetCurrentEncoding()));
    m_aRunText->append("}");
    m_aRunText->append(OOO_STRING_SVTOOLS_RTF_CHATN);

    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ANNOTATION);

    if (m_nCurrentAnnotationMarkId != -1)
    {
        m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ATNREF " ");
        m_aRunText->append(m_nCurrentAnnotationMarkId);
        m_aRunText->append('}');
    }
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ATNDATE " ");
    m_aRunText->append(static_cast<sal_Int32>(sw::ms::DateTime2DTTM(rPField.GetDateTime())));
    m_aRunText->append('}');
    if (const OutlinerParaObject* pObject = rPField.GetTextObject())
        m_rExport.SdrExporter().WriteOutliner(*pObject, TXT_ATN);
    m_aRunText->append('}');
}

bool RtfAttributeOutput::DropdownField(const SwField* /*pField*/)
{
    // this is handled in OutputFlyFrame_Impl()
    return true;
}

bool RtfAttributeOutput::PlaceholderField(const SwField* pField)
{
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_FIELD
                       "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FLDINST
                       " MACROBUTTON  None ");
    RunText(pField->GetPar1());
    m_aRunText->append("}}");
    return false; // do not expand
}

RtfAttributeOutput::RtfAttributeOutput(RtfExport& rExport)
    : AttributeOutputBase("") // ConvertURL isn't used now in RTF output
    , m_rExport(rExport)
    , m_pPrevPageDesc(nullptr)
    , m_nStyleId(0)
    , m_nListId(0)
    , m_bIsRTL(false)
    , m_nScript(i18n::ScriptType::LATIN)
    , m_bControlLtrRtl(false)
    , m_nNextAnnotationMarkId(0)
    , m_nCurrentAnnotationMarkId(-1)
    , m_bTableCellOpen(false)
    , m_nTableDepth(0)
    , m_bTableAfterCell(false)
    , m_nColBreakNeeded(false)
    , m_bBufferSectionBreaks(false)
    , m_bBufferSectionHeaders(false)
    , m_bLastTable(true)
    , m_bWroteCellInfo(false)
    , m_bTableRowEnded(false)
    , m_bIsBeforeFirstParagraph(true)
    , m_bSingleEmptyRun(false)
    , m_bInRun(false)
    , m_bInRuby(false)
    , m_pFlyFrameSize(nullptr)
    , m_bParaBeforeAutoSpacing(false)
    , m_nParaBeforeSpacing(0)
    , m_bParaAfterAutoSpacing(false)
    , m_nParaAfterSpacing(0)
{
}

RtfAttributeOutput::~RtfAttributeOutput() = default;

MSWordExportBase& RtfAttributeOutput::GetExport() { return m_rExport; }

// These are used by wwFont::WriteRtf()

/// Start the font.
void RtfAttributeOutput::StartFont(const OUString& rFamilyName) const
{
    // write the font name hex-encoded, but without Unicode - Word at least
    // cannot read *both* Unicode and fallback as written by OutString
    m_rExport.Strm().WriteCharPtr(
        msfilter::rtfutil::OutString(rFamilyName, m_rExport.GetCurrentEncoding(), false).getStr());
}

/// End the font.
void RtfAttributeOutput::EndFont() const
{
    m_rExport.Strm().WriteCharPtr(";}");
    m_rExport.SetCurrentEncoding(m_rExport.GetDefaultEncoding());
}

/// Alternate name for the font.
void RtfAttributeOutput::FontAlternateName(const OUString& rName) const
{
    m_rExport.Strm()
        .WriteChar('{')
        .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_IGNORE)
        .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_FALT)
        .WriteChar(' ');
    // write the font name hex-encoded, but without Unicode - Word at least
    // cannot read *both* Unicode and fallback as written by OutString
    m_rExport.Strm()
        .WriteCharPtr(
            msfilter::rtfutil::OutString(rName, m_rExport.GetCurrentEncoding(), false).getStr())
        .WriteChar('}');
}

/// Font charset.
void RtfAttributeOutput::FontCharset(sal_uInt8 nCharSet) const
{
    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_FCHARSET);
    m_rExport.OutULong(nCharSet);
    m_rExport.Strm().WriteChar(' ');
    m_rExport.SetCurrentEncoding(rtl_getTextEncodingFromWindowsCharset(nCharSet));
}

/// Font family.
void RtfAttributeOutput::FontFamilyType(FontFamily eFamily, const wwFont& rFont) const
{
    m_rExport.Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_F);

    const char* pStr = OOO_STRING_SVTOOLS_RTF_FNIL;
    switch (eFamily)
    {
        case FAMILY_ROMAN:
            pStr = OOO_STRING_SVTOOLS_RTF_FROMAN;
            break;
        case FAMILY_SWISS:
            pStr = OOO_STRING_SVTOOLS_RTF_FSWISS;
            break;
        case FAMILY_MODERN:
            pStr = OOO_STRING_SVTOOLS_RTF_FMODERN;
            break;
        case FAMILY_SCRIPT:
            pStr = OOO_STRING_SVTOOLS_RTF_FSCRIPT;
            break;
        case FAMILY_DECORATIVE:
            pStr = OOO_STRING_SVTOOLS_RTF_FDECOR;
            break;
        default:
            break;
    }
    m_rExport.OutULong(m_rExport.m_aFontHelper.GetId(rFont)).WriteCharPtr(pStr);
}

/// Font pitch.
void RtfAttributeOutput::FontPitchType(FontPitch ePitch) const
{
    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_FPRQ);

    sal_uInt16 nVal = 0;
    switch (ePitch)
    {
        case PITCH_FIXED:
            nVal = 1;
            break;
        case PITCH_VARIABLE:
            nVal = 2;
            break;
        default:
            break;
    }
    m_rExport.OutULong(nVal);
}

static void lcl_AppendSP(OStringBuffer& rBuffer, const char cName[], const OUString& rValue,
                         const RtfExport& rExport)
{
    rBuffer.append("{" OOO_STRING_SVTOOLS_RTF_SP "{"); // "{\sp{"
    rBuffer.append(OOO_STRING_SVTOOLS_RTF_SN " "); //" \sn "
    rBuffer.append(cName); //"PropName"
    rBuffer.append("}{" OOO_STRING_SVTOOLS_RTF_SV " ");
    // "}{ \sv "
    rBuffer.append(msfilter::rtfutil::OutString(rValue, rExport.GetCurrentEncoding()));
    rBuffer.append("}}");
}

static OString ExportPICT(const SwFlyFrameFormat* pFlyFrameFormat, const Size& rOrig,
                          const Size& rRendered, const Size& rMapped, const SwCropGrf& rCr,
                          const char* pBLIPType, const sal_uInt8* pGraphicAry, sal_uInt64 nSize,
                          const RtfExport& rExport, SvStream* pStream = nullptr,
                          bool bWritePicProp = true, const SwAttrSet* pAttrSet = nullptr)
{
    OStringBuffer aRet;
    if (pBLIPType && nSize && pGraphicAry)
    {
        bool bIsWMF = std::strcmp(pBLIPType, OOO_STRING_SVTOOLS_RTF_WMETAFILE) == 0;

        aRet.append("{" OOO_STRING_SVTOOLS_RTF_PICT);

        if (pFlyFrameFormat && bWritePicProp)
        {
            OUString sDescription = pFlyFrameFormat->GetObjDescription();
            //write picture properties - wzDescription at first
            //looks like: "{\*\picprop{\sp{\sn PropertyName}{\sv PropertyValue}}}"
            aRet.append(
                "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_PICPROP); //"{\*\picprop
            lcl_AppendSP(aRet, "wzDescription", sDescription, rExport);
            OUString sName = pFlyFrameFormat->GetObjTitle();
            lcl_AppendSP(aRet, "wzName", sName, rExport);

            if (pAttrSet)
            {
                MirrorGraph eMirror = pAttrSet->Get(RES_GRFATR_MIRRORGRF).GetValue();
                if (eMirror == MirrorGraph::Vertical || eMirror == MirrorGraph::Both)
                    // Mirror on the vertical axis is a horizontal flip.
                    lcl_AppendSP(aRet, "fFlipH", "1", rExport);
            }

            aRet.append("}"); //"}"
        }

        tools::Long nXCroppedSize = rOrig.Width() - (rCr.GetLeft() + rCr.GetRight());
        tools::Long nYCroppedSize = rOrig.Height() - (rCr.GetTop() + rCr.GetBottom());
        /* Graphic with a zero height or width, typically copied from webpages, caused crashes. */
        if (!nXCroppedSize)
            nXCroppedSize = 100;
        if (!nYCroppedSize)
            nYCroppedSize = 100;

        //Given the original size and taking cropping into account
        //first, how much has the original been scaled to get the
        //final rendered size
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICSCALEX);
        aRet.append(static_cast<sal_Int32>((100 * rRendered.Width()) / nXCroppedSize));
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICSCALEY);
        aRet.append(static_cast<sal_Int32>((100 * rRendered.Height()) / nYCroppedSize));

        aRet.append(OOO_STRING_SVTOOLS_RTF_PICCROPL);
        aRet.append(rCr.GetLeft());
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICCROPR);
        aRet.append(rCr.GetRight());
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICCROPT);
        aRet.append(rCr.GetTop());
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICCROPB);
        aRet.append(rCr.GetBottom());

        aRet.append(OOO_STRING_SVTOOLS_RTF_PICW);
        aRet.append(static_cast<sal_Int32>(rMapped.Width()));
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICH);
        aRet.append(static_cast<sal_Int32>(rMapped.Height()));

        aRet.append(OOO_STRING_SVTOOLS_RTF_PICWGOAL);
        aRet.append(static_cast<sal_Int32>(rOrig.Width()));
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICHGOAL);
        aRet.append(static_cast<sal_Int32>(rOrig.Height()));

        aRet.append(pBLIPType);
        if (bIsWMF)
        {
            aRet.append(sal_Int32(8));
            msfilter::rtfutil::StripMetafileHeader(pGraphicAry, nSize);
        }
        aRet.append(SAL_NEWLINE_STRING);
        if (pStream)
            pStream->WriteOString(aRet.makeStringAndClear());
        if (pStream)
            msfilter::rtfutil::WriteHex(pGraphicAry, nSize, pStream);
        else
            aRet.append(msfilter::rtfutil::WriteHex(pGraphicAry, nSize));
        aRet.append('}');
        if (pStream)
            pStream->WriteOString(aRet.makeStringAndClear());
    }
    return aRet.makeStringAndClear();
}

void RtfAttributeOutput::FlyFrameOLEReplacement(const SwFlyFrameFormat* pFlyFrameFormat,
                                                SwOLENode& rOLENode, const Size& rSize)
{
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_SHPPICT);
    Size aSize(rOLENode.GetTwipSize());
    Size aRendered(aSize);
    aRendered.setWidth(rSize.Width());
    aRendered.setHeight(rSize.Height());
    const Graphic* pGraphic = rOLENode.GetGraphic();
    Size aMapped(pGraphic->GetPrefSize());
    auto& rCr = static_cast<const SwCropGrf&>(rOLENode.GetAttr(RES_GRFATR_CROPGRF));
    const char* pBLIPType = OOO_STRING_SVTOOLS_RTF_PNGBLIP;
    const sal_uInt8* pGraphicAry = nullptr;
    SvMemoryStream aStream;
    if (GraphicConverter::Export(aStream, *pGraphic, ConvertDataFormat::PNG) != ERRCODE_NONE)
        SAL_WARN("sw.rtf", "failed to export the graphic");
    sal_uInt32 nSize = aStream.TellEnd();
    pGraphicAry = static_cast<sal_uInt8 const*>(aStream.GetData());
    m_aRunText->append(ExportPICT(pFlyFrameFormat, aSize, aRendered, aMapped, rCr, pBLIPType,
                                  pGraphicAry, nSize, m_rExport));
    m_aRunText->append("}"); // shppict
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_NONSHPPICT);
    pBLIPType = OOO_STRING_SVTOOLS_RTF_WMETAFILE;
    SvMemoryStream aWmfStream;
    if (GraphicConverter::Export(aWmfStream, *pGraphic, ConvertDataFormat::WMF) != ERRCODE_NONE)
        SAL_WARN("sw.rtf", "failed to export the graphic");
    nSize = aWmfStream.TellEnd();
    pGraphicAry = static_cast<sal_uInt8 const*>(aWmfStream.GetData());
    m_aRunText->append(ExportPICT(pFlyFrameFormat, aSize, aRendered, aMapped, rCr, pBLIPType,
                                  pGraphicAry, nSize, m_rExport));
    m_aRunText->append("}"); // nonshppict
}

bool RtfAttributeOutput::FlyFrameOLEMath(const SwFlyFrameFormat* pFlyFrameFormat,
                                         SwOLENode& rOLENode, const Size& rSize)
{
    uno::Reference<embed::XEmbeddedObject> xObj(rOLENode.GetOLEObj().GetOleRef());
    sal_Int64 nAspect = rOLENode.GetAspect();
    svt::EmbeddedObjectRef aObjRef(xObj, nAspect);
    SvGlobalName aObjName(aObjRef->getClassID());

    if (!SotExchange::IsMath(aObjName))
        return false;

    m_aRunText->append("{" LO_STRING_SVTOOLS_RTF_MMATH " ");
    uno::Reference<util::XCloseable> xClosable = xObj->getComponent();
    if (!xClosable.is())
        return false;
    // gcc4.4 (and 4.3 and possibly older) have a problem with dynamic_cast directly to the target class,
    // so help it with an intermediate cast. I'm not sure what exactly the problem is, seems to be unrelated
    // to RTLD_GLOBAL, so most probably a gcc bug.
    auto pBase
        = dynamic_cast<oox::FormulaExportBase*>(dynamic_cast<SfxBaseModel*>(xClosable.get()));
    assert(pBase != nullptr);
    OStringBuffer aBuf;
    if (pBase)
        pBase->writeFormulaRtf(aBuf, m_rExport.GetCurrentEncoding());
    m_aRunText->append(aBuf.makeStringAndClear());
    // Replacement graphic.
    m_aRunText->append("{" LO_STRING_SVTOOLS_RTF_MMATHPICT " ");
    FlyFrameOLEReplacement(pFlyFrameFormat, rOLENode, rSize);
    m_aRunText->append("}"); // mmathPict
    m_aRunText->append("}"); // mmath

    return true;
}

void RtfAttributeOutput::FlyFrameOLE(const SwFlyFrameFormat* pFlyFrameFormat, SwOLENode& rOLENode,
                                     const Size& rSize)
{
    if (FlyFrameOLEMath(pFlyFrameFormat, rOLENode, rSize))
        return;

    FlyFrameOLEReplacement(pFlyFrameFormat, rOLENode, rSize);
}

void RtfAttributeOutput::FlyFrameGraphic(const SwFlyFrameFormat* pFlyFrameFormat,
                                         const SwGrfNode* pGrfNode)
{
    SvMemoryStream aStream;
    const sal_uInt8* pGraphicAry = nullptr;
    sal_uInt32 nSize = 0;

    const Graphic& rGraphic(pGrfNode->GetGrf());

    // If there is no graphic there is not much point in parsing it
    if (rGraphic.GetType() == GraphicType::NONE)
        return;

    ConvertDataFormat aConvertDestinationFormat = ConvertDataFormat::WMF;
    const char* pConvertDestinationBLIPType = OOO_STRING_SVTOOLS_RTF_WMETAFILE;

    GfxLink aGraphicLink;
    const char* pBLIPType = nullptr;
    if (rGraphic.IsGfxLink())
    {
        aGraphicLink = rGraphic.GetGfxLink();
        nSize = aGraphicLink.GetDataSize();
        pGraphicAry = aGraphicLink.GetData();
        switch (aGraphicLink.GetType())
        {
            // #i15508# trying to add BMP type for better exports, need to check if this works
            // checked, does not work. Also need to reset pGraphicAry to NULL to force conversion
            // to PNG, else the BMP array will be used.
            // It may work using direct DIB data, but that needs to be checked eventually
            //
            // #i15508# before GfxLinkType::NativeBmp was added the graphic data
            // (to be hold in pGraphicAry) was not available; thus for now to stay
            // compatible, keep it that way by assigning NULL value to pGraphicAry
            case GfxLinkType::NativeBmp:
                //    pBLIPType = OOO_STRING_SVTOOLS_RTF_WBITMAP;
                pGraphicAry = nullptr;
                break;

            case GfxLinkType::NativeJpg:
                pBLIPType = OOO_STRING_SVTOOLS_RTF_JPEGBLIP;
                break;
            case GfxLinkType::NativePng:
                pBLIPType = OOO_STRING_SVTOOLS_RTF_PNGBLIP;
                break;
            case GfxLinkType::NativeWmf:
                pBLIPType = aGraphicLink.IsEMF() ? OOO_STRING_SVTOOLS_RTF_EMFBLIP
                                                 : OOO_STRING_SVTOOLS_RTF_WMETAFILE;
                break;
            case GfxLinkType::NativeGif:
                // GIF is not supported by RTF, but we override default conversion to WMF, PNG seems fits better here.
                aConvertDestinationFormat = ConvertDataFormat::PNG;
                pConvertDestinationBLIPType = OOO_STRING_SVTOOLS_RTF_PNGBLIP;
                break;
            default:
                break;
        }
    }

    GraphicType eGraphicType = rGraphic.GetType();
    if (!pGraphicAry)
    {
        if (ERRCODE_NONE
            == GraphicConverter::Export(aStream, rGraphic,
                                        (eGraphicType == GraphicType::Bitmap)
                                            ? ConvertDataFormat::PNG
                                            : ConvertDataFormat::WMF))
        {
            pBLIPType = (eGraphicType == GraphicType::Bitmap) ? OOO_STRING_SVTOOLS_RTF_PNGBLIP
                                                              : OOO_STRING_SVTOOLS_RTF_WMETAFILE;
            nSize = aStream.TellEnd();
            pGraphicAry = static_cast<sal_uInt8 const*>(aStream.GetData());
        }
    }

    Size aMapped(eGraphicType == GraphicType::Bitmap ? rGraphic.GetSizePixel()
                                                     : rGraphic.GetPrefSize());

    auto& rCr = static_cast<const SwCropGrf&>(pGrfNode->GetAttr(RES_GRFATR_CROPGRF));

    //Get original size in twips
    Size aSize(pGrfNode->GetTwipSize());
    Size aRendered(aSize);

    const SwFormatFrameSize& rS = pFlyFrameFormat->GetFrameSize();
    aRendered.setWidth(rS.GetWidth());
    aRendered.setHeight(rS.GetHeight());

    ww8::Frame* pFrame = nullptr;
    for (auto& rFrame : m_rExport.m_aFrames)
    {
        if (pFlyFrameFormat == &rFrame.GetFrameFormat())
        {
            pFrame = &rFrame;
            break;
        }
    }

    /*
       If the graphic is not of type WMF then we will have to store two
       graphics, one in the native format wrapped in shppict, and the other in
       the wmf format wrapped in nonshppict, so as to keep wordpad happy. If its
       a wmf already then we don't need any such wrapping
       */
    bool bIsWMF = pBLIPType && std::strcmp(pBLIPType, OOO_STRING_SVTOOLS_RTF_WMETAFILE) == 0;
    const SwAttrSet* pAttrSet = pGrfNode->GetpSwAttrSet();
    if (!pFrame || pFrame->IsInline())
    {
        if (!bIsWMF)
            m_rExport.Strm().WriteCharPtr(
                "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_SHPPICT);
    }
    else
    {
        m_rExport.Strm().WriteCharPtr(
            "{" OOO_STRING_SVTOOLS_RTF_SHP
            "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_SHPINST);
        m_pFlyFrameSize = &aRendered;
        m_rExport.m_pParentFrame = pFrame;
        m_rExport.m_bOutFlyFrameAttrs = true;
        m_rExport.SetRTFFlySyntax(true);
        m_rExport.OutputFormat(pFrame->GetFrameFormat(), false, false, true);
        m_rExport.m_bOutFlyFrameAttrs = false;
        m_rExport.SetRTFFlySyntax(false);
        m_rExport.m_pParentFrame = nullptr;
        m_pFlyFrameSize = nullptr;

        std::vector<std::pair<OString, OString>> aFlyProperties;
        aFlyProperties.push_back(std::make_pair<OString, OString>(
            "shapeType", OString::number(ESCHER_ShpInst_PictureFrame)));
        aFlyProperties.push_back(std::make_pair<OString, OString>(
            "wzDescription", msfilter::rtfutil::OutString(pFlyFrameFormat->GetObjDescription(),
                                                          m_rExport.GetCurrentEncoding())));
        aFlyProperties.push_back(std::make_pair<OString, OString>(
            "wzName", msfilter::rtfutil::OutString(pFlyFrameFormat->GetObjTitle(),
                                                   m_rExport.GetCurrentEncoding())));

        // If we have a wrap polygon, then handle that here.
        if (pFlyFrameFormat->GetSurround().IsContour())
        {
            if (const SwNoTextNode* pNd
                = sw::util::GetNoTextNodeFromSwFrameFormat(*pFlyFrameFormat))
            {
                const tools::PolyPolygon* pPolyPoly = pNd->HasContour();
                if (pPolyPoly && pPolyPoly->Count())
                {
                    tools::Polygon aPoly = sw::util::CorrectWordWrapPolygonForExport(
                        *pPolyPoly, pNd, /*bCorrectCrop=*/true);
                    OStringBuffer aVerticies;
                    for (sal_uInt16 i = 0; i < aPoly.GetSize(); ++i)
                        aVerticies.append(";(")
                            .append(aPoly[i].X())
                            .append(",")
                            .append(aPoly[i].Y())
                            .append(")");
                    aFlyProperties.push_back(std::make_pair<OString, OString>(
                        "pWrapPolygonVertices",
                        "8;" + OString::number(aPoly.GetSize()) + aVerticies.makeStringAndClear()));
                }
            }
        }

        // Below text, behind document, opaque: they all refer to the same thing.
        if (!pFlyFrameFormat->GetOpaque().GetValue())
            aFlyProperties.push_back(std::make_pair<OString, OString>("fBehindDocument", "1"));

        if (pAttrSet)
        {
            if (sal_Int32 nRot = pAttrSet->Get(RES_GRFATR_ROTATION).GetValue())
            {
                // See writerfilter::rtftok::RTFSdrImport::applyProperty(),
                // positive rotation angles are clockwise in RTF, we have them
                // as counter-clockwise.
                // Additionally, RTF type is 0..360*2^16, our is 0..360*10.
                nRot = nRot * -1 * RTF_MULTIPLIER / 10;
                aFlyProperties.emplace_back("rotation", OString::number(nRot));
            }
        }

        for (const std::pair<OString, OString>& rPair : aFlyProperties)
        {
            m_rExport.Strm().WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_SP "{");
            m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SN " ");
            m_rExport.Strm().WriteOString(rPair.first);
            m_rExport.Strm().WriteCharPtr("}{" OOO_STRING_SVTOOLS_RTF_SV " ");
            m_rExport.Strm().WriteOString(rPair.second);
            m_rExport.Strm().WriteCharPtr("}}");
        }
        m_rExport.Strm().WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_SP "{" OOO_STRING_SVTOOLS_RTF_SN
                                      " pib"
                                      "}{" OOO_STRING_SVTOOLS_RTF_SV " ");
    }

    bool bWritePicProp = !pFrame || pFrame->IsInline();
    if (pBLIPType)
        ExportPICT(pFlyFrameFormat, aSize, aRendered, aMapped, rCr, pBLIPType, pGraphicAry, nSize,
                   m_rExport, &m_rExport.Strm(), bWritePicProp, pAttrSet);
    else
    {
        aStream.Seek(0);
        if (GraphicConverter::Export(aStream, rGraphic, aConvertDestinationFormat) != ERRCODE_NONE)
            SAL_WARN("sw.rtf", "failed to export the graphic");
        pBLIPType = pConvertDestinationBLIPType;
        nSize = aStream.TellEnd();
        pGraphicAry = static_cast<sal_uInt8 const*>(aStream.GetData());

        ExportPICT(pFlyFrameFormat, aSize, aRendered, aMapped, rCr, pBLIPType, pGraphicAry, nSize,
                   m_rExport, &m_rExport.Strm(), bWritePicProp, pAttrSet);
    }

    if (!pFrame || pFrame->IsInline())
    {
        if (!bIsWMF)
        {
            m_rExport.Strm().WriteCharPtr("}"
                                          "{" OOO_STRING_SVTOOLS_RTF_NONSHPPICT);

            aStream.Seek(0);
            if (GraphicConverter::Export(aStream, rGraphic, ConvertDataFormat::WMF) != ERRCODE_NONE)
                SAL_WARN("sw.rtf", "failed to export the graphic");
            pBLIPType = OOO_STRING_SVTOOLS_RTF_WMETAFILE;
            nSize = aStream.TellEnd();
            pGraphicAry = static_cast<sal_uInt8 const*>(aStream.GetData());

            ExportPICT(pFlyFrameFormat, aSize, aRendered, aMapped, rCr, pBLIPType, pGraphicAry,
                       nSize, m_rExport, &m_rExport.Strm());

            m_rExport.Strm().WriteChar('}');
        }
    }
    else
        m_rExport.Strm().WriteCharPtr("}}}}"); // Close SV, SP, SHPINST and SHP.

    m_rExport.Strm().WriteCharPtr(SAL_NEWLINE_STRING);
}

void RtfAttributeOutput::BulletDefinition(int /*nId*/, const Graphic& rGraphic, Size aSize)
{
    m_rExport.Strm().WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_SHPPICT);
    m_rExport.Strm().WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_PICT OOO_STRING_SVTOOLS_RTF_PNGBLIP);

    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PICWGOAL);
    m_rExport.OutULong(aSize.Width());
    m_rExport.Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PICHGOAL);
    m_rExport.OutULong(aSize.Height());

    m_rExport.Strm().WriteCharPtr(SAL_NEWLINE_STRING);
    const sal_uInt8* pGraphicAry = nullptr;
    SvMemoryStream aStream;
    if (GraphicConverter::Export(aStream, rGraphic, ConvertDataFormat::PNG) != ERRCODE_NONE)
        SAL_WARN("sw.rtf", "failed to export the numbering picture bullet");
    sal_uInt32 nSize = aStream.TellEnd();
    pGraphicAry = static_cast<sal_uInt8 const*>(aStream.GetData());
    msfilter::rtfutil::WriteHex(pGraphicAry, nSize, &m_rExport.Strm());
    m_rExport.Strm().WriteCharPtr("}}"); // pict, shppict
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
