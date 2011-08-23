/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
/*
 * Dieses File enthaelt alle Ausgabe-Funktionen des W4W-Writers;
 * fuer alle Nodes, Attribute, Formate und Chars.
 */
#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <bf_svtools/whiter.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <bf_svx/hyznitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <bf_svx/langitem.hxx>
#endif
#ifndef _SVX_ORPHITEM_HXX //autogen
#include <bf_svx/orphitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <bf_svx/tstpitem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <bf_svx/spltitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <bf_svx/widwitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <bf_svx/adjitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <bf_svx/cmapitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <bf_svx/lspcitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <bf_svx/kernitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <bf_svx/wghtitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <bf_svx/udlnitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <bf_svx/escpitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <bf_svx/crsditem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <bf_svx/colritem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <bf_svx/brkitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <bf_svx/postitem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <bf_svx/shdditem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <bf_svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <bf_svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <bf_svx/fhgtitem.hxx>
#endif


#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>          // fuer SwField ...
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>      // fuer SwBookmark ...
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>     // fuer SwPageDesc...
#endif
#ifndef _PAGEITER_HXX
#include <pageiter.hxx>
#endif
#ifndef _WRTW4W_HXX
#include <wrtw4w.hxx>
#endif
namespace binfilter {


/*
 * um nicht immer wieder nach einem Update festzustellen, das irgendwelche
 * Hint-Ids dazugekommen sind, wird hier definiert, die Groesse der Tabelle
 * definiert und mit der akt. verglichen. Bei unterschieden wird der
 * Compiler schon meckern.
 *
 * diese Section und die dazugeherigen Tabellen muessen in folgenden Files
 * gepflegt werden: rtf\rtfatr.cxx, sw6\sw6atr.cxx, w4w\w4watr.cxx
 */

#if !defined(MSC) && !defined(UNX) && !defined(PPC) && !defined(CSET) && !defined(__MWERKS__) && !defined(WTC) && !defined(__MINGW32__) && !defined(OS2)

#define ATTRFNTAB_SIZE 130
#if ATTRFNTAB_SIZE != POOLATTR_END - POOLATTR_BEGIN
#error "Attribut-Tabelle ist ungueltigt. Wurden neue Hint-ID's zugefuegt ??"
#endif

#ifdef FORMAT_TABELLE
// da sie nicht benutzt wird!
#define FORMATTAB_SIZE 7
#if FORMATTAB_SIZE != RES_FMT_END - RES_FMT_BEGIN
#error "Format-Tabelle ist ungueltigt. Wurden neue Hint-ID's zugefuegt ??"
#endif
#endif

#define NODETAB_SIZE 3
#if NODETAB_SIZE != RES_NODE_END - RES_NODE_BEGIN
#error "Node-Tabelle ist ungueltigt. Wurden neue Hint-ID's zugefuegt ??"
#endif

#endif



//-----------------------------------------------------------------------

// einige Forward Deklarationen

#define OUTRECORD(rStrm,pStr)	rStrm << sW4W_RECBEGIN << pStr << cW4W_RED

inline int HI_BYTE( USHORT n )  { return (int)( ( n >> 8 ) & 0xff );  }
inline int LO_BYTE( USHORT n )  { return (int)( n & 0xff ); }


//-----------------------------------------------------------------------
// Footnotes
//

static Writer& OutW4W_SwFtn( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtFtn& rFtnFmt = (const SwFmtFtn&)rHt;
    const SwTxtFtn& rFtn = *rFtnFmt.GetTxtFtn();
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;

    do {    // middle-check-loop
        if( !rFtn.GetStartNode() )
            break;          // es gibt keine Kopf-/Fusszeile/Fussnote

        // Hole vom Node und vom letzten Node die Position in der Section
        SwNode* pSttNd = rWrt.pDoc->GetNodes()[ *rFtn.GetStartNode() ];
        ULONG  nEnd = pSttNd->EndOfSectionIndex(),
               nStart = rFtn.GetStartNode()->GetIndex()+1;

        // kein Bereich also kein gueltiger Node
        if( nStart >= nEnd )
            break;

        static sal_Char __FAR_DATA cFNT[] = "FNT00\x1f";		// FootNote
        if ( rW4WWrt.pDoc->GetFtnInfo().ePos != FTNPOS_PAGE )
            cFNT[ 4 ] = '2';			// EndNote

        rWrt.Strm() << sW4W_RECBEGIN << cFNT;

        rWrt.OutHex( HI_BYTE(rFtn.GetFtn().GetNumber()) ) << cW4W_TXTERM;
        rWrt.OutHex( LO_BYTE(rFtn.GetFtn().GetNumber()) ) << cW4W_TXTERM;
        rWrt.Strm() << "00" << cW4W_TXTERM << "01" << cW4W_TXTERM << // gemogelt: # of lines
                "00" << cW4W_TXTERM << "01" << cW4W_TXTERM <<     	// Old lines / lines in page
                "01" << cW4W_TXTERM << "FF" << cW4W_TXTERM <<		// const ( muss so sein )
                "13" << cW4W_TXTERM << "104" << cW4W_TXTERM <<   	// gemogelte Raender
                "8D" << sW4W_TERMEND;				// const

        W4WSaveData aSaveData( rW4WWrt, nStart, nEnd );
        rW4WWrt.pCurPam->GetMark()->nNode++;
        rW4WWrt.Out_SwDoc( rW4WWrt.pCurPam, FALSE );

        rWrt.Strm() << sW4W_RECBEGIN << "EFN" << cW4W_RED;

    } while( FALSE );
    return rWrt;
}


//-----------------------------------------------------------------------
// PageDescs
//

void OutW4W_SwFmtPageDesc1( SwW4WWriter& rW4WWrt, const SwPageDesc* pPg )
{
    if ( rW4WWrt.bStyleDef      // Was soll denn dieser Schwachsinn ???
                                // Ignorieren, sonst verschluckt sich W4W
         || rW4WWrt.bHdFt )     // hier ignorieren ( mag WW2 nicht )
        return;

    if ((( rW4WWrt.GetFilter() != 1 )	// Keiner (?) will ^L im Ascii-Text
        || (rW4WWrt.GetIniFlags() & 0x10000))
        && rW4WWrt.bIsTxtInPgDesc )
        rW4WWrt.Strm() << sW4W_RECBEGIN << "HNP" << cW4W_RED;

//    if ( rW4WWrt.pPgDsc2 != pPg )
      {
        rW4WWrt.pPgDsc2 = pPg;
        const SwFrmFmt *pFrm = &(pPg->GetMaster()); //!!! GetLeft()
#if OSL_DEBUG_LEVEL > 1
        rW4WWrt.Strm() << sW4W_RECBEGIN <<  "NOP_Pagedesc_Inhalt" << sW4W_TERMEND;
#endif
        BOOL bOldPg = rW4WWrt.bPageDesc;
        rW4WWrt.bPageDesc = TRUE;

        OutW4W_SwFmt( rW4WWrt, *pFrm );

        rW4WWrt.bPageDesc = bOldPg;
#if OSL_DEBUG_LEVEL > 1
        rW4WWrt.Strm() << sW4W_RECBEGIN << "NOP_Pagedesc_Inhalt_Ende" << sW4W_TERMEND ;//!!!
#endif
    }
    rW4WWrt.bIsTxtInPgDesc = FALSE;
    return;
}

static Writer& OutW4W_SwFmtPageDesc( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwPageDesc* pPg = ((SwFmtPageDesc&)rHt).GetPageDesc();

    if( pPg )				// pPg == 0 : So was kommt vor
        OutW4W_SwFmtPageDesc1( (SwW4WWriter&)rWrt, pPg );

    return rWrt;
}


static BOOL OutW4W_SwFmtHeader1( SwW4WWriter& rWrt, const SwFmtHeader& rHd,
                                const SwPageDesc * pPg, BOOL bFollow )
{
    BOOL bHasFollow = FALSE;

    if( !rHd.IsActive() )       // nicht aktiv, dann nichts weiter ausgeben
        return FALSE;

    // hole einen Node zu dem Request
        SwCntntNode *pNode = 0;
        const SwFmtCntnt& rCntnt = rHd.GetHeaderFmt()->GetCntnt();
        if( rCntnt.GetCntntIdx() )
            pNode = rWrt.pDoc->GetNodes()[ rCntnt.GetCntntIdx()->GetIndex()+ 1 ]->
                                GetCntntNode();
    do {    // middle-check-loop
        if( !pNode )
            break;          // es gibt keine Kopf-/Fusszeile/Fussnote

        // Hole vom Node und vom letzten Node die Position in der Section
        ULONG nStart = pNode->StartOfSectionIndex()+1,
              nEnd = pNode->EndOfSectionIndex();

        // kein Bereich also kein gueltiger Node
        if( nStart >= nEnd )
            break;
        ASSERT( pPg, "Header-Attribut ohne PageDesc" );

        BYTE nFirst = 3;    // Default: First and all Pages
        BYTE nDef = 0;
        // wird nicht die PageDesc-Tabelle ausgegeben und gibt es einen
        // Nachfolger, dann handelt es sich um die "1.Seite".

        if( !bFollow && pPg->GetFollow() && pPg->GetFollow() != pPg ){
            bHasFollow = TRUE;
            nFirst = 1;
        }
        else if( !pPg->IsHeaderShared() )
        {
            if( pPg->GetLeftFmt() &&
                rHd.GetHeaderFmt() == pPg->GetLeftFmt() )
                nDef |= 0x10;   // Even Pages
            else if( pPg->GetRightFmt() &&
                    rHd.GetHeaderFmt() == pPg->GetRightFmt() )
                nDef |= 0x8;    // Odd Pages (Header1/Header2 ???)
        }
        else
            nDef |= 0x4;    // All Pages

        if ( nDef == 0 ) nDef |= 0x4;   // Default: odd + even pages
        if ( bHasFollow ) nDef |= 1;    // 1. Seite -> "Header 2"
        if ( bFollow ) nFirst = 2;      // 2 = Non-first page

        // hole erstmal die Ober-/Unterkanten vom Header/Footer
        const SvxULSpaceItem& rUL = rHd.GetHeaderFmt()->GetULSpace();
        const SvxLRSpaceItem& rLR = rHd.GetHeaderFmt()->GetLRSpace();
        const SvxLRSpaceItem& rPgLR = pPg->GetMaster().GetLRSpace();

        USHORT nNewLeft = rPgLR.GetLeft() + rLR.GetLeft();
        USHORT nNewRight = USHORT(pPg->GetMaster().GetFrmSize().GetWidth()
                            - rPgLR.GetRight() - rLR.GetRight());
        if ( nNewRight - nNewLeft < 288 )
            nNewRight = nNewLeft + 288 ;

        rWrt.Strm() << sW4W_RECBEGIN << "HF200" << cW4W_TXTERM
                            << '0' << cW4W_TXTERM;
        rWrt.OutULong( nNewLeft / 144 ) << cW4W_TXTERM;
        rWrt.OutULong( nNewRight / 144 )
                    << cW4W_TXTERM << '1', cW4W_TXTERM;
        rWrt.OutULong( rUL.GetUpper() / 240 ) << sW4W_TERMEND;

        BOOL bOldHdFt = rWrt.bHdFt;
        rWrt.bHdFt = TRUE;
        {
            W4WSaveData aSaveData( rWrt, nStart, nEnd );
            rWrt.Out_SwDoc( rWrt.pCurPam, FALSE );
        }
        rWrt.bHdFt = bOldHdFt;
        rWrt.Strm() << sW4W_RECBEGIN << "HFX2" << cW4W_TXTERM;	// # of Lines (hoffentlich egal)
        rWrt.OutHex( nDef )
                << cW4W_TXTERM       		// Definition Byte
                << '0' << cW4W_TXTERM;		// Auto Height
        rWrt.OutULong( nFirst ) << sW4W_TERMEND; // First/NonFirst/All

    } while( FALSE );
    return bHasFollow;
}

// Header-Footer werden auch vom -Writer direkt gerufen, also kein static!

Writer& OutW4W_SwFmtHeader( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    BOOL bFollow =  OutW4W_SwFmtHeader1( rW4WWrt, (const SwFmtHeader&) rHt,
                                        rW4WWrt.pPgDsc2, FALSE );
    if ( bFollow )
        OutW4W_SwFmtHeader1( rW4WWrt,
         rW4WWrt.pPgDsc2->GetFollow()->GetMaster().GetHeader(), //!!! GetLeft()
                     rW4WWrt.pPgDsc2->GetFollow(), TRUE );
    return rWrt;
}

static BOOL OutW4W_SwFmtFooter1( SwW4WWriter& rWrt, const SwFmtFooter& rHd,
                                const SwPageDesc * pPg, BOOL bFollow )
{
    BOOL bHasFollow = FALSE;

    if( !rHd.IsActive() )       // nicht aktiv, dann nichts weiter ausgeben
        return FALSE;

    // hole einen Node zu dem Request
        SwCntntNode *pNode = 0;
        const SwFmtCntnt& rCntnt = rHd.GetFooterFmt()->GetCntnt();
        if( rCntnt.GetCntntIdx() )
            pNode = rWrt.pDoc->GetNodes()[ rCntnt.GetCntntIdx()->GetIndex()+ 1 ]->
                                GetCntntNode();
    do {    // middle-check-loop
        if( !pNode )
            break;          // es gibt keine Kopf-/Fusszeile/Fussnote

        // Hole vom Node und vom letzten Node die Position in der Section
        ULONG nStart = pNode->StartOfSectionIndex()+1,
                nEnd = pNode->EndOfSectionIndex();

        // kein Bereich also kein gueltiger Node
        if( nStart >= nEnd )
            break;
        ASSERT( pPg, "Footer-Attribut ohne PageDesc" );

        BYTE nFirst = 3;    // Default: First and all Pages
        BYTE nDef = 2;      // Default: Footer
        // wird nicht die PageDesc-Tabelle ausgegeben und gibt es einen
        // Nachfolger, dann handelt es sich um die "1.Seite" nach RTF.

        if( !bFollow && pPg->GetFollow() && pPg->GetFollow() != pPg ){
            bHasFollow = TRUE;
            nFirst = 1;
        }
        else if( !pPg->IsFooterShared() )
        {
            if( pPg->GetLeftFmt() &&
                rHd.GetFooterFmt() == pPg->GetLeftFmt() )
                nDef |= 0x10;   // Even Pages
            else if( pPg->GetRightFmt() &&
                    rHd.GetFooterFmt() == pPg->GetRightFmt() )
                nDef |= 0x8;    // Odd Pages (Footer1/Footer2 ???)
        }
        else
            nDef |= 0x4;    // All Pages

        if ( nDef == 2 ) nDef |= 0x4;   // Default: odd + even pages
        if ( bHasFollow ) nDef |= 1;    // 1. Seite -> "Footer 2"
        if ( bFollow ) nFirst = 2;      // 2 = Non-first page

        // hole erstmal die Ober-/Unterkanten vom Header/Footer
        const SvxULSpaceItem& rUL = rHd.GetFooterFmt()->GetULSpace();
        const SvxLRSpaceItem& rLR = rHd.GetFooterFmt()->GetLRSpace();
        const SvxLRSpaceItem& rPgLR = pPg->GetMaster().GetLRSpace();

        USHORT nNewLeft = rPgLR.GetLeft() + rLR.GetLeft();
        USHORT nNewRight = USHORT(pPg->GetMaster().GetFrmSize().GetWidth()
                            - rPgLR.GetRight() - rLR.GetRight());
        if ( nNewRight - nNewLeft < 288 )
            nNewRight = nNewLeft + 288 ;

        rWrt.Strm() << sW4W_RECBEGIN << "HF100" << cW4W_TXTERM
                            << '0', cW4W_TXTERM;
        rWrt.OutULong( nNewLeft / 144 ) << cW4W_TXTERM;
        rWrt.OutULong( nNewRight / 144 )
                    << cW4W_TXTERM << '1' << cW4W_TXTERM;
        rWrt.OutULong( 55 - rUL.GetUpper() / 240 ) << sW4W_TERMEND;

        BOOL bOldHdFt = rWrt.bHdFt;
        rWrt.bHdFt = TRUE;
        {
            W4WSaveData aSaveData( rWrt, nStart, nEnd );
            rWrt.Out_SwDoc( rWrt.pCurPam, FALSE );
        }
        rWrt.bHdFt = bOldHdFt;
        rWrt.Strm() << sW4W_RECBEGIN << "HFX2" << cW4W_TXTERM;	// # of Lines (hoffentlich egal)
        rWrt.OutHex( nDef )
                << cW4W_TXTERM        		// Definition Byte
                << '0' << cW4W_TXTERM;		// Auto Height
        rWrt.OutULong( nFirst ) << sW4W_TERMEND; // First/NonFirst/All

    } while( FALSE );
    return bHasFollow;
}

// Header-Footer werden auch vom -Writer direkt gerufen, also kein static!

Writer& OutW4W_SwFmtFooter( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    BOOL bFollow =  OutW4W_SwFmtFooter1( rW4WWrt, (const SwFmtFooter&) rHt,
                                        rW4WWrt.pPgDsc2, FALSE );
    if ( bFollow )
        OutW4W_SwFmtFooter1( rW4WWrt,
         rW4WWrt.pPgDsc2->GetFollow()->GetMaster().GetFooter(), //!!! GetLeft()
                     rW4WWrt.pPgDsc2->GetFollow(), TRUE );
    return rWrt;
}

//--------------------------------------------------------------------



static Writer& OutW4W_SwFmtBox( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    const SvxBoxItem& rBox = (const SvxBoxItem&)rHt;
    USHORT nBorder;

    rW4WWrt.OutW4W_GetBorder( &rBox, nBorder );

    rWrt.Strm() << sW4W_RECBEGIN << "PBC";
    rWrt.OutHex4( nBorder ) <<  sW4W_TERMEND;

    if ( rW4WWrt.bStyleDef || rW4WWrt.bStyleOnOff )
        rW4WWrt.GetStrm() << sW4W_RECBEGIN << "PBC0000" << sW4W_TERMEND;
    else
        rW4WWrt.GetNlStrm() << sW4W_RECBEGIN << "PBC0000" << sW4W_TERMEND;

    return rWrt;
}



Writer& OutW4W_SwChar( Writer& rWrt, sal_Unicode c, BOOL bRaw )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    if ( bRaw )
    {
        sal_Char cOut = ByteString::ConvertFromUnicode( c, rW4WWrt.eTargCharSet );
        if( !cOut )
            cOut = c;
        rWrt.Strm() << cOut;
        return rWrt;
    }

