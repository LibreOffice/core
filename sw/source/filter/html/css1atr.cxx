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

//-----------------------------------------------------------------------

sal_Char CSS1_CONSTASCII_DEF( sCSS1_rule_end, " }" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_span_tag_end, "\">" );
const sal_Char cCSS1_style_opt_end = '\"';

sal_Char CSS1_CONSTASCII_DEF( sHTML_FTN_fontheight, "57%" );

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

    // Pointer an das Bufferende setzen
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
        Strm() << sOut.makeStringAndClear().getStr();
        OutNewLine();
        Strm() << '<' << OOO_STRING_SVTOOLS_HTML_comment;

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
                HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_span, sal_False );
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
        // In STYLE-Optionen den String codieren
        Strm() << sOut.makeStringAndClear().getStr();
        if( pVal )
            HTMLOutFuncs::Out_String( Strm(), OUString::createFromAscii(pVal),
                                      eDestEnc, &aNonConvertableCharacters );
        else if( pSVal )
            HTMLOutFuncs::Out_String( Strm(), *pSVal, eDestEnc, &aNonConvertableCharacters );
    }
    else
    {
        // Im STYLE-Tag des String direct ausgeben
        if( pVal )
            sOut.append(pVal);
        else if( pSVal )
            sOut.append(OUStringToOString(*pSVal, eDestEnc));
    }

    if (!sOut.isEmpty())
        Strm() << sOut.makeStringAndClear().getStr();
}

static void AddUnitPropertyValue(OStringBuffer &rOut, long nVal,
    FieldUnit eUnit)
{
    if( nVal < 0 )
    {
        // Vorzeichen extra behandeln
        nVal = -nVal;
        rOut.append('-');
    }

    // Die umgerechnete Einheit ergibt sich aus (x * nMul)/(nDiv*nFac*10)
    long nMul = 1000;
    long nDiv = 1;
    long nFac = 100;
    const sal_Char *pUnit;
    switch( eUnit )
    {
    case FUNIT_100TH_MM:
        OSL_ENSURE( FUNIT_MM == eUnit, "Masseinheit wird nicht unterstuetzt" );
    case FUNIT_MM:
        // 0.01mm = 0.57twip
        nMul = 25400;   // 25.4 * 1000
        nDiv = 1440;    // 72 * 20;
        nFac = 100;
        pUnit = sCSS1_UNIT_mm;
        break;

    case FUNIT_M:
    case FUNIT_KM:
        OSL_ENSURE( FUNIT_CM == eUnit, "Masseinheit wird nicht unterstuetzt" );
    case FUNIT_CM:
        // 0.01cm = 5.7twip (ist zwar ungenau, aber die UI ist auch ungenau)
        nMul = 2540;    // 2.54 * 1000
        nDiv = 1440;    // 72 * 20;
        nFac = 100;
        pUnit = sCSS1_UNIT_cm;
        break;

    case FUNIT_TWIP:
        OSL_ENSURE( FUNIT_POINT == eUnit, "Masseinheit wird nicht unterstuetzt" );
    case FUNIT_POINT:
        // 0.1pt = 2.0twip (ist zwar ungenau, aber die UI ist auch ungenau)
        nMul = 100;
        nDiv = 20;
        nFac = 10;
        pUnit = sCSS1_UNIT_pt;
        break;

    case FUNIT_PICA:
        // 0.01pc = 2.40twip (ist zwar ungenau, aber die UI ist auch ungenau)
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
        OSL_ENSURE( FUNIT_INCH == eUnit, "Masseinheit wird nicht unterstuetzt" );
        // 0.01in = 14.4twip (ist zwar ungenau, aber die UI ist auch ungenau)
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
        // Zum Unrechnen der Einheit wird ein BigInt benoetigt
#ifdef SAL_INT64_IS_STRUCT
        BigInt nBigVal( nVal );
        nBigVal *= nMul;
        nBigVal /= nDiv;
        nBigVal += 5;
        nBigVal /= 10;

        if( nBigVal.IsLong() )
        {
            // Zum Ausgeben des Wertes reicht ein long.
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
            // Zum Ausgeben des Wertes reicht ein long.
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
    if( nVal && Application::GetDefaultDevice() )
    {
        Size aSz( bVert ? 0 : nVal, bVert ? nVal : 0 );
        aSz = Application::GetDefaultDevice()->LogicToPixel( aSz, MapMode( MAP_TWIP) );
        nVal = bVert ? aSz.Height() : aSz.Width();
        if( !nVal )
            nVal = 1;
    }

    OString sOut(OString::number(nVal) + sCSS1_UNIT_px);
    OutCSS1_PropertyAscii(pProp, sOut);
}

void SwHTMLWriter::OutCSS1_SfxItemSet( const SfxItemSet& rItemSet,
                                       sal_Bool bDeep )
{
    // den ItemSet ausgeben, und zwar inklusive aller Attribute
    Out_SfxItemSet( aCSS1AttrFnTab, *this, rItemSet, bDeep );

    // ein par Attribute benoetigen eine Spezial-Behandlung
    const SfxPoolItem *pItem = 0;

    // Underline, Overline, CrossedOut und Blink bilden zusammen eine CSS1-Property
    // (geht natuerlich nicht bei Hints)
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
        // wenn eine Property als Bestandteil einer Style-Option
        // ausgegeben wurde, muss die Optiomn noch beendet werden
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
            Strm() << sOut.makeStringAndClear().getStr();
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

        // Erstmal versuchen wir zu erraten, wie das Dokument so augebaut ist.
        // Erlaubt sind nur die Vorlagen HTML, erste Seite, linke Seite und
        // rechte Seite.
        // Eine erste Seite wird nur exportiert, wenn die erste Seite auch
        // wirklich die Vorlage "erste Seite" ist.
        // Linke und rechte Seiten werden nur exportiert, wenn diese beiden
        // Vorlagen untereinander verkettet werden.
        // Wenn andere Vorlagen verwendet werden, wird nur in sehr einfachen
        // Faellen etwas exportiert.
        const SwPageDesc *pPageDesc = &rPageDesc;
        const SwPageDesc *pFollow = rPageDesc.GetFollow();
        if( RES_POOLPAGE_FIRST == pPageDesc->GetPoolFmtId() &&
            pFollow != pPageDesc &&
            !IsPoolUserFmt( pFollow->GetPoolFmtId() ) )
        {
            // Das Dokument hat eine erste Seite
            pFirstPageDesc = pPageDesc;
            pPageDesc = pFollow;
            pFollow = pPageDesc->GetFollow();
        }

        IDocumentStylePoolAccess* pStylePoolAccess = getIDocumentStylePoolAccess();
        if( pPageDesc == pFollow )
        {
            // Das Dokument ist einseitig. Egal welche Seite verwendet wird,
            // es wird kein zweiseitiges Dokument daraus gemacht.
            // Die Attributierung wird relativ zur HTML-Seitenvorlage
            // aus der HTML-Vorlage exportiert.
          OutCSS1_SwPageDesc( *this, *pPageDesc, pStylePoolAccess, pTemplate,
                                RES_POOLPAGE_HTML, true, false );
            nFirstRefPoolId = pFollow->GetPoolFmtId();
        }
        else if( (RES_POOLPAGE_LEFT == pPageDesc->GetPoolFmtId() &&
                  RES_POOLPAGE_RIGHT == pFollow->GetPoolFmtId()) ||
                 (RES_POOLPAGE_RIGHT == pPageDesc->GetPoolFmtId() &&
                  RES_POOLPAGE_LEFT == pFollow->GetPoolFmtId()) )
        {
            // Das Dokument ist zweiseitig
          OutCSS1_SwPageDesc( *this, *pPageDesc, pStylePoolAccess, pTemplate,
                                RES_POOLPAGE_HTML, true );
          OutCSS1_SwPageDesc( *this, *pFollow, pStylePoolAccess, pTemplate,
                                RES_POOLPAGE_HTML, true );
            nFirstRefPoolId = RES_POOLPAGE_RIGHT;
            bCSS1IgnoreFirstPageDesc = sal_False;
        }
        // Alles andere bekommen wir nicht hin.

        if( pFirstPageDesc )
          OutCSS1_SwPageDesc( *this, *pFirstPageDesc, pStylePoolAccess, pTemplate,
                                nFirstRefPoolId, false );
    }
// /Feature: PrintExt


    // The text body style has to be exported always (if it is changed compared
    // to the template), because it is used as reference for any style
    // that maps to <P>, and that's especially the standard style
    getIDocumentStylePoolAccess()->GetTxtCollFromPool( RES_POOLCOLL_TEXT, false );

    // das Default-TextStyle wir nicht mit ausgegeben !!
    // das 0-Style ist das Default, wird nie ausgegeben !!
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

    // das Default-TextStyle wir nicht mit ausgegeben !!
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
        Strm() << "-->";

        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_style, sal_False );
    }
    else
    {
        bFirstCSS1Rule = sal_False;
    }

    nDfltTopMargin = 0;
    nDfltBottomMargin = 0;
}

