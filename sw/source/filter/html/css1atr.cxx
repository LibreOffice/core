/*************************************************************************
 *
 *  $RCSfile: css1atr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-01 19:23:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include "hintids.hxx"

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_BLNKITEM_HXX //autogen
#include <svx/blnkitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <svx/widwitem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_ORPHITEM_HXX //autogen
#include <svx/orphitem.hxx>
#endif
#ifndef _XOUTBMP_HXX //autogen
#include <svx/xoutbmp.hxx>
#endif
#ifndef _HTMLOUT_HXX //autogen
#include <svtools/htmlout.hxx>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _FMTCOL_HXX //autogen
#include <fmtcol.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTTSPLT_HXX //autogen
#include <fmtlsplt.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _VIEWSH_HXX //autogen
#include <viewsh.hxx>
#endif
#ifndef _VIEWOPT_HXX //autogen
#include <viewopt.hxx>
#endif
#ifndef _SWTABLE_HXX //autogen
#include <swtable.hxx>
#endif
// FOOTNOTES
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
// FOOTNOTES

#ifndef _DCONTACT_HXX //autogen
#include <dcontact.hxx>
#endif

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

/*
 * um nicht immer wieder nach einem Update festzustellen, das irgendwelche
 * Hint-Ids dazugekommen sind, wird hier definiert, die Groesse der Tabelle
 * definiert und mit der akt. verglichen. Bei unterschieden wird der
 * Compiler schon meckern.
 *
 * diese Section und die dazugeherigen Tabellen muessen in folgenden Files
 * gepflegt werden: rtf\rtfatr.cxx, sw6\sw6atr.cxx, w4w\w4watr.cxx
 */
#if !defined(UNX) && !defined(MSC) && !defined(PPC) && !defined(CSET) && !defined(__MWERKS__) && !defined(WTC)

#define ATTRFNTAB_SIZE 121
#if ATTRFNTAB_SIZE != POOLATTR_END - POOLATTR_BEGIN
#error Attribut-Tabelle ist ungueltigt. Wurden neue Hint-IDs zugefuegt ??
#endif

#endif

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

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_rule_end, " }" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_span_tag_end, "\">" );
const sal_Char cCSS1_style_opt_end = '\"';

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sHTML_FTN_fontheight, "57%" );

extern SwAttrFnTab aCSS1AttrFnTab;

static Writer& OutCSS1_SwFmt( Writer& rWrt, const SwFmt& rFmt,
                              SwDoc *pDoc, SwDoc *pTemplate );
static Writer& OutCSS1_SwPageDesc( Writer& rWrt, const SwPageDesc& rFmt,
                                   SwDoc *pDoc, SwDoc *pTemplate,
                                   USHORT nRefPoolId, BOOL bExtRef,
                                   BOOL bPseudo=TRUE );
static Writer& OutCSS1_SwFtnInfo( Writer& rWrt, const SwEndNoteInfo& rInfo,
                                  SwDoc *pDoc, USHORT nNotes, BOOL bEndNote );
static void OutCSS1_SwFmtDropAttrs( SwHTMLWriter& rHWrt,
                                    const SwFmtDrop& rDrop );
static Writer& OutCSS1_SvxUnderl_SvxCrOut_SvxBlink( Writer& rWrt,
                    const SvxUnderlineItem *pUItem,
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
                                        BOOL bDeep );
static Writer& OutCSS1_SvxBrush( Writer& rWrt, const SfxPoolItem& rHt,
                                 USHORT nMode, const String *pGrfName );
static Writer& OutCSS1_SvxBrush( Writer& rWrt, const SfxPoolItem& rHt );
static Writer& OutCSS1_SvxBox( Writer& rWrt, const SfxPoolItem& rHt );
static Writer& OutCSS1_SwFmtFrmSize( Writer& rWrt, const SfxPoolItem& rHt,
                                     USHORT nMode );
static Writer& OutCSS1_SvxFmtBreak_SwFmtPDesc_SvxFmtKeep( Writer& rWrt,
                                        const SfxItemSet& rItemSet,
                                        BOOL bDeep );
static Writer& OutCSS1_SwFmtLayoutSplit( Writer& rWrt, const SfxPoolItem& rHt );

static void ConvToHex( USHORT nHex, ByteString& rStr )
{
    sal_Char aNToABuf[] = "00";

    // Pointer an das Bufferende setzen
    sal_Char *pStr = aNToABuf + (sizeof(aNToABuf)-1);
    for( BYTE n = 0; n < 2; ++n )
    {
        *(--pStr) = (sal_Char)(nHex & 0xf ) + 48;
        if( *pStr > '9' )
            *pStr += 39;
        nHex >>= 4;
    }

    rStr.Append( aNToABuf );
}

static void GetCSS1Color( const Color& rColor, ByteString& rStr )
{
    rStr += '#';

    ConvToHex( rColor.GetRed(), rStr );
    ConvToHex( rColor.GetGreen(), rStr );
    ConvToHex( rColor.GetBlue(), rStr );
}

class SwCSS1OutMode
{
    SwHTMLWriter& rWrt;
    USHORT nOldMode;

public:

    SwCSS1OutMode( SwHTMLWriter& rHWrt, USHORT nMode, BOOL bStartFirst=TRUE,
                   const String *pSelector=0 ) :
        rWrt( rHWrt ),
        nOldMode( rHWrt.nCSS1OutMode )
    {
        rWrt.nCSS1OutMode = nMode;
        if( bStartFirst )
            rWrt.bFirstCSS1Property = TRUE;
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
                                     const String *pSVal )
{
    ByteString sOut;

    if( bFirstCSS1Rule && (nCSS1OutMode & CSS1_OUTMODE_RULE_ON)!=0 )
    {
        bFirstCSS1Rule = FALSE;
        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_style );
        OutNewLine();
        Strm() << '<' << sHTML_comment;

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
                ((((sOut += '<') += sHTML_span) += ' ') += sHTML_O_style) += "=\"";
            }
            else
            {
                HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_span, FALSE );
                return;
            }
            break;

        case CSS1_OUTMODE_RULE_ON:
            {
                ByteString sTmp( aCSS1Selector, eDestEnc );
                OutNewLine();
                (sOut = sTmp) += " { ";
            }
            break;

        case CSS1_OUTMODE_STYLE_OPT_ON:
            ((sOut = ' ') += sHTML_O_style) += "=\"";
            break;
        }
        bFirstCSS1Property = FALSE;
    }
    else
    {
        sOut += "; ";
    }


    (sOut += pProp) += ": ";
    if( nCSS1OutMode & CSS1_OUTMODE_ENCODE )
    {
        // In STYLE-Optionen den String codieren
        Strm() << sOut.GetBuffer();
        sOut.Erase();
        if( pVal )
            HTMLOutFuncs::Out_String( Strm(), String::CreateFromAscii(pVal),
                                      eDestEnc );
        else if( pSVal )
            HTMLOutFuncs::Out_String( Strm(), *pSVal, eDestEnc );
    }
    else
    {
        // Im STYLE-Tag des String direct ausgeben
        if( pVal )
            sOut += pVal;
        else if( pVal )
        {
            ByteString sTmp( *pSVal, eDestEnc );
            sOut += sTmp;
        }
    }

    if( sOut.Len() )
        Strm() << sOut.GetBuffer();
}

static void AddUnitPropertyValue( long nVal, FieldUnit eUnit, ByteString& rOut )
{
    if( nVal < 0 )
    {
        // Vorzeichen extra behandeln
        nVal = -nVal;
        rOut += '-';
    }

    // Die umgerechnete Einheit ergibt sich aus (x * nMul)/(nDiv*nFac*10)
    long nMul = 1000;
    long nDiv = 1;
    long nFac = 100;
    const sal_Char *pUnit;
    switch( eUnit )
    {
    case FUNIT_100TH_MM:
        ASSERT( FUNIT_MM == eUnit, "Masseinheit wird nicht unterstuetzt" );
    case FUNIT_MM:
        // 0.01mm = 0.57twip
        nMul = 25400;   // 25.4 * 1000
        nDiv = 1440;    // 72 * 20;
        nFac = 100;
        pUnit = sCSS1_UNIT_mm;
        break;

    case FUNIT_M:
    case FUNIT_KM:
        ASSERT( FUNIT_CM == eUnit, "Masseinheit wird nicht unterstuetzt" );
    case FUNIT_CM:
#ifdef EXACT_VALUES
        // 0.001cm = 0.57twip
        nMul = 25400;   // 2.54 * 10000
        nDiv = 1440;    // 72 * 20;
        nFac = 1000;
#else
        // 0.01cm = 5.7twip (ist zwar ungenau, aber die UI ist auch ungenau)
        nMul = 2540;    // 2.54 * 1000
        nDiv = 1440;    // 72 * 20;
        nFac = 100;
#endif
        pUnit = sCSS1_UNIT_cm;
        break;

    case FUNIT_TWIP:
        ASSERT( FUNIT_POINT == eUnit, "Masseinheit wird nicht unterstuetzt" );
    case FUNIT_POINT:
#ifdef EXACT_VALUES
        // 0.01pt = 0.2twip
        nMul = 1000;
        nDiv = 20;
        nFac = 100;
#else
        // 0.1pt = 2.0twip (ist zwar ungenau, aber die UI ist auch ungenau)
        nMul = 100;
        nDiv = 20;
        nFac = 10;
#endif
        pUnit = sCSS1_UNIT_pt;
        break;

    case FUNIT_PICA:
#ifdef EXACT_VALUES
        // 0.001pc = 0.24twip
        nMul = 10000;
        nDiv = 12 * 20;
        nFac = 1000;
#else
        // 0.01pc = 2.40twip (ist zwar ungenau, aber die UI ist auch ungenau)
        nMul = 1000;
        nDiv = 240;     // 12 * 20;
        nFac = 100;
#endif
        pUnit = sCSS1_UNIT_pc;
        break;

    case FUNIT_NONE:
    case FUNIT_FOOT:
    case FUNIT_MILE:
    case FUNIT_CUSTOM:
    case FUNIT_PERCENT:
    case FUNIT_INCH:
    default:
        ASSERT( FUNIT_INCH == eUnit, "Masseinheit wird nicht unterstuetzt" );
#ifdef EXACT_VALUES
        // 0.0001in = 0.144twip
        nMul = 100000;
        nDiv = 1440;    // 72 * 20;
        nFac = 10000;
#else
        // 0.01in = 14.4twip (ist zwar ungenau, aber die UI ist auch ungenau)
        nMul = 1000;
        nDiv = 1440;    // 72 * 20;
        nFac = 100;
#endif
        pUnit = sCSS1_UNIT_inch;
        break;
    }

    long nLongVal;
    BOOL bOutLongVal = TRUE;
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
                rOut += '.';
                while( nFac > 1 && !(nBigVal % nBigFac).IsZero() )
                {
                    nFac /= 10;
                    nBigFac = nFac;
                    rOut += (int)((nBigVal / nBigFac) % nBig10 );
                }
            }
            bOutLongVal = FALSE;
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
            rOut += ByteString::CreateFromInt64( nBigVal / (sal_Int64)nFac );
            if( (nBigVal % (sal_Int64)nFac) != 0 )
            {
                rOut += '.';
                while( nFac > 1 && (nBigVal % (sal_Int64)nFac) != 0 )
                {
                    nFac /= 10;
                    rOut += ByteString::CreateFromInt64(
                                (nBigVal / (sal_Int64)nFac) % (sal_Int64)10 );
                }
            }
            bOutLongVal = FALSE;
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
        rOut += ByteString::CreateFromInt32( nLongVal/nFac );
        if( (nLongVal % nFac) != 0 )
        {
            rOut += '.';
            while( nFac > 1 && (nLongVal % nFac) != 0 )
            {
                nFac /= 10;
                rOut += ByteString::CreateFromInt32( (nLongVal / nFac) % 10 );
            }
        }
    }

    rOut.Append( pUnit );
}