    do {        // block, damit aus diesem gesprungen werden kann !
        if( '\t' == c )
            rWrt.Strm() << sW4W_RECBEGIN << "TAB0" << sW4W_TERMEND;
        else if( '\n' == c )
            rWrt.Strm() << sW4W_RECBEGIN << "SNL" << cW4W_RED;
        else if( ' ' <= c && '~' >= c )
            rWrt.Strm() << (sal_Char)c;
        else if( CH_TXTATR_BREAKWORD == c || CH_TXTATR_INWORD == c )
            return rWrt;
        else
            break;
        return rWrt;
    } while( FALSE );

    rtl_TextEncoding eTargCSet = rW4WWrt.eTargCharSet;

    // der CodeSet fuer HEX / UCS
    sal_Char cPcCode = ByteString::ConvertFromUnicode( c, RTL_TEXTENCODING_IBM_850 );
    if( !cPcCode )
        cPcCode = '@';     /***** ?????? *******/

    sal_Char cTargCode = cPcCode;
    if( RTL_TEXTENCODING_IBM_850 != eTargCSet )
    {
        cTargCode = ByteString::ConvertFromUnicode( c, eTargCSet );
        if( !cTargCode )
            cTargCode = '@';     /***** ?????? *******/
    }

    if( '~' < cTargCode )
    {
        if( RTL_TEXTENCODING_MS_1252 == eTargCSet )
        {
            rWrt.Strm() << sW4W_RECBEGIN << "XCS819" << cW4W_TXTERM;	// ANSI
            rWrt.OutHex( cTargCode & 0xff ) << cW4W_RED;
        }
        else
        {
            rWrt.Strm() << sW4W_RECBEGIN << "XCS850" << cW4W_TXTERM;	// IBMPC850
            rWrt.OutHex( cPcCode & 0xff ) << cW4W_RED;
        }
        rWrt.Strm() << sW4W_RECBEGIN << "UCS";
        rWrt.OutHex( cPcCode & 0xff ) << sW4W_TERMEND;
    }
    else if( ' ' > cPcCode  )
    {
        rWrt.Strm() << sW4W_RECBEGIN << "HEX";
        rWrt.OutHex( cPcCode & 0xff ) << cW4W_RED;
    }
    else
        rWrt.Strm() << (sal_Char)cTargCode;

