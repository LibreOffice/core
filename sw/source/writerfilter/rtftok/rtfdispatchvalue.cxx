/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtfdocumentimpl.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <comphelper/sequence.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <osl/thread.h>
#include <sal/log.hxx>
#include <rtl/tencinfo.h>
#include <tools/UnitConversion.hxx>

#include <ooxml/resourceids.hxx>

#include "rtfcharsets.hxx"
#include "rtffly.hxx"
#include "rtfreferenceproperties.hxx"
#include "rtfskipdestination.hxx"

#include <unotools/defaultencoding.hxx>
#include <unotools/wincodepage.hxx>

using namespace com::sun::star;

namespace writerfilter
{
static int getNumberFormat(int nParam)
{
    static const int aMap[]
        = { NS_ooxml::LN_Value_ST_NumberFormat_decimal,
            NS_ooxml::LN_Value_ST_NumberFormat_upperRoman,
            NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman,
            NS_ooxml::LN_Value_ST_NumberFormat_upperLetter,
            NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter,
            NS_ooxml::LN_Value_ST_NumberFormat_ordinal,
            NS_ooxml::LN_Value_ST_NumberFormat_cardinalText,
            NS_ooxml::LN_Value_ST_NumberFormat_ordinalText,
            NS_ooxml::LN_Value_ST_NumberFormat_none, // Undefined in RTF 1.8 spec.
            NS_ooxml::LN_Value_ST_NumberFormat_none, // Undefined in RTF 1.8 spec.
            NS_ooxml::LN_Value_ST_NumberFormat_ideographDigital,
            NS_ooxml::LN_Value_ST_NumberFormat_japaneseCounting,
            NS_ooxml::LN_Value_ST_NumberFormat_aiueo,
            NS_ooxml::LN_Value_ST_NumberFormat_iroha,
            NS_ooxml::LN_Value_ST_NumberFormat_decimalFullWidth,
            NS_ooxml::LN_Value_ST_NumberFormat_decimalHalfWidth,
            NS_ooxml::LN_Value_ST_NumberFormat_japaneseLegal,
            NS_ooxml::LN_Value_ST_NumberFormat_japaneseDigitalTenThousand,
            NS_ooxml::LN_Value_ST_NumberFormat_decimalEnclosedCircleChinese,
            NS_ooxml::LN_Value_ST_NumberFormat_decimalFullWidth2,
            NS_ooxml::LN_Value_ST_NumberFormat_aiueoFullWidth,
            NS_ooxml::LN_Value_ST_NumberFormat_irohaFullWidth,
            NS_ooxml::LN_Value_ST_NumberFormat_decimalZero,
            NS_ooxml::LN_Value_ST_NumberFormat_bullet,
            NS_ooxml::LN_Value_ST_NumberFormat_ganada,
            NS_ooxml::LN_Value_ST_NumberFormat_chosung,
            NS_ooxml::LN_Value_ST_NumberFormat_decimalEnclosedFullstop,
            NS_ooxml::LN_Value_ST_NumberFormat_decimalEnclosedParen,
            NS_ooxml::LN_Value_ST_NumberFormat_decimalEnclosedCircleChinese,
            NS_ooxml::LN_Value_ST_NumberFormat_ideographEnclosedCircle,
            NS_ooxml::LN_Value_ST_NumberFormat_ideographTraditional,
            NS_ooxml::LN_Value_ST_NumberFormat_ideographZodiac,
            NS_ooxml::LN_Value_ST_NumberFormat_ideographZodiacTraditional,
            NS_ooxml::LN_Value_ST_NumberFormat_taiwaneseCounting,
            NS_ooxml::LN_Value_ST_NumberFormat_ideographLegalTraditional,
            NS_ooxml::LN_Value_ST_NumberFormat_taiwaneseCountingThousand,
            NS_ooxml::LN_Value_ST_NumberFormat_taiwaneseDigital,
            NS_ooxml::LN_Value_ST_NumberFormat_chineseCounting,
            NS_ooxml::LN_Value_ST_NumberFormat_chineseLegalSimplified,
            NS_ooxml::LN_Value_ST_NumberFormat_chineseCountingThousand,
            NS_ooxml::LN_Value_ST_NumberFormat_decimal,
            NS_ooxml::LN_Value_ST_NumberFormat_koreanDigital,
            NS_ooxml::LN_Value_ST_NumberFormat_koreanCounting,
            NS_ooxml::LN_Value_ST_NumberFormat_koreanLegal,
            NS_ooxml::LN_Value_ST_NumberFormat_koreanDigital2,
            NS_ooxml::LN_Value_ST_NumberFormat_hebrew1,
            NS_ooxml::LN_Value_ST_NumberFormat_arabicAlpha,
            NS_ooxml::LN_Value_ST_NumberFormat_hebrew2,
            NS_ooxml::LN_Value_ST_NumberFormat_arabicAbjad };
    const int nLen = SAL_N_ELEMENTS(aMap);
    int nValue = 0;
    if (nParam >= 0 && nParam < nLen)
        nValue = aMap[nParam];
    else // 255 and the other cases.
        nValue = NS_ooxml::LN_Value_ST_NumberFormat_none;
    return nValue;
}

namespace rtftok
{
bool RTFDocumentImpl::dispatchTableSprmValue(RTFKeyword nKeyword, int nParam)
{
    int nSprm = 0;
    tools::SvRef<RTFValue> pIntValue(new RTFValue(nParam));
    switch (nKeyword)
    {
        case RTFKeyword::LEVELJC:
        {
            nSprm = NS_ooxml::LN_CT_Lvl_lvlJc;
            int nValue = 0;
            switch (nParam)
            {
                case 0:
                    nValue = NS_ooxml::LN_Value_ST_Jc_left;
                    break;
                case 1:
                    nValue = NS_ooxml::LN_Value_ST_Jc_center;
                    break;
                case 2:
                    nValue = NS_ooxml::LN_Value_ST_Jc_right;
                    break;
            }
            pIntValue = new RTFValue(nValue);
            break;
        }
        case RTFKeyword::LEVELSTARTAT:
            nSprm = NS_ooxml::LN_CT_Lvl_start;
            break;
        case RTFKeyword::LEVELPICTURE:
            nSprm = NS_ooxml::LN_CT_Lvl_lvlPicBulletId;
            break;
        case RTFKeyword::SBASEDON:
            nSprm = NS_ooxml::LN_CT_Style_basedOn;
            pIntValue = new RTFValue(getStyleName(nParam));
            break;
        case RTFKeyword::SNEXT:
            nSprm = NS_ooxml::LN_CT_Style_next;
            pIntValue = new RTFValue(getStyleName(nParam));
            break;
        case RTFKeyword::LEVELLEGAL:
            nSprm = NS_ooxml::LN_CT_Lvl_isLgl;
            break;
        default:
            break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().getTableSprms().set(nSprm, pIntValue);
        return true;
    }
    if (nKeyword == RTFKeyword::LEVELNFC)
    {
        pIntValue = new RTFValue(getNumberFormat(nParam));
        putNestedAttribute(m_aStates.top().getTableSprms(), NS_ooxml::LN_CT_Lvl_numFmt,
                           NS_ooxml::LN_CT_NumFmt_val, pIntValue);
        return true;
    }

    return false;
}

bool RTFDocumentImpl::dispatchCharacterSprmValue(RTFKeyword nKeyword, int nParam)
{
    int nSprm = 0;
    tools::SvRef<RTFValue> pIntValue(new RTFValue(nParam));

    switch (nKeyword)
    {
        case RTFKeyword::FS:
        case RTFKeyword::AFS:
            switch (m_aStates.top().getRunType())
            {
                case RTFParserState::RunType::HICH:
                case RTFParserState::RunType::RTLCH_LTRCH_1:
                case RTFParserState::RunType::LTRCH_RTLCH_2:
                    nSprm = NS_ooxml::LN_EG_RPrBase_szCs;
                    break;
                case RTFParserState::RunType::NONE:
                case RTFParserState::RunType::LOCH:
                case RTFParserState::RunType::LTRCH_RTLCH_1:
                case RTFParserState::RunType::RTLCH_LTRCH_2:
                case RTFParserState::RunType::DBCH:
                default:
                    nSprm = NS_ooxml::LN_EG_RPrBase_sz;
                    break;
            }
            break;
        case RTFKeyword::EXPNDTW:
            nSprm = NS_ooxml::LN_EG_RPrBase_spacing;
            break;
        case RTFKeyword::KERNING:
            nSprm = NS_ooxml::LN_EG_RPrBase_kern;
            break;
        case RTFKeyword::CHARSCALEX:
            nSprm = NS_ooxml::LN_EG_RPrBase_w;
            break;
        default:
            break;
    }
    if (nSprm > 0)
    {
        if (m_aStates.top().getDestination() == Destination::LISTLEVEL)
        {
            m_aStates.top().getTableSprms().set(nSprm, pIntValue);
        }
        else
        {
            m_aStates.top().getCharacterSprms().set(nSprm, pIntValue);
        }
        return true;
    }

    return false;
}

bool RTFDocumentImpl::dispatchCharacterAttributeValue(RTFKeyword nKeyword, int nParam)
{
    int nSprm = 0;

    switch (nKeyword)
    {
        case RTFKeyword::LANG:
        case RTFKeyword::ALANG:
            switch (m_aStates.top().getRunType())
            {
                case RTFParserState::RunType::HICH:
                case RTFParserState::RunType::RTLCH_LTRCH_1:
                case RTFParserState::RunType::LTRCH_RTLCH_2:
                    nSprm = NS_ooxml::LN_CT_Language_bidi;
                    break;
                case RTFParserState::RunType::DBCH:
                    nSprm = NS_ooxml::LN_CT_Language_eastAsia;
                    break;
                case RTFParserState::RunType::NONE:
                case RTFParserState::RunType::LOCH:
                case RTFParserState::RunType::LTRCH_RTLCH_1:
                case RTFParserState::RunType::RTLCH_LTRCH_2:
                default:
                    nSprm = NS_ooxml::LN_CT_Language_val;
                    break;
            }
            break;
        case RTFKeyword::LANGFE: // this one is always CJK apparently
            nSprm = NS_ooxml::LN_CT_Language_eastAsia;
            break;
        default:
            break;
    }
    if (nSprm > 0)
    {
        LanguageTag aTag((LanguageType(static_cast<sal_uInt16>(nParam))));
        auto pValue = new RTFValue(aTag.getBcp47());
        putNestedAttribute(m_aStates.top().getCharacterSprms(), NS_ooxml::LN_EG_RPrBase_lang, nSprm,
                           pValue);
        // Language is a character property, but we should store it at a paragraph level as well for fields.
        if (nKeyword == RTFKeyword::LANG && m_bNeedPap)
            putNestedAttribute(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_EG_RPrBase_lang,
                               nSprm, pValue);
        return true;
    }

    return false;
}

bool RTFDocumentImpl::dispatchParagraphSprmValue(RTFKeyword nKeyword, int nParam)
{
    int nSprm = 0;
    tools::SvRef<RTFValue> pIntValue(new RTFValue(nParam));

    switch (nKeyword)
    {
        case RTFKeyword::ITAP:
            nSprm = NS_ooxml::LN_tblDepth;
            // tdf#117268: If \itap0 is encountered inside tables (between \cellxN and \cell), then
            // use the default value (1), as Word apparently does
            if (nParam == 0 && (m_nTopLevelCells != 0 || m_nNestedCells != 0))
            {
                nParam = 1;
                pIntValue = new RTFValue(nParam);
            }
            break;
        default:
            break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().getParagraphSprms().set(nSprm, pIntValue);
        if (nKeyword == RTFKeyword::ITAP && nParam > 0)
        {
            while (m_aTableBufferStack.size() < sal::static_int_cast<std::size_t>(nParam))
            {
                m_aTableBufferStack.emplace_back();
            }
            // Invalid tables may omit INTBL after ITAP
            dispatchFlag(RTFKeyword::INTBL); // sets newly pushed buffer as current
            assert(m_aStates.top().getCurrentBuffer() == &m_aTableBufferStack.back());
        }
        return true;
    }

    return false;
}

bool RTFDocumentImpl::dispatchInfoValue(RTFKeyword nKeyword, int nParam)
{
    int nSprm = 0;

    switch (nKeyword)
    {
        case RTFKeyword::YR:
        {
            m_aStates.top().setYear(nParam);
            nSprm = 1;
        }
        break;
        case RTFKeyword::MO:
        {
            m_aStates.top().setMonth(nParam);
            nSprm = 1;
        }
        break;
        case RTFKeyword::DY:
        {
            m_aStates.top().setDay(nParam);
            nSprm = 1;
        }
        break;
        case RTFKeyword::HR:
        {
            m_aStates.top().setHour(nParam);
            nSprm = 1;
        }
        break;
        case RTFKeyword::MIN:
        {
            m_aStates.top().setMinute(nParam);
            nSprm = 1;
        }
        break;
        default:
            break;
    }

    return nSprm > 0;
}

bool RTFDocumentImpl::dispatchFrameValue(RTFKeyword nKeyword, int nParam)
{
    Id nId = 0;
    switch (nKeyword)
    {
        case RTFKeyword::ABSW:
            nId = NS_ooxml::LN_CT_FramePr_w;
            break;
        case RTFKeyword::ABSH:
            nId = NS_ooxml::LN_CT_FramePr_h;
            break;
        case RTFKeyword::POSX:
        {
            nId = NS_ooxml::LN_CT_FramePr_x;
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_xAlign, 0);
        }
        break;
        case RTFKeyword::POSY:
        {
            nId = NS_ooxml::LN_CT_FramePr_y;
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_yAlign, 0);
        }
        break;
        default:
            break;
    }

