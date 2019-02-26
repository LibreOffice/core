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
#include <tools/mapunit.hxx>

#include <ooxml/resourceids.hxx>

#include "rtfcharsets.hxx"
#include "rtffly.hxx"
#include "rtfreferenceproperties.hxx"
#include "rtfskipdestination.hxx"

#include <officecfg/Setup.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <unotools/wincodepage.hxx>

using namespace com::sun::star;

namespace
{
OUString getLODefaultLanguage()
{
    OUString result(::officecfg::Office::Linguistic::General::DefaultLocale::get());
    if (result.isEmpty())
        result = ::officecfg::Setup::L10N::ooSetupSystemLocale::get();
    return result;
}
}

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
RTFError RTFDocumentImpl::dispatchValue(RTFKeyword nKeyword, int nParam)
{
    setNeedSect(true);
    checkUnicode(/*bUnicode =*/nKeyword != RTF_U, /*bHex =*/true);
    RTFSkipDestination aSkip(*this);
    int nSprm = 0;
    tools::SvRef<RTFValue> pIntValue(new RTFValue(nParam));
    // Trivial table sprms.
    switch (nKeyword)
    {
        case RTF_LEVELJC:
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
        case RTF_LEVELNFC:
            nSprm = NS_ooxml::LN_CT_Lvl_numFmt;
            pIntValue = new RTFValue(getNumberFormat(nParam));
            break;
        case RTF_LEVELSTARTAT:
            nSprm = NS_ooxml::LN_CT_Lvl_start;
            break;
        case RTF_LEVELPICTURE:
            nSprm = NS_ooxml::LN_CT_Lvl_lvlPicBulletId;
            break;
        case RTF_SBASEDON:
            nSprm = NS_ooxml::LN_CT_Style_basedOn;
            pIntValue = new RTFValue(getStyleName(nParam));
            break;
        default:
            break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aTableSprms.set(nSprm, pIntValue);
        return RTFError::OK;
    }
    // Trivial character sprms.
    switch (nKeyword)
    {
        case RTF_FS:
        case RTF_AFS:
            nSprm = (m_aStates.top().isRightToLeft
                     || m_aStates.top().eRunType == RTFParserState::RunType::HICH)
                        ? NS_ooxml::LN_EG_RPrBase_szCs
                        : NS_ooxml::LN_EG_RPrBase_sz;
            break;
        case RTF_EXPNDTW:
            nSprm = NS_ooxml::LN_EG_RPrBase_spacing;
            break;
        case RTF_KERNING:
            nSprm = NS_ooxml::LN_EG_RPrBase_kern;
            break;
        case RTF_CHARSCALEX:
            nSprm = NS_ooxml::LN_EG_RPrBase_w;
            break;
        default:
            break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aCharacterSprms.set(nSprm, pIntValue);
        return RTFError::OK;
    }
    // Trivial character attributes.
    switch (nKeyword)
    {
        case RTF_LANG:
        case RTF_ALANG:
            if (m_aStates.top().isRightToLeft
                || m_aStates.top().eRunType == RTFParserState::RunType::HICH)
            {
                nSprm = NS_ooxml::LN_CT_Language_bidi;
            }
            else if (m_aStates.top().eRunType == RTFParserState::RunType::DBCH)
            {
                nSprm = NS_ooxml::LN_CT_Language_eastAsia;
            }
            else
            {
                assert(m_aStates.top().eRunType == RTFParserState::RunType::LOCH);
                nSprm = NS_ooxml::LN_CT_Language_val;
            }
            break;
        case RTF_LANGFE: // this one is always CJK apparently
            nSprm = NS_ooxml::LN_CT_Language_eastAsia;
            break;
        default:
            break;
    }
    if (nSprm > 0)
    {
        LanguageTag aTag((LanguageType(static_cast<sal_uInt16>(nParam))));
        auto pValue = new RTFValue(aTag.getBcp47());
        putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_EG_RPrBase_lang, nSprm,
                           pValue);
        // Language is a character property, but we should store it at a paragraph level as well for fields.
        if (nKeyword == RTF_LANG && m_bNeedPap)
            putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_EG_RPrBase_lang, nSprm,
                               pValue);
        return RTFError::OK;
    }
    // Trivial paragraph sprms.
    switch (nKeyword)
    {
        case RTF_ITAP:
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
        m_aStates.top().aParagraphSprms.set(nSprm, pIntValue);
        if (nKeyword == RTF_ITAP && nParam > 0)
        {
            while (m_aTableBufferStack.size() < sal::static_int_cast<std::size_t>(nParam))
            {
                m_aTableBufferStack.emplace_back(RTFBuffer_t());
            }
            // Invalid tables may omit INTBL after ITAP
            dispatchFlag(RTF_INTBL); // sets newly pushed buffer as current
            assert(m_aStates.top().pCurrentBuffer == &m_aTableBufferStack.back());
        }
        return RTFError::OK;
    }

    // Info group.
    switch (nKeyword)
    {
        case RTF_YR:
        {
            m_aStates.top().nYear = nParam;
            nSprm = 1;
        }
        break;
        case RTF_MO:
        {
            m_aStates.top().nMonth = nParam;
            nSprm = 1;
        }
        break;
        case RTF_DY:
        {
            m_aStates.top().nDay = nParam;
            nSprm = 1;
        }
        break;
        case RTF_HR:
        {
            m_aStates.top().nHour = nParam;
            nSprm = 1;
        }
        break;
        case RTF_MIN:
        {
            m_aStates.top().nMinute = nParam;
            nSprm = 1;
        }
        break;
        default:
            break;
    }
    if (nSprm > 0)
        return RTFError::OK;

    // Frame size / position.
    Id nId = 0;
    switch (nKeyword)
    {
        case RTF_ABSW:
            nId = NS_ooxml::LN_CT_FramePr_w;
            break;
        case RTF_ABSH:
            nId = NS_ooxml::LN_CT_FramePr_h;
            break;
        case RTF_POSX:
        {
            nId = NS_ooxml::LN_CT_FramePr_x;
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, 0);
        }
        break;
        case RTF_POSY:
        {
            nId = NS_ooxml::LN_CT_FramePr_y;
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, 0);
        }
        break;
        default:
            break;
    }

    if (nId > 0)
    {
        m_bNeedPap = true;
        // Don't try to support text frames inside tables for now.
        if (m_aStates.top().pCurrentBuffer != &m_aTableBufferStack.back())
            m_aStates.top().aFrame.setSprm(nId, nParam);

        return RTFError::OK;
    }

    // Then check for the more complex ones.
    switch (nKeyword)
    {
        case RTF_F:
        case RTF_AF:
            if (m_aStates.top().isRightToLeft
                || m_aStates.top().eRunType == RTFParserState::RunType::HICH)
            {
                nSprm = NS_ooxml::LN_CT_Fonts_cs;
            }
            else if (m_aStates.top().eRunType == RTFParserState::RunType::DBCH)
            {
                nSprm = NS_ooxml::LN_CT_Fonts_eastAsia;
            }
            else
            {
                assert(m_aStates.top().eRunType == RTFParserState::RunType::LOCH);
                nSprm = NS_ooxml::LN_CT_Fonts_ascii;
            }
            if (m_aStates.top().eDestination == Destination::FONTTABLE
                || m_aStates.top().eDestination == Destination::FONTENTRY)
            {
                m_aFontIndexes.push_back(nParam);
                m_nCurrentFontIndex = getFontIndex(nParam);
            }
            else if (m_aStates.top().eDestination == Destination::LISTLEVEL)
            {
                RTFSprms aFontAttributes;
                aFontAttributes.set(nSprm, new RTFValue(m_aFontNames[getFontIndex(nParam)]));
                RTFSprms aRunPropsSprms;
                aRunPropsSprms.set(NS_ooxml::LN_EG_RPrBase_rFonts, new RTFValue(aFontAttributes));
                m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_rPr,
                                                new RTFValue(RTFSprms(), aRunPropsSprms),
                                                RTFOverwrite::NO_APPEND);
            }
            else
            {
                m_nCurrentFontIndex = getFontIndex(nParam);
                auto pValue = new RTFValue(getFontName(m_nCurrentFontIndex));
                putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_EG_RPrBase_rFonts,
                                   nSprm, pValue);
                if (nKeyword == RTF_F)
                    m_aStates.top().nCurrentEncoding = getEncoding(m_nCurrentFontIndex);
            }
            break;
        case RTF_RED:
            m_aStates.top().aCurrentColor.SetRed(nParam);
            break;
        case RTF_GREEN:
            m_aStates.top().aCurrentColor.SetGreen(nParam);
            break;
        case RTF_BLUE:
            m_aStates.top().aCurrentColor.SetBlue(nParam);
            break;
        case RTF_FCHARSET:
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
            m_aStates.top().nCurrentEncoding = m_nCurrentEncoding;
        }
        break;
        case RTF_ANSICPG:
        case RTF_CPG:
        {
            rtl_TextEncoding nEncoding
                = (nParam == 0)
                      ? utl_getWinTextEncodingFromLangStr(getLODefaultLanguage().toUtf8().getStr())
                      : rtl_getTextEncodingFromWindowsCodePage(nParam);
            if (nKeyword == RTF_ANSICPG)
                m_aDefaultState.nCurrentEncoding = nEncoding;
            else
                m_nCurrentEncoding = nEncoding;
            m_aStates.top().nCurrentEncoding = nEncoding;
        }
        break;
        case RTF_CF:
        {
            RTFSprms aAttributes;
            auto pValue = new RTFValue(sal_uInt32(getColorTable(nParam)));
            aAttributes.set(NS_ooxml::LN_CT_Color_val, pValue);
            m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_color,
                                                new RTFValue(aAttributes));
        }
        break;
        case RTF_S:
        {
            m_aStates.top().nCurrentStyleIndex = nParam;

            if (m_aStates.top().eDestination == Destination::STYLESHEET
                || m_aStates.top().eDestination == Destination::STYLEENTRY)
            {
                m_nCurrentStyleIndex = nParam;
                auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_StyleType_paragraph);
                m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_Style_type,
                                                     pValue); // paragraph style
            }
            else
            {
                OUString aName = getStyleName(nParam);
                if (!aName.isEmpty())
                {
                    if (m_aStates.top().eDestination == Destination::LISTLEVEL)
                        m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_pStyle,
                                                        new RTFValue(aName));
                    else
                        m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_pStyle,
                                                            new RTFValue(aName));
                }
            }
        }
        break;
        case RTF_CS:
            m_aStates.top().nCurrentCharacterStyleIndex = nParam;
            if (m_aStates.top().eDestination == Destination::STYLESHEET
                || m_aStates.top().eDestination == Destination::STYLEENTRY)
            {
                m_nCurrentStyleIndex = nParam;
                auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_StyleType_character);
                m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_Style_type,
                                                     pValue); // character style
            }
            else
            {
                OUString aName = getStyleName(nParam);
                if (!aName.isEmpty())
                    m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_rStyle,
                                                        new RTFValue(aName));
            }
            break;
        case RTF_DS:
            if (m_aStates.top().eDestination == Destination::STYLESHEET
                || m_aStates.top().eDestination == Destination::STYLEENTRY)
            {
                m_nCurrentStyleIndex = nParam;
                auto pValue = new RTFValue(0); // TODO no value in enum StyleType?
                m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_Style_type,
                                                     pValue); // section style
            }
            break;
        case RTF_TS:
            if (m_aStates.top().eDestination == Destination::STYLESHEET
                || m_aStates.top().eDestination == Destination::STYLEENTRY)
            {
                m_nCurrentStyleIndex = nParam;
                // FIXME the correct value would be NS_ooxml::LN_Value_ST_StyleType_table but maybe table styles mess things up in dmapper, be cautious and disable them for now
                auto pValue = new RTFValue(0);
                m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_Style_type,
                                                     pValue); // table style
            }
            break;
        case RTF_DEFF:
            m_nDefaultFontIndex = nParam;
            break;
        case RTF_DEFLANG:
        case RTF_ADEFLANG:
        {
            LanguageTag aTag((LanguageType(static_cast<sal_uInt16>(nParam))));
            auto pValue = new RTFValue(aTag.getBcp47());
            putNestedAttribute(m_aStates.top().aCharacterSprms,
                               (nKeyword == RTF_DEFLANG ? NS_ooxml::LN_EG_RPrBase_lang
                                                        : NS_ooxml::LN_CT_Language_bidi),
                               nSprm, pValue);
        }
        break;
        case RTF_CHCBPAT:
        {
            auto pValue = new RTFValue(sal_uInt32(nParam ? getColorTable(nParam) : COL_AUTO));
            putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_EG_RPrBase_shd,
                               NS_ooxml::LN_CT_Shd_fill, pValue);
        }
        break;
        case RTF_CLCBPAT:
        case RTF_CLCBPATRAW:
        {
            auto pValue = new RTFValue(sal_uInt32(getColorTable(nParam)));
            putNestedAttribute(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_shd,
                               NS_ooxml::LN_CT_Shd_fill, pValue);
        }
        break;
        case RTF_CBPAT:
            if (nParam)
            {
                auto pValue = new RTFValue(sal_uInt32(getColorTable(nParam)));
                putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PrBase_shd,
                                   NS_ooxml::LN_CT_Shd_fill, pValue);
            }
            break;
        case RTF_ULC:
        {
            auto pValue = new RTFValue(sal_uInt32(getColorTable(nParam)));
            m_aStates.top().aCharacterSprms.set(0x6877, pValue);
        }
        break;
        case RTF_HIGHLIGHT:
        {
            auto pValue = new RTFValue(sal_uInt32(nParam ? getColorTable(nParam) : COL_AUTO));
            m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_highlight, pValue);
        }
        break;
        case RTF_UP:
        case RTF_DN:
        {
            auto pValue = new RTFValue(nParam * (nKeyword == RTF_UP ? 1 : -1));
            m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_position, pValue);
        }
        break;
        case RTF_HORZVERT:
        {
            auto pValue = new RTFValue(int(true));
            m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_EastAsianLayout_vert, pValue);
            if (nParam)
                // rotate fits to a single line
                m_aStates.top().aCharacterAttributes.set(
                    NS_ooxml::LN_CT_EastAsianLayout_vertCompress, pValue);
        }
        break;
        case RTF_EXPND:
        {
            // Convert quarter-points to twentieths of a point
            auto pValue = new RTFValue(nParam * 5);
            m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_spacing, pValue);
        }
        break;
        case RTF_TWOINONE:
        {
            auto pValue = new RTFValue(int(true));
            m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_EastAsianLayout_combine,
                                                     pValue);
            nId = 0;
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
                m_aStates.top().aCharacterAttributes.set(
                    NS_ooxml::LN_CT_EastAsianLayout_combineBrackets, new RTFValue(nId));
        }
        break;
        case RTF_SL:
        {
            // This is similar to RTF_ABSH, negative value means 'exact', positive means 'at least'.
            tools::SvRef<RTFValue> pValue(
                new RTFValue(NS_ooxml::LN_Value_doc_ST_LineSpacingRule_atLeast));
            if (nParam < 0)
            {
                pValue = new RTFValue(NS_ooxml::LN_Value_doc_ST_LineSpacingRule_exact);
                pIntValue = new RTFValue(-nParam);
            }
            m_aStates.top().aParagraphAttributes.set(NS_ooxml::LN_CT_Spacing_lineRule, pValue);
            m_aStates.top().aParagraphAttributes.set(NS_ooxml::LN_CT_Spacing_line, pIntValue);
        }
        break;
        case RTF_SLMULT:
            if (nParam > 0)
            {
                auto pValue = new RTFValue(NS_ooxml::LN_Value_doc_ST_LineSpacingRule_auto);
                m_aStates.top().aParagraphAttributes.set(NS_ooxml::LN_CT_Spacing_lineRule, pValue);
            }
            break;
        case RTF_BRDRW:
        {
            // dmapper expects it in 1/8 pt, we have it in twip - but avoid rounding 1 to 0
            if (nParam > 1)
                nParam = nParam * 2 / 5;
            auto pValue = new RTFValue(nParam);
            putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_sz, pValue);
        }
        break;
        case RTF_BRDRCF:
        {
            auto pValue = new RTFValue(sal_uInt32(getColorTable(nParam)));
            putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_color, pValue);
        }
        break;
        case RTF_BRSP:
        {
            // dmapper expects it in points, we have it in twip
            auto pValue = new RTFValue(nParam / 20);
            putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_space, pValue);
        }
        break;
        case RTF_TX:
        {
            m_aStates.top().aTabAttributes.set(NS_ooxml::LN_CT_TabStop_pos, pIntValue);
            auto pValue = new RTFValue(m_aStates.top().aTabAttributes);
            if (m_aStates.top().eDestination == Destination::LISTLEVEL)
                putNestedSprm(m_aStates.top().aTableSprms, NS_ooxml::LN_CT_PPrBase_tabs,
                              NS_ooxml::LN_CT_Tabs_tab, pValue);
            else
                putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_tabs,
                              NS_ooxml::LN_CT_Tabs_tab, pValue);
            m_aStates.top().aTabAttributes.clear();
        }
        break;
        case RTF_ILVL:
            putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_numPr,
                          NS_ooxml::LN_CT_NumPr_ilvl, pIntValue);
            break;
        case RTF_LISTTEMPLATEID:
            // This one is not referenced anywhere, so it's pointless to store it at the moment.
            break;
        case RTF_LISTID:
        {
            if (m_aStates.top().eDestination == Destination::LISTENTRY)
                m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_AbstractNum_abstractNumId,
                                                     pIntValue);
            else if (m_aStates.top().eDestination == Destination::LISTOVERRIDEENTRY)
                m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Num_abstractNumId, pIntValue);
            m_aStates.top().nCurrentListIndex = nParam;
        }
        break;
        case RTF_LS:
        {
            if (m_aStates.top().eDestination == Destination::LISTOVERRIDEENTRY)
            {
                m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_AbstractNum_nsid, pIntValue);
                m_aStates.top().nCurrentListOverrideIndex = nParam;
            }
            else
            {
                // Insert at the start, so properties inherited from the list
                // can be overridden by direct formatting. But still allow the
                // case when old-style paragraph numbering is already
                // tokenized.
                putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_numPr,
                              NS_ooxml::LN_CT_NumPr_numId, pIntValue, RTFOverwrite::YES_PREPEND);
            }
        }
        break;
        case RTF_UC:
            if ((SAL_MIN_INT16 <= nParam) && (nParam <= SAL_MAX_INT16))
                m_aStates.top().nUc = nParam;
            break;
        case RTF_U:
            // sal_Unicode is unsigned 16-bit, RTF may represent that as a
            // signed SAL_MIN_INT16..SAL_MAX_INT16 or 0..SAL_MAX_UINT16. The
            // static_cast() will do the right thing.
            if ((SAL_MIN_INT16 <= nParam) && (nParam <= SAL_MAX_UINT16))
            {
                if (m_aStates.top().eDestination == Destination::LEVELNUMBERS)
                {
                    if (nParam != ';')
                        m_aStates.top().aLevelNumbers.push_back(sal_Int32(nParam));
                    else
                        // ';' in \u form is not considered valid.
                        m_aStates.top().bLevelNumbersValid = false;
                }
                else
                    m_aUnicodeBuffer.append(static_cast<sal_Unicode>(nParam));
                m_aStates.top().nCharsToSkip = m_aStates.top().nUc;
            }
            break;
        case RTF_LEVELFOLLOW:
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
                m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_suff, new RTFValue(sValue));
        }
        break;
        case RTF_FPRQ:
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
                m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Font_pitch,
                                                new RTFValue(aAttributes));
            }
        }
        break;
        case RTF_LISTOVERRIDECOUNT:
            // Ignore this for now, the exporter always emits it with a zero parameter.
            break;
        case RTF_PICSCALEX:
            m_aStates.top().aPicture.nScaleX = nParam;
            break;
        case RTF_PICSCALEY:
            m_aStates.top().aPicture.nScaleY = nParam;
            break;
        case RTF_PICW:
            m_aStates.top().aPicture.nWidth = nParam;
            break;
        case RTF_PICH:
            m_aStates.top().aPicture.nHeight = nParam;
            break;
        case RTF_PICWGOAL:
            m_aStates.top().aPicture.nGoalWidth = convertTwipToMm100(nParam);
            break;
        case RTF_PICHGOAL:
            m_aStates.top().aPicture.nGoalHeight = convertTwipToMm100(nParam);
            break;
        case RTF_PICCROPL:
            m_aStates.top().aPicture.nCropL = convertTwipToMm100(nParam);
            break;
        case RTF_PICCROPR:
            m_aStates.top().aPicture.nCropR = convertTwipToMm100(nParam);
            break;
        case RTF_PICCROPT:
            m_aStates.top().aPicture.nCropT = convertTwipToMm100(nParam);
            break;
        case RTF_PICCROPB:
            m_aStates.top().aPicture.nCropB = convertTwipToMm100(nParam);
            break;
        case RTF_SHPWRK:
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
                = m_aStates.top().aCharacterSprms.find(NS_ooxml::LN_EG_WrapType_wrapTight);
            if (pTight)
                pTight->getAttributes().set(NS_ooxml::LN_CT_WrapTight_wrapText, pValue);
            else
                m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_WrapSquare_wrapText,
                                                         pValue);
        }
        break;
        case RTF_SHPWR:
        {
            switch (nParam)
            {
                case 1:
                    m_aStates.top().aShape.setWrap(text::WrapTextMode_NONE);
                    break;
                case 2:
                    m_aStates.top().aShape.setWrap(text::WrapTextMode_PARALLEL);
                    break;
                case 3:
                    m_aStates.top().aShape.setWrap(text::WrapTextMode_THROUGH);
                    m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_WrapType_wrapNone,
                                                        new RTFValue());
                    break;
                case 4:
                    m_aStates.top().aShape.setWrap(text::WrapTextMode_PARALLEL);
                    m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_WrapType_wrapTight,
                                                        new RTFValue());
                    break;
                case 5:
                    m_aStates.top().aShape.setWrap(text::WrapTextMode_THROUGH);
                    break;
            }
        }
        break;
        case RTF_CELLX:
        {
            int& rCurrentCellX((Destination::NESTEDTABLEPROPERTIES == m_aStates.top().eDestination)
                                   ? m_nNestedCurrentCellX
                                   : m_nTopLevelCurrentCellX);
            int nCellX = nParam - rCurrentCellX;
            const int COL_DFLT_WIDTH
                = 41; // sw/source/filter/inc/wrtswtbl.hxx, minimal possible width of cells.
            if (!nCellX)
                nCellX = COL_DFLT_WIDTH;

            // If there is a negative left margin, then the first cellx is relative to that.
            RTFValue::Pointer_t pTblInd
                = m_aStates.top().aTableRowSprms.find(NS_ooxml::LN_CT_TblPrBase_tblInd);
            if (rCurrentCellX == 0 && pTblInd.get())
            {
                RTFValue::Pointer_t pWidth
                    = pTblInd->getAttributes().find(NS_ooxml::LN_CT_TblWidth_w);
                if (pWidth.get() && pWidth->getInt() < 0)
                    nCellX = -1 * (pWidth->getInt() - nParam);
            }

            rCurrentCellX = nParam;
            auto pXValue = new RTFValue(nCellX);
            m_aStates.top().aTableRowSprms.set(NS_ooxml::LN_CT_TblGridBase_gridCol, pXValue,
                                               RTFOverwrite::NO_APPEND);
            if (Destination::NESTEDTABLEPROPERTIES == m_aStates.top().eDestination)
            {
                m_nNestedCells++;
                // Push cell properties.
                m_aNestedTableCellsSprms.push_back(m_aStates.top().aTableCellSprms);
                m_aNestedTableCellsAttributes.push_back(m_aStates.top().aTableCellAttributes);
            }
            else
            {
                m_nTopLevelCells++;
                // Push cell properties.
                m_aTopLevelTableCellsSprms.push_back(m_aStates.top().aTableCellSprms);
                m_aTopLevelTableCellsAttributes.push_back(m_aStates.top().aTableCellAttributes);
            }

            m_aStates.top().aTableCellSprms = m_aDefaultState.aTableCellSprms;
            m_aStates.top().aTableCellAttributes = m_aDefaultState.aTableCellAttributes;
            // We assume text after a row definition always belongs to the table, to handle text before the real INTBL token
            dispatchFlag(RTF_INTBL);
            if (!m_nCellxMax)
            {
                // Wasn't in table, but now is -> tblStart.
                RTFSprms aAttributes;
                RTFSprms aSprms;
                aSprms.set(NS_ooxml::LN_tblStart, new RTFValue(1));
                writerfilter::Reference<Properties>::Pointer_t pProperties
                    = new RTFReferenceProperties(aAttributes, aSprms);
                Mapper().props(pProperties);
            }
            m_nCellxMax = std::max(m_nCellxMax, nParam);
        }
        break;
        case RTF_TRRH:
        {
            OUString hRule("auto");
            if (nParam < 0)
            {
                tools::SvRef<RTFValue> pAbsValue(new RTFValue(-nParam));
                std::swap(pIntValue, pAbsValue);

                hRule = "exact";
            }
            else if (nParam > 0)
                hRule = "atLeast";

            putNestedAttribute(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TrPrBase_trHeight,
                               NS_ooxml::LN_CT_Height_val, pIntValue);

            auto pHRule = new RTFValue(hRule);
            putNestedAttribute(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TrPrBase_trHeight,
                               NS_ooxml::LN_CT_Height_hRule, pHRule);
        }
        break;
        case RTF_TRLEFT:
        {
            // the value is in twips
            putNestedAttribute(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblInd,
                               NS_ooxml::LN_CT_TblWidth_type,
                               new RTFValue(NS_ooxml::LN_Value_ST_TblWidth_dxa));
            putNestedAttribute(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblInd,
                               NS_ooxml::LN_CT_TblWidth_w, new RTFValue(nParam));
            auto const aDestination = m_aStates.top().eDestination;
            int& rCurrentTRLeft((Destination::NESTEDTABLEPROPERTIES == aDestination)
                                    ? m_nNestedTRLeft
                                    : m_nTopLevelTRLeft);
            int& rCurrentCellX((Destination::NESTEDTABLEPROPERTIES == aDestination)
                                   ? m_nNestedCurrentCellX
                                   : m_nTopLevelCurrentCellX);
            rCurrentTRLeft = rCurrentCellX = nParam;
        }
        break;
        case RTF_COLS:
            putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_cols,
                               NS_ooxml::LN_CT_Columns_num, pIntValue);
            break;
        case RTF_COLSX:
            putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_cols,
                               NS_ooxml::LN_CT_Columns_space, pIntValue);
            break;
        case RTF_COLNO:
            putNestedSprm(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_cols,
                          NS_ooxml::LN_CT_Columns_col, pIntValue);
            break;
        case RTF_COLW:
        case RTF_COLSR:
        {
            RTFSprms& rAttributes = getLastAttributes(m_aStates.top().aSectionSprms,
                                                      NS_ooxml::LN_EG_SectPrContents_cols);
            rAttributes.set(
                (nKeyword == RTF_COLW ? NS_ooxml::LN_CT_Column_w : NS_ooxml::LN_CT_Column_space),
                pIntValue);
        }
        break;
        case RTF_PAPERH:
            putNestedAttribute(m_aDefaultState.aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgSz,
                               NS_ooxml::LN_CT_PageSz_h, pIntValue);
            [[fallthrough]]; // set the default + current value
        case RTF_PGHSXN:
            putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgSz,
                               NS_ooxml::LN_CT_PageSz_h, pIntValue);
            break;
        case RTF_PAPERW:
            putNestedAttribute(m_aDefaultState.aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgSz,
                               NS_ooxml::LN_CT_PageSz_w, pIntValue);
            [[fallthrough]]; // set the default + current value
        case RTF_PGWSXN:
            putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgSz,
                               NS_ooxml::LN_CT_PageSz_w, pIntValue);
            break;
        case RTF_MARGL:
            putNestedAttribute(m_aDefaultState.aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar,
                               NS_ooxml::LN_CT_PageMar_left, pIntValue);
            [[fallthrough]]; // set the default + current value
        case RTF_MARGLSXN:
            putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar,
                               NS_ooxml::LN_CT_PageMar_left, pIntValue);
            break;
        case RTF_MARGR:
            putNestedAttribute(m_aDefaultState.aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar,
                               NS_ooxml::LN_CT_PageMar_right, pIntValue);
            [[fallthrough]]; // set the default + current value
        case RTF_MARGRSXN:
            putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar,
                               NS_ooxml::LN_CT_PageMar_right, pIntValue);
            break;
        case RTF_MARGT:
            putNestedAttribute(m_aDefaultState.aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar,
                               NS_ooxml::LN_CT_PageMar_top, pIntValue);
            [[fallthrough]]; // set the default + current value
        case RTF_MARGTSXN:
            putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar,
                               NS_ooxml::LN_CT_PageMar_top, pIntValue);
            break;
        case RTF_MARGB:
            putNestedAttribute(m_aDefaultState.aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar,
                               NS_ooxml::LN_CT_PageMar_bottom, pIntValue);
            [[fallthrough]]; // set the default + current value
        case RTF_MARGBSXN:
            putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar,
                               NS_ooxml::LN_CT_PageMar_bottom, pIntValue);
            break;
        case RTF_HEADERY:
            putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar,
                               NS_ooxml::LN_CT_PageMar_header, pIntValue);
            break;
        case RTF_FOOTERY:
            putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar,
                               NS_ooxml::LN_CT_PageMar_footer, pIntValue);
            break;
        case RTF_DEFTAB:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_defaultTabStop, pIntValue);
            break;
        case RTF_LINEMOD:
            putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_lnNumType,
                               NS_ooxml::LN_CT_LineNumber_countBy, pIntValue);
            break;
        case RTF_LINEX:
            if (nParam)
                putNestedAttribute(m_aStates.top().aSectionSprms,
                                   NS_ooxml::LN_EG_SectPrContents_lnNumType,
                                   NS_ooxml::LN_CT_LineNumber_distance, pIntValue);
            break;
        case RTF_LINESTARTS:
        {
            // OOXML <w:lnNumType w:start="..."/> is 0-based, RTF is 1-based.
            auto pStart = tools::make_ref<RTFValue>(nParam - 1);
            putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_lnNumType,
                               NS_ooxml::LN_CT_LineNumber_start, pStart);
        }
        break;
        case RTF_REVAUTH:
        case RTF_REVAUTHDEL:
        {
            auto pValue = new RTFValue(m_aAuthors[nParam]);
            putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_trackchange,
                               NS_ooxml::LN_CT_TrackChange_author, pValue);
        }
        break;
        case RTF_REVDTTM:
        case RTF_REVDTTMDEL:
        {
            OUString aStr(
                OStringToOUString(DTTM22OString(nParam), m_aStates.top().nCurrentEncoding));
            auto pValue = new RTFValue(aStr);
            putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_trackchange,
                               NS_ooxml::LN_CT_TrackChange_date, pValue);
        }
        break;
        case RTF_SHPLEFT:
            m_aStates.top().aShape.setLeft(convertTwipToMm100(nParam));
            break;
        case RTF_SHPTOP:
            m_aStates.top().aShape.setTop(convertTwipToMm100(nParam));
            break;
        case RTF_SHPRIGHT:
            m_aStates.top().aShape.setRight(convertTwipToMm100(nParam));
            break;
        case RTF_SHPBOTTOM:
            m_aStates.top().aShape.setBottom(convertTwipToMm100(nParam));
            break;
        case RTF_SHPZ:
            m_aStates.top().aShape.setZ(nParam);
            break;
        case RTF_FFTYPE:
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
        case RTF_FFDEFRES:
            if (m_nFormFieldType == RTFFormFieldType::CHECKBOX)
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFCheckBox_default, pIntValue);
            else if (m_nFormFieldType == RTFFormFieldType::LIST)
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFDDList_default, pIntValue);
            break;
        case RTF_FFRES:
            // 25 means undefined, see [MS-DOC] 2.9.79, FFDataBits.
            if (m_nFormFieldType == RTFFormFieldType::CHECKBOX && nParam != 25)
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFCheckBox_checked, pIntValue);
            else if (m_nFormFieldType == RTFFormFieldType::LIST)
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFDDList_result, pIntValue);
            break;
        case RTF_EDMINS:
            if (m_xDocumentProperties.is())
            {
                // tdf#116851 some RTF may be malformed
                if (nParam < 0)
                    nParam = -nParam;
                m_xDocumentProperties->setEditingDuration(nParam);
            }
            break;
        case RTF_NOFPAGES:
        case RTF_NOFWORDS:
        case RTF_NOFCHARS:
        case RTF_NOFCHARSWS:
            if (m_xDocumentProperties.is())
            {
                comphelper::SequenceAsHashMap aSeq = m_xDocumentProperties->getDocumentStatistics();
                OUString aName;
                switch (nKeyword)
                {
                    case RTF_NOFPAGES:
                        aName = "PageCount";
                        nParam = 99;
                        break;
                    case RTF_NOFWORDS:
                        aName = "WordCount";
                        break;
                    case RTF_NOFCHARS:
                        aName = "CharacterCount";
                        break;
                    case RTF_NOFCHARSWS:
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
        case RTF_VERSION:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setEditingCycles(nParam);
            break;
        case RTF_VERN:
            // Ignore this for now, later the RTF writer version could be used to add hacks for older buggy writers.
            break;
        case RTF_FTNSTART:
            putNestedSprm(m_aDefaultState.aParagraphSprms,
                          NS_ooxml::LN_EG_SectPrContents_footnotePr,
                          NS_ooxml::LN_EG_FtnEdnNumProps_numStart, pIntValue);
            break;
        case RTF_AFTNSTART:
            putNestedSprm(m_aDefaultState.aParagraphSprms, NS_ooxml::LN_EG_SectPrContents_endnotePr,
                          NS_ooxml::LN_EG_FtnEdnNumProps_numStart, pIntValue);
            break;
        case RTF_DFRMTXTX:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hSpace, nParam);
            break;
        case RTF_DFRMTXTY:
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vSpace, nParam);
            break;
        case RTF_DXFRTEXT:
        {
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hSpace, nParam);
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vSpace, nParam);
        }
        break;
        case RTF_FLYVERT:
        {
            RTFVertOrient aVertOrient(nParam);
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, aVertOrient.GetAlign());
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vAnchor,
                                           aVertOrient.GetAnchor());
        }
        break;
        case RTF_FLYHORZ:
        {
            RTFHoriOrient aHoriOrient(nParam);
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, aHoriOrient.GetAlign());
            m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hAnchor,
                                           aHoriOrient.GetAnchor());
        }
        break;
        case RTF_FLYANCHOR:
            break;
        case RTF_WMETAFILE:
            m_aStates.top().aPicture.eWMetafile = nParam;
            break;
        case RTF_SB:
            putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_spacing,
                               NS_ooxml::LN_CT_Spacing_before, pIntValue);
            break;
        case RTF_SA:
            putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_spacing,
                               NS_ooxml::LN_CT_Spacing_after, pIntValue);
            break;
        case RTF_DPX:
            m_aStates.top().aDrawingObject.setLeft(convertTwipToMm100(nParam));
            break;
        case RTF_DPY:
            m_aStates.top().aDrawingObject.setTop(convertTwipToMm100(nParam));
            break;
        case RTF_DPXSIZE:
            m_aStates.top().aDrawingObject.setRight(convertTwipToMm100(nParam));
            break;
        case RTF_DPYSIZE:
            m_aStates.top().aDrawingObject.setBottom(convertTwipToMm100(nParam));
            break;
        case RTF_PNSTART:
            m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_start, pIntValue);
            break;
        case RTF_PNF:
        {
            auto pValue = new RTFValue(m_aFontNames[getFontIndex(nParam)]);
            RTFSprms aAttributes;
            aAttributes.set(NS_ooxml::LN_CT_Fonts_ascii, pValue);
            putNestedSprm(m_aStates.top().aTableSprms, NS_ooxml::LN_CT_Lvl_rPr,
                          NS_ooxml::LN_EG_RPrBase_rFonts, new RTFValue(aAttributes));
        }
        break;
        case RTF_VIEWSCALE:
            m_aSettingsTableAttributes.set(NS_ooxml::LN_CT_Zoom_percent, pIntValue);
            break;
        case RTF_BIN:
        {
            m_aStates.top().nInternalState = RTFInternalState::BIN;
            m_aStates.top().nBinaryToRead = nParam;
        }
        break;
        case RTF_DPLINECOR:
            m_aStates.top().aDrawingObject.setLineColorR(nParam);
            m_aStates.top().aDrawingObject.setHasLineColor(true);
            break;
        case RTF_DPLINECOG:
            m_aStates.top().aDrawingObject.setLineColorG(nParam);
            m_aStates.top().aDrawingObject.setHasLineColor(true);
            break;
        case RTF_DPLINECOB:
            m_aStates.top().aDrawingObject.setLineColorB(nParam);
            m_aStates.top().aDrawingObject.setHasLineColor(true);
            break;
        case RTF_DPFILLBGCR:
            m_aStates.top().aDrawingObject.setFillColorR(nParam);
            m_aStates.top().aDrawingObject.setHasFillColor(true);
            break;
        case RTF_DPFILLBGCG:
            m_aStates.top().aDrawingObject.setFillColorG(nParam);
            m_aStates.top().aDrawingObject.setHasFillColor(true);
            break;
        case RTF_DPFILLBGCB:
            m_aStates.top().aDrawingObject.setFillColorB(nParam);
            m_aStates.top().aDrawingObject.setHasFillColor(true);
            break;
        case RTF_CLSHDNG:
        {
            int nValue = -1;
            switch (nParam)
            {
                case 500:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct5;
                    break;
                case 1000:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct10;
                    break;
                case 1200:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct12;
                    break;
                case 1500:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct15;
                    break;
                case 2000:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct20;
                    break;
                case 2500:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct25;
                    break;
                case 3000:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct30;
                    break;
                case 3500:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct35;
                    break;
                case 3700:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct37;
                    break;
                case 4000:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct40;
                    break;
                case 4500:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct45;
                    break;
                case 5000:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct50;
                    break;
                case 5500:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct55;
                    break;
                case 6000:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct60;
                    break;
                case 6200:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct62;
                    break;
                case 6500:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct65;
                    break;
                case 7000:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct70;
                    break;
                case 7500:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct75;
                    break;
                case 8000:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct80;
                    break;
                case 8500:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct85;
                    break;
                case 8700:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct87;
                    break;
                case 9000:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct90;
                    break;
                case 9500:
                    nValue = NS_ooxml::LN_Value_ST_Shd_pct95;
                    break;
                default:
                    break;
            }
            if (nValue != -1)
                putNestedAttribute(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_shd,
                                   NS_ooxml::LN_CT_Shd_val, new RTFValue(nValue));
        }
        break;
        case RTF_DODHGT:
            m_aStates.top().aDrawingObject.setDhgt(nParam);
            break;
        case RTF_DPPOLYCOUNT:
            if (nParam >= 0)
            {
                m_aStates.top().aDrawingObject.setPolyLineCount(nParam);
            }
            break;
        case RTF_DPPTX:
        {
            RTFDrawingObject& rDrawingObject = m_aStates.top().aDrawingObject;

            if (rDrawingObject.getPolyLinePoints().empty())
                dispatchValue(RTF_DPPOLYCOUNT, 2);

            rDrawingObject.getPolyLinePoints().emplace_back(
                awt::Point(convertTwipToMm100(nParam), 0));
        }
        break;
        case RTF_DPPTY:
        {
            RTFDrawingObject& rDrawingObject = m_aStates.top().aDrawingObject;
            if (!rDrawingObject.getPolyLinePoints().empty())
            {
                rDrawingObject.getPolyLinePoints().back().Y = convertTwipToMm100(nParam);
                rDrawingObject.setPolyLineCount(rDrawingObject.getPolyLineCount() - 1);
                if (rDrawingObject.getPolyLineCount() == 0 && rDrawingObject.getPropertySet().is())
                {
                    uno::Sequence<uno::Sequence<awt::Point>> aPointSequenceSequence
                        = { comphelper::containerToSequence(rDrawingObject.getPolyLinePoints()) };
                    rDrawingObject.getPropertySet()->setPropertyValue(
                        "PolyPolygon", uno::Any(aPointSequenceSequence));
                }
            }
        }
        break;
        case RTF_SHPFBLWTXT:
            // Shape is below text -> send it to the background.
            m_aStates.top().aShape.setInBackground(nParam != 0);
            break;
        case RTF_CLPADB:
        case RTF_CLPADL:
        case RTF_CLPADR:
        case RTF_CLPADT:
        {
            RTFSprms aAttributes;
            aAttributes.set(NS_ooxml::LN_CT_TblWidth_type,
                            new RTFValue(NS_ooxml::LN_Value_ST_TblWidth_dxa));
            aAttributes.set(NS_ooxml::LN_CT_TblWidth_w, new RTFValue(nParam));
            // Top and left is swapped, that's what Word does.
            switch (nKeyword)
            {
                case RTF_CLPADB:
                    nSprm = NS_ooxml::LN_CT_TcMar_bottom;
                    break;
                case RTF_CLPADL:
                    nSprm = NS_ooxml::LN_CT_TcMar_top;
                    break;
                case RTF_CLPADR:
                    nSprm = NS_ooxml::LN_CT_TcMar_right;
                    break;
                case RTF_CLPADT:
                    nSprm = NS_ooxml::LN_CT_TcMar_left;
                    break;
                default:
                    break;
            }
            putNestedSprm(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcMar, nSprm,
                          new RTFValue(aAttributes));
        }
        break;
        case RTF_TRPADDFB:
        case RTF_TRPADDFL:
        case RTF_TRPADDFR:
        case RTF_TRPADDFT:
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
                case RTF_TRPADDFB:
                    nSprm = NS_ooxml::LN_CT_TcMar_bottom;
                    break;
                case RTF_TRPADDFL:
                    nSprm = NS_ooxml::LN_CT_TcMar_left;
                    break;
                case RTF_TRPADDFR:
                    nSprm = NS_ooxml::LN_CT_TcMar_right;
                    break;
                case RTF_TRPADDFT:
                    nSprm = NS_ooxml::LN_CT_TcMar_top;
                    break;
                default:
                    break;
            }
            putNestedAttribute(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcMar,
                               nSprm, new RTFValue(aAttributes));
            putNestedAttribute(m_aDefaultState.aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcMar,
                               nSprm, new RTFValue(aAttributes));
        }
        break;
        case RTF_TRPADDB:
        case RTF_TRPADDL:
        case RTF_TRPADDR:
        case RTF_TRPADDT:
        {
            RTFSprms aAttributes;
            aAttributes.set(NS_ooxml::LN_CT_TblWidth_w, new RTFValue(nParam));
            switch (nKeyword)
            {
                case RTF_TRPADDB:
                    nSprm = NS_ooxml::LN_CT_TcMar_bottom;
                    break;
                case RTF_TRPADDL:
                    nSprm = NS_ooxml::LN_CT_TcMar_left;
                    break;
                case RTF_TRPADDR:
                    nSprm = NS_ooxml::LN_CT_TcMar_right;
                    break;
                case RTF_TRPADDT:
                    nSprm = NS_ooxml::LN_CT_TcMar_top;
                    break;
                default:
                    break;
            }
            putNestedSprm(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcMar, nSprm,
                          new RTFValue(aAttributes));
            putNestedSprm(m_aDefaultState.aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcMar, nSprm,
                          new RTFValue(aAttributes));
        }
        break;
        case RTF_FI:
        {
            if (m_aStates.top().eDestination == Destination::LISTLEVEL)
            {
                if (m_aStates.top().bLevelNumbersValid)
                    putNestedAttribute(m_aStates.top().aTableSprms, NS_ooxml::LN_CT_PPrBase_ind,
                                       NS_ooxml::LN_CT_Ind_firstLine, pIntValue);
                else
                    m_aInvalidListLevelFirstIndents[m_nListLevel] = nParam;
            }
            else
                putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_ind,
                                   NS_ooxml::LN_CT_Ind_firstLine, pIntValue);
            break;
        }
        case RTF_LI:
        {
            if (m_aStates.top().eDestination == Destination::LISTLEVEL)
            {
                if (m_aStates.top().bLevelNumbersValid)
                    putNestedAttribute(m_aStates.top().aTableSprms, NS_ooxml::LN_CT_PPrBase_ind,
                                       NS_ooxml::LN_CT_Ind_left, pIntValue);
            }
            else
            {
                putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_ind,
                                   NS_ooxml::LN_CT_Ind_left, pIntValue);
            }
            // It turns out \li should reset the \fi inherited from the stylesheet.
            // So set the direct formatting to zero, if we don't have such direct formatting yet.
            putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_ind,
                               NS_ooxml::LN_CT_Ind_firstLine, new RTFValue(0),
                               RTFOverwrite::NO_IGNORE);
        }
        break;
        case RTF_RI:
            putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_ind,
                               NS_ooxml::LN_CT_Ind_right, pIntValue);
            break;
        case RTF_LIN:
            putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_ind,
                               NS_ooxml::LN_CT_Ind_start, pIntValue);
            break;
        case RTF_RIN:
            putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_ind,
                               NS_ooxml::LN_CT_Ind_end, pIntValue);
            break;
        case RTF_OUTLINELEVEL:
            m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_outlineLvl, pIntValue);
            break;
        case RTF_TRGAPH:
            // Half of the space between the cells of a table row: default left/right table cell margin.
            if (nParam > 0)
            {
                RTFSprms aAttributes;
                aAttributes.set(NS_ooxml::LN_CT_TblWidth_type,
                                new RTFValue(NS_ooxml::LN_Value_ST_TblWidth_dxa));
                aAttributes.set(NS_ooxml::LN_CT_TblWidth_w, pIntValue);
                putNestedSprm(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblCellMar,
                              NS_ooxml::LN_CT_TblCellMar_left, new RTFValue(aAttributes));
                putNestedSprm(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblCellMar,
                              NS_ooxml::LN_CT_TblCellMar_right, new RTFValue(aAttributes));
            }
            break;
        case RTF_TRFTSWIDTH:
            putNestedAttribute(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblW,
                               NS_ooxml::LN_CT_TblWidth_type, pIntValue);
            break;
        case RTF_TRWWIDTH:
            putNestedAttribute(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblW,
                               NS_ooxml::LN_CT_TblWidth_w, pIntValue);
            break;
        case RTF_PROPTYPE:
        {
            switch (nParam)
            {
                case 3:
                    m_aStates.top().aPropType = cppu::UnoType<sal_Int32>::get();
                    break;
                case 5:
                    m_aStates.top().aPropType = cppu::UnoType<double>::get();
                    break;
                case 11:
                    m_aStates.top().aPropType = cppu::UnoType<bool>::get();
                    break;
                case 30:
                    m_aStates.top().aPropType = cppu::UnoType<OUString>::get();
                    break;
                case 64:
                    m_aStates.top().aPropType = cppu::UnoType<util::DateTime>::get();
                    break;
            }
        }
        break;
        case RTF_DIBITMAP:
            m_aStates.top().aPicture.eStyle = RTFBmpStyle::DIBITMAP;
            break;
        case RTF_TRWWIDTHA:
            m_aStates.top().nTableRowWidthAfter = nParam;
            break;
        case RTF_ANIMTEXT:
        {
            nId = 0;
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
                m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_effect,
                                                    new RTFValue(nId));
            break;
        }
        case RTF_VIEWBKSP:
        {
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_displayBackgroundShape, pIntValue);
            // Send this token immediately, if it only appears before the first
            // run, it will be too late, we ignored the background shape already by then.
            outputSettingsTable();
            break;
        }
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
