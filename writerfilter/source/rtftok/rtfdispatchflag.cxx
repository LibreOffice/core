/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include "rtfdocumentimpl.hxx"

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <filter/msfilter/escherex.hxx>

#include <ooxml/resourceids.hxx>

#include <sal/log.hxx>

#include "rtfsdrimport.hxx"
#include "rtfskipdestination.hxx"

using namespace com::sun::star;

namespace writerfilter::rtftok
{
RTFError RTFDocumentImpl::dispatchFlag(RTFKeyword nKeyword)
{
    setNeedSect(true);
    checkUnicode(/*bUnicode =*/true, /*bHex =*/true);
    RTFSkipDestination aSkip(*this);
    int nParam = -1;
    int nSprm = -1;

    // Underline flags.
    switch (nKeyword)
    {
        case RTFKeyword::ULD:
            nSprm = NS_ooxml::LN_Value_ST_Underline_dotted;
            break;
        case RTFKeyword::ULW:
            nSprm = NS_ooxml::LN_Value_ST_Underline_words;
            break;
        default:
            break;
    }
    if (nSprm >= 0)
    {
        auto pValue = new RTFValue(nSprm);
        m_aStates.top().getCharacterAttributes().set(NS_ooxml::LN_CT_Underline_val, pValue);
        return RTFError::OK;
    }

    // Indentation
    switch (nKeyword)
    {
        case RTFKeyword::QC:
            nParam = NS_ooxml::LN_Value_ST_Jc_center;
            break;
        case RTFKeyword::QJ:
            nParam = NS_ooxml::LN_Value_ST_Jc_both;
            break;
        case RTFKeyword::QL:
            nParam = NS_ooxml::LN_Value_ST_Jc_left;
            break;
        case RTFKeyword::QR:
            nParam = NS_ooxml::LN_Value_ST_Jc_right;
            break;
        case RTFKeyword::QD:
            nParam = NS_ooxml::LN_Value_ST_Jc_distribute;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_CT_PPrBase_jc, pValue);
        m_bNeedPap = true;
        return RTFError::OK;
    }