    return rWrt;
}


Writer& OutW4W_String( Writer& rWrt, const String& rStr )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;

    BOOL bRaw = 0x40000 & rW4WWrt.GetIniFlags();	// 1: no XCS in struct names
    for( xub_StrLen n = 0; n < rStr.Len(); ++n )
        OutW4W_SwChar( rWrt, rStr.GetChar( n ), bRaw );
    return rWrt;
}

static void CalcFontSize( SwW4WWriter & rW4WWrt, const SwFmt& rFmt,
                            USHORT& rFontId, USHORT& rFontHeight )
{
    UINT32 nFontHeight = rFmt.GetSize().GetHeight();
    rFontHeight = ( nFontHeight > USHRT_MAX ) ? USHRT_MAX : (USHORT)nFontHeight;
    rFontId = rW4WWrt.GetId( rFmt.GetFont() );
}

/*
 * Format wie folgt ausgeben:
 *      - gebe die Attribute aus; ohne Parents!
 */

Writer& OutW4W_SwFmt( Writer& rWrt, const SwFmt& rFmt )
{
    SwW4WWriter & rW4WWrt = (SwW4WWriter&)rWrt;

    switch( rFmt.Which() )
    {
    case RES_CONDTXTFMTCOLL:
    case RES_TXTFMTCOLL:
        {
//          if( rW4WWrt.bAttrOnOff )
            {
                USHORT nFontHeight, nFontId;
                CalcFontSize( rW4WWrt, rFmt, nFontId, nFontHeight );

                // dann gebe mal das "W4W"-FontAttribut aus
                rW4WWrt.Strm() << sW4W_RECBEGIN << "SPF10" << cW4W_TXTERM
                        << '0' << cW4W_TXTERM << "10" << cW4W_TXTERM;
                rWrt.OutULong( nFontId ) << cW4W_TXTERM;
                rWrt.OutULong( nFontHeight ) << cW4W_TXTERM;
                OutW4W_String( rWrt, rW4WWrt.GetFont( nFontId ).
                            GetFamilyName() ).Strm() << sW4W_TERMEND;
            }

            // akt. Collection-Pointer am Writer setzen
            rW4WWrt.pNdFmt = &rFmt;

            const SfxItemSet& rSet = rFmt.GetAttrSet();
            if( rSet.Count() )
            {
                const SfxItemPool& rPool = *rSet.GetPool();
                const SfxPoolItem* pItem;
                SfxWhichIter aIter( rSet );
                register USHORT nWhich = aIter.FirstWhich();
                while( nWhich )
                {
                    switch( nWhich )
                    {
                    case RES_CHRATR_FONT:
                    case RES_CHRATR_PROPORTIONALFONTSIZE:
                    case RES_CHRATR_FONTSIZE:
                        break;
                    default:
                        if( SFX_ITEM_SET == rSet.GetItemState( nWhich,
                                                    TRUE, &pItem ) &&
                            *pItem != rPool.GetDefaultItem( nWhich ) )
                            Out( aW4WAttrFnTab, *pItem, rWrt );
                    }
                    nWhich = aIter.NextWhich();
                }
            }
        }
        break;

    default:
        Out_SfxItemSet( aW4WAttrFnTab, rWrt, rFmt.GetAttrSet(), TRUE );
        break;
    }

    return rWrt;
}



/* Ausgabe der Nodes */

/*
 *  Ausgabe der Texte-Attribute:
 *      Die Text-Attribute sind in einem VarArray nach ihren Start-Positionen
 *      sortiert. Fuer den W4W-Writer ist aber auch das Ende von Bedeutung.
 *      Darum die Idee, sobald im SwpHints-Array ein Attribut mit der Start-
 *      Position gefunden wurde, in einem Sortierten-Array die Endposition
 *      zu speichern. Sobald der Writer die Position erreicht hat, wird die
 *      schliessende Klammer ausgegeben und die Position aus dem Sort.Array
 *      geloescht.
 */

struct W4WSttEndPos
{
    USHORT nFontSize, nFontId;
    const SwTxtAttr* pTxtAttr;
    W4WSttEndPos( const SwTxtAttr* pTxtAt, USHORT nFSize, USHORT nFId );
    W4WSttEndPos( const W4WSttEndPos & rSEPos );
    BOOL operator<( const W4WSttEndPos & rSEPos );
};

W4WSttEndPos::W4WSttEndPos( const SwTxtAttr* pTxtAt, USHORT nFSize, USHORT nFId )
    : pTxtAttr(pTxtAt), nFontSize(nFSize), nFontId( nFId )
{}

W4WSttEndPos::W4WSttEndPos( const W4WSttEndPos & rSEPos )
    : pTxtAttr( rSEPos.pTxtAttr), nFontSize( rSEPos.nFontSize ),
    nFontId( rSEPos.nFontId )
{}

BOOL W4WSttEndPos::operator<( const W4WSttEndPos & rSEPos )
{
    USHORT nEnd1, nEnd2;
    if( ( nEnd1 = *pTxtAttr->GetEnd() ) < ( nEnd2 = *rSEPos.pTxtAttr->GetEnd() ) )
        return TRUE;
    if( nEnd1 == nEnd2 )
        return *pTxtAttr->GetStart() > *rSEPos.pTxtAttr->GetStart();
    return FALSE;
}

SV_DECL_VARARR( _W4WEndPosLst, W4WSttEndPos, 5, 5 )
SV_IMPL_VARARR( _W4WEndPosLst, W4WSttEndPos )

class W4WEndPosLst : private _W4WEndPosLst
{
public:
    W4WEndPosLst() : _W4WEndPosLst( 5 ) {}

    BOOL Insert( W4WSttEndPos & aNew );
    void Delete( USHORT n = 1 )
        { _W4WEndPosLst::Remove( 0, n ); }
    W4WSttEndPos operator[]( USHORT nPos )
        { return _W4WEndPosLst::operator[]( nPos ); }
    USHORT Count()
        { return _W4WEndPosLst::Count(); }
    void OutAttr( SwW4WWriter & rWrt, USHORT nStrPos, USHORT nEnde );
};

BOOL W4WEndPosLst::Insert( W4WSttEndPos & aNew )
{
    USHORT nPos;
    for( nPos = 0; nPos < Count(); nPos++ )
    {
        W4WSttEndPos aTmp = _W4WEndPosLst::operator[]( nPos );
        if( aNew < aTmp )
        {
            _W4WEndPosLst::Insert( aNew, nPos );
            return TRUE;
        }
    }
    _W4WEndPosLst::Insert( aNew, nPos );
    return TRUE;
}

void W4WEndPosLst::OutAttr( SwW4WWriter & rWrt, USHORT nStrPos, USHORT nEnde )
{
    rWrt.bAttrOnOff = FALSE;
    USHORT nWhch;
    while( 0 != Count() && ( nStrPos >= nEnde ||
          *operator[]( 0 ).pTxtAttr->GetEnd() == nStrPos ))
    {
        // das setzen der alten Attribut-Werte erfolgt nur durch die
        // entsprechenden Attribute !!
        const W4WSttEndPos& rSTPos = *GetData();
        if( ( nWhch = rSTPos.pTxtAttr->Which()) == RES_CHRATR_FONTSIZE ||
            nWhch == RES_CHRATR_PROPORTIONALFONTSIZE )
            rWrt.nFontHeight = rSTPos.nFontSize;
        else if( nWhch == RES_CHRATR_FONT )
            rWrt.nFontId = rSTPos.nFontId;
        Out( aW4WAttrFnTab, rSTPos.pTxtAttr->GetAttr(), rWrt );
        Delete();
    }
}


