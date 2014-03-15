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

#include "hintids.hxx"
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
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
#include <sfx2/htmlmode.hxx>
#include <svl/urihelper.hxx>
#include <tools/urlobj.hxx>
#include <tools/bigint.hxx>
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
// OTES
#include <ftninfo.hxx>
#include <ftnidx.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
// FOOTNOTES
#include "doc.hxx"
#include "swerror.h"
#include "charatr.hxx"
#include "paratr.hxx"
#include "frmatr.hxx"
#include "poolfmt.hxx"
#include "fltini.hxx"
#include "css1kywd.hxx"
#include "wrthtml.hxx"
#include "htmlnum.hxx"

#include <IDocumentStylePoolAccess.hxx>
#include <numrule.hxx>

#include <rtl/strbuf.hxx>

using namespace ::com::sun::star;
using ::editeng::SvxBorderLine;

#define HTML_HEADSPACE (12*20)

#define CSS1_BACKGROUND_ATTR    1
#define CSS1_BACKGROUND_PAGE    2
#define CSS1_BACKGROUND_TABLE   3
#define CSS1_BACKGROUND_FLY     4
#define CSS1_BACKGROUND_SECTION 5

#define CSS1_FRMSIZE_WIDTH      0x01
#define CSS1_FRMSIZE_VARHEIGHT  0x02
#define CSS1_FRMSIZE_MINHEIGHT  0x04
#define CSS1_FRMSIZE_FIXHEIGHT  0x08
#define CSS1_FRMSIZE_ANYHEIGHT  0x0e
#define CSS1_FRMSIZE_PIXEL      0x10

const sal_Char* sCSS1_rule_end      = " }";
const sal_Char* sCSS1_span_tag_end  = "\">";
const sal_Char cCSS1_style_opt_end  = '\"';

const sal_Char* sHTML_FTN_fontheight = "57%";

extern SwAttrFnTab aCSS1AttrFnTab;

static Writer& OutCSS1_SwFmt( Writer& rWrt, const SwFmt& rFmt,
                              IDocumentStylePoolAccess /*SwDoc*/ *pDoc, SwDoc *pTemplate );
static Writer& OutCSS1_SwPageDesc( Writer& rWrt, const SwPageDesc& rFmt,
                                   IDocumentStylePoolAccess /*SwDoc*/ *pDoc, SwDoc *pTemplate,
                                   sal_uInt16 nRefPoolId, bool bExtRef,
                                   bool bPseudo=true );
static Writer& OutCSS1_SwFtnInfo( Writer& rWrt, const SwEndNoteInfo& rInfo,
                                  SwDoc *pDoc, sal_uInt16 nNotes, bool bEndNote );
static void OutCSS1_SwFmtDropAttrs( SwHTMLWriter& rHWrt,
                                    const SwFmtDrop& rDrop,
                                     const SfxItemSet *pCharFmtItemSet=0 );
static Writer& OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink( Writer& rWrt,
                    const SvxUnderlineItem *pUItem,
                    const SvxOverlineItem *pOItem,
                    const SvxCrossedOutItem *pCOItem,
                    const SvxBlinkItem *pBItem );
static Writer& OutCSS1_SvxFontWeight( Writer& rWrt, const SfxPoolItem& rHt );
static Writer& OutCSS1_SvxPosture( Writer& rWrt, const SfxPoolItem& rHt );
static Writer& OutCSS1_SvxULSpace( Writer& rWrt, const SfxPoolItem& rHt );
static Writer& OutCSS1_SvxLRSpace( Writer& rWrt, const SfxPoolItem& rHt );
static Writer& OutCSS1_SvxULSpace_SvxLRSpace( Writer& rWrt,
                                        const SvxULSpaceItem *pULSpace,
                                        const SvxLRSpaceItem *pLRSpace );
static Writer& OutCSS1_SvxULSpace_SvxLRSpace( Writer& rWrt,
                                        const SfxItemSet& rItemSet,
                                        sal_Bool bDeep );
static Writer& OutCSS1_SvxBrush( Writer& rWrt, const SfxPoolItem& rHt,
                                 sal_uInt16 nMode );
static Writer& OutCSS1_SvxBrush( Writer& rWrt, const SfxPoolItem& rHt );
static Writer& OutCSS1_SwFmtFrmSize( Writer& rWrt, const SfxPoolItem& rHt,
                                     sal_uInt16 nMode );
static Writer& OutCSS1_SvxFmtBreak_SwFmtPDesc_SvxFmtKeep( Writer& rWrt,
                                        const SfxItemSet& rItemSet,
                                        sal_Bool bDeep );
static Writer& OutCSS1_SwFmtLayoutSplit( Writer& rWrt, const SfxPoolItem& rHt );

static OString ConvToHex(sal_uInt16 nHex)
{
    sal_Char aNToABuf[] = "00";

    // set pointer to end of buffer
    sal_Char *pStr = aNToABuf + (sizeof(aNToABuf)-1);
    for( sal_uInt8 n = 0; n < 2; ++n )
    {
        *(--pStr) = (sal_Char)(nHex & 0xf ) + 48;
        if( *pStr > '9' )
            *pStr += 39;
        nHex >>= 4;
    }

    return OString(aNToABuf, 2);
}

static OString GetCSS1Color(const Color& rColor)
{
    return "#" + ConvToHex(rColor.GetRed()) + ConvToHex(rColor.GetGreen()) + ConvToHex(rColor.GetBlue());
}

class SwCSS1OutMode
{
    SwHTMLWriter& rWrt;
    sal_uInt16 nOldMode;

public:

    SwCSS1OutMode( SwHTMLWriter& rHWrt, sal_uInt16 nMode, bool bStartFirst=true,
                   const OUString *pSelector=0 ) :
        rWrt( rHWrt ),
        nOldMode( rHWrt.nCSS1OutMode )
    {
        rWrt.nCSS1OutMode = nMode;
        if( bStartFirst )
            rWrt.bFirstCSS1Property = sal_True;
        if( pSelector )
            rWrt.aCSS1Selector = *pSelector;
    }

    ~SwCSS1OutMode()
    {
        rWrt.nCSS1OutMode = nOldMode;
    }
};

void SwHTMLWriter::OutCSS1_Property( const sal_Char *pProp,
                                     const sal_Char *pVal,
                                     const OUString *pSVal )
{
    OStringBuffer sOut;

    if( bFirstCSS1Rule && (nCSS1OutMode & CSS1_OUTMODE_RULE_ON)!=0 )
    {
        bFirstCSS1Rule = sal_False;
        OutNewLine();
        sOut.append("<" + OString(OOO_STRING_SVTOOLS_HTML_style) + " " +
                    OString(OOO_STRING_SVTOOLS_HTML_O_type) + "=\"text/css\">");
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

        IncIndentLevel();
    }

    if( bFirstCSS1Property )
    {
        switch( nCSS1OutMode & CSS1_OUTMODE_ANY_ON )
        {
        case CSS1_OUTMODE_SPAN_TAG_ON:
        case CSS1_OUTMODE_SPAN_TAG1_ON:
            if( bTagOn )
            {
                sOut.append("<" + OString(OOO_STRING_SVTOOLS_HTML_span) +
                            " " + OString(OOO_STRING_SVTOOLS_HTML_O_style) + "=\"");
            }
            else
            {
                HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_span, false );
                return;
            }
            break;

        case CSS1_OUTMODE_RULE_ON:
            {
                OutNewLine();
                sOut.append(OUStringToOString(aCSS1Selector, eDestEnc) + " { ");
            }
            break;

        case CSS1_OUTMODE_STYLE_OPT_ON:
            sOut.append(" " + OString(OOO_STRING_SVTOOLS_HTML_O_style) + "=\"");
            break;
        }
        bFirstCSS1Property = sal_False;
    }
    else
    {
        sOut.append("; ");
    }

    sOut.append(OString(pProp) + ": ");
    if( nCSS1OutMode & CSS1_OUTMODE_ENCODE )
    {
        // for STYLE-Option encode string
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        if( pVal )
            HTMLOutFuncs::Out_String( Strm(), OUString::createFromAscii(pVal),
                                      eDestEnc, &aNonConvertableCharacters );
        else if( pSVal )
            HTMLOutFuncs::Out_String( Strm(), *pSVal, eDestEnc, &aNonConvertableCharacters );
    }
    else
    {
        // for STYLE-Tag print string directly
        if( pVal )
            sOut.append(pVal);
        else if( pSVal )
            sOut.append(OUStringToOString(*pSVal, eDestEnc));
    }

    if (!sOut.isEmpty())
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
}

static void AddUnitPropertyValue(OStringBuffer &rOut, long nVal,
    FieldUnit eUnit)
{
    if( nVal < 0 )
    {
        // special-case sign symbol
        nVal = -nVal;
        rOut.append('-');
    }

    // the recalculated unit results from (x * nMul)/(nDiv*nFac*10)
    long nMul = 1000;
    long nDiv = 1;
    long nFac = 100;
    const sal_Char *pUnit;
    switch( eUnit )
    {
    case FUNIT_100TH_MM:
        OSL_ENSURE( FUNIT_MM == eUnit, "Measuring unit not supported" );
    case FUNIT_MM:
        // 0.01mm = 0.57twip
        nMul = 25400;   // 25.4 * 1000
        nDiv = 1440;    // 72 * 20;
        nFac = 100;
        pUnit = sCSS1_UNIT_mm;
        break;

    case FUNIT_M:
    case FUNIT_KM:
        OSL_ENSURE( FUNIT_CM == eUnit, "Measuring unit not supported" );
    case FUNIT_CM:
        // 0.01cm = 5.7twip (not exact, but the UI is also not exact)
        nMul = 2540;    // 2.54 * 1000
        nDiv = 1440;    // 72 * 20;
        nFac = 100;
        pUnit = sCSS1_UNIT_cm;
        break;

    case FUNIT_TWIP:
        OSL_ENSURE( FUNIT_POINT == eUnit, "Measuring unit not supported" );
    case FUNIT_POINT:
        // 0.1pt = 2.0twip (not exact, but the UI is also not exact)
        nMul = 100;
        nDiv = 20;
        nFac = 10;
        pUnit = sCSS1_UNIT_pt;
        break;

    case FUNIT_PICA:
        // 0.01pc = 2.40twip (not exact, but the UI is also not exact)
        nMul = 1000;
        nDiv = 240;     // 12 * 20;
        nFac = 100;
        pUnit = sCSS1_UNIT_pc;
        break;

    case FUNIT_NONE:
    case FUNIT_FOOT:
    case FUNIT_MILE:
    case FUNIT_CUSTOM:
    case FUNIT_PERCENT:
    case FUNIT_INCH:
    default:
        OSL_ENSURE( FUNIT_INCH == eUnit, "Measuring unit not supported" );
        // 0.01in = 14.4twip (not exact, but the UI is also not exact)
        nMul = 1000;
        nDiv = 1440;    // 72 * 20;
        nFac = 100;
        pUnit = sCSS1_UNIT_inch;
        break;
    }

    long nLongVal = 0;
    bool bOutLongVal = true;
    if( nVal > LONG_MAX / nMul )
    {
        // needs a BigInt to translate this unit
#ifdef SAL_INT64_IS_STRUCT
        BigInt nBigVal( nVal );
        nBigVal *= nMul;
        nBigVal /= nDiv;
        nBigVal += 5;
        nBigVal /= 10;

        if( nBigVal.IsLong() )
        {
            // a long is sufficient
            nLongVal = (long)nBigVal;
        }
        else
        {
            BigInt nBigFac( nFac );
            BigInt nBig10( 10 );
            rOut += (long)(nBigVal / nBigFac);
            if( !(nBigVal % nBigFac).IsZero() )
            {
                rOut.append('.');
                while( nFac > 1 && !(nBigVal % nBigFac).IsZero() )
                {
                    nFac /= 10;
                    nBigFac = nFac;
                    rOut.append(OString::number((nBigVal / nBigFac) % nBig10));
                }
            }
            bOutLongVal = false;
        }
#else
        sal_Int64 nBigVal( nVal );
        nBigVal *= nMul;
        nBigVal /= nDiv;
        nBigVal += 5;
        nBigVal /= 10;

        if( nBigVal <= LONG_MAX )
        {
            // a long is sufficient
            nLongVal = (long)nBigVal;
        }
        else
        {
            rOut.append(nBigVal / (sal_Int64)nFac);
            if( (nBigVal % (sal_Int64)nFac) != 0 )
            {
                rOut.append('.');
                while( nFac > 1 && (nBigVal % (sal_Int64)nFac) != 0 )
                {
                    nFac /= 10;
                    rOut.append((nBigVal / (sal_Int64)nFac) % (sal_Int64)10);
                }
            }
            bOutLongVal = false;
        }
#endif
    }
    else
    {
        nLongVal = nVal * nMul;
        nLongVal /= nDiv;
        nLongVal += 5;
        nLongVal /= 10;
    }

    if( bOutLongVal )
    {
        rOut.append(OString::number(nLongVal/nFac));
        if( (nLongVal % nFac) != 0 )
        {
            rOut.append('.');
            while( nFac > 1 && (nLongVal % nFac) != 0 )
            {
                nFac /= 10;
                rOut.append(OString::number((nLongVal / nFac) % 10));
            }
        }
    }

    rOut.append(pUnit);
}

void SwHTMLWriter::OutCSS1_UnitProperty( const sal_Char *pProp, long nVal )
{
    OStringBuffer sOut;
    AddUnitPropertyValue(sOut, nVal, eCSS1Unit);
    OutCSS1_PropertyAscii(pProp, sOut.makeStringAndClear());
}

void SwHTMLWriter::OutCSS1_PixelProperty( const sal_Char *pProp, long nVal,
                                          sal_Bool bVert )
{
    OString sOut(OString::number(ToPixel(nVal,bVert)) + sCSS1_UNIT_px);
    OutCSS1_PropertyAscii(pProp, sOut);
}

