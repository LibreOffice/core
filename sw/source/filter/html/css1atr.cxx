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

#include <string_view>

#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <comphelper/xmlencode.hxx>
#include <vcl/svapp.hxx>
#include <svl/whiter.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/svdobj.hxx>
#include <editeng/langitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/htmlkywd.hxx>
#include <svl/urihelper.hxx>
#include <unotools/charclass.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <charfmt.hxx>
#include <fmtclds.hxx>
#include <fmtcol.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtpdsc.hxx>
#include <fmtlsplt.hxx>
#include <pagedesc.hxx>
#include <fmtanchr.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>
#include <swtable.hxx>
// NOTES
#include <ftninfo.hxx>
#include <ftnidx.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
// FOOTNOTES
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <swerror.h>
#include <paratr.hxx>
#include <frmatr.hxx>
#include <poolfmt.hxx>
#include "css1kywd.hxx"
#include "wrthtml.hxx"
#include "htmlnum.hxx"
#include "css1atr.hxx"

#include <IDocumentStylePoolAccess.hxx>
#include <numrule.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <o3tl/unit_conversion.hxx>

#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>

using namespace css;
using editeng::SvxBorderLine;

#define HTML_HEADSPACE (12*20)

namespace {

enum class Css1FrameSize {
    NONE       = 0x00,
    Width      = 0x01,
    MinHeight  = 0x02,
    FixHeight  = 0x04,
    Pixel      = 0x10,
};

}

namespace o3tl {
    template<> struct typed_flags<Css1FrameSize> : is_typed_flags<Css1FrameSize, 0x17> {};
}

constexpr int DOT_LEADERS_MAX_WIDTH = 18; // cm

static SwHTMLWriter& OutCSS1_SwFormat( SwHTMLWriter& rWrt, const SwFormat& rFormat,
                              IDocumentStylePoolAccess /*SwDoc*/ *pDoc, SwDoc *pTemplate );
static SwHTMLWriter& OutCSS1_SwPageDesc( SwHTMLWriter& rWrt, const SwPageDesc& rFormat,
                                   IDocumentStylePoolAccess /*SwDoc*/ *pDoc, SwDoc *pTemplate,
                                   sal_uInt16 nRefPoolId, bool bExtRef,
                                   bool bPseudo=true );
static SwHTMLWriter& OutCSS1_SwFootnoteInfo( SwHTMLWriter& rWrt, const SwEndNoteInfo& rInfo,
                                  SwDoc *pDoc, bool bHasNotes, bool bEndNote );
static void OutCSS1_SwFormatDropAttrs( SwHTMLWriter& rHWrt,
                                    const SwFormatDrop& rDrop,
                                     const SfxItemSet *pCharFormatItemSet=nullptr );
static SwHTMLWriter& OutCSS1_SvxTextLn_SvxCrOut_SvxBlink( SwHTMLWriter& rWrt,
                    const SvxUnderlineItem *pUItem,
                    const SvxOverlineItem *pOItem,
                    const SvxCrossedOutItem *pCOItem,
                    const SvxBlinkItem *pBItem );
static SwHTMLWriter& OutCSS1_SvxFontWeight( SwHTMLWriter& rWrt, const SfxPoolItem& rHt );
static SwHTMLWriter& OutCSS1_SvxPosture( SwHTMLWriter& rWrt, const SfxPoolItem& rHt );
static SwHTMLWriter& OutCSS1_SvxULSpace( SwHTMLWriter& rWrt, const SfxPoolItem& rHt );
static SwHTMLWriter& OutCSS1_SvxFirstLineIndent(SwHTMLWriter& rWrt, const SfxPoolItem& rHt);
static SwHTMLWriter& OutCSS1_SvxTextLeftMargin(SwHTMLWriter& rWrt, const SfxPoolItem& rHt);
static SwHTMLWriter& OutCSS1_SvxRightMargin(SwHTMLWriter& rWrt, const SfxPoolItem& rHt);
static SwHTMLWriter& OutCSS1_SvxLRSpace( SwHTMLWriter& rWrt, const SfxPoolItem& rHt );
static SwHTMLWriter& OutCSS1_SvxULSpace_SvxLRSpace( SwHTMLWriter& rWrt,
                                        const SvxULSpaceItem *pULSpace,
                                        const SvxLRSpaceItem *pLRSpace );
static SwHTMLWriter& OutCSS1_SvxULSpace_SvxLRSpace( SwHTMLWriter& rWrt,
                                        const SfxItemSet& rItemSet );
static SwHTMLWriter& OutCSS1_SvxBrush( SwHTMLWriter& rWrt, const SfxPoolItem& rHt,
                                 sw::Css1Background nMode,
                                 const OUString *pGraphicName );
static SwHTMLWriter& OutCSS1_SvxBrush( SwHTMLWriter& rWrt, const SfxPoolItem& rHt );
static SwHTMLWriter& OutCSS1_SwFormatFrameSize( SwHTMLWriter& rWrt, const SfxPoolItem& rHt,
                                     Css1FrameSize nMode );
static SwHTMLWriter& OutCSS1_SvxFormatBreak_SwFormatPDesc_SvxFormatKeep( SwHTMLWriter& rWrt,
                                        const SfxItemSet& rItemSet,
                                        bool bDeep );
static SwHTMLWriter& OutCSS1_SwFormatLayoutSplit( SwHTMLWriter& rWrt, const SfxPoolItem& rHt );

namespace
{

const char sCSS1_rule_end[] = " }";
const char sCSS1_span_tag_end[] = "\">";
const char cCSS1_style_opt_end  = '\"';

const char* const sHTML_FTN_fontheight = "57%";

OString lclConvToHex(sal_uInt16 nHex)
{
    char aNToABuf[] = "00";

    // set pointer to end of buffer
    char *pStr = aNToABuf + (sizeof(aNToABuf)-1);
    for( sal_uInt8 n = 0; n < 2; ++n )
    {
        *(--pStr) = static_cast<char>(nHex & 0xf ) + 48;
        if( *pStr > '9' )
            *pStr += 39;
        nHex >>= 4;
    }

    return OString(aNToABuf, 2);
}
}

bool IgnorePropertyForReqIF(bool bReqIF, std::string_view rProperty, std::string_view rValue,
                            std::optional<sw::Css1Background> oMode)
{
    if (!bReqIF)
        return false;

    if (oMode.has_value() && *oMode != sw::Css1Background::TableCell)
    {
        // Table or row.
        if (rProperty == sCSS1_P_background && rValue == "transparent")
        {
            // This is the default already.
            return true;
        }

        return false;
    }

    // Only allow these two keys, nothing else in ReqIF mode.
    if (rProperty == sCSS1_P_text_decoration)
    {
        // Deny other text-decoration values (e.g. "none").
        if (rValue == "underline" || rValue == "line-through")
        {
            return false;
        }

        return true;
    }

    if (rProperty == sCSS1_P_color)
        return false;

    return true;
}

OString GetCSS1_Color(const Color& rColor)
{
    return "#" + lclConvToHex(rColor.GetRed()) + lclConvToHex(rColor.GetGreen()) + lclConvToHex(rColor.GetBlue());
}

namespace {

class SwCSS1OutMode
{
    SwHTMLWriter& rWrt;
    sal_uInt16 nOldMode;

public:

    SwCSS1OutMode( SwHTMLWriter& rHWrt, sal_uInt16 nMode,
                   const OUString *pSelector ) :
        rWrt( rHWrt ),
        nOldMode( rHWrt.m_nCSS1OutMode )
    {
        rWrt.m_nCSS1OutMode = nMode;
        rWrt.m_bFirstCSS1Property = true;
        if( pSelector )
            rWrt.m_aCSS1Selector = *pSelector;
    }

    ~SwCSS1OutMode()
    {
        rWrt.m_nCSS1OutMode = nOldMode;
    }
};

}

void SwHTMLWriter::OutCSS1_Property( std::string_view pProp,
                                     std::string_view sVal,
                                     const OUString *pSVal,
                                     std::optional<sw::Css1Background> oMode )
{
    OString aPropertyValue(sVal);
    if (aPropertyValue.isEmpty() && pSVal)
    {
        aPropertyValue = OUStringToOString(*pSVal, RTL_TEXTENCODING_UTF8);
    }
    if (IgnorePropertyForReqIF(mbReqIF, pProp, aPropertyValue, oMode))
        return;

    OStringBuffer sOut;

    if( m_bFirstCSS1Rule && (m_nCSS1OutMode & CSS1_OUTMODE_RULE_ON)!=0 )
    {
        m_bFirstCSS1Rule = false;
        OutNewLine();
        sOut.append("<" + GetNamespace() + OOO_STRING_SVTOOLS_HTML_style " "
                    OOO_STRING_SVTOOLS_HTML_O_type "=\"text/css\">");
    //  Optional CSS2 code for dot leaders (dotted line between the Table of Contents titles and page numbers):
    //  (More information: http://www.w3.org/Style/Examples/007/leaders.en.html)
    //
    //  p.leaders {
    //      /* FIXME:
    //         (1) dots line up vertically only in the paragraphs with the same alignment/level
    //         (2) max-width = 18 cm instead of 80em; possible improvement with the new CSS3 calc() */
    //      max-width: 18cm; /* note: need to overwrite max-width with max-width - border-left_of_the_actual_paragraph */
    //      padding: 0;
    //      overflow-x: hidden;
    //      line-height: 120%; /* note: avoid HTML scrollbars and missing descenders of the letters */
    //  }
    //  p.leaders:after {
    //      float: left;
    //      width: 0;
    //      white-space: nowrap;
    //      content: ". . . . . . . . . . . . . . . . . . ...";
    //  }
    //  p.leaders span:first-child {
    //      padding-right: 0.33em;
    //      background: white;
    //  }
    //  p.leaders span + span {
    //      float: right;
    //      padding-left: 0.33em;
    //      background: white;
    //      position: relative;
    //      z-index: 1
    //  }

        if (m_bCfgPrintLayout) {
            sOut.append(
                "p." sCSS2_P_CLASS_leaders "{max-width:" + OString::number(DOT_LEADERS_MAX_WIDTH) +
                    "cm;padding:0;overflow-x:hidden;line-height:120%}"
                "p." sCSS2_P_CLASS_leaders ":after{float:left;width:0;white-space:nowrap;content:\"");
            for (int i = 0; i < 100; i++ )
                sOut.append(". ");
            sOut.append(
                    "\"}p." sCSS2_P_CLASS_leaders " span:first-child{padding-right:0.33em;background:white}"
                    "p." sCSS2_P_CLASS_leaders " span+span{float:right;padding-left:0.33em;"
                    "background:white;position:relative;z-index:1}");
        }
        Strm().WriteOString( sOut );
        sOut.setLength(0);

        IncIndentLevel();
    }

    if( m_bFirstCSS1Property )
    {
        switch( m_nCSS1OutMode & CSS1_OUTMODE_ANY_ON )
        {
        case CSS1_OUTMODE_SPAN_TAG_ON:
        case CSS1_OUTMODE_SPAN_TAG1_ON:
            if( m_bTagOn )
            {
                sOut.append("<" + GetNamespace() + OOO_STRING_SVTOOLS_HTML_span
                            " " OOO_STRING_SVTOOLS_HTML_O_style "=\"");
            }
            else
            {
                HTMLOutFuncs::Out_AsciiTag( Strm(), Concat2View(GetNamespace() + OOO_STRING_SVTOOLS_HTML_span), false );
                return;
            }
            break;

        case CSS1_OUTMODE_RULE_ON:
            {
                OutNewLine();
                sOut.append(OUStringToOString(m_aCSS1Selector, RTL_TEXTENCODING_UTF8) + " { ");
            }
            break;

        case CSS1_OUTMODE_STYLE_OPT_ON:
            sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_style "=\"");
            break;
        }
        m_bFirstCSS1Property = false;
    }
    else
    {
        sOut.append("; ");
    }

    sOut.append(pProp + OString::Concat(": "));
    if( m_nCSS1OutMode & CSS1_OUTMODE_ENCODE )
    {
        // for STYLE-Option encode string
        Strm().WriteOString( sOut );
        sOut.setLength(0);
        if( !sVal.empty() )
            HTMLOutFuncs::Out_String( Strm(), OUString::createFromAscii(sVal) );
        else if( pSVal )
            HTMLOutFuncs::Out_String( Strm(), *pSVal );
    }
    else
    {
        // for STYLE-Tag print string directly
        sOut.append(aPropertyValue);
    }

    if (!sOut.isEmpty())
        Strm().WriteOString( sOut );
}

static void AddUnitPropertyValue(OStringBuffer &rOut, tools::Long nVal,
    FieldUnit eUnit)
{
    if( nVal < 0 )
    {
        // special-case sign symbol
        nVal = -nVal;
        rOut.append('-');
    }

    o3tl::Length eTo;
    int nFac; // used to get specific number of decimals
    std::string_view pUnit;
    switch( eUnit )
    {
    case FieldUnit::MM_100TH:
        OSL_ENSURE( FieldUnit::MM == eUnit, "Measuring unit not supported" );
        [[fallthrough]];
    case FieldUnit::MM:
        eTo = o3tl::Length::mm;
        nFac = 100;
        pUnit = sCSS1_UNIT_mm;
        break;

    case FieldUnit::M:
    case FieldUnit::KM:
        OSL_ENSURE( FieldUnit::CM == eUnit, "Measuring unit not supported" );
        [[fallthrough]];
    case FieldUnit::CM:
        eTo = o3tl::Length::cm;
        nFac = 100;
        pUnit = sCSS1_UNIT_cm;
        break;

    case FieldUnit::TWIP:
        OSL_ENSURE( FieldUnit::POINT == eUnit, "Measuring unit not supported" );
        [[fallthrough]];
    case FieldUnit::POINT:
        eTo = o3tl::Length::pt;
        nFac = 10;
        pUnit = sCSS1_UNIT_pt;
        break;

    case FieldUnit::PICA:
        eTo = o3tl::Length::pc;
        nFac = 100;
        pUnit = sCSS1_UNIT_pc;
        break;

    case FieldUnit::NONE:
    case FieldUnit::FOOT:
    case FieldUnit::MILE:
    case FieldUnit::CUSTOM:
    case FieldUnit::PERCENT:
    case FieldUnit::INCH:
    default:
        OSL_ENSURE( FieldUnit::INCH == eUnit, "Measuring unit not supported" );
        eTo = o3tl::Length::in;
        nFac = 100;
        pUnit = sCSS1_UNIT_inch;
        break;
    }

    sal_Int64 nResult = o3tl::convert(nVal * nFac, o3tl::Length::twip, eTo);
    rOut.append(nResult/nFac);
    if ((nResult % nFac) != 0)
    {
        rOut.append('.');
        while (nFac > 1 && (nResult % nFac) != 0)
        {
            nFac /= 10;
            rOut.append((nResult / nFac) % 10);
        }
    }

    rOut.append(pUnit);
}

void SwHTMLWriter::OutCSS1_UnitProperty( std::string_view pProp, tools::Long nVal )
{
    OStringBuffer sOut;
    AddUnitPropertyValue(sOut, nVal, m_eCSS1Unit);
    OutCSS1_PropertyAscii(pProp, sOut);
}

void SwHTMLWriter::OutCSS1_PixelProperty( std::string_view pProp, tools::Long nTwips )
{
    OString sOut(OString::number(ToPixel(nTwips)) + sCSS1_UNIT_px);
    OutCSS1_PropertyAscii(pProp, sOut);
}