//-----------------------------------------------------------------------

// wenn pPseudo gesetzt ist werden Styles-Sheets ausgegeben,
// sonst wird nur nach Token und Class fuer ein Format gesucht
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

    // Nach oben die Formate abklappern, bis man auf eine Standard-
    // oder eine HTML-Tag-Vorlage trifft
    const SwFmt *pPFmt = pFmt;
    while( pPFmt && !pPFmt->IsDefault() )
    {
        bool bStop = false;
        sal_uInt16 nPoolId = pPFmt->GetPoolFmtId();
        if( USER_FMT & nPoolId )
        {
            // Benutzer-Vorlagen
            const OUString& rNm = pPFmt->GetName();
            switch( rNm[0] )
            {
                        // nicht mehr unterstuetzt:
                        // OOO_STRING_SVTOOLS_HTML_author
                        // OOO_STRING_SVTOOLS_HTML_acronym
                        // OOO_STRING_SVTOOLS_HTML_abbreviation
                        // OOO_STRING_SVTOOLS_HTML_deletedtext
                        // OOO_STRING_SVTOOLS_HTML_insertedtext
                        // OOO_STRING_SVTOOLS_HTML_language
                        // OOO_STRING_SVTOOLS_HTML_person
            case 'B':   if( !bChrFmt && rNm == OOO_STRING_SVTOOLS_HTML_blockquote )
                        {
                            rRefPoolId = RES_POOLCOLL_HTML_BLOCKQUOTE;
                            rToken = OString(OOO_STRING_SVTOOLS_HTML_blockquote);
                        }
                        break;
            case 'C':   if( bChrFmt )
                        {
                            if( rNm == OOO_STRING_SVTOOLS_HTML_citiation )
                            {
                                rRefPoolId = RES_POOLCHR_HTML_CITIATION;
                                rToken = OString(OOO_STRING_SVTOOLS_HTML_citiation);
                            }
                            else if( rNm == OOO_STRING_SVTOOLS_HTML_code )
                            {
                                rRefPoolId = RES_POOLCHR_HTML_CODE;
                                rToken = OString(OOO_STRING_SVTOOLS_HTML_code);
                            }
                        }
                        break;
            case 'D':   if( bChrFmt && rNm == OOO_STRING_SVTOOLS_HTML_definstance )
                        {
                            rRefPoolId = RES_POOLCHR_HTML_DEFINSTANCE;
                            rToken = OString(OOO_STRING_SVTOOLS_HTML_definstance);
                        }
                        else if( !bChrFmt )
                        {
                            sal_uInt16 nDefListLvl = GetDefListLvl( rNm, nPoolId );
                            // Die Vorlagen DD 1/DT 1 werden ausgegeben,
                            // aber keine von ihnen abgeleiteten Vorlagen,
                            // auch nicht DD 2/DT 2 etc.
                            if( nDefListLvl )
                            {
                                if( pPseudo &&
                                    (nDeep || (nDefListLvl & 0x0fff) > 1) )
                                {
                                    bStop = true;
                                }
                                else if( nDefListLvl & HTML_DLCOLL_DD )
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
                        break;
            case 'E':   if( bChrFmt && rNm == OOO_STRING_SVTOOLS_HTML_emphasis )
                        {
                            rRefPoolId = RES_POOLCHR_HTML_EMPHASIS;
                            rToken = OString(OOO_STRING_SVTOOLS_HTML_emphasis);
                        }
                        break;
            case 'H':   if( !bChrFmt && rNm == OOO_STRING_SVTOOLS_HTML_horzrule )
                            // HR nicht ausgeben!
                            bStop = (nDeep==0);
                        break;
            case 'K':   if( bChrFmt && rNm == OOO_STRING_SVTOOLS_HTML_keyboard )
                        {
                            rRefPoolId = RES_POOLCHR_HTML_KEYBOARD;
                            rToken = OString(OOO_STRING_SVTOOLS_HTML_keyboard);
                        }
                        break;
            case 'L':   if( !bChrFmt && rNm == OOO_STRING_SVTOOLS_HTML_listing )
                        {
                            // Listing als PRE exportieren bzw. von
                            // PRE abgeleitete Vorlage exportieren
                            rToken = OString(OOO_STRING_SVTOOLS_HTML_preformtxt);
                            rRefPoolId = RES_POOLCOLL_HTML_PRE;
                            nDeep = CSS1_FMT_CMPREF;
                        }
                        break;
            case 'P':   if( !bChrFmt && rNm == OOO_STRING_SVTOOLS_HTML_preformtxt )
                        {
                            rRefPoolId = RES_POOLCOLL_HTML_PRE;
                            rToken = OString(OOO_STRING_SVTOOLS_HTML_preformtxt);
                        }
                        break;
            case 'S':   if( bChrFmt )
                        {
                            if( rNm == OOO_STRING_SVTOOLS_HTML_sample )
                            {
                                rRefPoolId = RES_POOLCHR_HTML_SAMPLE;
                                rToken = OString(OOO_STRING_SVTOOLS_HTML_sample);
                            }
                            else if( rNm == OOO_STRING_SVTOOLS_HTML_strong )
                            {
                                rRefPoolId = RES_POOLCHR_HTML_STRONG;
                                rToken = OString(OOO_STRING_SVTOOLS_HTML_strong);
                            }
                        }
                        break;
            case 'T':   if( bChrFmt && rNm == OOO_STRING_SVTOOLS_HTML_teletype )
                        {
                            rRefPoolId = RES_POOLCHR_HTML_TELETYPE;
                            rToken = OString(OOO_STRING_SVTOOLS_HTML_teletype);
                        }
                        break;
            case 'V':   if( bChrFmt && rNm == OOO_STRING_SVTOOLS_HTML_variable )
                        {
                            rRefPoolId = RES_POOLCHR_HTML_VARIABLE;
                            rToken = OString(OOO_STRING_SVTOOLS_HTML_variable);
                        }
                        break;
            case 'X':   if( !bChrFmt && rNm == OOO_STRING_SVTOOLS_HTML_xmp )
                        {
                            // XMP als PRE exportieren (aber nicht die
                            // Vorlage als Style)
                            rToken = OString(OOO_STRING_SVTOOLS_HTML_preformtxt);
                            rRefPoolId = RES_POOLCOLL_HTML_PRE;
                            nDeep = CSS1_FMT_CMPREF;
                        }
                        break;
            }

            // Wenn eine PoolId gesetzt ist, entspricht der Name der
            // Vorlage dem szugehoerigen Token
            OSL_ENSURE( (rRefPoolId != 0) == (!rToken.isEmpty()),
                    "Token missing" );
        }
        else
        {
            // Pool-Vorlagen
            switch( nPoolId )
            {
            // Absatz-Vorlagen
            case RES_POOLCOLL_HEADLINE_BASE:
            case RES_POOLCOLL_STANDARD:
                // diese Vorlagen nicht ausgeben
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
                // HR nicht ausgeben!
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

            // Zeichen-Vorlagen
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

            // Wenn ein Token gesetzt ist, enthaelt nPoolId die dazugehoerige
            // Vorlage
            if( !rToken.isEmpty() && !rRefPoolId )
                rRefPoolId = nPoolId;
        }

        if( !rToken.isEmpty() || bStop )
        {
            // Anhalten wenn eine HTML-Tag-Vorlage gefunden wurde
            break;
        }
        else
        {
            // sonst weitersuchen
            nDeep++;
            pPFmt = pPFmt->DerivedFrom();
        }
    }

    if( !rToken.isEmpty() )
    {
        // Es ist eine HTML-Tag-Vorlage
        if( !nDeep )
            nDeep = CSS1_FMT_ISTAG;
    }
    else
    {
        // Es ist keine HTML-Tag-Vorlage und auch keine davon abgeleitete
        nDeep = 0;
    }
    if( nDeep > 0 && nDeep < CSS1_FMT_SPECIAL )
    {
        // Wenn die Vorlage von einer HTML-Vorlage abgeleitet ist,
        // wird sie als <TOKEN>.<CLASS> exportiert, sonst als .<CLASS>.
        // <CLASS> ergibt sich aus dem Namen der Vorlage durch entfernen
        // aller Zeichen vor und inklusive dem ersten '.'
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
                "In der Dok-Vorlage gibt es keine Benutzer-Vorlagen" );
        if( POOLGRP_NOCOLLID & nPoolFmtId )
            pRefFmt = pTemplate->GetCharFmtFromPool( nPoolFmtId );
        else
            pRefFmt = pTemplate->GetTxtCollFromPool( nPoolFmtId, false );
    }

    return pRefFmt;
}

const SwFmt *SwHTMLWriter::GetParentFmt( const SwFmt& rFmt, sal_uInt16 nDeep )
{
    OSL_ENSURE( nDeep != USHRT_MAX, "GetParent fuer HTML-Vorlage aufgerufen!" );
    const SwFmt *pRefFmt = 0;

    if( nDeep > 0 )
    {
        // hier wird die HTML-Tag-Vorlage, von der die Vorlage abgeleitet
        // ist als Referenz geholt
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
            "SwHTMLWriter::SubtractItemSet: Bei diesen Flags passiert nix" );
    SfxItemSet aRefItemSet( *rRefItemSet.GetPool(), rRefItemSet.GetRanges() );
    aRefItemSet.Set( rRefItemSet );

    // und mit dem Attr-Set der Vorlage vergleichen
    SfxWhichIter aIter( rItemSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        const SfxPoolItem *pRefItem, *pItem;
        bool bItemSet = ( SFX_ITEM_SET ==
                rItemSet.GetItemState( nWhich, sal_False, &pItem) );
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
                    pRefScriptItemSet->GetItemState( nWhich, sal_True, &pRefItem) );
                break;
            default:
                bRefItemSet = ( SFX_ITEM_SET ==
                    aRefItemSet.GetItemState( nWhich, sal_False, &pRefItem) );
                break;
            }
        }
        else
        {
            bRefItemSet = ( SFX_ITEM_SET ==
                aRefItemSet.GetItemState( nWhich, sal_False, &pRefItem) );
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
                // das Attribut ist mit dem gleichen Wert in beiden
                // Vorlagen vorhanden und muss nicht ausgegeben werden
                rItemSet.ClearItem( nWhich );
            }
        }
        else
        {
            if( (bSetDefaults || pRefScriptItemSet) && bRefItemSet )
            {
                // das Attribut ist nur in der Referenz vorhanden. Das
                // Default muss ggf. ausgegeben werden
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
    rNames = aEmptyStr;
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
        if( SFX_ITEM_SET == rItemSet.GetItemState( aWhichIds[i], sal_False,
                                                   &pTmp ) )
        {
            pItem = pTmp;
            nItemCount++;
        }
        if( SFX_ITEM_SET == rItemSet.GetItemState( aWhichIds[i+1], sal_False,
                                                   &pTmp ) )
        {
            pItemCJK = pTmp;
            nItemCount++;
        }
        if( SFX_ITEM_SET == rItemSet.GetItemState( aWhichIds[i+2], sal_False,
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
        SFX_ITEM_SET == rItemSet.GetItemState( RES_PARATR_DROP, sal_True,
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
            aTstItemSet.Set( pDCCharFmt->GetAttrSet(), sal_True );
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
                aScriptItemSet.Set( pDCCharFmt->GetAttrSet(), sal_True );

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
        // diese Vorlagen-Typen koennen exportiert werden
        break;

    default:
        // und diese nicht
        return rWrt;
    }

    // den Selector und die auszugebende Attr-Set-Tiefe ermitteln
    OUString aSelector;
    sal_uInt16 nRefPoolId = 0;
    sal_uInt16 nDeep = GetCSS1Selector( &rFmt, aSelector, nRefPoolId );
    if( !nDeep )
        return rWrt;    // von keiner HTML-Vorlage abgeleitet

    sal_uInt16 nPoolFmtId = rFmt.GetPoolFmtId();

    // Den auszugebenden Attr-Set bestimmen. Hier muessen 3 Faelle
    // unterschieden werden:
    // - HTML-Tag-Vorlagen (nDeep==USHRT_MAX):
    //   Es werden die Attrs ausgegeben
    //     - die in der Vorlage gesetzt sind, aber nicht im Original aus
    //       der HTML-Vorlage
    //     - die Default-Attrs fuer die Attrs, die im Original aus der
    //       HTML-Vorlage gesetzt sind, aber nicht in der vorliegeden Vorlage.
    // - direkt von HTML-Vorlagen abgeleitete Vorlagen (nDeep==1):
    //   Es weren nur die Attribute des Vorlagen-Item-Set ohne seine
    //   Parents ausgegeben.
    // - indirekt von HTML-Tag-Vorlagen abgeleitete Vorlagen (nDeep>1)
    //   Es werden die Attribute des Vorlagen-Item-Sets inkl. seiner Parents,
    //   aber ohne die Attribute, die in der HTML-Tag-Vorlage gesetzt sind,
    //   ausgegeben.

    // einen Item-Set mit allen Attributen aus der Vorlage anlegen
    // (ausser fuer nDeep==1)
    const SfxItemSet& rFmtItemSet = rFmt.GetAttrSet();
    SfxItemSet aItemSet( *rFmtItemSet.GetPool(), rFmtItemSet.GetRanges() );
    aItemSet.Set( rFmtItemSet, sal_True ); // Was nDeep!=1 that is not working
                                       // for script dependent items buts should
                                       // not make a deifference for any other

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
        // Den Item-Set der Referenz-Vorlage (inkl. seiner Parents) vom
        // ItemSet abziehen
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
        // die Default-Abstaende nach oben und unten setzen (fuer den
        // Fall, dass es keine Vorlage als Referenz gibt)
        rHTMLWrt.nDfltTopMargin = 0;
        rHTMLWrt.nDfltBottomMargin = HTML_PARSPACE;
        if( USER_FMT & nPoolFmtId )
        {
            // Benutzer-Vorlagen
            const OUString& rNm = rFmt.GetName();
            switch( rNm[0] )
            {
            case 'D':   if( rNm == "DD 1" || rNm == "DT 1" )
                            rHTMLWrt.nDfltBottomMargin = 0;
                        break;
            case 'L':   if(rNm == OOO_STRING_SVTOOLS_HTML_listing )
                            rHTMLWrt.nDfltBottomMargin = 0;
                        break;
            case 'P':   if( rNm == OOO_STRING_SVTOOLS_HTML_preformtxt )
                            rHTMLWrt.nDfltBottomMargin = 0;
                        break;
            case 'X':   if( rNm == OOO_STRING_SVTOOLS_HTML_xmp )
                            rHTMLWrt.nDfltBottomMargin = 0;
                        break;
            }
        }
        else
        {
            // Pool-Vorlagen
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

    // wo nicht auszugeben ist ...
    if( !aItemSet.Count() )
        return rWrt;

    // There is no support for script dependent hyperlinks by now.
    bool bCheckForPseudo = false;
    if( bCharFmt &&
        (RES_POOLCHR_INET_NORMAL==nRefPoolId ||
         RES_POOLCHR_INET_VISIT==nRefPoolId) )
        bCheckForPseudo = true;


    // jetzt die Attribute (inkl. Selektor) ausgeben
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

    // Drop-Caps ausgeben
    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET==aItemSet.GetItemState( RES_PARATR_DROP, sal_False, &pItem ))
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

    // Die Groesse: Wenn sie sich nur durch das Landscape-Flag unterscheidet,
    // wird nur Portrait oder Landscape exportiert. Sonst wird die Groesse
    // exportiert.
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

    // Boeser uebler Hack: Auf der Seiten-Tabpage gibt es leichte
    // Rundungsfehler bei der Seitengroesse. Unter anderem wegen bug
    // 25535 wird dummerweise auch noch immer Size-Item vom Dialog geputtet,
    // auch wenn man gar nichts geaendert hat. Folge: Sobald man einmal im
    // Seiten-Dialog war und ihn mit OK verlassen hat, bekommt man eine
    // neue Seitengroesse, die dann hier exportiert wuerde. Um das
    // vermeiden erlauben wir hier kleine Abweichungen.
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

    // Die Abstand-Attribute koennen auf gwohnte Weise exportiert werden
    const SwFrmFmt &rMaster = rPageDesc.GetMaster();
    SfxItemSet aItemSet( *rMaster.GetAttrSet().GetPool(),
                         RES_LR_SPACE, RES_UL_SPACE );
    aItemSet.Set( rMaster.GetAttrSet(), sal_True );

    if( pRefPageDesc )
    {
        SwHTMLWriter::SubtractItemSet( aItemSet,
                                       pRefPageDesc->GetMaster().GetAttrSet(),
                                       sal_True );
    }

    OutCSS1_SvxULSpace_SvxLRSpace( rWrt, aItemSet, sal_False );

    // Wenn fuer einen Pseudo-Selektor keine Property ausgegeben wurde, muessen
    // wir trotzdem etwas ausgeben, damit beim Import die entsprechende
    // Vorlage angelegt wird.
    if( rHTMLWrt.bFirstCSS1Property && bPseudo )
    {
        rHTMLWrt.OutNewLine();
        OString sTmp(OUStringToOString(aSelector, rHTMLWrt.eDestEnc));
        rWrt.Strm() << sTmp.getStr() << " {";
        rHTMLWrt.bFirstCSS1Property = sal_False;
    }

    if( !rHTMLWrt.bFirstCSS1Property )
        rWrt.Strm() << sCSS1_rule_end;

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
        rHTMLWrt.Strm() << sCSS1_rule_end;
    }

    const SwCharFmt *pSymCharFmt = rInfo.GetCharFmt( *pDoc );
    if( pSymCharFmt )
    {
        const SfxItemSet& rFmtItemSet = pSymCharFmt->GetAttrSet();
        SfxItemSet aItemSet( *rFmtItemSet.GetPool(), rFmtItemSet.GetRanges() );
        aItemSet.Set( rFmtItemSet, sal_True );

        // Wenn es Fuss- bzw. Endnoten gibt, dann muessen alles Attribute
        // ausgegeben werden, damit Netscape das Dokument richtig anzeigt.
        // Anderenfalls genuegt es, die Unterschiede zur Fuss-/Endnoten
        // Vorlage rauszuschreiben.
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


    // Es werden nur die Attribute der Seiten-Vorlage ausgegeben.
    // Die Attribute der Standard-Absatz-Vorlage werden schon beim
    // Export der Absatz-Vorlagen beruecksichtigt.

    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, sal_False,
                                               &pItem ) )
    {
        OutCSS1_SvxBrush( rWrt, *pItem, CSS1_BACKGROUND_PAGE );
    }

    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BOX, sal_False,
                                               &pItem ))
    {
        OutCSS1_SvxBox( rWrt, *pItem );
    }

    if( !rHTMLWrt.bFirstCSS1Property )
    {
        // wenn eine Property als Bestandteil einer Style-Option
        // ausgegeben wurde, muss die Optiomn noch beendet werden
        rWrt.Strm() << '\"';
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
        rWrt.Strm() << sCSS1_span_tag_end;

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
        rWrt.Strm() << '\"';

    return rWrt;
}