static Writer& OutW4W_SwTxtNode( Writer& rWrt, SwCntntNode & rNode )
{
    SwTxtNode * pNd = &((SwTxtNode&)rNode);
    SwW4WWriter & rW4WWrt = (SwW4WWriter&)rWrt;
    W4WEndPosLst aW4WEndPosLst;
    xub_StrLen nAttrPos = 0;
    xub_StrLen nStrPos = rW4WWrt.pCurPam->GetPoint()->nContent.GetIndex();

    const String& rStr = pNd->GetTxt();
    xub_StrLen nEnde = rStr.Len();
    if( rW4WWrt.pCurPam->GetPoint()->nNode == rW4WWrt.pCurPam->GetMark()->nNode )
        nEnde = rW4WWrt.pCurPam->GetMark()->nContent.GetIndex();

    BOOL bLastPara = ( !rW4WWrt.bLastPg
             && ( rW4WWrt.pCurPam->GetPoint()->nNode >= rW4WWrt.pNxtPg->nNode ) );

    // akt. Collection-Pointer am Writer setzen
    rW4WWrt.pNdFmt = &pNd->GetAnyFmtColl();

    // bestimme die aktuelle FontSize aus der Collection
    CalcFontSize( rW4WWrt, *rW4WWrt.pNdFmt, rW4WWrt.nFontId, rW4WWrt.nFontHeight );

    if( rW4WWrt.bWriteAll || rW4WWrt.bTable )
        rW4WWrt.Out_SwTxtColl_OnOff( (SwTxtFmtColl&)*rW4WWrt.pNdFmt,
                        pNd->GetpSwAttrSet(), TRUE );
                        // erste Zeile und nur Teilweise ausgeben

    // ist der aktuelle Absatz in einem freifliegenden Rahmen ? Dann
    // muessen noch die Attribute dafuer ausgegeben werden.
    if( rW4WWrt.pFlyFmt )
        Out_SfxItemSet( aW4WAttrFnTab, rW4WWrt, rW4WWrt.pFlyFmt->GetAttrSet(), TRUE );

    // erstmal den Start berichtigen. D.h. wird nur ein Teil vom Satz
    // ausgegeben, so muessen auch da die Attribute stimmen!!
    const SwTxtAttr* pHt = 0;
    USHORT nCntAttr = pNd->HasHints() ? pNd->GetSwpHints().Count() : 0;
    if( nCntAttr && nStrPos > *( pHt = pNd->GetSwpHints()[ 0 ] )->GetStart() )
    {
        // Ok, es gibt vorher Attribute, die ausgegeben werden muessen
        rW4WWrt.bTxtAttr = rW4WWrt.bAttrOnOff = TRUE;
        do {
            nAttrPos++;
            if( RES_TXTATR_FIELD == pHt->Which() )      // Felder nicht
                continue;                               // ausgeben

            if( pHt->GetEnd() )
                if( *pHt->GetEnd() != *pHt->GetStart() )
                {
                    W4WSttEndPos aW4WSttEndPos( pHt,rW4WWrt.nFontHeight,
                                                        rW4WWrt.nFontId );
                    aW4WEndPosLst.Insert( aW4WSttEndPos );
                }
            Out( aW4WAttrFnTab, pHt->GetAttr(), rW4WWrt );
        } while( nAttrPos < nCntAttr && nStrPos >
            *( pHt = pNd->GetSwpHints()[ nAttrPos ] )->GetStart() );
        rW4WWrt.bTxtAttr =  FALSE;
    }

    if( !rW4WWrt.bWriteAll && rW4WWrt.bFirstLine )
    {
        ASSERT( !&rWrt, "Teilausgabe der Dokuments nicht implementiert" );
// jetzt noch die Attribute aus dem Format setzen
//        rW4WWrt.bAttrOnOff = TRUE;
//        OutW4W_SwFmt( rW4WWrt, *rW4WWrt.ChrFmt() );
    }


    for( ; nStrPos <= nEnde; nStrPos++ )
    {
        rW4WWrt.bTxtAttr = TRUE;
        aW4WEndPosLst.OutAttr( rW4WWrt, nStrPos, nEnde );
        rW4WWrt.bAttrOnOff = TRUE;
        if( nAttrPos < nCntAttr && *pHt->GetStart() == nStrPos
            && nStrPos != nEnde )
        {
            do {
                if( pHt->GetEnd() )
                {
                    if( *pHt->GetEnd() != nStrPos )
                    {
                       W4WSttEndPos aW4WSttEndPos( pHt, rW4WWrt.nFontHeight,
                                                            rW4WWrt.nFontId );
                       aW4WEndPosLst.Insert( aW4WSttEndPos );
                       Out( aW4WAttrFnTab, pHt->GetAttr(), rW4WWrt );
                    }
                }
                else
                    Out( aW4WAttrFnTab, pHt->GetAttr(), rW4WWrt );
                nAttrPos++;
            } while( nAttrPos < nCntAttr && nStrPos ==
                *( pHt = pNd->GetSwpHints()[ nAttrPos ] )->GetStart() );
        }

        if ( bLastPara
             && ( nStrPos >= rW4WWrt.pNxtPg->nContent.GetIndex() ))
        {
            rW4WWrt.Strm() << sW4W_RECBEGIN << "SNP" << cW4W_RED;

            ASSERT( rW4WWrt.pIter->GetPageDesc(), "PageDesc weg");
            rW4WWrt.pPageDesc = rW4WWrt.pIter->GetPageDesc();
            rW4WWrt.pPgFrm = (SwFrmFmt*)&(rW4WWrt.pPgDsc2->GetMaster()); //!!! GetLeft()
            rW4WWrt.bEvenPage = !rW4WWrt.bEvenPage;

            rW4WWrt.pIter->NextPage();
            rW4WWrt.bLastPg = !(rW4WWrt.pIter->GetPosition( *rW4WWrt.pNxtPg ));

            bLastPara = ( !rW4WWrt.bLastPg &&
             (rW4WWrt.pCurPam->GetPoint()->nNode >= rW4WWrt.pNxtPg->nNode ));
        }
        if( nStrPos != nEnde )      // eigentliche Buchstaben ausgeben
            OutW4W_SwChar( rW4WWrt, rStr.GetChar( nStrPos ), FALSE );
        rW4WWrt.bTxtAttr = FALSE;
    }
    // wenn bis zum Ende vom Node, dann auch das AbsatzEnde ausgeben

    if( rW4WWrt.pCurPam->GetMark()->nNode.GetIndex() !=
        rW4WWrt.pCurPam->GetPoint()->nNode.GetIndex()
        || pNd->Len() == rW4WWrt.pCurPam->GetMark()->nContent.GetIndex()
        && !rW4WWrt.bTable ){

        rW4WWrt.Out_SwTxtColl_OnOff( (SwTxtFmtColl&)pNd->GetAnyFmtColl(),
                                        0, FALSE );	// STF

        rW4WWrt.Strm() << sW4W_RECBEGIN << "HNL" << cW4W_RED;

        if ( rW4WWrt.pPostNlStrm)
        {
            // gebe die Sachen aus, die hinter HNL
            // kommen muessen (RSP, SAF, SBF)
            rW4WWrt.pPostNlStrm->Seek( 0L );
            rW4WWrt.Strm() << *rW4WWrt.pPostNlStrm;
            DELETEZ( rW4WWrt.pPostNlStrm );
        }
    }
    else if( rW4WWrt.bTable )
        rW4WWrt.Out_SwTxtColl_OnOff( (SwTxtFmtColl&)pNd->GetAnyFmtColl(), 0, FALSE );

    rW4WWrt.bIsTxtInPgDesc = TRUE;
    return rW4WWrt;
}


static Writer& OutW4W_SwGrfNode( Writer& rWrt, SwCntntNode & rNode )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    rW4WWrt.OutW4W_Grf( rNode.GetGrfNode() );
//    rWrt.Strm() << sW4W_RECBEGIN << "HNL" << cW4W_RED;		//!!!

    return rWrt;
}

static Writer& OutW4W_SwOLENode( Writer& rWrt, SwCntntNode& /*rNode*/ )
{
    rWrt.Strm() << sW4W_RECBEGIN << "HNL" << cW4W_RED;
    return rWrt;
}


/* File CHRATR.HXX: */

static Writer& OutW4W_SwFont( Writer& rWrt, const SfxPoolItem& rHt )
{
    /* trage den Font in die Font-Liste vom Writer ein und gebe hier nur
     *  die entsprechende Nummer aus. Der Font wird spaeter ueber diese
     * Tabelle im W4W-Dokument referenziert.
     */
    const SvxFontItem& rFont = (const SvxFontItem&)rHt;
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    if( rW4WWrt.bAttrOnOff )
        rW4WWrt.nFontId = rW4WWrt.GetId( rFont );

    rWrt.Strm() << sW4W_RECBEGIN << "SPF10" << cW4W_TXTERM << '0' << cW4W_TXTERM
        << "10" << cW4W_TXTERM;
    rWrt.OutULong( rW4WWrt.nFontId ) << cW4W_TXTERM;
    rWrt.OutULong( rW4WWrt.nFontHeight ) << cW4W_TXTERM;
    OutW4W_String( rWrt, rW4WWrt.GetFont( rW4WWrt.nFontId ).GetFamilyName())
        .Strm()	<< sW4W_TERMEND;
    return rWrt;
}

static Writer& OutW4W_SwPosture( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( ITALIC_NONE != ((const SvxPostureItem&)rHt).GetPosture() )
    {
        SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
        if( !rW4WWrt.bTxtAttr || rW4WWrt.bAttrOnOff )
            OUTRECORD( rWrt.Strm(),  "ITO" );

        if( !rW4WWrt.bTxtAttr || !rW4WWrt.bAttrOnOff )
            OUTRECORD( rW4WWrt.GetStrm( !rW4WWrt.bTxtAttr), "ITF" );
    }
    return rWrt;
}