void SwHTMLWriter::OutCSS1_SfxItemSet( const SfxItemSet& rItemSet,
                                       sal_Bool bDeep )
{
    // print ItemSet, including all attributes
    Out_SfxItemSet( aCSS1AttrFnTab, *this, rItemSet, bDeep );

    // some Attributes require special treatment
    const SfxPoolItem *pItem = 0;

    // Underline, Overline, CrossedOut and Blink form together a CSS1-Property
    // (doesn't work of course for Hints)
    if( !IsCSS1Source(CSS1_OUTMODE_HINT) )
    {
        const SvxUnderlineItem *pUnderlineItem = 0;
        if( SFX_ITEM_SET==rItemSet.GetItemState( RES_CHRATR_UNDERLINE, bDeep, &pItem ))
            pUnderlineItem = (const SvxUnderlineItem *)pItem;

        const SvxOverlineItem *pOverlineItem = 0;
        if( SFX_ITEM_SET==rItemSet.GetItemState( RES_CHRATR_OVERLINE, bDeep, &pItem ))
            pOverlineItem = (const SvxOverlineItem *)pItem;

        const SvxCrossedOutItem *pCrossedOutItem = 0;
        if( SFX_ITEM_SET==rItemSet.GetItemState( RES_CHRATR_CROSSEDOUT, bDeep, &pItem ))
            pCrossedOutItem = (const SvxCrossedOutItem *)pItem;

        const SvxBlinkItem *pBlinkItem = 0;
        if( SFX_ITEM_SET==rItemSet.GetItemState( RES_CHRATR_BLINK, bDeep, &pItem ))
            pBlinkItem = (const SvxBlinkItem *)pItem;

        if( pUnderlineItem || pOverlineItem || pCrossedOutItem || pBlinkItem )
            OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink( *this, pUnderlineItem,
                                                 pOverlineItem,
                                                 pCrossedOutItem,
                                                 pBlinkItem );

        OutCSS1_SvxFmtBreak_SwFmtPDesc_SvxFmtKeep( *this, rItemSet, bDeep );
    }

    if( !bFirstCSS1Property )
    {
        // if a Property was exported as part of a Style-Option,
        // the Option still needs to be finished
        OStringBuffer sOut;
        switch( nCSS1OutMode & CSS1_OUTMODE_ANY_OFF )
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
            Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    }
}

void SwHTMLWriter::OutStyleSheet( const SwPageDesc& rPageDesc, sal_Bool bUsed )
{
    bFirstCSS1Rule = sal_True;

// Feature: PrintExt
    if( IsHTMLMode(HTMLMODE_PRINT_EXT) )
    {
        const SwPageDesc *pFirstPageDesc = 0;
        sal_uInt16 nFirstRefPoolId = RES_POOLPAGE_HTML;
        bCSS1IgnoreFirstPageDesc = sal_True;

        // First we try to guess how the document is constructed.
        // Allowed are only the templates: HTML, 1st page, left page, and right page.
        // A first page is only exported, if it matches the template "1st page".
        // Left and right pages are only exported, if their templates are linked.
        // If other templates are used, only very simple cases are exported.
        const SwPageDesc *pPageDesc = &rPageDesc;
        const SwPageDesc *pFollow = rPageDesc.GetFollow();
        if( RES_POOLPAGE_FIRST == pPageDesc->GetPoolFmtId() &&
            pFollow != pPageDesc &&
            !IsPoolUserFmt( pFollow->GetPoolFmtId() ) )
        {
            // the document has a first page
            pFirstPageDesc = pPageDesc;
            pPageDesc = pFollow;
            pFollow = pPageDesc->GetFollow();
        }

        IDocumentStylePoolAccess* pStylePoolAccess = getIDocumentStylePoolAccess();
        if( pPageDesc == pFollow )
        {
            // The document is one-sided; no matter what page, we do not create a 2-sided doc.
            // The attribute is exported relative to the HTML page template.
          OutCSS1_SwPageDesc( *this, *pPageDesc, pStylePoolAccess, pTemplate,
                                RES_POOLPAGE_HTML, true, false );
            nFirstRefPoolId = pFollow->GetPoolFmtId();
        }
        else if( (RES_POOLPAGE_LEFT == pPageDesc->GetPoolFmtId() &&
                  RES_POOLPAGE_RIGHT == pFollow->GetPoolFmtId()) ||
                 (RES_POOLPAGE_RIGHT == pPageDesc->GetPoolFmtId() &&
                  RES_POOLPAGE_LEFT == pFollow->GetPoolFmtId()) )
        {
            // the document is double-sided
          OutCSS1_SwPageDesc( *this, *pPageDesc, pStylePoolAccess, pTemplate,
                                RES_POOLPAGE_HTML, true );
          OutCSS1_SwPageDesc( *this, *pFollow, pStylePoolAccess, pTemplate,
                                RES_POOLPAGE_HTML, true );
            nFirstRefPoolId = RES_POOLPAGE_RIGHT;
            bCSS1IgnoreFirstPageDesc = sal_False;
        }
        // other cases we miss

        if( pFirstPageDesc )
          OutCSS1_SwPageDesc( *this, *pFirstPageDesc, pStylePoolAccess, pTemplate,
                                nFirstRefPoolId, false );
    }
// /Feature: PrintExt

    // The text body style has to be exported always (if it is changed compared
    // to the template), because it is used as reference for any style
    // that maps to <P>, and that's especially the standard style
    getIDocumentStylePoolAccess()->GetTxtCollFromPool( RES_POOLCOLL_TEXT, false );

    // the Default-TextStyle is not also exported !!
    // 0-Style is the Default; is never exported !!
    sal_uInt16 nArrLen = pDoc->GetTxtFmtColls()->size();
    sal_uInt16 i;

    for( i = 1; i < nArrLen; i++ )
    {
        const SwTxtFmtColl* pColl = (*pDoc->GetTxtFmtColls())[i];
        sal_uInt16 nPoolId = pColl->GetPoolFmtId();
        if( !bUsed || nPoolId == RES_POOLCOLL_TEXT ||
            pDoc->IsUsed( *pColl ) )
            OutCSS1_SwFmt( *this, *pColl, pDoc, pTemplate );
    }

    // the Default-TextStyle is not also exported !!
    nArrLen = pDoc->GetCharFmts()->size();
    for( i=1; i<nArrLen; i++ )
    {
        const SwCharFmt *pCFmt = (*pDoc->GetCharFmts())[i];
        sal_uInt16 nPoolId = pCFmt->GetPoolFmtId();
        if( !bUsed || nPoolId == RES_POOLCHR_INET_NORMAL ||
            nPoolId == RES_POOLCHR_INET_VISIT ||
            pDoc->IsUsed( *pCFmt ) )
            OutCSS1_SwFmt( *this, *pCFmt, pDoc, pTemplate );
    }

    const SwFtnIdxs& rIdxs = pDoc->GetFtnIdxs();
    nArrLen = rIdxs.size();
    sal_uInt16 nEnd = 0, nFtn = 0;
    for( i=0; i < nArrLen; i++ )
    {
        if( rIdxs[i]->GetFtn().IsEndNote() )
            nEnd++;
        else
            nFtn++;
    }
    OutCSS1_SwFtnInfo( *this, pDoc->GetFtnInfo(), pDoc, nFtn, false );
    OutCSS1_SwFtnInfo( *this, pDoc->GetEndNoteInfo(), pDoc, nEnd, true );

    if( !bFirstCSS1Rule )
    {
        DecIndentLevel();

        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_style, false );
    }
    else
    {
        bFirstCSS1Rule = sal_False;
    }

    nDfltTopMargin = 0;
    nDfltBottomMargin = 0;
}

// if pPseudo is set, Styles-Sheets will be exported;
// otherwise we only search for Token and Class for a Format
sal_uInt16 SwHTMLWriter::GetCSS1Selector( const SwFmt *pFmt, OString& rToken,
                                      OUString& rClass, sal_uInt16& rRefPoolId,
                                      OUString *pPseudo )
{
    sal_uInt16 nDeep = 0;
    rToken = OString();
    rClass = "";
    rRefPoolId = 0;
    if( pPseudo )
        *pPseudo = "";

    sal_Bool bChrFmt = RES_CHRFMT==pFmt->Which();

    // search formats above for the nearest standard or HTML-Tag template
    const SwFmt *pPFmt = pFmt;
    while( pPFmt && !pPFmt->IsDefault() )
    {
        bool bStop = false;
        sal_uInt16 nPoolId = pPFmt->GetPoolFmtId();
        if( USER_FMT & nPoolId )
        {
            // user templates
            const OUString aNm(pPFmt->GetName());

            if (!bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_blockquote)
            {
                rRefPoolId = RES_POOLCOLL_HTML_BLOCKQUOTE;
                rToken = OString(OOO_STRING_SVTOOLS_HTML_blockquote);
            }
            else if (bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_citiation)
            {
                rRefPoolId = RES_POOLCHR_HTML_CITIATION;
                rToken = OString(OOO_STRING_SVTOOLS_HTML_citiation);
            }
            else if (bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_code)
            {
                rRefPoolId = RES_POOLCHR_HTML_CODE;
                rToken = OString(OOO_STRING_SVTOOLS_HTML_code);
            }
            else if (bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_definstance)
            {
                rRefPoolId = RES_POOLCHR_HTML_DEFINSTANCE;
                rToken = OString(OOO_STRING_SVTOOLS_HTML_definstance);
            }
            else if (!bChrFmt && (aNm == OOO_STRING_SVTOOLS_HTML_dd ||
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
                        rToken = OString(OOO_STRING_SVTOOLS_HTML_dd);
                    }
                    else
                    {
                        rRefPoolId = RES_POOLCOLL_HTML_DT;
                        rToken = OString(OOO_STRING_SVTOOLS_HTML_dt);
                    }
                }
            }
            else if (bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_emphasis)
            {
                rRefPoolId = RES_POOLCHR_HTML_EMPHASIS;
                rToken = OString(OOO_STRING_SVTOOLS_HTML_emphasis);
            }
            else if (!bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_horzrule)
            {
                // do not export HR !
                bStop = (nDeep==0);
            }
            else if (bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_keyboard)
            {
                rRefPoolId = RES_POOLCHR_HTML_KEYBOARD;
                rToken = OString(OOO_STRING_SVTOOLS_HTML_keyboard);
            }
            else if (!bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_listing)
            {
                // Export Listings as PRE or PRE-derived template
                rToken = OString(OOO_STRING_SVTOOLS_HTML_preformtxt);
                rRefPoolId = RES_POOLCOLL_HTML_PRE;
                nDeep = CSS1_FMT_CMPREF;
            }
            else if (!bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_preformtxt)
            {
                rRefPoolId = RES_POOLCOLL_HTML_PRE;
                rToken = OString(OOO_STRING_SVTOOLS_HTML_preformtxt);
            }
            else if (bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_sample)
            {
                rRefPoolId = RES_POOLCHR_HTML_SAMPLE;
                rToken = OString(OOO_STRING_SVTOOLS_HTML_sample);
            }
            else if (bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_strong)
            {
                rRefPoolId = RES_POOLCHR_HTML_STRONG;
                rToken = OString(OOO_STRING_SVTOOLS_HTML_strong);
            }
            else if (bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_teletype)
            {
                rRefPoolId = RES_POOLCHR_HTML_TELETYPE;
                rToken = OString(OOO_STRING_SVTOOLS_HTML_teletype);
            }
            else if (bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_variable)
            {
                rRefPoolId = RES_POOLCHR_HTML_VARIABLE;
                rToken = OString(OOO_STRING_SVTOOLS_HTML_variable);
            }
            else if (!bChrFmt && aNm == OOO_STRING_SVTOOLS_HTML_xmp)
            {
                // export XMP as PRE (but not the template as Style)
                rToken = OString(OOO_STRING_SVTOOLS_HTML_preformtxt);
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
                bStop = (nDeep==0);
                break;
            case RES_POOLCOLL_TEXT:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_parabreak);
                break;
            case RES_POOLCOLL_HEADLINE1:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_head1);
                break;
            case RES_POOLCOLL_HEADLINE2:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_head2);
                break;
            case RES_POOLCOLL_HEADLINE3:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_head3);
                break;
            case RES_POOLCOLL_HEADLINE4:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_head4);
                break;
            case RES_POOLCOLL_HEADLINE5:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_head5);
                break;
            case RES_POOLCOLL_HEADLINE6:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_head6);
                break;
            case RES_POOLCOLL_SENDADRESS:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_address);
                break;
            case RES_POOLCOLL_HTML_BLOCKQUOTE:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_blockquote);
                break;
            case RES_POOLCOLL_HTML_PRE:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_preformtxt);
                break;

            case RES_POOLCOLL_HTML_DD:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_dd);
                break;
            case RES_POOLCOLL_HTML_DT:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_dt);
                break;

            case RES_POOLCOLL_TABLE:
                if( pPseudo )
                {
                    rToken = OString(OOO_STRING_SVTOOLS_HTML_tabledata) + " " +
                             OString(OOO_STRING_SVTOOLS_HTML_parabreak);
                }
                else
                    rToken = OOO_STRING_SVTOOLS_HTML_parabreak;
                break;
            case RES_POOLCOLL_TABLE_HDLN:
                if( pPseudo )
                {
                    rToken = OString(OOO_STRING_SVTOOLS_HTML_tableheader) + " " +
                             OString(OOO_STRING_SVTOOLS_HTML_parabreak);
                }
                else
                    rToken = OString(OOO_STRING_SVTOOLS_HTML_parabreak);
                break;
            case RES_POOLCOLL_HTML_HR:
                // do not exort HR !
                bStop = (nDeep==0);
                break;
            case RES_POOLCOLL_FOOTNOTE:
                if( !nDeep )
                {
                    rToken = OString(OOO_STRING_SVTOOLS_HTML_parabreak);
                    rClass = OOO_STRING_SVTOOLS_HTML_sdfootnote;
                    rRefPoolId = RES_POOLCOLL_TEXT;
                    nDeep = CSS1_FMT_CMPREF;
                }
                break;
            case RES_POOLCOLL_ENDNOTE:
                if( !nDeep )
                {
                    rToken = OString(OOO_STRING_SVTOOLS_HTML_parabreak);
                    rClass = OOO_STRING_SVTOOLS_HTML_sdendnote;
                    rRefPoolId = RES_POOLCOLL_TEXT;
                    nDeep = CSS1_FMT_CMPREF;
                }
                break;

            // character templates
            case RES_POOLCHR_HTML_EMPHASIS:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_emphasis);
                break;
            case RES_POOLCHR_HTML_CITIATION:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_citiation);
                break;
            case RES_POOLCHR_HTML_STRONG:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_strong);
                break;
            case RES_POOLCHR_HTML_CODE:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_code);
                break;
            case RES_POOLCHR_HTML_SAMPLE:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_sample);
                break;
            case RES_POOLCHR_HTML_KEYBOARD:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_keyboard);
                break;
            case RES_POOLCHR_HTML_VARIABLE:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_variable);
                break;
            case RES_POOLCHR_HTML_DEFINSTANCE:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_definstance);
                break;
            case RES_POOLCHR_HTML_TELETYPE:
                rToken = OString(OOO_STRING_SVTOOLS_HTML_teletype);
                break;

            case RES_POOLCHR_INET_NORMAL:
                if( pPseudo )
                {
                    rToken = OString(OOO_STRING_SVTOOLS_HTML_anchor);
                    *pPseudo = OStringToOUString( sCSS1_link, RTL_TEXTENCODING_ASCII_US );
                }
                break;
            case RES_POOLCHR_INET_VISIT:
                if( pPseudo )
                {
                    rToken = OString(OOO_STRING_SVTOOLS_HTML_anchor);
                    *pPseudo = OStringToOUString( sCSS1_visited, RTL_TEXTENCODING_ASCII_US );
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
            pPFmt = pPFmt->DerivedFrom();
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
        rClass = pFmt->GetName();
        sal_Int32 nPos = rClass.indexOf( '.' );
        if( nPos >= 0 && rClass.getLength() > nPos+1 )
        {
            rClass = rClass.replaceAt( 0, nPos+1, "" );
        }

        rClass = GetAppCharClass().lowercase( rClass );
        rClass = rClass.replaceAll( ".", "-" );
        rClass = rClass.replaceAll( " ", "-" );
        rClass = rClass.replaceAll( "_", "-" );
    }

    return nDeep;
}