// Wrapper fuer die Ausgabe von Tabellen-Hintergruenden
Writer& OutCSS1_TableBGStyleOpt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_STYLE_OPT_ON |
                                   CSS1_OUTMODE_ENCODE|
                                   CSS1_OUTMODE_TABLEBOX );
    OutCSS1_SvxBrush( rWrt, rHt, CSS1_BACKGROUND_TABLE );

    if( !rHTMLWrt.bFirstCSS1Property )
        rWrt.Strm() << '\"';

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
        rWrt.Strm() << '\"';

    return rWrt;
}

//-----------------------------------------------------------------------

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

                // Fuer top/left muessen die Abstaende des Rahmens von
                // der Position abgezogen werden, da sie in CSS1 noch
                // zur Position addiert werden.
                // Das funktioniert auch fuer automatisch ausgerichtete
                // Rahmen, obwohl der Abstand da ja auch im Writer noch
                // addiert wird. Denn auch in diesem Fall enthalten
                // die Orient-Attribute die korrekte Position

                // top
                long nXPos=0, nYPos=0;
                bool bOutXPos = false, bOutYPos = false;
                if( RES_DRAWFRMFMT == rFrmFmt.Which() )
                {
                    OSL_ENSURE( pSdrObj, "Kein SdrObject uebergeben. Ineffizient" );
                    if( !pSdrObj )
                        pSdrObj = rFrmFmt.FindSdrObject();
                    OSL_ENSURE( pSdrObj, "Wo ist das SdrObject" );
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
            OSL_ENSURE( pSdrObj, "Kein SdrObject uebergeben. Ineffizient" );
            if( !pSdrObj )
                pSdrObj = rFrmFmt.FindSdrObject();
            OSL_ENSURE( pSdrObj, "Wo ist das SdrObject" );
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
                    "Absolute Groesse wird exportiert" );
            OSL_ENSURE( HTML_FRMOPT_ANYSIZE & nFrmOpts,
                    "Jede Groesse wird exportiert" );
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
        if( SFX_ITEM_SET == rItemSet.GetItemState( RES_LR_SPACE, sal_True ) )
            pLRItem = &aLRItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( RES_UL_SPACE, sal_True ) )
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
        else if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BOX, sal_True, &pItem ) )
            OutCSS1_SvxBox( *this, *pItem );
    }

    // background (wenn, dann muss auch eine Farbe ausgegeben werden)
    if( nFrmOpts & HTML_FRMOPT_S_BACKGROUND )
        OutCSS1_FrmFmtBackground( rFrmFmt );

    if( pItemSet )
        OutCSS1_SfxItemSet( *pItemSet, sal_False );

    if( !bFirstCSS1Property )
        Strm() << '\"';
}