static Writer& OutW4W_SwWeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( WEIGHT_BOLD == ((const SvxWeightItem&)rHt).GetWeight() )
    {
        SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
        if( !rW4WWrt.bTxtAttr || rW4WWrt.bAttrOnOff )
            OUTRECORD( rWrt.Strm(),  "BBT" );

        if( !rW4WWrt.bTxtAttr || !rW4WWrt.bAttrOnOff )
            OUTRECORD( rW4WWrt.GetStrm( !rW4WWrt.bTxtAttr), "EBT" );
    }
    return rWrt;
}


static Writer& OutW4W_SwShadowed( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( ((const SvxShadowedItem&)rHt).GetValue() )
    {
        SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
        if( !rW4WWrt.bTxtAttr || rW4WWrt.bAttrOnOff )
            OUTRECORD( rWrt.Strm(),  "BSP" );

        if( !rW4WWrt.bTxtAttr || !rW4WWrt.bAttrOnOff )
            OUTRECORD( rW4WWrt.GetStrm( !rW4WWrt.bTxtAttr ),  "ESP" );
    }
    return rWrt;
}


static Writer& OutW4W_SwContour( Writer& rWrt, const SfxPoolItem& /*rHt*/ )
{
    return rWrt;
}


static Writer& OutW4W_SwKerning( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxKerningItem& rKern = (const SvxKerningItem&)rHt;

    long nDist = 0;
    long nPos = 1;

    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    if( !rKern.GetValue() )
    {
        OUTRECORD( ((SwW4WWriter&)rWrt).Strm(),  "EKR" );
        return rWrt;
    }

#ifdef JP_NEWCORE
    // JP 23.09.94, wie nun ?????

    Fraction aTmp( 20, 1 );
    aTmp += rKern.frKern;
    nDist = (long)aTmp; // in Twips umrechnen
    if( nDist < 0 )
    {
        nDist = -nDist;
        nPos = 0;
    }
#endif

    if( !rW4WWrt.bTxtAttr || rW4WWrt.bAttrOnOff )
    {
        rWrt.Strm() << sW4W_RECBEGIN << "KER";
        rWrt.OutLong( nPos ) << cW4W_TXTERM;
        rWrt.OutLong( nDist ) << sW4W_TERMEND;
    }
    if( !rW4WWrt.bTxtAttr || !rW4WWrt.bAttrOnOff )
        OUTRECORD( rW4WWrt.GetStrm( !rW4WWrt.bTxtAttr ),  "EKR" );

    return rWrt;
}


static Writer& OutW4W_SwCrossedOut( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxCrossedOutItem& rCrossOut = (const SvxCrossedOutItem&)rHt;
    if( STRIKEOUT_SINGLE == rCrossOut.GetStrikeout() )
    {
        SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
        if( !rW4WWrt.bTxtAttr || rW4WWrt.bAttrOnOff )
        {
            rWrt.Strm() << sW4W_RECBEGIN << "BSO";
            OutW4W_SwChar( rWrt, rCrossOut.GetStrikeout(), TRUE ).Strm()
                << sW4W_TERMEND;
        }

        if( !rW4WWrt.bTxtAttr || !rW4WWrt.bAttrOnOff )
            OUTRECORD( rW4WWrt.GetStrm( !rW4WWrt.bTxtAttr ),  "ESO" );
    }
    return rWrt;
}


static Writer& OutW4W_SwCaseMap( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    if( SVX_CASEMAP_KAPITAELCHEN == ((const SvxCaseMapItem&)rHt).GetValue() )
    {
        if( !rW4WWrt.bTxtAttr || rW4WWrt.bAttrOnOff )
            OUTRECORD( rWrt.Strm(),  "BCS" );
        if( !rW4WWrt.bTxtAttr || !rW4WWrt.bAttrOnOff )
            OUTRECORD( rW4WWrt.GetStrm( !rW4WWrt.bTxtAttr ),  "ECS" );
    }
    else if( SVX_CASEMAP_VERSALIEN == ((const SvxCaseMapItem&)rHt).GetValue() )
    {
        if( !rW4WWrt.bTxtAttr || rW4WWrt.bAttrOnOff )
            OUTRECORD( rWrt.Strm(),  "BCU" );
        if( !rW4WWrt.bTxtAttr || !rW4WWrt.bAttrOnOff )
            OUTRECORD( rW4WWrt.GetStrm( !rW4WWrt.bTxtAttr ),  "ECU" );
    }
    return rWrt;
}


static Writer& OutW4W_SwUnderline( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxUnderlineItem & rULine = (const SvxUnderlineItem&)rHt;
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    const sal_Char *pSttStr, *pEndStr;
    if( UNDERLINE_SINGLE == rULine.GetUnderline() )
    {
        pSttStr = "BUL";
        pEndStr = "EUL";

//  ??? if( rULine.bUnderlineSpaces )
    }
    else if( UNDERLINE_DOUBLE == rULine.GetUnderline() )
    {
        pSttStr = "BDU";
        pEndStr = "EDU";
    }
    else if( UNDERLINE_NONE == rULine.GetUnderline() )
    {
        if( !rW4WWrt.bTxtAttr || !rW4WWrt.bAttrOnOff )
            OUTRECORD( rW4WWrt.GetStrm( !rW4WWrt.bTxtAttr ), "EUL" );
        return rWrt;
    }
    else
        return rWrt;

    if( !rW4WWrt.bTxtAttr || rW4WWrt.bAttrOnOff )
        OUTRECORD( rWrt.Strm(), pSttStr );

    if( !rW4WWrt.bTxtAttr || !rW4WWrt.bAttrOnOff )
        OUTRECORD( rW4WWrt.GetStrm( !rW4WWrt.bTxtAttr ), pEndStr );

    return rWrt;
}


static Writer& OutW4W_SwLanguage( Writer& rWrt, const SfxPoolItem& rHt )
{
    // die Werte des Enums sind mit den Werten fuer W4W identisch
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    if( !rW4WWrt.bTxtAttr || rW4WWrt.bAttrOnOff )
    {
        USHORT nLang = ((const SvxLanguageItem&)rHt).GetValue();
        if( LANGUAGE_DONTKNOW == nLang )
            nLang = 0;
        rWrt.Strm() << sW4W_RECBEGIN << "SLG";
        rWrt.OutULong( nLang ) << sW4W_TERMEND;
    }
    if( !rW4WWrt.bTxtAttr || !rW4WWrt.bAttrOnOff )
    {
        rWrt.Strm() << sW4W_RECBEGIN << "SLG";
        rWrt.OutULong( LANGUAGE_DONTKNOW == rW4WWrt.nDefLanguage
                        ? 0 : rW4WWrt.nDefLanguage ) << sW4W_TERMEND;
    }
    return rWrt;
}


static Writer& OutW4W_SwEscapement( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxEscapementItem & rEsc = (const SvxEscapementItem&)rHt;
    const sal_Char *pSttStr, *pEndStr;

    if( 0 < rEsc.GetEsc() )
    {
        pSttStr = "SPS";
        pEndStr = "EPS";
    }
    else if( 0 > rEsc.GetEsc() )
    {
        pSttStr = "SBS";
        pEndStr = "EBS";
    }
    else
        return rWrt;

    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    if( !rW4WWrt.bTxtAttr || rW4WWrt.bAttrOnOff )
        OUTRECORD( rWrt.Strm(), pSttStr );

    if( !rW4WWrt.bTxtAttr || !rW4WWrt.bAttrOnOff )
        OUTRECORD( rW4WWrt.GetStrm( !rW4WWrt.bTxtAttr ), pEndStr );

    return rWrt;
}


static Writer& OutW4W_SwSize( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxFontHeightItem& rSize = (const SvxFontHeightItem&)rHt;
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;

    if( rW4WWrt.bAttrOnOff )
        rW4WWrt.nFontHeight = (USHORT)rSize.GetHeight();

    rWrt.Strm() << sW4W_RECBEGIN << "SPF10" << cW4W_TXTERM << '0' << cW4W_TXTERM
        << "10" << cW4W_TXTERM;
    rWrt.OutULong( rW4WWrt.nFontId ) << cW4W_TXTERM;
    rWrt.OutULong( rW4WWrt.nFontHeight) << cW4W_TXTERM;
    OutW4W_String( rWrt, rW4WWrt.GetFont( rW4WWrt.nFontId ).GetFamilyName() )
        .Strm() << sW4W_TERMEND;
    return rWrt;
}


static Writer& OutW4W_SwColor( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    if( rW4WWrt.bStyleDef && ( rW4WWrt.GetFilter() == 44 || rW4WWrt.GetFilter() == 49 ) )
        return rWrt;    //!! umpopel W4W-Bug: Farbe in Style -> Farbe immer


    if( !rW4WWrt.bTxtAttr || !rW4WWrt.bAttrOnOff )
        OUTRECORD( rW4WWrt.GetStrm( !rW4WWrt.bTxtAttr ),  "ECL" );

    if( !(!rW4WWrt.bTxtAttr || rW4WWrt.bAttrOnOff ))
        return rWrt;

    const SvxColorItem& rCol = (const SvxColorItem&)rHt;
    USHORT nCol = 0;
    if( rCol.GetValue().GetRed() )
        nCol |= rCol.GetValue().GetRed() & 0x80   ? 0x0C : 0x04;
    if( rCol.GetValue().GetGreen())
        nCol |= rCol.GetValue().GetGreen() & 0x80 ? 0x0A : 0x02;
    if( rCol.GetValue().GetBlue() )
        nCol |= rCol.GetValue().GetBlue() & 0x80  ? 0x09 : 0x01;

//  laut Spec hex, laut output dec ??   rWrt.OutHex( nCol )
    rWrt.Strm() << sW4W_RECBEGIN << "BCL";
    rWrt.OutULong( nCol) << cW4W_TXTERM;
    rWrt.OutULong( rCol.GetValue().GetRed() ) << cW4W_TXTERM;
    rWrt.OutULong( rCol.GetValue().GetGreen() ) << cW4W_TXTERM;
    rWrt.OutULong( rCol.GetValue().GetBlue() ) << sW4W_TERMEND;

    return rWrt;
}