void SwHTMLWriter::OutCSS1_UnitProperty( const sal_Char *pProp, long nVal )
{
    ByteString sOut;
    AddUnitPropertyValue( nVal, eCSS1Unit, sOut );
    OutCSS1_PropertyAscii( pProp, sOut );
}

void SwHTMLWriter::OutCSS1_PixelProperty( const sal_Char *pProp, long nVal,
                                          BOOL bVert )
{
    if( nVal && Application::GetDefaultDevice() )
    {
        Size aSz( bVert ? 0 : nVal, bVert ? nVal : 0 );
        aSz = Application::GetDefaultDevice()->LogicToPixel( aSz, MapMode( MAP_TWIP) );
        nVal = bVert ? aSz.Height() : aSz.Width();
        if( !nVal )
            nVal = 1;
    }

    ByteString sOut( ByteString::CreateFromInt32( nVal ) );
    sOut.Append( sCSS1_UNIT_px );
    OutCSS1_PropertyAscii( pProp, sOut );
}

void SwHTMLWriter::OutCSS1_SfxItemSet( const SfxItemSet& rItemSet,
                                       BOOL bDeep )
{
    // den ItemSet ausgeben, und zwar inklusive aller Attribute
    Out_SfxItemSet( aCSS1AttrFnTab, *this, rItemSet, bDeep );

    // ein par Attribute benoetigen eine Spezial-Behandlung
    const SfxPoolItem *pItem = 0;

    // Underline, CrossedOut und Blink bilden zusammen eine CSS1-Property
    // (geht natuerlich nicht bei Hints)
    if( !IsCSS1Source(CSS1_OUTMODE_HINT) )
    {
        const SvxUnderlineItem *pUnderlineItem = 0;
        if( SFX_ITEM_SET==rItemSet.GetItemState( RES_CHRATR_UNDERLINE, bDeep, &pItem ))
            pUnderlineItem = (const SvxUnderlineItem *)pItem;

        const SvxCrossedOutItem *pCrossedOutItem = 0;
        if( SFX_ITEM_SET==rItemSet.GetItemState( RES_CHRATR_CROSSEDOUT, bDeep, &pItem ))
            pCrossedOutItem = (const SvxCrossedOutItem *)pItem;

        const SvxBlinkItem *pBlinkItem = 0;
        if( SFX_ITEM_SET==rItemSet.GetItemState( RES_CHRATR_BLINK, bDeep, &pItem ))
            pBlinkItem = (const SvxBlinkItem *)pItem;

        if( pUnderlineItem || pCrossedOutItem || pBlinkItem )
            OutCSS1_SvxUnderl_SvxCrOut_SvxBlink( *this, pUnderlineItem,
                                                 pCrossedOutItem,
                                                 pBlinkItem );

        OutCSS1_SvxFmtBreak_SwFmtPDesc_SvxFmtKeep( *this, rItemSet, bDeep );
    }

    if( !bFirstCSS1Property )
    {
        // wenn eine Property als Bestandteil einer Style-Option
        // ausgegeben wurde, muss die Optiomn noch beendet werden
        ByteString sOut;
        switch( nCSS1OutMode & CSS1_OUTMODE_ANY_OFF )
        {
        case CSS1_OUTMODE_SPAN_TAG_OFF:
            sOut = sCSS1_span_tag_end;
            break;

        case CSS1_OUTMODE_STYLE_OPT_OFF:
            sOut = cCSS1_style_opt_end;
            break;

        case CSS1_OUTMODE_RULE_OFF:
            sOut = sCSS1_rule_end;
            break;
        }
        if( sOut.Len() )
            Strm() << sOut.GetBuffer();
    }
}

#if USED
void SwHTMLWriter::OutCSS1_SfxItem( USHORT nMode,
                                    const SfxPoolItem *pItem1,
                                    const SfxPoolItem *pItem2 )
{
    // den ItemSet ausgeben, und zwar inklusive aller Attribute
    nSaveMode = nCSS1OutMode;
    nCSS1OutMode = nMode;
    bFirstCSS1Property = TRUE;

    Out( aCSS1AttrFnTab, *pItem1, *this );
    if( pItem2 )
        Out( aCSS1AttrFnTab, *pItem2, *this );


    if( !bFirstCSS1Property )
    {
        // wenn eine Property als Bestandteil einer Style-Option
        // ausgegeben wurde, muss die Optiomn noch beendet werden
        ByteString sOut;
        switch( nCSS1OutMode & CSS1_OUTMODE_ANY_OFF )
        {
        case CSS1_OUTMODE_SPAN_TAG_OFF:
            sOut = sCSS1_span_tag_end;
            break;

        case CSS1_OUTMODE_STYLE_OPT_OFF:
            sOut = cCSS1_style_opt_end;
            break;

        case CSS1_OUTMODE_RULE:
            sOut = sCSS1_rule_end;
            break;
        }
        if( sOut.Len() )
            rStrm << sOut.GetBuffer();
    }

    nCSS1OutMode = nSaveMode;
}
#endif

void SwHTMLWriter::OutStyleSheet( const SwPageDesc& rPageDesc, BOOL bUsed )
{
    bFirstCSS1Rule = TRUE;

// Feature: PrintExt
    if( IsHTMLMode(HTMLMODE_PRINT_EXT) )
    {
        const SwPageDesc *pFirstPageDesc = 0;
        USHORT nFirstRefPoolId = RES_POOLPAGE_HTML;
        bCSS1IgnoreFirstPageDesc = TRUE;

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
        if( pPageDesc == pFollow )
        {
            // Das Dokument ist einseitig. Egal welche Seite verwendet wird,
            // es wird kein zweiseitiges Dokument daraus gemacht.
            // Die Attributierung wird relativ zur HTML-Seitenvorlage
            // aus der HTML-Vorlage exportiert.
            OutCSS1_SwPageDesc( *this, *pPageDesc, pDoc, pTemplate,
                                RES_POOLPAGE_HTML, TRUE, FALSE );
            nFirstRefPoolId = pFollow->GetPoolFmtId();
        }
        else if( (RES_POOLPAGE_LEFT == pPageDesc->GetPoolFmtId() &&
                  RES_POOLPAGE_RIGHT == pFollow->GetPoolFmtId()) ||
                 (RES_POOLPAGE_RIGHT == pPageDesc->GetPoolFmtId() &&
                  RES_POOLPAGE_LEFT == pFollow->GetPoolFmtId()) )
        {
            // Das Dokument ist zweiseitig
            OutCSS1_SwPageDesc( *this, *pPageDesc, pDoc, pTemplate,
                                RES_POOLPAGE_HTML, TRUE );
            OutCSS1_SwPageDesc( *this, *pFollow, pDoc, pTemplate,
                                RES_POOLPAGE_HTML, TRUE );
            nFirstRefPoolId = RES_POOLPAGE_RIGHT;
            bCSS1IgnoreFirstPageDesc = FALSE;
        }
        // Alles andere bekommen wir nicht hin.

        if( pFirstPageDesc )
            OutCSS1_SwPageDesc( *this, *pFirstPageDesc, pDoc, pTemplate,
                                nFirstRefPoolId, FALSE );
    }
// /Feature: PrintExt


    // das Default-TextStyle wir nicht mit ausgegeben !!
    // das 0-Style ist das Default, wird nie ausgegeben !!
    USHORT nArrLen = pDoc->GetTxtFmtColls()->Count();
    for( USHORT i=1; i<nArrLen; i++ )
    {
        const SwTxtFmtColl* pColl = (*pDoc->GetTxtFmtColls())[i];
        if( !bUsed || pDoc->IsUsed( *pColl ) )
            OutCSS1_SwFmt( *this, *pColl, pDoc, pTemplate );
    }

    // das Default-TextStyle wir nicht mit ausgegeben !!
    nArrLen = pDoc->GetCharFmts()->Count();
    for( i=1; i<nArrLen; i++ )
    {
        const SwCharFmt *pCFmt = (*pDoc->GetCharFmts())[i];
        USHORT nPoolId = pCFmt->GetPoolFmtId();
        if( !bUsed || nPoolId == RES_POOLCHR_INET_NORMAL ||
            nPoolId == RES_POOLCHR_INET_VISIT ||
            pDoc->IsUsed( *pCFmt ) )
            OutCSS1_SwFmt( *this, *pCFmt, pDoc, pTemplate );
    }

    const SwFtnIdxs& rIdxs = pDoc->GetFtnIdxs();
    nArrLen = rIdxs.Count();
    USHORT nEnd = 0, nFtn = 0;
    for( i=0; i < nArrLen; i++ )
    {
        if( rIdxs[i]->GetFtn().IsEndNote() )
            nEnd++;
        else
            nFtn++;
    }
    OutCSS1_SwFtnInfo( *this, pDoc->GetFtnInfo(), pDoc, nFtn, FALSE );
    OutCSS1_SwFtnInfo( *this, pDoc->GetEndNoteInfo(), pDoc, nEnd, TRUE );

    if( !bFirstCSS1Rule )
    {
        DecIndentLevel();
        OutNewLine();
        Strm() << "-->";

        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_style, FALSE );
    }
    else
    {
        bFirstCSS1Rule = FALSE;
    }

    nDfltTopMargin = 0;
    nDfltBottomMargin = 0;
}