void SwHTMLWriter::OutStyleSheet( const SwPageDesc& rPageDesc )
{
    m_bFirstCSS1Rule = true;

// Feature: PrintExt
    if( IsHTMLMode(HTMLMODE_PRINT_EXT) )
    {
        const SwPageDesc *pFirstPageDesc = nullptr;
        sal_uInt16 nFirstRefPoolId = RES_POOLPAGE_HTML;
        m_bCSS1IgnoreFirstPageDesc = true;

        // First we try to guess how the document is constructed.
        // Allowed are only the templates: HTML, 1st page, left page, and right page.
        // A first page is only exported, if it matches the template "1st page".
        // Left and right pages are only exported, if their templates are linked.
        // If other templates are used, only very simple cases are exported.
        const SwPageDesc *pPageDesc = &rPageDesc;
        const SwPageDesc *pFollow = rPageDesc.GetFollow();
        if( RES_POOLPAGE_FIRST == pPageDesc->GetPoolFormatId() &&
            pFollow != pPageDesc &&
            !IsPoolUserFormat( pFollow->GetPoolFormatId() ) )
        {
            // the document has a first page
            pFirstPageDesc = pPageDesc;
            pPageDesc = pFollow;
            pFollow = pPageDesc->GetFollow();
        }

        IDocumentStylePoolAccess* pStylePoolAccess = &getIDocumentStylePoolAccess();
        if( pPageDesc == pFollow )
        {
            // The document is one-sided; no matter what page, we do not create a 2-sided doc.
            // The attribute is exported relative to the HTML page template.
            OutCSS1_SwPageDesc( *this, *pPageDesc, pStylePoolAccess, m_xTemplate.get(),
                                RES_POOLPAGE_HTML, true, false );
            nFirstRefPoolId = pFollow->GetPoolFormatId();
        }
        else if( (RES_POOLPAGE_LEFT == pPageDesc->GetPoolFormatId() &&
                  RES_POOLPAGE_RIGHT == pFollow->GetPoolFormatId()) ||
                 (RES_POOLPAGE_RIGHT == pPageDesc->GetPoolFormatId() &&
                  RES_POOLPAGE_LEFT == pFollow->GetPoolFormatId()) )
        {
            // the document is double-sided
            OutCSS1_SwPageDesc( *this, *pPageDesc, pStylePoolAccess, m_xTemplate.get(),
                                RES_POOLPAGE_HTML, true );
            OutCSS1_SwPageDesc( *this, *pFollow, pStylePoolAccess, m_xTemplate.get(),
                                RES_POOLPAGE_HTML, true );
            nFirstRefPoolId = RES_POOLPAGE_RIGHT;
            m_bCSS1IgnoreFirstPageDesc = false;
        }
        // other cases we miss

        if( pFirstPageDesc )
          OutCSS1_SwPageDesc( *this, *pFirstPageDesc, pStylePoolAccess, m_xTemplate.get(),
                                nFirstRefPoolId, false );
    }

    // The text body style has to be exported always (if it is changed compared
    // to the template), because it is used as reference for any style
    // that maps to <P>, and that's especially the standard style
    getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TEXT, false );

    // the Default-TextStyle is not also exported !!
    // 0-Style is the Default; is never exported !!
    const size_t nTextFormats = m_pDoc->GetTextFormatColls()->size();
    for( size_t i = 1; i < nTextFormats; ++i )
    {
        const SwTextFormatColl* pColl = (*m_pDoc->GetTextFormatColls())[i];
        sal_uInt16 nPoolId = pColl->GetPoolFormatId();
        if( nPoolId == RES_POOLCOLL_TEXT || m_pDoc->IsUsed( *pColl ) )
            OutCSS1_SwFormat( *this, *pColl, &m_pDoc->getIDocumentStylePoolAccess(), m_xTemplate.get() );
    }

    // the Default-TextStyle is not also exported !!
    const size_t nCharFormats = m_pDoc->GetCharFormats()->size();
    for( size_t i = 1; i < nCharFormats; ++i )
    {
        const SwCharFormat *pCFormat = (*m_pDoc->GetCharFormats())[i];
        sal_uInt16 nPoolId = pCFormat->GetPoolFormatId();
        if( nPoolId == RES_POOLCHR_INET_NORMAL ||
            nPoolId == RES_POOLCHR_INET_VISIT ||
            m_pDoc->IsUsed( *pCFormat ) )
            OutCSS1_SwFormat( *this, *pCFormat, &m_pDoc->getIDocumentStylePoolAccess(), m_xTemplate.get() );
    }

    bool bHasEndNotes {false};
    bool bHasFootNotes {false};
    const SwFootnoteIdxs& rIdxs = m_pDoc->GetFootnoteIdxs();
    for( auto pIdx : rIdxs )
    {
        if( pIdx->GetFootnote().IsEndNote() )
        {
            bHasEndNotes = true;
            if (bHasFootNotes)
                break;
        }
        else
        {
            bHasFootNotes = true;
            if (bHasEndNotes)
                break;
        }
    }
    OutCSS1_SwFootnoteInfo( *this, m_pDoc->GetFootnoteInfo(), m_pDoc, bHasFootNotes, false );
    OutCSS1_SwFootnoteInfo( *this, m_pDoc->GetEndNoteInfo(), m_pDoc, bHasEndNotes, true );

    if( !m_bFirstCSS1Rule )
    {
        DecIndentLevel();

        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), Concat2View(GetNamespace() + OOO_STRING_SVTOOLS_HTML_style), false );
    }
    else
    {
        m_bFirstCSS1Rule = false;
    }

    m_nDfltTopMargin = 0;
    m_nDfltBottomMargin = 0;
}

// if pPseudo is set, Styles-Sheets will be exported;
// otherwise we only search for Token and Class for a Format
sal_uInt16 SwHTMLWriter::GetCSS1Selector( const SwFormat *pFormat, OString& rToken,
                                      OUString& rClass, sal_uInt16& rRefPoolId,
                                      OUString *pPseudo )
{
    sal_uInt16 nDeep = 0;
    rToken.clear();
    rClass.clear();
    rRefPoolId = 0;
    if( pPseudo )
        pPseudo->clear();

    bool bChrFormat = RES_CHRFMT==pFormat->Which();

    // search formats above for the nearest standard or HTML-Tag template
    const SwFormat *pPFormat = pFormat;
    while( pPFormat && !pPFormat->IsDefault() )
    {
        bool bStop = false;
        sal_uInt16 nPoolId = pPFormat->GetPoolFormatId();
        if( USER_FMT & nPoolId )
        {
            // user templates
            const UIName& aNm(pPFormat->GetName());

            if (!bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_blockquote)
            {
                rRefPoolId = RES_POOLCOLL_HTML_BLOCKQUOTE;
                rToken = OOO_STRING_SVTOOLS_HTML_blockquote ""_ostr;
            }
            else if (bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_citation)
            {
                rRefPoolId = RES_POOLCHR_HTML_CITATION;
                rToken = OOO_STRING_SVTOOLS_HTML_citation ""_ostr;
            }
            else if (bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_code)
            {
                rRefPoolId = RES_POOLCHR_HTML_CODE;
                rToken = OOO_STRING_SVTOOLS_HTML_code ""_ostr;
            }
            else if (bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_definstance)
            {
                rRefPoolId = RES_POOLCHR_HTML_DEFINSTANCE;
                rToken = OOO_STRING_SVTOOLS_HTML_definstance ""_ostr;
            }
            else if (!bChrFormat && (aNm == OOO_STRING_SVTOOLS_HTML_dd ||
                                  aNm == OOO_STRING_SVTOOLS_HTML_dt))
            {
                sal_uInt16 nDefListLvl = GetDefListLvl(aNm, nPoolId);
                // Export the templates DD 1/DT 1,
                // but none of their derived templates,
                // also not DD 2/DT 2 etc.
                if (nDefListLvl)
                {
                    if (pPseudo && (nDeep || (nDefListLvl & 0x0fff) > 1))
                    {
                        bStop = true;
                    }
                    else if (nDefListLvl & HTML_DLCOLL_DD)
                    {
                        rRefPoolId = RES_POOLCOLL_HTML_DD;
                        rToken = OOO_STRING_SVTOOLS_HTML_dd ""_ostr;
                    }
                    else
                    {
                        rRefPoolId = RES_POOLCOLL_HTML_DT;
                        rToken = OOO_STRING_SVTOOLS_HTML_dt ""_ostr;
                    }
                }
            }
            else if (bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_emphasis)
            {
                rRefPoolId = RES_POOLCHR_HTML_EMPHASIS;
                rToken = OOO_STRING_SVTOOLS_HTML_emphasis ""_ostr;
            }
            else if (!bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_horzrule)
            {
                // do not export HR !
                bStop = (nDeep==0);
            }
            else if (bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_keyboard)
            {
                rRefPoolId = RES_POOLCHR_HTML_KEYBOARD;
                rToken = OOO_STRING_SVTOOLS_HTML_keyboard ""_ostr;
            }
            else if (!bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_listing)
            {
                // Export Listings as PRE or PRE-derived template
                rToken = OOO_STRING_SVTOOLS_HTML_preformtxt ""_ostr;
                rRefPoolId = RES_POOLCOLL_HTML_PRE;
                nDeep = CSS1_FMT_CMPREF;
            }
            else if (!bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_preformtxt)
            {
                rRefPoolId = RES_POOLCOLL_HTML_PRE;
                rToken = OOO_STRING_SVTOOLS_HTML_preformtxt ""_ostr;
            }
            else if (bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_sample)
            {
                rRefPoolId = RES_POOLCHR_HTML_SAMPLE;
                rToken = OOO_STRING_SVTOOLS_HTML_sample ""_ostr;
            }
            else if (bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_strong)
            {
                rRefPoolId = RES_POOLCHR_HTML_STRONG;
                rToken = OOO_STRING_SVTOOLS_HTML_strong ""_ostr;
            }
            else if (bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_teletype)
            {
                rRefPoolId = RES_POOLCHR_HTML_TELETYPE;
                rToken = OOO_STRING_SVTOOLS_HTML_teletype ""_ostr;
            }
            else if (bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_variable)
            {
                rRefPoolId = RES_POOLCHR_HTML_VARIABLE;
                rToken = OOO_STRING_SVTOOLS_HTML_variable ""_ostr;
            }
            else if (!bChrFormat && aNm == OOO_STRING_SVTOOLS_HTML_xmp)
            {
                // export XMP as PRE (but not the template as Style)
                rToken = OOO_STRING_SVTOOLS_HTML_preformtxt ""_ostr;
                rRefPoolId = RES_POOLCOLL_HTML_PRE;
                nDeep = CSS1_FMT_CMPREF;
            }

            // if a PoolId is set, the Name of the template is that of the related Token
            OSL_ENSURE( (rRefPoolId != 0) == (!rToken.isEmpty()),
                    "Token missing" );
        }
        else
        {
            // Pool templates
            switch( nPoolId )
            {
            // paragraph templates
            case RES_POOLCOLL_HEADLINE_BASE:
            case RES_POOLCOLL_STANDARD:
                // do not export this template
            case RES_POOLCOLL_HTML_HR:
                // do not export HR !
                bStop = (nDeep==0);
                break;
            case RES_POOLCOLL_TEXT:
                rToken = OOO_STRING_SVTOOLS_HTML_parabreak ""_ostr;
                break;
            case RES_POOLCOLL_HEADLINE1:
                rToken = OOO_STRING_SVTOOLS_HTML_head1 ""_ostr;
                break;
            case RES_POOLCOLL_HEADLINE2:
                rToken = OOO_STRING_SVTOOLS_HTML_head2 ""_ostr;
                break;
            case RES_POOLCOLL_HEADLINE3:
                rToken = OOO_STRING_SVTOOLS_HTML_head3 ""_ostr;
                break;
            case RES_POOLCOLL_HEADLINE4:
                rToken = OOO_STRING_SVTOOLS_HTML_head4 ""_ostr;
                break;
            case RES_POOLCOLL_HEADLINE5:
                rToken = OOO_STRING_SVTOOLS_HTML_head5 ""_ostr;
                break;
            case RES_POOLCOLL_HEADLINE6:
                rToken = OOO_STRING_SVTOOLS_HTML_head6 ""_ostr;
                break;
            case RES_POOLCOLL_SEND_ADDRESS:
                rToken = OOO_STRING_SVTOOLS_HTML_address ""_ostr;
                break;
            case RES_POOLCOLL_HTML_BLOCKQUOTE:
                rToken = OOO_STRING_SVTOOLS_HTML_blockquote ""_ostr;
                break;
            case RES_POOLCOLL_HTML_PRE:
                rToken = OOO_STRING_SVTOOLS_HTML_preformtxt ""_ostr;
                break;

            case RES_POOLCOLL_HTML_DD:
                rToken = OOO_STRING_SVTOOLS_HTML_dd ""_ostr;
                break;
            case RES_POOLCOLL_HTML_DT:
                rToken = OOO_STRING_SVTOOLS_HTML_dt ""_ostr;
                break;

            case RES_POOLCOLL_TABLE:
                if( pPseudo )
                {
                    rToken = OOO_STRING_SVTOOLS_HTML_tabledata " "
                             OOO_STRING_SVTOOLS_HTML_parabreak ""_ostr;
                }
                else
                    rToken = OOO_STRING_SVTOOLS_HTML_parabreak ""_ostr;
                break;
            case RES_POOLCOLL_TABLE_HDLN:
                if( pPseudo )
                {
                    rToken = OOO_STRING_SVTOOLS_HTML_tableheader " "
                             OOO_STRING_SVTOOLS_HTML_parabreak ""_ostr;
                }
                else
                    rToken = OOO_STRING_SVTOOLS_HTML_parabreak ""_ostr;
                break;
            case RES_POOLCOLL_FOOTNOTE:
                if( !nDeep )
                {
                    rToken = OOO_STRING_SVTOOLS_HTML_parabreak ""_ostr;
                    rClass = OOO_STRING_SVTOOLS_HTML_sdfootnote;
                    rRefPoolId = RES_POOLCOLL_TEXT;
                    nDeep = CSS1_FMT_CMPREF;
                }
                break;
            case RES_POOLCOLL_ENDNOTE:
                if( !nDeep )
                {
                    rToken = OOO_STRING_SVTOOLS_HTML_parabreak ""_ostr;
                    rClass = OOO_STRING_SVTOOLS_HTML_sdendnote;
                    rRefPoolId = RES_POOLCOLL_TEXT;
                    nDeep = CSS1_FMT_CMPREF;
                }
                break;

            // character templates
            case RES_POOLCHR_HTML_EMPHASIS:
                rToken = OOO_STRING_SVTOOLS_HTML_emphasis ""_ostr;
                break;
            case RES_POOLCHR_HTML_CITATION:
                rToken = OOO_STRING_SVTOOLS_HTML_citation ""_ostr;
                break;
            case RES_POOLCHR_HTML_STRONG:
                rToken = OOO_STRING_SVTOOLS_HTML_strong ""_ostr;
                break;
            case RES_POOLCHR_HTML_CODE:
                rToken = OOO_STRING_SVTOOLS_HTML_code ""_ostr;
                break;
            case RES_POOLCHR_HTML_SAMPLE:
                rToken = OOO_STRING_SVTOOLS_HTML_sample ""_ostr;
                break;
            case RES_POOLCHR_HTML_KEYBOARD:
                rToken = OOO_STRING_SVTOOLS_HTML_keyboard ""_ostr;
                break;
            case RES_POOLCHR_HTML_VARIABLE:
                rToken = OOO_STRING_SVTOOLS_HTML_variable ""_ostr;
                break;
            case RES_POOLCHR_HTML_DEFINSTANCE:
                rToken = OOO_STRING_SVTOOLS_HTML_definstance ""_ostr;
                break;
            case RES_POOLCHR_HTML_TELETYPE:
                rToken = OOO_STRING_SVTOOLS_HTML_teletype ""_ostr;
                break;

            case RES_POOLCHR_INET_NORMAL:
                if( pPseudo )
                {
                    rToken = OOO_STRING_SVTOOLS_HTML_anchor ""_ostr;
                    *pPseudo = sCSS1_link;
                }
                break;
            case RES_POOLCHR_INET_VISIT:
                if( pPseudo )
                {
                    rToken = OOO_STRING_SVTOOLS_HTML_anchor ""_ostr;
                    *pPseudo = sCSS1_visited;
                }
                break;
            }

            // if a token is set, PoolId contains the related template
            if( !rToken.isEmpty() && !rRefPoolId )
                rRefPoolId = nPoolId;
        }

        if( !rToken.isEmpty() || bStop )
        {
            // stop if a HTML-Tag template was found
            break;
        }
        else
        {
            // continue otherwise
            nDeep++;
            pPFormat = pPFormat->DerivedFrom();
        }
    }

    if( !rToken.isEmpty() )
    {
        // this is a HTML-Tag template
        if( !nDeep )
            nDeep = CSS1_FMT_ISTAG;
    }
    else
    {
        // this is not a HTML-Tag template nor derived from one
        nDeep = 0;
    }
    if( nDeep > 0 && nDeep < CSS1_FMT_SPECIAL )
    {
        // If the template is derived from a HTML template,
        // we export it as <TOKEN>.<CLASS>, otherwise as .<CLASS>.
        // <CLASS> is the name of the template after removing all characters
        // before and including the first '.'
        rClass = pFormat->GetName().toString();
        sal_Int32 nPos = rClass.indexOf( '.' );
        if( nPos >= 0 && rClass.getLength() > nPos+1 )
        {
            rClass = rClass.replaceAt( 0, nPos+1, u"" );
        }

        rClass = GetAppCharClass().lowercase( rClass );
        rClass = rClass.replaceAll( ".", "-" );
        rClass = rClass.replaceAll( " ", "-" );
        rClass = rClass.replaceAll( "_", "-" );
    }

    return nDeep;
}

static sal_uInt16 GetCSS1Selector( const SwFormat *pFormat, OUString& rSelector,
                               sal_uInt16& rRefPoolId )
{
    OString aToken;
    OUString aClass;
    OUString aPseudo;

    sal_uInt16 nDeep = SwHTMLWriter::GetCSS1Selector( pFormat, aToken, aClass,
                                                  rRefPoolId, &aPseudo );
    if( nDeep )
    {
        if( !aToken.isEmpty() )
            rSelector = OStringToOUString(aToken, RTL_TEXTENCODING_ASCII_US);
        else
            rSelector.clear();

        if( !aClass.isEmpty() )
            rSelector += "." + aClass;
        if( !aPseudo.isEmpty() )
            rSelector += ":" + aPseudo;
    }

    return nDeep;
}

const SwFormat *SwHTMLWriter::GetTemplateFormat( sal_uInt16 nPoolFormatId,
                                           IDocumentStylePoolAccess* pTemplate /*SwDoc *pTemplate*/)
{
    const SwFormat *pRefFormat = nullptr;

    if( pTemplate )
    {
        OSL_ENSURE( !(USER_FMT & nPoolFormatId),
                "No user templates found" );
        if( POOLGRP_NOCOLLID & nPoolFormatId )
            pRefFormat = pTemplate->GetCharFormatFromPool( nPoolFormatId );
        else
            pRefFormat = pTemplate->GetTextCollFromPool( nPoolFormatId, false );
    }

    return pRefFormat;
}