static Writer& OutW4W_SwField( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwField* pFld = ((SwFmtFld&)rHt).GetFld();

    switch( pFld->GetTyp()->Which() )
    {
    case RES_PAGENUMBERFLD: rWrt.Strm() << sW4W_RECBEGIN << "PPN";
                            break;
    case RES_DOCSTATFLD:    rWrt.Strm() << sW4W_RECBEGIN << "PLN";
                            break;
    default:                // nicht unterstuetztes Feld
#if OSL_DEBUG_LEVEL > 1
                            rWrt.Strm() << sW4W_RECBEGIN
                                        << "NOP_Unsupported_Field" << cW4W_RED;
                            break;
#else
                            return rWrt;    // Nix ausgeben
#endif
    }
    rWrt.Strm() << cW4W_RED;
    return rWrt;
}







/* File FRMATR.HXX */

static Writer& OutW4W_SwFrmSize( Writer& rWrt, const SfxPoolItem& /*rHt*/ )
{
    return rWrt;
}


static Writer& OutW4W_SwFillOrder( Writer& rWrt, const SfxPoolItem& /*rHt*/ )
{
    return rWrt;
}


static Writer& OutW4W_SwBreak( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter & rW4WWrt = (SwW4WWriter&)rWrt;

    if ( rW4WWrt.bHdFt )   // hier ignorieren ( mag WW2 nicht )
        return rWrt;
    const SvxFmtBreakItem &rBreak = (const SvxFmtBreakItem&)rHt;

#ifdef JP_NEWCORE
    was ist mit Column-Break ???
    - ist das 1. If ohne Block so richtig ???
        der String No_PageBreak wird immer ausgegeben !!!!
#endif

    if( SVX_BREAK_NONE != rBreak.GetBreak() )

    if ( ( rW4WWrt.GetFilter() != 1 )	// Keiner (?) will ^L im Ascii-Text
        || (rW4WWrt.GetIniFlags() & 0x10000))
        OUTRECORD( rWrt.Strm(), "HNP" );

        rWrt.Strm() << sW4W_RECBEGIN << "NOP_PageBreak" << sW4W_TERMEND;//!!!
        //!!! PAGEBREAK_AFTER stimmt nicht

#if 0
    const SwFmtPageBreak & rBreak = (const SwFmtPageBreak&)rHt;
    if ( rBreak.IsAuto() )
    {
        rWrt << "\\page";
    }
    else
        switch( rBreak.GetPageBreak() )
        {
        case PAGEBREAK_NONE:        break;
        case PAGEBREAK_BEFORE:      rWrt << "\\pagebb"; break;
        case PAGEBREAK_AFTER:       // rWrt << "\*pageba"; break;
        case PAGEBREAK_BOTH:        break;
        }
#endif
    return rWrt;
}

static Writer& OutW4W_SwPaperBin( Writer& rWrt, const SfxPoolItem& /*rHt*/ )
{
    return rWrt;
}


static Writer& OutW4W_SwFmtLRSpace( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter & rW4WWrt = (SwW4WWriter&)rWrt;

    // hole erstmal die Seitenangaben
//  const SwFrmFmt &rFmtPage = rW4WWrt.pDoc->GetPageDesc(0).GetMaster();
    const SwFrmFmt* pFmtPage = rW4WWrt.pPgFrm;
    const SvxLRSpaceItem& rPageLR = pFmtPage->GetLRSpace();
    const SwFmtFrmSize& rSz = pFmtPage->GetFrmSize();
    const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)rHt;

    USHORT nLeft = (USHORT)rLR.GetTxtLeft();
    USHORT n1st = nLeft + (USHORT)rLR.GetTxtFirstLineOfst();
    USHORT nRight = (USHORT)(rSz.GetWidth() - rLR.GetRight() );
    if ( !rW4WWrt.bPageDesc )
    {
        nLeft += (USHORT)rPageLR.GetLeft();
        n1st += (USHORT)rPageLR.GetLeft();
        nRight -= (USHORT)rPageLR.GetRight();
    }
#ifdef NIE          // Besser ist IPS statt RSM/STM
    rWrt.Strm() << W4W_RECBEGIN << "RSM0"  << W4W_TXTERM
            << '0' << W4W_TXTERM
            << String((long)Fraction( nLeft, 144 )).GetStr() << W4W_TXTERM
            << String((long)Fraction( nRight, 144 )).GetStr()
            << W4W_TXTERM
            << String(nLeft).GetStr() << W4W_TXTERM
            << String(nRight).GetStr() << W4W_TXTERM
            << W4W_RED;
#endif //NIE
                // Benutze IPS fuer links, 1.Zeile, rechts

    rWrt.Strm() << sW4W_RECBEGIN << "IPS";
    rWrt.OutLong( nLeft / 144 ) << cW4W_TXTERM;
    rWrt.OutLong( n1st / 144 ) << cW4W_TXTERM;
    rWrt.OutLong( nLeft ) << cW4W_TXTERM;
    rWrt.OutLong( n1st ) << cW4W_TXTERM;
    rWrt.OutLong( nRight ) << sW4W_TERMEND;

    return rWrt;
}

static Writer& OutW4W_SwFmtULSpace( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    if ( rW4WWrt.bPageDesc )	// SBF/SAF geht nur fuer Absaetze, fuer
        return rWrt;			// Pagedesc sollte STP/SBP sein oder ignorieren

    const SvxULSpaceItem & rUL = (const SvxULSpaceItem&)rHt;

    if( 0 != rUL.GetUpper() )
    {
        rWrt.Strm() << sW4W_RECBEGIN << "SBF";
        rWrt.OutLong( rUL.GetUpper() ) << cW4W_TXTERM
                << '0' << sW4W_TERMEND;	// WW2-Hack: Wenn nLines != 0 ist,
                                        // muessen nLines HNL1 folgen, sonst
                                    // wird HNL ignoriert. Es duerfen aber auch
                                // nicht zu viele HNL1 da sein, da sie sonst
                            // als HNL interpretiert werden. Deshalb behaupte
                        // ich, dass nLines = 0 statt nLines = GetUpper()/240.
                        // Das SBF duerfte daher bei zeilenorientierten
                        // Formaten verloren gehen.

        if ( rW4WWrt.bStyleDef || rW4WWrt.bStyleOnOff )
            rW4WWrt.GetStrm() << sW4W_RECBEGIN << "SBF" << '0' << cW4W_TXTERM
                    << '0' << sW4W_TERMEND;
        else
            rW4WWrt.GetNlStrm() << sW4W_RECBEGIN << "SBF" << '0' << cW4W_TXTERM
                    << '0' << sW4W_TERMEND;     // Wieder zurueckstellen ??
    }
    if( 0 != rUL.GetLower() )
    {
        rWrt.Strm() << sW4W_RECBEGIN << "SAF";
        rWrt.OutLong( rUL.GetLower() ) << cW4W_TXTERM
                << '0' << sW4W_TERMEND;     // WW2-Hack, siehe oben. Erschwerend
                                            // kommt hinzu, dass hier die HNL1 *vor*
                                            // dem HNL stehen muessen....

        if ( rW4WWrt.bStyleDef || rW4WWrt.bStyleOnOff )
            rW4WWrt.GetStrm() << sW4W_RECBEGIN << "SAF" << '0' << cW4W_TXTERM
                    << '0' << sW4W_TERMEND; // Wieder zurueckstellen ??
        else
            rW4WWrt.GetNlStrm() << sW4W_RECBEGIN << "SAF" << '0' << cW4W_TXTERM
                    << '0' << sW4W_TERMEND;
    }
    return rWrt;
}

static Writer& OutW4W_SwFmtPrint( Writer& rWrt, const SfxPoolItem& /*rHt*/ )
{
    return rWrt;
}

static Writer& OutW4W_SwFmtOpaque( Writer& rWrt, const SfxPoolItem& /*rHt*/ )
{
    return rWrt;
}

static Writer& OutW4W_SwFmtProtect( Writer& rWrt, const SfxPoolItem& /*rHt*/ )
{
    return rWrt;
}

static Writer& OutW4W_SwFmtSurround( Writer& rWrt, const SfxPoolItem& /*rHt*/ )
{
    return rWrt;
}

static Writer& OutW4W_SwFmtVertOrient( Writer& rWrt, const SfxPoolItem& /*rHt*/ )
{
    return rWrt;
}

static Writer& OutW4W_SwFmtHoriOrient( Writer& rWrt, const SfxPoolItem& /*rHt*/ )
{
    return rWrt;
}


/* File GRFATR.HXX */

static Writer& OutW4W_SwMirrorGrf( Writer& rWrt, const SfxPoolItem& /*rHt*/ )
{
    return rWrt;
}

static Writer& OutW4W_SwCropGrf( Writer& rWrt, const SfxPoolItem& /*rHt*/ )
{
    return rWrt;
}


/* File PARATR.HXX  */