#ifdef OLD_MS_IE

BOOL SwHTMLWriter::IsDfltLRSpace( const SvxLRSpaceItem& rLRItem ) const
{
    // Einen linken Rand kann es durch eine Liste bereits in der
    // Umgebung geben
    USHORT nLeft = rLRItem.GetTxtLeft() - nLeftMargin;

    // Der Erstzeilen-Einzug kann den Platz fuer eine Numerierung
    // enthalten
    short nIndent = rLRItem.GetTxtFirstLineOfst() - nFirstLineIndent;

    // Wenn Spacer exportiert werden, wird ein psositiver Erstzeilen-Einzug
    // als Spacer exportiert.
    return nDfltLeftMargin == nLeft &&
           nDfltRightMargin == rLRItem.GetRight() &&
           ( (bCfgSpacer && nIndent > 0) ||
              nDfltFirstLineIndent == nIndent );
}

BOOL SwHTMLWriter::IsDfltULSpace( const SvxULSpaceItem& rULItem ) const
{
    return nDfltTopMargin == rULItem.GetUpper() &&
           nDfltBottomMargin == rULItem.GetLower();
}

#endif

//-----------------------------------------------------------------------

// wenn pPseudo gesetzt ist werden Styles-Sheets ausgegeben,
// sonst wird nur nach Token und Class fuer ein Format gesucht
USHORT SwHTMLWriter::GetCSS1Selector( const SwFmt *pFmt, ByteString& rToken,
                                      String& rClass, USHORT& rRefPoolId,
                                      String *pPseudo )
{
    USHORT nDeep = 0;
    rToken.Erase(); rClass.Erase();
    rRefPoolId = 0;
    if( pPseudo )
        pPseudo->Erase();

    BOOL bChrFmt = RES_CHRFMT==pFmt->Which();

    // Nach oben die Formate abklappern, bis man auf eine Standard-
    // oder eine HTML-Tag-Vorlage trifft
    const SwFmt *pPFmt = pFmt;
    while( pPFmt && !pPFmt->IsDefault() )
    {
        BOOL bStop = FALSE;
        USHORT nPoolId = pPFmt->GetPoolFmtId();
        if( USER_FMT & nPoolId )
        {
            // Benutzer-Vorlagen
            const String& rNm = pPFmt->GetName();
            switch( rNm.GetChar(0) )
            {
                        // nicht mehr unterstuetzt:
                        // sHTML_author
                        // sHTML_acronym
                        // sHTML_abbreviation
                        // sHTML_deletedtext
                        // sHTML_insertedtext
                        // sHTML_language
                        // sHTML_person
            case 'B':   if( !bChrFmt && rNm.EqualsAscii(sHTML_blockquote) )
                        {
                            rRefPoolId = RES_POOLCOLL_HTML_BLOCKQUOTE;
                            rToken.Assign( sHTML_blockquote );
                        }
                        break;
            case 'C':   if( bChrFmt )
                        {
                            if( rNm.EqualsAscii(sHTML_citiation) )
                            {
                                rRefPoolId = RES_POOLCHR_HTML_CITIATION;
                                rToken.Assign( sHTML_citiation );
                            }
                            else if( rNm.EqualsAscii(sHTML_code) )
                            {
                                rRefPoolId = RES_POOLCHR_HTML_CODE;
                                rToken.Assign( sHTML_code );
                            }
                        }
                        break;
            case 'D':   if( bChrFmt && rNm.EqualsAscii(sHTML_definstance) )
                        {
                            rRefPoolId = RES_POOLCHR_HTML_DEFINSTANCE;
                            rToken.Assign( sHTML_definstance);
                        }
                        else if( !bChrFmt )
                        {
                            USHORT nDefListLvl = GetDefListLvl( rNm, nPoolId );
                            // Die Vorlagen DD 1/DT 1 werden ausgegeben,
                            // aber keine von ihnen abgeleiteten Vorlagen,
                            // auch nicht DD 2/DT 2 etc.
                            if( nDefListLvl )
                            {
                                if( pPseudo &&
                                    (nDeep || (nDefListLvl & 0x0fff) > 1) )
                                {
                                    bStop = TRUE;
                                }
                                else if( nDefListLvl & HTML_DLCOLL_DD )
                                {
                                    rRefPoolId = RES_POOLCOLL_HTML_DD;
                                    rToken.Assign( sHTML_dd );
                                }
                                else
                                {
                                    rRefPoolId = RES_POOLCOLL_HTML_DT;
                                    rToken.Assign( sHTML_dt );
                                }
                            }
                        }
                        break;
            case 'E':   if( bChrFmt && rNm.EqualsAscii( sHTML_emphasis ) )
                        {
                            rRefPoolId = RES_POOLCHR_HTML_EMPHASIS;
                            rToken.Assign( sHTML_emphasis );
                        }
                        break;
            case 'H':   if( !bChrFmt && rNm.EqualsAscii( sHTML_horzrule ) )
                            // HR nicht ausgeben!
                            bStop = (nDeep==0);
                        break;
            case 'K':   if( bChrFmt && rNm.EqualsAscii( sHTML_keyboard ) )
                        {
                            rRefPoolId = RES_POOLCHR_HTML_KEYBOARD;
                            rToken.Assign( sHTML_keyboard );
                        }
                        break;
            case 'L':   if( !bChrFmt && rNm.EqualsAscii( sHTML_listing ) )
                        {
                            // Listing als PRE exportieren bzw. von
                            // PRE abgeleitete Vorlage exportieren
                            rToken.Assign( sHTML_preformtxt );
                            rRefPoolId = RES_POOLCOLL_HTML_PRE;
                            nDeep = CSS1_FMT_CMPREF;
                        }
                        break;
            case 'P':   if( !bChrFmt && rNm.EqualsAscii( sHTML_preformtxt ) )
                        {
                            rRefPoolId = RES_POOLCOLL_HTML_PRE;
                            rToken.Assign( sHTML_preformtxt );
                        }
                        break;
            case 'S':   if( bChrFmt )
                        {
                            if( rNm.EqualsAscii( sHTML_sample ) )
                            {
                                rRefPoolId = RES_POOLCHR_HTML_SAMPLE;
                                rToken.Assign( sHTML_sample );
                            }
                            else if( rNm.EqualsAscii( sHTML_strong ) )
                            {
                                rRefPoolId = RES_POOLCHR_HTML_STRONG;
                                rToken.Assign( sHTML_strong );
                            }
                        }
                        break;
            case 'T':   if( bChrFmt && rNm.EqualsAscii( sHTML_teletype ) )
                        {
                            rRefPoolId = RES_POOLCHR_HTML_TELETYPE;
                            rToken.Assign( sHTML_teletype );
                        }
                        break;
            case 'V':   if( bChrFmt && rNm.EqualsAscii( sHTML_variable ) )
                        {
                            rRefPoolId = RES_POOLCHR_HTML_VARIABLE;
                            rToken.Assign( sHTML_variable );
                        }
                        break;
            case 'X':   if( !bChrFmt && rNm.EqualsAscii( sHTML_xmp ) )
                        {
                            // XMP als PRE exportieren (aber nicht die
                            // Vorlage als Style)
                            rToken.Assign( sHTML_preformtxt );
                            rRefPoolId = RES_POOLCOLL_HTML_PRE;
                            nDeep = CSS1_FMT_CMPREF;
                        }
                        break;
            }

            // Wenn eine PoolId gesetzt ist, entspricht der Name der
            // Vorlage dem szugehoerigen Token
            ASSERT( rRefPoolId != 0 == rToken.Len() > 0,
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
                rToken.Assign( sHTML_parabreak );
                break;
            case RES_POOLCOLL_HEADLINE1:
                rToken.Assign( sHTML_head1 );
                break;
            case RES_POOLCOLL_HEADLINE2:
                rToken.Assign( sHTML_head2 );
                break;
            case RES_POOLCOLL_HEADLINE3:
                rToken.Assign( sHTML_head3 );
                break;
            case RES_POOLCOLL_HEADLINE4:
                rToken.Assign( sHTML_head4 );
                break;
            case RES_POOLCOLL_HEADLINE5:
                rToken.Assign( sHTML_head5 );
                break;
            case RES_POOLCOLL_HEADLINE6:
                rToken.Assign( sHTML_head6 );
                break;
            case RES_POOLCOLL_SENDADRESS:
                rToken.Assign( sHTML_address );
                break;
            case RES_POOLCOLL_HTML_BLOCKQUOTE:
                rToken.Assign( sHTML_blockquote );
                break;
            case RES_POOLCOLL_HTML_PRE:
                rToken.Assign( sHTML_preformtxt );
                break;

            case RES_POOLCOLL_HTML_DD:
                rToken.Assign( sHTML_dd );
                break;
            case RES_POOLCOLL_HTML_DT:
                rToken.Assign( sHTML_dt );
                break;

            case RES_POOLCOLL_TABLE:
                if( pPseudo )
                {
                    rToken.Assign( sHTML_tabledata );
                    rToken.Append( ' ' );
                    rToken.Assign( sHTML_parabreak );
                }
                else
                    rToken.Assign( sHTML_parabreak );
                break;
            case RES_POOLCOLL_TABLE_HDLN:
                if( pPseudo )
                {
                    rToken.Assign( sHTML_tableheader );
                    rToken.Assign( ' ' );
                    rToken.Assign( sHTML_parabreak );
                }
                else
                    rToken.Assign( sHTML_parabreak );
                break;
            case RES_POOLCOLL_HTML_HR:
                // HR nicht ausgeben!
                bStop = (nDeep==0);
                break;
            case RES_POOLCOLL_FOOTNOTE:
                if( !nDeep )
                {
                    rToken.Assign( sHTML_parabreak );
                    rClass.AssignAscii( sHTML_sdfootnote );
                    rRefPoolId = RES_POOLCOLL_TEXT;
                    nDeep = CSS1_FMT_CMPREF;
                }
                break;
            case RES_POOLCOLL_ENDNOTE:
                if( !nDeep )
                {
                    rToken.Assign( sHTML_parabreak );
                    rClass.AssignAscii( sHTML_sdendnote );
                    rRefPoolId = RES_POOLCOLL_TEXT;
                    nDeep = CSS1_FMT_CMPREF;
                }
                break;

            // Zeichen-Vorlagen
            case RES_POOLCHR_HTML_EMPHASIS:
                rToken.Assign( sHTML_emphasis );
                break;
            case RES_POOLCHR_HTML_CITIATION:
                rToken.Assign( sHTML_citiation );
                break;
            case RES_POOLCHR_HTML_STRONG:
                rToken.Assign( sHTML_strong );
                break;
            case RES_POOLCHR_HTML_CODE:
                rToken.Assign( sHTML_code );
                break;
            case RES_POOLCHR_HTML_SAMPLE:
                rToken.Assign( sHTML_sample );
                break;
            case RES_POOLCHR_HTML_KEYBOARD:
                rToken.Assign( sHTML_keyboard );
                break;
            case RES_POOLCHR_HTML_VARIABLE:
                rToken.Assign( sHTML_variable );
                break;
            case RES_POOLCHR_HTML_DEFINSTANCE:
                rToken.Assign( sHTML_definstance );
                break;
            case RES_POOLCHR_HTML_TELETYPE:
                rToken.Assign( sHTML_teletype );
                break;

            case RES_POOLCHR_INET_NORMAL:
                if( pPseudo )
                {
                    rToken.Assign( sHTML_anchor );
                    pPseudo->AssignAscii( sCSS1_link );
                }
                break;
            case RES_POOLCHR_INET_VISIT:
                if( pPseudo )
                {
                    rToken.Assign( sHTML_anchor );
                    pPseudo->AssignAscii( sCSS1_visited );
                }
                break;
            }

            // Wenn ein Token gesetzt ist, enthaelt nPoolId die dazugehoerige
            // Vorlage
            if( rToken.Len() && !rRefPoolId )
                rRefPoolId = nPoolId;
        }

        if( rToken.Len() || bStop )
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

    if( rToken.Len() )
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
        xub_StrLen nPos = rClass.Search( '.' );
        if( nPos != STRING_NOTFOUND && rClass.Len() > nPos+1 )
        {
            rClass.Erase( 0, nPos+1 );
        }

        GetAppCharClass().toLower( rClass );
        while( STRING_NOTFOUND != rClass.SearchAndReplace( '.', '-' ) )
            ;
        while( STRING_NOTFOUND != rClass.SearchAndReplace( ' ', '-' ) )
            ;
        while( STRING_NOTFOUND != rClass.SearchAndReplace( '_', '-' ) )
            ;
    }

    return nDeep;
}

