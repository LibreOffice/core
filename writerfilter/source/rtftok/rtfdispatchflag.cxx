/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

namespace writerfilter
{
namespace rtftok
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
        case RTF_ULD:
            nSprm = NS_ooxml::LN_Value_ST_Underline_dotted;
            break;
        case RTF_ULW:
            nSprm = NS_ooxml::LN_Value_ST_Underline_words;
            break;
        default:
            break;
    }
    if (nSprm >= 0)
    {
        auto pValue = new RTFValue(nSprm);
        m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_Underline_val, pValue);
        return RTFError::OK;
    }

    // Indentation
    switch (nKeyword)
    {
        case RTF_QC:
            nParam = NS_ooxml::LN_Value_ST_Jc_center;
            break;
        case RTF_QJ:
            nParam = NS_ooxml::LN_Value_ST_Jc_both;
            break;
        case RTF_QL:
            nParam = NS_ooxml::LN_Value_ST_Jc_left;
            break;
        case RTF_QR:
            nParam = NS_ooxml::LN_Value_ST_Jc_right;
            break;
        case RTF_QD:
            nParam = NS_ooxml::LN_Value_ST_Jc_distribute;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_jc, pValue);
        m_bNeedPap = true;
        return RTFError::OK;
    }

    // Font Alignment
    switch (nKeyword)
    {
        case RTF_FAFIXED:
        case RTF_FAAUTO:
            nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_auto;
            break;
        case RTF_FAHANG:
            nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_top;
            break;
        case RTF_FACENTER:
            nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_center;
            break;
        case RTF_FAROMAN:
            nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_baseline;
            break;
        case RTF_FAVAR:
            nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_bottom;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_textAlignment, pValue);
        return RTFError::OK;
    }

    // Tab kind.
    switch (nKeyword)
    {
        case RTF_TQR:
            nParam = NS_ooxml::LN_Value_ST_TabJc_right;
            break;
        case RTF_TQC:
            nParam = NS_ooxml::LN_Value_ST_TabJc_center;
            break;
        case RTF_TQDEC:
            nParam = NS_ooxml::LN_Value_ST_TabJc_decimal;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        m_aStates.top().aTabAttributes.set(NS_ooxml::LN_CT_TabStop_val, pValue);
        return RTFError::OK;
    }

    // Tab lead.
    switch (nKeyword)
    {
        case RTF_TLDOT:
            nParam = NS_ooxml::LN_Value_ST_TabTlc_dot;
            break;
        case RTF_TLMDOT:
            nParam = NS_ooxml::LN_Value_ST_TabTlc_middleDot;
            break;
        case RTF_TLHYPH:
            nParam = NS_ooxml::LN_Value_ST_TabTlc_hyphen;
            break;
        case RTF_TLUL:
            nParam = NS_ooxml::LN_Value_ST_TabTlc_underscore;
            break;
        case RTF_TLTH:
            nParam = NS_ooxml::LN_Value_ST_TabTlc_hyphen;
            break; // thick line is not supported by dmapper, this is just a hack
        case RTF_TLEQ:
            nParam = NS_ooxml::LN_Value_ST_TabTlc_none;
            break; // equal sign isn't, either
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        m_aStates.top().aTabAttributes.set(NS_ooxml::LN_CT_TabStop_leader, pValue);
        return RTFError::OK;
    }

    // Border types
    {
        switch (nKeyword)
        {
            // brdrhair and brdrs are the same, brdrw will make a difference
            // map to values in ooxml/model.xml resource ST_Border
            case RTF_BRDRHAIR:
            case RTF_BRDRS:
                nParam = NS_ooxml::LN_Value_ST_Border_single;
                break;
            case RTF_BRDRDOT:
                nParam = NS_ooxml::LN_Value_ST_Border_dotted;
                break;
            case RTF_BRDRDASH:
                nParam = NS_ooxml::LN_Value_ST_Border_dashed;
                break;
            case RTF_BRDRDB:
                nParam = NS_ooxml::LN_Value_ST_Border_double;
                break;
            case RTF_BRDRTNTHSG:
                nParam = NS_ooxml::LN_Value_ST_Border_thinThickSmallGap;
                break;
            case RTF_BRDRTNTHMG:
                nParam = NS_ooxml::LN_Value_ST_Border_thinThickMediumGap;
                break;
            case RTF_BRDRTNTHLG:
                nParam = NS_ooxml::LN_Value_ST_Border_thinThickLargeGap;
                break;
            case RTF_BRDRTHTNSG:
                nParam = NS_ooxml::LN_Value_ST_Border_thickThinSmallGap;
                break;
            case RTF_BRDRTHTNMG:
                nParam = NS_ooxml::LN_Value_ST_Border_thickThinMediumGap;
                break;
            case RTF_BRDRTHTNLG:
                nParam = NS_ooxml::LN_Value_ST_Border_thickThinLargeGap;
                break;
            case RTF_BRDREMBOSS:
                nParam = NS_ooxml::LN_Value_ST_Border_threeDEmboss;
                break;
            case RTF_BRDRENGRAVE:
                nParam = NS_ooxml::LN_Value_ST_Border_threeDEngrave;
                break;
            case RTF_BRDROUTSET:
                nParam = NS_ooxml::LN_Value_ST_Border_outset;
                break;
            case RTF_BRDRINSET:
                nParam = NS_ooxml::LN_Value_ST_Border_inset;
                break;
            case RTF_BRDRDASHSM:
                nParam = NS_ooxml::LN_Value_ST_Border_dashSmallGap;
                break;
            case RTF_BRDRDASHD:
                nParam = NS_ooxml::LN_Value_ST_Border_dotDash;
                break;
            case RTF_BRDRDASHDD:
                nParam = NS_ooxml::LN_Value_ST_Border_dotDotDash;
                break;
            case RTF_BRDRNONE:
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
        case RTF_SBKNONE:
            nParam = NS_ooxml::LN_Value_ST_SectionMark_continuous;
            break;
        case RTF_SBKCOL:
            nParam = NS_ooxml::LN_Value_ST_SectionMark_nextColumn;
            break;
        case RTF_SBKPAGE:
            nParam = NS_ooxml::LN_Value_ST_SectionMark_nextPage;
            break;
        case RTF_SBKEVEN:
            nParam = NS_ooxml::LN_Value_ST_SectionMark_evenPage;
            break;
        case RTF_SBKODD:
            nParam = NS_ooxml::LN_Value_ST_SectionMark_oddPage;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        if (m_nResetBreakOnSectBreak != RTF_invalid)
        {
            m_nResetBreakOnSectBreak = nKeyword;
        }
        auto pValue = new RTFValue(nParam);
        m_aStates.top().aSectionSprms.set(NS_ooxml::LN_EG_SectPrContents_type, pValue);
        return RTFError::OK;
    }

    // Footnote numbering
    switch (nKeyword)
    {
        case RTF_FTNNAR:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_decimal;
            break;
        case RTF_FTNNALC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter;
            break;
        case RTF_FTNNAUC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperLetter;
            break;
        case RTF_FTNNRLC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman;
            break;
        case RTF_FTNNRUC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperRoman;
            break;
        case RTF_FTNNCHI:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_chicago;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        putNestedSprm(m_aDefaultState.aParagraphSprms, NS_ooxml::LN_EG_SectPrContents_footnotePr,
                      NS_ooxml::LN_CT_FtnProps_numFmt, pValue);
        return RTFError::OK;
    }

    // Footnote restart type
    switch (nKeyword)
    {
        case RTF_FTNRSTPG:
            nParam = NS_ooxml::LN_Value_ST_RestartNumber_eachPage;
            break;
        case RTF_FTNRESTART:
            nParam = NS_ooxml::LN_Value_ST_RestartNumber_eachSect;
            break;
        case RTF_FTNRSTCONT:
            nParam = NS_ooxml::LN_Value_ST_RestartNumber_continuous;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        putNestedSprm(m_aDefaultState.aParagraphSprms, NS_ooxml::LN_EG_SectPrContents_footnotePr,
                      NS_ooxml::LN_EG_FtnEdnNumProps_numRestart, pValue);
        return RTFError::OK;
    }

    // Endnote numbering
    switch (nKeyword)
    {
        case RTF_AFTNNAR:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_decimal;
            break;
        case RTF_AFTNNALC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter;
            break;
        case RTF_AFTNNAUC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperLetter;
            break;
        case RTF_AFTNNRLC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman;
            break;
        case RTF_AFTNNRUC:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperRoman;
            break;
        case RTF_AFTNNCHI:
            nParam = NS_ooxml::LN_Value_ST_NumberFormat_chicago;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        putNestedSprm(m_aDefaultState.aParagraphSprms, NS_ooxml::LN_EG_SectPrContents_endnotePr,
                      NS_ooxml::LN_CT_EdnProps_numFmt, pValue);
        return RTFError::OK;
    }

    switch (nKeyword)
    {
        case RTF_TRQL:
            nParam = NS_ooxml::LN_Value_ST_Jc_left;
            break;
        case RTF_TRQC:
            nParam = NS_ooxml::LN_Value_ST_Jc_center;
            break;
        case RTF_TRQR:
            nParam = NS_ooxml::LN_Value_ST_Jc_right;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        m_aStates.top().aTableRowSprms.set(NS_ooxml::LN_CT_TrPrBase_jc, pValue);
        return RTFError::OK;
    }

    // Cell Text Flow
    switch (nKeyword)
    {
        case RTF_CLTXLRTB:
            nParam = NS_ooxml::LN_Value_ST_TextDirection_lrTb;
            break;
        case RTF_CLTXTBRL:
            nParam = NS_ooxml::LN_Value_ST_TextDirection_tbRl;
            break;
        case RTF_CLTXBTLR:
            nParam = NS_ooxml::LN_Value_ST_TextDirection_btLr;
            break;
        case RTF_CLTXLRTBV:
            nParam = NS_ooxml::LN_Value_ST_TextDirection_lrTbV;
            break;
        case RTF_CLTXTBRLV:
            nParam = NS_ooxml::LN_Value_ST_TextDirection_tbRlV;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(nParam);
        m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_textDirection, pValue);
    }

    // Trivial paragraph flags
    switch (nKeyword)
    {
        case RTF_KEEP:
            if (m_aStates.top().pCurrentBuffer != &m_aTableBufferStack.back())
                nParam = NS_ooxml::LN_CT_PPrBase_keepLines;
            break;
        case RTF_KEEPN:
            if (m_aStates.top().pCurrentBuffer != &m_aTableBufferStack.back())
                nParam = NS_ooxml::LN_CT_PPrBase_keepNext;
            break;
        case RTF_INTBL:
        {
            m_aStates.top().pCurrentBuffer = &m_aTableBufferStack.back();
            nParam = NS_ooxml::LN_inTbl;
        }
        break;
        case RTF_PAGEBB:
            nParam = NS_ooxml::LN_CT_PPrBase_pageBreakBefore;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        auto pValue = new RTFValue(1);
        m_aStates.top().aParagraphSprms.erase(NS_ooxml::LN_inTbl);
        m_aStates.top().aParagraphSprms.set(nParam, pValue);
        return RTFError::OK;
    }

    switch (nKeyword)
    {
        case RTF_FNIL:
        case RTF_FROMAN:
        case RTF_FSWISS:
        case RTF_FMODERN:
        case RTF_FSCRIPT:
        case RTF_FDECOR:
        case RTF_FTECH:
        case RTF_FBIDI:
            // TODO ooxml:CT_Font_family seems to be ignored by the domain mapper
            break;
        case RTF_ANSI:
            m_aStates.top().nCurrentEncoding = RTL_TEXTENCODING_MS_1252;
            break;
        case RTF_MAC:
            m_aDefaultState.nCurrentEncoding = RTL_TEXTENCODING_APPLE_ROMAN;
            m_aStates.top().nCurrentEncoding = m_aDefaultState.nCurrentEncoding;
            break;
        case RTF_PC:
            m_aDefaultState.nCurrentEncoding = RTL_TEXTENCODING_IBM_437;
            m_aStates.top().nCurrentEncoding = m_aDefaultState.nCurrentEncoding;
            break;
        case RTF_PCA:
            m_aDefaultState.nCurrentEncoding = RTL_TEXTENCODING_IBM_850;
            m_aStates.top().nCurrentEncoding = m_aDefaultState.nCurrentEncoding;
            break;
        case RTF_PLAIN:
        {
            m_aStates.top().aCharacterSprms = getDefaultState().aCharacterSprms;
            m_aStates.top().nCurrentEncoding = getEncoding(getFontIndex(m_nDefaultFontIndex));
            m_aStates.top().aCharacterAttributes = getDefaultState().aCharacterAttributes;
            m_aStates.top().nCurrentCharacterStyleIndex = -1;
            m_aStates.top().isRightToLeft = false;
            m_aStates.top().eRunType = RTFParserState::RunType::LOCH;
        }
        break;
        case RTF_PARD:
        {
            if (m_bHadPicture)
                dispatchSymbol(RTF_PAR);
            // \pard is allowed between \cell and \row, but in that case it should not reset the fact that we're inside a table.
            // It should not reset the paragraph style, either, so remember the old paragraph style.
            RTFValue::Pointer_t pOldStyle
                = m_aStates.top().aParagraphSprms.find(NS_ooxml::LN_CT_PPrBase_pStyle);
            m_aStates.top().aParagraphSprms = m_aDefaultState.aParagraphSprms;
            m_aStates.top().aParagraphAttributes = m_aDefaultState.aParagraphAttributes;

            if (m_nTopLevelCells == 0 && m_nNestedCells == 0)
            {
                // Reset that we're in a table.
                m_aStates.top().pCurrentBuffer = nullptr;
            }
            else
            {
                // We are still in a table.
                m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_inTbl, new RTFValue(1));
                if (m_bAfterCellBeforeRow && pOldStyle)
                    // And we still have the same paragraph style.
                    m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_pStyle, pOldStyle);
                // Ideally getDefaultSPRM() would take care of this, but it would not when we're buffering.
                m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_tabs, new RTFValue());
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
                    m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_pStyle,
                                                        new RTFValue(aName));
                    m_aStates.top().nCurrentStyleIndex = 0;
                }
                else
                {
                    m_aStates.top().nCurrentStyleIndex = -1;
                }
            }
            // Need to send paragraph properties again, if there will be any.
            m_bNeedPap = true;
            break;
        }
        case RTF_SECTD:
        {
            m_aStates.top().aSectionSprms = m_aDefaultState.aSectionSprms;
            m_aStates.top().aSectionAttributes = m_aDefaultState.aSectionAttributes;
        }
        break;
        case RTF_TROWD:
        {
            // Back these up, in case later we still need this info.
            backupTableRowProperties();
            resetTableRowProperties();
            // In case the table definition is in the middle of the row
            // (invalid), make sure table definition is emitted.
            m_bNeedPap = true;
        }
        break;
        case RTF_WIDCTLPAR:
        case RTF_NOWIDCTLPAR:
        {
            auto pValue = new RTFValue(int(nKeyword == RTF_WIDCTLPAR));
            m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_widowControl, pValue);
        }
        break;
        case RTF_BOX:
        {
            RTFSprms aAttributes;
            auto pValue = new RTFValue(aAttributes);
            for (int i = 0; i < 4; i++)
                m_aStates.top().aParagraphSprms.set(getParagraphBorder(i), pValue);
            m_aStates.top().nBorderState = RTFBorderState::PARAGRAPH_BOX;
        }
        break;
        case RTF_LTRSECT:
        case RTF_RTLSECT:
        {
            auto pValue = new RTFValue(nKeyword == RTF_LTRSECT ? 0 : 1);
            m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_EG_SectPrContents_textDirection,
                                                pValue);
        }
        break;
        case RTF_LTRPAR:
        case RTF_RTLPAR:
        {
            auto pValue = new RTFValue(nKeyword == RTF_LTRPAR ? 0 : 1);
            m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_bidi, pValue);
        }
        break;
        case RTF_LTRROW:
        case RTF_RTLROW:
            m_aStates.top().aTableRowSprms.set(NS_ooxml::LN_CT_TblPrBase_bidiVisual,
                                               new RTFValue(int(nKeyword == RTF_RTLROW)));
            break;
        case RTF_LTRCH:
            // dmapper does not support this.
            m_aStates.top().isRightToLeft = false;
            break;
        case RTF_RTLCH:
            m_aStates.top().isRightToLeft = true;
            if (m_aDefaultState.nCurrentEncoding == RTL_TEXTENCODING_MS_1255)
                m_aStates.top().nCurrentEncoding = m_aDefaultState.nCurrentEncoding;
            break;
        case RTF_ULNONE:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_Underline_none);
            m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_Underline_val, pValue);
        }
        break;
        case RTF_NONSHPPICT:
        case RTF_MMATHPICT: // Picture group used by readers not understanding \moMath group
            m_aStates.top().eDestination = Destination::SKIP;
            break;
        case RTF_CLBRDRT:
        case RTF_CLBRDRL:
        case RTF_CLBRDRB:
        case RTF_CLBRDRR:
        {
            RTFSprms aAttributes;
            RTFSprms aSprms;
            auto pValue = new RTFValue(aAttributes, aSprms);
            switch (nKeyword)
            {
                case RTF_CLBRDRT:
                    nParam = NS_ooxml::LN_CT_TcBorders_top;
                    break;
                case RTF_CLBRDRL:
                    nParam = NS_ooxml::LN_CT_TcBorders_left;
                    break;
                case RTF_CLBRDRB:
                    nParam = NS_ooxml::LN_CT_TcBorders_bottom;
                    break;
                case RTF_CLBRDRR:
                    nParam = NS_ooxml::LN_CT_TcBorders_right;
                    break;
                default:
                    break;
            }
            putNestedSprm(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcBorders,
                          nParam, pValue);
            m_aStates.top().nBorderState = RTFBorderState::CELL;
        }
        break;
        case RTF_PGBRDRT:
        case RTF_PGBRDRL:
        case RTF_PGBRDRB:
        case RTF_PGBRDRR:
        {
            RTFSprms aAttributes;
            RTFSprms aSprms;
            auto pValue = new RTFValue(aAttributes, aSprms);
            switch (nKeyword)
            {
                case RTF_PGBRDRT:
                    nParam = NS_ooxml::LN_CT_PageBorders_top;
                    break;
                case RTF_PGBRDRL:
                    nParam = NS_ooxml::LN_CT_PageBorders_left;
                    break;
                case RTF_PGBRDRB:
                    nParam = NS_ooxml::LN_CT_PageBorders_bottom;
                    break;
                case RTF_PGBRDRR:
                    nParam = NS_ooxml::LN_CT_PageBorders_right;
                    break;
                default:
                    break;
            }
            putNestedSprm(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgBorders,
                          nParam, pValue);
            m_aStates.top().nBorderState = RTFBorderState::PAGE;
        }
        break;
        case RTF_BRDRT:
        case RTF_BRDRL:
        case RTF_BRDRB:
        case RTF_BRDRR:
        {
            RTFSprms aAttributes;
            RTFSprms aSprms;
            auto pValue = new RTFValue(aAttributes, aSprms);
            switch (nKeyword)
            {
                case RTF_BRDRT:
                    nParam = getParagraphBorder(0);
                    break;
                case RTF_BRDRL:
                    nParam = getParagraphBorder(1);
                    break;
                case RTF_BRDRB:
                    nParam = getParagraphBorder(2);
                    break;
                case RTF_BRDRR:
                    nParam = getParagraphBorder(3);
                    break;
                default:
                    break;
            }
            putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PrBase_pBdr, nParam,
                          pValue);
            m_aStates.top().nBorderState = RTFBorderState::PARAGRAPH;
        }
        break;
        case RTF_CHBRDR:
        {
            RTFSprms aAttributes;
            auto pValue = new RTFValue(aAttributes);
            m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_bdr, pValue);
            m_aStates.top().nBorderState = RTFBorderState::CHARACTER;
        }
        break;
        case RTF_CLMGF:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_Merge_restart);
            m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_hMerge, pValue);
        }
        break;
        case RTF_CLMRG:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_Merge_continue);
            m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_hMerge, pValue);
        }
        break;
        case RTF_CLVMGF:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_Merge_restart);
            m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_vMerge, pValue);
        }
        break;
        case RTF_CLVMRG:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_Merge_continue);
            m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_vMerge, pValue);
        }
        break;
        case RTF_CLVERTALT:
        case RTF_CLVERTALC:
        case RTF_CLVERTALB:
        {
            switch (nKeyword)
            {
                case RTF_CLVERTALT:
                    nParam = NS_ooxml::LN_Value_ST_VerticalJc_top;
                    break;
                case RTF_CLVERTALC:
                    nParam = NS_ooxml::LN_Value_ST_VerticalJc_center;
                    break;
                case RTF_CLVERTALB:
                    nParam = NS_ooxml::LN_Value_ST_VerticalJc_bottom;
                    break;
                default:
                    break;
            }
            auto pValue = new RTFValue(nParam);
            m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_vAlign, pValue);
        }
        break;
        case RTF_TRKEEP:
        {
            auto pValue = new RTFValue(1);
            m_aStates.top().aTableRowSprms.set(NS_ooxml::LN_CT_TrPrBase_cantSplit, pValue);
        }
        break;
        case RTF_SECTUNLOCKED:
        {
            auto pValue = new RTFValue(0);
            m_aStates.top().aSectionSprms.set(NS_ooxml::LN_EG_SectPrContents_formProt, pValue);
        }
        break;
        case RTF_PGNBIDIA:
        case RTF_PGNBIDIB:
            // These should be mapped to NS_ooxml::LN_EG_SectPrContents_pgNumType, but dmapper has no API for that at the moment.
            break;
        case RTF_LOCH:
            m_aStates.top().eRunType = RTFParserState::RunType::LOCH;
            break;
        case RTF_HICH:
            m_aStates.top().eRunType = RTFParserState::RunType::HICH;
            break;
        case RTF_DBCH:
            m_aStates.top().eRunType = RTFParserState::RunType::DBCH;
            break;
        case RTF_TITLEPG:
        {
            auto pValue = new RTFValue(1);
            m_aStates.top().aSectionSprms.set(NS_ooxml::LN_EG_SectPrContents_titlePg, pValue);
        }
        break;
        case RTF_SUPER:
        {
            // Make sure character properties are not lost if the document
            // starts with a footnote.
            if (!isStyleSheetImport())
            {
                checkFirstRun();
                checkNeedPap();
            }

            if (!m_aStates.top().pCurrentBuffer)
                m_aStates.top().pCurrentBuffer = &m_aSuperBuffer;

            auto pValue = new RTFValue("superscript");
            m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue);
        }
        break;
        case RTF_SUB:
        {
            auto pValue = new RTFValue("subscript");
            m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue);
        }
        break;
        case RTF_NOSUPERSUB:
        {
            if (m_aStates.top().pCurrentBuffer == &m_aSuperBuffer)
            {
                replayBuffer(m_aSuperBuffer, nullptr, nullptr);
                m_aStates.top().pCurrentBuffer = nullptr;
            }
            m_aStates.top().aCharacterSprms.erase(NS_ooxml::LN_EG_RPrBase_vertAlign);
        }
        break;
        case RTF_LINEPPAGE:
        case RTF_LINECONT:
        {
            auto pValue = new RTFValue(nKeyword == RTF_LINEPPAGE
                                           ? NS_ooxml::LN_Value_ST_LineNumberRestart_newPage
                                           : NS_ooxml::LN_Value_ST_LineNumberRestart_continuous);
            putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_lnNumType,
                               NS_ooxml::LN_CT_LineNumber_restart, pValue);
        }
        break;
        case RTF_AENDDOC:
            // Noop, this is the default in Writer.
            break;
        case RTF_AENDNOTES:
            // Noop, Writer does not support having endnotes at the end of section.
            break;
        case RTF_AFTNRSTCONT:
            // Noop, this is the default in Writer.
            break;
        case RTF_AFTNRESTART:
            // Noop, Writer does not support restarting endnotes at each section.
            break;
        case RTF_FTNBJ:
            // Noop, this is the default in Writer.
            break;
        case RTF_ENDDOC:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_RestartNumber_eachSect);
            putNestedSprm(m_aDefaultState.aParagraphSprms,
                          NS_ooxml::LN_EG_SectPrContents_footnotePr,
                          NS_ooxml::LN_EG_FtnEdnNumProps_numRestart, pValue);
        }
        break;
        case RTF_NOLINE:
            eraseNestedAttribute(m_aStates.top().aSectionSprms,
                                 NS_ooxml::LN_EG_SectPrContents_lnNumType,
                                 NS_ooxml::LN_CT_LineNumber_distance);
            break;
        case RTF_FORMSHADE:
            // Noop, this is the default in Writer.
            break;
        case RTF_PNGBLIP:
            m_aStates.top().aPicture.eStyle = RTFBmpStyle::PNG;
            break;
        case RTF_JPEGBLIP:
            m_aStates.top().aPicture.eStyle = RTFBmpStyle::JPEG;
            break;
        case RTF_POSYT:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                           NS_ooxml::LN_Value_doc_ST_YAlign_top);
            break;
        case RTF_POSYB:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                           NS_ooxml::LN_Value_doc_ST_YAlign_bottom);
            break;
        case RTF_POSYC:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                           NS_ooxml::LN_Value_doc_ST_YAlign_center);
            break;
        case RTF_POSYIN:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                           NS_ooxml::LN_Value_doc_ST_YAlign_inside);
            break;
        case RTF_POSYOUT:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                           NS_ooxml::LN_Value_doc_ST_YAlign_outside);
            break;
        case RTF_POSYIL:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                           NS_ooxml::LN_Value_doc_ST_YAlign_inline);
            break;

        case RTF_PHMRG:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hAnchor,
                                           NS_ooxml::LN_Value_doc_ST_HAnchor_margin);
            break;
        case RTF_PVMRG:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vAnchor,
                                           NS_ooxml::LN_Value_doc_ST_VAnchor_margin);
            break;
        case RTF_PHPG:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hAnchor,
                                           NS_ooxml::LN_Value_doc_ST_HAnchor_page);
            break;
        case RTF_PVPG:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vAnchor,
                                           NS_ooxml::LN_Value_doc_ST_VAnchor_page);
            break;
        case RTF_PHCOL:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hAnchor,
                                           NS_ooxml::LN_Value_doc_ST_HAnchor_text);
            break;
        case RTF_PVPARA:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vAnchor,
                                           NS_ooxml::LN_Value_doc_ST_VAnchor_text);
            break;

        case RTF_POSXC:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign,
                                           NS_ooxml::LN_Value_doc_ST_XAlign_center);
            break;
        case RTF_POSXI:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign,
                                           NS_ooxml::LN_Value_doc_ST_XAlign_inside);
            break;
        case RTF_POSXO:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign,
                                           NS_ooxml::LN_Value_doc_ST_XAlign_outside);
            break;
        case RTF_POSXL:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign,
                                           NS_ooxml::LN_Value_doc_ST_XAlign_left);
            break;
        case RTF_POSXR:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign,
                                           NS_ooxml::LN_Value_doc_ST_XAlign_right);
            break;

        case RTF_DPLINE:
        case RTF_DPRECT:
        case RTF_DPELLIPSE:
        case RTF_DPTXBX:
        case RTF_DPPOLYLINE:
        case RTF_DPPOLYGON:
        {
            sal_Int32 nType = 0;
            switch (nKeyword)
            {
                case RTF_DPLINE:
                {
                    uno::Reference<drawing::XShape> xShape(
                        getModelFactory()->createInstance("com.sun.star.drawing.LineShape"),
                        uno::UNO_QUERY);
                    m_aStates.top().aDrawingObject.setShape(xShape);
                    break;
                }
                case RTF_DPPOLYLINE:
                {
                    // The reason this is not a simple CustomShape is that in the old syntax we have no ViewBox info.
                    uno::Reference<drawing::XShape> xShape(
                        getModelFactory()->createInstance("com.sun.star.drawing.PolyLineShape"),
                        uno::UNO_QUERY);
                    m_aStates.top().aDrawingObject.setShape(xShape);
                    break;
                }
                case RTF_DPPOLYGON:
                {
                    uno::Reference<drawing::XShape> xShape(
                        getModelFactory()->createInstance("com.sun.star.drawing.PolyPolygonShape"),
                        uno::UNO_QUERY);
                    m_aStates.top().aDrawingObject.setShape(xShape);
                    break;
                }
                case RTF_DPRECT:
                {
                    uno::Reference<drawing::XShape> xShape(
                        getModelFactory()->createInstance("com.sun.star.drawing.RectangleShape"),
                        uno::UNO_QUERY);
                    m_aStates.top().aDrawingObject.setShape(xShape);
                    break;
                }
                case RTF_DPELLIPSE:
                    nType = ESCHER_ShpInst_Ellipse;
                    break;
                case RTF_DPTXBX:
                {
                    uno::Reference<drawing::XShape> xShape(
                        getModelFactory()->createInstance("com.sun.star.text.TextFrame"),
                        uno::UNO_QUERY);
                    m_aStates.top().aDrawingObject.setShape(xShape);
                    std::vector<beans::PropertyValue> aDefaults
                        = RTFSdrImport::getTextFrameDefaults(false);
                    for (const auto& rDefault : aDefaults)
                    {
                        if (!findPropertyName(m_aStates.top().aDrawingObject.getPendingProperties(),
                                              rDefault.Name))
                            m_aStates.top().aDrawingObject.getPendingProperties().push_back(
                                rDefault);
                    }
                    checkFirstRun();
                    Mapper().startShape(m_aStates.top().aDrawingObject.getShape());
                    m_aStates.top().aDrawingObject.setHadShapeText(true);
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
                m_aStates.top().aDrawingObject.setShape(xShape);
            }
            uno::Reference<drawing::XDrawPageSupplier> xDrawSupplier(m_xDstDoc, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xPropertySet(
                m_aStates.top().aDrawingObject.getShape(), uno::UNO_QUERY);
            m_aStates.top().aDrawingObject.setPropertySet(xPropertySet);
            if (xDrawSupplier.is())
            {
                uno::Reference<drawing::XShapes> xShapes(xDrawSupplier->getDrawPage(),
                                                         uno::UNO_QUERY);
                if (xShapes.is() && nKeyword != RTF_DPTXBX)
                {
                    // set default VertOrient before inserting
                    m_aStates.top().aDrawingObject.getPropertySet()->setPropertyValue(
                        "VertOrient", uno::makeAny(text::VertOrientation::NONE));
                    xShapes->add(m_aStates.top().aDrawingObject.getShape());
                }
            }
            if (nType)
            {
                uno::Reference<drawing::XEnhancedCustomShapeDefaulter> xDefaulter(
                    m_aStates.top().aDrawingObject.getShape(), uno::UNO_QUERY);
                xDefaulter->createCustomShapeDefaults(OUString::number(nType));
            }
            std::vector<beans::PropertyValue>& rPendingProperties
                = m_aStates.top().aDrawingObject.getPendingProperties();
            for (auto& rPendingProperty : rPendingProperties)
                m_aStates.top().aDrawingObject.getPropertySet()->setPropertyValue(
                    rPendingProperty.Name, rPendingProperty.Value);
            m_pSdrImport->resolveDhgt(m_aStates.top().aDrawingObject.getPropertySet(),
                                      m_aStates.top().aDrawingObject.getDhgt(), /*bOldStyle=*/true);
        }
        break;
        case RTF_DOBXMARGIN:
        case RTF_DOBYMARGIN:
        {
            beans::PropertyValue aPropertyValue;
            aPropertyValue.Name = (nKeyword == RTF_DOBXMARGIN ? OUString("HoriOrientRelation")
                                                              : OUString("VertOrientRelation"));
            aPropertyValue.Value <<= text::RelOrientation::PAGE_PRINT_AREA;
            m_aStates.top().aDrawingObject.getPendingProperties().push_back(aPropertyValue);
        }
        break;
        case RTF_DOBXPAGE:
        case RTF_DOBYPAGE:
        {
            beans::PropertyValue aPropertyValue;
            aPropertyValue.Name = (nKeyword == RTF_DOBXPAGE ? OUString("HoriOrientRelation")
                                                            : OUString("VertOrientRelation"));
            aPropertyValue.Value <<= text::RelOrientation::PAGE_FRAME;
            m_aStates.top().aDrawingObject.getPendingProperties().push_back(aPropertyValue);
        }
        break;
        case RTF_DOBYPARA:
        {
            beans::PropertyValue aPropertyValue;
            aPropertyValue.Name = "VertOrientRelation";
            aPropertyValue.Value <<= text::RelOrientation::FRAME;
            m_aStates.top().aDrawingObject.getPendingProperties().push_back(aPropertyValue);
        }
        break;
        case RTF_CONTEXTUALSPACE:
        {
            auto pValue = new RTFValue(1);
            m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_contextualSpacing, pValue);
        }
        break;
        case RTF_LINKSTYLES:
        {
            auto pValue = new RTFValue(1);
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_linkStyles, pValue);
        }
        break;
        case RTF_PNLVLBODY:
        {
            auto pValue = new RTFValue(2);
            m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_AbstractNum_nsid, pValue);
        }
        break;
        case RTF_PNDEC:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_decimal);
            m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_numFmt, pValue);
        }
        break;
        case RTF_PNLVLBLT:
        {
            m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_AbstractNum_nsid, new RTFValue(1));
            m_aStates.top().aTableSprms.set(
                NS_ooxml::LN_CT_Lvl_numFmt,
                new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_bullet));
        }
        break;
        case RTF_LANDSCAPE:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_PageOrientation_landscape);
            putNestedAttribute(m_aDefaultState.aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgSz,
                               NS_ooxml::LN_CT_PageSz_orient, pValue);
            [[fallthrough]]; // set the default + current value
        }
        case RTF_LNDSCPSXN:
        {
            auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_PageOrientation_landscape);
            putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgSz,
                               NS_ooxml::LN_CT_PageSz_orient, pValue);
        }
        break;
        case RTF_SHPBXPAGE:
            m_aStates.top().aShape.setHoriOrientRelation(text::RelOrientation::PAGE_FRAME);
            m_aStates.top().aShape.setHoriOrientRelationToken(
                NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_page);
            break;
        case RTF_SHPBYPAGE:
            m_aStates.top().aShape.setVertOrientRelation(text::RelOrientation::PAGE_FRAME);
            m_aStates.top().aShape.setVertOrientRelationToken(
                NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_page);
            break;
        case RTF_DPLINEHOLLOW:
            m_aStates.top().aDrawingObject.setFLine(0);
            break;
        case RTF_DPROUNDR:
            if (m_aStates.top().aDrawingObject.getPropertySet().is())
                // Seems this old syntax has no way to specify a custom radius, and this is the default
                m_aStates.top().aDrawingObject.getPropertySet()->setPropertyValue(
                    "CornerRadius", uno::makeAny(sal_Int32(83)));
            break;
        case RTF_NOWRAP:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_wrap,
                                           NS_ooxml::LN_Value_doc_ST_Wrap_notBeside);
            break;
        case RTF_MNOR:
            m_bMathNor = true;
            break;
        case RTF_REVISIONS:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_trackRevisions, new RTFValue(1));
            break;
        case RTF_BRDRSH:
            putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_shadow, new RTFValue(1));
            break;
        case RTF_NOCOLBAL:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Compat_noColumnBalance, new RTFValue(1));
            break;
        case RTF_MARGMIRROR:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_mirrorMargins, new RTFValue(1));
            break;
        case RTF_SAUTOUPD:
            m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Style_autoRedefine, new RTFValue(1));
            break;
        case RTF_WIDOWCTRL:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_widowControl, new RTFValue(1));
            break;
        case RTF_LINEBETCOL:
            putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_cols,
                               NS_ooxml::LN_CT_Columns_sep, new RTFValue(1));
            break;
        case RTF_PGNRESTART:
            putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgNumType,
                               NS_ooxml::LN_CT_PageNumber_start, new RTFValue(1));
            break;
        case RTF_PGNUCLTR:
        {
            auto pIntValue = new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_upperLetter);
            putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgNumType,
                               NS_ooxml::LN_CT_PageNumber_fmt, pIntValue);
        }
        break;
        case RTF_PGNLCLTR:
        {
            auto pIntValue = new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter);
            putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgNumType,
                               NS_ooxml::LN_CT_PageNumber_fmt, pIntValue);
        }
        break;
        case RTF_PGNUCRM:
        {
            auto pIntValue = new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_upperRoman);
            putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgNumType,
                               NS_ooxml::LN_CT_PageNumber_fmt, pIntValue);
        }
        break;
        case RTF_PGNLCRM:
        {
            auto pIntValue = new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman);
            putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgNumType,
                               NS_ooxml::LN_CT_PageNumber_fmt, pIntValue);
        }
        break;
        case RTF_PGNDEC:
        {
            auto pIntValue = new RTFValue(NS_ooxml::LN_Value_ST_NumberFormat_decimal);
            putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgNumType,
                               NS_ooxml::LN_CT_PageNumber_fmt, pIntValue);
        }
        break;
        case RTF_HTMAUTSP:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Compat_doNotUseHTMLParagraphAutoSpacing,
                                      new RTFValue(0));
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

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