static Writer& OutW4W_SwLineSpacing( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxLineSpacingItem &rLs = (const SvxLineSpacingItem&)rHt;
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;

    switch( rLs.GetLineSpaceRule() )
    {
    case SVX_LINE_SPACE_AUTO:
    case SVX_LINE_SPACE_FIX:
    case SVX_LINE_SPACE_MIN:
    {

        short nLsp = 0;

        switch( rLs.GetInterLineSpaceRule() )
        {
        case SVX_INTER_LINE_SPACE_FIX:  // Durchschuss
                nLsp = 240 + rLs.GetInterLineSpace();
                break;
        case SVX_INTER_LINE_SPACE_PROP: // 1.5, 2-zeilig, Prop
                {
                    long nTmp = 240;
                    nTmp *= rLs.GetPropLineSpace();
                    nLsp = (short)nTmp;
                }
                break;
        default:                    // z.B. Minimum
                nLsp = rLs.GetLineHeight();
                break;
        }
        if ( nLsp < 60 ) nLsp = 60; // keine sinnlosen Werte
        rW4WWrt.Strm() << sW4W_RECBEGIN
             << "RSP2" << cW4W_TXTERM;                    // old Spacing 1-zeilig
        rWrt.OutLong( ( nLsp + 60 ) / 120 ) << cW4W_TXTERM;	// new in 1/2 zeilen
        rWrt.OutLong( nLsp ) << sW4W_TERMEND;  // dito in Twips

        if ( rW4WWrt.bStyleDef || rW4WWrt.bStyleOnOff )
            rW4WWrt.GetStrm() << sW4W_RECBEGIN		// RSP per StylePostNlStrm
                << "RSP2" << cW4W_TXTERM				// wieder ausschalten
                << '2' << cW4W_TXTERM
                << "240" << sW4W_TERMEND;
        else
            rW4WWrt.GetNlStrm() << sW4W_RECBEGIN		// RSP per StylePostNlStrm
                << "RSP2" << cW4W_TXTERM				// wieder ausschalten
                << '2' << cW4W_TXTERM
                << "240" << sW4W_TERMEND;
    }
    break;
    }
    return rWrt;
}

static Writer& OutW4W_SwAdjust( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxAdjustItem & rAdjust = ((const SvxAdjustItem&)rHt);
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;

    switch( rAdjust.GetAdjust() )
    {
    case SVX_ADJUST_RIGHT:
        {
            rW4WWrt.Strm() << sW4W_RECBEGIN << "AFR10" << cW4W_TXTERM
                        << "10" << cW4W_TXTERM
                        << '0' << sW4W_TERMEND;
            OUTRECORD( rW4WWrt.GetStrm(), "EAT" );
        }
        break;
    case SVX_ADJUST_BLOCK:
    case SVX_ADJUST_BLOCKLINE:
        {
            OUTRECORD( rW4WWrt.Strm(), "BRJ" );
            OUTRECORD( rW4WWrt.GetStrm(), "ERJ" );
        }
        break;

    case SVX_ADJUST_CENTER:
        {
            rW4WWrt.Strm() << sW4W_RECBEGIN << "CTX0" << cW4W_TXTERM
                    << '0' << cW4W_TXTERM
                    << '0' << sW4W_TERMEND;
            OUTRECORD( rW4WWrt.GetStrm(), "ECT" );
        }
        break;
    }
    return rWrt;
}


static Writer& OutW4W_SwSplit( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    if( !((SvxFmtSplitItem&)rHt).GetValue() )
        OUTRECORD( rW4WWrt.GetStrm(), "KEP" );
    return rWrt;
}

static Writer& OutW4W_SwWidows( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    const SvxWidowsItem& rWid = (const SvxWidowsItem&)rHt;

    rW4WWrt.Strm() << sW4W_RECBEGIN << "WON";
    rWrt.OutLong( rWid.GetValue() ) << sW4W_TERMEND;
    OUTRECORD( rW4WWrt.GetStrm(), "WOY" );

    return rWrt;
}

static Writer& OutW4W_SwOrphans( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    const SvxOrphansItem& rOr = (const SvxOrphansItem&)rHt;

    rW4WWrt.Strm() << sW4W_RECBEGIN << "WON";
    rWrt.OutLong( rOr.GetValue() ) << sW4W_TERMEND;
    OUTRECORD( rW4WWrt.GetStrm(), "WOY" );
    return rWrt;
}


#define MAX_TAB_OVER 1000		// soviel twips ueberhaengen ist ok

// Wenn beim SWG Tabe bis zu 2 cm ueberhaengen, gelten sie trotzdem noch.
// Wenn bei WW2-Export die Tabs nur 1 Twip ueberhaengen, werden sie
// ignoriert. Das passiert innerhalb von Frames leicht. Deshalb wird hier
// gemogelt. Dass bei Flys die Page-Grenzen statt der Fly-Grenzen genommen
// werden, ist ok, da WW2 dann die Tabs immer noch anerkennt


static Writer& OutW4W_SwTabStop( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;

    const SvxTabStopItem & rTStops = (const SvxTabStopItem&)rHt;
//    long nLeftMargin = ( rW4WWrt).GetLeftMargin();
    SwTwips nLeftMargin, nWidth;
    rW4WWrt.GetMargins( nLeftMargin, nWidth );

    BYTE aTabPosArr[ 32 ];
    BYTE aTabTypesArr[ 20 ];
    BYTE aTabLeadingArr[ 40 ];

    memset( aTabPosArr, 0, 32 );
    memset( aTabTypesArr, 0, 20 );
    memset( aTabLeadingArr, 0, 40 );
        // Umsetzungtabelle fuer das Adjustment
    static BYTE __READONLY_DATA aAdjustArr[ SVX_TAB_ADJUST_DEFAULT - SVX_TAB_ADJUST_LEFT +1 ] = {
        /* SVX_TAB_ADJUST_LEFT      */      0,
        /* SVX_TAB_ADJUST_RIGHT     */      2,
        /* SVX_TAB_ADJUST_DECIMAL   */      3,
        /* SVX_TAB_ADJUST_CENTER    */      1,
        /* SVX_TAB_ADJUST_DEFAULT   */      0
             };

    USHORT n;
    for( n = 0; n < rTStops.Count() && n < 40; n++ )
    {
        const SvxTabStop & rTS = rTStops[ n ];           // in 1/10 Inch
        SwTwips nTabRel = rTS.GetTabPos();
        if ( nTabRel > nWidth + MAX_TAB_OVER )	// teste auf Sinn der Positionen
            continue;							// Tab out of SRange
        if ( nTabRel > nWidth )
            nTabRel = nWidth;					// Tab nur wenig out of SRange

        USHORT nSize =  (USHORT)( ((long)(nLeftMargin + nTabRel)) / 144 );
        if ( /*nSize < 0 ||*/ nSize / 8 >= 32 )     // Stuss-Werte -> ignorieren
            continue;
        aTabPosArr[ nSize / 8 ] |= ( 0x80 >> ( nSize % 8 ));
        aTabTypesArr[ n / 2 ] |= ( n & 1 ? aAdjustArr[ rTS.GetAdjustment() ] :
                                        aAdjustArr[ rTS.GetAdjustment() ] << 4 );
        if( cDfltFillChar != rTS.GetFill() )
            aTabLeadingArr[ n ] = rTS.GetFill();
    }

    rWrt.Strm() << sW4W_RECBEGIN << "NTB";
    for( n = 0; n < 32; ++n )
        rWrt.OutHex( aTabPosArr[ n ] ) << cW4W_TXTERM;
    for( n = 0; n < 20; ++n )
        rWrt.OutHex( aTabTypesArr[ n ] ) << cW4W_TXTERM;
    for( n = 0; n < 40; ++n )
        rWrt.OutHex( aTabLeadingArr[ n ] ) << cW4W_TXTERM;

    rWrt.OutLong( rTStops.Count() ) << cW4W_TXTERM;

    for( n = 0; n < rTStops.Count() && n < 40; n++ )
    {
        SwTwips nTabRel = rTStops[ n ].GetTabPos();
        if ( nTabRel > nWidth + MAX_TAB_OVER )	// teste auf Sinn der Positionen
            continue;							// Tab out of SRange
        if ( nTabRel > nWidth )
            nTabRel = nWidth;					// Tab nur wenig out of SRange
        rWrt.OutLong( nLeftMargin + nTabRel ) << cW4W_TXTERM;
    }

    rWrt.Strm() << cW4W_RED;
    return rWrt;
}

static Writer& OutW4W_SwHyphenZone( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwW4WWriter& rW4WWrt = (SwW4WWriter&)rWrt;
    const SvxHyphenZoneItem& rAttr = (const SvxHyphenZoneItem&)rHt;

    if ( rW4WWrt.GetFilter() == 48 ) 	//!!! Bug WP6.0-Filter
        return rWrt;                    // fuehrt zu Absturz WP6

    if ( !rAttr.IsHyphen() )
    {
        OUTRECORD( rW4WWrt.Strm(), "EHP" );
        return rWrt;
    }
    OUTRECORD( rW4WWrt.Strm(), "BHP" );
    OUTRECORD( rW4WWrt.GetStrm(), "EHP" );

                        // Nach Angabe der W4W-Doku gibt es auch den
                        // RHZ-Befehl, der sagt, wie getrennt werden soll.
                        // Bei WP6 scheint er aber zumindest
                        // beim Import nicht zu funktionieren.
                        // In WW2 gibt es diese Einstellmoeglichkeit
                        // anscheinend nicht.
    return rWrt;
}

/*
 * lege hier die Tabellen fuer die W4W-Funktions-Pointer auf
 * die Ausgabe-Funktionen an.
 * Es sind lokale Strukturen, die nur innerhalb der W4W-DLL
 * bekannt sein muessen.
 */