static USHORT GetCSS1Selector( const SwFmt *pFmt, String& rSelector,
                               USHORT& rRefPoolId )
{
    ByteString aToken;
    String aClass;
    String aPseudo;

    USHORT nDeep = SwHTMLWriter::GetCSS1Selector( pFmt, aToken, aClass,
                                                  rRefPoolId, &aPseudo );
    if( nDeep )
    {
        if( aToken.Len() )
            rSelector = String( aToken, RTL_TEXTENCODING_ASCII_US );
        else
            rSelector.Erase();

        if( aClass.Len() )
            (rSelector += '.') += aClass;
        if( aPseudo.Len() )
            (rSelector += ':') += aPseudo;
    }

    return nDeep;
}

const SwFmt *SwHTMLWriter::GetTemplateFmt( USHORT nPoolFmtId,
                                           SwDoc *pTemplate )
{
    const SwFmt *pRefFmt = 0;

    if( pTemplate )
    {
        ASSERT( !(USER_FMT & nPoolFmtId),
                "In der Dok-Vorlage gibt es keine Benutzer-Vorlagen" );
        if( POOLGRP_NOCOLLID & nPoolFmtId )
            pRefFmt = pTemplate->GetCharFmtFromPool( nPoolFmtId );
        else
            pRefFmt = pTemplate->GetTxtCollFromPool( nPoolFmtId );
    }

    return pRefFmt;
}

const SwFmt *SwHTMLWriter::GetParentFmt( const SwFmt& rFmt, USHORT nDeep )
{
    ASSERT( nDeep != USHRT_MAX, "GetParent fuer HTML-Vorlage aufgerufen!" );
    const SwFmt *pRefFmt = 0;

    if( nDeep > 0 )
    {
        // hier wird die HTML-Tag-Vorlage, von der die Vorlage abgeleitet
        // ist als Referenz geholt
        pRefFmt = &rFmt;
        for( USHORT i=nDeep; i>0; i-- )
            pRefFmt = pRefFmt->DerivedFrom();

        if( pRefFmt && pRefFmt->IsDefault() )
            pRefFmt = 0;
    }

    return pRefFmt;
}

BOOL lcl_css1atr_equalFontItems( const SfxPoolItem& r1, const SfxPoolItem& r2 )
{
    return  ((const SvxFontItem &)r1).GetFamilyName() ==
                    ((const SvxFontItem &)r2).GetFamilyName() &&
            ((const SvxFontItem &)r1).GetFamily() ==
                    ((const SvxFontItem &)r2).GetFamily() &&
            ((const SvxFontItem &)r1).GetPitch() ==
                    ((const SvxFontItem &)r2).GetPitch();
}

void SwHTMLWriter::SubtractItemSet( SfxItemSet& rItemSet,
                                    const SfxItemSet& rRefItemSet,
                                    BOOL bSetDefaults,
                                    BOOL bClearSame )
{
    ASSERT( bSetDefaults || bClearSame,
            "SwHTMLWriter::SubtractItemSet: Bei diesen Flags passiert nix" );
    SfxItemSet aRefItemSet( *rRefItemSet.GetPool(), rRefItemSet.GetRanges() );
    aRefItemSet.Set( rRefItemSet );

    // und mit dem Attr-Set der Vorlage vergleichen
    SfxWhichIter aIter( rItemSet );
    USHORT nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        const SfxPoolItem *pRefItem, *pItem;
        BOOL bItemSet = ( SFX_ITEM_SET ==
                rItemSet.GetItemState( nWhich, FALSE, &pItem) );
        BOOL bRefItemSet = ( SFX_ITEM_SET ==
                aRefItemSet.GetItemState( nWhich, FALSE, &pRefItem) );
        if( bItemSet )
        {
            if( bClearSame && bRefItemSet &&
                ( *pItem == *pRefItem ||
                  (RES_CHRATR_FONT == nWhich &&
                   lcl_css1atr_equalFontItems( *pItem, *pRefItem )) ) )
            {
                // das Attribut ist mit dem gleichen Wert in beiden
                // Vorlagen vorhanden und muss nicht ausgegeben werden
                rItemSet.ClearItem( nWhich );
            }
        }
        else
        {
            if( bSetDefaults && bRefItemSet )
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
                                    String& rNames,
                                    sal_Unicode cQuote, BOOL bGeneric )
{
    rNames = aEmptyStr;
    const String& rName = rFontItem.GetFamilyName();
    BOOL bContainsKeyword = FALSE;
    if( rName.Len() )
    {
        xub_StrLen nStrPos = 0;
        while( nStrPos != STRING_NOTFOUND )
        {
            String aName = rName.GetToken( 0, ';', nStrPos );
            aName.EraseTrailingChars().EraseLeadingChars();
            if( !aName.Len() )
                continue;

            BOOL bIsKeyword = FALSE;
            switch( aName.GetChar( 0 ) )
            {
            case 'c':
            case 'C':
                bIsKeyword = aName.EqualsIgnoreCaseAscii( sCSS1_PV_cursive );
                break;

            case 'f':
            case 'F':
                bIsKeyword = aName.EqualsIgnoreCaseAscii( sCSS1_PV_fantasy );
                break;

            case 'm':
            case 'M':
                bIsKeyword = aName.EqualsIgnoreCaseAscii( sCSS1_PV_monospace );
                break;

            case 's':
            case 'S':
                bIsKeyword =
                    aName.EqualsIgnoreCaseAscii( sCSS1_PV_serif ) ||
                    aName.EqualsIgnoreCaseAscii( sCSS1_PV_sans_serif );
                break;
            }

            bContainsKeyword |= bIsKeyword;

            if( rNames.Len() )
                rNames.AppendAscii( ", " );
            if( cQuote && !bIsKeyword )
                rNames += cQuote;
            rNames += aName;
            if( cQuote && !bIsKeyword )
                rNames += cQuote;
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
        }

        if( pStr )
        {
            if( rNames.Len() )
                rNames.AppendAscii( ", " );
            rNames.AppendAscii( pStr );
        }
    }
}

static Writer& OutCSS1_SwFmt( Writer& rWrt, const SwFmt& rFmt,
                              SwDoc *pDoc, SwDoc *pTemplate )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    BOOL bCharFmt = FALSE;
    switch( rFmt.Which() )
    {
    case RES_CHRFMT:
        bCharFmt = TRUE;
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
    String aSelector;
    USHORT nRefPoolId = 0;
    USHORT nDeep = GetCSS1Selector( &rFmt, aSelector, nRefPoolId );
    if( !nDeep )
        return rWrt;    // von keiner HTML-Vorlage abgeleitet

    USHORT nPoolFmtId = rFmt.GetPoolFmtId();

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
    aItemSet.Set( rFmtItemSet, nDeep!=1 );

    BOOL bSetDefaults = TRUE, bClearSame = TRUE;
    const SwFmt *pRefFmt = 0;
    switch( nDeep )
    {
    case CSS1_FMT_ISTAG:
        pRefFmt = SwHTMLWriter::GetTemplateFmt( nRefPoolId, pTemplate );
        break;
    case CSS1_FMT_CMPREF:
        pRefFmt = SwHTMLWriter::GetTemplateFmt( nRefPoolId, pDoc );
        bClearSame = FALSE;
        break;
    default:
        pRefFmt = SwHTMLWriter::GetParentFmt( rFmt, nDeep );
        bSetDefaults = FALSE;
        break;
    }

    if( pRefFmt )
    {
        // Den Item-Set der Referenz-Vorlage (inkl. seiner Parents) vom
        // ItemSet abziehen
        SwHTMLWriter::SubtractItemSet( aItemSet, pRefFmt->GetAttrSet(),
                                       bSetDefaults, bClearSame );
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
            const String& rNm = rFmt.GetName();
            switch( rNm.GetChar(0) )
            {
            case 'D':   if( rNm.EqualsAscii("DD 1") || rNm.EqualsAscii("DT 1") )
                            rHTMLWrt.nDfltBottomMargin = 0;
                        break;
            case 'L':   if(rNm.EqualsAscii(sHTML_listing) )
                            rHTMLWrt.nDfltBottomMargin = 0;
                        break;
            case 'P':   if( rNm.EqualsAscii(sHTML_preformtxt) )
                            rHTMLWrt.nDfltBottomMargin = 0;
                        break;
            case 'X':   if( rNm.EqualsAscii(sHTML_xmp) )
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

    // jetzt die Attribute (inkl. Selektor) ausgeben
    {
        SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                             TRUE, &aSelector );
        rHTMLWrt.OutCSS1_SfxItemSet( aItemSet, FALSE );

        if( nPoolFmtId==RES_POOLCOLL_TEXT && !rHTMLWrt.bFirstCSS1Property )
            rHTMLWrt.bPoolCollTextModified = TRUE;
    }

    // Drop-Caps ausgeben
    const SfxPoolItem *pItem;
    if( rHTMLWrt.IsHTMLMode(HTMLMODE_DROPCAPS) &&
        SFX_ITEM_SET==aItemSet.GetItemState( RES_PARATR_DROP, FALSE, &pItem ))
    {
        String sOut( aSelector );
        sOut.Append( ':');
        sOut.AppendAscii( sCSS1_first_letter );
        SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_RULE|CSS1_OUTMODE_DROPCAP,
                             TRUE, &sOut );

        const SwFmtDrop *pDrop = (const SwFmtDrop *)pItem;

        OutCSS1_SwFmtDropAttrs( rHTMLWrt, *pDrop );
    }

    return rWrt;
}

static Writer& OutCSS1_SwPageDesc( Writer& rWrt, const SwPageDesc& rPageDesc,
                                   SwDoc *pDoc, SwDoc *pTemplate,
                                   USHORT nRefPoolId, BOOL bExtRef,
                                   BOOL bPseudo )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SwPageDesc* pRefPageDesc = 0;
    if( !bExtRef )
        pRefPageDesc = pDoc->GetPageDescFromPool( nRefPoolId );
    else if( pTemplate )
        pRefPageDesc = pTemplate->GetPageDescFromPool( nRefPoolId );

    String aSelector( '@' );
    aSelector.AppendAscii( sCSS1_page );

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
        {
            aSelector.Append( ':' );
            aSelector.AppendAscii( pPseudo );
        }
    }

    SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_RULE_ON|CSS1_OUTMODE_TEMPLATE,
                         TRUE, &aSelector );

    // Die Groesse: Wenn sie sich nur durch das Landscape-Flag unterscheidet,
    // wird nur Portrait oder Landscape exportiert. Sonst wird die Groesse
    // exportiert.
    BOOL bRefLandscape = pRefPageDesc ? pRefPageDesc->GetLandscape() : FALSE;
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
    if( Abs( rSz.Width() - aRefSz.Width() ) <= 2 &&
        Abs( rSz.Height() - aRefSz.Height() ) <= 2 )
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
        ByteString sVal;
        AddUnitPropertyValue( rSz.Width(), rHTMLWrt.GetCSS1Unit(), sVal );
        sVal += ' ';
        AddUnitPropertyValue( rSz.Height(), rHTMLWrt.GetCSS1Unit(), sVal );
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_size, sVal );
    }

    // Die Abstand-Attribute koennen auf gwohnte Weise exportiert werden
    const SwFrmFmt &rMaster = rPageDesc.GetMaster();
    SfxItemSet aItemSet( *rMaster.GetAttrSet().GetPool(),
                         RES_LR_SPACE, RES_UL_SPACE );
    aItemSet.Set( rMaster.GetAttrSet(), TRUE );

    if( pRefPageDesc )
    {
        SwHTMLWriter::SubtractItemSet( aItemSet,
                                       pRefPageDesc->GetMaster().GetAttrSet(),
                                       TRUE );
    }

    OutCSS1_SvxULSpace_SvxLRSpace( rWrt, aItemSet, FALSE );

    // Wenn fuer einen Pseudo-Selektor keine Property ausgegeben wurde, muessen
    // wir trotzdem etwas ausgeben, damit beim Import die entsprechende
    // Vorlage angelegt wird.
    if( rHTMLWrt.bFirstCSS1Property && bPseudo )
    {
        rHTMLWrt.OutNewLine();
        ByteString sTmp( aSelector, rHTMLWrt.eDestEnc );
        rWrt.Strm() << sTmp.GetBuffer() << " {";
        rHTMLWrt.bFirstCSS1Property = FALSE;
    }

    if( !rHTMLWrt.bFirstCSS1Property )
        rWrt.Strm() << sCSS1_rule_end;

    return rWrt;
}