static sal_uInt16 GetCSS1Selector( const SwFmt *pFmt, OUString& rSelector,
                               sal_uInt16& rRefPoolId )
{
    OString aToken;
    OUString aClass;
    OUString aPseudo;

    sal_uInt16 nDeep = SwHTMLWriter::GetCSS1Selector( pFmt, aToken, aClass,
                                                  rRefPoolId, &aPseudo );
    if( nDeep )
    {
        if( !aToken.isEmpty() )
            rSelector = OStringToOUString(aToken, RTL_TEXTENCODING_ASCII_US);
        else
            rSelector = "";

        if( !aClass.isEmpty() )
            rSelector += "." + aClass;
        if( !aPseudo.isEmpty() )
            rSelector += ":" + aPseudo;
    }

    return nDeep;
}

const SwFmt *SwHTMLWriter::GetTemplateFmt( sal_uInt16 nPoolFmtId,
                                           IDocumentStylePoolAccess* pTemplate /*SwDoc *pTemplate*/)
{
    const SwFmt *pRefFmt = 0;

    if( pTemplate )
    {
        OSL_ENSURE( !(USER_FMT & nPoolFmtId),
                "No user templates found" );
        if( POOLGRP_NOCOLLID & nPoolFmtId )
            pRefFmt = pTemplate->GetCharFmtFromPool( nPoolFmtId );
        else
            pRefFmt = pTemplate->GetTxtCollFromPool( nPoolFmtId, false );
    }

    return pRefFmt;
}

const SwFmt *SwHTMLWriter::GetParentFmt( const SwFmt& rFmt, sal_uInt16 nDeep )
{
    OSL_ENSURE( nDeep != USHRT_MAX, "Called GetParent for HTML-template!" );
    const SwFmt *pRefFmt = 0;

    if( nDeep > 0 )
    {
        // get the pointer for the HTML-Tag template, from which the template is derived
        pRefFmt = &rFmt;
        for( sal_uInt16 i=nDeep; i>0; i-- )
            pRefFmt = pRefFmt->DerivedFrom();

        if( pRefFmt && pRefFmt->IsDefault() )
            pRefFmt = 0;
    }

    return pRefFmt;
}

bool swhtml_css1atr_equalFontItems( const SfxPoolItem& r1, const SfxPoolItem& r2 )
{
    return  ((const SvxFontItem &)r1).GetFamilyName() ==
                    ((const SvxFontItem &)r2).GetFamilyName() &&
            ((const SvxFontItem &)r1).GetFamily() ==
                    ((const SvxFontItem &)r2).GetFamily();
}

void SwHTMLWriter::SubtractItemSet( SfxItemSet& rItemSet,
                                    const SfxItemSet& rRefItemSet,
                                    sal_Bool bSetDefaults,
                                    sal_Bool bClearSame,
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
        bool bItemSet = ( SFX_ITEM_SET ==
                rItemSet.GetItemState( nWhich, false, &pItem) );
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
                bRefItemSet = ( SFX_ITEM_SET ==
                    pRefScriptItemSet->GetItemState( nWhich, true, &pRefItem) );
                break;
            default:
                bRefItemSet = ( SFX_ITEM_SET ==
                    aRefItemSet.GetItemState( nWhich, false, &pRefItem) );
                break;
            }
        }
        else
        {
            bRefItemSet = ( SFX_ITEM_SET ==
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
                // the Attribute is in both templates wit the same value
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
                rItemSet.Put( rItemSet.GetPool()->GetDefaultItem(nWhich) );
            }
        }

        nWhich = aIter.NextWhich();
    }
}

void SwHTMLWriter::PrepareFontList( const SvxFontItem& rFontItem,
                                    OUString& rNames,
                                    sal_Unicode cQuote, sal_Bool bGeneric )
{
    rNames = aEmptyOUStr;
    const OUString& rName = rFontItem.GetFamilyName();
    sal_Bool bContainsKeyword = sal_False;
    if( !rName.isEmpty() )
    {
        sal_Int32 nStrPos = 0;
        while( nStrPos != -1 )
        {
            OUString aName = rName.getToken( 0, ';', nStrPos );
            aName = comphelper::string::strip(aName, ' ');
            if( aName.isEmpty() )
                continue;

            sal_Bool bIsKeyword = sal_False;
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
                rNames += OUString( cQuote );
            rNames += aName;
            if( cQuote && !bIsKeyword )
                rNames += OUString( cQuote );
        }
    }

    if( !bContainsKeyword && bGeneric )
    {
        const sal_Char *pStr = 0;
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

        if( pStr )
        {
            if( !rNames.isEmpty() )
                rNames += ", ";
            rNames += OStringToOUString( pStr, RTL_TEXTENCODING_ASCII_US );
        }
    }
}

sal_Bool SwHTMLWriter::HasScriptDependentItems( const SfxItemSet& rItemSet,
                                                 sal_Bool bCheckDropCap )
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

    for( sal_uInt16 i=0; aWhichIds[i]; i += 3 )
    {
        const SfxPoolItem *pItem = 0, *pItemCJK = 0, *pItemCTL = 0, *pTmp;
        sal_uInt16 nItemCount = 0;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aWhichIds[i], false,
                                                   &pTmp ) )
        {
            pItem = pTmp;
            nItemCount++;
        }
        if( SFX_ITEM_SET == rItemSet.GetItemState( aWhichIds[i+1], false,
                                                   &pTmp ) )
        {
            pItemCJK = pTmp;
            nItemCount++;
        }
        if( SFX_ITEM_SET == rItemSet.GetItemState( aWhichIds[i+2], false,
                                                   &pTmp ) )
        {
            pItemCTL = pTmp;
            nItemCount++;
        }

        // If some of the items are set, but not all, we need script dependent
        // styles
        if( nItemCount > 0 && nItemCount < 3 )
            return sal_True;

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
                    return sal_True;
            }
            else
            {
                if( !( *pItem == *pItemCJK ) ||
                    !( *pItem == *pItemCTL ) ||
                    !( *pItemCJK == *pItemCTL ) )
                    return sal_True;
            }
        }
    }

    const SfxPoolItem *pItem;
    if( bCheckDropCap &&
        SFX_ITEM_SET == rItemSet.GetItemState( RES_PARATR_DROP, true,
                &pItem ) )
    {
        const SwFmtDrop *pDrop = (const SwFmtDrop *)pItem;
        const SwCharFmt *pDCCharFmt = pDrop->GetCharFmt();
        if( pDCCharFmt )
        {
            //sequence of (start, end) property ranges we want to
            //query
            SfxItemSet aTstItemSet( *pDCCharFmt->GetAttrSet().GetPool(),
                RES_CHRATR_FONT,        RES_CHRATR_FONT,
                RES_CHRATR_POSTURE,     RES_CHRATR_POSTURE,
                RES_CHRATR_WEIGHT,      RES_CHRATR_WEIGHT,
                RES_CHRATR_CJK_FONT,    RES_CHRATR_CJK_FONT,
                RES_CHRATR_CJK_POSTURE, RES_CHRATR_CJK_WEIGHT,
                RES_CHRATR_CTL_FONT,    RES_CHRATR_CTL_FONT,
                RES_CHRATR_CTL_POSTURE, RES_CHRATR_CTL_WEIGHT,
                0 );
            aTstItemSet.Set( pDCCharFmt->GetAttrSet(), true );
            return HasScriptDependentItems( aTstItemSet, sal_False );
        }
    }

    return sal_False;
}

static bool OutCSS1Rule( SwHTMLWriter& rHTMLWrt, const OUString& rSelector,
                    const SfxItemSet& rItemSet, sal_Bool bHasClass,
                     bool bCheckForPseudo  )
{
    bool bScriptDependent = false;
    if( SwHTMLWriter::HasScriptDependentItems( rItemSet, bHasClass ) )
    {
        bScriptDependent = true;
        OUString aSelector( rSelector );

        OUString aPseudo;
        if( bCheckForPseudo )
        {
            sal_Int32 nPos = aSelector.lastIndexOf( ':' );
            if( nPos >= 0 )
            {
                aPseudo = aSelector.copy( nPos );
                aSelector =aSelector.copy( 0, nPos );
            }
        }

        if( !bHasClass )
        {
            // If we are exporting styles for a tag we have to export a tag
            // rule for all properties that aren't style dependent and
            // some class rule for the additional style dependen properties
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_NO_SCRIPT|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     true, &rSelector );
                rHTMLWrt.OutCSS1_SfxItemSet( rItemSet, sal_False );
            }

            //sequence of (start, end) property ranges we want to
            //query
            SfxItemSet aScriptItemSet( *rItemSet.GetPool(),
                                       RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
                                       RES_CHRATR_LANGUAGE, RES_CHRATR_POSTURE,
                                       RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT,
                                       RES_CHRATR_CJK_FONT, RES_CHRATR_CTL_WEIGHT,
                                       0 );
            aScriptItemSet.Put( rItemSet );

            OUString aNewSelector( aSelector );
            aNewSelector += ".western" + aPseudo;
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_WESTERN|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     true, &aNewSelector );
                rHTMLWrt.OutCSS1_SfxItemSet( aScriptItemSet, sal_False );
            }

            aNewSelector = aSelector + ".cjk" + aPseudo;
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_CJK|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     true, &aNewSelector );
                rHTMLWrt.OutCSS1_SfxItemSet( aScriptItemSet, sal_False );
            }

            aNewSelector = aSelector + ".ctl" + aPseudo;
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_CTL|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     true, &aNewSelector );
                rHTMLWrt.OutCSS1_SfxItemSet( aScriptItemSet, sal_False );
            }
        }
        else
        {
            // If ther are script dependencies and we are derived from a tag,
            // when we have to export a style dependent class for all
            // scripts
            OUString aNewSelector( aSelector );
            aNewSelector += "-western" + aPseudo;
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_WESTERN|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     true, &aNewSelector );
                rHTMLWrt.OutCSS1_SfxItemSet( rItemSet, sal_False );
            }

            aNewSelector = aSelector + "-cjk" + aPseudo;
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_CJK|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     true, &aNewSelector );
                rHTMLWrt.OutCSS1_SfxItemSet( rItemSet, sal_False );
            }

            aNewSelector = aSelector + "-ctl" + aPseudo;
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_CTL|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                     true, &aNewSelector );
                rHTMLWrt.OutCSS1_SfxItemSet( rItemSet, sal_False );
            }
        }
    }
    else
    {
        // If there are no script dependencies, when all items are
        // exported in one step. For hyperlinks only, a script information
        // must be there, because these two chr formats don't support
        // script dependencies by now.
        SwCSS1OutMode aMode( rHTMLWrt,
                rHTMLWrt.nCSS1Script|CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                             true, &rSelector );
        rHTMLWrt.OutCSS1_SfxItemSet( rItemSet, sal_False );
    }

    return bScriptDependent;
}

static void OutCSS1DropCapRule(
                    SwHTMLWriter& rHTMLWrt, const OUString& rSelector,
                    const SwFmtDrop& rDrop, sal_Bool bHasClass,
                     bool bHasScriptDependencies  )
{
    const SwCharFmt *pDCCharFmt = rDrop.GetCharFmt();
    if( (bHasScriptDependencies && bHasClass) ||
         (pDCCharFmt && SwHTMLWriter::HasScriptDependentItems( pDCCharFmt->GetAttrSet(), sal_False ) ) )
    {
        OUString aSelector( rSelector );

        OUString aPseudo;
        sal_Int32 nPos = aSelector.lastIndexOf( ':' );
        if( nPos >= 0 )
        {
            aPseudo = aSelector.copy( nPos );
            aSelector = aSelector.copy( 0, nPos );
        }

        if( !bHasClass )
        {
            // If we are exporting styles for a tag we have to export a tag
            // rule for all properties that aren't style dependent and
            // some class rule for the additional style dependen properties
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_NO_SCRIPT|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     true, &rSelector );
                OutCSS1_SwFmtDropAttrs( rHTMLWrt, rDrop );
            }

            SfxItemSet aScriptItemSet( rHTMLWrt.pDoc->GetAttrPool(),
                                       RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
                                       RES_CHRATR_LANGUAGE, RES_CHRATR_POSTURE,
                                       RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT,
                                       RES_CHRATR_CJK_FONT, RES_CHRATR_CTL_WEIGHT,
                                       0 );
            if( pDCCharFmt )
                aScriptItemSet.Set( pDCCharFmt->GetAttrSet(), true );

            OUString aNewSelector( aSelector );
            aNewSelector += ".western" + aPseudo;
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_WESTERN|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     true, &aNewSelector );
                OutCSS1_SwFmtDropAttrs(  rHTMLWrt, rDrop, &aScriptItemSet );
            }

            aNewSelector = aSelector + ".cjk" + aPseudo;
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_CJK|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     true, &aNewSelector );
                OutCSS1_SwFmtDropAttrs(  rHTMLWrt, rDrop, &aScriptItemSet );
            }

            aNewSelector = aSelector + ".ctl" + aPseudo;
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_CTL|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     true, &aNewSelector );
                OutCSS1_SwFmtDropAttrs(  rHTMLWrt, rDrop, &aScriptItemSet );
            }
        }
        else
        {
            // If ther are script dependencies and we are derived from a tag,
            // when we have to export a style dependent class for all
            // scripts
            OUString aNewSelector( aSelector );
            aNewSelector += "-western" + aPseudo;
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_WESTERN|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     true, &aNewSelector );
                OutCSS1_SwFmtDropAttrs(  rHTMLWrt, rDrop );
            }

            aNewSelector = aSelector + "-cjk" + aPseudo;
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_CJK|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     true, &aNewSelector );
                OutCSS1_SwFmtDropAttrs(  rHTMLWrt, rDrop );
            }

            aNewSelector = aSelector + "-ctl" + aPseudo;
            {
                SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_CTL|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                                     true, &aNewSelector );
                OutCSS1_SwFmtDropAttrs(  rHTMLWrt, rDrop );
            }
        }
    }
    else
    {
        // If there are no script dependencies, when all items are
        // exported in one step. For hyperlinks only, a script information
        // must be there, because these two chr formats don't support
        // script dependencies by now.
        SwCSS1OutMode aMode( rHTMLWrt,
                rHTMLWrt.nCSS1Script|CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                             true, &rSelector );
                OutCSS1_SwFmtDropAttrs( rHTMLWrt, rDrop );
    }
}