void SwHTMLWriter::OutCSS1_TableFrmFmtOptions( const SwFrmFmt& rFrmFmt )
{
    SwCSS1OutMode aMode( *this, CSS1_OUTMODE_STYLE_OPT_ON |
                                CSS1_OUTMODE_ENCODE|
                                CSS1_OUTMODE_TABLE );

    const SfxPoolItem *pItem;
    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();
    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BACKGROUND, sal_False, &pItem ) )
        OutCSS1_SvxBrush( *this, *pItem, CSS1_BACKGROUND_TABLE );

    if( IsHTMLMode( HTMLMODE_PRINT_EXT ) )
        OutCSS1_SvxFmtBreak_SwFmtPDesc_SvxFmtKeep( *this, rItemSet, sal_False );

    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_LAYOUT_SPLIT, sal_False, &pItem ) )
        OutCSS1_SwFmtLayoutSplit( *this, *pItem );

    if( !bFirstCSS1Property )
        Strm() << '\"';
}

void SwHTMLWriter::OutCSS1_TableCellBorderHack(SwFrmFmt const& rFrmFmt)
{
    SwCSS1OutMode const aMode( *this,
        CSS1_OUTMODE_STYLE_OPT_ON|CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_TABLEBOX );
    OutCSS1_SvxBox(*this, rFrmFmt.GetBox());
    if (!bFirstCSS1Property)
    {
        this->Strm() << cCSS1_style_opt_end;
    }
}