static Writer& OutCSS1_SwFtnInfo( Writer& rWrt, const SwEndNoteInfo& rInfo,
                                  SwDoc *pDoc, USHORT nNotes, BOOL bEndNote )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    String aSelector;

    if( nNotes > 0 )
    {
        aSelector.AssignAscii( sHTML_anchor );
        aSelector.Append( '.');
        aSelector.AppendAscii( bEndNote ? sHTML_sdendnote_anc
                                        : sHTML_sdfootnote_anc );
        SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                             TRUE, &aSelector );
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_font_size,
                                        sHTML_FTN_fontheight );
        rHTMLWrt.Strm() << sCSS1_rule_end;
    }

    const SwCharFmt *pSymCharFmt = rInfo.GetCharFmt( *pDoc );
    if( pSymCharFmt )
    {
        const SfxItemSet& rFmtItemSet = pSymCharFmt->GetAttrSet();
        SfxItemSet aItemSet( *rFmtItemSet.GetPool(), rFmtItemSet.GetRanges() );
        aItemSet.Set( rFmtItemSet, TRUE );

        USHORT nPoolFmtId = pSymCharFmt->GetPoolFmtId();
        // Wenn es Fuss- bzw. Endnoten gibt, dann muessen alles Attribute
        // ausgegeben werden, damit Netscape das Dokument richtig anzeigt.
        // Anderenfalls genuegt es, die Unterschiede zur Fuss-/Endnoten
        // Vorlage rauszuschreiben.
        if( nNotes == 0 && rHTMLWrt.pTemplate )
        {
            SwFmt *pRefFmt = rHTMLWrt.pTemplate->GetCharFmtFromPool(
                        bEndNote ? RES_POOLCHR_ENDNOTE : RES_POOLCHR_FOOTNOTE );
            if( pRefFmt )
                SwHTMLWriter::SubtractItemSet( aItemSet, pRefFmt->GetAttrSet(),
                                               TRUE );
        }
        if( aItemSet.Count() )
        {
            aSelector.AssignAscii( sHTML_anchor );
            aSelector.Append( '.');
            aSelector.AppendAscii( bEndNote ? sHTML_sdendnote_sym
                                        : sHTML_sdfootnote_sym );
            SwCSS1OutMode aMode( rHTMLWrt,
                                 CSS1_OUTMODE_RULE|CSS1_OUTMODE_TEMPLATE,
                                 TRUE, &aSelector );
            rHTMLWrt.OutCSS1_SfxItemSet( aItemSet, FALSE );
        }
    }

    return rWrt;
}

Writer& OutCSS1_BodyTagStyleOpt( Writer& rWrt, const SfxItemSet& rItemSet,
                                 String aEmbBGGrfName )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_STYLE_OPT_ON |
                                   CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_BODY );


    // Es werden nur die Attribute der Seiten-Vorlage ausgegeben.
    // Die Attribute der Standard-Absatz-Vorlage werden schon beim
    // Export der Absatz-Vorlagen beruecksichtigt.

    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, FALSE,
                                               &pItem ) )
    {
        OutCSS1_SvxBrush( rWrt, *pItem, CSS1_BACKGROUND_PAGE,
                          &aEmbBGGrfName );
    }

    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BOX, FALSE,
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

    SwCSS1OutMode aMode( rHTMLWrt, CSS1_OUTMODE_STYLE_OPT |
                                   CSS1_OUTMODE_ENCODE|CSS1_OUTMODE_PARA );
    rHTMLWrt.OutCSS1_SfxItemSet( rItemSet, FALSE );

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
    OutCSS1_SvxBrush( rWrt, rHt, CSS1_BACKGROUND_TABLE, 0 );

    if( !rHTMLWrt.bFirstCSS1Property )
        rWrt.Strm() << '\"';

    return rWrt;
}