    // Font Alignment
    switch (nKeyword)
    {
        case RTFKeyword::FAFIXED:
        case RTFKeyword::FAAUTO:
            nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_auto;
            break;
        case RTFKeyword::FAHANG:
            nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_top;
            break;
        case RTFKeyword::FACENTER:
            nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_center;
            break;
        case RTFKeyword::FAROMAN:
            nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_baseline;
            break;
        case RTFKeyword::FAVAR:
            nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_bottom;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_CT_PPrBase_textAlignment, pValue);
        return RTFError::OK;
    }

    // Tab kind.
    switch (nKeyword)
    {
        case RTFKeyword::TQR:
            nParam = NS_ooxml::LN_Value_ST_TabJc_right;
            break;
        case RTFKeyword::TQC:
            nParam = NS_ooxml::LN_Value_ST_TabJc_center;
            break;
        case RTFKeyword::TQDEC:
            nParam = NS_ooxml::LN_Value_ST_TabJc_decimal;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        m_aStates.top().getTabAttributes().set(NS_ooxml::LN_CT_TabStop_val, pValue);
        return RTFError::OK;
    }

    // Tab lead.
    switch (nKeyword)
    {
        case RTFKeyword::TLDOT:
            nParam = NS_ooxml::LN_Value_ST_TabTlc_dot;
            break;
        case RTFKeyword::TLMDOT:
            nParam = NS_ooxml::LN_Value_ST_TabTlc_middleDot;
            break;
        case RTFKeyword::TLHYPH:
            nParam = NS_ooxml::LN_Value_ST_TabTlc_hyphen;
            break;
        case RTFKeyword::TLUL:
        case RTFKeyword::TLTH:
            nParam = NS_ooxml::LN_Value_ST_TabTlc_underscore;
            break;
        case RTFKeyword::TLEQ:
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        m_aStates.top().getTabAttributes().set(NS_ooxml::LN_CT_TabStop_leader, pValue);
        return RTFError::OK;
    }

    // Border types
    {
        switch (nKeyword)
        {
            // brdrhair and brdrs are the same, brdrw will make a difference
            // map to values in ooxml/model.xml resource ST_Border
            case RTFKeyword::BRDRHAIR:
            case RTFKeyword::BRDRS:
                nParam = NS_ooxml::LN_Value_ST_Border_single;
                break;
            case RTFKeyword::BRDRDOT:
                nParam = NS_ooxml::LN_Value_ST_Border_dotted;
                break;
            case RTFKeyword::BRDRDASH:
                nParam = NS_ooxml::LN_Value_ST_Border_dashed;
                break;
            case RTFKeyword::BRDRDB:
                nParam = NS_ooxml::LN_Value_ST_Border_double;
                break;
            case RTFKeyword::BRDRTNTHSG:
                nParam = NS_ooxml::LN_Value_ST_Border_thinThickSmallGap;
                break;
            case RTFKeyword::BRDRTNTHMG:
                nParam = NS_ooxml::LN_Value_ST_Border_thinThickMediumGap;
                break;
            case RTFKeyword::BRDRTNTHLG:
                nParam = NS_ooxml::LN_Value_ST_Border_thinThickLargeGap;
                break;
            case RTFKeyword::BRDRTHTNSG:
                nParam = NS_ooxml::LN_Value_ST_Border_thickThinSmallGap;
                break;
            case RTFKeyword::BRDRTHTNMG:
                nParam = NS_ooxml::LN_Value_ST_Border_thickThinMediumGap;
                break;
            case RTFKeyword::BRDRTHTNLG:
                nParam = NS_ooxml::LN_Value_ST_Border_thickThinLargeGap;
                break;
            case RTFKeyword::BRDREMBOSS:
                nParam = NS_ooxml::LN_Value_ST_Border_threeDEmboss;
                break;
            case RTFKeyword::BRDRENGRAVE:
                nParam = NS_ooxml::LN_Value_ST_Border_threeDEngrave;
                break;
            case RTFKeyword::BRDROUTSET:
                nParam = NS_ooxml::LN_Value_ST_Border_outset;
                break;
            case RTFKeyword::BRDRINSET:
                nParam = NS_ooxml::LN_Value_ST_Border_inset;
                break;
            case RTFKeyword::BRDRDASHSM:
                nParam = NS_ooxml::LN_Value_ST_Border_dashSmallGap;
                break;
            case RTFKeyword::BRDRDASHD:
                nParam = NS_ooxml::LN_Value_ST_Border_dotDash;
                break;
            case RTFKeyword::BRDRDASHDD:
                nParam = NS_ooxml::LN_Value_ST_Border_dotDotDash;
                break;
            case RTFKeyword::BRDRNONE:
                nParam = NS_ooxml::LN_Value_ST_Border_none;
                break;
            default:
                break;
        }
        if (nParam >= 0)
        {
            auto pValue = new RTFValue(nParam);
            putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_val, pValue);
            return RTFError::OK;
        }
    }

    // Section breaks
    switch (nKeyword)
    {
        case RTFKeyword::SBKNONE:
            nParam = NS_ooxml::LN_Value_ST_SectionMark_continuous;
            break;
        case RTFKeyword::SBKCOL:
            nParam = NS_ooxml::LN_Value_ST_SectionMark_nextColumn;
            break;
        case RTFKeyword::SBKPAGE:
            nParam = NS_ooxml::LN_Value_ST_SectionMark_nextPage;
            break;
        case RTFKeyword::SBKEVEN:
            nParam = NS_ooxml::LN_Value_ST_SectionMark_evenPage;
            break;
        case RTFKeyword::SBKODD:
            nParam = NS_ooxml::LN_Value_ST_SectionMark_oddPage;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        if (m_nResetBreakOnSectBreak != RTFKeyword::invalid)
        {
            m_nResetBreakOnSectBreak = nKeyword;
        }
        auto pValue = new RTFValue(nParam);
        m_aStates.top().getSectionSprms().set(NS_ooxml::LN_EG_SectPrContents_type, pValue);
        return RTFError::OK;
    }

    // Footnote numbering
    switch (nKeyword)
    {
        case RTFKeyword::FTNNAR:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_decimal;
            break;
        case RTFKeyword::FTNNALC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter;
            break;
        case RTFKeyword::FTNNAUC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperLetter;
            break;
        case RTFKeyword::FTNNRLC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman;
            break;
        case RTFKeyword::FTNNRUC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperRoman;
            break;
        case RTFKeyword::FTNNCHI:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_chicago;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pInner = new RTFValue(nParam);
        RTFSprms aAttributes;
        aAttributes.set(NS_ooxml::LN_CT_NumFmt_val, pInner);
        auto pOuter = new RTFValue(aAttributes);
        putNestedSprm(m_aDefaultState.getParagraphSprms(),
                      NS_ooxml::LN_EG_SectPrContents_footnotePr, NS_ooxml::LN_CT_FtnProps_numFmt,
                      pOuter);
        return RTFError::OK;
    }

    // Footnote restart type
    switch (nKeyword)
    {
        case RTFKeyword::FTNRSTPG:
            nParam = NS_ooxml::LN_Value_ST_RestartNumber_eachPage;
            break;
        case RTFKeyword::FTNRESTART:
            nParam = NS_ooxml::LN_Value_ST_RestartNumber_eachSect;
            break;
        case RTFKeyword::FTNRSTCONT:
            nParam = NS_ooxml::LN_Value_ST_RestartNumber_continuous;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        putNestedSprm(m_aDefaultState.getParagraphSprms(),
                      NS_ooxml::LN_EG_SectPrContents_footnotePr,
                      NS_ooxml::LN_EG_FtnEdnNumProps_numRestart, pValue);
        return RTFError::OK;
    }

    // Endnote numbering
    switch (nKeyword)
    {
        case RTFKeyword::AFTNNAR:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_decimal;
            break;
        case RTFKeyword::AFTNNALC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter;
            break;
        case RTFKeyword::AFTNNAUC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperLetter;
            break;
        case RTFKeyword::AFTNNRLC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman;
            break;
        case RTFKeyword::AFTNNRUC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperRoman;
            break;
        case RTFKeyword::AFTNNCHI:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_chicago;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pInner = new RTFValue(nParam);
        RTFSprms aAttributes;
        aAttributes.set(NS_ooxml::LN_CT_NumFmt_val, pInner);
        auto pOuter = new RTFValue(aAttributes);
        putNestedSprm(m_aDefaultState.getParagraphSprms(), NS_ooxml::LN_EG_SectPrContents_endnotePr,
                      NS_ooxml::LN_CT_EdnProps_numFmt, pOuter);
        return RTFError::OK;
    }

    switch (nKeyword)
    {
        case RTFKeyword::TRQL:
            nParam = NS_ooxml::LN_Value_ST_Jc_left;
            break;
        case RTFKeyword::TRQC:
            nParam = NS_ooxml::LN_Value_ST_Jc_center;
            break;
        case RTFKeyword::TRQR:
            nParam = NS_ooxml::LN_Value_ST_Jc_right;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        m_aStates.top().getTableRowSprms().set(NS_ooxml::LN_CT_TrPrBase_jc, pValue);
        return RTFError::OK;
    }

    // Cell Text Flow
    switch (nKeyword)
    {
        case RTFKeyword::CLTXLRTB:
            nParam = NS_ooxml::LN_Value_ST_TextDirection_lrTb;
            break;
        case RTFKeyword::CLTXTBRL:
            nParam = NS_ooxml::LN_Value_ST_TextDirection_tbRl;
            break;
        case RTFKeyword::CLTXBTLR:
            nParam = NS_ooxml::LN_Value_ST_TextDirection_btLr;
            break;
        case RTFKeyword::CLTXLRTBV:
            nParam = NS_ooxml::LN_Value_ST_TextDirection_lrTbV;
            break;
        case RTFKeyword::CLTXTBRLV:
            nParam = NS_ooxml::LN_Value_ST_TextDirection_tbRlV;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        m_aStates.top().getTableCellSprms().set(NS_ooxml::LN_CT_TcPrBase_textDirection, pValue);
    }

    // Trivial paragraph flags
    switch (nKeyword)
    {
        case RTFKeyword::KEEP:
            if (m_aStates.top().getCurrentBuffer() != &m_aTableBufferStack.back())
                nParam = NS_ooxml::LN_CT_PPrBase_keepLines;
            break;
        case RTFKeyword::KEEPN:
            if (m_aStates.top().getCurrentBuffer() != &m_aTableBufferStack.back())
                nParam = NS_ooxml::LN_CT_PPrBase_keepNext;
            break;
        case RTFKeyword::INTBL:
        {
            m_aStates.top().setCurrentBuffer(&m_aTableBufferStack.back());
            nParam = NS_ooxml::LN_inTbl;
        }
        break;
        case RTFKeyword::PAGEBB:
            nParam = NS_ooxml::LN_CT_PPrBase_pageBreakBefore;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(1);
        m_aStates.top().getParagraphSprms().set(nParam, pValue);
        return RTFError::OK;
    }

    switch (nKeyword)
    {
        case RTFKeyword::FNIL:
        case RTFKeyword::FROMAN:
        case RTFKeyword::FSWISS:
        case RTFKeyword::FMODERN:
        case RTFKeyword::FSCRIPT:
        case RTFKeyword::FDECOR:
        case RTFKeyword::FTECH:
        case RTFKeyword::FBIDI:
            // TODO ooxml:CT_Font_family seems to be ignored by the domain mapper
            break;
        case RTFKeyword::ANSI:
            m_aStates.top().setCurrentEncoding(RTL_TEXTENCODING_MS_1252);
            break;
        case RTFKeyword::MAC:
            m_aDefaultState.setCurrentEncoding(RTL_TEXTENCODING_APPLE_ROMAN);
            m_aStates.top().setCurrentEncoding(m_aDefaultState.getCurrentEncoding());
            break;
        case RTFKeyword::PC:
            m_aDefaultState.setCurrentEncoding(RTL_TEXTENCODING_IBM_437);
            m_aStates.top().setCurrentEncoding(m_aDefaultState.getCurrentEncoding());
            break;
        case RTFKeyword::PCA:
            m_aDefaultState.setCurrentEncoding(RTL_TEXTENCODING_IBM_850);
            m_aStates.top().setCurrentEncoding(m_aDefaultState.getCurrentEncoding());
            break;
        case RTFKeyword::PLAIN:
        {
            m_aStates.top().getCharacterSprms() = getDefaultState().getCharacterSprms();
            m_aStates.top().setCurrentEncoding(getEncoding(getFontIndex(m_nDefaultFontIndex)));
            m_aStates.top().getCharacterAttributes() = getDefaultState().getCharacterAttributes();
            m_aStates.top().setCurrentCharacterStyleIndex(-1);
            m_aStates.top().setRunType(RTFParserState::RunType::NONE);
        }
        break;
        case RTFKeyword::PARD:
        {
            if (m_bHadPicture)
                dispatchSymbol(RTFKeyword::PAR);
            // \pard is allowed between \cell and \row, but in that case it should not reset the fact that we're inside a table.
            // It should not reset the paragraph style, either, so remember the old paragraph style.
            RTFValue::Pointer_t pOldStyle
                = m_aStates.top().getParagraphSprms().find(NS_ooxml::LN_CT_PPrBase_pStyle);
            m_aStates.top().getParagraphSprms() = m_aDefaultState.getParagraphSprms();
            m_aStates.top().getParagraphAttributes() = m_aDefaultState.getParagraphAttributes();

            if (m_nTopLevelCells == 0 && m_nNestedCells == 0)
            {
                // Reset that we're in a table.
                m_aStates.top().setCurrentBuffer(nullptr);
            }
            else
            {
                // We are still in a table.
                m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_inTbl, new RTFValue(1));
                if (m_bAfterCellBeforeRow && pOldStyle)
                    // And we still have the same paragraph style.
                    m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_CT_PPrBase_pStyle,
                                                            pOldStyle);
                // Ideally getDefaultSPRM() would take care of this, but it would not when we're buffering.
                m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_CT_PPrBase_tabs,
                                                        new RTFValue());
            }
            resetFrame();

            // Reset currently selected paragraph style as well, unless we are in the special "after \cell, before \row" state.
            // By default the style with index 0 is applied.
            if (!m_bAfterCellBeforeRow)
            {
                OUString const aName = getStyleName(0);
                // But only in case it's not a character style.
                if (!aName.isEmpty()
                    && getStyleType(0) != NS_ooxml::LN_Value_ST_StyleType_character)
                {
                    m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_CT_PPrBase_pStyle,
                                                            new RTFValue(aName));
                    m_aStates.top().setCurrentStyleIndex(0);
                }
                else
                {
                    m_aStates.top().setCurrentStyleIndex(-1);
                }
            }
            // Need to send paragraph properties again, if there will be any.
            m_bNeedPap = true;
            break;
        }
        case RTFKeyword::SECTD:
        {
            m_aStates.top().getSectionSprms() = m_aDefaultState.getSectionSprms();
            m_aStates.top().getSectionAttributes() = m_aDefaultState.getSectionAttributes();
        }
        break;
        case RTFKeyword::TROWD:
        {
            // Back these up, in case later we still need this info.
            backupTableRowProperties();
            resetTableRowProperties();
            // In case the table definition is in the middle of the row
            // (invalid), make sure table definition is emitted.
            m_bNeedPap = true;
        }
        break;
        case RTFKeyword::WIDCTLPAR:
        case RTFKeyword::NOWIDCTLPAR:
        {
            auto pValue = new RTFValue(int(nKeyword == RTFKeyword::WIDCTLPAR));
            m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_CT_PPrBase_widowControl, pValue);
        }
        break;
        case RTFKeyword::BOX:
        {
            RTFSprms aAttributes;
            auto pValue = new RTFValue(aAttributes);
            for (int i = 0; i < 4; i++)
                m_aStates.top().getParagraphSprms().set(getParagraphBorder(i), pValue);
            m_aStates.top().setBorderState(RTFBorderState::PARAGRAPH_BOX);
        }
        break;
        case RTFKeyword::LTRSECT:
        case RTFKeyword::RTLSECT:
        {
            auto pValue = new RTFValue(nKeyword == RTFKeyword::LTRSECT ? 0 : 1);
            m_aStates.top().setRunType(RTFParserState::RunType::NONE);
            m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_EG_SectPrContents_textDirection,
                                                    pValue);
        }
        break;
        case RTFKeyword::LTRPAR:
        case RTFKeyword::RTLPAR:
        {
            auto pValue = new RTFValue(nKeyword == RTFKeyword::LTRPAR ? 0 : 1);
            m_aStates.top().setRunType(RTFParserState::RunType::NONE);
            m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_CT_PPrBase_bidi, pValue);
        }
        break;
        case RTFKeyword::LTRROW:
        case RTFKeyword::RTLROW:
            m_aStates.top().setRunType(RTFParserState::RunType::NONE);
            m_aStates.top().getTableRowSprms().set(
                NS_ooxml::LN_CT_TblPrBase_bidiVisual,
                new RTFValue(int(nKeyword == RTFKeyword::RTLROW)));
            break;
        case RTFKeyword::LTRCH:
            // dmapper does not support this.
            if (m_aStates.top().getRunType() == RTFParserState::RunType::RTLCH_LTRCH_1)
                m_aStates.top().setRunType(RTFParserState::RunType::RTLCH_LTRCH_2);
            else
                m_aStates.top().setRunType(RTFParserState::RunType::LTRCH_RTLCH_1);
            break;
        case RTFKeyword::RTLCH:
            if (m_aStates.top().getRunType() == RTFParserState::RunType::LTRCH_RTLCH_1)
                m_aStates.top().setRunType(RTFParserState::RunType::LTRCH_RTLCH_2);
            else
                m_aStates.top().setRunType(RTFParserState::RunType::RTLCH_LTRCH_1);

            if (m_aDefaultState.getCurrentEncoding() == RTL_TEXTENCODING_MS_1255)
                m_aStates.top().setCurrentEncoding(m_aDefaultState.getCurrentEncoding());
            break;
        case RTFKeyword::ULNONE:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_Underline_none);
            m_aStates.top().getCharacterAttributes().set(NS_ooxml::LN_CT_Underline_val, pValue);
        }
        break;
        case RTFKeyword::NONSHPPICT:
        case RTFKeyword::MMATHPICT: // Picture group used by readers not understanding \moMath group
            m_aStates.top().setDestination(Destination::SKIP);
            break;
        case RTFKeyword::CLBRDRT:
        case RTFKeyword::CLBRDRL:
        case RTFKeyword::CLBRDRB:
        case RTFKeyword::CLBRDRR:
        {
            RTFSprms aAttributes;
            RTFSprms aSprms;
            auto pValue = new RTFValue(aAttributes, aSprms);
            switch (nKeyword)
            {
                case RTFKeyword::CLBRDRT:
                    nSprm = NS_ooxml::LN_CT_TcBorders_top;
                    break;
                case RTFKeyword::CLBRDRL:
                    nSprm = NS_ooxml::LN_CT_TcBorders_left;
                    break;
                case RTFKeyword::CLBRDRB:
                    nSprm = NS_ooxml::LN_CT_TcBorders_bottom;
                    break;
                case RTFKeyword::CLBRDRR:
                    nSprm = NS_ooxml::LN_CT_TcBorders_right;
                    break;
                default:
                    break;
            }
            putNestedSprm(m_aStates.top().getTableCellSprms(), NS_ooxml::LN_CT_TcPrBase_tcBorders,
                          nSprm, pValue);
            m_aStates.top().setBorderState(RTFBorderState::CELL);
        }
        break;
        case RTFKeyword::PGBRDRT:
        case RTFKeyword::PGBRDRL:
        case RTFKeyword::PGBRDRB:
        case RTFKeyword::PGBRDRR:
        {
            RTFSprms aAttributes;
            RTFSprms aSprms;
            auto pValue = new RTFValue(aAttributes, aSprms);
            switch (nKeyword)
            {
                case RTFKeyword::PGBRDRT:
                    nSprm = NS_ooxml::LN_CT_PageBorders_top;
                    break;
                case RTFKeyword::PGBRDRL:
                    nSprm = NS_ooxml::LN_CT_PageBorders_left;
                    break;
                case RTFKeyword::PGBRDRB:
                    nSprm = NS_ooxml::LN_CT_PageBorders_bottom;
                    break;
                case RTFKeyword::PGBRDRR:
                    nSprm = NS_ooxml::LN_CT_PageBorders_right;
                    break;
                default:
                    break;
            }
            putNestedSprm(m_aStates.top().getSectionSprms(),
                          NS_ooxml::LN_EG_SectPrContents_pgBorders, nSprm, pValue);
            m_aStates.top().setBorderState(RTFBorderState::PAGE);
        }
        break;
        case RTFKeyword::BRDRT:
        case RTFKeyword::BRDRL:
        case RTFKeyword::BRDRB:
        case RTFKeyword::BRDRR:
        {
            RTFSprms aAttributes;
            RTFSprms aSprms;
            auto pValue = new RTFValue(aAttributes, aSprms);
            switch (nKeyword)
            {
                case RTFKeyword::BRDRT:
                    nSprm = getParagraphBorder(0);
                    break;
                case RTFKeyword::BRDRL:
                    nSprm = getParagraphBorder(1);
                    break;
                case RTFKeyword::BRDRB:
                    nSprm = getParagraphBorder(2);
                    break;
                case RTFKeyword::BRDRR:
                    nSprm = getParagraphBorder(3);
                    break;
                default:
                    break;
            }
            putNestedSprm(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PrBase_pBdr, nSprm,
                          pValue);
            m_aStates.top().setBorderState(RTFBorderState::PARAGRAPH);
        }
        break;
        case RTFKeyword::CHBRDR:
        {
            RTFSprms aAttributes;
            auto pValue = new RTFValue(aAttributes);
            m_aStates.top().getCharacterSprms().set(NS_ooxml::LN_EG_RPrBase_bdr, pValue);
            m_aStates.top().setBorderState(RTFBorderState::CHARACTER);
        }
        break;
        case RTFKeyword::CLMGF:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_Merge_restart);
            m_aStates.top().getTableCellSprms().set(NS_ooxml::LN_CT_TcPrBase_hMerge, pValue);
        }
        break;
        case RTFKeyword::CLMRG:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_Merge_continue);
            m_aStates.top().getTableCellSprms().set(NS_ooxml::LN_CT_TcPrBase_hMerge, pValue);
        }
        break;
        case RTFKeyword::CLVMGF:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_Merge_restart);
            m_aStates.top().getTableCellSprms().set(NS_ooxml::LN_CT_TcPrBase_vMerge, pValue);
        }
        break;
        case RTFKeyword::CLVMRG:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_Merge_continue);
            m_aStates.top().getTableCellSprms().set(NS_ooxml::LN_CT_TcPrBase_vMerge, pValue);
        }
        break;
        case RTFKeyword::CLVERTALT:
        case RTFKeyword::CLVERTALC:
        case RTFKeyword::CLVERTALB:
        {
            switch (nKeyword)
            {
                case RTFKeyword::CLVERTALT:
                    nParam = NS_ooxml::LN_Value_ST_VerticalJc_top;
                    break;
                case RTFKeyword::CLVERTALC:
                    nParam = NS_ooxml::LN_Value_ST_VerticalJc_center;
                    break;
                case RTFKeyword::CLVERTALB:
                    nParam = NS_ooxml::LN_Value_ST_VerticalJc_bottom;
                    break;
                default:
                    break;
            }
            auto pValue = new RTFValue(nParam);
            m_aStates.top().getTableCellSprms().set(NS_ooxml::LN_CT_TcPrBase_vAlign, pValue);
        }
        break;
        case RTFKeyword::TRKEEP:
        {
            auto pValue = new RTFValue(1);
            m_aStates.top().getTableRowSprms().set(NS_ooxml::LN_CT_TrPrBase_cantSplit, pValue);
        }
        break;
        case RTFKeyword::SECTUNLOCKED:
        {
            auto pValue = new RTFValue(0);
            m_aStates.top().getSectionSprms().set(NS_ooxml::LN_EG_SectPrContents_formProt, pValue);
        }
        break;
        case RTFKeyword::PGNBIDIA:
        case RTFKeyword::PGNBIDIB:
            // These should be mapped to NS_ooxml::LN_EG_SectPrContents_pgNumType, but dmapper has no API for that at the moment.
            break;
        case RTFKeyword::LOCH:
            m_aStates.top().setRunType(RTFParserState::RunType::LOCH);
            break;
        case RTFKeyword::HICH:
            m_aStates.top().setRunType(RTFParserState::RunType::HICH);
            break;
        case RTFKeyword::DBCH:
            m_aStates.top().setRunType(RTFParserState::RunType::DBCH);
            break;
        case RTFKeyword::TITLEPG:
        {
            auto pValue = new RTFValue(1);
            m_aStates.top().getSectionSprms().set(NS_ooxml::LN_EG_SectPrContents_titlePg, pValue);
        }
        break;
        case RTFKeyword::SUPER:
        {
            // Make sure character properties are not lost if the document
            // starts with a footnote.
            if (!isStyleSheetImport())
            {
                checkFirstRun();
                checkNeedPap();
            }

            if (!m_aStates.top().getCurrentBuffer())
                m_aStates.top().setCurrentBuffer(&m_aSuperBuffer);

            auto pValue = new RTFValue("superscript");
            m_aStates.top().getCharacterSprms().set(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue);
        }
        break;
        case RTFKeyword::SUB:
        {
            auto pValue = new RTFValue("subscript");
            m_aStates.top().getCharacterSprms().set(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue);
        }
        break;
        case RTFKeyword::NOSUPERSUB:
        {
            if (m_aStates.top().getCurrentBuffer() == &m_aSuperBuffer)
            {
                replayBuffer(m_aSuperBuffer, nullptr, nullptr);
                m_aStates.top().setCurrentBuffer(nullptr);
            }
            m_aStates.top().getCharacterSprms().erase(NS_ooxml::LN_EG_RPrBase_vertAlign);
        }
        break;
        case RTFKeyword::LINEPPAGE:
        case RTFKeyword::LINECONT:
        {
            auto pValue = new RTFValue(nKeyword == RTFKeyword::LINEPPAGE
                                           ? NS_ooxml::LN_Value_ST_LineNumberRestart_newPage
                                           : NS_ooxml::LN_Value_ST_LineNumberRestart_continuous);
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_lnNumType,
                               NS_ooxml::LN_CT_LineNumber_restart, pValue);
        }
        break;
        case RTFKeyword::AENDDOC:
            // Noop, this is the default in Writer.
        case RTFKeyword::AENDNOTES:
            // Noop
        case RTFKeyword::AFTNRSTCONT:
            // Noop, this is the default in Writer.
        case RTFKeyword::AFTNRESTART:
            // Noop
        case RTFKeyword::FTNBJ:
            // Noop, this is the default in Writer.
            break;
        case RTFKeyword::ENDDOC:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_RestartNumber_eachSect);
            putNestedSprm(m_aDefaultState.getParagraphSprms(),
                          NS_ooxml::LN_EG_SectPrContents_footnotePr,
                          NS_ooxml::LN_EG_FtnEdnNumProps_numRestart, pValue);
        }
        break;
        case RTFKeyword::NOLINE:
            eraseNestedAttribute(m_aStates.top().getSectionSprms(),
                                 NS_ooxml::LN_EG_SectPrContents_lnNumType,
                                 NS_ooxml::LN_CT_LineNumber_distance);
            break;
        case RTFKeyword::FORMSHADE:
            // Noop, this is the default in Writer.
            break;
        case RTFKeyword::PNGBLIP:
            m_aStates.top().getPicture().eStyle = RTFBmpStyle::PNG;
            break;
        case RTFKeyword::JPEGBLIP:
            m_aStates.top().getPicture().eStyle = RTFBmpStyle::JPEG;
            break;
        case RTFKeyword::POSYT:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                               NS_ooxml::LN_Value_doc_ST_YAlign_top);
            break;
        case RTFKeyword::POSYB:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                               NS_ooxml::LN_Value_doc_ST_YAlign_bottom);
            break;
        case RTFKeyword::POSYC:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                               NS_ooxml::LN_Value_doc_ST_YAlign_center);
            break;
        case RTFKeyword::POSYIN:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                               NS_ooxml::LN_Value_doc_ST_YAlign_inside);
            break;
        case RTFKeyword::POSYOUT:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                               NS_ooxml::LN_Value_doc_ST_YAlign_outside);
            break;
        case RTFKeyword::POSYIL:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                               NS_ooxml::LN_Value_doc_ST_YAlign_inline);
            break;

        case RTFKeyword::PHMRG:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_hAnchor,
                                               NS_ooxml::LN_Value_doc_ST_HAnchor_margin);
            break;
        case RTFKeyword::PVMRG:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_vAnchor,
                                               NS_ooxml::LN_Value_doc_ST_VAnchor_margin);
            break;
        case RTFKeyword::PHPG:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_hAnchor,
                                               NS_ooxml::LN_Value_doc_ST_HAnchor_page);
            break;
        case RTFKeyword::PVPG:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_vAnchor,
                                               NS_ooxml::LN_Value_doc_ST_VAnchor_page);
            break;
        case RTFKeyword::PHCOL:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_hAnchor,
                                               NS_ooxml::LN_Value_doc_ST_HAnchor_text);
            break;
        case RTFKeyword::PVPARA:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_vAnchor,
                                               NS_ooxml::LN_Value_doc_ST_VAnchor_text);
            break;

        case RTFKeyword::POSXC:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_xAlign,
                                               NS_ooxml::LN_Value_doc_ST_XAlign_center);
            break;
        case RTFKeyword::POSXI:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_xAlign,
                                               NS_ooxml::LN_Value_doc_ST_XAlign_inside);
            break;
        case RTFKeyword::POSXO:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_xAlign,
                                               NS_ooxml::LN_Value_doc_ST_XAlign_outside);
            break;
        case RTFKeyword::POSXL:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_xAlign,
                                               NS_ooxml::LN_Value_doc_ST_XAlign_left);
            break;
        case RTFKeyword::POSXR:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_xAlign,
                                               NS_ooxml::LN_Value_doc_ST_XAlign_right);
            break;

        case RTFKeyword::DPLINE:
        case RTFKeyword::DPRECT:
        case RTFKeyword::DPELLIPSE:
        case RTFKeyword::DPTXBX:
        case RTFKeyword::DPPOLYLINE:
        case RTFKeyword::DPPOLYGON:
        {
            sal_Int32 nType = 0;
            switch (nKeyword)
            {
                case RTFKeyword::DPLINE:
                {
                    uno::Reference<drawing::XShape> xShape(
                        getModelFactory()->createInstance("com.sun.star.drawing.LineShape"),
                        uno::UNO_QUERY);
                    m_aStates.top().getDrawingObject().setShape(xShape);
                    break;
                }
                case RTFKeyword::DPPOLYLINE:
                {
                    // The reason this is not a simple CustomShape is that in the old syntax we have no ViewBox info.
                    uno::Reference<drawing::XShape> xShape(
                        getModelFactory()->createInstance("com.sun.star.drawing.PolyLineShape"),
                        uno::UNO_QUERY);
                    m_aStates.top().getDrawingObject().setShape(xShape);
                    break;
                }
                case RTFKeyword::DPPOLYGON:
                {
                    uno::Reference<drawing::XShape> xShape(
                        getModelFactory()->createInstance("com.sun.star.drawing.PolyPolygonShape"),
                        uno::UNO_QUERY);
                    m_aStates.top().getDrawingObject().setShape(xShape);
                    break;
                }
                case RTFKeyword::DPRECT:
                {
                    uno::Reference<drawing::XShape> xShape(
                        getModelFactory()->createInstance("com.sun.star.drawing.RectangleShape"),
                        uno::UNO_QUERY);
                    m_aStates.top().getDrawingObject().setShape(xShape);
                    break;
                }
                case RTFKeyword::DPELLIPSE:
                    nType = ESCHER_ShpInst_Ellipse;
                    break;
                case RTFKeyword::DPTXBX:
                {
                    uno::Reference<drawing::XShape> xShape(
                        getModelFactory()->createInstance("com.sun.star.text.TextFrame"),
                        uno::UNO_QUERY);
                    m_aStates.top().getDrawingObject().setShape(xShape);
                    std::vector<beans::PropertyValue> aDefaults
                        = RTFSdrImport::getTextFrameDefaults(false);
                    for (const auto& rDefault : aDefaults)
                    {
                        if (!findPropertyName(
                                m_aStates.top().getDrawingObject().getPendingProperties(),
                                rDefault.Name))
                            m_aStates.top().getDrawingObject().getPendingProperties().push_back(
                                rDefault);
                    }
                    checkFirstRun();
                    Mapper().startShape(m_aStates.top().getDrawingObject().getShape());
                    m_aStates.top().getDrawingObject().setHadShapeText(true);
                }
                break;
                default:
                    break;
            }
            if (nType)
            {
                uno::Reference<drawing::XShape> xShape(
                    getModelFactory()->createInstance("com.sun.star.drawing.CustomShape"),
                    uno::UNO_QUERY);
                m_aStates.top().getDrawingObject().setShape(xShape);
            }
            uno::Reference<drawing::XDrawPageSupplier> xDrawSupplier(m_xDstDoc, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xPropertySet(
                m_aStates.top().getDrawingObject().getShape(), uno::UNO_QUERY);
            m_aStates.top().getDrawingObject().setPropertySet(xPropertySet);
            if (xDrawSupplier.is())
            {
                uno::Reference<drawing::XShapes> xShapes = xDrawSupplier->getDrawPage();
                if (xShapes.is() && nKeyword != RTFKeyword::DPTXBX)
                {
                    // set default VertOrient before inserting
                    m_aStates.top().getDrawingObject().getPropertySet()->setPropertyValue(
                        "VertOrient", uno::makeAny(text::VertOrientation::NONE));
                    xShapes->add(m_aStates.top().getDrawingObject().getShape());
                }
            }
            if (nType)
            {
                uno::Reference<drawing::XEnhancedCustomShapeDefaulter> xDefaulter(
                    m_aStates.top().getDrawingObject().getShape(), uno::UNO_QUERY);
                xDefaulter->createCustomShapeDefaults(OUString::number(nType));
            }
            std::vector<beans::PropertyValue>& rPendingProperties
                = m_aStates.top().getDrawingObject().getPendingProperties();
            for (const auto& rPendingProperty : rPendingProperties)
                m_aStates.top().getDrawingObject().getPropertySet()->setPropertyValue(
                    rPendingProperty.Name, rPendingProperty.Value);
            m_pSdrImport->resolveDhgt(m_aStates.top().getDrawingObject().getPropertySet(),
                                      m_aStates.top().getDrawingObject().getDhgt(),
                                      /*bOldStyle=*/true);
        }
        break;
        case RTFKeyword::DOBXMARGIN:
        case RTFKeyword::DOBYMARGIN:
        {
            beans::PropertyValue aPropertyValue;
            aPropertyValue.Name
                = (nKeyword == RTFKeyword::DOBXMARGIN ? std::u16string_view(u"HoriOrientRelation")
                                                      : std::u16string_view(u"VertOrientRelation"));
            aPropertyValue.Value <<= text::RelOrientation::PAGE_PRINT_AREA;
            m_aStates.top().getDrawingObject().getPendingProperties().push_back(aPropertyValue);
        }
        break;
        case RTFKeyword::DOBXPAGE:
        case RTFKeyword::DOBYPAGE:
        {
            beans::PropertyValue aPropertyValue;
            aPropertyValue.Name
                = (nKeyword == RTFKeyword::DOBXPAGE ? std::u16string_view(u"HoriOrientRelation")
                                                    : std::u16string_view(u"VertOrientRelation"));
            aPropertyValue.Value <<= text::RelOrientation::PAGE_FRAME;
            m_aStates.top().getDrawingObject().getPendingProperties().push_back(aPropertyValue);
        }
        break;
        case RTFKeyword::DOBYPARA:
        {
            beans::PropertyValue aPropertyValue;
            aPropertyValue.Name = "VertOrientRelation";
            aPropertyValue.Value <<= text::RelOrientation::FRAME;
            m_aStates.top().getDrawingObject().getPendingProperties().push_back(aPropertyValue);
        }
        break;
        case RTFKeyword::CONTEXTUALSPACE:
        {
            auto pValue = new RTFValue(1);
            m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_CT_PPrBase_contextualSpacing,
                                                    pValue);
        }
        break;
        case RTFKeyword::LINKSTYLES:
        {
            auto pValue = new RTFValue(1);
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_linkStyles, pValue);
        }
        break;
        case RTFKeyword::PNLVLBODY:
        {
            auto pValue = new RTFValue(2);
            m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_AbstractNum_nsid, pValue);
        }
        break;
        case RTFKeyword::PNDEC:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_decimal);
            putNestedAttribute(m_aStates.top().getTableSprms(), NS_ooxml::LN_CT_Lvl_numFmt,
                               NS_ooxml::LN_CT_NumFmt_val, pValue);
        }
        break;
        case RTFKeyword::PNLVLBLT:
        {
            m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_AbstractNum_nsid,
                                                     new RTFValue(1));
            putNestedAttribute(m_aStates.top().getTableSprms(), NS_ooxml::LN_CT_Lvl_numFmt,
                               NS_ooxml::LN_CT_NumFmt_val,
                               new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_bullet));
        }
        break;
        case RTFKeyword::LANDSCAPE:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_PageOrientation_landscape);
            putNestedAttribute(m_aDefaultState.getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_orient,
                               pValue);
            [[fallthrough]]; // set the default + current value
        }
        case RTFKeyword::LNDSCPSXN:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_PageOrientation_landscape);
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_orient,
                               pValue);
        }
        break;
        case RTFKeyword::SHPBXPAGE:
            m_aStates.top().getShape().setHoriOrientRelation(text::RelOrientation::PAGE_FRAME);
            m_aStates.top().getShape().setHoriOrientRelationToken(
                NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_page);
            break;
        case RTFKeyword::SHPBYPAGE:
            m_aStates.top().getShape().setVertOrientRelation(text::RelOrientation::PAGE_FRAME);
            m_aStates.top().getShape().setVertOrientRelationToken(
                NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_page);
            break;
        case RTFKeyword::DPLINEHOLLOW:
            m_aStates.top().getDrawingObject().setFLine(0);
            break;
        case RTFKeyword::DPROUNDR:
            if (m_aStates.top().getDrawingObject().getPropertySet().is())
                // Seems this old syntax has no way to specify a custom radius, and this is the default
                m_aStates.top().getDrawingObject().getPropertySet()->setPropertyValue(
                    "CornerRadius", uno::makeAny(sal_Int32(83)));
            break;
        case RTFKeyword::NOWRAP:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_wrap,
                                               NS_ooxml::LN_Value_doc_ST_Wrap_notBeside);
            break;
        case RTFKeyword::MNOR:
            m_bMathNor = true;
            break;
        case RTFKeyword::REVISIONS:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_trackRevisions, new RTFValue(1));
            break;
        case RTFKeyword::BRDRSH:
            putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_shadow, new RTFValue(1));
            break;
        case RTFKeyword::NOCOLBAL:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Compat_noColumnBalance, new RTFValue(1));
            break;
        case RTFKeyword::MARGMIRROR:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_mirrorMargins, new RTFValue(1));
            break;
        case RTFKeyword::SAUTOUPD:
            m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_Style_autoRedefine,
                                                new RTFValue(1));
            break;
        case RTFKeyword::WIDOWCTRL:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_widowControl, new RTFValue(1));
            break;
        case RTFKeyword::LINEBETCOL:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_cols, NS_ooxml::LN_CT_Columns_sep,
                               new RTFValue(1));
            break;
        case RTFKeyword::PGNRESTART:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgNumType,
                               NS_ooxml::LN_CT_PageNumber_start, new RTFValue(1));
            break;
        case RTFKeyword::PGNUCLTR:
        {
            auto pIntValue = new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_upperLetter);
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgNumType,
                               NS_ooxml::LN_CT_PageNumber_fmt, pIntValue);
        }
        break;
        case RTFKeyword::PGNLCLTR:
        {
            auto pIntValue = new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter);
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgNumType,
                               NS_ooxml::LN_CT_PageNumber_fmt, pIntValue);
        }
        break;
        case RTFKeyword::PGNUCRM:
        {
            auto pIntValue = new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_upperRoman);
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgNumType,
                               NS_ooxml::LN_CT_PageNumber_fmt, pIntValue);
        }
        break;
        case RTFKeyword::PGNLCRM:
        {
            auto pIntValue = new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman);
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgNumType,
                               NS_ooxml::LN_CT_PageNumber_fmt, pIntValue);
        }
        break;
        case RTFKeyword::PGNDEC:
        {
            auto pIntValue = new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_decimal);
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgNumType,
                               NS_ooxml::LN_CT_PageNumber_fmt, pIntValue);
        }
        break;
        case RTFKeyword::HTMAUTSP:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Compat_doNotUseHTMLParagraphAutoSpacing,
                                      new RTFValue(0));
            break;
        case RTFKeyword::DNTBLNSBDB:
            // tdf#128428 switch off longer space sequence
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_longerSpaceSequence,
                                      new RTFValue(0));
            break;
        case RTFKeyword::GUTTERPRL:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_gutterAtTop, new RTFValue(1));
            break;
        case RTFKeyword::RTLGUTTER:
        {
            m_aStates.top().getSectionSprms().set(NS_ooxml::LN_EG_SectPrContents_rtlGutter,
                                                  new RTFValue(1));
        }
        break;
        case RTFKeyword::FLDLOCK:
        {
            if (m_aStates.top().getDestination() == Destination::FIELD)
                m_aStates.top().setFieldLocked(true);
        }
        break;
        default:
        {
            SAL_INFO("writerfilter", "TODO handle flag '" << keywordToString(nKeyword) << "'");
            aSkip.setParsed(false);
        }
        break;
    }
    return RTFError::OK;
}

} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