const SwFormat *SwHTMLWriter::GetParentFormat( const SwFormat& rFormat, sal_uInt16 nDeep )
{
    OSL_ENSURE( nDeep != USHRT_MAX, "Called GetParent for HTML-template!" );
    const SwFormat *pRefFormat = nullptr;

    if( nDeep > 0 )
    {
        // get the pointer for the HTML-Tag template, from which the template is derived
        pRefFormat = &rFormat;
        for( sal_uInt16 i=nDeep; i>0; i-- )
            pRefFormat = pRefFormat->DerivedFrom();

        if( pRefFormat && pRefFormat->IsDefault() )
            pRefFormat = nullptr;
    }

    return pRefFormat;
}

bool swhtml_css1atr_equalFontItems( const SfxPoolItem& r1, const SfxPoolItem& r2 )
{
    return  static_cast<const SvxFontItem &>(r1).GetFamilyName() ==
                    static_cast<const SvxFontItem &>(r2).GetFamilyName() &&
            static_cast<const SvxFontItem &>(r1).GetFamily() ==
                    static_cast<const SvxFontItem &>(r2).GetFamily();
}

void SwHTMLWriter::SubtractItemSet( SfxItemSet& rItemSet,
                                    const SfxItemSet& rRefItemSet,
                                    bool bSetDefaults,
                                    bool bClearSame,
                                     const SfxItemSet *pRefScriptItemSet )
{
    OSL_ENSURE( bSetDefaults || bClearSame,
            "SwHTMLWriter::SubtractItemSet: No action for this Flag" );
    SfxItemSet aRefItemSet( *rRefItemSet.GetPool(), rRefItemSet.GetRanges() );
    aRefItemSet.Set( rRefItemSet );

    // compare with the Attr-Set of the template
    SfxWhichIter aIter( rItemSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        const SfxPoolItem *pRefItem, *pItem;
        bool bItemSet = ( SfxItemState::SET ==
                aIter.GetItemState( false, &pItem) );
        bool bRefItemSet;

        if( pRefScriptItemSet )
        {
            switch( nWhich )
            {
            case RES_CHRATR_FONT:
            case RES_CHRATR_FONTSIZE:
            case RES_CHRATR_LANGUAGE:
            case RES_CHRATR_POSTURE:
            case RES_CHRATR_WEIGHT:
            case RES_CHRATR_CJK_FONT:
            case RES_CHRATR_CJK_FONTSIZE:
            case RES_CHRATR_CJK_LANGUAGE:
            case RES_CHRATR_CJK_POSTURE:
            case RES_CHRATR_CJK_WEIGHT:
            case RES_CHRATR_CTL_FONT:
            case RES_CHRATR_CTL_FONTSIZE:
            case RES_CHRATR_CTL_LANGUAGE:
            case RES_CHRATR_CTL_POSTURE:
            case RES_CHRATR_CTL_WEIGHT:
                bRefItemSet = ( SfxItemState::SET ==
                    pRefScriptItemSet->GetItemState( nWhich, true, &pRefItem) );
                break;
            default:
                bRefItemSet = ( SfxItemState::SET ==
                    aRefItemSet.GetItemState( nWhich, false, &pRefItem) );
                break;
            }
        }
        else
        {
            bRefItemSet = ( SfxItemState::SET ==
                aRefItemSet.GetItemState( nWhich, false, &pRefItem) );
        }

        if( bItemSet )
        {
            if( (bClearSame || pRefScriptItemSet) && bRefItemSet &&
                ( *pItem == *pRefItem ||
                  ((RES_CHRATR_FONT == nWhich ||
                    RES_CHRATR_CJK_FONT == nWhich ||
                    RES_CHRATR_CTL_FONT == nWhich)  &&
                   swhtml_css1atr_equalFontItems( *pItem, *pRefItem )) ) )
            {
                // the Attribute is in both templates with the same value
                // and does not need to be exported
                rItemSet.ClearItem( nWhich );
            }
        }
        else
        {
            if( (bSetDefaults || pRefScriptItemSet) && bRefItemSet )
            {
                // the Attribute exists only in the reference; the default
                // might have to be exported
                rItemSet.Put( rItemSet.GetPool()->GetUserOrPoolDefaultItem(nWhich) );
            }
        }

        nWhich = aIter.NextWhich();
    }
}

void SwHTMLWriter::PrepareFontList( const SvxFontItem& rFontItem,
                                    OUString& rNames,
                                    sal_Unicode cQuote, bool bGeneric )
{
    rNames.clear();
    const OUString& rName = rFontItem.GetFamilyName();
    bool bContainsKeyword = false;
    if( !rName.isEmpty() )
    {
        sal_Int32 nStrPos = 0;
        while( nStrPos != -1 )
        {
            OUString aName = rName.getToken( 0, ';', nStrPos );
            aName = comphelper::string::encodeForXml(comphelper::string::strip(aName, ' '));
            if( aName.isEmpty() )
                continue;

            bool bIsKeyword = false;
            switch( aName[0] )
            {
            case 'c':
            case 'C':
                bIsKeyword = aName.equalsIgnoreAsciiCaseAscii( sCSS1_PV_cursive );
                break;

            case 'f':
            case 'F':
                bIsKeyword = aName.equalsIgnoreAsciiCaseAscii( sCSS1_PV_fantasy );
                break;

            case 'm':
            case 'M':
                bIsKeyword = aName.equalsIgnoreAsciiCaseAscii( sCSS1_PV_monospace );
                break;

            case 's':
            case 'S':
                bIsKeyword =
                    aName.equalsIgnoreAsciiCaseAscii( sCSS1_PV_serif ) ||
                    aName.equalsIgnoreAsciiCaseAscii( sCSS1_PV_sans_serif );
                break;
            }

            bContainsKeyword |= bIsKeyword;

            if( !rNames.isEmpty() )
                rNames += ", ";
            if( cQuote && !bIsKeyword )
                rNames += OUStringChar( cQuote );
            rNames += aName;
            if( cQuote && !bIsKeyword )
                rNames += OUStringChar( cQuote );
        }
    }

    if( bContainsKeyword || !bGeneric )
        return;

    std::string_view pStr;
    switch( rFontItem.GetFamily() )
    {
    case FAMILY_ROMAN:      pStr = sCSS1_PV_serif;      break;
    case FAMILY_SWISS:      pStr = sCSS1_PV_sans_serif; break;
    case FAMILY_SCRIPT:     pStr = sCSS1_PV_cursive;    break;
    case FAMILY_DECORATIVE: pStr = sCSS1_PV_fantasy;    break;
    case FAMILY_MODERN:     pStr = sCSS1_PV_monospace;  break;
    default:
        ;
    }

    if( !pStr.empty() )
    {
        if( !rNames.isEmpty() )
            rNames += ", ";
        rNames += OStringToOUString( pStr, RTL_TEXTENCODING_ASCII_US );
    }
}

bool SwHTMLWriter::HasScriptDependentItems( const SfxItemSet& rItemSet,
                                                 bool bCheckDropCap )
{
    static const sal_uInt16 aWhichIds[] =
    {
        RES_CHRATR_FONT,        RES_CHRATR_CJK_FONT,        RES_CHRATR_CTL_FONT,
        RES_CHRATR_FONTSIZE,    RES_CHRATR_CJK_FONTSIZE,    RES_CHRATR_CTL_FONTSIZE,
        RES_CHRATR_LANGUAGE,    RES_CHRATR_CJK_LANGUAGE,    RES_CHRATR_CTL_LANGUAGE,
        RES_CHRATR_POSTURE,     RES_CHRATR_CJK_POSTURE,     RES_CHRATR_CTL_POSTURE,
        RES_CHRATR_WEIGHT,      RES_CHRATR_CJK_WEIGHT,      RES_CHRATR_CTL_WEIGHT,
        0,                      0,                          0
    };

    for( int i=0; aWhichIds[i]; i += 3 )
    {
        const SfxPoolItem *pItem = nullptr, *pItemCJK = nullptr, *pItemCTL = nullptr, *pTmp;
        int nItemCount = 0;
        if( SfxItemState::SET == rItemSet.GetItemState( aWhichIds[i], false,
                                                   &pTmp ) )
        {
            pItem = pTmp;
            nItemCount++;
        }
        if( SfxItemState::SET == rItemSet.GetItemState( aWhichIds[i+1], false,
                                                   &pTmp ) )
        {
            pItemCJK = pTmp;
            nItemCount++;
        }
        if( SfxItemState::SET == rItemSet.GetItemState( aWhichIds[i+2], false,
                                                   &pTmp ) )
        {
            pItemCTL = pTmp;
            nItemCount++;
        }

        // If some of the items are set, but not all, we need script dependent
        // styles
        if( nItemCount > 0 && nItemCount < 3 )
            return true;

        if( 3 == nItemCount )
        {
            // If all items are set, but some of them have different values,
            // we need script dependent styles, too. For font items, we have
            // to take care about their special HTML/CSS1 representation.
            if( RES_CHRATR_FONT == aWhichIds[i] )
            {
                if( !swhtml_css1atr_equalFontItems( *pItem, *pItemCJK ) ||
                    !swhtml_css1atr_equalFontItems( *pItem, *pItemCTL ) ||
                    !swhtml_css1atr_equalFontItems( *pItemCJK, *pItemCTL ) )
                    return true;
            }
            else
            {
                if( *pItem != *pItemCJK ||
                    *pItem != *pItemCTL ||
                    *pItemCJK != *pItemCTL )
                    return true;
            }
        }
    }

    const SwFormatDrop *pDrop;
    if( bCheckDropCap &&
        (pDrop = rItemSet.GetItemIfSet( RES_PARATR_DROP )) )
    {
        const SwCharFormat *pDCCharFormat = pDrop->GetCharFormat();
        if( pDCCharFormat )
        {
            //sequence of (start, end) property ranges we want to
            //query
            SfxItemSetFixed<
                    RES_CHRATR_FONT, RES_CHRATR_FONT,
                    RES_CHRATR_POSTURE, RES_CHRATR_POSTURE,
                    RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT,
                    RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONT,
                    RES_CHRATR_CJK_POSTURE, RES_CHRATR_CTL_FONT,
                    RES_CHRATR_CTL_POSTURE, RES_CHRATR_CTL_WEIGHT>
                 aTstItemSet(*pDCCharFormat->GetAttrSet().GetPool());
            aTstItemSet.Set( pDCCharFormat->GetAttrSet() );
            return HasScriptDependentItems( aTstItemSet, false );
        }
    }

    return false;
}

static bool OutCSS1Rule( SwHTMLWriter& rWrt, const OUString& rSelector,
                    const SfxItemSet& rItemSet, bool bHasClass,
                     bool bCheckForPseudo  )
{
    bool bScriptDependent = false;
    if( SwHTMLWriter::HasScriptDependentItems( rItemSet, bHasClass ) )
    {
        bScriptDependent = true;
        std::u16string_view aSelector( rSelector );

        std::u16string_view aPseudo;
        if( bCheckForPseudo )
        {
            size_t nPos = aSelector.rfind( ':' );
            if( nPos != std::u16string_view::npos )
            {
                aPseudo = aSelector.substr( nPos );
                aSelector =aSelector.substr( 0, nPos );
            }
        }

        if( !bHasClass )
        {
            // If we are exporting styles for a tag we have to export a tag
            // rule for all properties that aren't style dependent and
            // some class rule for the additional style dependent properties
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_NO_SCRIPT|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     &rSelector );
                rWrt.OutCSS1_SfxItemSet( rItemSet, false );
            }

            //sequence of (start, end) property ranges we want to
            //query
            SfxItemSetFixed<RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
                                       RES_CHRATR_LANGUAGE, RES_CHRATR_POSTURE,
                                       RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT,
                                       RES_CHRATR_CJK_FONT, RES_CHRATR_CTL_WEIGHT>
                aScriptItemSet( *rItemSet.GetPool() );
            aScriptItemSet.Put( rItemSet );

            OUString aNewSelector = OUString::Concat(aSelector) + ".western" + aPseudo;
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_WESTERN|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     &aNewSelector );
                rWrt.OutCSS1_SfxItemSet( aScriptItemSet, false );
            }

            aNewSelector = OUString::Concat(aSelector) + ".cjk" + aPseudo;
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_CJK|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     &aNewSelector );
                rWrt.OutCSS1_SfxItemSet( aScriptItemSet, false );
            }

            aNewSelector = OUString::Concat(aSelector) + ".ctl" + aPseudo;
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_CTL|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     &aNewSelector );
                rWrt.OutCSS1_SfxItemSet( aScriptItemSet, false );
            }
        }
        else
        {
            // If there are script dependencies and we are derived from a tag,
            // when we have to export a style dependent class for all
            // scripts
            OUString aNewSelector = OUString::Concat(aSelector) + "-western" + aPseudo;
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_WESTERN|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     &aNewSelector );
                rWrt.OutCSS1_SfxItemSet( rItemSet, false );
            }

            aNewSelector = OUString::Concat(aSelector) + "-cjk" + aPseudo;
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_CJK|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     &aNewSelector );
                rWrt.OutCSS1_SfxItemSet( rItemSet, false );
            }

            aNewSelector = OUString::Concat(aSelector) + "-ctl" + aPseudo;
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_CTL|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     &aNewSelector );
                rWrt.OutCSS1_SfxItemSet( rItemSet, false );
            }
        }
    }
    else
    {
        // If there are no script dependencies, when all items are
        // exported in one step. For hyperlinks only, a script information
        // must be there, because these two chr formats don't support
        // script dependencies by now.
        SwCSS1OutMode aMode( rWrt,
                rWrt.m_nCSS1Script|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                             &rSelector );
        rWrt.OutCSS1_SfxItemSet( rItemSet, false );
    }

    return bScriptDependent;
}

static void OutCSS1DropCapRule(
                    SwHTMLWriter& rWrt, const OUString& rSelector,
                    const SwFormatDrop& rDrop, bool bHasClass,
                     bool bHasScriptDependencies  )
{
    const SwCharFormat *pDCCharFormat = rDrop.GetCharFormat();
    if( (bHasScriptDependencies && bHasClass) ||
         (pDCCharFormat && SwHTMLWriter::HasScriptDependentItems( pDCCharFormat->GetAttrSet(), false ) ) )
    {
        std::u16string_view aSelector( rSelector );

        std::u16string_view aPseudo;
        size_t nPos = aSelector.rfind( ':' );
        if( nPos != std::u16string_view::npos )
        {
            aPseudo = aSelector.substr( nPos );
            aSelector = aSelector.substr( 0, nPos );
        }

        if( !bHasClass )
        {
            // If we are exporting styles for a tag we have to export a tag
            // rule for all properties that aren't style dependent and
            // some class rule for the additional style dependent properties
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_NO_SCRIPT|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     &rSelector );
                OutCSS1_SwFormatDropAttrs( rWrt, rDrop );
            }

            SfxItemSetFixed<RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
                           RES_CHRATR_LANGUAGE, RES_CHRATR_POSTURE,
                           RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT,
                           RES_CHRATR_CJK_FONT, RES_CHRATR_CTL_WEIGHT>
                aScriptItemSet( rWrt.m_pDoc->GetAttrPool() );
            if( pDCCharFormat )
                aScriptItemSet.Set( pDCCharFormat->GetAttrSet() );

            OUString aNewSelector = OUString::Concat(aSelector) + ".western" + aPseudo;
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_WESTERN|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     &aNewSelector );
                OutCSS1_SwFormatDropAttrs(  rWrt, rDrop, &aScriptItemSet );
            }

            aNewSelector = OUString::Concat(aSelector) + ".cjk" + aPseudo;
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_CJK|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     &aNewSelector );
                OutCSS1_SwFormatDropAttrs(  rWrt, rDrop, &aScriptItemSet );
            }

            aNewSelector = OUString::Concat(aSelector) + ".ctl" + aPseudo;
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_CTL|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     &aNewSelector );
                OutCSS1_SwFormatDropAttrs(  rWrt, rDrop, &aScriptItemSet );
            }
        }
        else
        {
            // If there are script dependencies and we are derived from a tag,
            // when we have to export a style dependent class for all
            // scripts
            OUString aNewSelector = OUString::Concat(aSelector) + "-western" + aPseudo;
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_WESTERN|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     &aNewSelector );
                OutCSS1_SwFormatDropAttrs(  rWrt, rDrop );
            }

            aNewSelector = OUString::Concat(aSelector) + "-cjk" + aPseudo;
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_CJK|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     &aNewSelector );
                OutCSS1_SwFormatDropAttrs(  rWrt, rDrop );
            }

            aNewSelector = OUString::Concat(aSelector) + "-ctl" + aPseudo;
            {
                SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_CTL|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     &aNewSelector );
                OutCSS1_SwFormatDropAttrs(  rWrt, rDrop );
            }
        }
    }
    else
    {
        // If there are no script dependencies, when all items are
        // exported in one step. For hyperlinks only, a script information
        // must be there, because these two chr formats don't support
        // script dependencies by now.
        SwCSS1OutMode aMode( rWrt,
                rWrt.m_nCSS1Script|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                             &rSelector );
        OutCSS1_SwFormatDropAttrs( rWrt, rDrop );
    }
}