    if (nId > 0)
    {
        m_bNeedPap = true;
        // Don't try to support text frames inside tables for now.
        if (m_aStates.top().getCurrentBuffer() != &m_aTableBufferStack.back())
            m_aStates.top().getFrame().setSprm(nId, nParam);

        return true;
    }

    return false;
}

bool RTFDocumentImpl::dispatchTableValue(RTFKeyword nKeyword, int nParam)
{
    int nSprm = 0;
    tools::SvRef<RTFValue> pIntValue(new RTFValue(nParam));

    switch (nKeyword)
    {
        case RTFKeyword::CELLX:
        {
            int& rCurrentCellX(
                (Destination::NESTEDTABLEPROPERTIES == m_aStates.top().getDestination())
                    ? m_nNestedCurrentCellX
                    : m_nTopLevelCurrentCellX);
            int nCellX = nParam - rCurrentCellX;

            if (!nCellX && nParam > 0)
            {
                // If width of cell is 0, BUT there is a value for \cellxN use minimal
                // possible width. But if \cellxN has no value leave 0 so autofit will
                // try to resolve this.

                // sw/source/filter/inc/wrtswtbl.hxx, minimal possible width of cells.
                const int COL_DFLT_WIDTH = 41;
                nCellX = COL_DFLT_WIDTH;
            }

            // If there is a negative left margin, then the first cellx is relative to that.
            RTFValue::Pointer_t pTblInd
                = m_aStates.top().getTableRowSprms().find(NS_ooxml::LN_CT_TblPrBase_tblInd);
            if (rCurrentCellX == 0 && pTblInd)
            {
                RTFValue::Pointer_t pWidth
                    = pTblInd->getAttributes().find(NS_ooxml::LN_CT_TblWidth_w);
                if (pWidth && pWidth->getInt() < 0)
                    nCellX = -1 * (pWidth->getInt() - nParam);
            }

            rCurrentCellX = nParam;
            auto pXValue = new RTFValue(nCellX);
            m_aStates.top().getTableRowSprms().set(NS_ooxml::LN_CT_TblGridBase_gridCol, pXValue,
                                                   RTFOverwrite::NO_APPEND);
            if (Destination::NESTEDTABLEPROPERTIES == m_aStates.top().getDestination())
            {
                m_nNestedCells++;
                // Push cell properties.
                m_aNestedTableCellsSprms.push_back(m_aStates.top().getTableCellSprms());
                m_aNestedTableCellsAttributes.push_back(m_aStates.top().getTableCellAttributes());
            }
            else
            {
                m_nTopLevelCells++;
                // Push cell properties.
                m_aTopLevelTableCellsSprms.push_back(m_aStates.top().getTableCellSprms());
                m_aTopLevelTableCellsAttributes.push_back(m_aStates.top().getTableCellAttributes());
            }

            m_aStates.top().getTableCellSprms() = m_aDefaultState.getTableCellSprms();
            m_aStates.top().getTableCellAttributes() = m_aDefaultState.getTableCellAttributes();
            // We assume text after a row definition always belongs to the table, to handle text before the real INTBL token
            dispatchFlag(RTFKeyword::INTBL);
            if (!m_nCellxMax)
            {
                // Wasn't in table, but now is -> tblStart.
                RTFSprms aAttributes;
                RTFSprms aSprms;
                aSprms.set(NS_ooxml::LN_tblStart, new RTFValue(1));
                writerfilter::Reference<Properties>::Pointer_t pProperties
                    = new RTFReferenceProperties(std::move(aAttributes), std::move(aSprms));
                Mapper().props(pProperties);
            }
            m_nCellxMax = std::max(m_nCellxMax, nParam);
            return true;
        }
        break;
        case RTFKeyword::TRRH:
        {
            OUString hRule(u"auto"_ustr);
            if (nParam < 0)
            {
                tools::SvRef<RTFValue> pAbsValue(new RTFValue(-nParam));
                std::swap(pIntValue, pAbsValue);

                hRule = "exact";
            }
            else if (nParam > 0)
                hRule = "atLeast";

            putNestedAttribute(m_aStates.top().getTableRowSprms(),
                               NS_ooxml::LN_CT_TrPrBase_trHeight, NS_ooxml::LN_CT_Height_val,
                               pIntValue);

            auto pHRule = new RTFValue(hRule);
            putNestedAttribute(m_aStates.top().getTableRowSprms(),
                               NS_ooxml::LN_CT_TrPrBase_trHeight, NS_ooxml::LN_CT_Height_hRule,
                               pHRule);
            return true;
        }
        break;
        case RTFKeyword::TRLEFT:
        case RTFKeyword::TBLIND:
        {
            // the value is in twips
            auto const aDestination = m_aStates.top().getDestination();
            int& rCurrentTRLeft((Destination::NESTEDTABLEPROPERTIES == aDestination)
                                    ? m_nNestedTRLeft
                                    : m_nTopLevelTRLeft);
            int& rCurrentCellX((Destination::NESTEDTABLEPROPERTIES == aDestination)
                                   ? m_nNestedCurrentCellX
                                   : m_nTopLevelCurrentCellX);
            putNestedAttribute(m_aStates.top().getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblInd,
                               NS_ooxml::LN_CT_TblWidth_type,
                               new RTFValue(NS_ooxml::LN_Value_ST_TblWidth_dxa));

            if (nKeyword == RTFKeyword::TBLIND)
            {
                RTFValue::Pointer_t pCellMargin
                    = m_aStates.top().getTableRowSprms().find(NS_ooxml::LN_CT_TblPrBase_tblCellMar);
                if (pCellMargin)
                {
                    RTFValue::Pointer_t pMarginLeft
                        = pCellMargin->getSprms().find(NS_ooxml::LN_CT_TcMar_left);
                    if (pMarginLeft)
                        nParam -= pMarginLeft->getAttributes()
                                      .find(NS_ooxml::LN_CT_TblWidth_w)
                                      ->getInt();
                }
                rCurrentTRLeft = nParam;
            }
            else
                rCurrentTRLeft = rCurrentCellX = nParam;

            putNestedAttribute(m_aStates.top().getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblInd,
                               +NS_ooxml::LN_CT_TblWidth_w, new RTFValue(nParam));
            return true;
        }
        break;
        case RTFKeyword::CLSHDNG:
        {
            int nValue = -1;

            if (nParam < 1)
                nValue = NS_ooxml::LN_Value_ST_Shd_clear;
            else if (nParam < 750)
                // Values in between 1 and 250 visually closer to 0% shading (white)
                // But this will mean "no shading" while cell actually have some.
                // So lets use minimal available value.
                nValue = NS_ooxml::LN_Value_ST_Shd_pct5;
            else if (nParam < 1100)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct10;
            else if (nParam < 1350)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct12;
            else if (nParam < 1750)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct15;
            else if (nParam < 2250)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct20;
            else if (nParam < 2750)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct25;
            else if (nParam < 3250)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct30;
            else if (nParam < 3600)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct35;
            else if (nParam < 3850)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct37;
            else if (nParam < 4250)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct40;
            else if (nParam < 4750)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct45;
            else if (nParam < 5250)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct50;
            else if (nParam < 5750)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct55;
            else if (nParam < 6100)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct60;
            else if (nParam < 6350)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct62;
            else if (nParam < 6750)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct65;
            else if (nParam < 7250)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct70;
            else if (nParam < 7750)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct75;
            else if (nParam < 8250)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct80;
            else if (nParam < 8600)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct85;
            else if (nParam < 8850)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct87;
            else if (nParam < 9250)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct90;
            else if (nParam < 9750)
                nValue = NS_ooxml::LN_Value_ST_Shd_pct95;
            else
                // Solid fill
                nValue = NS_ooxml::LN_Value_ST_Shd_solid;

            putNestedAttribute(m_aStates.top().getTableCellSprms(), NS_ooxml::LN_CT_TcPrBase_shd,
                               NS_ooxml::LN_CT_Shd_val, new RTFValue(nValue));
            return true;
        }
        break;
        case RTFKeyword::CLPADB:
        case RTFKeyword::CLPADL:
        case RTFKeyword::CLPADR:
        case RTFKeyword::CLPADT:
        {
            RTFSprms aAttributes;
            aAttributes.set(NS_ooxml::LN_CT_TblWidth_type,
                            new RTFValue(NS_ooxml::LN_Value_ST_TblWidth_dxa));
            aAttributes.set(NS_ooxml::LN_CT_TblWidth_w, new RTFValue(nParam));
            // Top and left is swapped, that's what Word does.
            switch (nKeyword)
            {
                case RTFKeyword::CLPADB:
                    nSprm = NS_ooxml::LN_CT_TcMar_bottom;
                    break;
                case RTFKeyword::CLPADL:
                    nSprm = NS_ooxml::LN_CT_TcMar_top;
                    break;
                case RTFKeyword::CLPADR:
                    nSprm = NS_ooxml::LN_CT_TcMar_right;
                    break;
                case RTFKeyword::CLPADT:
                    nSprm = NS_ooxml::LN_CT_TcMar_left;
                    break;
                default:
                    break;
            }
            putNestedSprm(m_aStates.top().getTableCellSprms(), NS_ooxml::LN_CT_TcPrBase_tcMar,
                          nSprm, new RTFValue(aAttributes));
            return true;
        }
        break;
        case RTFKeyword::TRPADDFB:
        case RTFKeyword::TRPADDFL:
        case RTFKeyword::TRPADDFR:
        case RTFKeyword::TRPADDFT:
        {
            RTFSprms aAttributes;
            switch (nParam)
            {
                case 3:
                    aAttributes.set(NS_ooxml::LN_CT_TblWidth_type,
                                    new RTFValue(NS_ooxml::LN_Value_ST_TblWidth_dxa));
                    break;
            }
            switch (nKeyword)
            {
                case RTFKeyword::TRPADDFB:
                    nSprm = NS_ooxml::LN_CT_TcMar_bottom;
                    break;
                case RTFKeyword::TRPADDFL:
                    nSprm = NS_ooxml::LN_CT_TcMar_left;
                    break;
                case RTFKeyword::TRPADDFR:
                    nSprm = NS_ooxml::LN_CT_TcMar_right;
                    break;
                case RTFKeyword::TRPADDFT:
                    nSprm = NS_ooxml::LN_CT_TcMar_top;
                    break;
                default:
                    break;
            }
            putNestedAttribute(m_aStates.top().getTableRowSprms(),
                               NS_ooxml::LN_CT_TblPrBase_tblCellMar, nSprm,
                               new RTFValue(aAttributes));
            // tdf#74795 also set on current cell, and as default for table cells
            // (why isn't this done by domainmapper?)
            putNestedAttribute(m_aStates.top().getTableCellSprms(), NS_ooxml::LN_CT_TcPrBase_tcMar,
                               nSprm, new RTFValue(aAttributes));
            putNestedAttribute(m_aDefaultState.getTableCellSprms(), NS_ooxml::LN_CT_TcPrBase_tcMar,
                               nSprm, new RTFValue(aAttributes));
            return true;
        }
        break;
        case RTFKeyword::TRPADDB:
        case RTFKeyword::TRPADDL:
        case RTFKeyword::TRPADDR:
        case RTFKeyword::TRPADDT:
        {
            RTFSprms aAttributes;
            aAttributes.set(NS_ooxml::LN_CT_TblWidth_w, new RTFValue(nParam));
            switch (nKeyword)
            {
                case RTFKeyword::TRPADDB:
                    nSprm = NS_ooxml::LN_CT_TcMar_bottom;
                    break;
                case RTFKeyword::TRPADDL:
                    nSprm = NS_ooxml::LN_CT_TcMar_left;
                    break;
                case RTFKeyword::TRPADDR:
                    nSprm = NS_ooxml::LN_CT_TcMar_right;
                    break;
                case RTFKeyword::TRPADDT:
                    nSprm = NS_ooxml::LN_CT_TcMar_top;
                    break;
                default:
                    break;
            }
            putNestedSprm(m_aStates.top().getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblCellMar,
                          nSprm, new RTFValue(aAttributes));
            // tdf#74795 also set on current cell, and as default for table cells
            // (why isn't this done by domainmapper?)
            putNestedSprm(m_aStates.top().getTableCellSprms(), NS_ooxml::LN_CT_TcPrBase_tcMar,
                          nSprm, new RTFValue(aAttributes));
            putNestedSprm(m_aDefaultState.getTableCellSprms(), NS_ooxml::LN_CT_TcPrBase_tcMar,
                          nSprm, new RTFValue(aAttributes));
            return true;
        }
        case RTFKeyword::TRGAPH:
            // Half of the space between the cells of a table row: default left/right table cell margin.
            if (nParam > 0)
            {
                RTFSprms aAttributes;
                aAttributes.set(NS_ooxml::LN_CT_TblWidth_type,
                                new RTFValue(NS_ooxml::LN_Value_ST_TblWidth_dxa));
                aAttributes.set(NS_ooxml::LN_CT_TblWidth_w, pIntValue);
                // FIXME: this is wrong, it is half-gap, needs to be distinguished from margin! depending on TRPADDFL/TRPADDFR
                putNestedSprm(m_aStates.top().getTableRowSprms(),
                              NS_ooxml::LN_CT_TblPrBase_tblCellMar, NS_ooxml::LN_CT_TblCellMar_left,
                              new RTFValue(aAttributes));
                putNestedSprm(m_aStates.top().getTableRowSprms(),
                              NS_ooxml::LN_CT_TblPrBase_tblCellMar,
                              NS_ooxml::LN_CT_TblCellMar_right, new RTFValue(aAttributes));
            }
            return true;
        case RTFKeyword::TRFTSWIDTH:
            putNestedAttribute(m_aStates.top().getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblW,
                               NS_ooxml::LN_CT_TblWidth_type, pIntValue);
            return true;
        case RTFKeyword::TRWWIDTH:
            putNestedAttribute(m_aStates.top().getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblW,
                               NS_ooxml::LN_CT_TblWidth_w, pIntValue);
            return true;
        default:
            break;
    }

    return false;
}