Writer& OutCSS1_NumBulListStyleOpt( Writer& rWrt, const SwNumRule& rNumRule,
                                    BYTE nLevel )
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
                                          ULONG nFrmOpts,
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
        case FLY_AT_CNTNT:
        case FLY_AUTO_CNTNT:
            if( FRAME == rHoriOri.GetRelationOrient() ||
                PRTAREA == rHoriOri.GetRelationOrient() )
            {
                if( !(nFrmOpts & HTML_FRMOPT_ALIGN) )
                {
                    // float
                    const sal_Char *pStr = HORI_RIGHT==rHoriOri.GetHoriOrient()
                            ? sCSS1_PV_right
                            : sCSS1_PV_left;
                    OutCSS1_PropertyAscii( sCSS1_P_float, pStr );
                }
                break;
            }

        case FLY_PAGE:
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
                BOOL bOutXPos = FALSE, bOutYPos = FALSE;
                if( RES_DRAWFRMFMT == rFrmFmt.Which() )
                {
                    ASSERT( pSdrObj, "Kein SdrObject uebergeben. Ineffizient" );
                    if( !pSdrObj )
                        pSdrObj = rFrmFmt.FindSdrObject();
                    ASSERT( pSdrObj, "Wo ist das SdrObject" );
                    if( pSdrObj )
                    {
                        Point aPos( pSdrObj->GetRelativePos() );
                        nXPos = aPos.A();
                        nYPos = aPos.B();
                    }
                    bOutXPos = bOutYPos = TRUE;
                }
                else
                {
                    bOutXPos = REL_CHAR != rHoriOri.GetRelationOrient();
                    nXPos = HORI_NONE == rHoriOri.GetHoriOrient()
                                ? rHoriOri.GetPos() : 0;

                    const SwFmtVertOrient& rVertOri = rFrmFmt.GetVertOrient();
                    bOutYPos = REL_CHAR != rVertOri.GetRelationOrient();
                    nYPos = VERT_NONE == rVertOri.GetVertOrient()
                                 ? rVertOri.GetPos() : 0;
                }

                if( bOutYPos )
                {
                    if( IsHTMLMode( HTMLMODE_FLY_MARGINS) )
                    {
                        nYPos -= aULItem.GetUpper();
                        if( nYPos < 0 )
                        {
                            aULItem.SetUpper( (USHORT)(aULItem.GetUpper() + nYPos) );
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
                            aLRItem.SetLeft( (USHORT)(aLRItem.GetLeft() + nXPos) );
                            nXPos = 0;
                        }
                    }

                    OutCSS1_UnitProperty( sCSS1_P_left, nXPos );
                }
            }
            break;
        }
    }

    // width/height
    if( nFrmOpts & HTML_FRMOPT_S_SIZE )
    {
        if( RES_DRAWFRMFMT == rFrmFmt.Which() )
        {
            ASSERT( pSdrObj, "Kein SdrObject uebergeben. Ineffizient" );
            if( !pSdrObj )
                pSdrObj = rFrmFmt.FindSdrObject();
            ASSERT( pSdrObj, "Wo ist das SdrObject" );
            if( pSdrObj )
            {
                Size aTwipSz( pSdrObj->GetLogicRect().GetSize() );
                if( nFrmOpts & HTML_FRMOPT_S_WIDTH )
                {
                    if( nFrmOpts & HTML_FRMOPT_S_PIXSIZE )
                        OutCSS1_PixelProperty( sCSS1_P_width, aTwipSz.Width(),
                                               FALSE );
                    else
                        OutCSS1_UnitProperty( sCSS1_P_width, aTwipSz.Width() );
                }
                if( nFrmOpts & HTML_FRMOPT_S_HEIGHT )
                {
                    if( nFrmOpts & HTML_FRMOPT_S_PIXSIZE )
                        OutCSS1_PixelProperty( sCSS1_P_height, aTwipSz.Height(),
                                               TRUE );
                    else
                        OutCSS1_UnitProperty( sCSS1_P_height, aTwipSz.Height() );
                }
            }
        }
        else
        {
            ASSERT( HTML_FRMOPT_ABSSIZE & nFrmOpts,
                    "Absolute Groesse wird exportiert" );
            ASSERT( HTML_FRMOPT_ANYSIZE & nFrmOpts,
                    "Jede Groesse wird exportiert" );
            USHORT nMode = 0;
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
        if( SFX_ITEM_SET == rItemSet.GetItemState( RES_LR_SPACE, TRUE ) )
            pLRItem = &aLRItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( RES_UL_SPACE, TRUE ) )
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
        else if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BOX, TRUE, &pItem ) )
            OutCSS1_SvxBox( *this, *pItem );
    }

    // background (wenn, dann muss auch eine Farbe ausgegeben werden)
    if( nFrmOpts & HTML_FRMOPT_S_BACKGROUND )
        OutCSS1_FrmFmtBackground( rFrmFmt );

    if( pItemSet )
        OutCSS1_SfxItemSet( *pItemSet, FALSE );

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
    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BACKGROUND, FALSE, &pItem ) )
        OutCSS1_SvxBrush( *this, *pItem, CSS1_BACKGROUND_TABLE, 0 );

    if( IsHTMLMode( HTMLMODE_PRINT_EXT ) )
        OutCSS1_SvxFmtBreak_SwFmtPDesc_SvxFmtKeep( *this, rItemSet, FALSE );

    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_LAYOUT_SPLIT, FALSE, &pItem ) )
        OutCSS1_SwFmtLayoutSplit( *this, *pItem );

    if( !bFirstCSS1Property )
        Strm() << '\"';
}

void SwHTMLWriter::OutCSS1_SectionFmtOptions( const SwFrmFmt& rFrmFmt )
{
    SwCSS1OutMode aMode( *this, CSS1_OUTMODE_STYLE_OPT_ON |
                                CSS1_OUTMODE_ENCODE|
                                CSS1_OUTMODE_SECTION );

    const SfxPoolItem *pItem;
    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();
    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BACKGROUND, FALSE, &pItem ) )
        OutCSS1_SvxBrush( *this, *pItem, CSS1_BACKGROUND_SECTION, 0 );

    if( !bFirstCSS1Property )
        Strm() << '\"';
}

static BOOL OutCSS1_FrmFmtBrush( SwHTMLWriter& rWrt,
                                 const SvxBrushItem& rBrushItem )
{
    BOOL bWritten = FALSE;
    if( rBrushItem.GetColor().GetTransparency() ||
        0 != rBrushItem.GetGraphicLink() ||
        0 != rBrushItem.GetGraphicPos() )
    {
        OutCSS1_SvxBrush( rWrt, rBrushItem, CSS1_BACKGROUND_FLY, 0 );
        bWritten = TRUE;
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
    if( FLY_PAGE != eAnchorId && pAnchorPos )
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
    ASSERT( pCurrPageDesc, "Keine Seiten-Vorlage gemerkt" );
    if( !OutCSS1_FrmFmtBrush( *this,
                              pCurrPageDesc->GetMaster().GetBackground() ) )
    {
        Color aColor( COL_WHITE );

        // Die Hintergrund-Farbe wird normalerweise nur in Browse-Mode
        // benutzt. Wir benutzen si bei einem HTML-Dokument immer und
        // bei einem Text-Dokument nur, wenn es im Browse-Mode angezeigt
        // wird.
        if( pDoc->IsHTMLMode() || pDoc->IsBrowseMode() )
        {
            ViewShell *pVSh = 0;
            pDoc->GetEditShell( &pVSh );
            if ( pVSh &&
                 COL_TRANSPARENT != pVSh->GetViewOptions()->GetRetoucheColor().GetColor())
                aColor = pVSh->GetViewOptions()->GetRetoucheColor().GetColor();
        }

        ByteString sOut;
        GetCSS1Color( aColor, sOut );
        OutCSS1_PropertyAscii( sCSS1_P_background, sOut );
    }
}

//-----------------------------------------------------------------------

static Writer& OutCSS1_SvxUnderl_SvxCrOut_SvxBlink( Writer& rWrt,
                    const SvxUnderlineItem *pUItem,
                    const SvxCrossedOutItem *pCOItem,
                    const SvxBlinkItem *pBItem )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    const sal_Char *pUStr = 0;
    if( pUItem )
    {
        switch( pUItem->GetUnderline() )
        {
        case UNDERLINE_NONE:
            pUStr = sCSS1_PV_none;
            break;
        case UNDERLINE_DONTKNOW:
            break;
        default:
            if( !rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
            {
                // das geht auch in HTML und muss nicht als STYLE-Option
                // und darf nicht als Hint geschrieben werden
                ASSERT( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                        "Underline als Hint schreiben?" );
                pUStr = sCSS1_PV_underline;
            }
            break;
        }
    }

    const sal_Char *pCOStr = 0;
    if( pCOItem )
    {
        switch( pCOItem->GetStrikeout() )
        {
        case STRIKEOUT_DONTKNOW:
            break;
        case STRIKEOUT_NONE:
            if( !pUStr )
                pUStr = sCSS1_PV_none;
            break;
        default:
            if( !rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
            {
                // das geht auch in HTML und muss nicht als STYLE-Option
                // und darf nicht als Hint geschrieben werden
                ASSERT( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                        "CrossedOut als Hint schreiben?" );
                pCOStr = sCSS1_PV_line_through;
                if( pUStr && sCSS1_PV_none == pUStr )
                    pUStr = 0;
            }
            break;
        }
    }

    const sal_Char *pBStr = 0;
    if( pBItem && rHTMLWrt.IsHTMLMode(HTMLMODE_BLINK) )
    {
        if( !pBItem->GetValue() )
        {
            if( !pUStr && !pCOStr )
                pBStr = sCSS1_PV_none;
        }
        else if( !rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        {
            // das geht auch in HTML und muss nicht als STYLE-Option
            // und darf nicht als Hint geschrieben werden
            ASSERT( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                    "Blink als Hint schreiben?" );
            pBStr = sCSS1_PV_blink;
            if( pUStr && sCSS1_PV_none == pUStr )
                pUStr = 0;
            if( pCOStr && sCSS1_PV_none == pCOStr )
                pCOStr = 0;
        }
    }

    ByteString sOut;
    if( pUStr )
        sOut.Append( pUStr );

    if( pCOStr )
    {
        if( sOut.Len() )
            sOut += ' ';

        sOut.Append( pCOStr );
    }

    if( pBStr )
    {
        if( sOut.Len() )
            sOut += ' ';

        sOut.Append( pBStr );
    }

    if( sOut.Len() )
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_text_decoration, sOut );

    return rWrt;
}


static Writer& OutCSS1_SvxCaseMap( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    if( !rHTMLWrt.IsHTMLMode(HTMLMODE_SMALL_CAPS) )
        return rWrt;

    const sal_Char *pStr = 0;
    switch( ((const SvxCaseMapItem&)rHt).GetCaseMap() )
    {
    case SVX_CASEMAP_NOT_MAPPED:    pStr = sCSS1_PV_normal;     break;
    case SVX_CASEMAP_KAPITAELCHEN:  pStr = sCSS1_PV_small_caps; break;
    }

    if( pStr )
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_font_variant, pStr );

    return rWrt;
}


static Writer& OutCSS1_SvxColor( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Farben muessen nicht in der Style-Option ausgegeben werden.
    if( rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) &&
        !rHTMLWrt.bCfgPreferStyles )
        return rWrt;
    ASSERT( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
            "Farbe wirklich als Hint ausgeben?" );

    const Color& rColor = ((const SvxColorItem&)rHt).GetValue();

    ByteString sOut;
    GetCSS1Color( rColor, sOut );

    rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_color, sOut );

    return rWrt;
}