static Writer& OutCSS1_SwFmt( Writer& rWrt, const SwFmt& rFmt,
                              IDocumentStylePoolAccess/*SwDoc*/ *pDoc, SwDoc *pTemplate )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    bool bCharFmt = false;
    switch( rFmt.Which() )
    {
    case RES_CHRFMT:
        bCharFmt = true;
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
    sal_uInt16 nDeep = GetCSS1Selector( &rFmt, aSelector, nRefPoolId );
    if( !nDeep )
        return rWrt;    // not derived from a HTML-template

    sal_uInt16 nPoolFmtId = rFmt.GetPoolFmtId();

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
    const SfxItemSet& rFmtItemSet = rFmt.GetAttrSet();
    SfxItemSet aItemSet( *rFmtItemSet.GetPool(), rFmtItemSet.GetRanges() );
    aItemSet.Set( rFmtItemSet, true ); // Was nDeep!=1 that is not working
                                       // for script dependent items buts should
                                       // not make a difference for any other

    sal_Bool bSetDefaults = sal_True, bClearSame = sal_True;
    const SwFmt *pRefFmt = 0;
    const SwFmt *pRefFmtScript = 0;
    switch( nDeep )
    {
    case CSS1_FMT_ISTAG:
        pRefFmt = SwHTMLWriter::GetTemplateFmt( nRefPoolId, pTemplate );
        break;
    case CSS1_FMT_CMPREF:
        pRefFmt = SwHTMLWriter::GetTemplateFmt( nRefPoolId, pDoc );
        pRefFmtScript = SwHTMLWriter::GetTemplateFmt( nRefPoolId, pTemplate );
        bClearSame = sal_False;
        break;
    default:
        pRefFmt = SwHTMLWriter::GetParentFmt( rFmt, nDeep );
        pRefFmtScript = SwHTMLWriter::GetTemplateFmt( nRefPoolId, pTemplate );
        bSetDefaults = sal_False;
        break;
    }

    if( pRefFmt )
    {
        // subtract Item-Set of the Reference template (incl. its Parents)
        SwHTMLWriter::SubtractItemSet( aItemSet, pRefFmt->GetAttrSet(),
                                       bSetDefaults, bClearSame,
                                       pRefFmtScript
                                               ? &pRefFmtScript->GetAttrSet()
                                            : 0  );

        if( !bCharFmt )
        {
            const SvxULSpaceItem& rULItem = pRefFmt->GetULSpace();
            rHTMLWrt.nDfltTopMargin = rULItem.GetUpper();
            rHTMLWrt.nDfltBottomMargin = rULItem.GetLower();
        }
    }
    else if( CSS1_FMT_ISTAG==nDeep && !bCharFmt )
    {
        // set Default-distance above and below (for the
        // case that there is no reference template)
        rHTMLWrt.nDfltTopMargin = 0;
        rHTMLWrt.nDfltBottomMargin = HTML_PARSPACE;
        if( USER_FMT & nPoolFmtId )
        {
            // user templates
            const OUString aNm(rFmt.GetName());

            if (aNm == "DD 1" || aNm == "DT 1")
                rHTMLWrt.nDfltBottomMargin = 0;
            else if (aNm == OOO_STRING_SVTOOLS_HTML_listing)
                rHTMLWrt.nDfltBottomMargin = 0;
            else if (aNm == OOO_STRING_SVTOOLS_HTML_preformtxt)
                rHTMLWrt.nDfltBottomMargin = 0;
            else if (aNm == OOO_STRING_SVTOOLS_HTML_xmp)
                rHTMLWrt.nDfltBottomMargin = 0;
        }
        else
        {
            // Pool templates
            switch( nPoolFmtId )
            {
            case RES_POOLCOLL_HEADLINE1:
            case RES_POOLCOLL_HEADLINE2:
            case RES_POOLCOLL_HEADLINE3:
            case RES_POOLCOLL_HEADLINE4:
            case RES_POOLCOLL_HEADLINE5:
            case RES_POOLCOLL_HEADLINE6:
                rHTMLWrt.nDfltTopMargin = HTML_HEADSPACE;
                break;
            case RES_POOLCOLL_SENDADRESS:
            case RES_POOLCOLL_HTML_DT:
            case RES_POOLCOLL_HTML_DD:
            case RES_POOLCOLL_HTML_PRE:
                rHTMLWrt.nDfltBottomMargin = 0;
                break;
            }
        }
    }

    // if nothing is to be exported ...
    if( !aItemSet.Count() )
        return rWrt;

    // There is no support for script dependent hyperlinks by now.
    bool bCheckForPseudo = false;
    if( bCharFmt &&
        (RES_POOLCHR_INET_NORMAL==nRefPoolId ||
         RES_POOLCHR_INET_VISIT==nRefPoolId) )
        bCheckForPseudo = true;

    // export now the Attributes (incl. Selektor)
    bool bHasScriptDependencies = false;
    if( OutCSS1Rule( rHTMLWrt, aSelector, aItemSet, CSS1_FMT_ISTAG != nDeep,
                      bCheckForPseudo ) )
    {
        if( bCharFmt )
            rHTMLWrt.aScriptTextStyles.insert( rFmt.GetName() );
        else
        {
            if( nPoolFmtId==RES_POOLCOLL_TEXT )
                rHTMLWrt.aScriptParaStyles.insert( pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false )->GetName() );
            rHTMLWrt.aScriptParaStyles.insert( rFmt.GetName() );
        }
        bHasScriptDependencies = true;
    }

    if( nPoolFmtId==RES_POOLCOLL_TEXT && !rHTMLWrt.bFirstCSS1Property )
        rHTMLWrt.bPoolCollTextModified = sal_True;

    // export Drop-Caps
    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET==aItemSet.GetItemState( RES_PARATR_DROP, false, &pItem ))
    {
        OUString sOut( aSelector );
        sOut += ":" + OStringToOUString( sCSS1_first_letter, RTL_TEXTENCODING_ASCII_US );
        const SwFmtDrop *pDrop = (const SwFmtDrop *)pItem;
        OutCSS1DropCapRule( rHTMLWrt, sOut, *pDrop, CSS1_FMT_ISTAG != nDeep, bHasScriptDependencies );
    }

    return rWrt;
}

static Writer& OutCSS1_SwPageDesc( Writer& rWrt, const SwPageDesc& rPageDesc,
                                   IDocumentStylePoolAccess/*SwDoc*/ *pDoc, SwDoc *pTemplate,
                                   sal_uInt16 nRefPoolId, bool bExtRef,
                                   bool bPseudo )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SwPageDesc* pRefPageDesc = 0;
    if( !bExtRef )
        pRefPageDesc = pDoc->GetPageDescFromPool( nRefPoolId, false );
    else if( pTemplate )
        pRefPageDesc = pTemplate->GetPageDescFromPool( nRefPoolId, false );

    OUString aSelector = "@" + OStringToOUString( sCSS1_page, RTL_TEXTENCODING_ASCII_US );

    if( bPseudo )
    {
        const sal_Char *pPseudo = 0;
        switch( rPageDesc.GetPoolFmtId() )
        {
        case RES_POOLPAGE_FIRST:    pPseudo = sCSS1_first;  break;
        case RES_POOLPAGE_LEFT:     pPseudo = sCSS1_left;   break;
        case RES_POOLPAGE_RIGHT:    pPseudo = sCSS1_right;  break;
        }
        if( pPseudo )
            aSelector += ":" + OStringToOUString( pPseudo, RTL_TEXTENCODING_ASCII_US );
    }

    SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_RULE_ON|CSS1_OUTMODE_TEMPLATE,
                         true, &aSelector );

    // Size: If the only difference is the Landscape-Flag,
    // only export Portrait oder Landscape. Otherwise export size.
    sal_Bool bRefLandscape = pRefPageDesc ? pRefPageDesc->GetLandscape() : sal_False;
    Size aRefSz;
    const Size& rSz = rPageDesc.GetMaster().GetFrmSize().GetSize();
    if( pRefPageDesc )
    {
        aRefSz = pRefPageDesc->GetMaster().GetFrmSize().GetSize();
        if( bRefLandscape != rPageDesc.GetLandscape() )
        {
            long nTmp = aRefSz.Height();
            aRefSz.Height() = aRefSz.Width();
            aRefSz.Width() = nTmp;
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
            rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_size,
                rPageDesc.GetLandscape() ? sCSS1_PV_landscape
                                         : sCSS1_PV_portrait );
        }
    }
    else
    {
        OStringBuffer sVal;
        AddUnitPropertyValue(sVal, rSz.Width(), rHTMLWrt.GetCSS1Unit());
        sVal.append(' ');
        AddUnitPropertyValue(sVal, rSz.Height(), rHTMLWrt.GetCSS1Unit());
        rHTMLWrt.OutCSS1_PropertyAscii(sCSS1_P_size, sVal.makeStringAndClear());
    }

    // Export the distance-Attributes as normally
    const SwFrmFmt &rMaster = rPageDesc.GetMaster();
    SfxItemSet aItemSet( *rMaster.GetAttrSet().GetPool(),
                         RES_LR_SPACE, RES_UL_SPACE );
    aItemSet.Set( rMaster.GetAttrSet(), true );

    if( pRefPageDesc )
    {
        SwHTMLWriter::SubtractItemSet( aItemSet,
                                       pRefPageDesc->GetMaster().GetAttrSet(),
                                       sal_True );
    }

    OutCSS1_SvxULSpace_SvxLRSpace( rWrt, aItemSet, sal_False );

    // If for a Pseudo-Selector no Property had been set, we still
    // have to export something, so that the corresponding template is
    // created on the next import.
    if( rHTMLWrt.bFirstCSS1Property && bPseudo )
    {
        rHTMLWrt.OutNewLine();
        OString sTmp(OUStringToOString(aSelector, rHTMLWrt.eDestEnc));
        rWrt.Strm().WriteCharPtr( sTmp.getStr() ).WriteCharPtr( " {" );
        rHTMLWrt.bFirstCSS1Property = sal_False;
    }

    if( !rHTMLWrt.bFirstCSS1Property )
        rWrt.Strm().WriteCharPtr( sCSS1_rule_end );

    return rWrt;
}

static Writer& OutCSS1_SwFtnInfo( Writer& rWrt, const SwEndNoteInfo& rInfo,
                                  SwDoc *pDoc, sal_uInt16 nNotes, bool bEndNote )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    OUString aSelector;

    if( nNotes > 0 )
    {
        aSelector = (OUString)OOO_STRING_SVTOOLS_HTML_anchor + "." +
                    OStringToOUString( bEndNote ? OOO_STRING_SVTOOLS_HTML_sdendnote_anc
                                       : OOO_STRING_SVTOOLS_HTML_sdfootnote_anc, RTL_TEXTENCODING_ASCII_US );
        SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                             true, &aSelector );
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_font_size,
                                        sHTML_FTN_fontheight );
        rHTMLWrt.Strm().WriteCharPtr( sCSS1_rule_end );
    }

    const SwCharFmt *pSymCharFmt = rInfo.GetCharFmt( *pDoc );
    if( pSymCharFmt )
    {
        const SfxItemSet& rFmtItemSet = pSymCharFmt->GetAttrSet();
        SfxItemSet aItemSet( *rFmtItemSet.GetPool(), rFmtItemSet.GetRanges() );
        aItemSet.Set( rFmtItemSet, true );

        // If there are footnotes or endnotes, then all Attributes have to be
        // exported, so that Netscape displays the document correctly.
        // Otherwise it is sufficient, to export the differences to the
        // footnote and endnote template.
        if( nNotes == 0 && rHTMLWrt.pTemplate )
        {
            SwFmt *pRefFmt = rHTMLWrt.pTemplate->GetCharFmtFromPool(
                        static_cast< sal_uInt16 >(bEndNote ? RES_POOLCHR_ENDNOTE : RES_POOLCHR_FOOTNOTE) );
            if( pRefFmt )
                SwHTMLWriter::SubtractItemSet( aItemSet, pRefFmt->GetAttrSet(),
                                               sal_True );
        }
        if( aItemSet.Count() )
        {
            aSelector = (OUString)OOO_STRING_SVTOOLS_HTML_anchor + "." +
                        OStringToOUString( bEndNote ? OOO_STRING_SVTOOLS_HTML_sdendnote_sym
                                            : OOO_STRING_SVTOOLS_HTML_sdfootnote_sym, RTL_TEXTENCODING_ASCII_US );
            if( OutCSS1Rule( rHTMLWrt, aSelector, aItemSet, sal_True, false ))
                rHTMLWrt.aScriptTextStyles.insert( pSymCharFmt->GetName() );
        }
    }

    return rWrt;
}

Writer& OutCSS1_BodyTagStyleOpt( Writer& rWrt, const SfxItemSet& rItemSet )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_STYLE_OPT_ON |
                                   CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_BODY );

    // Only export the attributes of the page template.
    // The attributes of the default paragraph template were
    // considered already when exporting the paragraph template.

    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, false,
                                               &pItem ) )
    {
        OutCSS1_SvxBrush( rWrt, *pItem, CSS1_BACKGROUND_PAGE );
    }

    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BOX, false,
                                               &pItem ))
    {
        OutCSS1_SvxBox( rWrt, *pItem );
    }

    if( !rHTMLWrt.bFirstCSS1Property )
    {
        // if a Property was exported as part of a Style-Option,
        // the Option still needs to be finished
        rWrt.Strm().WriteChar( '\"' );
    }

    return rWrt;
}

Writer& OutCSS1_ParaTagStyleOpt( Writer& rWrt, const SfxItemSet& rItemSet )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    SwCSS1OutMode aMode( rHTMLWrt, rHTMLWrt.nCSS1Script|CSS1_OUTMODE_STYLE_OPT |
                                   CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_PARA );
    rHTMLWrt.OutCSS1_SfxItemSet( rItemSet, sal_False );

    return rWrt;
}

Writer& OutCSS1_HintSpanTag( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_SPAN_TAG |
                                   CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_HINT );

    Out( aCSS1AttrFnTab, rHt, rWrt );

    if( !rHTMLWrt.bFirstCSS1Property  && rHTMLWrt.bTagOn )
        rWrt.Strm().WriteCharPtr( sCSS1_span_tag_end );

    return rWrt;
}