void SwHTMLWriter::OutCSS1_SectionFmtOptions( const SwFrmFmt& rFrmFmt, const SwFmtCol *pCol )
{
    SwCSS1OutMode aMode( *this, CSS1_OUTMODE_STYLE_OPT_ON |
                                CSS1_OUTMODE_ENCODE|
                                CSS1_OUTMODE_SECTION );

    const SfxPoolItem *pItem;
    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();
    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BACKGROUND, sal_False, &pItem ) )
        OutCSS1_SvxBrush( *this, *pItem, CSS1_BACKGROUND_SECTION );

    if (pCol)
    {
        OString sColumnCount(OString::number(static_cast<sal_Int32>(pCol->GetNumCols())));
        OutCSS1_PropertyAscii(sCSS1_P_column_count, sColumnCount);
    }

    if( !bFirstCSS1Property )
        Strm() << '\"';
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
    // Wenn der Rahmen selbst einen Hintergrund hat, wird der ausgegeben.
    if( OutCSS1_FrmFmtBrush( *this, rFrmFmt.GetBackground() ) )
        return;

    // Wenn der Rahmen nicht seitengebunden ist, wird sonst muss der
    // Hintergrund vom Anker betrachtet
    const SwFmtAnchor& rAnchor = rFrmFmt.GetAnchor();
    RndStdIds eAnchorId = rAnchor.GetAnchorId();
    const SwPosition *pAnchorPos = rAnchor.GetCntntAnchor();
    if (FLY_AT_PAGE != eAnchorId && pAnchorPos)
    {
        const SwNode& rNode = pAnchorPos->nNode.GetNode();
        if( rNode.IsCntntNode() )
        {
            // Wenn der Rahmen in einem Content-Node verankert ist,
            // wird der Hintergrund von Content-Node ausgegeben, wenn
            // der einen hat.
            if( OutCSS1_FrmFmtBrush( *this,
                    rNode.GetCntntNode()->GetSwAttrSet().GetBackground()) )
                return;

            // Sonst koennen wir evtl. auch in einer Tabelle stehen
            const SwTableNode *pTableNd = rNode.FindTableNode();
            if( pTableNd )
            {
                const SwStartNode *pBoxSttNd = rNode.FindTableBoxStartNode();
                const SwTableBox *pBox =
                    pTableNd->GetTable().GetTblBox( pBoxSttNd->GetIndex() );

                // Wenn die Box einen Hintergrund hat, nehmen wir den.
                if( OutCSS1_FrmFmtBrush( *this,
                        pBox->GetFrmFmt()->GetBackground() ) )
                    return;

                // Sonst betrachten wir den der Lines
                const SwTableLine *pLine = pBox->GetUpper();
                while( pLine )
                {
                    if( OutCSS1_FrmFmtBrush( *this,
                            pLine->GetFrmFmt()->GetBackground() ) )
                        return;
                    pBox = pLine->GetUpper();
                    pLine = pBox ? pBox->GetUpper() : 0;
                }

                // Wenn da auch nichts war den der Tabelle.
                if( OutCSS1_FrmFmtBrush( *this,
                        pTableNd->GetTable().GetFrmFmt()->GetBackground() ) )
                    return;
            }

        }

        // Wenn der Anker wieder in einem Fly-Frame steht, dann
        // wird der Hintergrund des Fly-Frames ausgegeben.
        const SwFrmFmt *pFrmFmt = rNode.GetFlyFmt();
        if( pFrmFmt )
        {
            OutCSS1_FrmFmtBackground( *pFrmFmt );
            return;
        }
    }

    // Schliesslich bleibt noch der Hintergrund der Seite uebrig und als
    // letzte Rettung das Item der Config.
    OSL_ENSURE( pCurrPageDesc, "Keine Seiten-Vorlage gemerkt" );
    if( !OutCSS1_FrmFmtBrush( *this,
                              pCurrPageDesc->GetMaster().GetBackground() ) )
    {
        Color aColor( COL_WHITE );

        // Die Hintergrund-Farbe wird normalerweise nur in Browse-Mode
        // benutzt. Wir benutzen si bei einem HTML-Dokument immer und
        // bei einem Text-Dokument nur, wenn es im Browse-Mode angezeigt
        // wird.
        if( pDoc->get(IDocumentSettingAccess::HTML_MODE) ||
            pDoc->get(IDocumentSettingAccess::BROWSE_MODE))
        {
            ViewShell *pVSh = 0;
            pDoc->GetEditShell( &pVSh );
            if ( pVSh &&
                 COL_TRANSPARENT != pVSh->GetViewOptions()->GetRetoucheColor().GetColor())
                aColor = pVSh->GetViewOptions()->GetRetoucheColor().GetColor();
        }

        OutCSS1_PropertyAscii(sCSS1_P_background, GetCSS1Color(aColor));
    }
}