static Writer& OutCSS1_SvxCrossedOut( Writer& rWrt, const SfxPoolItem& rHt )
{
    // Mit dieser Methode werden nur Hints ausgegeben!
    // Sonst wird OutCSS1_SvxUnderl_SvxCrOut_SvxBlink() direkt aufgerufen.

    if( ((SwHTMLWriter&)rWrt).IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxUnderl_SvxCrOut_SvxBlink( rWrt, 0,
                (const SvxCrossedOutItem *)&rHt, 0 );

    return rWrt;
}

static Writer& OutCSS1_SvxFont( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Fonts muessen nicht in der Style-Option ausgegeben werden.
    if( rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        return rWrt;

    ASSERT( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
            "Font wirklich als Hint ausgeben?" );

    String sOut;
    // MS IE3b1 hat mit einfachen Haekchen Probleme
    USHORT nMode = rHTMLWrt.nCSS1OutMode & CSS1_OUTMODE_ANY_ON;
    sal_Unicode cQuote = nMode == CSS1_OUTMODE_RULE_ON ? '\"' : '\'';
    SwHTMLWriter::PrepareFontList( ((const SvxFontItem&)rHt), sOut, cQuote,
                                   TRUE );

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

    UINT32 nHeight = ((const SvxFontHeightItem&)rHt).GetHeight();
    if( rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT) )
    {
        // einen Hint nur dann ausgeben wenn es auch was bringt
        USHORT nSize = rHTMLWrt.GetHTMLFontSize( nHeight );
        if( rHTMLWrt.aFontHeights[nSize-1] == nHeight )
            return rWrt;
    }
    ByteString sHeight( ByteString::CreateFromInt32(
                                            (sal_Int32)(nHeight/20) ) );
    sHeight.Append( sCSS1_UNIT_pt );

    rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_font_size, sHeight );

    return rWrt;
}

static Writer& OutCSS1_SvxPosture( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

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
            ASSERT( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                    "Italic als Hint schreiben?" );
            pStr = sCSS1_PV_italic;
        }
        break;
    }

    if( pStr )
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_font_style, pStr );

    return rWrt;
}

static Writer& OutCSS1_SvxKerning( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Kerning-Item nur ausgeben, wenn volle Style-UnterstÅtzung da ist
    if( !rHTMLWrt.IsHTMLMode(HTMLMODE_FULL_STYLES) )
        return rWrt;

    INT16 nValue = ((const SvxKerningItem&)rHt).GetValue();
    if( nValue )
    {
        ByteString sOut;
        if( nValue < 0 )
        {
            sOut = '-';
            nValue = -nValue;
        }

        // Breite als n.n pt
        nValue = (nValue + 1) / 2;  // 1/10pt
        sOut.Append( ByteString::CreateFromInt32( (sal_Int32)(nValue  / 10) ) );
        sOut.Append( '.' );
        sOut.Append( ByteString::CreateFromInt32( (sal_Int32)(nValue % 10) ) );
        sOut.Append( sCSS1_UNIT_pt );

        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_letter_spacing, sOut );
    }
    else
    {
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_letter_spacing,
                                        sCSS1_PV_normal );
    }

    return rWrt;
}

static Writer& OutCSS1_SvxUnderline( Writer& rWrt, const SfxPoolItem& rHt )
{
    // Mit dieser Methode werden nur Hints ausgegeben!
    // Sonst wird OutCSS1_SvxUnderl_SvxCrOut_SvxBlink() direkt aufgerufen.

    if( ((SwHTMLWriter&)rWrt).IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxUnderl_SvxCrOut_SvxBlink( rWrt,
                (const SvxUnderlineItem *)&rHt, 0, 0 );

    return rWrt;
}


static Writer& OutCSS1_SvxFontWeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    const sal_Char *pStr = 0;
    switch( ((const SvxWeightItem&)rHt).GetWeight() )
    {
    case WEIGHT_ULTRALIGHT: pStr = sCSS1_PV_extra_light;    break;
    case WEIGHT_LIGHT:      pStr = sCSS1_PV_light;          break;
    case WEIGHT_SEMILIGHT:  pStr = sCSS1_PV_demi_light;     break;
    case WEIGHT_NORMAL:     pStr = sCSS1_PV_medium;         break;
    case WEIGHT_SEMIBOLD:   pStr = sCSS1_PV_demi_bold;      break;
    case WEIGHT_BOLD:
        if( !rHTMLWrt.IsCSS1Source( CSS1_OUTMODE_PARA ) )
        {
            // das geht auch in HTML und muss nicht als STYLE-Option
            // und darf nicht als Hint geschrieben werden
            ASSERT( !rHTMLWrt.IsCSS1Source(CSS1_OUTMODE_HINT),
                    "Fett als Hint schreiben?" );
            pStr = sCSS1_PV_bold;
        }
        break;
    case WEIGHT_ULTRABOLD:  pStr = sCSS1_PV_extra_bold;     break;
    }

    if( pStr )
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_font_weight, pStr );

    return rWrt;
}

static Writer& OutCSS1_SvxBlink( Writer& rWrt, const SfxPoolItem& rHt )
{
    // Mit dieser Methode werden nur Hints ausgegeben!
    // Sonst wird OutCSS1_SvxUnderl_SvxCrOut_SvxBlink() direkt aufgerufen.

    if( ((SwHTMLWriter&)rWrt).IsCSS1Source(CSS1_OUTMODE_HINT) )
        OutCSS1_SvxUnderl_SvxCrOut_SvxBlink( rWrt,
                0, 0, (const SvxBlinkItem *)&rHt );

    return rWrt;
}

static Writer& OutCSS1_SvxLineSpacing( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // #60393#: Netscape4 hat massive Probleme mit den Zellenhoehen
    // wenn der Zeilenabstand innerhalb einer Tabelle geaendert wird
    // und die Breite der Tabelle nicht automatisch berechnet wird
    // (also wenn eine WIDTH-Option vorhanden ist).
    if( rHTMLWrt.bOutTable && rHTMLWrt.bCfgNetscape4 )
        return rWrt;

    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)rHt;

    USHORT nHeight = 0;
    USHORT nPrcHeight = 0;
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
            }
        }
        break;
    case SVX_INTER_LINE_SPACE_PROP:
        nPrcHeight = rLSItem.GetPropLineSpace();
        break;
    }

    if( nHeight )
        rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_line_height, (long)nHeight );
    else if( nPrcHeight )
    {
        ByteString sHeight(
                ByteString::CreateFromInt32( (sal_Int32)nPrcHeight ) );
        sHeight += '%';
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_line_height, sHeight );
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

    ByteString aStr(
        ByteString::CreateFromInt32( ((const SvxWidowsItem&)rHt).GetValue() ) );
    rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_widows, aStr );

    return rWrt;
}

static Writer& OutCSS1_SvxOrphans( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    ByteString aStr(
       ByteString::CreateFromInt32( ((const SvxOrphansItem&)rHt).GetValue() ) );
    rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_orphans, aStr );

    return rWrt;
}

static void OutCSS1_SwFmtDropAttrs( SwHTMLWriter& rHWrt,
                                    const SwFmtDrop& rDrop )
{
    // Text fliesst rechts drumrum
    rHWrt.OutCSS1_PropertyAscii( sCSS1_P_float, sCSS1_PV_left );

    // Anzahl der Zeilen -> %-Angabe fuer Font-Hoehe!
    ByteString sOut( ByteString::CreateFromInt32( rDrop.GetLines()*100 ) );
    sOut += '%';
    rHWrt.OutCSS1_PropertyAscii( sCSS1_P_font_size, sOut );

    // Abstand zum Text = rechter Rand
    USHORT nDistance = rDrop.GetDistance();
    if( nDistance > 0 )
        rHWrt.OutCSS1_UnitProperty( sCSS1_P_margin_right, nDistance );

    const SwCharFmt *pDCCharFmt = rDrop.GetCharFmt();
    if( pDCCharFmt )
        rHWrt.OutCSS1_SfxItemSet( pDCCharFmt->GetAttrSet() );
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
                             CSS1_OUTMODE_SPAN_TAG1_ON|CSS1_OUTMODE_ENCODE|
                             CSS1_OUTMODE_DROPCAP );

        OutCSS1_SwFmtDropAttrs( rHTMLWrt, (const SwFmtDrop&)rHt );
        // Ein "> wird schon vom aufrufenden OutCSS1_HintAsSpanTag geschrieben.
    }
    else
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_span, FALSE );
    }

    return rWrt;
}

static Writer& OutCSS1_SwFmtFrmSize( Writer& rWrt, const SfxPoolItem& rHt,
                                     USHORT nMode )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    ByteString sOut;
    const SwFmtFrmSize& rFSItem = (const SwFmtFrmSize&)rHt;

    if( nMode & CSS1_FRMSIZE_WIDTH )
    {
        BYTE nPrcWidth = rFSItem.GetWidthPercent();
        if( nPrcWidth )
        {
            (sOut = ByteString::CreateFromInt32( nPrcWidth) ) += '%';
            rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_width, sOut );
        }
        else if( nMode & CSS1_FRMSIZE_PIXEL )
        {
            rHTMLWrt.OutCSS1_PixelProperty( sCSS1_P_width,
                                            rFSItem.GetSize().Width(), FALSE );
        }
        else
        {
            rHTMLWrt.OutCSS1_UnitProperty( sCSS1_P_width,
                                           rFSItem.GetSize().Width() );
        }
    }

    if( nMode & CSS1_FRMSIZE_ANYHEIGHT )
    {
        BOOL bOutHeight = FALSE;
        switch( rFSItem.GetSizeType() )
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
            ASSERT( bOutHeight, "Hoehe wird nicht exportiert" );
            break;
        }

        if( bOutHeight )
        {
            BYTE nPrcHeight = rFSItem.GetHeightPercent();
            if( nPrcHeight )
            {
                (sOut = ByteString::CreateFromInt32( nPrcHeight ) ) += '%';
                rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_height, sOut );
            }
            else if( nMode & CSS1_FRMSIZE_PIXEL )
            {
                rHTMLWrt.OutCSS1_PixelProperty( sCSS1_P_height,
                                                rFSItem.GetSize().Width(),
                                                TRUE );
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
                                        BOOL bDeep )
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
                                        BOOL bDeep )
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
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Absatz-Hintergruende werden nur fuer den SW ausgegeben
    if( rHt.Which() < RES_CHRATR_END ||
        rHTMLWrt.IsHTMLMode(HTMLMODE_FULL_STYLES) )
        OutCSS1_SvxBrush( rWrt, rHt, CSS1_BACKGROUND_ATTR, 0 );

    return rWrt;
}