Writer& OutCSS1_HintStyleOpt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_STYLE_OPT_ON |
                                   CSS1_OUTMODE_ENCODE|
                                   CSS1_OUTMODE_HINT );

    Out( aCSS1AttrFnTab, rHt, rWrt );

    if( !rHTMLWrt.bFirstCSS1Property )
        rWrt.Strm().WriteChar( '\"' );

    return rWrt;
}

// Wrapper for Table background
Writer& OutCSS1_TableBGStyleOpt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_STYLE_OPT_ON |
                                   CSS1_OUTMODE_ENCODE|
                                   CSS1_OUTMODE_TABLEBOX );
    OutCSS1_SvxBrush( rWrt, rHt, CSS1_BACKGROUND_TABLE );

    if( !rHTMLWrt.bFirstCSS1Property )
        rWrt.Strm().WriteChar( '\"' );

    return rWrt;
}

Writer& OutCSS1_NumBulListStyleOpt( Writer& rWrt, const SwNumRule& rNumRule,
                                    sal_uInt8 nLevel )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_STYLE_OPT |
                                   CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_PARA );

    const SwNumFmt& rNumFmt = rNumRule.Get( nLevel );

    long nLSpace = rNumFmt.GetAbsLSpace();
    long nFirstLineOffset = rNumFmt.GetFirstLineOffset();
    long nDfltFirstLineOffset = HTML_NUMBUL_INDENT;
    if( nLevel > 0 )
    {
        const SwNumFmt& rPrevNumFmt = rNumRule.Get( nLevel-1 );
        nLSpace -= rPrevNumFmt.GetAbsLSpace();
        nDfltFirstLineOffset = rPrevNumFmt.GetFirstLineOffset();
    }

    if( rHTMLWrt.IsHTMLMode(HTMLMODE_LSPACE_IN_NUMBUL) &&
        nLSpace != HTML_NUMBUL_MARGINLEFT )
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_margin_left, nLSpace );

    if( rHTMLWrt.IsHTMLMode(HTMLMODE_FRSTLINE_IN_NUMBUL) &&
        nFirstLineOffset != nDfltFirstLineOffset )
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_text_indent, nFirstLineOffset );

    if( !rHTMLWrt.bFirstCSS1Property )
        rWrt.Strm().WriteChar( '\"' );

    return rWrt;
}

void SwHTMLWriter::OutCSS1_FrmFmtOptions( const SwFrmFmt& rFrmFmt,
                                          sal_uInt32 nFrmOpts,
                                          const SdrObject *pSdrObj,
                                          const SfxItemSet *pItemSet )
{
    SwCSS1OutMode aMode( *this, CSS1_OUTMODE_STYLE_OPT_ON |
                                CSS1_OUTMODE_ENCODE|
                                CSS1_OUTMODE_FRAME );

    const SwFmtHoriOrient& rHoriOri = rFrmFmt.GetHoriOrient();
    SvxLRSpaceItem aLRItem( rFrmFmt.GetLRSpace() );
    SvxULSpaceItem aULItem( rFrmFmt.GetULSpace() );
    if( nFrmOpts & HTML_FRMOPT_S_ALIGN )
    {
        const SwFmtAnchor& rAnchor = rFrmFmt.GetAnchor();
        switch( rAnchor.GetAnchorId() )
        {
        case FLY_AT_PARA:
        case FLY_AT_CHAR:
            if( text::RelOrientation::FRAME == rHoriOri.GetRelationOrient() ||
                text::RelOrientation::PRINT_AREA == rHoriOri.GetRelationOrient() )
            {
                if( !(nFrmOpts & HTML_FRMOPT_ALIGN) )
                {
                    // float
                    const sal_Char *pStr = text::HoriOrientation::RIGHT==rHoriOri.GetHoriOrient()
                            ? sCSS1_PV_right
                            : sCSS1_PV_left;
                    OutCSS1_PropertyAscii( sCSS1_P_float, pStr );
                }
                break;
            }

        case FLY_AT_PAGE:
        case FLY_AT_FLY:
            {
                // position
                OutCSS1_PropertyAscii( sCSS1_P_position, sCSS1_PV_absolute );

                // For top/left we need to subtract the distance to the frame
                // from the position, as in CSS1 it is added to the position.
                // This works also for automatically aligned frames, even that
                // in this case Writer also adds the distance; because in this
                // case the Orient-Attribute contains the correct position.

                // top
                long nXPos=0, nYPos=0;
                bool bOutXPos = false, bOutYPos = false;
                if( RES_DRAWFRMFMT == rFrmFmt.Which() )
                {
                    OSL_ENSURE( pSdrObj, "Do not pass a SdrObject. Inefficient" );
                    if( !pSdrObj )
                        pSdrObj = rFrmFmt.FindSdrObject();
                    OSL_ENSURE( pSdrObj, "Where is the SdrObject" );
                    if( pSdrObj )
                    {
                        Point aPos( pSdrObj->GetRelativePos() );
                        nXPos = aPos.A();
                        nYPos = aPos.B();
                    }
                    bOutXPos = bOutYPos = true;
                }
                else
                {
                    bOutXPos = text::RelOrientation::CHAR != rHoriOri.GetRelationOrient();
                    nXPos = text::HoriOrientation::NONE == rHoriOri.GetHoriOrient()
                                ? rHoriOri.GetPos() : 0;

                    const SwFmtVertOrient& rVertOri = rFrmFmt.GetVertOrient();
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
                            aULItem.SetUpper( (sal_uInt16)(aULItem.GetUpper() + nYPos) );
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
                        nXPos -= aLRItem.GetLeft();
                        if( nXPos < 0 )
                        {
                            aLRItem.SetLeft( (sal_uInt16)(aLRItem.GetLeft() + nXPos) );
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
    if( nFrmOpts & HTML_FRMOPT_S_SIZE )
    {
        if( RES_DRAWFRMFMT == rFrmFmt.Which() )
        {
            OSL_ENSURE( pSdrObj, "Do not pass a SdrObject. Inefficient" );
            if( !pSdrObj )
                pSdrObj = rFrmFmt.FindSdrObject();
            OSL_ENSURE( pSdrObj, "Where is the SdrObject" );
            if( pSdrObj )
            {
                Size aTwipSz( pSdrObj->GetLogicRect().GetSize() );
                if( nFrmOpts & HTML_FRMOPT_S_WIDTH )
                {
                    if( nFrmOpts & HTML_FRMOPT_S_PIXSIZE )
                        OutCSS1_PixelProperty( sCSS1_P_width, aTwipSz.Width(),
                                               sal_False );
                    else
                        OutCSS1_UnitProperty( sCSS1_P_width, aTwipSz.Width() );
                }
                if( nFrmOpts & HTML_FRMOPT_S_HEIGHT )
                {
                    if( nFrmOpts & HTML_FRMOPT_S_PIXSIZE )
                        OutCSS1_PixelProperty( sCSS1_P_height, aTwipSz.Height(),
                                               sal_True );
                    else
                        OutCSS1_UnitProperty( sCSS1_P_height, aTwipSz.Height() );
                }
            }
        }
        else
        {
            OSL_ENSURE( HTML_FRMOPT_ABSSIZE & nFrmOpts,
                    "Export absolute size" );
            OSL_ENSURE( HTML_FRMOPT_ANYSIZE & nFrmOpts,
                    "Export every size" );
            sal_uInt16 nMode = 0;
            if( nFrmOpts & HTML_FRMOPT_S_WIDTH )
                nMode |= CSS1_FRMSIZE_WIDTH;
            if( nFrmOpts & HTML_FRMOPT_S_HEIGHT )
                nMode |= (CSS1_FRMSIZE_MINHEIGHT|CSS1_FRMSIZE_FIXHEIGHT);
            if( nFrmOpts & HTML_FRMOPT_S_PIXSIZE )
                nMode |= CSS1_FRMSIZE_PIXEL;

            OutCSS1_SwFmtFrmSize( *this, rFrmFmt.GetFrmSize(), nMode );
        }
    }

    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();
    // margin-*
    if( (nFrmOpts & HTML_FRMOPT_S_SPACE) &&
        IsHTMLMode( HTMLMODE_FLY_MARGINS) )
    {
        const SvxLRSpaceItem *pLRItem = 0;
        const SvxULSpaceItem *pULItem = 0;
        if( SFX_ITEM_SET == rItemSet.GetItemState( RES_LR_SPACE, true ) )
            pLRItem = &aLRItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( RES_UL_SPACE, true ) )
            pULItem = &aULItem;
        if( pLRItem || pULItem )
            OutCSS1_SvxULSpace_SvxLRSpace( *this, pULItem, pLRItem );
    }

    // border
    if( nFrmOpts & HTML_FRMOPT_S_BORDER )
    {
        const SfxPoolItem* pItem;
        if( nFrmOpts & HTML_FRMOPT_S_NOBORDER )
            OutCSS1_SvxBox( *this, rFrmFmt.GetBox() );
        else if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BOX, true, &pItem ) )
            OutCSS1_SvxBox( *this, *pItem );
    }

    // background (if, then the color must be set also)
    if( nFrmOpts & HTML_FRMOPT_S_BACKGROUND )
        OutCSS1_FrmFmtBackground( rFrmFmt );

    if( pItemSet )
        OutCSS1_SfxItemSet( *pItemSet, sal_False );

    if( !bFirstCSS1Property )
        Strm().WriteChar( '\"' );
}

void SwHTMLWriter::OutCSS1_TableFrmFmtOptions( const SwFrmFmt& rFrmFmt )
{
    SwCSS1OutMode aMode( *this, CSS1_OUTMODE_STYLE_OPT_ON |
                                CSS1_OUTMODE_ENCODE|
                                CSS1_OUTMODE_TABLE );

    const SfxPoolItem *pItem;
    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();
    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BACKGROUND, false, &pItem ) )
        OutCSS1_SvxBrush( *this, *pItem, CSS1_BACKGROUND_TABLE );

    if( IsHTMLMode( HTMLMODE_PRINT_EXT ) )
        OutCSS1_SvxFmtBreak_SwFmtPDesc_SvxFmtKeep( *this, rItemSet, sal_False );

    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_LAYOUT_SPLIT, false, &pItem ) )
        OutCSS1_SwFmtLayoutSplit( *this, *pItem );

    if( !bFirstCSS1Property )
        Strm().WriteChar( '\"' );
}

void SwHTMLWriter::OutCSS1_TableCellBorderHack(SwFrmFmt const& rFrmFmt)
{
    SwCSS1OutMode const aMode( *this,
        CSS1_OUTMODE_STYLE_OPT_ON|CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_TABLEBOX );
    OutCSS1_SvxBox(*this, rFrmFmt.GetBox());
    if (!bFirstCSS1Property)
    {
        this->Strm().WriteChar( cCSS1_style_opt_end );
    }
}

void SwHTMLWriter::OutCSS1_SectionFmtOptions( const SwFrmFmt& rFrmFmt, const SwFmtCol *pCol )
{
    SwCSS1OutMode aMode( *this, CSS1_OUTMODE_STYLE_OPT_ON |
                                CSS1_OUTMODE_ENCODE|
                                CSS1_OUTMODE_SECTION );

    const SfxPoolItem *pItem;
    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();
    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BACKGROUND, false, &pItem ) )
        OutCSS1_SvxBrush( *this, *pItem, CSS1_BACKGROUND_SECTION );

    if (pCol)
    {
        OString sColumnCount(OString::number(static_cast<sal_Int32>(pCol->GetNumCols())));
        OutCSS1_PropertyAscii(sCSS1_P_column_count, sColumnCount);
    }

    if( !bFirstCSS1Property )
        Strm().WriteChar( '\"' );
}

static bool OutCSS1_FrmFmtBrush( SwHTMLWriter& rWrt,
                                 const SvxBrushItem& rBrushItem )
{
    bool bWritten = false;
    /// output brush of frame format, if its background color is not "no fill"/"auto fill"
    /// or it has a background graphic.
    if( rBrushItem.GetColor() != COL_TRANSPARENT ||
        0 != rBrushItem.GetGraphicLink() ||
        0 != rBrushItem.GetGraphicPos() )
    {
        OutCSS1_SvxBrush( rWrt, rBrushItem, CSS1_BACKGROUND_FLY );
        bWritten = true;
    }
    return bWritten;
}

void SwHTMLWriter::OutCSS1_FrmFmtBackground( const SwFrmFmt& rFrmFmt )
{
    // If the frame itself has a background, then export.
    if( OutCSS1_FrmFmtBrush( *this, rFrmFmt.GetBackground() ) )
        return;

    // If the frame is not linked to a page, we use the background of the anchor.
    const SwFmtAnchor& rAnchor = rFrmFmt.GetAnchor();
    RndStdIds eAnchorId = rAnchor.GetAnchorId();
    const SwPosition *pAnchorPos = rAnchor.GetCntntAnchor();
    if (FLY_AT_PAGE != eAnchorId && pAnchorPos)
    {
        const SwNode& rNode = pAnchorPos->nNode.GetNode();
        if( rNode.IsCntntNode() )
        {
            // If the frame is linked to a content-node,
            // we take the background of the content-node, if it has one.
            if( OutCSS1_FrmFmtBrush( *this,
                    rNode.GetCntntNode()->GetSwAttrSet().GetBackground()) )
                return;

            // Otherwise we also could be in a table
            const SwTableNode *pTableNd = rNode.FindTableNode();
            if( pTableNd )
            {
                const SwStartNode *pBoxSttNd = rNode.FindTableBoxStartNode();
                const SwTableBox *pBox =
                    pTableNd->GetTable().GetTblBox( pBoxSttNd->GetIndex() );

                // If the box has a background, we take it.
                if( OutCSS1_FrmFmtBrush( *this,
                        pBox->GetFrmFmt()->GetBackground() ) )
                    return;

                // Otherwise we use that of the lines
                const SwTableLine *pLine = pBox->GetUpper();
                while( pLine )
                {
                    if( OutCSS1_FrmFmtBrush( *this,
                            pLine->GetFrmFmt()->GetBackground() ) )
                        return;
                    pBox = pLine->GetUpper();
                    pLine = pBox ? pBox->GetUpper() : 0;
                }

                // If there was none either, we use the background of the table.
                if( OutCSS1_FrmFmtBrush( *this,
                        pTableNd->GetTable().GetFrmFmt()->GetBackground() ) )
                    return;
            }

        }

        // If the anchor is again in a Fly-Frame, use the background of the Fly-Frame.
        const SwFrmFmt *pFrmFmt = rNode.GetFlyFmt();
        if( pFrmFmt )
        {
            OutCSS1_FrmFmtBackground( *pFrmFmt );
            return;
        }
    }

    // At last there is the background of the page, and as the final rescue
    // the value of the Config.
    OSL_ENSURE( pCurrPageDesc, "no page template found" );
    if( !OutCSS1_FrmFmtBrush( *this,
                              pCurrPageDesc->GetMaster().GetBackground() ) )
    {
        Color aColor( COL_WHITE );

        // The background color is normally only used in Browse-Mode.
        // We always use it for a HTML document, but for a text document
        // only if viewed in Browse-Mode.
        if( pDoc->get(IDocumentSettingAccess::HTML_MODE) ||
            pDoc->get(IDocumentSettingAccess::BROWSE_MODE))
        {
            SwViewShell *pVSh = 0;
            pDoc->GetEditShell( &pVSh );
            if ( pVSh &&
                 COL_TRANSPARENT != pVSh->GetViewOptions()->GetRetoucheColor().GetColor())
                aColor = pVSh->GetViewOptions()->GetRetoucheColor().GetColor();
        }

        OutCSS1_PropertyAscii(sCSS1_P_background, GetCSS1Color(aColor));
    }
}