//-----------------------------------------------------------------------

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
                // das geht auch in HTML und muss nicht als STYLE-Option
                // und darf nicht als Hint geschrieben werden
                OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                        "Underline als Hint schreiben?" );
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
                // das geht auch in HTML und muss nicht als STYLE-Option
                // und darf nicht als Hint geschrieben werden
                OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                        "Overline als Hint schreiben?" );
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
                // das geht auch in HTML und muss nicht als STYLE-Option
                // und darf nicht als Hint geschrieben werden
                OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                        "CrossedOut als Hint schreiben?" );
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
            // das geht auch in HTML und muss nicht als STYLE-Option
            // und darf nicht als Hint geschrieben werden
            OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                    "Blink als Hint schreiben?" );
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

    // Farben muessen nicht in der Style-Option ausgegeben werden.
    if( rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) &&
        !rHTMLWrt.bCfgPreferStyles )
        return rWrt;
    OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
            "Farbe wirklich als Hint ausgeben?" );

    Color aColor( ((const SvxColorItem&)rHt).GetValue() );
    if( COL_AUTO == aColor.GetColor() )
        aColor.SetColor( COL_BLACK );

    rHTMLWrt.OutCSS1_PropertyAscii(sCSS1_P_color, GetCSS1Color(aColor));

    return rWrt;
}