static SwHTMLWriter& OutCSS1_SwFormat( SwHTMLWriter& rWrt, const SwFormat& rFormat,
                              IDocumentStylePoolAccess/*SwDoc*/ *pDoc, SwDoc *pTemplate )
{
    bool bCharFormat = false;
    switch( rFormat.Which() )
    {
    case RES_CHRFMT:
        bCharFormat = true;
        break;

    case RES_TXTFMTCOLL:
    case RES_CONDTXTFMTCOLL:
        // these template-types can be exported
        break;

    default:
        // but not these
        return rWrt;
    }

    // determine Selector and the to-be-exported Attr-Set-depth
    OUString aSelector;
    sal_uInt16 nRefPoolId = 0;
    sal_uInt16 nDeep = GetCSS1Selector( &rFormat, aSelector, nRefPoolId );
    if( !nDeep )
        return rWrt;    // not derived from a HTML-template

    sal_uInt16 nPoolFormatId = rFormat.GetPoolFormatId();

    // Determine the to-be-exported Attr-Set. We have to distinguish 3 cases:
    // - HTML-Tag templates (nDeep==USHRT_MAX):
    //   Export Attrs...
    //     - that are set in the template, but not in the original of the HTML template
    //     - the Default-Attrs for the Attrs, that are set in the Original of the
    //       HTML template, but not in the current template.
    // - templates directly derived from HTML templates (nDeep==1):
    //   Export only Attributes of the template Item-Set w/o its parents.
    // - templates in-directly derived from HTML templates (nDeep>1):
    //   Export Attributes of the template Item-Set incl. its Parents,
    //   but w/o Attributes that are set in the HTML-Tag template.

    // create Item-Set with all Attributes from the template
    // (all but for nDeep==1)
    const SfxItemSet& rFormatItemSet = rFormat.GetAttrSet();
    SfxItemSet aItemSet( *rFormatItemSet.GetPool(), rFormatItemSet.GetRanges() );
    aItemSet.Set( rFormatItemSet ); // Was nDeep!=1 that is not working
                                    // for script dependent items but should
                                    // not make a difference for any other

    bool bSetDefaults = true, bClearSame = true;
    const SwFormat *pRefFormat = nullptr;
    const SwFormat *pRefFormatScript = nullptr;
    switch( nDeep )
    {
    case CSS1_FMT_ISTAG:
        pRefFormat = SwHTMLWriter::GetTemplateFormat( nRefPoolId, pTemplate == nullptr ? nullptr : &pTemplate->getIDocumentStylePoolAccess() );
        break;
    case CSS1_FMT_CMPREF:
        pRefFormat = SwHTMLWriter::GetTemplateFormat( nRefPoolId, pDoc );
        pRefFormatScript = SwHTMLWriter::GetTemplateFormat( nRefPoolId, pTemplate == nullptr ? nullptr : &pTemplate->getIDocumentStylePoolAccess() );
        bClearSame = false;
        break;
    default:
        pRefFormat = SwHTMLWriter::GetParentFormat( rFormat, nDeep );
        pRefFormatScript = SwHTMLWriter::GetTemplateFormat( nRefPoolId, pTemplate == nullptr ? nullptr : &pTemplate->getIDocumentStylePoolAccess() );
        bSetDefaults = false;
        break;
    }

    if( pRefFormat )
    {
        // subtract Item-Set of the Reference template (incl. its Parents)
        SwHTMLWriter::SubtractItemSet( aItemSet, pRefFormat->GetAttrSet(),
                                       bSetDefaults, bClearSame,
                                       pRefFormatScript
                                               ? &pRefFormatScript->GetAttrSet()
                                            : nullptr  );

        if( !bCharFormat )
        {
            const SvxULSpaceItem& rULItem = pRefFormat->GetULSpace();
            rWrt.m_nDfltTopMargin = rULItem.GetUpper();
            rWrt.m_nDfltBottomMargin = rULItem.GetLower();
        }
    }
    else if( CSS1_FMT_ISTAG==nDeep && !bCharFormat )
    {
        // set Default-distance above and below (for the
        // case that there is no reference template)
        rWrt.m_nDfltTopMargin = 0;
        rWrt.m_nDfltBottomMargin = HTML_PARSPACE;
        if( USER_FMT & nPoolFormatId )
        {
            // user templates
            const UIName& aNm(rFormat.GetName());

            if (aNm == "DD 1" || aNm == "DT 1")
                rWrt.m_nDfltBottomMargin = 0;
            else if (aNm == OOO_STRING_SVTOOLS_HTML_listing)
                rWrt.m_nDfltBottomMargin = 0;
            else if (aNm == OOO_STRING_SVTOOLS_HTML_preformtxt)
                rWrt.m_nDfltBottomMargin = 0;
            else if (aNm == OOO_STRING_SVTOOLS_HTML_xmp)
                rWrt.m_nDfltBottomMargin = 0;
        }
        else
        {
            // Pool templates
            switch( nPoolFormatId )
            {
            case RES_POOLCOLL_HEADLINE1:
            case RES_POOLCOLL_HEADLINE2:
            case RES_POOLCOLL_HEADLINE3:
            case RES_POOLCOLL_HEADLINE4:
            case RES_POOLCOLL_HEADLINE5:
            case RES_POOLCOLL_HEADLINE6:
                rWrt.m_nDfltTopMargin = HTML_HEADSPACE;
                break;
            case RES_POOLCOLL_SEND_ADDRESS:
            case RES_POOLCOLL_HTML_DT:
            case RES_POOLCOLL_HTML_DD:
            case RES_POOLCOLL_HTML_PRE:
                rWrt.m_nDfltBottomMargin = 0;
                break;
            }
        }
    }

    // if nothing is to be exported ...
    if( !aItemSet.Count() )
        return rWrt;

    // There is no support for script dependent hyperlinks by now.
    bool bCheckForPseudo = false;
    if( bCharFormat &&
        (RES_POOLCHR_INET_NORMAL==nRefPoolId ||
         RES_POOLCHR_INET_VISIT==nRefPoolId) )
        bCheckForPseudo = true;

    // export now the Attributes (incl. selector)
    bool bHasScriptDependencies = false;
    if( OutCSS1Rule( rWrt, aSelector, aItemSet, CSS1_FMT_ISTAG != nDeep,
                      bCheckForPseudo ) )
    {
        if( bCharFormat )
            rWrt.m_aScriptTextStyles.insert( rFormat.GetName() );
        else
        {
            if( nPoolFormatId==RES_POOLCOLL_TEXT )
                rWrt.m_aScriptParaStyles.insert( pDoc->GetTextCollFromPool( RES_POOLCOLL_STANDARD, false )->GetName().toString() );
            rWrt.m_aScriptParaStyles.insert( rFormat.GetName().toString() );
        }
        bHasScriptDependencies = true;
    }

    // export Drop-Caps
    if( const SwFormatDrop *pDrop = aItemSet.GetItemIfSet( RES_PARATR_DROP, false ) )
    {
        OUString sOut = aSelector + ":" + sCSS1_first_letter;
        OutCSS1DropCapRule( rWrt, sOut, *pDrop, CSS1_FMT_ISTAG != nDeep, bHasScriptDependencies );
    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SwPageDesc( SwHTMLWriter& rWrt, const SwPageDesc& rPageDesc,
                                   IDocumentStylePoolAccess/*SwDoc*/ *pDoc, SwDoc *pTemplate,
                                   sal_uInt16 nRefPoolId, bool bExtRef,
                                   bool bPseudo )
{
    const SwPageDesc* pRefPageDesc = nullptr;
    if( !bExtRef )
        pRefPageDesc = pDoc->GetPageDescFromPool( nRefPoolId, false );
    else if( pTemplate )
        pRefPageDesc = pTemplate->getIDocumentStylePoolAccess().GetPageDescFromPool( nRefPoolId, false );

    OUString aSelector = OUString::Concat("@") + sCSS1_page;

    if( bPseudo )
    {
        std::u16string_view pPseudo;
        switch( rPageDesc.GetPoolFormatId() )
        {
        case RES_POOLPAGE_FIRST:    pPseudo = sCSS1_first;  break;
        case RES_POOLPAGE_LEFT:     pPseudo = sCSS1_left;   break;
        case RES_POOLPAGE_RIGHT:    pPseudo = sCSS1_right;  break;
        }
        if( !pPseudo.empty() )
            aSelector += OUString::Concat(":") + pPseudo;
    }

    SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_RULE_ON|CSS1_OUTMODE_TEMPLATE,
                         &aSelector );

    // Size: If the only difference is the Landscape-Flag,
    // only export Portrait or Landscape. Otherwise export size.
    bool bRefLandscape = pRefPageDesc && pRefPageDesc->GetLandscape();
    Size aRefSz;
    const Size& rSz = rPageDesc.GetMaster().GetFrameSize().GetSize();
    if( pRefPageDesc )
    {
        aRefSz = pRefPageDesc->GetMaster().GetFrameSize().GetSize();
        if( bRefLandscape != rPageDesc.GetLandscape() )
        {
            tools::Long nTmp = aRefSz.Height();
            aRefSz.setHeight( aRefSz.Width() );
            aRefSz.setWidth( nTmp );
        }
    }

    // TODO: Bad Hack: On the Page-Tabpage there are small rounding errors
    // for the page size. Partially because of bug 25535, we stupidly still
    // use the Size-Item from Dialog, even if nothing changed.
    // Thus: once one visited the Page-Dialog and left it with OK, we get a
    // new page size that then gets exported here. To avoid that, we allow
    // here small deviations.
    if( std::abs( rSz.Width() - aRefSz.Width() ) <= 2 &&
        std::abs( rSz.Height() - aRefSz.Height() ) <= 2 )
    {
        if( bRefLandscape != rPageDesc.GetLandscape() )
        {
            rWrt.OutCSS1_PropertyAscii( sCSS1_P_size,
                rPageDesc.GetLandscape() ? sCSS1_PV_landscape
                                         : sCSS1_PV_portrait );
        }
    }
    else
    {
        OStringBuffer sVal;
        AddUnitPropertyValue(sVal, rSz.Width(), rWrt.GetCSS1Unit());
        sVal.append(' ');
        AddUnitPropertyValue(sVal, rSz.Height(), rWrt.GetCSS1Unit());
        rWrt.OutCSS1_PropertyAscii(sCSS1_P_size, sVal);
    }

    // Export the distance-Attributes as normally
    const SwFrameFormat &rMaster = rPageDesc.GetMaster();
    SfxItemSetFixed<RES_LR_SPACE, RES_UL_SPACE> aItemSet( *rMaster.GetAttrSet().GetPool() );
    aItemSet.Set( rMaster.GetAttrSet() );

    if( pRefPageDesc )
    {
        SwHTMLWriter::SubtractItemSet( aItemSet,
                                       pRefPageDesc->GetMaster().GetAttrSet(),
                                       true );
    }

    OutCSS1_SvxULSpace_SvxLRSpace( rWrt, aItemSet );

    // If for a Pseudo-Selector no Property had been set, we still
    // have to export something, so that the corresponding template is
    // created on the next import.
    if( rWrt.m_bFirstCSS1Property && bPseudo )
    {
        rWrt.OutNewLine();
        OString sTmp(OUStringToOString(aSelector, RTL_TEXTENCODING_UTF8));
        rWrt.Strm().WriteOString( sTmp ).WriteOString( " {" );
        rWrt.m_bFirstCSS1Property = false;
    }

    if( !rWrt.m_bFirstCSS1Property )
        rWrt.Strm().WriteOString( sCSS1_rule_end );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SwFootnoteInfo( SwHTMLWriter& rWrt, const SwEndNoteInfo& rInfo,
                                  SwDoc *pDoc, bool bHasNotes, bool bEndNote )
{
    OUString aSelector;

    if( bHasNotes )
    {
        aSelector = OUString::Concat(OOO_STRING_SVTOOLS_HTML_anchor ".") +
                    ( bEndNote ? std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_sdendnote_anc)
                               : std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_sdfootnote_anc) );
        SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                             &aSelector );
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_font_size,
                                        sHTML_FTN_fontheight );
        rWrt.Strm().WriteOString( sCSS1_rule_end );
    }

    const SwCharFormat *pSymCharFormat = rInfo.GetCharFormat( *pDoc );
    if( pSymCharFormat )
    {
        const SfxItemSet& rFormatItemSet = pSymCharFormat->GetAttrSet();
        SfxItemSet aItemSet( *rFormatItemSet.GetPool(), rFormatItemSet.GetRanges() );
        aItemSet.Set( rFormatItemSet );

        // If there are footnotes or endnotes, then all Attributes have to be
        // exported, so that Netscape displays the document correctly.
        // Otherwise it is sufficient, to export the differences to the
        // footnote and endnote template.
        if( !bHasNotes && rWrt.m_xTemplate.is() )
        {
            SwFormat *pRefFormat = rWrt.m_xTemplate->getIDocumentStylePoolAccess().GetCharFormatFromPool(
                        static_cast< sal_uInt16 >(bEndNote ? RES_POOLCHR_ENDNOTE : RES_POOLCHR_FOOTNOTE) );
            if( pRefFormat )
                SwHTMLWriter::SubtractItemSet( aItemSet, pRefFormat->GetAttrSet(),
                                               true );
        }
        if( aItemSet.Count() )
        {
            aSelector = OUString::Concat(OOO_STRING_SVTOOLS_HTML_anchor ".") +
                        ( bEndNote ? std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_sdendnote_sym)
                                   : std::u16string_view(
                                       u"" OOO_STRING_SVTOOLS_HTML_sdfootnote_sym));
            if( OutCSS1Rule( rWrt, aSelector, aItemSet, true, false ))
                rWrt.m_aScriptTextStyles.insert( pSymCharFormat->GetName() );
        }
    }

    return rWrt;
}

SwHTMLWriter& OutCSS1_BodyTagStyleOpt( SwHTMLWriter& rWrt, const SfxItemSet& rItemSet )
{
    SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_STYLE_OPT_ON |
                                   CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_BODY, nullptr );

    // Only export the attributes of the page template.
    // The attributes of the default paragraph template were
    // considered already when exporting the paragraph template.

    const SfxPoolItem *pItem;
    if( SfxItemState::SET == rItemSet.GetItemState( RES_BACKGROUND, false,
                                               &pItem ) )
    {
        OUString rEmbeddedGraphicName;
        OutCSS1_SvxBrush( rWrt, *pItem, sw::Css1Background::Page, &rEmbeddedGraphicName );
    }

    if( SfxItemState::SET == rItemSet.GetItemState( RES_BOX, false,
                                               &pItem ))
    {
        OutCSS1_SvxBox( rWrt, *pItem );
    }

    if( !rWrt.m_bFirstCSS1Property )
    {
        // if a Property was exported as part of a Style-Option,
        // the Option still needs to be finished
        rWrt.Strm().WriteChar( '\"' );
    }

    return rWrt;
}

SwHTMLWriter& OutCSS1_ParaTagStyleOpt( SwHTMLWriter& rWrt, const SfxItemSet& rItemSet, std::string_view rAdd )
{
    SwCSS1OutMode aMode( rWrt, rWrt.m_nCSS1Script|CSS1_OUTMODE_STYLE_OPT |
                                   CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_PARA, nullptr );
    rWrt.OutCSS1_SfxItemSet( rItemSet, false, rAdd );

    return rWrt;
}

SwHTMLWriter& OutCSS1_TableBGStyleOpt( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_STYLE_OPT_ON |
                                   CSS1_OUTMODE_ENCODE|
                                   CSS1_OUTMODE_TABLEBOX, nullptr );
    OutCSS1_SvxBrush( rWrt, rHt, sw::Css1Background::TableRow, nullptr );

    if (!rWrt.m_bFirstCSS1Property)
        rWrt.Strm().WriteChar(cCSS1_style_opt_end);

    return rWrt;
}

SwHTMLWriter& OutCSS1_NumberBulletListStyleOpt( SwHTMLWriter& rWrt, const SwNumRule& rNumRule,
                                    sal_uInt8 nLevel )
{
    SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_STYLE_OPT |
                                   CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_PARA, nullptr );

    const SwNumFormat& rNumFormat = rNumRule.Get( nLevel );

    tools::Long nLSpace = rNumFormat.GetAbsLSpace();
    tools::Long nFirstLineOffset = rNumFormat.GetFirstLineOffset();
    tools::Long nDfltFirstLineOffset = HTML_NUMBER_BULLET_INDENT;
    if( nLevel > 0 )
    {
        const SwNumFormat& rPrevNumFormat = rNumRule.Get( nLevel-1 );
        nLSpace -= rPrevNumFormat.GetAbsLSpace();
        nDfltFirstLineOffset = rPrevNumFormat.GetFirstLineOffset();
    }

    if( rWrt.IsHTMLMode(HTMLMODE_LSPACE_IN_NUMBER_BULLET) &&
        nLSpace != HTML_NUMBER_BULLET_MARGINLEFT )
        rWrt.OutCSS1_UnitProperty( sCSS1_P_margin_left, nLSpace );

    if( rWrt.IsHTMLMode(HTMLMODE_FRSTLINE_IN_NUMBER_BULLET) &&
        nFirstLineOffset != nDfltFirstLineOffset )
        rWrt.OutCSS1_UnitProperty( sCSS1_P_text_indent, nFirstLineOffset );

    if( !rWrt.m_bFirstCSS1Property )
        rWrt.Strm().WriteChar( '\"' );

    return rWrt;
}