static Writer& OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink( Writer& rWrt,
                    const SvxUnderlineItem *pUItem,
                    const SvxOverlineItem *pOItem,
                    const SvxCrossedOutItem *pCOItem,
                    const SvxBlinkItem *pBItem )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    bool bNone = false;

    const sal_Char *pUStr = 0;
    if( pUItem )
    {
        switch( pUItem->GetLineStyle() )
        {
        case UNDERLINE_NONE:
            bNone = true;
            break;
        case UNDERLINE_DONTKNOW:
            break;
        default:
            if( !rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
            {
                // this also works in HTML does not need to be written as
                // a STYLE-Options, and must not be written as Hint
                OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                        "write underline as Hint?" );
                pUStr = sCSS1_PV_underline;
            }
            break;
        }
    }

    const sal_Char *pOStr = 0;
    if( pOItem )
    {
        switch( pOItem->GetLineStyle() )
        {
        case UNDERLINE_NONE:
            bNone = true;
            break;
        case UNDERLINE_DONTKNOW:
            break;
        default:
            if( !rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
            {
                // this also works in HTML does not need to be written as
                // a STYLE-Options, and must not be written as Hint
                OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                        "write overline as Hint?" );
                pOStr = sCSS1_PV_overline;
            }
            break;
        }
    }

    const sal_Char *pCOStr = 0;
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
            if( !rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
            {
                // this also works in HTML does not need to be written as
                // a STYLE-Options, and must not be written as Hint
                OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                        "write crossedOut as Hint?" );
                pCOStr = sCSS1_PV_line_through;
            }
            break;
        }
    }

    const sal_Char *pBStr = 0;
    if( pBItem )
    {
        if( !pBItem->GetValue() )
        {
            bNone = true;
        }
        else if( !rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        {
            // this also works in HTML does not need to be written as
            // a STYLE-Options, and must not be written as Hint
            OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                    "write blink as Hint?" );
            pBStr = sCSS1_PV_blink;
        }
    }

    OStringBuffer sOut;
    if( pUStr )
        sOut.append(pUStr);

    if( pOStr )
    {
        if (!sOut.isEmpty())
            sOut.append(' ');
        sOut.append(pOStr);
    }

    if( pCOStr )
    {
        if (!sOut.isEmpty())
            sOut.append(' ');
        sOut.append(pCOStr);
    }

    if( pBStr )
    {
        if (!sOut.isEmpty())
            sOut.append(' ');
        sOut.append(pBStr);
    }

    if (!sOut.isEmpty())
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_text_decoration, sOut.makeStringAndClear() );
    else if( bNone )
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_text_decoration, sCSS1_PV_none );

    return rWrt;
}

static Writer& OutCSS1_SvxCaseMap( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    switch( ((const SvxCaseMapItem&)rHt).GetCaseMap() )
    {
    case SVX_CASEMAP_NOT_MAPPED:
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_font_variant, sCSS1_PV_normal );
        break;
    case SVX_CASEMAP_KAPITAELCHEN:
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_font_variant, sCSS1_PV_small_caps );
        break;
    case SVX_CASEMAP_VERSALIEN:
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_text_transform, sCSS1_PV_uppercase );
        break;
    case SVX_CASEMAP_GEMEINE:
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_text_transform, sCSS1_PV_lowercase );
        break;
    case SVX_CASEMAP_TITEL:
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_text_transform, sCSS1_PV_capitalize );
        break;
    default:
        ;
    }

    return rWrt;
}

static Writer& OutCSS1_SvxColor( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Colors do not need to be exported for Style-Option.
    if( rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) &&
        !rHTMLWrt.bCfgPreferStyles )
        return rWrt;
    OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
            "write color as Hint?" );

    Color aColor( ((const SvxColorItem&)rHt).GetValue() );
    if( COL_AUTO == aColor.GetColor() )
        aColor.SetColor( COL_BLACK );

    rHTMLWrt.OutCSS1_PropertyAscii(sCSS1_P_color, GetCSS1Color(aColor));

    return rWrt;
}

static Writer& OutCSS1_SvxCrossedOut( Writer& rWrt, const SfxPoolItem& rHt )
{
    // This function only exports Hints!
    // Otherwise OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink() is called directly.

    if( ((SwHTMLWriter&)rWrt).IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink( rWrt,
                0, 0, (const SvxCrossedOutItem *)&rHt, 0 );

    return rWrt;
}

static Writer& OutCSS1_SvxFont( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // No need to export Fonts for the Style-Option.
    if( rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        return rWrt;

    sal_uInt16 nScript = CSS1_OUTMODE_WESTERN;
    switch( rHt.Which() )
    {
    case RES_CHRATR_CJK_FONT:   nScript = CSS1_OUTMODE_CJK; break;
    case RES_CHRATR_CTL_FONT:   nScript = CSS1_OUTMODE_CTL; break;
    }
    if( !rHTMLWrt.IsCSS1Script( nScript ) )
        return rWrt;

    OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
            "write Font as Hint?" );

    OUString sOut;
    // MS IE3b1 has problems with single quotes
    sal_uInt16 nMode = rHTMLWrt.nCSS1OutMode & CSS1_OUTMODE_ANY_ON;
    sal_Unicode cQuote = nMode == CSS1_OUTMODE_RULE_ON ? '\"' : '\'';
    SwHTMLWriter::PrepareFontList( ((const SvxFontItem&)rHt), sOut, cQuote,
                                   sal_True );

    rHTMLWrt.OutCSS1_Property( sCSS1_P_font_family, sOut );

    return rWrt;
}

static Writer& OutCSS1_SvxFontHeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Font-Height need not be exported in the Style-Option.
    // For Drop-Caps another Font-Size is exported.
    if( rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) ||
        rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_DROPCAP ) )
        return rWrt;

    sal_uInt16 nScript = CSS1_OUTMODE_WESTERN;
    switch( rHt.Which() )
    {
    case RES_CHRATR_CJK_FONTSIZE:   nScript = CSS1_OUTMODE_CJK; break;
    case RES_CHRATR_CTL_FONTSIZE:   nScript = CSS1_OUTMODE_CTL; break;
    }
    if( !rHTMLWrt.IsCSS1Script( nScript ) )
        return rWrt;

    sal_uInt32 nHeight = ((const SvxFontHeightItem&)rHt).GetHeight();
    OString sHeight(OString::number(nHeight/20) + OString(sCSS1_UNIT_pt));
    rHTMLWrt.OutCSS1_PropertyAscii(sCSS1_P_font_size, sHeight);

    return rWrt;
}

static Writer& OutCSS1_SvxPosture( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    sal_uInt16 nScript = CSS1_OUTMODE_WESTERN;
    switch( rHt.Which() )
    {
    case RES_CHRATR_CJK_POSTURE:    nScript = CSS1_OUTMODE_CJK; break;
    case RES_CHRATR_CTL_POSTURE:    nScript = CSS1_OUTMODE_CTL; break;
    }
    if( !rHTMLWrt.IsCSS1Script( nScript ) )
        return rWrt;

    const sal_Char *pStr = 0;
    switch( ((const SvxPostureItem&)rHt).GetPosture() )
    {
    case ITALIC_NONE:       pStr = sCSS1_PV_normal;     break;
    case ITALIC_OBLIQUE:    pStr = sCSS1_PV_oblique;    break;
    case ITALIC_NORMAL:
        if( !rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        {
            // this also works in HTML does not need to be written as
            // a STYLE-Options, and must not be written as Hint
            OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                    "write italic as Hint?" );
            pStr = sCSS1_PV_italic;
        }
        break;
    default:
        ;
    }

    if( pStr )
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_font_style, pStr );

    return rWrt;
}

static Writer& OutCSS1_SvxKerning( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Only export Kerning-Item, if the Style supports it fully
    if( !rHTMLWrt.IsHTMLMode(HTMLMODE_FULL_STYLES) )
        return rWrt;

    sal_Int16 nValue = ((const SvxKerningItem&)rHt).GetValue();
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
                    OString(sCSS1_UNIT_pt));

        rHTMLWrt.OutCSS1_PropertyAscii(sCSS1_P_letter_spacing,
            sOut.makeStringAndClear());
    }
    else
    {
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_letter_spacing,
                                        sCSS1_PV_normal );
    }

    return rWrt;
}

static Writer& OutCSS1_SvxLanguage( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Only export Language rules
    if( rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        return rWrt;

    sal_uInt16 nScript = CSS1_OUTMODE_WESTERN;
    switch( rHt.Which() )
    {
    case RES_CHRATR_CJK_LANGUAGE:   nScript = CSS1_OUTMODE_CJK; break;
    case RES_CHRATR_CTL_LANGUAGE:   nScript = CSS1_OUTMODE_CTL; break;
    }
    if( !rHTMLWrt.IsCSS1Script( nScript ) )
        return rWrt;

    OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
            "write Language as Hint?" );

    LanguageType eLang = ((const SvxLanguageItem &)rHt).GetLanguage();
    if( LANGUAGE_DONTKNOW == eLang )
        return rWrt;

    OUString sOut = LanguageTag::convertToBcp47( eLang );

    rHTMLWrt.OutCSS1_Property( sCSS1_P_so_language, sOut );

    return rWrt;
}

static Writer& OutCSS1_SvxUnderline( Writer& rWrt, const SfxPoolItem& rHt )
{
    // This function only exports Hints!
    // Otherwise OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink() is called directly.

    if( ((SwHTMLWriter&)rWrt).IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink( rWrt,
                (const SvxUnderlineItem *)&rHt, 0, 0, 0 );

    return rWrt;
}

static Writer& OutCSS1_SvxOverline( Writer& rWrt, const SfxPoolItem& rHt )
{
    // This function only exports Hints!
    // Otherwise OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink() is called directly.

    if( ((SwHTMLWriter&)rWrt).IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink( rWrt,
                0, (const SvxOverlineItem *)&rHt, 0, 0 );

    return rWrt;
}

static Writer& OutCSS1_SvxHidden( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    if ( ((const SvxCharHiddenItem&)rHt).GetValue() )
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_display, sCSS1_PV_none );

    return rWrt;
}

static Writer& OutCSS1_SvxFontWeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    sal_uInt16 nScript = CSS1_OUTMODE_WESTERN;
    switch( rHt.Which() )
    {
    case RES_CHRATR_CJK_WEIGHT: nScript = CSS1_OUTMODE_CJK; break;
    case RES_CHRATR_CTL_WEIGHT: nScript = CSS1_OUTMODE_CTL; break;
    }
    if( !rHTMLWrt.IsCSS1Script( nScript ) )
        return rWrt;

    const sal_Char *pStr = 0;
    switch( ((const SvxWeightItem&)rHt).GetWeight() )
    {
    case WEIGHT_ULTRALIGHT: pStr = sCSS1_PV_extra_light;    break;
    case WEIGHT_LIGHT:      pStr = sCSS1_PV_light;          break;
    case WEIGHT_SEMILIGHT:  pStr = sCSS1_PV_demi_light;     break;
    case WEIGHT_NORMAL:     pStr = sCSS1_PV_normal;         break;
    case WEIGHT_SEMIBOLD:   pStr = sCSS1_PV_demi_bold;      break;
    case WEIGHT_BOLD:
        if( !rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        {
            // this also works in HTML does not need to be written as
            // a STYLE-Options, and must not be written as Hint
            OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                    "write bold as Hint?" );
            pStr = sCSS1_PV_bold;
        }
        break;
    case WEIGHT_ULTRABOLD:  pStr = sCSS1_PV_extra_bold;     break;
    default:
        pStr = sCSS1_PV_normal;;
    }

    if( pStr )
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_font_weight, pStr );

    return rWrt;
}

static Writer& OutCSS1_SvxBlink( Writer& rWrt, const SfxPoolItem& rHt )
{
    // This function only exports Hints!
    // Otherwise OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink() is called directly.

    if( ((SwHTMLWriter&)rWrt).IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink( rWrt,
                0, 0, 0, (const SvxBlinkItem *)&rHt );

    return rWrt;
}

static Writer& OutCSS1_SvxLineSpacing( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Netscape4 has big problems with cell heights if the line spacing is
    // changed within a table and the width of the table is not calculated
    // automatically (== if there is a WIDTH-Option)
    if( rHTMLWrt.bOutTable && rHTMLWrt.bCfgNetscape4 )
        return rWrt;

    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)rHt;

    sal_uInt16 nHeight = 0;
    sal_uInt16 nPrcHeight = 0;
    SvxLineSpace eLineSpace = rLSItem.GetLineSpaceRule();
    switch( rLSItem.GetInterLineSpaceRule() )
    {
    case SVX_INTER_LINE_SPACE_OFF:
    case SVX_INTER_LINE_SPACE_FIX:
        {
            switch( eLineSpace )
            {
            case SVX_LINE_SPACE_MIN:
            case SVX_LINE_SPACE_FIX:
                nHeight = rLSItem.GetLineHeight();
                break;
            case SVX_LINE_SPACE_AUTO:
                nPrcHeight = 100;
                break;
            default:
                ;
            }
        }
        break;
    case SVX_INTER_LINE_SPACE_PROP:
        nPrcHeight = rLSItem.GetPropLineSpace();
        break;

    default:
        ;
    }

    if( nHeight )
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_line_height, (long)nHeight );
    else if( nPrcHeight )
    {
        OString sHeight(OString::number(nPrcHeight) + "%");
        rHTMLWrt.OutCSS1_PropertyAscii(sCSS1_P_line_height, sHeight);
    }

    return rWrt;
}

static Writer& OutCSS1_SvxAdjust( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Export Alignment in Style-Option only if the Tag does not allow ALIGN=xxx
    if( rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) &&
        !rHTMLWrt.bNoAlign)
        return rWrt;

    const sal_Char* pStr = 0;
    switch( ((const SvxAdjustItem&)rHt).GetAdjust() )
    {
    case SVX_ADJUST_LEFT:   pStr = sCSS1_PV_left;       break;
    case SVX_ADJUST_RIGHT:  pStr = sCSS1_PV_right;      break;
    case SVX_ADJUST_BLOCK:  pStr = sCSS1_PV_justify;    break;
    case SVX_ADJUST_CENTER: pStr = sCSS1_PV_center;     break;
    default:
        ;
    }

    if( pStr )
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_text_align, pStr );

    return rWrt;
}