static Writer& OutCSS1_SvxCrossedOut( Writer& rWrt, const SfxPoolItem& rHt )
{
    // Mit dieser Methode werden nur Hints ausgegeben!
    // Sonst wird OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink() direkt aufgerufen.

    if( ((SwHTMLWriter&)rWrt).IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink( rWrt,
                0, 0, (const SvxCrossedOutItem *)&rHt, 0 );

    return rWrt;
}

static Writer& OutCSS1_SvxFont( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Fonts muessen nicht in der Style-Option ausgegeben werden.
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
            "Font wirklich als Hint ausgeben?" );

    OUString sOut;
    // MS IE3b1 hat mit einfachen Haekchen Probleme
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

    // Font-Hoehen muessen nicht in der Style-Option ausgegeben werden.
    // Fuer Drop-Caps wird ein andewres font-size ausgegeben
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
    if( rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT) )
    {
        // einen Hint nur dann ausgeben wenn es auch was bringt
        sal_uInt16 nSize = rHTMLWrt.GetHTMLFontSize( nHeight );
        if( rHTMLWrt.aFontHeights[nSize-1] == nHeight )
            return rWrt;
    }

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
            // das geht auch in HTML und muss nicht als STYLE-Option
            // und darf nicht als Hint geschrieben werden
            OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                    "Italic als Hint schreiben?" );
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

    // Kerning-Item nur ausgeben, wenn volle Style-Unterst?tzung da ist
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

        // Breite als n.n pt
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

    // Language will be exported rules only
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
            "Language wirklich als Hint ausgeben?" );

    LanguageType eLang = ((const SvxLanguageItem &)rHt).GetLanguage();
    if( LANGUAGE_DONTKNOW == eLang )
        return rWrt;

    OUString sOut = LanguageTag::convertToBcp47( eLang );

    rHTMLWrt.OutCSS1_Property( sCSS1_P_so_language, sOut );

    return rWrt;
}

static Writer& OutCSS1_SvxUnderline( Writer& rWrt, const SfxPoolItem& rHt )
{
    // Mit dieser Methode werden nur Hints ausgegeben!
    // Sonst wird OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink() direkt aufgerufen.

    if( ((SwHTMLWriter&)rWrt).IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink( rWrt,
                (const SvxUnderlineItem *)&rHt, 0, 0, 0 );

    return rWrt;
}


static Writer& OutCSS1_SvxOverline( Writer& rWrt, const SfxPoolItem& rHt )
{
    // Mit dieser Methode werden nur Hints ausgegeben!
    // Sonst wird OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink() direkt aufgerufen.

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
            // das geht auch in HTML und muss nicht als STYLE-Option
            // und darf nicht als Hint geschrieben werden
            OSL_ENSURE( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                    "Fett als Hint schreiben?" );
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
    // Mit dieser Methode werden nur Hints ausgegeben!
    // Sonst wird OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink() direkt aufgerufen.

    if( ((SwHTMLWriter&)rWrt).IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxTxtLn_SvxCrOut_SvxBlink( rWrt,
                0, 0, 0, (const SvxBlinkItem *)&rHt );

    return rWrt;
}

static Writer& OutCSS1_SvxLineSpacing( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Netscape4 hat massive Probleme mit den Zellenhoehen
    // wenn der Zeilenabstand innerhalb einer Tabelle geaendert wird
    // und die Breite der Tabelle nicht automatisch berechnet wird
    // (also wenn eine WIDTH-Option vorhanden ist).
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
        OString sHeight(OString(nPrcHeight) + "%");
        rHTMLWrt.OutCSS1_PropertyAscii(sCSS1_P_line_height, sHeight);
    }

    return rWrt;
}

static Writer& OutCSS1_SvxAdjust( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Alignment in Style-Option nur ausgeben, wenn das Tag kein
    // ALIGN=xxx zulaesst
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

    OString aStr(OString::valueOf(static_cast<sal_Int32>(
        ((const SvxWidowsItem&)rHt).GetValue())));
    rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_widows, aStr );

    return rWrt;
}

static Writer& OutCSS1_SvxOrphans( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    OString aStr(OString::valueOf(static_cast<sal_Int32>(
       ((const SvxOrphansItem&)rHt).GetValue())));
    rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_orphans, aStr );

    return rWrt;
}

static void OutCSS1_SwFmtDropAttrs( SwHTMLWriter& rHWrt,
                                    const SwFmtDrop& rDrop,
                                     const SfxItemSet *pCharFmtItemSet )
{
    // Text fliesst rechts drumrum
    rHWrt.OutCSS1_PropertyAscii( sCSS1_P_float, sCSS1_PV_left );

    // Anzahl der Zeilen -> %-Angabe fuer Font-Hoehe!
    OString sOut(OString::number(rDrop.GetLines()*100) + "%");
    rHWrt.OutCSS1_PropertyAscii(sCSS1_P_font_size, sOut);

    // Abstand zum Text = rechter Rand
    sal_uInt16 nDistance = rDrop.GetDistance();
    if( nDistance > 0 )
        rHWrt.OutCSS1_UnitProperty( sCSS1_P_margin_right, nDistance );

    const SwCharFmt *pDCCharFmt = rDrop.GetCharFmt();
    if( pCharFmtItemSet )
        rHWrt.OutCSS1_SfxItemSet( *pCharFmtItemSet );
    else if( pDCCharFmt )
        rHWrt.OutCSS1_SfxItemSet( pDCCharFmt->GetAttrSet() );
    else if( (rHWrt.nCSS1OutMode & CSS1_OUTMODE_ANY_OFF) == CSS1_OUTMODE_RULE_OFF )
        rHWrt.Strm() << sCSS1_rule_end;

}

static Writer& OutCSS1_SwFmtDrop( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    // nie als Option eines Absatzes ausgeben, sondern nur als Hints
    if( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT) )
        return rWrt;

    if( rHTMLWrt.bTagOn )
    {
        SwCSS1OutMode aMode( rHTMLWrt,
                             rHTMLWrt.nCSS1Script|CSS1_OUTMODE_SPAN_TAG1_ON|CSS1_OUTMODE_ENCODE|
                             CSS1_OUTMODE_DROPCAP );

        OutCSS1_SwFmtDropAttrs( rHTMLWrt, (const SwFmtDrop&)rHt );
        // Ein "> wird schon vom aufrufenden OutCSS1_HintAsSpanTag geschrieben.
    }
    else
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_span, sal_False );
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

    // Der Export der harten Attributierung ist unnoetig, wenn die
    // neuen Werte denen der aktuellen Vorlage entsprechen

    // Einen linken Rand kann es durch eine Liste bereits in der
    // Umgebung geben
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

    // Der Erstzeilen-Einzug kann den Platz fuer eine Numerierung
    // enthalten
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