void SwHTMLWriter::OutCSS1_FrameFormatOptions( const SwFrameFormat& rFrameFormat,
                                          HtmlFrmOpts nFrameOpts,
                                          const SdrObject *pSdrObj,
                                          const SfxItemSet *pItemSet )
{
    SwCSS1OutMode aMode( *this, CSS1_OUTMODE_STYLE_OPT_ON |
                                CSS1_OUTMODE_ENCODE|
                                CSS1_OUTMODE_FRAME, nullptr );

    const SwFormatHoriOrient& rHoriOri = rFrameFormat.GetHoriOrient();
    SvxLRSpaceItem aLRItem( rFrameFormat.GetLRSpace() );
    SvxULSpaceItem aULItem( rFrameFormat.GetULSpace() );
    if( nFrameOpts & HtmlFrmOpts::SAlign )
    {
        const SwFormatAnchor& rAnchor = rFrameFormat.GetAnchor();
        switch( rAnchor.GetAnchorId() )
        {
        case RndStdIds::FLY_AT_PARA:
        case RndStdIds::FLY_AT_CHAR:
            if( text::RelOrientation::FRAME == rHoriOri.GetRelationOrient() ||
                text::RelOrientation::PRINT_AREA == rHoriOri.GetRelationOrient() )
            {
                if( !(nFrameOpts & HtmlFrmOpts::Align) )
                {
                    // float
                    std::string_view pStr = text::HoriOrientation::RIGHT==rHoriOri.GetHoriOrient()
                            ? sCSS1_PV_right
                            : sCSS1_PV_left;
                    OutCSS1_PropertyAscii( sCSS1_P_float, pStr );
                }
                break;
            }
            [[fallthrough]];

        case RndStdIds::FLY_AT_PAGE:
        case RndStdIds::FLY_AT_FLY:
            {
                // position
                OutCSS1_PropertyAscii( sCSS1_P_position, sCSS1_PV_absolute );

                // For top/left we need to subtract the distance to the frame
                // from the position, as in CSS1 it is added to the position.
                // This works also for automatically aligned frames, even that
                // in this case Writer also adds the distance; because in this
                // case the Orient-Attribute contains the correct position.

                // top
                tools::Long nXPos=0, nYPos=0;
                bool bOutXPos = false, bOutYPos = false;
                if( RES_DRAWFRMFMT == rFrameFormat.Which() )
                {
                    OSL_ENSURE( pSdrObj, "Do not pass a SdrObject. Inefficient" );
                    if( !pSdrObj )
                        pSdrObj = rFrameFormat.FindSdrObject();
                    OSL_ENSURE( pSdrObj, "Where is the SdrObject" );
                    if( pSdrObj )
                    {
                        Point aPos( pSdrObj->GetRelativePos() );
                        nXPos = aPos.X();
                        nYPos = aPos.Y();
                    }
                    bOutXPos = bOutYPos = true;
                }
                else
                {
                    bOutXPos = text::RelOrientation::CHAR != rHoriOri.GetRelationOrient();
                    nXPos = text::HoriOrientation::NONE == rHoriOri.GetHoriOrient()
                                ? rHoriOri.GetPos() : 0;

                    const SwFormatVertOrient& rVertOri = rFrameFormat.GetVertOrient();
                    bOutYPos = text::RelOrientation::CHAR != rVertOri.GetRelationOrient();
                    nYPos = text::VertOrientation::NONE == rVertOri.GetVertOrient()
                                 ? rVertOri.GetPos() : 0;
                }

                if( bOutYPos )
                {
                    if( IsHTMLMode( HTMLMODE_FLY_MARGINS) )
                    {
                        nYPos -= aULItem.GetUpper();
                        if( nYPos < 0 )
                        {
                            aULItem.SetUpper( o3tl::narrowing<sal_uInt16>(aULItem.GetUpper() + nYPos) );
                            nYPos = 0;
                        }
                    }

                    OutCSS1_UnitProperty( sCSS1_P_top, nYPos );
                }

                if( bOutXPos )
                {
                    // left
                    if( IsHTMLMode( HTMLMODE_FLY_MARGINS) )
                    {
                        nXPos -= aLRItem.ResolveLeft({});
                        if( nXPos < 0 )
                        {
                            aLRItem.SetLeft(SvxIndentValue::twips(
                                o3tl::narrowing<sal_uInt16>(aLRItem.ResolveLeft({}) + nXPos)));
                            nXPos = 0;
                        }
                    }

                    OutCSS1_UnitProperty( sCSS1_P_left, nXPos );
                }
            }
            break;

        default:
            ;
        }
    }

    // width/height
    if( nFrameOpts & HtmlFrmOpts::SSize )
    {
        if( RES_DRAWFRMFMT == rFrameFormat.Which() )
        {
            OSL_ENSURE( pSdrObj, "Do not pass a SdrObject. Inefficient" );
            if( !pSdrObj )
                pSdrObj = rFrameFormat.FindSdrObject();
            OSL_ENSURE( pSdrObj, "Where is the SdrObject" );
            if( pSdrObj )
            {
                Size aTwipSz( pSdrObj->GetLogicRect().GetSize() );
                if( nFrameOpts & HtmlFrmOpts::SWidth )
                {
                    if( nFrameOpts & HtmlFrmOpts::SPixSize )
                        OutCSS1_PixelProperty( sCSS1_P_width, aTwipSz.Width() );
                    else
                        OutCSS1_UnitProperty( sCSS1_P_width, aTwipSz.Width() );
                }
                if( nFrameOpts & HtmlFrmOpts::SHeight )
                {
                    if( nFrameOpts & HtmlFrmOpts::SPixSize )
                        OutCSS1_PixelProperty( sCSS1_P_height, aTwipSz.Height() );
                    else
                        OutCSS1_UnitProperty( sCSS1_P_height, aTwipSz.Height() );
                }
            }
        }
        else
        {
            OSL_ENSURE( HtmlFrmOpts::AbsSize & nFrameOpts,
                    "Export absolute size" );
            OSL_ENSURE( HtmlFrmOpts::AnySize & nFrameOpts,
                    "Export every size" );
            Css1FrameSize nMode = Css1FrameSize::NONE;
            if( nFrameOpts & HtmlFrmOpts::SWidth )
                nMode |= Css1FrameSize::Width;
            if( nFrameOpts & HtmlFrmOpts::SHeight )
                nMode |= Css1FrameSize::MinHeight|Css1FrameSize::FixHeight;
            if( nFrameOpts & HtmlFrmOpts::SPixSize )
                nMode |= Css1FrameSize::Pixel;

            OutCSS1_SwFormatFrameSize( *this, rFrameFormat.GetFrameSize(), nMode );
        }
    }

    const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();
    // margin-*
    if( (nFrameOpts & HtmlFrmOpts::SSpace) &&
        IsHTMLMode( HTMLMODE_FLY_MARGINS) )
    {
        const SvxLRSpaceItem *pLRItem = nullptr;
        const SvxULSpaceItem *pULItem = nullptr;
        if( SfxItemState::SET == rItemSet.GetItemState( RES_LR_SPACE ) )
            pLRItem = &aLRItem;
        if( SfxItemState::SET == rItemSet.GetItemState( RES_UL_SPACE ) )
            pULItem = &aULItem;
        if( pLRItem || pULItem )
            OutCSS1_SvxULSpace_SvxLRSpace( *this, pULItem, pLRItem );
    }

    // border
    if( nFrameOpts & HtmlFrmOpts::SBorder )
    {
        const SfxPoolItem* pItem;
        if( nFrameOpts & HtmlFrmOpts::SNoBorder )
            OutCSS1_SvxBox( *this, rFrameFormat.GetBox() );
        else if( SfxItemState::SET==rItemSet.GetItemState( RES_BOX, true, &pItem ) )
            OutCSS1_SvxBox( *this, *pItem );
    }

    // background (if, then the color must be set also)
    if( nFrameOpts & HtmlFrmOpts::SBackground )
        OutCSS1_FrameFormatBackground( rFrameFormat );

    if( pItemSet )
        OutCSS1_SfxItemSet( *pItemSet, false );

    if( !m_bFirstCSS1Property )
        Strm().WriteChar( '\"' );
}

void SwHTMLWriter::OutCSS1_TableFrameFormatOptions( const SwFrameFormat& rFrameFormat )
{
    SwCSS1OutMode aMode( *this, CSS1_OUTMODE_STYLE_OPT_ON |
                                CSS1_OUTMODE_ENCODE|
                                CSS1_OUTMODE_TABLE, nullptr );

    const SfxPoolItem *pItem;
    const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();
    if( SfxItemState::SET==rItemSet.GetItemState( RES_BACKGROUND, false, &pItem ) )
        OutCSS1_SvxBrush( *this, *pItem, sw::Css1Background::Table, nullptr );

    if( IsHTMLMode( HTMLMODE_PRINT_EXT ) )
        OutCSS1_SvxFormatBreak_SwFormatPDesc_SvxFormatKeep( *this, rItemSet, false );

    if( SfxItemState::SET==rItemSet.GetItemState( RES_LAYOUT_SPLIT, false, &pItem ) )
        OutCSS1_SwFormatLayoutSplit( *this, *pItem );

    if (mbXHTML)
    {
        sal_Int16 eTabHoriOri = rFrameFormat.GetHoriOrient().GetHoriOrient();
        if (eTabHoriOri == text::HoriOrientation::CENTER)
        {
            // Emit XHTML's center using inline CSS.
            OutCSS1_Property(sCSS1_P_margin_left, "auto", nullptr, sw::Css1Background::Table);
            OutCSS1_Property(sCSS1_P_margin_right, "auto", nullptr, sw::Css1Background::Table);
        }
    }

    if( !m_bFirstCSS1Property )
        Strm().WriteChar( '\"' );
}

void SwHTMLWriter::OutCSS1_TableCellBordersAndBG(SwFrameFormat const& rFrameFormat, const SvxBrushItem *pBrushItem)
{
    SwCSS1OutMode const aMode( *this,
        CSS1_OUTMODE_STYLE_OPT_ON|CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_TABLEBOX, nullptr );
    if (pBrushItem)
        OutCSS1_SvxBrush(*this, *pBrushItem, sw::Css1Background::TableCell, nullptr);
    OutCSS1_SvxBox(*this, rFrameFormat.GetBox());
    if (!m_bFirstCSS1Property)
        Strm().WriteChar(cCSS1_style_opt_end);
}

void SwHTMLWriter::OutCSS1_SectionFormatOptions( const SwFrameFormat& rFrameFormat, const SwFormatCol *pCol )
{
    SwCSS1OutMode aMode( *this, CSS1_OUTMODE_STYLE_OPT_ON |
                                CSS1_OUTMODE_ENCODE|
                                CSS1_OUTMODE_SECTION, nullptr );

    const SfxPoolItem *pItem;
    const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();
    if( SfxItemState::SET==rItemSet.GetItemState( RES_BACKGROUND, false, &pItem ) )
        OutCSS1_SvxBrush( *this, *pItem, sw::Css1Background::Section, nullptr );

    if (mbXHTML)
    {
        SvxFrameDirection nDir = GetHTMLDirection(rFrameFormat.GetAttrSet());
        OString sConvertedDirection = convertDirection(nDir);
        if (!sConvertedDirection.isEmpty())
        {
            OutCSS1_Property(sCSS1_P_dir, sConvertedDirection, nullptr,
                             sw::Css1Background::Section);
        }
    }

    if (pCol)
    {
        OString sColumnCount(OString::number(static_cast<sal_Int32>(pCol->GetNumCols())));
        OutCSS1_PropertyAscii(sCSS1_P_column_count, sColumnCount);
    }

    if( !m_bFirstCSS1Property )
        Strm().WriteChar( '\"' );
}

static bool OutCSS1_FrameFormatBrush( SwHTMLWriter& rWrt,
                                 const SvxBrushItem& rBrushItem )
{
    bool bWritten = false;
    /// output brush of frame format, if its background color is not "no fill"/"auto fill"
    /// or it has a background graphic.
    if( rBrushItem.GetColor() != COL_TRANSPARENT ||
        !rBrushItem.GetGraphicLink().isEmpty() ||
        0 != rBrushItem.GetGraphicPos() )
    {
        OutCSS1_SvxBrush( rWrt, rBrushItem, sw::Css1Background::Fly, nullptr );
        bWritten = true;
    }
    return bWritten;
}

void SwHTMLWriter::OutCSS1_FrameFormatBackground( const SwFrameFormat& rFrameFormat )
{
    // If the frame itself has a background, then export.
    if( OutCSS1_FrameFormatBrush( *this, *rFrameFormat.makeBackgroundBrushItem() ) )
        return;

    // If the frame is not linked to a page, we use the background of the anchor.
    const SwFormatAnchor& rAnchor = rFrameFormat.GetAnchor();
    RndStdIds eAnchorId = rAnchor.GetAnchorId();
    const SwNode *pAnchorNode = rAnchor.GetAnchorNode();
    if (RndStdIds::FLY_AT_PAGE != eAnchorId && pAnchorNode)
    {
        if( pAnchorNode->IsContentNode() )
        {
            // If the frame is linked to a content-node,
            // we take the background of the content-node, if it has one.
            if( OutCSS1_FrameFormatBrush( *this,
                    pAnchorNode->GetContentNode()->GetSwAttrSet().GetBackground()) )
                return;

            // Otherwise we also could be in a table
            const SwTableNode *pTableNd = pAnchorNode->FindTableNode();
            if( pTableNd )
            {
                const SwStartNode *pBoxSttNd = pAnchorNode->FindTableBoxStartNode();
                const SwTableBox *pBox =
                    pTableNd->GetTable().GetTableBox( pBoxSttNd->GetIndex() );

                // If the box has a background, we take it.
                if( OutCSS1_FrameFormatBrush( *this,
                        *pBox->GetFrameFormat()->makeBackgroundBrushItem() ) )
                    return;

                // Otherwise we use that of the lines
                const SwTableLine *pLine = pBox->GetUpper();
                while( pLine )
                {
                    if( OutCSS1_FrameFormatBrush( *this,
                            *pLine->GetFrameFormat()->makeBackgroundBrushItem() ) )
                        return;
                    pBox = pLine->GetUpper();
                    pLine = pBox ? pBox->GetUpper() : nullptr;
                }

                // If there was none either, we use the background of the table.
                if( OutCSS1_FrameFormatBrush( *this,
                        *pTableNd->GetTable().GetFrameFormat()->makeBackgroundBrushItem() ) )
                    return;
            }

        }

        // If the anchor is again in a Fly-Frame, use the background of the Fly-Frame.
        const SwFrameFormat *pFrameFormat = pAnchorNode->GetFlyFormat();
        if( pFrameFormat )
        {
            OutCSS1_FrameFormatBackground( *pFrameFormat );
            return;
        }
    }

    // At last there is the background of the page, and as the final rescue
    // the value of the Config.
    assert(m_pCurrPageDesc && "no page template found");
    if( OutCSS1_FrameFormatBrush( *this,
                              *m_pCurrPageDesc->GetMaster().makeBackgroundBrushItem() ) )
        return;

    Color aColor( COL_WHITE );

    // The background color is normally only used in Browse-Mode.
    // We always use it for a HTML document, but for a text document
    // only if viewed in Browse-Mode.
    if( m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) ||
        m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE))
    {
        SwViewShell *pVSh = m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
        if ( pVSh &&
             COL_TRANSPARENT != pVSh->GetViewOptions()->GetRetoucheColor())
            aColor = pVSh->GetViewOptions()->GetRetoucheColor();
    }

    OutCSS1_PropertyAscii(sCSS1_P_background, GetCSS1_Color(aColor));
}