static Writer& OutCSS1_SvxFmtSplit( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const sal_Char *pStr = ((const SvxFmtSplitItem&)rHt).GetValue()
                            ? sCSS1_PV_auto
                            : sCSS1_PV_avoid;
    rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_page_break_inside, pStr );

    return rWrt;
}

static Writer& OutCSS1_SwFmtLayoutSplit( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const sal_Char *pStr = ((const SwFmtLayoutSplit&)rHt).GetValue()
                            ? sCSS1_PV_auto
                            : sCSS1_PV_avoid;
    rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_page_break_inside, pStr );

    return rWrt;
}

static Writer& OutCSS1_SvxWidows( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    OString aStr(OString::number(((const SvxWidowsItem&)rHt).GetValue()));
    rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_widows, aStr );

    return rWrt;
}

static Writer& OutCSS1_SvxOrphans( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    OString aStr(OString::number(((const SvxOrphansItem&)rHt).GetValue()));
    rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_orphans, aStr );

    return rWrt;
}

static void OutCSS1_SwFmtDropAttrs( SwHTMLWriter& rHWrt,
                                    const SwFmtDrop& rDrop,
                                     const SfxItemSet *pCharFmtItemSet )
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

    const SwCharFmt *pDCCharFmt = rDrop.GetCharFmt();
    if( pCharFmtItemSet )
        rHWrt.OutCSS1_SfxItemSet( *pCharFmtItemSet );
    else if( pDCCharFmt )
        rHWrt.OutCSS1_SfxItemSet( pDCCharFmt->GetAttrSet() );
    else if( (rHWrt.nCSS1OutMode & CSS1_OUTMODE_ANY_OFF) == CSS1_OUTMODE_RULE_OFF )
        rHWrt.Strm().WriteCharPtr( sCSS1_rule_end );

}

static Writer& OutCSS1_SwFmtDrop( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    // never export as an Option of a paragraph, but only as Hints
    if( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT) )
        return rWrt;

    if( rHTMLWrt.bTagOn )
    {
        SwCSS1OutMode aMode( rHTMLWrt,
                             rHTMLWrt.nCSS1Script|CSS1_OUTMODE_SPAN_TAG1_ON|CSS1_OUTMODE_ENCODE|
                             CSS1_OUTMODE_DROPCAP );

        OutCSS1_SwFmtDropAttrs( rHTMLWrt, (const SwFmtDrop&)rHt );
        // A "> is already printed by the calling OutCSS1_HintAsSpanTag.
    }
    else
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_span, false );
    }

    return rWrt;
}

static Writer& OutCSS1_SwFmtFrmSize( Writer& rWrt, const SfxPoolItem& rHt,
                                     sal_uInt16 nMode )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SwFmtFrmSize& rFSItem = (const SwFmtFrmSize&)rHt;

    if( nMode & CSS1_FRMSIZE_WIDTH )
    {
        sal_uInt8 nPrcWidth = rFSItem.GetWidthPercent();
        if( nPrcWidth )
        {
            OString sOut(OString::number(nPrcWidth) + "%");
            rHTMLWrt.OutCSS1_PropertyAscii(sCSS1_P_width, sOut);
        }
        else if( nMode & CSS1_FRMSIZE_PIXEL )
        {
            rHTMLWrt.OutCSS1_PixelProperty( sCSS1_P_width,
                                            rFSItem.GetSize().Width(), sal_False );
        }
        else
        {
            rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_width,
                                           rFSItem.GetSize().Width() );
        }
    }

    if( nMode & CSS1_FRMSIZE_ANYHEIGHT )
    {
        bool bOutHeight = false;
        switch( rFSItem.GetHeightSizeType() )
        {
        case ATT_FIX_SIZE:
            bOutHeight = (nMode & CSS1_FRMSIZE_FIXHEIGHT) != 0;
            break;
        case ATT_MIN_SIZE:
            bOutHeight = (nMode & CSS1_FRMSIZE_MINHEIGHT) != 0;
            break;
        case ATT_VAR_SIZE:
            bOutHeight = (nMode & CSS1_FRMSIZE_VARHEIGHT) != 0;
            break;
        default:
            OSL_ENSURE( bOutHeight, "Hoehe wird nicht exportiert" );
            break;
        }

        if( bOutHeight )
        {
            sal_uInt8 nPrcHeight = rFSItem.GetHeightPercent();
            if( nPrcHeight )
            {
                OString sOut(OString::number(nPrcHeight) + "%");
                rHTMLWrt.OutCSS1_PropertyAscii(sCSS1_P_height, sOut);
            }
            else if( nMode & CSS1_FRMSIZE_PIXEL )
            {
                rHTMLWrt.OutCSS1_PixelProperty( sCSS1_P_height,
                                                rFSItem.GetSize().Height(),
                                                sal_True );
            }
            else
            {
                rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_height,
                                               rFSItem.GetSize().Height() );
            }
        }
    }

    return rWrt;
}

static Writer& OutCSS1_SvxLRSpace( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SvxLRSpaceItem& rLRItem = (const SvxLRSpaceItem&)rHt;

    // No Export of a firm attribute is needed if the new values
    // match that of the current template

    // A left margin can exist because of a list nearby
    long nLeftMargin = (long)rLRItem.GetTxtLeft() - rHTMLWrt.nLeftMargin;
    if( rHTMLWrt.nDfltLeftMargin != nLeftMargin )
    {
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_margin_left, nLeftMargin );
    }

    if( rHTMLWrt.nDfltRightMargin != rLRItem.GetRight() )
    {
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_margin_right,
                                 (long)rLRItem.GetRight() );
    }

    // The LineIndent of the first line might contain the room for numbering
    long nFirstLineIndent = (long)rLRItem.GetTxtFirstLineOfst() -
        rHTMLWrt.nFirstLineIndent;
    if( rHTMLWrt.nDfltFirstLineIndent != nFirstLineIndent )
    {
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_text_indent,
                                     nFirstLineIndent );
    }

    return rWrt;
}

static Writer& OutCSS1_SvxULSpace( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SvxULSpaceItem& rULItem = (const SvxULSpaceItem&)rHt;

    if( rHTMLWrt.nDfltTopMargin != rULItem.GetUpper() )
    {
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_margin_top,
                                     (long)rULItem.GetUpper() );
    }

    if( rHTMLWrt.nDfltBottomMargin != rULItem.GetLower() )
    {
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_margin_bottom,
                                     (long)rULItem.GetLower() );
    }

    return rWrt;
}

static Writer& OutCSS1_SvxULSpace_SvxLRSpace( Writer& rWrt,
                                        const SvxULSpaceItem *pULItem,
                                        const SvxLRSpaceItem *pLRItem )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    if( pLRItem && pULItem &&
        pLRItem->GetLeft() == pLRItem->GetRight() &&
        pLRItem->GetLeft() == pULItem->GetUpper() &&
        pLRItem->GetLeft() == pULItem->GetLower() &&
        pLRItem->GetLeft() != rHTMLWrt.nDfltLeftMargin &&
        pLRItem->GetRight() != rHTMLWrt.nDfltRightMargin &&
        pULItem->GetUpper() != rHTMLWrt.nDfltTopMargin &&
        pULItem->GetLower() != rHTMLWrt.nDfltBottomMargin )
    {
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_margin, (long)pLRItem->GetLeft() );
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

static Writer& OutCSS1_SvxULSpace_SvxLRSpace( Writer& rWrt,
                                        const SfxItemSet& rItemSet,
                                        sal_Bool bDeep )
{
    const SvxULSpaceItem *pULSpace = 0;
    const SvxLRSpaceItem *pLRSpace = 0;
    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_LR_SPACE, bDeep, &pItem ) )
        pLRSpace = (const SvxLRSpaceItem *)pItem;

    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_UL_SPACE, bDeep, &pItem ) )
        pULSpace = (const SvxULSpaceItem *)pItem;

    if( pLRSpace || pULSpace )
        OutCSS1_SvxULSpace_SvxLRSpace( rWrt, pULSpace, pLRSpace );

    return rWrt;
}

static Writer& OutCSS1_SvxFmtBreak_SwFmtPDesc_SvxFmtKeep( Writer& rWrt,
                                        const SvxFmtBreakItem *pBreakItem,
                                        const SwFmtPageDesc *pPDescItem,
                                        const SvxFmtKeepItem *pKeepItem )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    if( !rHTMLWrt.IsHTMLMode(HTMLMODE_PRINT_EXT) )
        return rWrt;

    const sal_Char *pBreakBefore = 0;
    const sal_Char *pBreakAfter = 0;

    if( pKeepItem )
    {
        pBreakAfter = pKeepItem->GetValue() ? sCSS1_PV_avoid : sCSS1_PV_auto;
    }
    if( pBreakItem )
    {
        switch( pBreakItem->GetBreak() )
        {
        case SVX_BREAK_NONE:
            pBreakBefore = sCSS1_PV_auto;
            if( !pBreakAfter )
                pBreakAfter = sCSS1_PV_auto;
            break;

        case SVX_BREAK_PAGE_BEFORE:
            pBreakBefore = sCSS1_PV_always;
            break;

        case SVX_BREAK_PAGE_AFTER:
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
            switch( pPDesc->GetPoolFmtId() )
            {
            case RES_POOLPAGE_LEFT:     pBreakBefore = sCSS1_PV_left;   break;
            case RES_POOLPAGE_RIGHT:    pBreakBefore = sCSS1_PV_right;  break;
            default:                    pBreakBefore = sCSS1_PV_always; break;
            }
        }
        else if( !pBreakBefore )
        {
            pBreakBefore = sCSS1_PV_auto;
        }
    }

    if( pBreakBefore )
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_page_break_before,
                                        pBreakBefore );
    if( pBreakAfter )
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_page_break_after,
                                        pBreakAfter );

    return rWrt;
}

static Writer& OutCSS1_SvxFmtBreak_SwFmtPDesc_SvxFmtKeep( Writer& rWrt,
                                        const SfxItemSet& rItemSet,
                                        sal_Bool bDeep )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    const SfxPoolItem *pItem;
    const SvxFmtBreakItem *pBreakItem = 0;
    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BREAK, bDeep, &pItem ))
        pBreakItem = (const SvxFmtBreakItem *)pItem;

    const SwFmtPageDesc *pPDescItem = 0;
    if( ( !rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) ||
          !rHTMLWrt.bCSS1IgnoreFirstPageDesc ||
          rHTMLWrt.pStartNdIdx->GetIndex() !=
                      rHTMLWrt.pCurPam->GetPoint()->nNode.GetIndex() ) &&
        SFX_ITEM_SET==rItemSet.GetItemState( RES_PAGEDESC, bDeep, &pItem ))
        pPDescItem = (const SwFmtPageDesc*)pItem;

    const SvxFmtKeepItem *pKeepItem = 0;
    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_KEEP, bDeep, &pItem ))
        pKeepItem = (const SvxFmtKeepItem *)pItem;

    if( pBreakItem || pPDescItem || pKeepItem )
        OutCSS1_SvxFmtBreak_SwFmtPDesc_SvxFmtKeep( rWrt, pBreakItem,
                                                   pPDescItem, pKeepItem );

    return rWrt;
}

// Wrapper for OutCSS1_SfxItemSet etc.
static Writer& OutCSS1_SvxBrush( Writer& rWrt, const SfxPoolItem& rHt )
{
    OutCSS1_SvxBrush( rWrt, rHt, CSS1_BACKGROUND_ATTR );
    return rWrt;
}

static Writer& OutCSS1_SvxBrush( Writer& rWrt, const SfxPoolItem& rHt,
                                 sal_uInt16 nMode)
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // The Character-Attribute is skipped, if we are about to
    // exporting options
    if( rHt.Which() < RES_CHRATR_END &&
        rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        return rWrt;

    // start getting a few values
//  const Brush &rBrush = ((const SvxBrushItem &)rHt).GetBrush();
    const Color & rColor = ((const SvxBrushItem &)rHt).GetColor();
    SvxGraphicPosition ePos = ((const SvxBrushItem &)rHt).GetGraphicPos();

    // get the color
    bool bColor = false;
    /// set <bTransparent> to sal_True, if color is "no fill"/"auto fill"
    bool bTransparent = (rColor.GetColor() == COL_TRANSPARENT);
    Color aColor;
    if( !bTransparent )
    {
        aColor = rColor;
        bColor = true;
    }

    // and now the Graphic
    OUString aGraphicInBase64;

    // Embedded Grafic -> export WriteEmbedded
    const Graphic* pGrf = ((const SvxBrushItem &)rHt).GetGraphic();
    if( pGrf )
    {
        sal_uLong nErr = XOutBitmap::GraphicToBase64(*pGrf, aGraphicInBase64);
        if( nErr )
        {
            rHTMLWrt.nWarn = WARN_SWG_POOR_LOAD | WARN_SW_WRITE_BASE;
        }
    }

    // In tables we only export something if there is a Graphic
    if( CSS1_BACKGROUND_TABLE==nMode && !pGrf )
        return rWrt;

    // if necessary, add the orientation of the Graphic
    const sal_Char *pRepeat = 0, *pHori = 0, *pVert = 0;
    if( pGrf )
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

            if( pHori || pVert )
                pRepeat = sCSS1_PV_no_repeat;
        }
    }

    // now build the string
    OUString sOut;
    if( !pGrf && !bColor )
    {
        // no color and no Link, but a transparent Brush
        if( bTransparent && CSS1_BACKGROUND_FLY != nMode )
            sOut += OStringToOUString(sCSS1_PV_transparent, RTL_TEXTENCODING_ASCII_US);
    }
    else
    {
        if( bColor )
        {
            OString sTmp(GetCSS1Color(aColor));
            sOut += OStringToOUString(sTmp, RTL_TEXTENCODING_ASCII_US);
        }

        if( pGrf )
        {
            if( bColor )
                sOut += " ";

            sOut += OStringToOUString(sCSS1_url, RTL_TEXTENCODING_ASCII_US) +
                "(\'" + OOO_STRING_SVTOOLS_HTML_O_data +  ":" + aGraphicInBase64 + "\')";

            if( pRepeat )
            {
                sOut += " " + OStringToOUString(pRepeat, RTL_TEXTENCODING_ASCII_US);
            }

            if( pHori )
            {
                sOut += " " + OStringToOUString(pHori, RTL_TEXTENCODING_ASCII_US);
            }
            if( pVert )
            {
                sOut += " " + OStringToOUString(pVert, RTL_TEXTENCODING_ASCII_US);
            }

            sOut += " " + OStringToOUString(sCSS1_PV_scroll, RTL_TEXTENCODING_ASCII_US) + " ";
        }
    }

    if( !sOut.isEmpty() )
        rHTMLWrt.OutCSS1_Property( sCSS1_P_background, sOut );

    return rWrt;
}