RTFError RTFDocumentImpl::dispatchValue(RTFKeyword nKeyword, int nParam)
{
    setNeedSect(true);
    checkUnicode(/*bUnicode =*/nKeyword != RTFKeyword::U, /*bHex =*/true);
    RTFSkipDestination aSkip(*this);
    int nSprm = 0;
    tools::SvRef<RTFValue> pIntValue(new RTFValue(nParam));
    // Trivial table sprms.
    if (dispatchTableSprmValue(nKeyword, nParam))
    {
        return RTFError::OK;
    }

    // Trivial character sprms.
    if (dispatchCharacterSprmValue(nKeyword, nParam))
    {
        return RTFError::OK;
    }

    // Trivial character attributes.
    if (dispatchCharacterAttributeValue(nKeyword, nParam))
    {
        return RTFError::OK;
    }

    // Trivial paragraph sprms.
    if (dispatchParagraphSprmValue(nKeyword, nParam))
    {
        return RTFError::OK;
    }

    // Info group.
    if (dispatchInfoValue(nKeyword, nParam))
    {
        return RTFError::OK;
    }

    // Frame size / position.
    if (dispatchFrameValue(nKeyword, nParam))
    {
        return RTFError::OK;
    }

    // Table-related values.
    if (dispatchTableValue(nKeyword, nParam))
    {
        return RTFError::OK;
    }

    // Then check for the more complex ones.
    switch (nKeyword)
    {
        case RTFKeyword::F:
        case RTFKeyword::AF:
            switch (m_aStates.top().getRunType())
            {
                case RTFParserState::RunType::RTLCH_LTRCH_1:
                case RTFParserState::RunType::LTRCH_RTLCH_2:
                    nSprm = NS_ooxml::LN_CT_Fonts_cs;
                    break;
                case RTFParserState::RunType::DBCH:
                    nSprm = NS_ooxml::LN_CT_Fonts_eastAsia;
                    break;
                case RTFParserState::RunType::NONE:
                case RTFParserState::RunType::LOCH:
                case RTFParserState::RunType::HICH:
                case RTFParserState::RunType::LTRCH_RTLCH_1:
                case RTFParserState::RunType::RTLCH_LTRCH_2:
                default:
                    nSprm = NS_ooxml::LN_CT_Fonts_ascii;
                    break;
            }

            if (m_aStates.top().getDestination() == Destination::FONTTABLE
                || m_aStates.top().getDestination() == Destination::FONTENTRY)
            {
                // Some text in buffer? It is font name. So previous font definition is complete
                if (m_aStates.top().getCurrentDestinationText()->getLength())
                    handleFontTableEntry();

                m_aFontIndexes.push_back(nParam);
                m_nCurrentFontIndex = getFontIndex(nParam);
            }
            else if (m_aStates.top().getDestination() == Destination::LISTLEVEL)
            {
                RTFSprms aFontAttributes;
                aFontAttributes.set(nSprm, new RTFValue(m_aFontNames[getFontIndex(nParam)]));
                RTFSprms aRunPropsSprms;
                aRunPropsSprms.set(NS_ooxml::LN_EG_RPrBase_rFonts, new RTFValue(aFontAttributes));
                m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_Lvl_rPr,
                                                    new RTFValue(RTFSprms(), aRunPropsSprms),
                                                    RTFOverwrite::NO_APPEND);
            }
            else
            {
                m_nCurrentFontIndex = getFontIndex(nParam);
                auto pValue = new RTFValue(getFontName(m_nCurrentFontIndex));
                putNestedAttribute(m_aStates.top().getCharacterSprms(),
                                   NS_ooxml::LN_EG_RPrBase_rFonts, nSprm, pValue);
                if (nKeyword == RTFKeyword::F)
                    m_aStates.top().setCurrentEncoding(getEncoding(m_nCurrentFontIndex));
            }
            break;
        case RTFKeyword::RED:
            m_aStates.top().getCurrentColor().SetRed(nParam);
            break;
        case RTFKeyword::GREEN:
            m_aStates.top().getCurrentColor().SetGreen(nParam);
            break;
        case RTFKeyword::BLUE:
            m_aStates.top().getCurrentColor().SetBlue(nParam);
            break;
        case RTFKeyword::FCHARSET:
        {
            // we always send text to the domain mapper in OUString, so no
            // need to send encoding info
            int i;
            for (i = 0; i < nRTFEncodings; i++)
            {
                if (aRTFEncodings[i].charset == nParam)
                    break;
            }
            if (i == nRTFEncodings)
                // not found
                return RTFError::OK;

            m_nCurrentEncoding
                = aRTFEncodings[i].codepage == 0 // Default (CP_ACP)
                      ? osl_getThreadTextEncoding()
                      : rtl_getTextEncodingFromWindowsCodePage(aRTFEncodings[i].codepage);
            m_aStates.top().setCurrentEncoding(m_nCurrentEncoding);
        }
        break;
        case RTFKeyword::ANSICPG:
        case RTFKeyword::CPG:
        {
            rtl_TextEncoding nEncoding
                = (nParam == 0)
                      ? utl_getWinTextEncodingFromLangStr(utl_getLocaleForGlobalDefaultEncoding())
                      : rtl_getTextEncodingFromWindowsCodePage(nParam);
            if (nKeyword == RTFKeyword::ANSICPG)
                m_aDefaultState.setCurrentEncoding(nEncoding);
            else
                m_nCurrentEncoding = nEncoding;
            m_aStates.top().setCurrentEncoding(nEncoding);
        }
        break;
        case RTFKeyword::CF:
        {
            RTFSprms aAttributes;
            auto pValue = new RTFValue(sal_uInt32(getColorTable(nParam)));
            aAttributes.set(NS_ooxml::LN_CT_Color_val, pValue);
            m_aStates.top().getCharacterSprms().set(NS_ooxml::LN_EG_RPrBase_color,
                                                    new RTFValue(aAttributes));
        }
        break;
        case RTFKeyword::S:
        {
            m_aStates.top().setCurrentStyleIndex(nParam);

            if (m_aStates.top().getDestination() == Destination::STYLESHEET
                || m_aStates.top().getDestination() == Destination::STYLEENTRY)
            {
                m_nCurrentStyleIndex = nParam;
                auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_StyleType_paragraph);
                m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_Style_type,
                                                         pValue); // paragraph style
            }
            else
            {
                OUString aName = getStyleName(nParam);
                if (!aName.isEmpty())
                {
                    if (m_aStates.top().getDestination() == Destination::LISTLEVEL)
                        m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_Lvl_pStyle,
                                                            new RTFValue(aName));
                    else
                        m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_CT_PPrBase_pStyle,
                                                                new RTFValue(aName));
                }
            }
        }
        break;
        case RTFKeyword::CS:
            m_aStates.top().setCurrentCharacterStyleIndex(nParam);
            if (m_aStates.top().getDestination() == Destination::STYLESHEET
                || m_aStates.top().getDestination() == Destination::STYLEENTRY)
            {
                m_nCurrentStyleIndex = nParam;
                auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_StyleType_character);
                m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_Style_type,
                                                         pValue); // character style
            }
            else
            {
                OUString aName = getStyleName(nParam);
                if (!aName.isEmpty())
                    m_aStates.top().getCharacterSprms().set(NS_ooxml::LN_EG_RPrBase_rStyle,
                                                            new RTFValue(aName));
            }
            break;
        case RTFKeyword::DS:
            if (m_aStates.top().getDestination() == Destination::STYLESHEET
                || m_aStates.top().getDestination() == Destination::STYLEENTRY)
            {
                m_nCurrentStyleIndex = nParam;
                auto pValue = new RTFValue(0); // TODO no value in enum StyleType?
                m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_Style_type,
                                                         pValue); // section style
            }
            break;
        case RTFKeyword::TS:
            if (m_aStates.top().getDestination() == Destination::STYLESHEET
                || m_aStates.top().getDestination() == Destination::STYLEENTRY)
            {
                m_nCurrentStyleIndex = nParam;
                // FIXME the correct value would be NS_ooxml::LN_Value_ST_StyleType_table but maybe table styles mess things up in dmapper, be cautious and disable them for now
                auto pValue = new RTFValue(0);
                m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_Style_type,
                                                         pValue); // table style
            }
            break;
        case RTFKeyword::DEFF:
            m_nDefaultFontIndex = nParam;
            break;
        case RTFKeyword::STSHFDBCH:
            // tdf#123703 switch off longer space sequence except in the case of the fixed compatibility setting font id 31505
            if (nParam != 31505)
                m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_longerSpaceSequence,
                                          new RTFValue(0));
            break;
        case RTFKeyword::DEFLANG:
        case RTFKeyword::ADEFLANG:
        {
            LanguageTag aTag((LanguageType(static_cast<sal_uInt16>(nParam))));
            auto pValue = new RTFValue(aTag.getBcp47());
            putNestedAttribute(m_aStates.top().getCharacterSprms(),
                               (nKeyword == RTFKeyword::DEFLANG ? NS_ooxml::LN_EG_RPrBase_lang
                                                                : NS_ooxml::LN_CT_Language_bidi),
                               nSprm, pValue);
        }
        break;
        case RTFKeyword::CHCBPAT:
        {
            auto pValue = new RTFValue(sal_uInt32(nParam ? getColorTable(nParam) : COL_AUTO));
            putNestedAttribute(m_aStates.top().getCharacterSprms(), NS_ooxml::LN_EG_RPrBase_shd,
                               NS_ooxml::LN_CT_Shd_fill, pValue);
        }
        break;
        case RTFKeyword::CLCBPAT:
        case RTFKeyword::CLCBPATRAW:
        {
            auto pValue = new RTFValue(sal_uInt32(getColorTable(nParam)));
            putNestedAttribute(m_aStates.top().getTableCellSprms(), NS_ooxml::LN_CT_TcPrBase_shd,
                               NS_ooxml::LN_CT_Shd_fill, pValue);
        }
        break;
        case RTFKeyword::CBPAT:
            if (nParam)
            {
                auto pValue = new RTFValue(sal_uInt32(getColorTable(nParam)));
                putNestedAttribute(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PrBase_shd,
                                   NS_ooxml::LN_CT_Shd_fill, pValue);
            }
            break;
        case RTFKeyword::ULC:
        {
            auto pValue = new RTFValue(sal_uInt32(getColorTable(nParam)));
            m_aStates.top().getCharacterSprms().set(0x6877, pValue);
        }
        break;
        case RTFKeyword::HIGHLIGHT:
        {
            auto pValue = new RTFValue(sal_uInt32(nParam ? getColorTable(nParam) : COL_AUTO));
            m_aStates.top().getCharacterSprms().set(NS_ooxml::LN_EG_RPrBase_highlight, pValue);
        }
        break;
        case RTFKeyword::UP:
        case RTFKeyword::DN:
        {
            auto pValue = new RTFValue(nParam * (nKeyword == RTFKeyword::UP ? 1 : -1));
            m_aStates.top().getCharacterSprms().set(NS_ooxml::LN_EG_RPrBase_position, pValue);
        }
        break;
        case RTFKeyword::HORZVERT:
        {
            auto pValue = new RTFValue(int(true));
            m_aStates.top().getCharacterAttributes().set(NS_ooxml::LN_CT_EastAsianLayout_vert,
                                                         pValue);
            if (nParam)
                // rotate fits to a single line
                m_aStates.top().getCharacterAttributes().set(
                    NS_ooxml::LN_CT_EastAsianLayout_vertCompress, pValue);
        }
        break;
        case RTFKeyword::EXPND:
        {
            // Convert quarter-points to twentieths of a point
            auto pValue = new RTFValue(nParam * 5);
            m_aStates.top().getCharacterSprms().set(NS_ooxml::LN_EG_RPrBase_spacing, pValue);
        }
        break;
        case RTFKeyword::TWOINONE:
        {
            auto pValue = new RTFValue(int(true));
            m_aStates.top().getCharacterAttributes().set(NS_ooxml::LN_CT_EastAsianLayout_combine,
                                                         pValue);
            Id nId = 0;
            switch (nParam)
            {
                case 0:
                    nId = NS_ooxml::LN_Value_ST_CombineBrackets_none;
                    break;
                case 1:
                    nId = NS_ooxml::LN_Value_ST_CombineBrackets_round;
                    break;
                case 2:
                    nId = NS_ooxml::LN_Value_ST_CombineBrackets_square;
                    break;
                case 3:
                    nId = NS_ooxml::LN_Value_ST_CombineBrackets_angle;
                    break;
                case 4:
                    nId = NS_ooxml::LN_Value_ST_CombineBrackets_curly;
                    break;
            }
            if (nId > 0)
                m_aStates.top().getCharacterAttributes().set(
                    NS_ooxml::LN_CT_EastAsianLayout_combineBrackets, new RTFValue(nId));
        }
        break;
        case RTFKeyword::SL:
        {
            // This is similar to RTFKeyword::ABSH, negative value means 'exact', positive means 'at least'.
            tools::SvRef<RTFValue> pValue(
                new RTFValue(NS_ooxml::LN_Value_doc_ST_LineSpacingRule_atLeast));
            if (nParam < 0)
            {
                pValue = new RTFValue(NS_ooxml::LN_Value_doc_ST_LineSpacingRule_exact);
                pIntValue = new RTFValue(-nParam);
            }
            m_aStates.top().getParagraphAttributes().set(NS_ooxml::LN_CT_Spacing_lineRule, pValue);
            m_aStates.top().getParagraphAttributes().set(NS_ooxml::LN_CT_Spacing_line, pIntValue);
        }
        break;
        case RTFKeyword::SLMULT:
            if (nParam > 0)
            {
                auto pValue = new RTFValue(NS_ooxml::LN_Value_doc_ST_LineSpacingRule_auto);
                m_aStates.top().getParagraphAttributes().set(NS_ooxml::LN_CT_Spacing_lineRule,
                                                             pValue);
            }
            break;
        case RTFKeyword::BRDRW:
        {
            // dmapper expects it in 1/8 pt, we have it in twip - but avoid rounding 1 to 0
            if (nParam > 1)
                nParam = nParam * 2 / 5;
            auto pValue = new RTFValue(nParam);
            putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_sz, pValue);
        }
        break;
        case RTFKeyword::BRDRCF:
        {
            auto pValue = new RTFValue(sal_uInt32(getColorTable(nParam)));
            putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_color, pValue);
        }
        break;
        case RTFKeyword::BRSP:
        {
            // dmapper expects it in points, we have it in twip
            auto pValue = new RTFValue(nParam / 20);
            putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_space, pValue);
        }
        break;
        case RTFKeyword::TX:
        {
            m_aStates.top().getTabAttributes().set(NS_ooxml::LN_CT_TabStop_pos, pIntValue);
            auto pValue = new RTFValue(m_aStates.top().getTabAttributes());
            if (m_aStates.top().getDestination() == Destination::LISTLEVEL)
                putNestedSprm(m_aStates.top().getTableSprms(), NS_ooxml::LN_CT_PPrBase_tabs,
                              NS_ooxml::LN_CT_Tabs_tab, pValue);
            else
                putNestedSprm(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_tabs,
                              NS_ooxml::LN_CT_Tabs_tab, pValue);
            m_aStates.top().getTabAttributes().clear();
        }
        break;
        case RTFKeyword::ILVL:
            putNestedSprm(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_numPr,
                          NS_ooxml::LN_CT_NumPr_ilvl, pIntValue);
            break;
        case RTFKeyword::LISTTEMPLATEID:
            // This one is not referenced anywhere, so it's pointless to store it at the moment.
            break;
        case RTFKeyword::LISTID:
        {
            if (m_aStates.top().getDestination() == Destination::LISTENTRY)
                m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_AbstractNum_abstractNumId,
                                                         pIntValue);
            else if (m_aStates.top().getDestination() == Destination::LISTOVERRIDEENTRY)
                m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_Num_abstractNumId, pIntValue);
            m_aStates.top().setCurrentListIndex(nParam);
        }
        break;
        case RTFKeyword::LS:
        {
            if (m_aStates.top().getDestination() == Destination::LISTOVERRIDEENTRY)
            {
                m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_AbstractNum_nsid,
                                                         pIntValue);
                m_aStates.top().setCurrentListOverrideIndex(nParam);
            }
            else
            {
                // Insert at the start, so properties inherited from the list
                // can be overridden by direct formatting. But still allow the
                // case when old-style paragraph numbering is already
                // tokenized.
                putNestedSprm(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_numPr,
                              NS_ooxml::LN_CT_NumPr_numId, pIntValue, RTFOverwrite::YES_PREPEND);
            }
        }
        break;
        case RTFKeyword::UC:
            if ((SAL_MIN_INT16 <= nParam) && (nParam <= SAL_MAX_INT16))
                m_aStates.top().setUc(nParam);
            break;
        case RTFKeyword::U:
            // sal_Unicode is unsigned 16-bit, RTF may represent that as a
            // signed SAL_MIN_INT16..SAL_MAX_INT16 or 0..SAL_MAX_UINT16. The
            // static_cast() will do the right thing.
            if ((SAL_MIN_INT16 <= nParam) && (nParam <= SAL_MAX_UINT16))
            {
                if (m_aStates.top().getDestination() == Destination::LEVELNUMBERS)
                {
                    if (nParam != ';')
                        m_aStates.top().getLevelNumbers().push_back(sal_Int32(nParam));
                    else
                        // ';' in \u form is not considered valid.
                        m_aStates.top().setLevelNumbersValid(false);
                }
                else
                    m_aUnicodeBuffer.append(static_cast<sal_Unicode>(nParam));
                m_aStates.top().getCharsToSkip() = m_aStates.top().getUc();
            }
            break;
        case RTFKeyword::LEVELFOLLOW:
        {
            OUString sValue;
            switch (nParam)
            {
                case 0:
                    sValue = "tab";
                    break;
                case 1:
                    sValue = "space";
                    break;
                case 2:
                    sValue = "nothing";
                    break;
            }
            if (!sValue.isEmpty())
                m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_Lvl_suff, new RTFValue(sValue));
        }
        break;
        case RTFKeyword::FPRQ:
        {
            sal_Int32 nValue = 0;
            switch (nParam)
            {
                case 0:
                    nValue = NS_ooxml::LN_Value_ST_Pitch_default;
                    break;
                case 1:
                    nValue = NS_ooxml::LN_Value_ST_Pitch_fixed;
                    break;
                case 2:
                    nValue = NS_ooxml::LN_Value_ST_Pitch_variable;
                    break;
            }
            if (nValue)
            {
                RTFSprms aAttributes;
                aAttributes.set(NS_ooxml::LN_CT_Pitch_val, new RTFValue(nValue));
                m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_Font_pitch,
                                                    new RTFValue(aAttributes));
            }
        }
        break;
        case RTFKeyword::LISTOVERRIDECOUNT:
            // Ignore this for now, the exporter always emits it with a zero parameter.
            break;
        case RTFKeyword::PICSCALEX:
            m_aStates.top().getPicture().nScaleX = nParam;
            break;
        case RTFKeyword::PICSCALEY:
            m_aStates.top().getPicture().nScaleY = nParam;
            break;
        case RTFKeyword::PICW:
            m_aStates.top().getPicture().nWidth = nParam;
            break;
        case RTFKeyword::PICH:
            m_aStates.top().getPicture().nHeight = nParam;
            break;
        case RTFKeyword::PICWGOAL:
            m_aStates.top().getPicture().nGoalWidth = convertTwipToMm100(nParam);
            break;
        case RTFKeyword::PICHGOAL:
            m_aStates.top().getPicture().nGoalHeight = convertTwipToMm100(nParam);
            break;
        case RTFKeyword::PICCROPL:
            m_aStates.top().getPicture().nCropL = convertTwipToMm100(nParam);
            break;
        case RTFKeyword::PICCROPR:
            m_aStates.top().getPicture().nCropR = convertTwipToMm100(nParam);
            break;
        case RTFKeyword::PICCROPT:
            m_aStates.top().getPicture().nCropT = convertTwipToMm100(nParam);
            break;
        case RTFKeyword::PICCROPB:
            m_aStates.top().getPicture().nCropB = convertTwipToMm100(nParam);
            break;
        case RTFKeyword::SHPWRK:
        {
            int nValue = 0;
            switch (nParam)
            {
                case 0:
                    nValue = NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_bothSides;
                    break;
                case 1:
                    nValue = NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_left;
                    break;
                case 2:
                    nValue = NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_right;
                    break;
                case 3:
                    nValue = NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_largest;
                    break;
                default:
                    break;
            }
            auto pValue = new RTFValue(nValue);
            RTFValue::Pointer_t pTight
                = m_aStates.top().getCharacterSprms().find(NS_ooxml::LN_EG_WrapType_wrapTight);
            if (pTight)
                pTight->getAttributes().set(NS_ooxml::LN_CT_WrapTight_wrapText, pValue);
            else
                m_aStates.top().getCharacterAttributes().set(NS_ooxml::LN_CT_WrapSquare_wrapText,
                                                             pValue);
        }
        break;
        case RTFKeyword::SHPWR:
        {
            switch (nParam)
            {
                case 1:
                    m_aStates.top().getShape().setWrap(text::WrapTextMode_NONE);
                    break;
                case 2:
                    m_aStates.top().getShape().setWrap(text::WrapTextMode_PARALLEL);
                    break;
                case 3:
                    m_aStates.top().getShape().setWrap(text::WrapTextMode_THROUGH);
                    m_aStates.top().getCharacterSprms().set(NS_ooxml::LN_EG_WrapType_wrapNone,
                                                            new RTFValue());
                    break;
                case 4:
                    m_aStates.top().getShape().setWrap(text::WrapTextMode_PARALLEL);
                    m_aStates.top().getCharacterSprms().set(NS_ooxml::LN_EG_WrapType_wrapTight,
                                                            new RTFValue());
                    break;
                case 5:
                    m_aStates.top().getShape().setWrap(text::WrapTextMode_THROUGH);
                    break;
            }
        }
        break;
        case RTFKeyword::COLS:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_cols, NS_ooxml::LN_CT_Columns_num,
                               pIntValue);
            break;
        case RTFKeyword::COLSX:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_cols, NS_ooxml::LN_CT_Columns_space,
                               pIntValue);
            break;
        case RTFKeyword::COLNO:
            putNestedSprm(m_aStates.top().getSectionSprms(), NS_ooxml::LN_EG_SectPrContents_cols,
                          NS_ooxml::LN_CT_Columns_col, pIntValue);
            break;
        case RTFKeyword::COLW:
        case RTFKeyword::COLSR:
        {
            RTFSprms& rAttributes = getLastAttributes(m_aStates.top().getSectionSprms(),
                                                      NS_ooxml::LN_EG_SectPrContents_cols);
            rAttributes.set((nKeyword == RTFKeyword::COLW ? NS_ooxml::LN_CT_Column_w
                                                          : NS_ooxml::LN_CT_Column_space),
                            pIntValue);
        }
        break;
        case RTFKeyword::PAPERH:
            putNestedAttribute(m_aDefaultState.getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_h,
                               pIntValue);
            [[fallthrough]]; // set the default + current value
        case RTFKeyword::PGHSXN:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_h,
                               pIntValue);
            break;
        case RTFKeyword::PAPERW:
            putNestedAttribute(m_aDefaultState.getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_w,
                               pIntValue);
            [[fallthrough]]; // set the default + current value
        case RTFKeyword::PGWSXN:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_w,
                               pIntValue);
            break;
        case RTFKeyword::BINFSXN:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_paperSrc,
                               NS_ooxml::LN_CT_PaperSource_first, pIntValue);
            break;
        case RTFKeyword::BINSXN:
        {
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_paperSrc,
                               NS_ooxml::LN_CT_PaperSource_other, pIntValue);
        }
        break;
        case RTFKeyword::MARGL:
            putNestedAttribute(m_aDefaultState.getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_left,
                               pIntValue);
            [[fallthrough]]; // set the default + current value
        case RTFKeyword::MARGLSXN:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_left,
                               pIntValue);
            break;
        case RTFKeyword::MARGR:
            putNestedAttribute(m_aDefaultState.getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_right,
                               pIntValue);
            [[fallthrough]]; // set the default + current value
        case RTFKeyword::MARGRSXN:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_right,
                               pIntValue);
            break;
        case RTFKeyword::MARGT:
            putNestedAttribute(m_aDefaultState.getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_top,
                               pIntValue);
            [[fallthrough]]; // set the default + current value
        case RTFKeyword::MARGTSXN:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_top,
                               pIntValue);
            break;
        case RTFKeyword::MARGB:
            putNestedAttribute(m_aDefaultState.getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_bottom,
                               pIntValue);
            [[fallthrough]]; // set the default + current value
        case RTFKeyword::MARGBSXN:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_bottom,
                               pIntValue);
            break;
        case RTFKeyword::HEADERY:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_header,
                               pIntValue);
            break;
        case RTFKeyword::FOOTERY:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_footer,
                               pIntValue);
            break;
        case RTFKeyword::GUTTER:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_gutter,
                               pIntValue);
            break;
        case RTFKeyword::DEFTAB:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_defaultTabStop, pIntValue);
            break;
        case RTFKeyword::LINEMOD:
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_lnNumType,
                               NS_ooxml::LN_CT_LineNumber_countBy, pIntValue);
            break;
        case RTFKeyword::LINEX:
            if (nParam)
                putNestedAttribute(m_aStates.top().getSectionSprms(),
                                   NS_ooxml::LN_EG_SectPrContents_lnNumType,
                                   NS_ooxml::LN_CT_LineNumber_distance, pIntValue);
            break;
        case RTFKeyword::LINESTARTS:
        {
            // OOXML <w:lnNumType w:start="..."/> is 0-based, RTF is 1-based.
            auto pStart = tools::make_ref<RTFValue>(nParam - 1);
            putNestedAttribute(m_aStates.top().getSectionSprms(),
                               NS_ooxml::LN_EG_SectPrContents_lnNumType,
                               NS_ooxml::LN_CT_LineNumber_start, pStart);
        }
        break;
        case RTFKeyword::REVAUTH:
        case RTFKeyword::REVAUTHDEL:
        {
            auto pValue = new RTFValue(m_aAuthors[nParam]);
            putNestedAttribute(m_aStates.top().getCharacterSprms(), NS_ooxml::LN_trackchange,
                               NS_ooxml::LN_CT_TrackChange_author, pValue);
        }
        break;
        case RTFKeyword::REVDTTM:
        case RTFKeyword::REVDTTMDEL:
        {
            auto pValue = new RTFValue(DTTM22OUString(nParam));
            putNestedAttribute(m_aStates.top().getCharacterSprms(), NS_ooxml::LN_trackchange,
                               NS_ooxml::LN_CT_TrackChange_date, pValue);
        }
        break;
        case RTFKeyword::SHPLEFT:
            m_aStates.top().getShape().setLeft(convertTwipToMm100(nParam));
            break;
        case RTFKeyword::SHPTOP:
            m_aStates.top().getShape().setTop(convertTwipToMm100(nParam));
            break;
        case RTFKeyword::SHPRIGHT:
            m_aStates.top().getShape().setRight(convertTwipToMm100(nParam));
            break;
        case RTFKeyword::SHPBOTTOM:
            m_aStates.top().getShape().setBottom(convertTwipToMm100(nParam));
            break;
        case RTFKeyword::SHPZ:
            m_aStates.top().getShape().setZ(nParam);
            break;
        case RTFKeyword::FFTYPE:
            switch (nParam)
            {
                case 0:
                    m_nFormFieldType = RTFFormFieldType::TEXT;
                    break;
                case 1:
                    m_nFormFieldType = RTFFormFieldType::CHECKBOX;
                    break;
                case 2:
                    m_nFormFieldType = RTFFormFieldType::LIST;
                    break;
                default:
                    m_nFormFieldType = RTFFormFieldType::NONE;
                    break;
            }
            break;
        case RTFKeyword::FFDEFRES:
            if (m_nFormFieldType == RTFFormFieldType::CHECKBOX)
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFCheckBox_default, pIntValue);
            else if (m_nFormFieldType == RTFFormFieldType::LIST)
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFDDList_default, pIntValue);
            break;
        case RTFKeyword::FFRES:
            // 25 means undefined, see [MS-DOC] 2.9.79, FFDataBits.
            if (m_nFormFieldType == RTFFormFieldType::CHECKBOX && nParam != 25)
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFCheckBox_checked, pIntValue);
            else if (m_nFormFieldType == RTFFormFieldType::LIST)
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFDDList_result, pIntValue);
            break;
        case RTFKeyword::EDMINS:
            if (m_xDocumentProperties.is())
            {
                // tdf#116851 some RTF may be malformed
                if (nParam < 0)
                    nParam = -nParam;
                m_xDocumentProperties->setEditingDuration(nParam);
            }
            break;
        case RTFKeyword::NOFPAGES:
        case RTFKeyword::NOFWORDS:
        case RTFKeyword::NOFCHARS:
        case RTFKeyword::NOFCHARSWS:
            if (m_xDocumentProperties.is())
            {
                comphelper::SequenceAsHashMap aSeq = m_xDocumentProperties->getDocumentStatistics();
                OUString aName;
                switch (nKeyword)
                {
                    case RTFKeyword::NOFPAGES:
                        aName = "PageCount";
                        nParam = 99;
                        break;
                    case RTFKeyword::NOFWORDS:
                        aName = "WordCount";
                        break;
                    case RTFKeyword::NOFCHARS:
                        aName = "CharacterCount";
                        break;
                    case RTFKeyword::NOFCHARSWS:
                        aName = "NonWhitespaceCharacterCount";
                        break;
                    default:
                        break;
                }
                if (!aName.isEmpty())
                {
                    aSeq[aName] <<= sal_Int32(nParam);
                    m_xDocumentProperties->setDocumentStatistics(aSeq.getAsConstNamedValueList());
                }
            }
            break;
        case RTFKeyword::VERSION:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setEditingCycles(nParam);
            break;
        case RTFKeyword::VERN:
            // Ignore this for now, later the RTF writer version could be used to add hacks for older buggy writers.
            break;
        case RTFKeyword::FTNSTART:
            putNestedSprm(m_aDefaultState.getParagraphSprms(),
                          NS_ooxml::LN_EG_SectPrContents_footnotePr,
                          NS_ooxml::LN_EG_FtnEdnNumProps_numStart, pIntValue);
            break;
        case RTFKeyword::AFTNSTART:
            putNestedSprm(m_aDefaultState.getParagraphSprms(),
                          NS_ooxml::LN_EG_SectPrContents_endnotePr,
                          NS_ooxml::LN_EG_FtnEdnNumProps_numStart, pIntValue);
            break;
        case RTFKeyword::DFRMTXTX:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_hSpace, nParam);
            break;
        case RTFKeyword::DFRMTXTY:
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_vSpace, nParam);
            break;
        case RTFKeyword::DXFRTEXT:
        {
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_hSpace, nParam);
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_vSpace, nParam);
        }
        break;
        case RTFKeyword::FLYVERT:
        {
            RTFVertOrient aVertOrient(nParam);
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_yAlign,
                                               aVertOrient.GetAlign());
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_vAnchor,
                                               aVertOrient.GetAnchor());
        }
        break;
        case RTFKeyword::FLYHORZ:
        {
            RTFHoriOrient aHoriOrient(nParam);
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_xAlign,
                                               aHoriOrient.GetAlign());
            m_aStates.top().getFrame().setSprm(NS_ooxml::LN_CT_FramePr_hAnchor,
                                               aHoriOrient.GetAnchor());
        }
        break;
        case RTFKeyword::FLYANCHOR:
            break;
        case RTFKeyword::WMETAFILE:
            m_aStates.top().getPicture().eWMetafile = nParam;
            break;
        case RTFKeyword::SB:
            putNestedAttribute(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_spacing,
                               NS_ooxml::LN_CT_Spacing_before, pIntValue);
            break;
        case RTFKeyword::SA:
            putNestedAttribute(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_spacing,
                               NS_ooxml::LN_CT_Spacing_after, pIntValue);
            break;
        case RTFKeyword::DPX:
            m_aStates.top().getDrawingObject().setLeft(convertTwipToMm100(nParam));
            break;
        case RTFKeyword::DPY:
            m_aStates.top().getDrawingObject().setTop(convertTwipToMm100(nParam));
            break;
        case RTFKeyword::DPXSIZE:
            m_aStates.top().getDrawingObject().setRight(convertTwipToMm100(nParam));
            break;
        case RTFKeyword::DPYSIZE:
            m_aStates.top().getDrawingObject().setBottom(convertTwipToMm100(nParam));
            break;
        case RTFKeyword::PNSTART:
            m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_Lvl_start, pIntValue);
            break;
        case RTFKeyword::PNF:
        {
            auto pValue = new RTFValue(m_aFontNames[getFontIndex(nParam)]);
            RTFSprms aAttributes;
            aAttributes.set(NS_ooxml::LN_CT_Fonts_ascii, pValue);
            putNestedSprm(m_aStates.top().getTableSprms(), NS_ooxml::LN_CT_Lvl_rPr,
                          NS_ooxml::LN_EG_RPrBase_rFonts, new RTFValue(aAttributes));
        }
        break;
        case RTFKeyword::VIEWSCALE:
            m_aSettingsTableAttributes.set(NS_ooxml::LN_CT_Zoom_percent, pIntValue);
            break;
        case RTFKeyword::BIN:
        {
            m_aStates.top().setInternalState(RTFInternalState::BIN);
            m_aStates.top().setBinaryToRead(nParam);
        }
        break;
        case RTFKeyword::DPLINECOR:
            m_aStates.top().getDrawingObject().setLineColorR(nParam);
            m_aStates.top().getDrawingObject().setHasLineColor(true);
            break;
        case RTFKeyword::DPLINECOG:
            m_aStates.top().getDrawingObject().setLineColorG(nParam);
            m_aStates.top().getDrawingObject().setHasLineColor(true);
            break;
        case RTFKeyword::DPLINECOB:
            m_aStates.top().getDrawingObject().setLineColorB(nParam);
            m_aStates.top().getDrawingObject().setHasLineColor(true);
            break;
        case RTFKeyword::DPFILLBGCR:
            m_aStates.top().getDrawingObject().setFillColorR(nParam);
            m_aStates.top().getDrawingObject().setHasFillColor(true);
            break;
        case RTFKeyword::DPFILLBGCG:
            m_aStates.top().getDrawingObject().setFillColorG(nParam);
            m_aStates.top().getDrawingObject().setHasFillColor(true);
            break;
        case RTFKeyword::DPFILLBGCB:
            m_aStates.top().getDrawingObject().setFillColorB(nParam);
            m_aStates.top().getDrawingObject().setHasFillColor(true);
            break;
        case RTFKeyword::DODHGT:
            m_aStates.top().getDrawingObject().setDhgt(nParam);
            break;
        case RTFKeyword::DPPOLYCOUNT:
            if (nParam >= 0)
            {
                m_aStates.top().getDrawingObject().setPolyLineCount(nParam);
            }
            break;
        case RTFKeyword::DPPTX:
        {
            RTFDrawingObject& rDrawingObject = m_aStates.top().getDrawingObject();

            if (rDrawingObject.getPolyLinePoints().empty())
                dispatchValue(RTFKeyword::DPPOLYCOUNT, 2);

            rDrawingObject.getPolyLinePoints().emplace_back(convertTwipToMm100(nParam), 0);
        }
        break;
        case RTFKeyword::DPPTY:
        {
            RTFDrawingObject& rDrawingObject = m_aStates.top().getDrawingObject();
            if (!rDrawingObject.getPolyLinePoints().empty())
            {
                rDrawingObject.getPolyLinePoints().back().Y = convertTwipToMm100(nParam);
                rDrawingObject.setPolyLineCount(rDrawingObject.getPolyLineCount() - 1);
                if (rDrawingObject.getPolyLineCount() == 0 && rDrawingObject.getPropertySet().is())
                {
                    uno::Sequence<uno::Sequence<awt::Point>> aPointSequenceSequence
                        = { comphelper::containerToSequence(rDrawingObject.getPolyLinePoints()) };
                    rDrawingObject.getPropertySet()->setPropertyValue(
                        u"PolyPolygon"_ustr, uno::Any(aPointSequenceSequence));
                }
            }
        }
        break;
        case RTFKeyword::SHPFBLWTXT:
            // Shape is below text -> send it to the background.
            m_aStates.top().getShape().setInBackground(nParam != 0);
            break;
        case RTFKeyword::FI:
        {
            if (m_aStates.top().getDestination() == Destination::LISTLEVEL)
            {
                if (m_aStates.top().getLevelNumbersValid())
                    putNestedAttribute(m_aStates.top().getTableSprms(), NS_ooxml::LN_CT_PPrBase_ind,
                                       NS_ooxml::LN_CT_Ind_firstLine, pIntValue);
                else
                    m_aInvalidListLevelFirstIndents[m_nListLevel] = nParam;
            }
            else
                putNestedAttribute(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_ind,
                                   NS_ooxml::LN_CT_Ind_firstLine, pIntValue);
            break;
        }
        case RTFKeyword::LI:
        {
            if (m_aStates.top().getDestination() == Destination::LISTLEVEL)
            {
                if (m_aStates.top().getLevelNumbersValid())
                    putNestedAttribute(m_aStates.top().getTableSprms(), NS_ooxml::LN_CT_PPrBase_ind,
                                       NS_ooxml::LN_CT_Ind_left, pIntValue);
            }
            else
            {
                putNestedAttribute(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_ind,
                                   NS_ooxml::LN_CT_Ind_left, pIntValue);
            }
            // It turns out \li should reset the \fi inherited from the stylesheet.
            // So set the direct formatting to zero, if we don't have such direct formatting yet.
            putNestedAttribute(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_ind,
                               NS_ooxml::LN_CT_Ind_firstLine, new RTFValue(0),
                               RTFOverwrite::NO_IGNORE);
        }
        break;
        case RTFKeyword::RI:
            putNestedAttribute(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_ind,
                               NS_ooxml::LN_CT_Ind_right, pIntValue);
            break;
        case RTFKeyword::LIN:
            putNestedAttribute(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_ind,
                               NS_ooxml::LN_CT_Ind_start, pIntValue);
            break;
        case RTFKeyword::RIN:
            putNestedAttribute(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_ind,
                               NS_ooxml::LN_CT_Ind_end, pIntValue);
            break;
        case RTFKeyword::OUTLINELEVEL:
            m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_CT_PPrBase_outlineLvl, pIntValue);
            break;
        case RTFKeyword::PROPTYPE:
        {
            switch (nParam)
            {
                case 3:
                    m_aStates.top().setPropType(cppu::UnoType<sal_Int32>::get());
                    break;
                case 5:
                    m_aStates.top().setPropType(cppu::UnoType<double>::get());
                    break;
                case 11:
                    m_aStates.top().setPropType(cppu::UnoType<bool>::get());
                    break;
                case 30:
                    m_aStates.top().setPropType(cppu::UnoType<OUString>::get());
                    break;
                case 64:
                    m_aStates.top().setPropType(cppu::UnoType<util::DateTime>::get());
                    break;
            }
        }
        break;
        case RTFKeyword::DIBITMAP:
            m_aStates.top().getPicture().eStyle = RTFBmpStyle::DIBITMAP;
            break;
        case RTFKeyword::TRWWIDTHA:
            m_aStates.top().setTableRowWidthAfter(nParam);
            break;
        case RTFKeyword::ANIMTEXT:
        {
            Id nId = 0;
            switch (nParam)
            {
                case 0:
                    nId = NS_ooxml::LN_Value_ST_TextEffect_none;
                    break;
                case 2:
                    nId = NS_ooxml::LN_Value_ST_TextEffect_blinkBackground;
                    break;
            }

            if (nId > 0)
                m_aStates.top().getCharacterSprms().set(NS_ooxml::LN_EG_RPrBase_effect,
                                                        new RTFValue(nId));
            break;
        }
        case RTFKeyword::VIEWBKSP:
        {
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_displayBackgroundShape, pIntValue);
            // Send this token immediately, if it only appears before the first
            // run, it will be too late, we ignored the background shape already by then.
            outputSettingsTable();
            break;
        }
        case RTFKeyword::STEXTFLOW:
        {
            Id nId = 0;
            switch (nParam)
            {
                case 0:
                    nId = NS_ooxml::LN_Value_ST_TextDirection_lrTb;
                    break;
                case 1:
                    nId = NS_ooxml::LN_Value_ST_TextDirection_tbRl;
                    break;
            }

            if (nId > 0)
            {
                m_aStates.top().getSectionSprms().set(NS_ooxml::LN_EG_SectPrContents_textDirection,
                                                      new RTFValue(nId));
            }
        }
        break;
        case RTFKeyword::LBR:
        {
            Id nId = 0;
            switch (nParam)
            {
                case 1:
                    nId = NS_ooxml::LN_Value_ST_BrClear_left;
                    break;
                case 2:
                    nId = NS_ooxml::LN_Value_ST_BrClear_right;
                    break;
                case 3:
                    nId = NS_ooxml::LN_Value_ST_BrClear_all;
                    break;
            }

            if (nId > 0)
            {
                m_aStates.top().getCharacterAttributes().set(NS_ooxml::LN_CT_Br_clear,
                                                             new RTFValue(nId));
            }
        }
        break;
        case RTFKeyword::PGBRDROPT:
        {
            sal_Int16 nOffsetFrom = (nParam & 0xe0) >> 5;
            bool bFromEdge = nOffsetFrom == 1;
            if (bFromEdge)
            {
                Id nId = NS_ooxml::LN_Value_doc_ST_PageBorderOffset_page;
                putNestedAttribute(m_aStates.top().getSectionSprms(),
                                   NS_ooxml::LN_EG_SectPrContents_pgBorders,
                                   NS_ooxml::LN_CT_PageBorders_offsetFrom, new RTFValue(nId));
            }
        }
        break;
        case RTFKeyword::TPOSY:
        {
            putNestedAttribute(m_aStates.top().getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblpPr,
                               NS_ooxml::LN_CT_TblPPr_tblpY, new RTFValue(nParam));
        }
        break;
        case RTFKeyword::TPOSX:
        {
            putNestedAttribute(m_aStates.top().getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblpPr,
                               NS_ooxml::LN_CT_TblPPr_tblpX, new RTFValue(nParam));
        }
        break;
        case RTFKeyword::TDFRMTXTLEFT:
        {
            putNestedAttribute(m_aStates.top().getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblpPr,
                               NS_ooxml::LN_CT_TblPPr_leftFromText, new RTFValue(nParam));
        }
        break;
        case RTFKeyword::TDFRMTXTRIGHT:
        {
            putNestedAttribute(m_aStates.top().getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblpPr,
                               NS_ooxml::LN_CT_TblPPr_rightFromText, new RTFValue(nParam));
        }
        break;
        case RTFKeyword::TDFRMTXTTOP:
        {
            putNestedAttribute(m_aStates.top().getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblpPr,
                               NS_ooxml::LN_CT_TblPPr_topFromText, new RTFValue(nParam));
        }
        break;
        case RTFKeyword::TDFRMTXTBOTTOM:
        {
            putNestedAttribute(m_aStates.top().getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblpPr,
                               NS_ooxml::LN_CT_TblPPr_bottomFromText, new RTFValue(nParam));
        }
        break;
        default:
        {
            SAL_INFO("writerfilter", "TODO handle value '" << keywordToString(nKeyword) << "'");
            aSkip.setParsed(false);
        }
        break;
    }
    return RTFError::OK;
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