SwAttrFnTab aW4WAttrFnTab = {
/* RES_CHRATR_CASEMAP   */          OutW4W_SwCaseMap,
/* RES_CHRATR_CHARSETCOLOR */		0,
/* RES_CHRATR_COLOR */              OutW4W_SwColor,
/* RES_CHRATR_CONTOUR   */          OutW4W_SwContour,
/* RES_CHRATR_CROSSEDOUT    */      OutW4W_SwCrossedOut,
/* RES_CHRATR_ESCAPEMENT    */      OutW4W_SwEscapement,
/* RES_CHRATR_FONT  */              OutW4W_SwFont,
/* RES_CHRATR_FONTSIZE  */          OutW4W_SwSize,
/* RES_CHRATR_KERNING   */          OutW4W_SwKerning,
/* RES_CHRATR_LANGUAGE  */          OutW4W_SwLanguage,
/* RES_CHRATR_POSTURE   */          OutW4W_SwPosture,
/* RES_CHRATR_PROPORTIONALFONTSIZE*/0,
/* RES_CHRATR_SHADOWED  */          OutW4W_SwShadowed,
/* RES_CHRATR_UNDERLINE */          OutW4W_SwUnderline,
/* RES_CHRATR_WEIGHT    */          OutW4W_SwWeight,
/* RES_CHRATR_WORDLINEMODE   */     0,   // AMA 14.10.94: NEU
/* RES_CHRATR_AUTOKERN   */     	0,   // AMA 27.01.95: NEU
/* RES_CHRATR_BLINK	*/          	0,
/* RES_CHRATR_NOHYPHEN	*/          0, // Neu: nicht trennen
/* RES_CHRATR_NOLINEBREAK */        0, // Neu: nicht umbrechen
/* RES_CHRATR_BACKGROUND */        	0, // Neu: Zeichenhintergrund
/* RES_CHRATR_CJK_FONT */			0,
/* RES_CHRATR_CJK_FONTSIZE */		0,
/* RES_CHRATR_CJK_LANGUAGE */		0,
/* RES_CHRATR_CJK_POSTURE */		0,
/* RES_CHRATR_CJK_WEIGHT */			0,
/* RES_CHRATR_CTL_FONT */			0,
/* RES_CHRATR_CTL_FONTSIZE */		0,
/* RES_CHRATR_CTL_LANGUAGE */		0,
/* RES_CHRATR_CTL_POSTURE */		0,
/* RES_CHRATR_CTL_WEIGHT */			0,
/* RES_CHRATR_WRITING_DIRECTION */	0,
/* RES_CHRATR_DUMMY2 */				0,
/* RES_CHRATR_DUMMY3 */				0,
/* RES_CHRATR_DUMMY4 */				0,
/* RES_CHRATR_DUMMY5 */				0,
/* RES_CHRATR_DUMMY1 */        	    0, // Dummy:

/* RES_TXTATR_NOLINEBREAK */  		0,
/* RES_TXTATR_NOHYPHEN          */  0,
/* RES_TXTATR_REFMARK */            0, /* OutW4W_RefMark */
/* RES_TXTATR_TOXMARK   */          0, /* OutW4W_SwTxtTOXMark */
/* RES_TXTATR_CHARFMT   */          0,
/* RES_TXTATR_TWO_LINES */			0,
/* RES_TXTATR_CJK_RUBY */			0,
/* RES_TXTATR_UNKNOWN_CONTAINER */	0,
/* RES_TXTATR_DUMMY5 */				0,
/* RES_TXTATR_DUMMY6 */				0,

/* RES_TXTATR_FIELD */              OutW4W_SwField,
/* RES_TXTATR_FLYCNT    */          0, /* OutW4W_SwFlyCntnt */
/* RES_TXTATR_FTN       */          OutW4W_SwFtn,
/* RES_TXTATR_SOFTHYPH  */          0, /* OutW4W_SwSoftHyph */
/* RES_TXTATR_HARDBLANK */          0, /* OutW4W_HardBlank */
/* RES_TXTATR_DUMMY1 */        	    0, // Dummy:
/* RES_TXTATR_DUMMY2 */        	    0, // Dummy:

/* RES_PARATR_LINESPACING   */      OutW4W_SwLineSpacing,
/* RES_PARATR_ADJUST    */          OutW4W_SwAdjust,
/* RES_PARATR_SPLIT */              OutW4W_SwSplit,
/* RES_PARATR_WIDOWS    */          OutW4W_SwWidows,
/* RES_PARATR_ORPHANS   */          OutW4W_SwOrphans,
/* RES_PARATR_TABSTOP   */          OutW4W_SwTabStop,
/* RES_PARATR_HYPHENZONE*/          OutW4W_SwHyphenZone,
/* RES_PARATR_DROP */               0,
/* RES_PARATR_REGISTER */        	0, // neu:  Registerhaltigkeit
/* RES_PARATR_NUMRULE */        	0, // NumRule
/* RES_PARATR_SCRIPTSPACE */   	    0, // Dummy:
/* RES_PARATR_HANGINGPUNCTUATION */	0, // Dummy:
/* RES_PARATR_DUMMY1 */        	    0, // Dummy:
/* RES_PARATR_DUMMY2 */        	    0, // Dummy:
/* RES_PARATR_DUMMY3 */        	    0, // Dummy:
/* RES_PARATR_DUMMY4 */        	    0, // Dummy:
/* RES_PARATR_DUMMY5 */        	    0, // Dummy:
/* RES_PARATR_DUMMY6 */        	    0, // Dummy:
/* RES_PARATR_DUMMY7 */        	    0, // Dummy:
/* RES_PARATR_DUMMY8 */        	    0, // Dummy:

/* RES_FILL_ORDER   */              OutW4W_SwFillOrder,
/* RES_FRM_SIZE */                  OutW4W_SwFrmSize,
/* RES_PAPER_BIN   */              	OutW4W_SwPaperBin,
/* RES_LR_SPACE */                  OutW4W_SwFmtLRSpace,
/* RES_UL_SPACE */                  OutW4W_SwFmtULSpace,
/* RES_PAGEDESC */                  OutW4W_SwFmtPageDesc,
/* RES_BREAK */                		OutW4W_SwBreak,
/* RES_CNTNT */                     0, /* OutW4W_??? */
/* RES_HEADER */                    OutW4W_SwFmtHeader,
/* RES_FOOTER */                    OutW4W_SwFmtFooter,
/* RES_PRINT */                 	OutW4W_SwFmtPrint,
/* RES_OPAQUE */                	OutW4W_SwFmtOpaque,
/* RES_PROTECT */               	OutW4W_SwFmtProtect,
/* RES_SURROUND */             		OutW4W_SwFmtSurround,
/* RES_VERT_ORIENT */           	OutW4W_SwFmtVertOrient,
/* RES_HORI_ORIENT */           	OutW4W_SwFmtHoriOrient,
/* RES_ANCHOR */                	0, /* OutW4W_??? */
/* RES_BACKGROUND */                0, /* OutW4W_??? */
/* RES_BOX	*/                      OutW4W_SwFmtBox,
/* RES_SHADOW */                    0, /* OutW4W_??? */
/* RES_FRMMACRO */                  0, /* OutW4W_??? */
/* RES_COL */                       0, /* OutW4W_??? */
/* RES_KEEP */						0, /* OutW4W_??? */
/* RES_URL */        	    		0, // URL:
/* RES_EDIT_IN_READONLY */        	0,
/* RES_LAYOUT_SPLIT */ 	    		0,
/* RES_FRMATR_DUMMY1 */        	    0, // Dummy:
/* RES_FRMATR_DUMMY2 */        	    0, // Dummy:
/* RES_FRMATR_DUMMY3 */        	    0, // Dummy:
/* RES_FRMATR_DUMMY4 */        	    0, // Dummy:
/* RES_FRMATR_DUMMY5 */        	    0, // Dummy:
/* RES_FRMATR_DUMMY6 */        	    0, // Dummy:
/* RES_FRMATR_DUMMY7 */        	    0, // Dummy:
/* RES_FRMATR_DUMMY8 */        	    0, // Dummy:
/* RES_FRMATR_DUMMY9 */        	    0, // Dummy:

/* RES_GRFATR_MIRRORGRF */          OutW4W_SwMirrorGrf,
/* RES_GRFATR_CROPGRF   */          OutW4W_SwCropGrf,
/* RES_GRFATR_ROTATION */			0,
/* RES_GRFATR_LUMINANCE */			0,
/* RES_GRFATR_CONTRAST */			0,
/* RES_GRFATR_CHANNELR */			0,
/* RES_GRFATR_CHANNELG */			0,
/* RES_GRFATR_CHANNELB */			0,
/* RES_GRFATR_GAMMA */				0,
/* RES_GRFATR_INVERT */				0,
/* RES_GRFATR_TRANSPARENCY */		0,
/* RES_GRFATR_DRWAMODE */			0,
/* RES_GRFATR_DUMMY1 */				0,
/* RES_GRFATR_DUMMY2 */				0,
/* RES_GRFATR_DUMMY3 */				0,
/* RES_GRFATR_DUMMY4 */				0,
/* RES_GRFATR_DUMMY5 */				0,

/* RES_BOXATR_FORMAT */				0,
/* RES_BOXATR_FORMULA */			0,
/* RES_BOXATR_VALUE */				0,

/* RES_UNKNOWNATR_CONTAINER */		0
};

SwNodeFnTab aW4WNodeFnTab = {
/* RES_TXTNODE  */                   OutW4W_SwTxtNode,
/* RES_GRFNODE  */                   OutW4W_SwGrfNode,
/* RES_OLENODE  */                   OutW4W_SwOLENode
};


}