static SwHTMLWriter& OutCSS1_SvxTextLn_SvxCrOut_SvxBlink( SwHTMLWriter& rWrt,
                    const SvxUnderlineItem *pUItem,
                    const SvxOverlineItem *pOItem,
                    const SvxCrossedOutItem *pCOItem,
                    const SvxBlinkItem *pBItem )
{
    bool bNone = false;
    OStringBuffer sOut;

    if( pUItem )
    {
        switch( pUItem->GetLineStyle() )
        {
        case LINESTYLE_NONE:
            bNone = true;
            break;
        case LINESTYLE_DONTKNOW:
            break;
        default:
            if( !rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
            {
                // this also works in HTML does not need to be written as
                // a STYLE-Options, and must not be written as Hint
                OSL_ENSURE( !rWrt.IsCSS1Source(CSS1_OUTMODE_HINT) || rWrt.mbReqIF,
                        "write underline as Hint?" );
                sOut.append(sCSS1_PV_underline);
            }
            break;
        }
    }

    if( pOItem )
    {
        switch( pOItem->GetLineStyle() )
        {
        case LINESTYLE_NONE:
            bNone = true;
            break;
        case LINESTYLE_DONTKNOW:
            break;
        default:
            if( !rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
            {
                // this also works in HTML does not need to be written as
                // a STYLE-Options, and must not be written as Hint
                OSL_ENSURE( !rWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                        "write overline as Hint?" );
                if (!sOut.isEmpty())
                    sOut.append(' ');
                sOut.append(sCSS1_PV_overline);
            }
            break;
        }
    }

    if( pCOItem )
    {
        switch( pCOItem->GetStrikeout() )
        {
        case STRIKEOUT_NONE:
            bNone = true;
            break;
        case STRIKEOUT_DONTKNOW:
            break;
        default:
            if( !rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
            {
                // this also works in HTML does not need to be written as
                // a STYLE-Options, and must not be written as Hint
                OSL_ENSURE( !rWrt.IsCSS1Source(CSS1_OUTMODE_HINT) || rWrt.mbReqIF,
                        "write crossedOut as Hint?" );
                if (!sOut.isEmpty())
                    sOut.append(' ');
                sOut.append(sCSS1_PV_line_through);
            }
            break;
        }
    }

    if( pBItem )
    {
        if( !pBItem->GetValue() )
        {
            bNone = true;
        }
        else if( !rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        {
            // this also works in HTML does not need to be written as
            // a STYLE-Options, and must not be written as Hint
            OSL_ENSURE( !rWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                    "write blink as Hint?" );
            if (!sOut.isEmpty())
                sOut.append(' ');
            sOut.append(sCSS1_PV_blink);
        }
    }

    if (!sOut.isEmpty())
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_text_decoration, sOut );
    else if( bNone )
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_text_decoration, sCSS1_PV_none );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxCaseMap( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    switch( static_cast<const SvxCaseMapItem&>(rHt).GetCaseMap() )
    {
    case SvxCaseMap::NotMapped:
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_font_variant, sCSS1_PV_normal );
        break;
    case SvxCaseMap::SmallCaps:
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_font_variant, sCSS1_PV_small_caps );
        break;
    case SvxCaseMap::Uppercase:
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_text_transform, sCSS1_PV_uppercase );
        break;
    case SvxCaseMap::Lowercase:
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_text_transform, sCSS1_PV_lowercase );
        break;
    case SvxCaseMap::Capitalize:
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_text_transform, sCSS1_PV_capitalize );
        break;
    default:
        ;
    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxColor( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // Colors do not need to be exported for Style-Option.
    if( rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) &&
        !rWrt.m_bCfgPreferStyles )
        return rWrt;
    OSL_ENSURE( !rWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
            "write color as Hint?" );

    Color aColor( static_cast<const SvxColorItem&>(rHt).GetValue() );
    if( COL_AUTO == aColor )
        aColor = COL_BLACK;

    rWrt.OutCSS1_PropertyAscii(sCSS1_P_color, GetCSS1_Color(aColor));

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxCrossedOut( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // This function only exports Hints!
    // Otherwise OutCSS1_SvxTextLn_SvxCrOut_SvxBlink() is called directly.

    if( rWrt.IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxTextLn_SvxCrOut_SvxBlink( rWrt,
                nullptr, nullptr, static_cast<const SvxCrossedOutItem *>(&rHt), nullptr );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxFont( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // No need to export Fonts for the Style-Option.
    if( rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        return rWrt;

    sal_uInt16 nScript = CSS1_OUTMODE_WESTERN;
    switch( rHt.Which() )
    {
    case RES_CHRATR_CJK_FONT:   nScript = CSS1_OUTMODE_CJK; break;
    case RES_CHRATR_CTL_FONT:   nScript = CSS1_OUTMODE_CTL; break;
    }
    if( !rWrt.IsCSS1Script( nScript ) )
        return rWrt;

    OSL_ENSURE( !rWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
            "write Font as Hint?" );

    OUString sOut;
    // MS IE3b1 has problems with single quotes
    sal_uInt16 nMode = rWrt.m_nCSS1OutMode & CSS1_OUTMODE_ANY_ON;
    sal_Unicode cQuote = nMode == CSS1_OUTMODE_RULE_ON ? '\"' : '\'';
    SwHTMLWriter::PrepareFontList( static_cast<const SvxFontItem&>(rHt), sOut, cQuote,
                                   true );

    rWrt.OutCSS1_Property( sCSS1_P_font_family, sOut );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxFontHeight( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // Font-Height need not be exported in the Style-Option.
    // For Drop-Caps another Font-Size is exported.
    if( rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) ||
        rWrt.IsCSS1Source( CSS1_OUTMODE_DROPCAP ) )
        return rWrt;

    sal_uInt16 nScript = CSS1_OUTMODE_WESTERN;
    switch( rHt.Which() )
    {
    case RES_CHRATR_CJK_FONTSIZE:   nScript = CSS1_OUTMODE_CJK; break;
    case RES_CHRATR_CTL_FONTSIZE:   nScript = CSS1_OUTMODE_CTL; break;
    }
    if( !rWrt.IsCSS1Script( nScript ) )
        return rWrt;

    sal_uInt32 nHeight = static_cast<const SvxFontHeightItem&>(rHt).GetHeight();
    OString sHeight(OString::number(nHeight/20) + sCSS1_UNIT_pt);
    rWrt.OutCSS1_PropertyAscii(sCSS1_P_font_size, sHeight);

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxPosture( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    sal_uInt16 nScript = CSS1_OUTMODE_WESTERN;
    switch( rHt.Which() )
    {
    case RES_CHRATR_CJK_POSTURE:    nScript = CSS1_OUTMODE_CJK; break;
    case RES_CHRATR_CTL_POSTURE:    nScript = CSS1_OUTMODE_CTL; break;
    }
    if( !rWrt.IsCSS1Script( nScript ) )
        return rWrt;

    std::string_view pStr;
    switch( static_cast<const SvxPostureItem&>(rHt).GetPosture() )
    {
    case ITALIC_NONE:       pStr = sCSS1_PV_normal;     break;
    case ITALIC_OBLIQUE:    pStr = sCSS1_PV_oblique;    break;
    case ITALIC_NORMAL:
        if( !rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        {
            // this also works in HTML does not need to be written as
            // a STYLE-Options, and must not be written as Hint
            OSL_ENSURE( !rWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                    "write italic as Hint?" );
            pStr = sCSS1_PV_italic;
        }
        break;
    default:
        ;
    }

    if( !pStr.empty() )
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_font_style, pStr );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxKerning( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    sal_Int16 nValue = static_cast<const SvxKerningItem&>(rHt).GetValue();
    if( nValue )
    {
        OStringBuffer sOut;
        if( nValue < 0 )
        {
            sOut.append('-');
            nValue = -nValue;
        }

        // Width as n.n pt
        nValue = (nValue + 1) / 2;  // 1/10pt
        sOut.append(OString::number(nValue  / 10) + "." + OString::number(nValue % 10) +
                    sCSS1_UNIT_pt);

        rWrt.OutCSS1_PropertyAscii(sCSS1_P_letter_spacing, sOut);
        sOut.setLength(0);
    }
    else
    {
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_letter_spacing,
                                        sCSS1_PV_normal );
    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxLanguage( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // Only export Language rules
    if( rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        return rWrt;

    sal_uInt16 nScript = CSS1_OUTMODE_WESTERN;
    switch( rHt.Which() )
    {
    case RES_CHRATR_CJK_LANGUAGE:   nScript = CSS1_OUTMODE_CJK; break;
    case RES_CHRATR_CTL_LANGUAGE:   nScript = CSS1_OUTMODE_CTL; break;
    }
    if( !rWrt.IsCSS1Script( nScript ) )
        return rWrt;

    OSL_ENSURE( !rWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
            "write Language as Hint?" );

    LanguageType eLang = static_cast<const SvxLanguageItem &>(rHt).GetLanguage();
    if( LANGUAGE_DONTKNOW == eLang )
        return rWrt;

    OUString sOut = LanguageTag::convertToBcp47( eLang );

    rWrt.OutCSS1_Property( sCSS1_P_so_language, sOut );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxUnderline( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // This function only exports Hints!
    // Otherwise OutCSS1_SvxTextLn_SvxCrOut_SvxBlink() is called directly.

    if( rWrt.IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxTextLn_SvxCrOut_SvxBlink( rWrt,
                static_cast<const SvxUnderlineItem *>(&rHt), nullptr, nullptr, nullptr );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxOverline( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // This function only exports Hints!
    // Otherwise OutCSS1_SvxTextLn_SvxCrOut_SvxBlink() is called directly.

    if( rWrt.IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxTextLn_SvxCrOut_SvxBlink( rWrt,
                nullptr, static_cast<const SvxOverlineItem *>(&rHt), nullptr, nullptr );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxHidden( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    if ( static_cast<const SvxCharHiddenItem&>(rHt).GetValue() )
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_display, sCSS1_PV_none );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxFontWeight( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    sal_uInt16 nScript = CSS1_OUTMODE_WESTERN;
    switch( rHt.Which() )
    {
    case RES_CHRATR_CJK_WEIGHT: nScript = CSS1_OUTMODE_CJK; break;
    case RES_CHRATR_CTL_WEIGHT: nScript = CSS1_OUTMODE_CTL; break;
    }
    if( !rWrt.IsCSS1Script( nScript ) )
        return rWrt;

    std::string_view pStr;
    switch( static_cast<const SvxWeightItem&>(rHt).GetWeight() )
    {
    case WEIGHT_ULTRALIGHT: pStr = sCSS1_PV_extra_light;    break;
    case WEIGHT_LIGHT:      pStr = sCSS1_PV_light;          break;
    case WEIGHT_SEMILIGHT:  pStr = sCSS1_PV_demi_light;     break;
    case WEIGHT_NORMAL:     pStr = sCSS1_PV_normal;         break;
    case WEIGHT_SEMIBOLD:   pStr = sCSS1_PV_demi_bold;      break;
    case WEIGHT_BOLD:
        if( !rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        {
            // this also works in HTML does not need to be written as
            // a STYLE-Options, and must not be written as Hint
            OSL_ENSURE( !rWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                    "write bold as Hint?" );
            pStr = sCSS1_PV_bold;
        }
        break;
    case WEIGHT_ULTRABOLD:  pStr = sCSS1_PV_extra_bold;     break;
    default:
        pStr = sCSS1_PV_normal;
    }

    if( !pStr.empty() )
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_font_weight, pStr );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxBlink( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // This function only exports Hints!
    // Otherwise OutCSS1_SvxTextLn_SvxCrOut_SvxBlink() is called directly.

    if( rWrt.IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxTextLn_SvxCrOut_SvxBlink( rWrt,
                nullptr, nullptr, nullptr, static_cast<const SvxBlinkItem *>(&rHt) );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxLineSpacing( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // Netscape4 has big problems with cell heights if the line spacing is
    // changed within a table and the width of the table is not calculated
    // automatically (== if there is a WIDTH-Option)
    if( rWrt.m_bOutTable && rWrt.m_bCfgNetscape4 )
        return rWrt;

    const SvxLineSpacingItem& rLSItem = static_cast<const SvxLineSpacingItem&>(rHt);

    sal_uInt16 nHeight = 0;
    sal_uInt16 nPercentHeight = 0;
    SvxLineSpaceRule eLineSpace = rLSItem.GetLineSpaceRule();
    switch( rLSItem.GetInterLineSpaceRule() )
    {
    case SvxInterLineSpaceRule::Off:
    case SvxInterLineSpaceRule::Fix:
        {
            switch( eLineSpace )
            {
            case SvxLineSpaceRule::Min:
            case SvxLineSpaceRule::Fix:
                nHeight = rLSItem.GetLineHeight();
                break;
            case SvxLineSpaceRule::Auto:
                nPercentHeight = 100;
                break;
            default:
                ;
            }
        }
        break;
    case SvxInterLineSpaceRule::Prop:
        nPercentHeight = rLSItem.GetPropLineSpace();
        break;

    default:
        ;
    }

    if( nHeight )
        rWrt.OutCSS1_UnitProperty( sCSS1_P_line_height, static_cast<tools::Long>(nHeight) );
    else if( nPercentHeight &&
        !(nPercentHeight < 115 && rWrt.m_bParaDotLeaders )) // avoid HTML scrollbars and missing descenders
    {
        OString sHeight(OString::number(nPercentHeight) + "%");
        rWrt.OutCSS1_PropertyAscii(sCSS1_P_line_height, sHeight);
    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxAdjust( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // Export Alignment in Style-Option only if the Tag does not allow ALIGN=xxx
    if( rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) &&
        !rWrt.m_bNoAlign)
        return rWrt;

    std::string_view pStr;
    switch( static_cast<const SvxAdjustItem&>(rHt).GetAdjust() )
    {
    case SvxAdjust::Left:   pStr = sCSS1_PV_left;       break;
    case SvxAdjust::Right:  pStr = sCSS1_PV_right;      break;
    case SvxAdjust::Block:  pStr = sCSS1_PV_justify;    break;
    case SvxAdjust::Center: pStr = sCSS1_PV_center;     break;
    default:
        ;
    }

    if( !pStr.empty() )
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_text_align, pStr );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxFormatSplit( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    std::string_view pStr = static_cast<const SvxFormatSplitItem&>(rHt).GetValue()
                            ? sCSS1_PV_auto
                            : sCSS1_PV_avoid;
    rWrt.OutCSS1_PropertyAscii( sCSS1_P_page_break_inside, pStr );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SwFormatLayoutSplit( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    std::string_view pStr = static_cast<const SwFormatLayoutSplit&>(rHt).GetValue()
                            ? sCSS1_PV_auto
                            : sCSS1_PV_avoid;
    rWrt.OutCSS1_PropertyAscii( sCSS1_P_page_break_inside, pStr );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxWidows( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    OString aStr(OString::number(static_cast<const SvxWidowsItem&>(rHt).GetValue()));
    rWrt.OutCSS1_PropertyAscii( sCSS1_P_widows, aStr );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxOrphans( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    OString aStr(OString::number(static_cast<const SvxOrphansItem&>(rHt).GetValue()));
    rWrt.OutCSS1_PropertyAscii( sCSS1_P_orphans, aStr );

    return rWrt;
}

static void OutCSS1_SwFormatDropAttrs( SwHTMLWriter& rHWrt,
                                    const SwFormatDrop& rDrop,
                                     const SfxItemSet *pCharFormatItemSet )
{
    // Text flows around on right side
    rHWrt.OutCSS1_PropertyAscii( sCSS1_P_float, sCSS1_PV_left );

    // number of lines -> use % for Font-Height!
    OString sOut(OString::number(rDrop.GetLines()*100) + "%");
    rHWrt.OutCSS1_PropertyAscii(sCSS1_P_font_size, sOut);

    // distance to Text = right margin
    sal_uInt16 nDistance = rDrop.GetDistance();
    if( nDistance > 0 )
        rHWrt.OutCSS1_UnitProperty( sCSS1_P_margin_right, nDistance );

    const SwCharFormat *pDCCharFormat = rDrop.GetCharFormat();
    if( pCharFormatItemSet )
        rHWrt.OutCSS1_SfxItemSet( *pCharFormatItemSet );
    else if( pDCCharFormat )
        rHWrt.OutCSS1_SfxItemSet( pDCCharFormat->GetAttrSet() );
    else if( (rHWrt.m_nCSS1OutMode & CSS1_OUTMODE_ANY_OFF) == CSS1_OUTMODE_RULE_OFF )
        rHWrt.Strm().WriteOString( sCSS1_rule_end );

}

static SwHTMLWriter& OutCSS1_SwFormatDrop( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // never export as an Option of a paragraph, but only as Hints
    if( !rWrt.IsCSS1Source(CSS1_OUTMODE_HINT) )
        return rWrt;

    if( rWrt.m_bTagOn )
    {
        SwCSS1OutMode aMode( rWrt,
                             rWrt.m_nCSS1Script|CSS1_OUTMODE_SPAN_TAG1_ON|CSS1_OUTMODE_ENCODE|
                             CSS1_OUTMODE_DROPCAP, nullptr );

        OutCSS1_SwFormatDropAttrs( rWrt, static_cast<const SwFormatDrop&>(rHt) );
        // A "> is already printed by the calling OutCSS1_HintAsSpanTag.
    }
    else
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_span), false );
    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SwFormatFrameSize( SwHTMLWriter& rWrt, const SfxPoolItem& rHt,
                                     Css1FrameSize nMode )
{
    const SwFormatFrameSize& rFSItem = static_cast<const SwFormatFrameSize&>(rHt);

    if( nMode & Css1FrameSize::Width )
    {
        sal_uInt8 nPercentWidth = rFSItem.GetWidthPercent();
        if( nPercentWidth )
        {
            OString sOut(OString::number(nPercentWidth) + "%");
            rWrt.OutCSS1_PropertyAscii(sCSS1_P_width, sOut);
        }
        else if( nMode & Css1FrameSize::Pixel )
        {
            rWrt.OutCSS1_PixelProperty( sCSS1_P_width,
                                            rFSItem.GetSize().Width() );
        }
        else
        {
            rWrt.OutCSS1_UnitProperty( sCSS1_P_width,
                                           rFSItem.GetSize().Width() );
        }
    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxFirstLineIndent(SwHTMLWriter & rWrt, SfxPoolItem const& rHt)
{
    const SvxFirstLineIndentItem & rFirstLine(static_cast<const SvxFirstLineIndentItem&>(rHt));

    // No Export of a firm attribute is needed if the new values
    // match that of the current template

    // The LineIndent of the first line might contain the room for numbering
    tools::Long nFirstLineIndent
        = static_cast<tools::Long>(rFirstLine.ResolveTextFirstLineOffset({}))
          - rWrt.m_nFirstLineIndent;
    if (rWrt.m_nDfltFirstLineIndent != nFirstLineIndent)
    {
        rWrt.OutCSS1_UnitProperty(sCSS1_P_text_indent, nFirstLineIndent);
    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxTextLeftMargin(SwHTMLWriter & rWrt, SfxPoolItem const& rHt)
{
    const SvxTextLeftMarginItem& rLeftMargin(static_cast<const SvxTextLeftMarginItem&>(rHt));

    // No Export of a firm attribute is needed if the new values
    // match that of the current template

    // A left margin can exist because of a list nearby
    tools::Long nLeftMargin = rLeftMargin.ResolveTextLeft({}) - rWrt.m_nLeftMargin;
    if (rWrt.m_nDfltLeftMargin != nLeftMargin)
    {
        rWrt.OutCSS1_UnitProperty(sCSS1_P_margin_left, nLeftMargin);

        // max-width = max-width - margin-left for TOC paragraphs with dot leaders
        if (rWrt.m_bParaDotLeaders)
            rWrt.OutCSS1_UnitProperty(sCSS1_P_max_width, o3tl::convert(DOT_LEADERS_MAX_WIDTH, o3tl::Length::cm, o3tl::Length::twip) - nLeftMargin);

    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxRightMargin(SwHTMLWriter & rWrt, SfxPoolItem const& rHt)
{
    const SvxRightMarginItem& rRightMargin(static_cast<const SvxRightMarginItem&>(rHt));

    // No Export of a firm attribute is needed if the new values
    // match that of the current template

    if (rWrt.m_nDfltRightMargin != rRightMargin.ResolveRight({}))
    {
        rWrt.OutCSS1_UnitProperty(sCSS1_P_margin_right, rRightMargin.ResolveRight({}));
    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxLRSpace( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    const SvxLRSpaceItem& rLRItem = static_cast<const SvxLRSpaceItem&>(rHt);

    // No Export of a firm attribute is needed if the new values
    // match that of the current template

    // A left margin can exist because of a list nearby
    tools::Long nLeftMargin = rLRItem.ResolveTextLeft({}) - rWrt.m_nLeftMargin;
    if( rWrt.m_nDfltLeftMargin != nLeftMargin )
    {
        rWrt.OutCSS1_UnitProperty( sCSS1_P_margin_left, nLeftMargin );

        // max-width = max-width - margin-left for TOC paragraphs with dot leaders
        if( rWrt.m_bParaDotLeaders )
            rWrt.OutCSS1_UnitProperty( sCSS1_P_max_width, o3tl::convert(DOT_LEADERS_MAX_WIDTH, o3tl::Length::cm, o3tl::Length::twip) - nLeftMargin );

    }

    if (rWrt.m_nDfltRightMargin != rLRItem.ResolveRight({}))
    {
        rWrt.OutCSS1_UnitProperty(sCSS1_P_margin_right, rLRItem.ResolveRight({}));
    }

    // The LineIndent of the first line might contain the room for numbering
    tools::Long nFirstLineIndent = static_cast<tools::Long>(rLRItem.ResolveTextFirstLineOffset({}))
                                   - rWrt.m_nFirstLineIndent;
    if( rWrt.m_nDfltFirstLineIndent != nFirstLineIndent )
    {
        rWrt.OutCSS1_UnitProperty( sCSS1_P_text_indent,
                                     nFirstLineIndent );
    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxULSpace( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    const SvxULSpaceItem& rULItem = static_cast<const SvxULSpaceItem&>(rHt);

    if( rWrt.m_nDfltTopMargin != rULItem.GetUpper() )
    {
        rWrt.OutCSS1_UnitProperty( sCSS1_P_margin_top,
                                     static_cast<tools::Long>(rULItem.GetUpper()) );
    }

    if( rWrt.m_nDfltBottomMargin != rULItem.GetLower() )
    {
        rWrt.OutCSS1_UnitProperty( sCSS1_P_margin_bottom,
                                     static_cast<tools::Long>(rULItem.GetLower()) );
    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxULSpace_SvxLRSpace( SwHTMLWriter& rWrt,
                                        const SvxULSpaceItem *pULItem,
                                        const SvxLRSpaceItem *pLRItem )
{
    if (pLRItem && pULItem && pLRItem->GetLeft() == pLRItem->GetRight()
        && pLRItem->GetLeft() == SvxIndentValue::twips(pULItem->GetUpper())
        && pLRItem->GetLeft() == SvxIndentValue::twips(pULItem->GetLower())
        && pLRItem->GetLeft() != SvxIndentValue::twips(rWrt.m_nDfltLeftMargin)
        && pLRItem->GetRight() != SvxIndentValue::twips(rWrt.m_nDfltRightMargin)
        && pULItem->GetUpper() != rWrt.m_nDfltTopMargin
        && pULItem->GetLower() != rWrt.m_nDfltBottomMargin)
    {
        rWrt.OutCSS1_UnitProperty(sCSS1_P_margin, pLRItem->ResolveLeft({}));
    }
    else
    {
        if( pLRItem )
            OutCSS1_SvxLRSpace( rWrt, *pLRItem );
        if( pULItem )
            OutCSS1_SvxULSpace( rWrt, *pULItem );
    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxULSpace_SvxLRSpace( SwHTMLWriter& rWrt,
                                        const SfxItemSet& rItemSet )
{
    const SvxLRSpaceItem *pLRSpace = rItemSet.GetItemIfSet( RES_LR_SPACE, false/*bDeep*/ );
    const SvxULSpaceItem *pULSpace = rItemSet.GetItemIfSet( RES_UL_SPACE, false/*bDeep*/ );

    if( pLRSpace || pULSpace )
        OutCSS1_SvxULSpace_SvxLRSpace( rWrt, pULSpace, pLRSpace );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxFormatBreak_SwFormatPDesc_SvxFormatKeep( SwHTMLWriter& rWrt,
                                        const SvxFormatBreakItem *pBreakItem,
                                        const SwFormatPageDesc *pPDescItem,
                                        const SvxFormatKeepItem *pKeepItem )
{
    if( !rWrt.IsHTMLMode(HTMLMODE_PRINT_EXT) )
        return rWrt;

    std::string_view pBreakBefore;
    std::string_view pBreakAfter;

    if( pKeepItem )
    {
        pBreakAfter = pKeepItem->GetValue() ? sCSS1_PV_avoid : sCSS1_PV_auto;
    }
    if( pBreakItem )
    {
        switch( pBreakItem->GetBreak() )
        {
        case SvxBreak::NONE:
            pBreakBefore = sCSS1_PV_auto;
            if( pBreakAfter.empty() )
                pBreakAfter = sCSS1_PV_auto;
            break;

        case SvxBreak::PageBefore:
            pBreakBefore = sCSS1_PV_always;
            break;

        case SvxBreak::PageAfter:
            pBreakAfter= sCSS1_PV_always;
            break;

        default:
            ;
        }
    }
    if( pPDescItem )
    {
        const SwPageDesc *pPDesc = pPDescItem->GetPageDesc();
        if( pPDesc )
        {
            switch( pPDesc->GetPoolFormatId() )
            {
            case RES_POOLPAGE_LEFT:     pBreakBefore = sCSS1_PV_left;   break;
            case RES_POOLPAGE_RIGHT:    pBreakBefore = sCSS1_PV_right;  break;
            default:                    pBreakBefore = sCSS1_PV_always; break;
            }
        }
        else if( pBreakBefore.empty() )
        {
            pBreakBefore = sCSS1_PV_auto;
        }
    }

    if (rWrt.mbSkipHeaderFooter)
        // No page break when writing only a fragment.
        return rWrt;

    if( !pBreakBefore.empty() )
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_page_break_before,
                                        pBreakBefore );
    if( !pBreakAfter.empty() )
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_page_break_after,
                                        pBreakAfter );

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxFormatBreak_SwFormatPDesc_SvxFormatKeep( SwHTMLWriter& rWrt,
                                        const SfxItemSet& rItemSet,
                                        bool bDeep )
{
    const SvxFormatBreakItem *pBreakItem = rItemSet.GetItemIfSet( RES_BREAK, bDeep );

    const SwFormatPageDesc *pPDescItem = nullptr;
    if( !rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) ||
        !rWrt.m_bCSS1IgnoreFirstPageDesc ||
        rWrt.m_pStartNdIdx->GetIndex() !=
                    rWrt.m_pCurrentPam->GetPoint()->GetNodeIndex() )
        pPDescItem = rItemSet.GetItemIfSet( RES_PAGEDESC, bDeep );

    const SvxFormatKeepItem *pKeepItem = rItemSet.GetItemIfSet( RES_KEEP, bDeep );

    if( pBreakItem || pPDescItem || pKeepItem )
        OutCSS1_SvxFormatBreak_SwFormatPDesc_SvxFormatKeep( rWrt, pBreakItem,
                                                   pPDescItem, pKeepItem );

    return rWrt;
}

// Wrapper for OutCSS1_SfxItemSet etc.
static SwHTMLWriter& OutCSS1_SvxBrush( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    OutCSS1_SvxBrush( rWrt, rHt, sw::Css1Background::Attr, nullptr );
    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxBrush( SwHTMLWriter& rWrt, const SfxPoolItem& rHt,
                                 sw::Css1Background nMode,
                                 const OUString* pGraphicName)
{
    // The Character-Attribute is skipped, if we are about to
    // exporting options
    if( rHt.Which() < RES_CHRATR_END &&
        rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        return rWrt;

    // start getting a few values
//  const Brush &rBrush = static_cast<const SvxBrushItem &>(rHt).GetBrush();
    const Color & rColor = static_cast<const SvxBrushItem &>(rHt).GetColor();
    OUString aLink = pGraphicName ? *pGraphicName
                            : static_cast<const SvxBrushItem &>(rHt).GetGraphicLink();
    bool bOwn = false;
    SvxGraphicPosition ePos = static_cast<const SvxBrushItem &>(rHt).GetGraphicPos();
    if( sw::Css1Background::Page == nMode && !rWrt.mbEmbedImages )
    {
        // page style images are exported if not tiled
        if( aLink.isEmpty() || GPOS_TILED==ePos )
            return rWrt;
    }

    // get the color
    bool bColor = false;
    /// set <bTransparent> to true, if color is "no fill"/"auto fill"
    bool bTransparent = (rColor == COL_TRANSPARENT);
    Color aColor;
    if( !bTransparent )
    {
        aColor = rColor;
        bColor = true;
    }

    // and now the Graphic
    OUString aGraphicInBase64;

    // Embedded Graphic -> export WriteEmbedded
    const Graphic* pGrf = nullptr;
    if( rWrt.mbEmbedImages || aLink.isEmpty())
    {
        pGrf = static_cast<const SvxBrushItem &>(rHt).GetGraphic();
        if( pGrf )
        {
            if( !XOutBitmap::GraphicToBase64(*pGrf, aGraphicInBase64) )
            {
                rWrt.m_nWarn = WARN_SWG_POOR_LOAD;
            }
        }
        aLink.clear();
    }
    else if( !pGraphicName && rWrt.m_bCfgCpyLinkedGrfs )
    {
        OUString aGraphicAsLink = aLink;
        bOwn = rWrt.CopyLocalFileToINet( aGraphicAsLink );
        aLink = aGraphicAsLink;
    }
    // In tables we only export something if there is a Graphic
    if( (nMode == sw::Css1Background::Table || nMode == sw::Css1Background::TableRow) && !pGrf && !aLink.isEmpty())
        return rWrt;

    // if necessary, add the orientation of the Graphic
    std::u16string_view pRepeat, pHori;
    std::string_view pVert;
    if( pGrf || !aLink.isEmpty() )
    {
        if( GPOS_TILED==ePos )
        {
            pRepeat = sCSS1_PV_repeat;
        }
        else
        {
            switch( ePos )
            {
            case GPOS_LT:
            case GPOS_MT:
            case GPOS_RT:
                pHori = sCSS1_PV_top;
                break;

            case GPOS_LM:
            case GPOS_MM:
            case GPOS_RM:
                pHori = sCSS1_PV_middle;
                break;

            case GPOS_LB:
            case GPOS_MB:
            case GPOS_RB:
                pHori = sCSS1_PV_bottom;
                break;

            default:
                ;
            }

            switch( ePos )
            {
            case GPOS_LT:
            case GPOS_LM:
            case GPOS_LB:
                pVert = sCSS1_PV_left;
                break;

            case GPOS_MT:
            case GPOS_MM:
            case GPOS_MB:
                pVert = sCSS1_PV_center;
                break;

            case GPOS_RT:
            case GPOS_RM:
            case GPOS_RB:
                pVert = sCSS1_PV_right;
                break;

            default:
                ;
            }

            if( !pHori.empty() || !pVert.empty() )
                pRepeat = sCSS1_PV_no_repeat;
        }
    }

    // now build the string
    OUString sOut;
    if( !pGrf && aLink.isEmpty() && !bColor )
    {
        // no color and no Link, but a transparent Brush
        if( bTransparent && sw::Css1Background::Fly != nMode )
            sOut += sCSS1_PV_transparent;
    }
    else
    {
        if( bColor )
        {
            OString sTmp(GetCSS1_Color(aColor));
            sOut += OStringToOUString(sTmp, RTL_TEXTENCODING_ASCII_US);
        }

        if( pGrf || !aLink.isEmpty() )
        {
            if( bColor )
                sOut += " ";

            if(pGrf)
            {
                sOut += OUString::Concat(sCSS1_url) +
                    "(\'" OOO_STRING_SVTOOLS_HTML_O_data ":" + aGraphicInBase64 + "\')";
            }
            else
            {
                sOut += OUString::Concat(sCSS1_url) + "(" + rWrt.normalizeURL(aLink, bOwn) + ")";
            }

            if( !pRepeat.empty() )
            {
                sOut += OUString::Concat(" ") + pRepeat;
            }

            if( !pHori.empty() )
            {
                sOut += OUString::Concat(" ") + pHori;
            }
            if( !pVert.empty() )
            {
                sOut += " " + OStringToOUString(pVert, RTL_TEXTENCODING_ASCII_US);
            }

            sOut += OUString::Concat(" ") + sCSS1_PV_scroll + " ";
        }
    }

    if( !sOut.isEmpty() )
    {
        rWrt.OutCSS1_Property(sCSS1_P_background, std::string_view(), &sOut,
                                  nMode);
    }

    return rWrt;
}

static void OutCSS1_SvxBorderLine( SwHTMLWriter& rWrt,
                                   std::string_view pProperty,
                                   const SvxBorderLine *pLine )
{
    if( !pLine || pLine->isEmpty() )
    {
        rWrt.OutCSS1_PropertyAscii( pProperty, sCSS1_PV_none );
        return;
    }

    sal_Int32 nWidth = pLine->GetWidth();

    OStringBuffer sOut;
    if( nWidth <= o3tl::convert(1, o3tl::Length::px, o3tl::Length::twip) )
    {
        // If the width is smaller than one pixel, then export as 1px
        // so that Netscape and IE show the line.
        sOut.append("1px");
    }
    else
    {
        nWidth *= 5;    // 1/100pt

        // width in n.nn pt
        sOut.append(OString::number(nWidth / 100) + "." + OString::number((nWidth/10) % 10) +
                    OString::number(nWidth % 10) + sCSS1_UNIT_pt);
    }

    // Line-Style: solid or double
    sOut.append(' ');
    switch (pLine->GetBorderLineStyle())
    {
        case SvxBorderLineStyle::SOLID:
            sOut.append(sCSS1_PV_solid);
            break;
        case SvxBorderLineStyle::DOTTED:
            sOut.append(sCSS1_PV_dotted);
            break;
        case SvxBorderLineStyle::DASHED:
            sOut.append(sCSS1_PV_dashed);
            break;
        case SvxBorderLineStyle::DOUBLE:
        case SvxBorderLineStyle::THINTHICK_SMALLGAP:
        case SvxBorderLineStyle::THINTHICK_MEDIUMGAP:
        case SvxBorderLineStyle::THINTHICK_LARGEGAP:
        case SvxBorderLineStyle::THICKTHIN_SMALLGAP:
        case SvxBorderLineStyle::THICKTHIN_MEDIUMGAP:
        case SvxBorderLineStyle::THICKTHIN_LARGEGAP:
            sOut.append(sCSS1_PV_double);
            break;
        case SvxBorderLineStyle::EMBOSSED:
            sOut.append(sCSS1_PV_ridge);
            break;
        case SvxBorderLineStyle::ENGRAVED:
            sOut.append(sCSS1_PV_groove);
            break;
        case SvxBorderLineStyle::INSET:
            sOut.append(sCSS1_PV_inset);
            break;
        case SvxBorderLineStyle::OUTSET:
            sOut.append(sCSS1_PV_outset);
            break;
        default:
            sOut.append(sCSS1_PV_none);
    }
    sOut.append(' ');

    // and also the color
    sOut.append(GetCSS1_Color(pLine->GetColor()));

    rWrt.OutCSS1_PropertyAscii(pProperty, sOut);
}

SwHTMLWriter& OutCSS1_SvxBox( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // Avoid interference between character and paragraph attributes
    if( rHt.Which() < RES_CHRATR_END &&
        rWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        return rWrt;

    if( rHt.Which() == RES_CHRATR_BOX )
    {
        static constexpr std::string_view inline_block("inline-block");
        if( rWrt.m_bTagOn )
        {
            // Inline-block to make the line height changing correspond to the character border
            rWrt.OutCSS1_PropertyAscii(sCSS1_P_display, inline_block);
        }
        else
        {
            if (!IgnorePropertyForReqIF(rWrt.mbReqIF, sCSS1_P_display, inline_block))
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_span), false );
            return rWrt;
        }
    }

    const SvxBoxItem& rBoxItem = static_cast<const SvxBoxItem&>(rHt);
    const SvxBorderLine *pTop = rBoxItem.GetTop();
    const SvxBorderLine *pBottom = rBoxItem.GetBottom();
    const SvxBorderLine *pLeft = rBoxItem.GetLeft();
    const SvxBorderLine *pRight = rBoxItem.GetRight();

    if( (pTop && pBottom && pLeft && pRight &&
         *pTop == *pBottom && *pTop == *pLeft && *pTop == *pRight) ||
         (!pTop && !pBottom && !pLeft && !pRight) )
    {
        // all Lines are set and equal, or all Lines are not set
        // => border : ...
        OutCSS1_SvxBorderLine( rWrt, sCSS1_P_border, pTop );
    }
    else
    {
        // otherwise export all Lines separately
        OutCSS1_SvxBorderLine( rWrt, sCSS1_P_border_top, pTop );
        OutCSS1_SvxBorderLine( rWrt, sCSS1_P_border_bottom, pBottom );
        OutCSS1_SvxBorderLine( rWrt, sCSS1_P_border_left, pLeft );
        OutCSS1_SvxBorderLine( rWrt, sCSS1_P_border_right, pRight );
    }

    tools::Long nTopDist = pTop ? rBoxItem.GetDistance( SvxBoxItemLine::TOP ) : 0;
    tools::Long nBottomDist = pBottom ? rBoxItem.GetDistance( SvxBoxItemLine::BOTTOM ) : 0;
    tools::Long nLeftDist = pLeft ? rBoxItem.GetDistance( SvxBoxItemLine::LEFT ) : 0;
    tools::Long nRightDist = pRight ? rBoxItem.GetDistance( SvxBoxItemLine::RIGHT ) : 0;

    if( nTopDist == nBottomDist && nLeftDist == nRightDist )
    {
        OStringBuffer sVal;
        AddUnitPropertyValue(sVal, nTopDist, rWrt.GetCSS1Unit());
        if( nTopDist != nLeftDist )
        {
            sVal.append(' ');
            AddUnitPropertyValue(sVal, nLeftDist, rWrt.GetCSS1Unit());
        }
        rWrt.OutCSS1_PropertyAscii(sCSS1_P_padding, sVal);
    }
    else
    {
        rWrt.OutCSS1_UnitProperty( sCSS1_P_padding_top, nTopDist );
        rWrt.OutCSS1_UnitProperty( sCSS1_P_padding_bottom, nBottomDist );
        rWrt.OutCSS1_UnitProperty( sCSS1_P_padding_left, nLeftDist );
        rWrt.OutCSS1_UnitProperty( sCSS1_P_padding_right, nRightDist );
    }

    return rWrt;
}

static SwHTMLWriter& OutCSS1_SvxFrameDirection( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    // Language will be exported rules only
    if( !rWrt.IsCSS1Source( CSS1_OUTMODE_TEMPLATE ) )
        return rWrt;

    SvxFrameDirection nDir =
        static_cast< const SvxFrameDirectionItem& >( rHt ).GetValue();
    std::string_view pStr;
    switch( nDir )
    {
    case SvxFrameDirection::Horizontal_LR_TB:
    case SvxFrameDirection::Vertical_LR_TB:
        pStr = sCSS1_PV_ltr;
        break;
    case SvxFrameDirection::Horizontal_RL_TB:
    case SvxFrameDirection::Vertical_RL_TB:
        pStr = sCSS1_PV_rtl;
        break;
    case SvxFrameDirection::Environment:
        pStr = sCSS1_PV_inherit;
        break;
    default: break;
    }

    if( !pStr.empty() )
        rWrt.OutCSS1_PropertyAscii( sCSS1_P_direction, pStr );

    return rWrt;
}

/*
 * Place here the table for the HTML-Function-Pointer to the
 * Export-Functions.
 * They are local structures, only needed within the HTML-DLL.
 */

SwAttrFnTab const aCSS1AttrFnTab = {
/* RES_CHRATR_CASEMAP   */          OutCSS1_SvxCaseMap,
/* RES_CHRATR_CHARSETCOLOR  */      nullptr,
/* RES_CHRATR_COLOR */              OutCSS1_SvxColor,
/* RES_CHRATR_CONTOUR   */          nullptr,
/* RES_CHRATR_CROSSEDOUT    */      OutCSS1_SvxCrossedOut,
/* RES_CHRATR_ESCAPEMENT    */      nullptr,
/* RES_CHRATR_FONT  */              OutCSS1_SvxFont,
/* RES_CHRATR_FONTSIZE  */          OutCSS1_SvxFontHeight,
/* RES_CHRATR_KERNING   */          OutCSS1_SvxKerning,
/* RES_CHRATR_LANGUAGE  */          OutCSS1_SvxLanguage,
/* RES_CHRATR_POSTURE   */          OutCSS1_SvxPosture,
/* RES_CHRATR_UNUSED1*/             nullptr,
/* RES_CHRATR_SHADOWED  */          nullptr,
/* RES_CHRATR_UNDERLINE */          OutCSS1_SvxUnderline,
/* RES_CHRATR_WEIGHT    */          OutCSS1_SvxFontWeight,
/* RES_CHRATR_WORDLINEMODE  */      nullptr,
/* RES_CHRATR_AUTOKERN  */          nullptr,
/* RES_CHRATR_BLINK */              OutCSS1_SvxBlink,
/* RES_CHRATR_NOHYPHEN  */          nullptr, // new: don't separate
/* RES_CHRATR_UNUSED2 */            nullptr,
/* RES_CHRATR_BACKGROUND */         OutCSS1_SvxBrush, // new: character background
/* RES_CHRATR_CJK_FONT */           OutCSS1_SvxFont,
/* RES_CHRATR_CJK_FONTSIZE */       OutCSS1_SvxFontHeight,
/* RES_CHRATR_CJK_LANGUAGE */       OutCSS1_SvxLanguage,
/* RES_CHRATR_CJK_POSTURE */        OutCSS1_SvxPosture,
/* RES_CHRATR_CJK_WEIGHT */         OutCSS1_SvxFontWeight,
/* RES_CHRATR_CTL_FONT */           OutCSS1_SvxFont,
/* RES_CHRATR_CTL_FONTSIZE */       OutCSS1_SvxFontHeight,
/* RES_CHRATR_CTL_LANGUAGE */       OutCSS1_SvxLanguage,
/* RES_CHRATR_CTL_POSTURE */        OutCSS1_SvxPosture,
/* RES_CHRATR_CTL_WEIGHT */         OutCSS1_SvxFontWeight,
/* RES_CHRATR_ROTATE */             nullptr,
/* RES_CHRATR_EMPHASIS_MARK */      nullptr,
/* RES_CHRATR_TWO_LINES */          nullptr,
/* RES_CHRATR_SCALEW */             nullptr,
/* RES_CHRATR_RELIEF */             nullptr,
/* RES_CHRATR_HIDDEN */             OutCSS1_SvxHidden,
/* RES_CHRATR_OVERLINE */           OutCSS1_SvxOverline,
/* RES_CHRATR_RSID */               nullptr,
/* RES_CHRATR_BOX */                OutCSS1_SvxBox,
/* RES_CHRATR_SHADOW */             nullptr,
/* RES_CHRATR_HIGHLIGHT */          nullptr,
/* RES_CHRATR_GRABBAG */            nullptr,
/* RES_CHRATR_BIDIRTL */            nullptr,
/* RES_CHRATR_UNUSED3 */            nullptr,
/* RES_CHRATR_SCRIPT_HINT */        nullptr,

/* RES_TXTATR_REFMARK */            nullptr,
/* RES_TXTATR_TOXMARK */            nullptr,
/* RES_TXTATR_META */               nullptr,
/* RES_TXTATR_METAFIELD */          nullptr,
/* RES_TXTATR_AUTOFMT */            nullptr,
/* RES_TXTATR_INETFMT */            nullptr,
/* RES_TXTATR_CHARFMT */            nullptr,
/* RES_TXTATR_CJK_RUBY */           nullptr,
/* RES_TXTATR_UNKNOWN_CONTAINER */  nullptr,
/* RES_TXTATR_INPUTFIELD */         nullptr,
/* RES_TXTATR_CONTENTCONTROL */     nullptr,

/* RES_TXTATR_FIELD */              nullptr,
/* RES_TXTATR_FLYCNT */             nullptr,
/* RES_TXTATR_FTN */                nullptr,
/* RES_TXTATR_ANNOTATION */         nullptr,
/* RES_TXTATR_LINEBREAK */          nullptr,
/* RES_TXTATR_DUMMY1 */             nullptr, // Dummy:

/* RES_PARATR_LINESPACING   */      OutCSS1_SvxLineSpacing,
/* RES_PARATR_ADJUST    */          OutCSS1_SvxAdjust,
/* RES_PARATR_SPLIT */              OutCSS1_SvxFormatSplit,
/* RES_PARATR_ORPHANS   */          OutCSS1_SvxOrphans,
/* RES_PARATR_WIDOWS    */          OutCSS1_SvxWidows,
/* RES_PARATR_TABSTOP   */          nullptr,
/* RES_PARATR_HYPHENZONE*/          nullptr,
/* RES_PARATR_DROP */               OutCSS1_SwFormatDrop,
/* RES_PARATR_REGISTER */           nullptr, // new: register-true
/* RES_PARATR_NUMRULE */            nullptr,
/* RES_PARATR_SCRIPTSPACE */        nullptr,
/* RES_PARATR_HANGINGPUNCTUATION */ nullptr,
/* RES_PARATR_FORBIDDEN_RULES */    nullptr, // new
/* RES_PARATR_VERTALIGN */          nullptr, // new
/* RES_PARATR_SNAPTOGRID*/          nullptr, // new
/* RES_PARATR_CONNECT_TO_BORDER */  nullptr, // new
/* RES_PARATR_OUTLINELEVEL */       nullptr, // new since cws outlinelevel
/* RES_PARATR_RSID */               nullptr, // new
/* RES_PARATR_GRABBAG */            nullptr,

/* RES_PARATR_LIST_ID */            nullptr, // new
/* RES_PARATR_LIST_LEVEL */         nullptr, // new
/* RES_PARATR_LIST_ISRESTART */     nullptr, // new
/* RES_PARATR_LIST_RESTARTVALUE */  nullptr, // new
/* RES_PARATR_LIST_ISCOUNTED */     nullptr, // new
/* RES_PARATR_LIST_AUTOFMT */       nullptr, // new

/* RES_FILL_ORDER   */              nullptr,
/* RES_FRM_SIZE */                  nullptr,
/* RES_PAPER_BIN    */              nullptr,
/* RES_MARGIN_FIRSTLINE */          OutCSS1_SvxFirstLineIndent,
/* RES_MARGIN_TEXTLEFT */           OutCSS1_SvxTextLeftMargin,
/* RES_MARGIN_RIGHT */              OutCSS1_SvxRightMargin,
/* RES_MARGIN_LEFT */               nullptr,
/* RES_MARGIN_GUTTER */             nullptr,
/* RES_MARGIN_GUTTER_RIGHT */       nullptr,
/* RES_LR_SPACE */                  OutCSS1_SvxLRSpace,
/* RES_UL_SPACE */                  OutCSS1_SvxULSpace,
/* RES_PAGEDESC */                  nullptr,
/* RES_BREAK */                     nullptr,
/* RES_CNTNT */                     nullptr,
/* RES_HEADER */                    nullptr,
/* RES_FOOTER */                    nullptr,
/* RES_PRINT */                     nullptr,
/* RES_OPAQUE */                    nullptr,
/* RES_PROTECT */                   nullptr,
/* RES_SURROUND */                  nullptr,
/* RES_VERT_ORIENT */               nullptr,
/* RES_HORI_ORIENT */               nullptr,
/* RES_ANCHOR */                    nullptr,
/* RES_BACKGROUND */                OutCSS1_SvxBrush,
/* RES_BOX  */                      OutCSS1_SvxBox,
/* RES_SHADOW */                    nullptr,
/* RES_FRMMACRO */                  nullptr,
/* RES_COL */                       nullptr,
/* RES_KEEP */                      nullptr,
/* RES_URL */                       nullptr,
/* RES_EDIT_IN_READONLY */          nullptr,
/* RES_LAYOUT_SPLIT */              nullptr,
/* RES_CHAIN */                     nullptr,
/* RES_TEXTGRID */                  nullptr,
/* RES_LINENUMBER */                nullptr,
/* RES_FTN_AT_TXTEND */             nullptr,
/* RES_END_AT_TXTEND */             nullptr,
/* RES_COLUMNBALANCE */             nullptr,
/* RES_FRAMEDIR */                  OutCSS1_SvxFrameDirection,
/* RES_HEADER_FOOTER_EAT_SPACING */ nullptr,
/* RES_ROW_SPLIT */                 nullptr,
/* RES_FLY_SPLIT */                 nullptr,
/* RES_FOLLOW_TEXT_FLOW */          nullptr,
/* RES_COLLAPSING_BORDERS */        nullptr,
/* RES_WRAP_INFLUENCE_ON_OBJPOS */  nullptr,
/* RES_AUTO_STYLE */                nullptr,
/* RES_FRMATR_STYLE_NAME */         nullptr,
/* RES_FRMATR_CONDITIONAL_STYLE_NAME */ nullptr,
/* RES_FRMATR_GRABBAG */            nullptr,
/* RES_TEXT_VERT_ADJUST */          nullptr,
/* RES_BACKGROUND_FULL_SIZE */      nullptr,
/* RES_RTL_GUTTER */                nullptr,
/* RES_DECORATIVE */                nullptr,

/* RES_GRFATR_MIRRORGRF */          nullptr,
/* RES_GRFATR_CROPGRF   */          nullptr,
/* RES_GRFATR_ROTATION */           nullptr,
/* RES_GRFATR_LUMINANCE */          nullptr,
/* RES_GRFATR_CONTRAST */           nullptr,
/* RES_GRFATR_CHANNELR */           nullptr,
/* RES_GRFATR_CHANNELG */           nullptr,
/* RES_GRFATR_CHANNELB */           nullptr,
/* RES_GRFATR_GAMMA */              nullptr,
/* RES_GRFATR_INVERT */             nullptr,
/* RES_GRFATR_TRANSPARENCY */       nullptr,
/* RES_GRFATR_DRWAMODE */           nullptr,
/* RES_GRFATR_DUMMY3 */             nullptr,
/* RES_GRFATR_DUMMY4 */             nullptr,
/* RES_GRFATR_DUMMY5 */             nullptr,

/* RES_BOXATR_FORMAT */             nullptr,
/* RES_BOXATR_FORMULA */            nullptr,
/* RES_BOXATR_VALUE */              nullptr
};

static_assert(SAL_N_ELEMENTS(aCSS1AttrFnTab) == RES_BOXATR_END);

void SwHTMLWriter::OutCSS1_SfxItemSet( const SfxItemSet& rItemSet,
                                       bool bDeep, std::string_view rAdd )
{
    // print ItemSet, including all attributes
    Out_SfxItemSet( aCSS1AttrFnTab, *this, rItemSet, bDeep );

    // some Attributes require special treatment

    // Underline, Overline, CrossedOut and Blink form together a CSS1-Property
    // (doesn't work of course for Hints)
    if( !IsCSS1Source(CSS1_OUTMODE_HINT) )
    {
        const SvxUnderlineItem *pUnderlineItem =
            rItemSet.GetItemIfSet( RES_CHRATR_UNDERLINE, bDeep );

        const SvxOverlineItem *pOverlineItem =
            rItemSet.GetItemIfSet( RES_CHRATR_OVERLINE, bDeep );

        const SvxCrossedOutItem *pCrossedOutItem =
            rItemSet.GetItemIfSet( RES_CHRATR_CROSSEDOUT, bDeep );

        const SvxBlinkItem *pBlinkItem =
            rItemSet.GetItemIfSet( RES_CHRATR_BLINK, bDeep );

        if( pUnderlineItem || pOverlineItem || pCrossedOutItem || pBlinkItem )
            OutCSS1_SvxTextLn_SvxCrOut_SvxBlink( *this, pUnderlineItem,
                                                 pOverlineItem,
                                                 pCrossedOutItem,
                                                 pBlinkItem );

        OutCSS1_SvxFormatBreak_SwFormatPDesc_SvxFormatKeep( *this, rItemSet, bDeep );
    }

    if (!rAdd.empty())
    {
        for (std::size_t index = 0; index != std::string_view::npos;)
        {
            std::string_view attr = o3tl::trim(o3tl::getToken(rAdd, ':', index));
            assert(!attr.empty());
            assert(index != std::string_view::npos);

            std::string_view val = o3tl::trim(o3tl::getToken(rAdd, ':', index));
            assert(!val.empty());
            OutCSS1_PropertyAscii(attr, val);
        }
    }

    if( m_bFirstCSS1Property )
        return;

    // if a Property was exported as part of a Style-Option,
    // the Option still needs to be finished
    OStringBuffer sOut;
    switch( m_nCSS1OutMode & CSS1_OUTMODE_ANY_OFF )
    {
    case CSS1_OUTMODE_SPAN_TAG_OFF:
        sOut.append(sCSS1_span_tag_end);
        break;

    case CSS1_OUTMODE_STYLE_OPT_OFF:
        sOut.append(cCSS1_style_opt_end);
        break;

    case CSS1_OUTMODE_RULE_OFF:
        sOut.append(sCSS1_rule_end);
        break;
    }
    if (!sOut.isEmpty())
        Strm().WriteOString( sOut );
}

SwHTMLWriter& OutCSS1_HintSpanTag( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_SPAN_TAG |
                                   CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_HINT, nullptr );

    Out( aCSS1AttrFnTab, rHt, rWrt );

    if( !rWrt.m_bFirstCSS1Property  && rWrt.m_bTagOn )
        rWrt.Strm().WriteOString( sCSS1_span_tag_end );

    return rWrt;
}

SwHTMLWriter& OutCSS1_HintStyleOpt( SwHTMLWriter& rWrt, const SfxPoolItem& rHt )
{
    SwCSS1OutMode aMode( rWrt, CSS1_OUTMODE_STYLE_OPT_ON |
                                   CSS1_OUTMODE_ENCODE|
                                   CSS1_OUTMODE_HINT, nullptr );

    Out( aCSS1AttrFnTab, rHt, rWrt );

    if( !rWrt.m_bFirstCSS1Property )
        rWrt.Strm().WriteChar( '\"' );

    return rWrt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