static void OutCSS1_SvxBorderLine( SwHTMLWriter& rHTMLWrt,
                                   const sal_Char *pProperty,
                                   const SvxBorderLine *pLine )
{
    if( !pLine || pLine->isEmpty() )
    {
        rHTMLWrt.OutCSS1_PropertyAscii( pProperty, sCSS1_PV_none );
        return;
    }

    sal_Int32 nWidth = pLine->GetWidth();

    OStringBuffer sOut;
    if( Application::GetDefaultDevice() &&
        nWidth <= Application::GetDefaultDevice()->PixelToLogic(
                    Size( 1, 1 ), MapMode( MAP_TWIP) ).Width() )
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
                    OString::number(nWidth % 10) + OString(sCSS1_UNIT_pt));
    }

    // Line-Style: solid or double
    sOut.append(' ');
    switch (pLine->GetBorderLineStyle())
    {
        case table::BorderLineStyle::SOLID:
            sOut.append(sCSS1_PV_solid);
            break;
        case table::BorderLineStyle::DOTTED:
            sOut.append(sCSS1_PV_dotted);
            break;
        case table::BorderLineStyle::DASHED:
            sOut.append(sCSS1_PV_dashed);
            break;
        case table::BorderLineStyle::DOUBLE:
        case table::BorderLineStyle::THINTHICK_SMALLGAP:
        case table::BorderLineStyle::THINTHICK_MEDIUMGAP:
        case table::BorderLineStyle::THINTHICK_LARGEGAP:
        case table::BorderLineStyle::THICKTHIN_SMALLGAP:
        case table::BorderLineStyle::THICKTHIN_MEDIUMGAP:
        case table::BorderLineStyle::THICKTHIN_LARGEGAP:
            sOut.append(sCSS1_PV_double);
            break;
        case table::BorderLineStyle::EMBOSSED:
            sOut.append(sCSS1_PV_ridge);
            break;
        case table::BorderLineStyle::ENGRAVED:
            sOut.append(sCSS1_PV_groove);
            break;
        case table::BorderLineStyle::INSET:
            sOut.append(sCSS1_PV_inset);
            break;
        case table::BorderLineStyle::OUTSET:
            sOut.append(sCSS1_PV_outset);
            break;
        default:
            sOut.append(sCSS1_PV_none);
    }
    sOut.append(' ');

    // and also the color
    sOut.append(GetCSS1Color(pLine->GetColor()));

    rHTMLWrt.OutCSS1_PropertyAscii(pProperty, sOut.makeStringAndClear());
}

Writer& OutCSS1_SvxBox( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Avoid interference between character and paragraph attributes
    if( rHt.Which() < RES_CHRATR_END &&
        rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        return rWrt;

    if( rHt.Which() == RES_CHRATR_BOX )
    {
        if( rHTMLWrt.bTagOn )
        {
            // Inline-block to make the line height changing correspond to the character border
            rHTMLWrt.OutCSS1_PropertyAscii(sCSS1_P_display, "inline-block");
        }
        else
        {
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_span, false );
            return rWrt;
        }
    }

    const SvxBoxItem& rBoxItem = (const SvxBoxItem&)rHt;
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
        OutCSS1_SvxBorderLine( rHTMLWrt, sCSS1_P_border, pTop );
    }
    else
    {
        // otherwise export all Lines separately
        OutCSS1_SvxBorderLine( rHTMLWrt, sCSS1_P_border_top, pTop );
        OutCSS1_SvxBorderLine( rHTMLWrt, sCSS1_P_border_bottom, pBottom );
        OutCSS1_SvxBorderLine( rHTMLWrt, sCSS1_P_border_left, pLeft );
        OutCSS1_SvxBorderLine( rHTMLWrt, sCSS1_P_border_right, pRight );
    }

    long nTopDist = pTop ? rBoxItem.GetDistance( BOX_LINE_TOP ) : 0;
    long nBottomDist = pBottom ? rBoxItem.GetDistance( BOX_LINE_BOTTOM ) : 0;
    long nLeftDist = pLeft ? rBoxItem.GetDistance( BOX_LINE_LEFT ) : 0;
    long nRightDist = pRight ? rBoxItem.GetDistance( BOX_LINE_RIGHT ) : 0;

    if( nTopDist == nBottomDist && nLeftDist == nRightDist )
    {
        OStringBuffer sVal;
        AddUnitPropertyValue(sVal, nTopDist, rHTMLWrt.GetCSS1Unit());
        if( nTopDist != nLeftDist )
        {
            sVal.append(' ');
            AddUnitPropertyValue(sVal, nLeftDist, rHTMLWrt.GetCSS1Unit());
        }
        rHTMLWrt.OutCSS1_PropertyAscii(sCSS1_P_padding, sVal.makeStringAndClear());
    }
    else
    {
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_padding_top, nTopDist );
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_padding_bottom, nBottomDist );
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_padding_left, nLeftDist );
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_padding_right, nRightDist );
    }

    return rWrt;
}

static Writer& OutCSS1_SvxFrameDirection( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast< SwHTMLWriter& >( rWrt  );

    // Language will be exported rules only
    if( !rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_TEMPLATE ) )
        return rWrt;

    sal_uInt16 nDir =
        static_cast< const SvxFrameDirectionItem& >( rHt ).GetValue();
    const sal_Char* pStr = NULL;
    switch( nDir )
    {
    case FRMDIR_HORI_LEFT_TOP:
    case FRMDIR_VERT_TOP_LEFT:
        pStr = sCSS1_PV_ltr;
        break;
    case FRMDIR_HORI_RIGHT_TOP:
    case FRMDIR_VERT_TOP_RIGHT:
        pStr = sCSS1_PV_rtl;
        break;
    case FRMDIR_ENVIRONMENT:
        pStr = sCSS1_PV_inherit;
        break;
    }

    if( pStr )
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_direction, pStr );

    return rWrt;
}

/*
 * Place here the table for the HTML-Function-Pointer to the
 * Export-Functions.
 * They are local structures, only needed within the HTML-DLL.
 */

SwAttrFnTab aCSS1AttrFnTab = {
/* RES_CHRATR_CASEMAP   */          OutCSS1_SvxCaseMap,
/* RES_CHRATR_CHARSETCOLOR  */      0,
/* RES_CHRATR_COLOR */              OutCSS1_SvxColor,
/* RES_CHRATR_CONTOUR   */          0,
/* RES_CHRATR_CROSSEDOUT    */      OutCSS1_SvxCrossedOut,
/* RES_CHRATR_ESCAPEMENT    */      0,
/* RES_CHRATR_FONT  */              OutCSS1_SvxFont,
/* RES_CHRATR_FONTSIZE  */          OutCSS1_SvxFontHeight,
/* RES_CHRATR_KERNING   */          OutCSS1_SvxKerning,
/* RES_CHRATR_LANGUAGE  */          OutCSS1_SvxLanguage,
/* RES_CHRATR_POSTURE   */          OutCSS1_SvxPosture,
/* RES_CHRATR_PROPORTIONALFONTSIZE*/0,
/* RES_CHRATR_SHADOWED  */          0,
/* RES_CHRATR_UNDERLINE */          OutCSS1_SvxUnderline,
/* RES_CHRATR_WEIGHT    */          OutCSS1_SvxFontWeight,
/* RES_CHRATR_WORDLINEMODE  */      0,
/* RES_CHRATR_AUTOKERN  */          0,
/* RES_CHRATR_BLINK */              OutCSS1_SvxBlink,
/* RES_CHRATR_NOHYPHEN  */          0, // Neu: nicht trennen
/* RES_CHRATR_NOLINEBREAK */        0, // Neu: nicht umbrechen
/* RES_CHRATR_BACKGROUND */         OutCSS1_SvxBrush, // Neu: Zeichenhintergrund
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
/* RES_CHRATR_ROTATE */             0,
/* RES_CHRATR_EMPHASIS_MARK */      0,
/* RES_CHRATR_TWO_LINES */          0,
/* RES_CHRATR_SCALEW */             0,
/* RES_CHRATR_RELIEF */             0,
/* RES_CHRATR_HIDDEN */             OutCSS1_SvxHidden,
/* RES_CHRATR_OVERLINE */           OutCSS1_SvxOverline,
/* RES_CHRATR_RSID */               0,
/* RES_CHRATR_BOX */                OutCSS1_SvxBox,
/* RES_CHRATR_SHADOW */             0,
/* RES_CHRATR_HIGHLIGHT */          0,
/* RES_CHRATR_GRABBAG */            0,
/* RES_CHRATR_BIDIRTL */            0,
/* RES_CHRATR_IDCTHINT */           0,

/* RES_TXTATR_REFMARK */            0,
/* RES_TXTATR_TOXMARK */            0,
/* RES_TXTATR_META */               0,
/* RES_TXTATR_METAFIELD */          0,
/* RES_TXTATR_AUTOFMT */            0,
/* RES_TXTATR_INETFMT */            0,
/* RES_TXTATR_CHARFMT */            0,
/* RES_TXTATR_CJK_RUBY */           0,
/* RES_TXTATR_UNKNOWN_CONTAINER */  0,
/* RES_TXTATR_INPUTFIELD */         0,

/* RES_TXTATR_FIELD */              0,
/* RES_TXTATR_FLYCNT */             0,
/* RES_TXTATR_FTN */                0,
/* RES_TXTATR_ANNOTATION */         0,
/* RES_TXTATR_DUMMY3 */             0,
/* RES_TXTATR_DUMMY1 */             0, // Dummy:
/* RES_TXTATR_DUMMY2 */             0, // Dummy:

/* RES_PARATR_LINESPACING   */      OutCSS1_SvxLineSpacing,
/* RES_PARATR_ADJUST    */          OutCSS1_SvxAdjust,
/* RES_PARATR_SPLIT */              OutCSS1_SvxFmtSplit,
/* RES_PARATR_WIDOWS    */          OutCSS1_SvxWidows,
/* RES_PARATR_ORPHANS   */          OutCSS1_SvxOrphans,
/* RES_PARATR_TABSTOP   */          0,
/* RES_PARATR_HYPHENZONE*/          0,
/* RES_PARATR_DROP */               OutCSS1_SwFmtDrop,
/* RES_PARATR_REGISTER */           0, // neu:  Registerhaltigkeit
/* RES_PARATR_NUMRULE */            0,
/* RES_PARATR_SCRIPTSPACE */        0,
/* RES_PARATR_HANGINGPUNCTUATION */ 0,
/* RES_PARATR_FORBIDDEN_RULES */    0, // new
/* RES_PARATR_VERTALIGN */          0, // new
/* RES_PARATR_SNAPTOGRID*/          0, // new
/* RES_PARATR_CONNECT_TO_BORDER */  0, // new
/* RES_PARATR_OUTLINELEVEL */       0, // new since cws outlinelevel
/* RES_PARATR_RSID */               0, // new
/* RES_PARATR_GRABBAG */            0,

/* RES_PARATR_LIST_ID */            0, // new
/* RES_PARATR_LIST_LEVEL */         0, // new
/* RES_PARATR_LIST_ISRESTART */     0, // new
/* RES_PARATR_LIST_RESTARTVALUE */  0, // new
/* RES_PARATR_LIST_ISCOUNTED */     0, // new

/* RES_FILL_ORDER   */              0,
/* RES_FRM_SIZE */                  0,
/* RES_PAPER_BIN    */              0,
/* RES_LR_SPACE */                  OutCSS1_SvxLRSpace,
/* RES_UL_SPACE */                  OutCSS1_SvxULSpace,
/* RES_PAGEDESC */                  0,
/* RES_BREAK */                     0,
/* RES_CNTNT */                     0,
/* RES_HEADER */                    0,
/* RES_FOOTER */                    0,
/* RES_PRINT */                     0,
/* RES_OPAQUE */                    0,
/* RES_PROTECT */                   0,
/* RES_SURROUND */                  0,
/* RES_VERT_ORIENT */               0,
/* RES_HORI_ORIENT */               0,
/* RES_ANCHOR */                    0,
/* RES_BACKGROUND */                OutCSS1_SvxBrush,
/* RES_BOX  */                      OutCSS1_SvxBox,
/* RES_SHADOW */                    0,
/* RES_FRMMACRO */                  0,
/* RES_COL */                       0,
/* RES_KEEP */                      0,
/* RES_URL */                       0,
/* RES_EDIT_IN_READONLY */          0,
/* RES_LAYOUT_SPLIT */              0,
/* RES_CHAIN */                     0,
/* RES_TEXTGRID */                  0,
/* RES_LINENUMBER */                0,
/* RES_FTN_AT_TXTEND */             0,
/* RES_END_AT_TXTEND */             0,
/* RES_COLUMNBALANCE */             0,
/* RES_FRAMEDIR */                  OutCSS1_SvxFrameDirection,
/* RES_HEADER_FOOTER_EAT_SPACING */ 0,
/* RES_ROW_SPLIT */                 0,
/* RES_FOLLOW_TEXT_FLOW */          0,
/* RES_COLLAPSING_BORDERS */        0,
/* RES_WRAP_INFLUENCE_ON_OBJPOS */  0,
/* RES_AUTO_STYLE */                0,
/* RES_FRMATR_STYLE_NAME */         0,
/* RES_FRMATR_CONDITIONAL_STYLE_NAME */ 0,
/* RES_FRMATR_GRABBAG */            0,
/* RES_TEXT_VERT_ADJUST */          0,

/* RES_GRFATR_MIRRORGRF */          0,
/* RES_GRFATR_CROPGRF   */          0,
/* RES_GRFATR_ROTATION */           0,
/* RES_GRFATR_LUMINANCE */          0,
/* RES_GRFATR_CONTRAST */           0,
/* RES_GRFATR_CHANNELR */           0,
/* RES_GRFATR_CHANNELG */           0,
/* RES_GRFATR_CHANNELB */           0,
/* RES_GRFATR_GAMMA */              0,
/* RES_GRFATR_INVERT */             0,
/* RES_GRFATR_TRANSPARENCY */       0,
/* RES_GRFATR_DRWAMODE */           0,
/* RES_GRFATR_DUMMY1 */             0,
/* RES_GRFATR_DUMMY2 */             0,
/* RES_GRFATR_DUMMY3 */             0,
/* RES_GRFATR_DUMMY4 */             0,
/* RES_GRFATR_DUMMY5 */             0,

/* RES_BOXATR_FORMAT */             0,
/* RES_BOXATR_FORMULA */            0,
/* RES_BOXATR_VALUE */              0
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