static Writer& OutCSS1_SvxBrush( Writer& rWrt, const SfxPoolItem& rHt,
                                 USHORT nMode, const String *pGrfName )
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
    const String *pLink = pGrfName ? pGrfName
                            : ((const SvxBrushItem &)rHt).GetGraphicLink();
    SvxGraphicPosition ePos = ((const SvxBrushItem &)rHt).GetGraphicPos();

    if( CSS1_BACKGROUND_PAGE==nMode )
    {
        // Fuer Seitenvorlagen wurde der Grafik-Name uebergeben. Es wird
        // nur ein Attribut ausgegeben, wenn die Grafik nicht gekachelt ist.
        ASSERT( pLink, "Wo ist der Grafik-Name der Seitenvorlage?" );
        if( !pLink || !pLink->Len() || GPOS_TILED==ePos )
            return rWrt;
    }

    // Erstmal die Farbe holen
    BOOL bColor = FALSE;
    BOOL bTransparent = rColor.GetTransparency() > 0;
    Color aColor;
    if( !bTransparent )
    {
        aColor = rColor;
        bColor = TRUE;
    }

    // und jetzt eine Grafik
    String sGrfNm;

    if( !pLink )
    {
        // embeddete Grafik -> WriteEmbedded schreiben
        const Graphic* pGrf = ((const SvxBrushItem &)rHt).GetGraphic();
        if( pGrf )
        {
            // Grafik als (JPG-)File speichern
            const String* pTempFileName = rHTMLWrt.GetOrigFileName();
            if( pTempFileName )
                sGrfNm = *pTempFileName;
            USHORT nErr = XOutBitmap::WriteGraphic( *pGrf, sGrfNm,
                        String::CreateFromAscii("JPG") );
            if( !nErr )     // fehlerhaft, da ist nichts auszugeben
            {
                sGrfNm = URIHelper::SmartRelToAbs( sGrfNm );
                pLink = &sGrfNm;
            }
            else
            {
                rHTMLWrt.nWarn = WARN_SWG_POOR_LOAD | WARN_SW_WRITE_BASE;
            }
        }
    }
    else if( !pGrfName && rHTMLWrt.bCfgCpyLinkedGrfs )
    {
        sGrfNm = *pLink;
        rWrt.CopyLocalFileToINet( sGrfNm );
        pLink = &sGrfNm;
    }

    // In Tabellen wird nur dann etwas exportiert, wenn eine Grafik
    // existiert.
    if( CSS1_BACKGROUND_TABLE==nMode && !pLink )
        return rWrt;

    // ggf. noch die Ausrichtung der Grafik
    const sal_Char *pRepeat = 0, *pHori = 0, *pVert = 0;
    if( pLink )
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
            }

            if( pHori || pVert )
                pRepeat = sCSS1_PV_no_repeat;
        }
    }

    // jetzt den String zusammen bauen
    String sOut;
    if( !pLink && !bColor )
    {
        // keine Farbe und kein Link, aber ein transparenter Brush
        if( bTransparent && CSS1_BACKGROUND_FLY != nMode )
            sOut.AssignAscii( sCSS1_PV_transparent );
    }
    else
    {
        if( bColor )
        {
            ByteString sTmp;
            GetCSS1Color( aColor, sTmp );
            sOut += String( sTmp, RTL_TEXTENCODING_ASCII_US );
        }

        if( pLink )
        {
            if( bColor )
                sOut += ' ';

            sOut.AppendAscii( sCSS1_url );
            sOut.Append( '(' );
            sOut.Append( INetURLObject::AbsToRel( *pLink,
                                    INetURLObject::WAS_ENCODED,
                                    INetURLObject::DECODE_UNAMBIGUOUS));
            sOut.Append( ')' );

            if( pRepeat )
            {
                sOut.Append( ' ' );
                sOut.AppendAscii( pRepeat );
            }

            if( pHori )
            {
                sOut.Append( ' ' );
                sOut.AppendAscii( pHori );
            }
            if( pVert )
            {
                sOut.Append( ' ' );
                sOut.AppendAscii( pVert );
            }

            sOut.Append( ' ' );
            sOut.AppendAscii( sCSS1_PV_scroll );
        }
    }

    if( sOut.Len() )
        rHTMLWrt.OutCSS1_Property( sCSS1_P_background, sOut );

    return rWrt;
}

static void OutCSS1_SvxBorderLine( SwHTMLWriter& rHTMLWrt,
                                   const sal_Char *pProperty,
                                   const SvxBorderLine *pLine )
{
    if( !pLine )
    {
        rHTMLWrt.OutCSS1_PropertyAscii( pProperty, sCSS1_PV_none );
        return;
    }

    BOOL bDouble = FALSE;
    USHORT nWidth = pLine->GetOutWidth();
    if( pLine->GetInWidth() )
    {
        nWidth += pLine->GetDistance();
        nWidth += pLine->GetInWidth();
        bDouble = TRUE;
    }

    ByteString sOut;
    if( Application::GetDefaultDevice() &&
        nWidth <= Application::GetDefaultDevice()->PixelToLogic(
                    Size( 1, 1 ), MapMode( MAP_TWIP) ).Width() )
    {
        // Wenn die Breite kleiner ist als ein Pixel, dann als 1px
        // ausgeben, damit Netscape und IE die Linie auch darstellen.
        sOut += "1px";
    }
    else
    {
#ifdef USE_MAPMODE_MM100
        nWidth = (USHORT)((double)nWidth * (25.4/72));  // 1/100pt
#else
        nWidth *= 5;    // 1/100pt
#endif

        // Breite als n.nn pt
        sOut += ByteString::CreateFromInt32( nWidth / 100 );
        (((sOut += '.')
            += ByteString::CreateFromInt32((nWidth/10) % 10))
            += ByteString::CreateFromInt32(nWidth % 10)) += sCSS1_UNIT_pt;
    }

    // Linien-Stil: solid oder double
    ((sOut += ' ')
        += (bDouble ? sCSS1_PV_double : sCSS1_PV_solid)) += ' ';

    // und noch die Farbe
    GetCSS1Color( pLine->GetColor(), sOut );

    rHTMLWrt.OutCSS1_PropertyAscii( pProperty, sOut );
}

static Writer& OutCSS1_SvxBox( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // Das Zeichen-Attribut wird nicht ausgegeben, wenn gerade
    // Optionen ausgegeben werden
    if( !rHTMLWrt.IsHTMLMode(HTMLMODE_PARA_BORDER))
        return rWrt;

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
        ByteString sVal;
        AddUnitPropertyValue( nTopDist, rHTMLWrt.GetCSS1Unit(), sVal );
        if( nTopDist != nLeftDist )
        {
            sVal += ' ';
            AddUnitPropertyValue( nLeftDist, rHTMLWrt.GetCSS1Unit(), sVal );
        }
        rHTMLWrt.OutCSS1_PropertyAscii( sCSS1_P_padding, sVal );
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
/* RES_CHRATR_LANGUAGE  */          0,
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
/* RES_CHRATR_CJK_FONT */           0,
/* RES_CHRATR_CJK_FONTSIZE */       0,
/* RES_CHRATR_CJK_LANGUAGE */       0,
/* RES_CHRATR_CJK_POSTURE */        0,
/* RES_CHRATR_CJK_WEIGHT */         0,
/* RES_CHRATR_CTL_FONT */           0,
/* RES_CHRATR_CTL_FONTSIZE */       0,
/* RES_CHRATR_CTL_LANGUAGE */       0,
/* RES_CHRATR_CTL_POSTURE */        0,
/* RES_CHRATR_CTL_WEIGHT */         0,
/* RES_CHRATR_WRITING_DIRECTION */  0,
/* RES_CHRATR_DUMMY2 */             0,
/* RES_CHRATR_DUMMY3 */             0,
/* RES_CHRATR_DUMMY4 */             0,
/* RES_CHRATR_DUMMY5 */             0,
/* RES_CHRATR_DUMMY1 */             0, // Dummy:

/* RES_TXTATR_NOLINEBREAK   */      0,
/* RES_TXTATR_NOHYPHEN  */          0,
/* RES_TXTATR_REFMARK*/             0,
/* RES_TXTATR_TOXMARK */            0,
/* RES_TXTATR_CHARFMT   */          0,
/* RES_TXTATR_TWO_LINES */          0,
/* RES_TXTATR_CJK_RUBY */           0,
/* RES_TXTATR_UNKNOWN_CONTAINER */  0,
/* RES_TXTATR_DUMMY5 */             0,
/* RES_TXTATR_DUMMY6 */             0,

/* RES_TXTATR_FIELD */              0,
/* RES_TXTATR_FLYCNT */             0,
/* RES_TXTATR_FTN */                0,
/* RES_TXTATR_SOFTHYPH */           0,
/* RES_TXTATR_HARDBLANK*/           0,
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
/* RES_PARATR_DUMMY1 */             0, // Dummy:
/* RES_PARATR_DUMMY2 */             0, // Dummy:

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
/* RES_FRMATR_DUMMY1 */             0, // Dummy:
/* RES_FRMATR_DUMMY2 */             0, // Dummy:
/* RES_FRMATR_DUMMY3 */             0, // Dummy:
/* RES_FRMATR_DUMMY4 */             0, // Dummy:
/* RES_FRMATR_DUMMY5 */             0, // Dummy:
/* RES_FRMATR_DUMMY6 */             0, // Dummy:
/* RES_FRMATR_DUMMY7 */             0, // Dummy:
/* RES_FRMATR_DUMMY8 */             0, // Dummy:
/* RES_FRMATR_DUMMY9 */             0, // Dummy:

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


/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/10/20 13:42:31  jp
      use correct INetURL-Decode enum

      Revision 1.1.1.1  2000/09/18 17:14:55  hr
      initial import

      Revision 1.110  2000/09/18 16:04:42  willem.vandorp
      OpenOffice header added.

      Revision 1.109  2000/08/30 16:46:22  jp
      use CharClass instead of international

      Revision 1.108  2000/08/18 13:03:06  jp
      don't export escaped URLs

      Revision 1.107  2000/07/31 19:24:15  jp
      new attributes for CJK/CTL and graphic

      Revision 1.106  2000/06/26 12:55:05  os
      INetURLObject::SmartRelToAbs removed

      Revision 1.105  2000/06/26 09:51:49  jp
      must change: GetAppWindow->GetDefaultDevice

      Revision 1.104  2000/05/15 10:06:56  os
      Chg: GetOrigFileName()

      Revision 1.103  2000/04/28 14:29:10  mib
      unicode

      Revision 1.102  2000/04/10 12:20:54  mib
      unicode

      Revision 1.101  2000/03/03 15:21:00  os
      StarView remainders removed

      Revision 1.100  2000/02/11 14:36:48  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.99  1999/09/21 09:48:39  mib
      multiple text encodings

      Revision 1.98  1999/09/20 13:16:17  mib
      COL_WINDOWWORKSPACE->COL-WHITE

      Revision 1.97  1999/07/23 13:34:28  MIB
      #67578#: Export multicolumned sections as <MULTICOL>, section backgrounds


*************************************************************************/