// Wrapper fuer OutCSS1_SfxItemSet etc.
static Writer& OutCSS1_SvxBrush( Writer& rWrt, const SfxPoolItem& rHt )
{
    OutCSS1_SvxBrush( rWrt, rHt, CSS1_BACKGROUND_ATTR );
    return rWrt;
}

static Writer& OutCSS1_SvxBrush( Writer& rWrt, const SfxPoolItem& rHt,
                                 sal_uInt16 nMode)
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Das Zeichen-Attribut wird nicht ausgegeben, wenn gerade
    // Optionen ausgegeben werden
    if( rHt.Which() < RES_CHRATR_END &&
        rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        return rWrt;

    // Erstmal ein par Werte holen
//  const Brush &rBrush = ((const SvxBrushItem &)rHt).GetBrush();
    const Color & rColor = ((const SvxBrushItem &)rHt).GetColor();
    SvxGraphicPosition ePos = ((const SvxBrushItem &)rHt).GetGraphicPos();

    // Erstmal die Farbe holen
    bool bColor = false;
    /// set <bTransparent> to sal_True, if color is "no fill"/"auto fill"
    bool bTransparent = (rColor.GetColor() == COL_TRANSPARENT);
    Color aColor;
    if( !bTransparent )
    {
        aColor = rColor;
        bColor = true;
    }

    // und jetzt eine Grafik
    OUString aGraphicInBase64;

    // embeddete Grafik -> WriteEmbedded schreiben
    const Graphic* pGrf = ((const SvxBrushItem &)rHt).GetGraphic();
    if( pGrf )
    {
        sal_uLong nErr = XOutBitmap::GraphicToBase64(*pGrf, aGraphicInBase64);
        if( nErr )
        {
            rHTMLWrt.nWarn = WARN_SWG_POOR_LOAD | WARN_SW_WRITE_BASE;
        }
    }

    // In Tabellen wird nur dann etwas exportiert, wenn eine Grafik
    // existiert.
    if( CSS1_BACKGROUND_TABLE==nMode && !pGrf )
        return rWrt;

    // ggf. noch die Ausrichtung der Grafik
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

    // jetzt den String zusammen bauen
    OUString sOut;
    if( !pGrf && !bColor )
    {
        // keine Farbe und kein Link, aber ein transparenter Brush
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
        // Wenn die Breite kleiner ist als ein Pixel, dann als 1px
        // ausgeben, damit Netscape und IE die Linie auch darstellen.
        sOut.append("1px");
    }
    else
    {
        nWidth *= 5;    // 1/100pt

        // Breite als n.nn pt
        sOut.append(OString::number(nWidth / 100) + "." + OString::number((nWidth/10) % 10) +
                    OString::number(nWidth % 10) + OString(sCSS1_UNIT_pt));
    }

    // Linien-Stil: solid oder double
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

    // und noch die Farbe
    sOut.append(GetCSS1Color(pLine->GetColor()));

    rHTMLWrt.OutCSS1_PropertyAscii(pProperty, sOut.makeStringAndClear());
}

Writer& OutCSS1_SvxBox( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SvxBoxItem& rBoxItem = (const SvxBoxItem&)rHt;
    const SvxBorderLine *pTop = rBoxItem.GetTop();
    const SvxBorderLine *pBottom = rBoxItem.GetBottom();
    const SvxBorderLine *pLeft = rBoxItem.GetLeft();
    const SvxBorderLine *pRight = rBoxItem.GetRight();

    if( (pTop && pBottom && pLeft && pRight &&
         *pTop == *pBottom && *pTop == *pLeft && *pTop == *pRight) ||
         (!pTop && !pBottom && !pLeft && !pRight) )
    {
        // alle Linien gesetzt und gleich oder alle Linien nicht gesetzt
        // => border : ...
        OutCSS1_SvxBorderLine( rHTMLWrt, sCSS1_P_border, pTop );
    }
    else
    {
        // sonst alle Linien individuell ausgeben
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
    sal_Char *pStr = 0;
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
 * lege hier die Tabellen fuer die HTML-Funktions-Pointer auf
 * die Ausgabe-Funktionen an.
 * Es sind lokale Strukturen, die nur innerhalb der HTML-DLL
 * bekannt sein muessen.
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
/* RES_CHRATR_DUMMY1 */             0,

/* RES_TXTATR_REFMARK */            0,
/* RES_TXTATR_TOXMARK */            0,
/* RES_TXTATR_META */               0,
/* RES_TXTATR_METAFIELD */          0,
/* RES_TXTATR_AUTOFMT */            0,
/* RES_TXTATR_INETFMT */            0,
/* RES_TXTATR_CHARFMT */            0,
/* RES_TXTATR_CJK_RUBY */           0,
/* RES_TXTATR_UNKNOWN_CONTAINER */  0,
/* RES_TXTATR_DUMMY5 */             0,

/* RES_TXTATR_FIELD */              0,
/* RES_TXTATR_FLYCNT */             0,
/* RES_TXTATR_FTN */                0,
/* RES_TXTATR_DUMMY4 */             0,
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
/* RES_PARATR_NUMRULE */            0, // Dummy:
/* RES_PARATR_SCRIPTSPACE */        0, // Dummy:
/* RES_PARATR_HANGINGPUNCTUATION */ 0, // Dummy:
/* RES_PARATR_FORBIDDEN_RULES */    0, // new
/* RES_PARATR_VERTALIGN */          0, // new
/* RES_PARATR_SNAPTOGRID*/          0, // new
/* RES_PARATR_CONNECT_TO_BORDER */  0, // new
/* RES_PARATR_OUTLINELEVEL */       0, // new since cws outlinelevel
/* RES_PARATR_RSID */               0, // new

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
/* RES_FRMATR_DUMMY9 */             0, // Dummy:
/* RES_FOLLOW_TEXT_FLOW */          0,
/* RES_WRAP_INFLUENCE_ON_OBJPOS */  0,
/* RES_FRMATR_DUMMY2 */             0, // Dummy:
/* RES_AUTO_STYLE */                0, // Dummy:
/* RES_FRMATR_DUMMY4 */             0, // Dummy:
/* RES_FRMATR_DUMMY5 */             0, // Dummy:

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
