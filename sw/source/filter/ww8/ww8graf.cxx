/*************************************************************************
 *
 *  $RCSfile: ww8graf.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: sj $ $Date: 2000-11-23 16:18:18 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include <math.h>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SDTAITM_HXX
#include <svx/sdtaitm.hxx>
#endif
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_SHORTS
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_XLINIIT_HXX //autogen
#include <svx/xlineit.hxx>
#endif
#ifndef _SVX_FILLITEM_HXX //autogen
#include <svx/xfillit.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SDTAITM_HXX //autogen
#include <svx/sdtaitm.hxx>
#endif
#ifndef _SVDCAPT_HXX //autogen
#include <svx/svdocapt.hxx>
#endif
#ifndef _SXCTITM_HXX //autogen
#include <svx/sxctitm.hxx>
#endif
#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#ifndef _SVDOCIRC_HXX //autogen
#include <svx/svdocirc.hxx>
#endif
#ifndef _OUTLOBJ_HXX //autogen
#include <svx/outlobj.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVDOGRAF_HXX
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_CNTRITEM_HXX
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _TOOLS_URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX
#include <svx/impgrf.hxx>
#endif
#ifndef _MSDFFIMP_HXX
#include <svx/msdffimp.hxx>
#endif
#ifndef _SVDOATTR_HXX
#include <svx/svdoattr.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_RECTENUM_HXX //autogen
#include <svx/rectenum.hxx>
#endif
#ifndef _SVX_XFLTRIT_HXX
#include <svx/xfltrit.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif

#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif

#ifndef _WW8STRUC_HXX
#include <ww8struc.hxx>
#endif
#ifndef _WW8SCAN_HXX
#include <ww8scan.hxx>
#endif
#ifndef _WW8PAR_HXX
#include <ww8par.hxx>           // class SwWWImplReader
#endif
#ifndef _WW8GRAF_HXX
#include <ww8graf.hxx>
#endif
#ifndef _WW8PAR2_HXX
#include <ww8par2.hxx>          // SwWW8StyInf
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>           // class SwCropGrf
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // Progress
#endif
#ifndef _FLTSHELL_HXX
#include <fltshell.hxx>
#endif
#ifndef _FMTCNCT_HXX
#include <fmtcnct.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::ucb;
using namespace ::rtl;

// Hilfsroutinen

// inline geht bei BLC leider nicht mit "for"
/*inline*/ Color WW8TransCol( SVBT32 nWC )
{
#if 1               // 1 = Vordefinierte Farben benutzen, 0 = ignorieren

    // Farbtabelle zum Umrechnen RGB-Werte in vordefinierte Farben
    // ( Damit bei der Writer-UI die Farbnamen stimmen )
    // Die Tabelle int im *3er-System* aufgeteilt. Die Grauwerte fehlen,
    // da sie nicht ins 3er-System passen ( 4 Werte: sw, ws, 2 * grau )
    static ColorData eColA[] = {                    //  B G R  B G R  B G R
        COL_BLACK, COL_RED, COL_LIGHTRED,           //  0 0 0, 0 0 1, 0 0 2
        COL_GREEN, COL_BROWN, COL_BLACK,            //  0 1 0, 0 1 1, 0 1 2
        COL_LIGHTGREEN, COL_BLACK, COL_YELLOW,      //  0 2 0, 0 2 1, 0 2 2
        COL_BLUE, COL_MAGENTA, COL_BLACK,           //  1 0 0, 1 0 1, 1 0 2
        COL_CYAN, COL_LIGHTGRAY, COL_BLACK,         //  1 1 0, 1 1 1, 1 1 2
        COL_BLACK, COL_BLACK, COL_BLACK,            //  1 2 0, 1 2 1, 1 2 2
        COL_LIGHTBLUE, COL_BLACK, COL_LIGHTMAGENTA, //  2 0 0, 2 0 1, 2 0 2
        COL_BLACK, COL_BLACK, COL_BLACK,            //  2 1 0, 2 1 1, 2 1 2
        COL_LIGHTCYAN, COL_BLACK, COL_WHITE };      //  2 2 0, 2 2 1, 2 2 2

    // In nWC[3] steht ein Byte, dass in der WW-Doku nicht beschrieben ist.
    // Die Bedeutung ist anscheinend folgende: Bei 0 ist es eine normale
    // Farbe, dessen RGB-Wert in nWC[0..2] steht. stehen in nWC[3] die
    // Werte 0x1, 0x7d oder 0x83, dann ist es ein Grauwert, dessen
    // Schwarzanteil in 1/2 % in nWC[0] steht.
    // Ich vermute, dass es auf Bit0 in nWV[3] ankommt, ob es RGB oder Grau ist.

    if( !( nWC[3] & 0x1 ) &&                        // keine Spezial-Farbe (grau)
        (    ( nWC[0] == 0 ||  nWC[0]== 0x80 || nWC[0] == 0xff )    // R-Anteil
          && ( nWC[1] == 0 ||  nWC[1]== 0x80 || nWC[1] == 0xff )    // G-Anteil
          && ( nWC[2] == 0 ||  nWC[2]== 0x80 || nWC[2] == 0xff ) ) ){// B-Anteil
        int nIdx = 0;       // und nun: Idx-Berechnung im 3er-System
        for( int i = 2; i >= 0; i-- ){
            nIdx *= 3;
            if( nWC[i] )
                nIdx += ( ( nWC[i] == 0xff ) ? 2 : 1 );
        }
        if( eColA[ nIdx ] != COL_BLACK )
            return Color( eColA[ nIdx ] );  // Standard-Color
    }
#endif
    if( nWC[3] & 0x1 ){                             // Spezialfarbe: Grau
#ifdef VCL
        register BYTE u = (BYTE)( (ULONG)( 200 - nWC[0] ) * 256 / 200 );
#else
        register USHORT u = (USHORT)( (ULONG)( 200 - nWC[0] ) * 65535L / 200 );
#endif
        return Color( u, u, u );
    }
                    // User-Color
    return Color( (USHORT)nWC[0] << 8, (USHORT)nWC[1] << 8,
                  (USHORT)nWC[2] << 8 );
}



// MakeUniqueGraphName erzeugt einen einzigartigen Namen fuer eine Grafik
//                     ( nur falls nicht bNew gesetzt ist )

BOOL SwWW8ImplReader::MakeUniqueGraphName( String& rName,
                                            const String& rFixedPart )
{
    if( !bNew )
        return FALSE;
    nImportedGraphicsCount++;
    rName = WW8_ASCII2STR( "G" );
    rName += String::CreateFromInt32( nImportedGraphicsCount );
    rName += WW8_ASCII2STR( ": " );
    rName += rFixedPart;
    return TRUE;
}



// ReadGrafStart liest die ObjektDaten ein und erzeugt falls noetig einen Anker

BOOL SwWW8ImplReader::ReadGrafStart( void* pData, short nDataSiz, WW8_DPHEAD* pHd,
                             WW8_DO* pDo )
{
    if( (INT16)SVBT16ToShort( pHd->cb )
        < (short)sizeof( WW8_DPHEAD ) + nDataSiz ){
        ASSERT( !this, "+Grafik-Element: Size ?" );
        pStrm->SeekRel( (INT16)SVBT16ToShort( pHd->cb ) - sizeof( WW8_DPHEAD ) );
        return FALSE;
    }
    pStrm->Read( pData, nDataSiz );

    RndStdIds eAnchor = ( SVBT8ToByte( pDo->by ) < 2 ) ? FLY_PAGE : FLY_AT_CNTNT;
    if( (bIsHeader || bIsFooter) && (FLY_AT_CNTNT != eAnchor) )
    {
        eAnchor = FLY_AT_CNTNT;
        pNode_FLY_AT_CNTNT = &pPaM->GetPoint()->nNode.GetNode();
    }

    pDrawFmt->SetAttr( SwFmtAnchor( eAnchor, 1 ) );
    pCtrlStck->NewAttr( *pPaM->GetPoint(), SwFltAnchor( pDrawFmt ) );
    pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_FLTR_ANCHOR );

    nDrawXOfs2 = nDrawXOfs;
    nDrawYOfs2 = nDrawYOfs;

    if( eAnchor == FLY_AT_CNTNT ){
        if( SVBT8ToByte( pDo->bx ) == 1 )       // Pos: echt links
            nDrawXOfs2 -= nPgLeft;
        if( bTable )                            // Obj in Table
            nDrawXOfs2 -= GetTableLeft();       // -> siehe Kommentar
                                                // bei GetTableLeft()
    }else{
        if( SVBT8ToByte( pDo->bx ) != 1 )
            nDrawXOfs2 += nPgLeft;
        if( SVBT8ToByte( pDo->by ) == 0 )
            nDrawYOfs2 += nPgTop;
    }

    return TRUE;
}

// SetStdAttr() setzt die Attribute, die jedes Objekt hat

static void SetStdAttr( SfxItemSet& rSet, WW8_DP_LINETYPE& rL,
                        WW8_DP_SHADOW& rSh )
{
    if( SVBT16ToShort( rL.lnps ) == 5 ){            // unsichtbar
        rSet.Put( XLineStyleItem( XLINE_NONE ) );
    }else{                                          // sichtbar
        Color aCol( WW8TransCol( rL.lnpc ) );           // LinienFarbe
        rSet.Put( XLineColorItem( aEmptyStr, aCol ) );
        rSet.Put( XLineWidthItem( SVBT16ToShort( rL.lnpw ) ) );
                                                    // LinienDicke
        if( SVBT16ToShort( rL.lnps ) >= 1
            && SVBT16ToShort(rL.lnps ) <= 4 ){      // LinienStil
            rSet.Put( XLineStyleItem( XLINE_DASH ) );
            INT16 nLen = SVBT16ToShort( rL.lnpw );
            XDash aD( XDASH_RECT, 1, 2 * nLen, 1, 5 * nLen, 5 * nLen );
            switch( SVBT16ToShort( rL.lnps ) ){
            case 1: aD.SetDots( 0 );            // Dash
                    aD.SetDashLen( 6 * nLen );
                    aD.SetDistance( 4 * nLen );
                    break;
            case 2: aD.SetDashes( 0 ); break;   // Dot
            case 3: break;                      // Dash Dot
            case 4: aD.SetDots( 2 ); break;     // Dash Dot Dot
            }
            rSet.Put( XLineDashItem( aEmptyStr, aD ) );
        }else{
            rSet.Put( XLineStyleItem( XLINE_SOLID ) );  // noetig fuer TextBox
        }
    }
    if( SVBT16ToShort( rSh.shdwpi ) ){                  // Schatten
        rSet.Put( SdrShadowItem( TRUE ) );
        rSet.Put( SdrShadowXDistItem( SVBT16ToShort( rSh.xaOffset ) ) );
        rSet.Put( SdrShadowYDistItem( SVBT16ToShort( rSh.yaOffset ) ) );
    }
}

// SetFill setzt Fuellattribute wie Vordergrund- und Hintergrund-Farbe
// und Muster durch Reduktion auf eine Farbe.
// SetFill() setzt z.Zt kein Muster, da Sdr das nur sehr umstaendlich kann
// und die Sdr-Schraffur ( XDash ) noch nicht fertig ist.
// Statt dessen wird eine Mischfarbe gewaehlt, die auf den entsprechenden
// Farbton zwischen den Farben liegt.

static void SetFill( SfxItemSet& rSet, WW8_DP_FILL& rFill )
{
    static BYTE nPatA[] = { 0, 0, 5, 10, 20, 25, 30, 40, 50, 60, 70, 75, 80,
                        90, 50, 50, 50, 50,50, 50, 33, 33, 33, 33, 33, 33 };
    register short nPat = SVBT16ToShort( rFill.flpp );

    if( nPat == 0 ){                            // durchsichtig
        rSet.Put( XFillStyleItem( XFILL_NONE ) );
    }else{
        rSet.Put( XFillStyleItem( XFILL_SOLID ) );  // noetig fuer TextBox
        if( nPat <= 1 || nPat > sizeof( nPatA ) ){  // solid Bg oder unbekannt
            rSet.Put( XFillColorItem( aEmptyStr, WW8TransCol( rFill.dlpcBg ) ) );
        }else{                                      // Brush -> Farbmischung
            Color aB( WW8TransCol( rFill.dlpcBg ) );
            Color aF( WW8TransCol( rFill.dlpcFg ) );
#ifdef VCL
            aB.SetRed( (BYTE)( ( (ULONG)aF.GetRed() * nPatA[nPat]
                        + (ULONG)aB.GetRed() * ( 100 - nPatA[nPat] ) ) / 100 ) );
            aB.SetGreen( (BYTE)( ( (ULONG)aF.GetGreen() * nPatA[nPat]
                        + (ULONG)aB.GetGreen() * ( 100 - nPatA[nPat] ) ) / 100 ) );
            aB.SetBlue( (BYTE)( ( (ULONG)aF.GetBlue() * nPatA[nPat]
                        + (ULONG)aB.GetBlue() * ( 100 - nPatA[nPat] ) ) / 100 ) );
#else
            aB.SetRed( (USHORT)( ( (ULONG)aF.GetRed() * nPatA[nPat]
                        + (ULONG)aB.GetRed() * ( 100 - nPatA[nPat] ) ) / 100 ) );
            aB.SetGreen( (USHORT)( ( (ULONG)aF.GetGreen() * nPatA[nPat]
                        + (ULONG)aB.GetGreen() * ( 100 - nPatA[nPat] ) ) / 100 ) );
            aB.SetBlue( (USHORT)( ( (ULONG)aF.GetBlue() * nPatA[nPat]
                        + (ULONG)aB.GetBlue() * ( 100 - nPatA[nPat] ) ) / 100 ) );
#endif
            rSet.Put( XFillColorItem( aEmptyStr, aB ) );
        }
    }
}

static void SetLineEndAttr( SfxItemSet& rSet, WW8_DP_LINEEND& rLe,
                            WW8_DP_LINETYPE& rLt )
{
    UINT16 aSB = SVBT16ToShort( rLe.aStartBits );
    if( aSB & 0x3 ){
        XPolygon aXP(3);
        aXP[0] = Point( 0, 330 );
        aXP[1] = Point( 100, 0 );
        aXP[2] = Point( 200, 330 );
        rSet.Put( XLineEndItem( aEmptyStr, aXP ) );
        USHORT nSiz = SVBT16ToShort( rLt.lnpw )
                        * ( ( aSB >> 2 & 0x3 ) + ( aSB >> 4 & 0x3 ) );
        if( nSiz < 220 ) nSiz = 220;
        rSet.Put( XLineEndWidthItem( nSiz ) );
        rSet.Put( XLineEndCenterItem( FALSE ) );
    }

    UINT16 aEB = SVBT16ToShort( rLe.aEndBits );
    if( aEB & 0x3 ){
        XPolygon aXP(3);
        aXP[0] = Point( 0, 330 );
        aXP[1] = Point( 100, 0 );
        aXP[2] = Point( 200, 330 );
        rSet.Put( XLineStartItem( aEmptyStr, aXP ) );
        USHORT nSiz = SVBT16ToShort( rLt.lnpw )
                        * ( ( aEB >> 2 & 0x3 ) + ( aEB >> 4 & 0x3 ) );
        if( nSiz < 220 ) nSiz = 220;
        rSet.Put( XLineStartWidthItem( nSiz ) );
        rSet.Put( XLineStartCenterItem( FALSE ) );
    }
}

// Parallel zu dem Obj-Array im Dokument baue ich ein Array auf,
// dass die Ww-Height ( -> Wer ueberdeckt wen ) beinhaltet.
// Anhand dieses VARARR wird die Einfuegeposition ermittelt.
// Der Offset bei Datei in bestehendes Dokument mit Grafiklayer einfuegen
// muss der Aufrufer den Index um nDrawObjOfs erhoeht werden, damit die
// neuen Objekte am Ende landen ( Einfuegen ist dann schneller )
static USHORT SearchPos( SvShorts* pHeight, short nWwHeight )
{
    USHORT i, nMax = pHeight->Count();
    for( i=0; i<nMax; i++ )     // lineare Suche: langsam
        if( ( pHeight->GetObject( i ) & 0x1fff ) > ( nWwHeight & 0x1fff ) )
            return i;           // vor i-tem Objekt einfuegen
    return nMax;                // am Ende anhaengen
}

// InsertObj() fuegt das Objekt in die Sw-Page ein und merkt sich die Z-Pos in
// einem VarArr

void SwWW8ImplReader::InsertObj( SdrObject* pObj, short nWwHeight )
{
    if( pDrawGroup )
        pDrawGroup->InsertObject( pObj, 0 );        // Group: Vorne einfuegen
    else
    {
        SwDrawContact* pContact = new SwDrawContact( pDrawFmt, pObj );
        USHORT nPos = SearchPos( pDrawHeight, nWwHeight );
        if( nWwHeight & 0x2000 )                    // Heaven ?
            pObj->SetLayer( nDrawHeaven );
        else
        {
            pObj->SetLayer( nDrawHell );
            pDrawFmt->SetAttr( SvxOpaqueItem( RES_OPAQUE, FALSE ) );
        }
        pDrawFmt->SetAttr( SwFmtSurround( SURROUND_THROUGHT ) );

        pDrawPg->InsertObject( pObj, nDrawObjOfs + nPos );
        pDrawHeight->Insert( nWwHeight, nPos ); // Pflege WW-Height-Array mit

            // Wenn es sich um seitengebundene Anker handelt, muss man noch
            // ein Problem umpopeln: Da ich die Seitennummer nicht weiss,
            // stecke ich den PaM stattdessen in den Anker. MA setzt das
            // bei der Formatierung dann um. Bis die Formatierung beim Anker
            // ist, liegen alle seitengebundenen Grafiken auf der 1. Seite,
            // dann verschwinden sie.
            // Abhilfe: AnkerPos initial weit auuserhalb des sichtbaren Bereiches
            // stellen, nach der Formatierung korrigiert MA sie.
            // schneller waere: ImpSetAnchor() statt NbcSetAnchor, allerdings
            // muesste dann die Relative Pos aller Objekte um die Ankerpos
            // korrigiert werden.

/*
        SwFmtAnchor aAnchor( FLY_AT_CNTNT );
        aAnchor.SetAnchor( pPaM->GetPoint() );
//      aFlySet.Put( aAnchor );
        pDrawFmt->SetAttr( aAnchor );
*/

        pObj->NbcSetAnchorPos( Point( USHRT_MAX, USHRT_MAX ) );
        pContact->ConnectToLayout( &pDrawFmt->GetAnchor() );

    }
}

// Ab hier folgen die Routinen fuer die einzelnen Objekte

void SwWW8ImplReader::ReadLine( WW8_DPHEAD* pHd, WW8_DO* pDo )
{
    WW8_DP_LINE aLine;

    if( !ReadGrafStart( (void*)&aLine, sizeof( aLine ), pHd, pDo ) )
        return;

    Point aP[2];
    {
        Point& rP0 = aP[0];
        Point& rP1 = aP[1];

        rP0.X() = (INT16)SVBT16ToShort( pHd->xa ) + nDrawXOfs2;
        rP0.Y() = (INT16)SVBT16ToShort( pHd->ya ) + nDrawYOfs2;
        rP1 = rP0;
        rP0.X() += (INT16)SVBT16ToShort( aLine.xaStart );
        rP0.Y() += (INT16)SVBT16ToShort( aLine.yaStart );
        rP1.X() += (INT16)SVBT16ToShort( aLine.xaEnd );
        rP1.Y() += (INT16)SVBT16ToShort( aLine.yaEnd );
    }
    SdrObject* pObj = new SdrPathObj( OBJ_LINE, XPolygon( Polygon( 2, aP ) ) );
    InsertObj( pObj, SVBT16ToShort( pDo->dhgt ) );

    SfxAllItemSet aSet( pDrawModel->GetItemPool() );

    SetStdAttr( aSet, aLine.aLnt, aLine.aShd );
    SetLineEndAttr( aSet, aLine.aEpp, aLine.aLnt );

//-/    pObj->SetAttributes( aSet, FALSE );
    pObj->SetItemSetAndBroadcast(aSet);
}

void SwWW8ImplReader::ReadRect( WW8_DPHEAD* pHd, WW8_DO* pDo )
{
    WW8_DP_RECT aRect;

    if( !ReadGrafStart( (void*)&aRect, sizeof( aRect ), pHd, pDo ) )
        return;

    Point aP0( (INT16)SVBT16ToShort( pHd->xa ) + nDrawXOfs2,
               (INT16)SVBT16ToShort( pHd->ya ) + nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (INT16)SVBT16ToShort( pHd->dxa );
    aP1.Y() += (INT16)SVBT16ToShort( pHd->dya );

    SdrObject* pObj = new SdrRectObj( Rectangle( aP0, aP1 ) );
    InsertObj( pObj, SVBT16ToShort( pDo->dhgt ) );

    SfxAllItemSet aSet( pDrawModel->GetItemPool() );

    SetStdAttr( aSet, aRect.aLnt, aRect.aShd );
    SetFill( aSet, aRect.aFill );

//-/    pObj->SetAttributes( aSet, FALSE );
    pObj->SetItemSetAndBroadcast(aSet);
}

void SwWW8ImplReader::ReadElipse( WW8_DPHEAD* pHd, WW8_DO* pDo )
{
    WW8_DP_ELIPSE aElipse;

    if( !ReadGrafStart( (void*)&aElipse, sizeof( aElipse ), pHd, pDo ) )
        return;

    Point aP0( (INT16)SVBT16ToShort( pHd->xa ) + nDrawXOfs2,
               (INT16)SVBT16ToShort( pHd->ya ) + nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (INT16)SVBT16ToShort( pHd->dxa );
    aP1.Y() += (INT16)SVBT16ToShort( pHd->dya );

    SdrObject* pObj = new SdrCircObj( OBJ_CIRC, Rectangle( aP0, aP1 ) );
    InsertObj( pObj, SVBT16ToShort( pDo->dhgt ) );

    SfxAllItemSet aSet( pDrawModel->GetItemPool() );

    SetStdAttr( aSet, aElipse.aLnt, aElipse.aShd );
    SetFill( aSet, aElipse.aFill );

//-/    pObj->SetAttributes( aSet, FALSE );
    pObj->SetItemSetAndBroadcast(aSet);
}

void SwWW8ImplReader::ReadArc( WW8_DPHEAD* pHd, WW8_DO* pDo )
{
    WW8_DP_ARC aArc;

    if( !ReadGrafStart( (void*)&aArc, sizeof( aArc ), pHd, pDo ) )
        return;

    Point aP0( (INT16)SVBT16ToShort( pHd->xa ) + nDrawXOfs2,
               (INT16)SVBT16ToShort( pHd->ya ) + nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (INT16)SVBT16ToShort( pHd->dxa ) * 2;
    aP1.Y() += (INT16)SVBT16ToShort( pHd->dya ) * 2;

    short nA[] = { 2, 3, 1, 0 };
    short nW = nA[ ( ( SVBT8ToByte( aArc.fLeft ) & 1 ) << 1 )
                    + ( SVBT8ToByte( aArc.fUp ) & 1 ) ];
    if( !SVBT8ToByte( aArc.fLeft ) ){
        aP0.Y() -= (INT16)SVBT16ToShort( pHd->dya );
        aP1.Y() -= (INT16)SVBT16ToShort( pHd->dya );
    }
    if( SVBT8ToByte( aArc.fUp ) ){
        aP0.X() -= (INT16)SVBT16ToShort( pHd->dxa );
        aP1.X() -= (INT16)SVBT16ToShort( pHd->dxa );
    }

    SdrObject* pObj = new SdrCircObj( OBJ_SECT, Rectangle( aP0, aP1 ),
                               nW * 9000, ( ( nW + 1 ) & 3 ) * 9000 );
    InsertObj( pObj, SVBT16ToShort( pDo->dhgt ) );

    SfxAllItemSet aSet( pDrawModel->GetItemPool() );

    SetStdAttr( aSet, aArc.aLnt, aArc.aShd );
    SetFill( aSet, aArc.aFill );

//-/    pObj->SetAttributes( aSet, FALSE );
    pObj->SetItemSetAndBroadcast(aSet);
}

void SwWW8ImplReader::ReadPolyLine( WW8_DPHEAD* pHd, WW8_DO* pDo )
{
    WW8_DP_POLYLINE aPoly;

    if( !ReadGrafStart( (void*)&aPoly, sizeof( aPoly ), pHd, pDo ) )
        return;

    UINT16 nCount = SVBT16ToShort( aPoly.aBits1 ) >> 1 & 0x7fff;
    SVBT16 *pP = new SVBT16[nCount * 2];
    pStrm->Read( pP, nCount * 4 );      // Punkte einlesen
    Polygon aP( nCount );
    Point aPt;
    USHORT i;

    for( i=0; i<nCount; i++ ){
        aPt.X() = SVBT16ToShort( pP[i << 1] ) + nDrawXOfs2
                  + (INT16)SVBT16ToShort( pHd->xa );
        aPt.Y() = SVBT16ToShort( pP[( i << 1 ) + 1] ) + nDrawYOfs2
                  + (INT16)SVBT16ToShort( pHd->ya );
        aP[i] = aPt;
    }
    delete[]( pP );

    SdrObject* pObj = new SdrPathObj(
                ( SVBT16ToShort( aPoly.aBits1 ) & 0x1 ) ? OBJ_POLY : OBJ_PLIN,
                                XPolygon( aP ) );

    InsertObj( pObj, SVBT16ToShort( pDo->dhgt ) );

    SfxAllItemSet aSet( pDrawModel->GetItemPool() );

    SetStdAttr( aSet, aPoly.aLnt, aPoly.aShd );
    SetFill( aSet, aPoly.aFill );

//-/    pObj->SetAttributes( aSet, FALSE );
    pObj->SetItemSetAndBroadcast(aSet);
}

ESelection SwWW8ImplReader::GetESelection( long nCpStart, long nCpEnd )
{
    USHORT nPCnt = pDrawEditEngine->GetParagraphCount();
    USHORT nSP = 0;
    USHORT nEP = 0;
    while(      (nSP < nPCnt)
            &&  (nCpStart >= pDrawEditEngine->GetTextLen( nSP ) + 1) )
    {
        nCpStart -= pDrawEditEngine->GetTextLen( nSP ) + 1;
        nSP++;
    }
        // Beim Ende erst 1 Zeichen spaeter auf naechste Zeile umschalten,
        // da sonst Zeilenattribute immer eine Zeile zu weit reichen.
    while(      (nEP < nPCnt)
            &&  (nCpEnd > pDrawEditEngine->GetTextLen( nEP ) + 1) )
    {
        nCpEnd -= pDrawEditEngine->GetTextLen( nEP ) + 1;
        nEP++;
    }
    return ESelection( nSP, (USHORT)nCpStart, nEP, (USHORT)nCpEnd );
}

// GetTxbxCharAttrs() setzt die harten Zeichen-Attribute in den angegebenen Set.
// Toggle-Attribute werden z.Zt. nicht beruecksichtigt
void SwWW8ImplReader::GetTxbxPapAndCharAttrs( SfxItemSet& rS,
                                              const WW8PLCFManResult& rRes )
{
    static SvxAdjust aAdjArr[] = {  SVX_ADJUST_LEFT,
                                    SVX_ADJUST_CENTER,
                                    SVX_ADJUST_RIGHT,
                                    SVX_ADJUST_BLOCK  };

    BYTE* pData = rRes.pMemPos + 1  + (8 > pWwFib->nVersion ? 0 : 1)
                                    + WW8SprmDataOfs( rRes.nSprmId );

    switch( rRes.nSprmId )
    {
    //
    // PAP attributes
    //
    case  5:
    case 0x2403: rS.Put(SvxAdjustItem(
                            aAdjArr[pData[0]&0x3], EE_PARA_JUST ) );
                 // "&0x3 gegen Tabellenueberlauf bei Stuss-Werten
                 break;
    case 21:
    case 0xA413:
    case 22:
    case 0xA414:
        {
            short nPara = SVBT16ToShort( pData );
            if( nPara < 0 )
                nPara = -nPara;

            SvxULSpaceItem aUL((SvxULSpaceItem&)rS.Get(EE_PARA_ULSPACE, TRUE));

            if( 1 & rRes.nSprmId )      // 21, 0xA413 resp.
                aUL.SetUpper( nPara );
            else                        // 22, 0xA414 resp.
                aUL.SetLower( nPara );
            rS.Put( aUL );
        };
        break;
    //
    // CHAR attributes
    //
    case      85:
    case  0x0835: rS.Put( SvxWeightItem( (*pData & 0x1)?WEIGHT_BOLD:WEIGHT_NORMAL,
                         EE_CHAR_WEIGHT ) );
                 break;
    case      86:
    case  0x0836: rS.Put( SvxPostureItem( (*pData & 0x1)?ITALIC_NORMAL:ITALIC_NONE,
                         EE_CHAR_ITALIC ) );
                 break;
    case      87:
    case  0x0837: rS.Put( SvxCrossedOutItem( (*pData & 0x1)?STRIKEOUT_SINGLE:STRIKEOUT_NONE,
                         EE_CHAR_STRIKEOUT ) );
                 break;
    case      88:
    case  0x0838: rS.Put( SvxContourItem( *pData & 0x1, EE_CHAR_OUTLINE));
                 break;
    case      89:
    case  0x0839: rS.Put( SvxShadowedItem( *pData & 0x1, EE_CHAR_SHADOW));
                 break;
    case      94:
    case  0x2A3E:if(    (1 > *pData)
                    ||
                        (     (4 <  *pData)
                           && (6 != *pData) ) )
                    rS.Put( SvxUnderlineItem(UNDERLINE_NONE,
                                             EE_CHAR_UNDERLINE) );
                else
                {
                    if( 6 == *pData )
                        rS.Put( SvxWeightItem(WEIGHT_BOLD,
                                              EE_CHAR_WEIGHT) );
                    rS.Put( SvxUnderlineItem(UNDERLINE_SINGLE,
                                             EE_CHAR_UNDERLINE) );
                }
                break;
    case      98:
    case  0x2A42: rS.Put( SvxColorItem( Color( GetCol( pData[0] ) ),
                           EE_CHAR_COLOR ) );
                 break;
    case      99:
    case  0x4A43:{
                    USHORT nFSize = SVBT16ToShort( pData );
                                // Font-Groesse in halben Point
                            //  10 = 1440 / ( 72 * 2 )
                    rS.Put( SvxFontHeightItem(
                                (const ULONG) ( (ULONG) nFSize * 10 ),
                                100, EE_CHAR_FONTHEIGHT ) );
                 }
                 break;
    case 93:
    case 0x4A4F:{
                    USHORT nFCode = SVBT16ToShort( pData ); // Font-Nummer
                    FontFamily eFamily;
                    String aName;
                    FontPitch ePitch;
                    CharSet eSrcCharSet;

                    if( GetFontParams( nFCode, eFamily, aName, ePitch,
                                       eSrcCharSet ) )
                    {
                        rS.Put( SvxFontItem( eFamily, aName, aEmptyStr, ePitch,
                                             eSrcCharSet, EE_CHAR_FONTINFO ) );
                    }
                }
                 break;
    }
}

/*
// InsertTxbxCharAttrs() setzt die harten Zeichen-Attribute
void SwWW8ImplReader::InsertTxbxCharAttrs( long nStartCp, long nEndCp, BOOL bONLYnPicLocFc )
{
    nStartCp += nDrawCpO;
    nEndCp   += nDrawCpO;
    WW8PLCFx_Cp_FKP* pChp = pPlcxMan->GetChpPLCF();
    pChp->SeekPos( nStartCp );

    nPicLocFc = LONG_MAX;
    WW8_CP nStart = pChp->Where();
    while( nStart <= nEndCp )
    {
        SfxItemSet aS( pDrawEditEngine->GetEmptyItemSet() );
        WW8PLCFxDesc aDesc;
        pChp->GetSprms( &aDesc );
        (*pChp)++;
        WW8_CP nNextEnd = pChp->Where();
        WW8_CP nEnd     = ( nNextEnd < nEndCp ) ? nNextEnd : nEndCp;

        if( aDesc.nSprmsLen && aDesc.pMemPos )  // Attribut(e) vorhanden?
            GetTxbxCharAttrs( aS, aDesc, bONLYnPicLocFc );

        if( bONLYnPicLocFc ) // Picture-Position-Attribut gefunden?
        {
            if( LONG_MAX != nPicLocFc ) break;
        //  ==================================
        }
        else
            if( aS.Count() && !bONLYnPicLocFc )
                pDrawEditEngine->QuickSetAttribs( aS,
                        GetESelection( nStart - nStartCp, nEnd - nStartCp ) );
        nStart = nNextEnd;
    }
    if( LONG_MAX == nPicLocFc ) nPicLocFc = 0;
}

// GetTxbxParaAttrs() setzt die harten Para-Attribute in den angegebenen Set.
// z.Zt. wird nur Justify beachtet, da einfacher
void SwWW8ImplReader::GetTxbxParaAttrs( SfxItemSet& rS, const WW8PLCFxDesc& rD )
{
    static SvxAdjust aAdjArr[] = { SVX_ADJUST_LEFT,
                     SVX_ADJUST_CENTER, SVX_ADJUST_RIGHT, SVX_ADJUST_BLOCK };

    long  nLen  = rD.nSprmsLen;
    BYTE* pSprm = rD.pMemPos;

    while( nLen >= 2 )
    {
        BYTE   nDelta;
        USHORT nId = WW8GetSprmId( pWwFib->nVersion, pSprm, &nDelta );

        short nSL = WW8GetSprmSizeBrutto( pWwFib->nVersion, pSprm, &nId );

        if( nLen < nSL )
            return;                 // nicht mehr genug Bytes ueber

        BYTE* pData = pSprm + 1 + nDelta + WW8SprmDataOfs( nId );

        switch( nId )
        {
        case  5:
        case 0x2403: rS.Put(SvxAdjustItem(
                                aAdjArr[pData[0]&0x3], EE_PARA_JUST ) );
                     // "&0x3 gegen Tabellenueberlauf bei Stuss-Werten
                     break;
        case 21:
        case 0xA413:
        case 22:
        case 0xA414:
            {
                short nPara = SVBT16ToShort( pData );
                if( nPara < 0 )
                    nPara = -nPara;

                SvxULSpaceItem aUL( (SvxULSpaceItem&)rS.Get(EE_PARA_ULSPACE,TRUE ));

                switch( nId ){                // keine Versuche
                case 21:
                case 0xA413: aUL.SetUpper( nPara ); break;
                case 22:
                case 0xA414: aUL.SetLower( nPara ); break;
                };
                rS.Put( aUL );
            };
            break;
        }
        pSprm += nSL;
        nLen -= nSL;
    }
}
*/
// InsertTxbxStyAttrs() setzt die Style-Attribute in den uebergebenen ItemSet.
// Es werden die SW-Styles genommen, die Import-WW-Styles sind zu diesem
// Zeitpunkt schon destruiert.
// Die SW-Styles werden per Tiefensuche, d.h. mit Parent-Styles nach den
// in aSrcTab angegebenen Attributen untersucht. Diese werden per Clone
// dupliziert, bei den Duplikaten werden die Which-IDs
// gemaess der Tabelle aDstTab umgesetzt, damit die EditEngine sie nicht
// ignoriert.
// Es werden hierbei sowohl Para- wie auch Zeichen-Attribute in den
// ItemSet gestopft.
void SwWW8ImplReader::InsertTxbxStyAttrs( SfxItemSet& rS, USHORT nColl )
{
    static USHORT __READONLY_DATA aSrcTab[] = {
        ITEMID_FONT,
        ITEMID_POSTURE,
        ITEMID_WEIGHT,
        ITEMID_SHADOWED,
        ITEMID_CONTOUR,
        ITEMID_CROSSEDOUT,
        ITEMID_UNDERLINE,
        ITEMID_FONTHEIGHT,
        ITEMID_COLOR,
        ITEMID_WORDLINEMODE,
        ITEMID_ESCAPEMENT,
        ITEMID_AUTOKERN,
        ITEMID_KERNING,
        ITEMID_ADJUST,
        ITEMID_LINESPACING,
        ITEMID_TABSTOP,
        ITEMID_LRSPACE,
        ITEMID_ULSPACE };

    static USHORT __READONLY_DATA aDstTab[] = {
        EE_CHAR_FONTINFO,
        EE_CHAR_ITALIC,
        EE_CHAR_WEIGHT,
        EE_CHAR_SHADOW,
        EE_CHAR_OUTLINE,
        EE_CHAR_STRIKEOUT,
        EE_CHAR_UNDERLINE,
        EE_CHAR_FONTHEIGHT,
        EE_CHAR_COLOR,
        EE_CHAR_WLM,
        EE_CHAR_ESCAPEMENT,
        EE_CHAR_PAIRKERNING,
        EE_CHAR_KERNING,
        EE_PARA_JUST,
        EE_PARA_SBL,
        EE_PARA_TABS,
        EE_PARA_LRSPACE,
        EE_PARA_ULSPACE };

    if( nColl < nColls && pCollA[nColl].pFmt && pCollA[nColl].bColl ){
        const SfxPoolItem* pItem;
        for( USHORT i = 0; i < sizeof(aSrcTab)/sizeof(aSrcTab[0]); i++ ){
            if( SFX_ITEM_SET == pCollA[nColl].pFmt->GetItemState(
                                   aSrcTab[i], TRUE, &pItem ) ){
                SfxPoolItem* pCopy = pItem->Clone();
                pCopy->SetWhich( aDstTab[i] );
                rS.Put( *pCopy );
                delete pCopy;
            }
        }
    }

}
/*
// InsertTxbxParaAttrs() setzt zwischen StartCp und EndCp die Style-
// uns Absatz-Attribute. Dabei werden Style-Attribute als harte Attribute
// gesetzt, da die EditEngine-Styles im SW UI-maessig nicht benutzt
// werden und der Import daher das auch nicht soll.
// Es werden also harte Para-Attrs und *alle* Style-Attrs gesetzt.
void SwWW8ImplReader::InsertTxbxParaAttrs( long nStartCp, long nEndCp )
{
    nStartCp += nDrawCpO;
    nEndCp   += nDrawCpO;
    WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();
    pPap->SeekPos( nStartCp );

    WW8_CP nStart = pPap->Where();
    while( nStart <= nEndCp )
    {
        SfxItemSet aS( pDrawEditEngine->GetEmptyItemSet() );
        WW8PLCFxDesc aDesc;
        pPap->GetSprms( &aDesc );
        (*pPap)++;
        WW8_CP nNextEnd = pPap->Where();
        WW8_CP nEnd = ( nNextEnd < nEndCp ) ? nNextEnd : nEndCp;

        InsertTxbxStyAttrs( aS, pPap->GetIstd() );  // Style-Kram rein

        if( aDesc.nSprmsLen && aDesc.pMemPos )  // Attribut(e) vorhanden
            GetTxbxParaAttrs( aS, aDesc );

        if( aS.Count() )
            pDrawEditEngine->QuickSetAttribs( aS,
                GetESelection( nStart - nStartCp, nEnd - nStartCp ) );
        nStart = nNextEnd;
    }
}
*/
// InsertTxbxAttrs() setzt zwischen StartCp und EndCp die Attribute.
// Dabei werden Style-Attribute als harte Attribute, Absatz- und Zeichen-
// attribute gesetzt.
void SwWW8ImplReader::InsertTxbxAttrs( long nStartCp,
                                       long nEndCp,
                                       BOOL bONLYnPicLocFc )
{
    WW8PLCFManResult aRes;
    SfxItemSet aS( pDrawEditEngine->GetEmptyItemSet() );

    nStartCp += nDrawCpO;
    nEndCp   += nDrawCpO;
    WW8ReaderSave aSave(this,nStartCp);


    WW8_CP nStart = pPlcxMan->Where();
    WW8_CP nNext;
    WW8_CP nEnd;
    USHORT nIstd     = pPlcxMan->GetPapPLCF()->GetIstd();
    USHORT nNextIstd = USHRT_MAX;

    // store the *first* Style's SPRMs
    InsertTxbxStyAttrs( aS, nIstd );

    while( nStart <= nEndCp )
    {
        // get position of next SPRM
        if(    pPlcxMan->Get( &aRes )
            && aRes.pMemPos && aRes.nSprmId )
        {
            if( bONLYnPicLocFc )
            {
                if(    (    68 == aRes.nSprmId)
                    || (0x6A03 == aRes.nSprmId) )
                {
                    Read_PicLoc( aRes.nSprmId,
                                 aRes.pMemPos
                                 + 1
                                 + (8 > pWwFib->nVersion ? 0 : 1)
                                 + WW8SprmDataOfs( aRes.nSprmId ),
                                 4 );
                    // Ok, that's it.  Now let's get out of here!
                    break;
                }
            }
            else if( aRes.nSprmId && (    (    256 >  aRes.nSprmId )
                                       || ( 0x0800 <= aRes.nSprmId ) ) )
            {
                GetTxbxPapAndCharAttrs( aS, aRes );
            }
        }

        (*pPlcxMan)++;
        nNext = pPlcxMan->Where();

        if( (nNext != nStart) && !bONLYnPicLocFc )
        {
            nNextIstd = pPlcxMan->GetPapPLCF()->GetIstd();
            if( nNextIstd != nIstd )
            {
                nIstd = nNextIstd;
                // store the *next* Style's SPRMs
                InsertTxbxStyAttrs( aS, nIstd );
            }

            nEnd = ( nNext < nEndCp ) ? nNext : nEndCp;
            // put the attrs into the doc
            if( aS.Count() )
                pDrawEditEngine->QuickSetAttribs( aS,
                    GetESelection( nStart - nStartCp, nEnd - nStartCp ) );
        }
        nStart = nNext;
    }

    aSave.Restore(this);
}

BOOL SwWW8ImplReader::GetTxbxTextSttEndCp( long& rStartCp, long& rEndCp,
                                            USHORT nTxBxS, USHORT nSequence )
{
    // rasch den TextBox-PLCF greifen
    WW8PLCFspecial* pT = pPlcxMan->GetTxbx();
    if( !pT )
    {
        ASSERT( !this, "+Wo ist der Grafik-Text (1) ?" );
        return FALSE;
    }

    // ggfs. zuerst die richtige TextBox-Story finden
    BOOL bCheckTextBoxStory = ( nTxBxS && pT->GetIMax() >= nTxBxS );
    if(  bCheckTextBoxStory )
        pT->SetIdx( nTxBxS-1 );

    // dann Start und Ende ermitteln
    void* pT0;
    if( !pT->Get( rStartCp, pT0 ) )
    {
        ASSERT( !this, "+Wo ist der Grafik-Text (2) ?" );
        return FALSE;
    }

    if( bCheckTextBoxStory )
    {
        BOOL bReusable = (0 != SVBT16ToShort( ((WW8_TXBXS*)pT0)->fReusable ));
        while( bReusable )
        {
            (*pT)++;
            if( !pT->Get( rStartCp, pT0 ) )
            {
                ASSERT( !this, "+Wo ist der Grafik-Text (2-a) ?" );
                return FALSE;
            }
            bReusable = (0 != SVBT16ToShort( ((WW8_TXBXS*)pT0)->fReusable ));
        }
    }
    (*pT)++;
    if( !pT->Get( rEndCp, pT0 ) )
    {
        ASSERT( !this, "+Wo ist der Grafik-Text (3) ?" );
        return FALSE;
    }

    // jetzt ggfs. die passende Page in der Break-Table finden
    if( bCheckTextBoxStory )
    {
        // Sonderfall: gesamte(!) Kette soll ermittelt werden,
        //             dann sind wir hier schon fertig!
        if( USHRT_MAX > nSequence )
        {
            long nMinStartCp = rStartCp;
            long nMaxEndCp   = rEndCp;
            // rasch den TextBox-Break-Deskriptor-PLCF greifen
            pT = pPlcxMan->GetTxbxBkd();
            if (!pT) //It can occur on occasion, Caolan
                return FALSE;

            // den ersten Eintrag fuer diese TextBox-Story finden
            if( !pT->SeekPos( rStartCp ) )
            {
                ASSERT( !this, "+Wo ist der Grafik-Text (4) ?" );
                return FALSE;
            }
            // ggfs. entsprechende Anzahl Eintraege weitergehen
            for(USHORT iSequence = 0; iSequence < nSequence; iSequence++) (*pT)++;
            // dann die tatsaechlichen Start und Ende ermitteln
            if(    (!pT->Get( rStartCp, pT0 ))
                || ( nMinStartCp > rStartCp  ) )
            {
                ASSERT( !this, "+Wo ist der Grafik-Text (5) ?" );
                return FALSE;
            }
            if( rStartCp >= nMaxEndCp )
                rEndCp = rStartCp;  // kein Error: leerer String!
            else
            {
                (*pT)++;
                if(    (!pT->Get( rEndCp, pT0 ))
                    || ( nMaxEndCp < rEndCp-1  ) )
                {
                    ASSERT( !this, "+Wo ist der Grafik-Text (6) ?" );
                    return FALSE;
                }
                rEndCp -= 1;
            }
        }
        else
            rEndCp -= 1;
    }
    else
        rEndCp -= 2;
    return TRUE;
}

// TxbxText() holt aus WW-File den Text und gibt diesen und den Anfangs- und
// den um -2 (bzw. -1 bei Ver8) korrigierten End-Cp zurueck


BOOL SwWW8ImplReader::GetTxbxText( String& rString,
                                   long nStartCp, long nEndCp )
{
    nDrawTxbx++;
    BOOL bOk = FALSE;
    if( nStartCp > nEndCp )
    {
        ASSERT( !this, "+Wo ist der Grafik-Text (7) ?" );
    }
    else
    if( nStartCp == nEndCp )
        rString.Erase();      // leerer String: durchaus denkbar!
    else
    {
        // den Text einlesen: kann sich ueber mehrere Pieces erstrecken!!!
        USHORT nLen = pSBase->WW8ReadString( *pStrm, rString,
                                            nStartCp + nDrawCpO,
                                            nEndCp   - nStartCp,
                                            eTextCharSet );
        if( !nLen )
        {
            ASSERT( !this, "+Wo ist der Grafik-Text (8) ?" );
        }
        else
        {
            bOk = TRUE;
            if( 0x0d == rString.GetChar(nLen - 1) )
                rString.Erase(nLen - 1);
        }
    }
    return bOk;
}


// InsertTxbxText() fuegt fuer TextBoxen und CaptionBoxen den Text
// und die Attribute ein
SwFrmFmt* SwWW8ImplReader::InsertTxbxText(SdrTextObj* pTextObj,
                                     Size*       pObjSiz,
                                     USHORT      nTxBxS,
                                     USHORT      nSequence,
                                     long        nPosCp,
                                     SwFrmFmt*   pOldFlyFmt,
                                     BOOL        bMakeSdrGrafObj,
                                     BOOL&       rbEraseTextObj,
                                     BOOL*       pbTestTxbxContainsText,
                                     long*       pnStartCp,
                                     long*       pnEndCp,
                                     SvxMSDffImportRec* pRecord)
{
    SwFrmFmt* pFlyFmt = 0;
    ULONG nOld = pStrm->Tell();

    rbEraseTextObj = FALSE;

    String aString;
    long nStartCp, nEndCp, nNewStartCp;
    BOOL bTextWasRead = GetTxbxTextSttEndCp( nStartCp, nEndCp,
                                             nTxBxS, nSequence ) &&
                        GetTxbxText( aString, nStartCp, nEndCp );
    nNewStartCp = nStartCp;

    if( !pbTestTxbxContainsText )
    {
        if( bTextWasRead )
            while( STRING_NOTFOUND != aString.SearchAndReplace( 0xb, ' ' ))
                ;   // HardNewline kann EE noch nicht in der EE-Core

        if( !pDrawEditEngine )
#if SUPD>601
            pDrawEditEngine = new EditEngine(0);
#else
            pDrawEditEngine = new EditEngine;
#endif
        if( pObjSiz )
            pDrawEditEngine->SetPaperSize( *pObjSiz );
    }

    if( bTextWasRead )
    {
        for(USHORT i=0; i < aString.Len(); i++)
        {
            if( 0x13 == aString.GetChar( i ) )
            {
                do
                {
                    aString.Erase( i, 1 );
                    nNewStartCp++;
                }
                while(              aString.Len()
                        && (   i  < aString.Len())
                        && (0x14 != aString.GetChar( i ) )
                        && (0x15 != aString.GetChar( i ) ) );
                if( aString.Len() )
                {
                    if( 0x14 == aString.GetChar( i ) )
                    {
                        aString.Erase( i, 1 );
                        nNewStartCp++;
                        do
                        {
                            i++;
                        }
                        while(              aString.Len()
                                && (   i  < aString.Len())
                                && (0x15 != aString.GetChar( i ) ) );
                        if( i < aString.Len() )
                            aString.Erase( i, 1 );
                    }
                    else if( 0x15 == aString.GetChar( i ) )
                    {
                        aString.Erase( i, 1 );
                    }
                }
            }
        }
        if (1 == aString.Len())
        {
            BOOL bDone = TRUE;
            switch( aString.GetChar(0) )
            {
                case 0x1:if( !pbTestTxbxContainsText )
                        {
                            WW8ReaderSave aSave(this,nNewStartCp + nDrawCpO -1);
                            BOOL bOldEmbeddObj = bEmbeddObj;
                            //bEmbedd Ordinarily would have been set by field
                            //parse, but this is impossible here so...
                            bEmbeddObj = TRUE;

                            // 1st look for OLE- or Graph-Indicator Sprms
                            WW8PLCFx_Cp_FKP* pChp = pPlcxMan->GetChpPLCF();
                            WW8PLCFxDesc aDesc;
                            pChp->GetSprms( &aDesc );
                            WW8SprmIter aSprmIter( aDesc.pMemPos, aDesc.nSprmsLen,
                                                   GetFib().nVersion );

                            //BOOL bRead_Obj    = FALSE;
                            //BOOL bRead_PicLoc = FALSE;
                            BYTE* pParams = aSprmIter.GetAktParams();
                            for( int nLoop = 0; nLoop < 2; ++nLoop )
                            {
                                while(     aSprmIter.GetSprms()
                                        && (0 != (pParams = aSprmIter.GetAktParams())) )
                                {
                                    USHORT nAktId = aSprmIter.GetAktId();
                                    switch( nAktId )
                                    {
                                        case     75:
                                        case    118:
                                        case 0x080A:
                                        case 0x0856:
                                            //if( !bRead_Obj )
                                            //{
                                                Read_Obj(nAktId, pParams, 1);
                                            //  bRead_Obj = TRUE;
                                            //}
                                            break;
                                        case     68:  // Read_Pic()
                                        case 0x6A03:
                                        case 0x680E:
                                            //if( !bRead_PicLoc )
                                            //{
                                                Read_PicLoc(nAktId, pParams, 1);
                                            //  bRead_PicLoc = TRUE;
                                            //}
                                            break;
                                    }
                                    aSprmIter++;
                                }

                                //if( bRead_Obj || bRead_PicLoc ) break;
                                if( !nLoop )
                                {
                                    pChp->GetPCDSprms(  aDesc );
                                    aSprmIter.SetSprms( aDesc.pMemPos,
                                        aDesc.nSprmsLen );
                                }
                                //if( bRead_Obj || bRead_PicLoc ) break;
                            }
                            aSave.Restore(this);
                            bEmbeddObj=bOldEmbeddObj;

                            // then import either an OLE of a Graphic
                            if( bObj )
                                {
                                pFlyFmt = ImportOle();
                                bObj=FALSE;
                                }
                            else
                            {
                                InsertTxbxAttrs(nNewStartCp,
                                                nNewStartCp+1,
                                                TRUE);
                                //InsertTxbxCharAttrs(nNewStartCp, nNewStartCp+1, TRUE);
                                pFlyFmt = ImportGraf(
                                            bMakeSdrGrafObj ? pTextObj : 0,
                                            pOldFlyFmt,
                                            pTextObj
                                          ? (nDrawHell == pTextObj->GetLayer())
                                          : FALSE );
                            }
                        }
                        break;
                case 0x8:if( !pbTestTxbxContainsText )
                        {
                            if( !bObj )
                                 pFlyFmt = Read_GrafLayer( nPosCp );
                        }
                        break;
                default:bDone = FALSE;
                        break;
            }
            if( bDone )
            {
                if( pFlyFmt )
                {
                    if( pRecord )
                    {
                        SfxItemSet aFlySet( rDoc.GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1 );

                        Rectangle aInnerDist(   pRecord->nDxTextLeft,
                                                pRecord->nDyTextTop,
                                                pRecord->nDxTextRight,
                                                pRecord->nDyTextBottom  );
                        MatchSdrItemsIntoFlySet( pTextObj,
                                                 aFlySet,
                                                 pRecord->eLineStyle,
                                                 aInnerDist,
                                                 !pRecord->bLastBoxInChain );

                        pFlyFmt->SetAttr( aFlySet );


                        MatchWrapDistancesIntoFlyFmt( pRecord, pFlyFmt );
                    }

                }
                aString.Erase();
                rbEraseTextObj = (0 != pFlyFmt);
            }
        }

    }

    if( pbTestTxbxContainsText )
    {
        if( pnStartCp ) *pnStartCp = nStartCp;
        if( pnEndCp   ) *pnEndCp   = nEndCp;
        *pbTestTxbxContainsText    =    bTextWasRead
                                     && ! rbEraseTextObj;
    }
    else if( !rbEraseTextObj )
    {
        if( bTextWasRead )
        {
            pDrawEditEngine->SetText( aString );
            InsertTxbxAttrs( nStartCp, nEndCp, FALSE );
    //      pDrawEditEngine->QuickFormatDoc();  // nach MT nicht noetig
        }

#if SUPD>600
#if SUPD>601
        OutlinerParaObject* pOp = new OutlinerParaObject( *pDrawEditEngine->CreateTextObject() );
#else
        OutlinerParaObject* pOp = new OutlinerParaObject( *pDrawEditEngine );
#endif
        pTextObj->NbcSetOutlinerParaObject( pOp );
#endif

        // Fuer die naechste Textbox noch die alten Absatz-Attribute
        // und Styles entfernen, sonst startet die naechste Box
        // mit falschen Attributen.
        // Vorgehen: Text loeschen = auf 1 Absatz reduzieren
        // und an diesem Absatz die Absatzattribute und Styles loeschen
        // (Empfehlung JOE)
        pDrawEditEngine->SetText( aEmptyStr );
        pDrawEditEngine->SetParaAttribs( 0, pDrawEditEngine->GetEmptyItemSet() );
    }

    pStrm->Seek( nOld );
    return pFlyFmt;
}

BOOL SwWW8ImplReader::TxbxChainContainsRealText( USHORT nTxBxS,
                                                 long&  rStartCp,
                                                 long&  rEndCp )
{
    BOOL bErase, bContainsText;
    InsertTxbxText( 0,0,nTxBxS,USHRT_MAX,0,0,0, bErase,
                                                &bContainsText,
                                                &rStartCp,
                                                &rEndCp );
    return bContainsText;
}


// TextBoxes only for Ver67 !!
void SwWW8ImplReader::ReadTxtBox( WW8_DPHEAD* pHd, WW8_DO* pDo )
{
    BOOL bDummy;
    WW8_DP_TXTBOX aTxtB;

    if( !ReadGrafStart( (void*)&aTxtB, sizeof( aTxtB ), pHd, pDo ) )
        return;

    Point aP0( (INT16)SVBT16ToShort( pHd->xa ) + nDrawXOfs2,
               (INT16)SVBT16ToShort( pHd->ya ) + nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (INT16)SVBT16ToShort( pHd->dxa );
    aP1.Y() += (INT16)SVBT16ToShort( pHd->dya );

    SdrTextObj* pObj = new SdrRectObj( OBJ_TEXT, Rectangle( aP0, aP1 ) );
    pObj->SetModel( pDrawModel );
    Size aSize( (INT16)SVBT16ToShort( pHd->dxa ) ,
                                 (INT16)SVBT16ToShort( pHd->dya ) );

    InsertTxbxText(pObj, &aSize, 0, 0, 0, 0, FALSE, bDummy );

    InsertObj( pObj, SVBT16ToShort( pDo->dhgt ) );

    SfxAllItemSet aSet( pDrawModel->GetItemPool() );

    SetStdAttr( aSet, aTxtB.aLnt, aTxtB.aShd );
    SetFill( aSet, aTxtB.aFill );

    aSet.Put(SdrTextFitToSizeTypeItem( SDRTEXTFIT_NONE ));
    aSet.Put( SdrTextAutoGrowWidthItem(  FALSE ) );
    aSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );
    aSet.Put( SdrTextLeftDistItem(  MIN_BORDER_DIST*2 ) );
    aSet.Put( SdrTextRightDistItem( MIN_BORDER_DIST*2 ) );
    aSet.Put( SdrTextUpperDistItem( MIN_BORDER_DIST ) );
    aSet.Put( SdrTextLowerDistItem( MIN_BORDER_DIST ) );

//-/    pObj->SetAttributes( aSet, FALSE );
    pObj->SetItemSetAndBroadcast(aSet);
}


void SwWW8ImplReader::ReadCaptionBox( WW8_DPHEAD* pHd, WW8_DO* pDo )
{
    static SdrCaptionType aCaptA[] = { SDRCAPT_TYPE1, SDRCAPT_TYPE2,
                                       SDRCAPT_TYPE3, SDRCAPT_TYPE4 };

    WW8_DP_CALLOUT_TXTBOX aCallB;

    if( !ReadGrafStart( (void*)&aCallB, sizeof( aCallB ), pHd, pDo ) )
        return;

    UINT16 nCount = SVBT16ToShort( aCallB.dpPolyLine.aBits1 ) >> 1 & 0x7fff;
    SVBT16 *pP = new SVBT16[nCount * 2];
    pStrm->Read( pP, nCount * 4 );      // Punkte einlesen
    BYTE nTyp = (BYTE)nCount - 1;
    if( nTyp == 1 && SVBT16ToShort( pP[0] ) == SVBT16ToShort( pP[2] ) )
        nTyp = 0;

    Point aP0( (INT16)SVBT16ToShort( pHd->xa ) +
               (INT16)SVBT16ToShort( aCallB.dpheadTxbx.xa ) + nDrawXOfs2,
               (INT16)SVBT16ToShort( pHd->ya )
               + (INT16)SVBT16ToShort( aCallB.dpheadTxbx.ya ) + nDrawYOfs2 );
    Point aP1( aP0 );
    aP1.X() += (INT16)SVBT16ToShort( aCallB.dpheadTxbx.dxa );
    aP1.Y() += (INT16)SVBT16ToShort( aCallB.dpheadTxbx.dya );
    Point aP2( (INT16)SVBT16ToShort( pHd->xa )
                + (INT16)SVBT16ToShort( aCallB.dpheadPolyLine.xa )
                + nDrawXOfs2 + (INT16)SVBT16ToShort( pP[0] ),
               (INT16)SVBT16ToShort( pHd->ya )
               + (INT16)SVBT16ToShort( aCallB.dpheadPolyLine.ya )
               + nDrawYOfs2 + (INT16)SVBT16ToShort( pP[1] ) );
    delete[]( pP );

    SdrCaptionObj* pObj = new SdrCaptionObj( Rectangle( aP0, aP1 ), aP2 );
    Size aSize( (INT16)SVBT16ToShort( aCallB.dpheadTxbx.dxa ),
                           (INT16)SVBT16ToShort(  aCallB.dpheadTxbx.dya ) );
    BOOL bEraseThisObject;

    InsertTxbxText(pObj, &aSize, 0, 0, 0, 0, FALSE, bEraseThisObject );

    InsertObj( pObj, SVBT16ToShort( pDo->dhgt ) );

    SfxAllItemSet aSet( pDrawModel->GetItemPool() );

    if( SVBT16ToShort( aCallB.dptxbx.aLnt.lnps ) != 5 ) // Umrandung sichtbar ?
        SetStdAttr( aSet, aCallB.dptxbx.aLnt, aCallB.dptxbx.aShd );
    else                                                // nein -> Nimm Linie
        SetStdAttr( aSet, aCallB.dpPolyLine.aLnt, aCallB.dptxbx.aShd );
    SetFill( aSet, aCallB.dptxbx.aFill );
    aSet.Put( SdrCaptionTypeItem( aCaptA[nTyp] ) );

//-/    pObj->SetAttributes( aSet, FALSE );
    pObj->SetItemSetAndBroadcast(aSet);
}


void SwWW8ImplReader::ReadGroup( WW8_DPHEAD* pHd, WW8_DO* pDo )
{
    INT16 nGrouped;

    if( !ReadGrafStart( (void*)&nGrouped, sizeof( nGrouped ), pHd, pDo ) )
        return;

#ifdef __BIGENDIAN
    nGrouped = (INT16)SWAPSHORT( nGrouped );
#endif

    nDrawXOfs += (INT16)SVBT16ToShort( pHd->xa );
    nDrawYOfs += (INT16)SVBT16ToShort( pHd->ya );

    SdrObject* pObj = new SdrObjGroup;

    InsertObj( pObj, SVBT16ToShort( pDo->dhgt ) );

    SdrObjList* pOldGroup = pDrawGroup;
    pDrawGroup = pObj->GetSubList();

    short nLeft = (INT16)SVBT16ToShort( pHd->cb ) - sizeof( WW8_DPHEAD );
    for( int i = 0; i < nGrouped; i++ )
        ReadGrafPrimitive( nLeft, pDo );

    pDrawGroup = pOldGroup;
    nDrawXOfs -= (INT16)SVBT16ToShort( pHd->xa );
    nDrawYOfs -= (INT16)SVBT16ToShort( pHd->ya );
}

void SwWW8ImplReader::ReadGrafPrimitive( short& rLeft, WW8_DO* pDo )
{
    WW8_DPHEAD aHd;                         // Lese Draw-Primitive-Header
    pStrm->Read( &aHd, sizeof( WW8_DPHEAD ) );

    if( rLeft >= (INT16)SVBT16ToShort( aHd.cb ) )   // Vorsichtsmassmahme
    {
        switch( (INT16)SVBT16ToShort( aHd.dpk ) & 0xff )
        {
        case 0:  ReadGroup( &aHd, pDo );
                 break;
        case 1:  ReadLine( &aHd, pDo );
                 break;
        case 2:  ReadTxtBox( &aHd, pDo );
                 break;
        case 3:  ReadRect( &aHd, pDo );
                 break;
        case 4:  ReadElipse( &aHd, pDo );
                 break;
        case 5:  ReadArc( &aHd, pDo );
                 break;
        case 6:  ReadPolyLine( &aHd, pDo );
                 break;
        case 7:  ReadCaptionBox( &aHd, pDo );
                 break;
        default:                            // unbekannt
                 pStrm->SeekRel( (INT16)SVBT16ToShort( aHd.cb )
                                 - sizeof( WW8_DPHEAD )  );
                 break;
        }
    }
    else
    {
        ASSERT( !this, "+Grafik-Overlap" );
    }
    rLeft -= (INT16)SVBT16ToShort( aHd.cb );
}

void SwWW8ImplReader::ReadGrafLayer1( WW8PLCFspecial* pPF, long nGrafAnchorCp )
{
    pPF->SeekPos( nGrafAnchorCp );
    long nStartFc;
    void* pF0;
    if( !pPF->Get( nStartFc, pF0 ) )
    {
        ASSERT( !this, "+Wo ist die Grafik (2) ?" );
        return;
    }
    WW8_FDOA* pF = (WW8_FDOA*)pF0;
    if( !SVBT32ToLong( pF->fc ) )
    {
        ASSERT( !this, "+Wo ist die Grafik (3) ?" );
        return;
    }
    WW8_DO aDo;
    pStrm->Seek( SVBT32ToLong( pF->fc ) );                  // Lese Draw-Header
    pStrm->Read( &aDo, sizeof( WW8_DO ) );

    short nLeft = SVBT16ToShort( aDo.cb ) - sizeof( WW8_DO );
    while( nLeft > sizeof( WW8_DPHEAD ) )
    {
        ReadGrafPrimitive( nLeft, &aDo );
    }
}

BOOL SwWW8ImplReader::MatchSdrBoxIntoFlyBoxItem(const Color& rLineColor,
                                                MSO_LineStyle eLineStyle,
                                                USHORT      nLineWidth,
                                                SvxBoxItem& rBox )
{   // Deklarationen gemaess BOXITEM.HXX
    WW8_DECL_LINETAB_ARRAY

    BOOL   bRet = FALSE;
    USHORT nIdx = USHRT_MAX;

    if( nLineWidth )
    {
        /*
            Beachte: im Gegensatz zu den Winword-ueblichen Tabellen- und
            Rahmen-Randbreiten-Angaben, bei denen jeweils aus der Staerke
            *einer* Linie die Gesamt-Randbreite zu errechnen ist,
            liegen die aus dem ESCHER stammenden Daten bereits als
            Gesamt-Breite [twips] vor!

            Der Winword default ist 15 tw. Wir nehmen hierfuer unsere 20 tw Linie.
            ( 0.75 pt uns 1.0 pt sehen sich auf dem Ausdruck naemlich
              aehnlicher als etwas 0.75 pt und unsere 0.05 pt Haarlinie. )
            Die Haarlinie setzen wir nur bei Winword-Staerken bis zu
            maximal 0.5 pt ein.
        */
        switch( eLineStyle )
        {
        // zuerst die Einzel-Linien
        case mso_lineSimple:     if( nLineWidth < 11) nIdx =            0;//   1 Twip bei uns
                            else if( nLineWidth < 46) nIdx =            1;//  20 Twips
                            else if( nLineWidth < 66) nIdx =            2;//  50
                            else if( nLineWidth < 91) nIdx =            3;//  80
                            else if( nLineWidth <126) nIdx =            4;// 100
                            // Pfusch: fuer die ganz dicken Linien muessen
                            //         wir doppelte Linien malen, weil unsere
                            //               Einfach-Linie nicht dicker als 5 Punkt wird
                            else if( nLineWidth <166) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+2;// 150
                            else                      nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+5;// 180
                            break;
        // dann die Doppel-Linien, fuer die wir feine Entsprechungen haben :-)))
        case mso_lineDouble:     if( nLineWidth <  46) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 0;//  22 Twips bei uns
                            else if( nLineWidth < 106) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 1;//  60
                            else                       nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 2;// 150
                            break;
        case mso_lineThickThin:  if( nLineWidth <  87) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 8;//  71 Twips bei uns
                            else if( nLineWidth < 117) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 9;// 101
                            else if( nLineWidth < 166) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+10;// 131
                            else                       nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 5;// 180
                            break;
        case mso_lineThinThick:  if( nLineWidth < 137) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 4;//  90 Twips bei uns
                            else                       nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 6;// 180
                            break;
        // zu guter Letzt die Dreifach-Linien, an deren Stelle wir eine Doppel-Linie setzen
        case mso_lineTriple:     if( nLineWidth <  46) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 0;//  22 Twips bei uns
                            else if( nLineWidth < 106) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 1;//  60
                            else if( nLineWidth < 166) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 2;// 150
                            else                       nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 5;// 180
                            break;
        // no line style is set
        case (MSO_LineStyle)USHRT_MAX:
                            break;
        // erroneously not implemented line style is set
        default: ASSERT( !this, "eLineStyle is not (yet) implemented!" );
        }
    }

    if( USHRT_MAX != nIdx )
    {
        SvxBorderLine aLine;
        aLine.SetColor( rLineColor );

        const WW8_BordersSO& rBorders = nLineTabVer8[ nIdx ];
        aLine.SetOutWidth( rBorders.Out  );
        aLine.SetInWidth ( rBorders.In   );
        aLine.SetDistance( rBorders.Dist );

        for(USHORT nLine = 0; nLine < 4; ++nLine)
            rBox.SetLine(new SvxBorderLine( aLine ), nLine);

        bRet = TRUE;
    }
    return bRet;
}


#define WW8ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

void SwWW8ImplReader::MatchSdrItemsIntoFlySet( SdrObject* pSdrObj,
                                               SfxItemSet& rFlySet,
                                               MSO_LineStyle eLineStyle,
                                               Rectangle& rInnerDist,
                                               BOOL bFixSize )
{   /*
        am Rahmen zu setzende Frame-Attribute
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        SwFmtFrmSize            falls noch nicht gesetzt, hier setzen
        SvxLRSpaceItem          hier setzen
        SvxULSpaceItem          hier setzen
        SvxOpaqueItem           (Derzeit bei Rahmen nicht moeglich! khz 10.2.1999)
        SwFmtSurround           bereits gesetzt
        SwFmtVertOrient         bereits gesetzt
        SwFmtHoriOrient         bereits gesetzt
        SwFmtAnchor             bereits gesetzt
        SvxBoxItem              hier setzen
        SvxBrushItem            hier setzen
        SvxShadowItem           hier setzen
    */

    // 1. GrafikObjekt des Docs?
    if( !pDrawModel )
        GrafikCtor();


    // im Sdr-Objekt eingestellten Attribute greifen
//-/    SfxItemSet aOldSet(pDrawModel->GetItemPool());
//-/    pSdrObj->TakeAttributes( aOldSet, FALSE, FALSE );
    const SfxItemSet& rOldSet = pSdrObj->GetItemSet();

    BOOL bIsAAttrObj = pSdrObj->ISA(SdrAttrObj);
//-/    const XLineAttrSetItem* pLineAttrSetItem
//-/        = bIsAAttrObj ? ((SdrAttrObj*)pSdrObj)->GetLineAttrSetItem() : 0;
//-/    const XFillAttrSetItem* pFillAttrSetItem
//-/        = bIsAAttrObj ? ((SdrAttrObj*)pSdrObj)->GetFillAttrSetItem() : 0;



    // einige Items koennen direkt so uebernommen werden
    const USHORT nDirectMatch = 2;
    static RES_FRMATR __READONLY_DATA aDirectMatch[ nDirectMatch ] =
    {
        RES_LR_SPACE,   // Aussenabstand links/rechts: SvxLRSpaceItem
        RES_UL_SPACE    // Aussenabstand Oben/unten:   SvxULSpaceItem
    };
    const SfxPoolItem* pPoolItem;
    for(USHORT nItem = 0; nItem < nDirectMatch; ++nItem)
        if( SFX_ITEM_SET == rOldSet.GetItemState( aDirectMatch[ nItem ],
                                                  FALSE,
                                                  &pPoolItem) )
        {
            rFlySet.Put( *pPoolItem );
        }


    // jetzt die Umrandung berechnen und die Box bauen:
    //       Das Mass wird fuer die Rahmen-GROESSE benoetigt!
    SvxBoxItem aBox;
    // dashed oder solid wird zu solid
    USHORT nLineWidth = 0;

    // check if LineStyle is *really* set!
    const SfxPoolItem* pItem;

//-/    if( pLineAttrSetItem )
    {
//-/        SfxItemState eState = pLineAttrSetItem->GetItemSet().GetItemState(
//-/                                                              XATTR_LINESTYLE,
//-/                                                              TRUE, &pItem );
        SfxItemState eState = rOldSet.GetItemState(XATTR_LINESTYLE,TRUE, &pItem );
        if( eState == SFX_ITEM_SET )
        {
            // Now, that we know there is a line style we will make use the
            // parameter given to us when calling the method...  :-)
            /*
            const XLineStyle eLineStyle =
                                ((const XLineStyleItem*)pItem)->GetValue();
            if( XLINE_NONE != eLineStyle )

            {
            */
//-/                const Color aLineColor
//-/                                = WW8ITEMVALUE( pLineAttrSetItem->GetItemSet(),
//-/                                                XATTR_LINECOLOR,
//-/                                                XLineColorItem );

//-/                nLineWidth
//-/                    = (USHORT)(WW8ITEMVALUE( pLineAttrSetItem->GetItemSet(),
//-/                                             XATTR_LINEWIDTH, XLineWidthItem ));

                const Color aLineColor = WW8ITEMVALUE(rOldSet, XATTR_LINECOLOR, XLineColorItem);
                nLineWidth = (USHORT)(WW8ITEMVALUE(rOldSet, XATTR_LINEWIDTH, XLineWidthItem));

                if( !nLineWidth )
                    nLineWidth = 15; // WW-default: 0.75 pt

                MatchSdrBoxIntoFlyBoxItem( aLineColor, eLineStyle, nLineWidth, aBox);
            //}
        }
    }

    // set distances from box's border to text contained within the box
    if( 0 < rInnerDist.Left() )
        aBox.SetDistance( (USHORT)rInnerDist.Left(), BOX_LINE_LEFT );
    if( 0 < rInnerDist.Top() )
        aBox.SetDistance( (USHORT)rInnerDist.Top(), BOX_LINE_TOP );
    if( 0 < rInnerDist.Right() )
        aBox.SetDistance( (USHORT)rInnerDist.Right(), BOX_LINE_RIGHT );
    if( 0 < rInnerDist.Bottom() )
        aBox.SetDistance( (USHORT)rInnerDist.Bottom(), BOX_LINE_BOTTOM );

    // Groesse: SwFmtFrmSize
    if( SFX_ITEM_SET != rFlySet.GetItemState(RES_FRM_SIZE, FALSE) )
    {
        const Rectangle& rSnapRect = pSdrObj->GetSnapRect();
        // ggfs. Breite und Position des Rahmens anpassen:
        // Der beschreibbare Innenraum soll trotz breitem Rand
        // gleich gross bleiben.
        long nWidth  = rSnapRect.GetWidth()  + 2*nLineWidth;
        long nHeight = rSnapRect.GetHeight() + 2*nLineWidth;
        rFlySet.Put( SwFmtFrmSize(bFixSize ? ATT_FIX_SIZE : ATT_MIN_SIZE,
                        rSnapRect.GetWidth()  + 2*nLineWidth,
                        rSnapRect.GetHeight() + 2*nLineWidth) );
    }

    /*
        Traurig: das SvxOpaqueItem macht bei Rahmen etwas ganz anderes,
        als in /svx/inc/opaqitem.hxx angegeben:

        Statt den Rahmen (un)durchsichtig zu machen, bewirkt es, dass er
        einfach bloss in einen anderen Layer (Hell/Heaven) gestellt wird.
        Fuer unsere Zwecke ist es also nicht zu gebrauchen.

        Da auch das Transparent-Flag am SvxBrushItem wirkungslos ist,
        koennen wir das WinWord-Attribut "Halbtransparent" leider nicht
        umsetzen.

        :-((
    */
    // Durchsichtigkeit: SvxOpaqueItem

    //  INT16 nFillTransparence
    //      = WW8ITEMVALUE(aOldSet, XATTR_FILLTRANSPARENCE, XFillTransparenceItem);
    //  if( 0x032 <= nFillTransparence )
    //      rFlySet.Put( SvxOpaqueItem(RES_OPAQUE, FALSE) );
    //

    // jetzt die Umrandung setzen
    rFlySet.Put( aBox );

    // Schattenwurf der Box: SvxShadowItem
    if( WW8ITEMVALUE(rOldSet, SDRATTR_SHADOW, SdrShadowItem) )
    {
        SvxShadowItem aShadow;

        const Color aShdColor = WW8ITEMVALUE(rOldSet, SDRATTR_SHADOWCOLOR,
                                                        SdrShadowColorItem);

        const INT32 nShdDistX = WW8ITEMVALUE(rOldSet, SDRATTR_SHADOWXDIST,
                                                        SdrShadowXDistItem);
        const INT32 nShdDistY = WW8ITEMVALUE(rOldSet, SDRATTR_SHADOWYDIST,
                                                        SdrShadowYDistItem);

        const USHORT nShdTrans= WW8ITEMVALUE(rOldSet, SDRATTR_SHADOWTRANSPARENCE,
                                                        SdrShadowTransparenceItem);
        // diese gibt es im Writer nicht  :-(
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        //
        // SfxVoidItem( SDRATTR_SHADOW3D    )
        // SfxVoidItem( SDRATTR_SHADOWPERSP )

        aShadow.SetColor( Color( aShdColor ) );

        aShadow.SetWidth( (Abs( nShdDistX ) + Abs( nShdDistY )) / 2 );

        SvxShadowLocation eShdPosi;
        if( 0 <= nShdDistX )
        {
            if( 0 <= nShdDistY )
                eShdPosi = SVX_SHADOW_BOTTOMRIGHT;
            else
                eShdPosi = SVX_SHADOW_TOPRIGHT;
        }
        else
        {
            if( 0 <= nShdDistY )
                eShdPosi = SVX_SHADOW_BOTTOMLEFT;
            else
                eShdPosi = SVX_SHADOW_TOPLEFT;
        }
        aShadow.SetLocation( eShdPosi );

        rFlySet.Put( aShadow );
    }

    // Hintergrund: SvxBrushItem
//-/    if( pFillAttrSetItem )
    {
//-/        SfxItemState eState = pFillAttrSetItem->GetItemSet().GetItemState(
//-/                                                              XATTR_FILLSTYLE,
//-/                                                              TRUE, &pItem );
        SfxItemState eState = rOldSet.GetItemState(XATTR_FILLSTYLE, TRUE, &pItem);
        if( eState == SFX_ITEM_SET )
        {

            const XFillStyle eFillStyle =
                                ((const XFillStyleItem*)pItem)->GetValue();

            if(eFillStyle != XFILL_NONE)
            {
                SvxBrushItem aBrushItem;
                BOOL bBrushItemOk = FALSE;

                switch( eFillStyle )
                {
                case XFILL_NONE     :
                    {
                        aBrushItem.SetColor( Color( COL_TRANSPARENT ) );
                        bBrushItemOk = TRUE;
                    }break;
                case XFILL_SOLID    :
                    {
//-/                        const Color aColor =
//-/                                WW8ITEMVALUE( pFillAttrSetItem->GetItemSet(),
//-/                                              XATTR_FILLCOLOR,
//-/                                              XFillColorItem );
                        const Color aColor = WW8ITEMVALUE(rOldSet, XATTR_FILLCOLOR, XFillColorItem);
                        aBrushItem.SetColor( aColor );
                        bBrushItemOk = TRUE;
                    }break;
                case XFILL_GRADIENT :
                                    break;
                case XFILL_HATCH    :
                                    break;
                case XFILL_BITMAP   :
                    {
//-/                        const Graphic aGraphic(
//-/                                WW8ITEMVALUE( pFillAttrSetItem->GetItemSet(),
//-/                                              XATTR_FILLBITMAP,
//-/                                              XFillBitmapItem ).GetBitmap() );

//-/                        BOOL bTile =
//-/                                WW8ITEMVALUE( pFillAttrSetItem->GetItemSet(),
//-/                                              XATTR_FILLBMP_TILE,
//-/                                              SfxBoolItem );

                        const Graphic aGraphic(WW8ITEMVALUE(rOldSet, XATTR_FILLBITMAP, XFillBitmapItem).GetBitmap());
                        BOOL bTile = WW8ITEMVALUE(rOldSet, XATTR_FILLBMP_TILE, SfxBoolItem);
                        aBrushItem.SetGraphic( aGraphic );
                        aBrushItem.SetGraphicPos(  bTile
                                                 ? GPOS_TILED
                                                 : GPOS_AREA );
                        bBrushItemOk = TRUE;
                    }break;
                }
                if( bBrushItemOk )
                    rFlySet.Put( aBrushItem, RES_BACKGROUND );
            }
        }
    }

}


void SwWW8ImplReader::MatchWrapDistancesIntoFlyFmt( SvxMSDffImportRec* pRecord,
                                                    SwFrmFmt*          pFlyFmt )
{
    if( pRecord->nDxWrapDistLeft || pRecord->nDxWrapDistRight )
    {
        SvxLRSpaceItem aLR;
        aLR.SetLeft(    (USHORT)pRecord->nDxWrapDistLeft );
        aLR.SetRight(   (USHORT)pRecord->nDxWrapDistRight );
        pFlyFmt->SetAttr( aLR );
    }
    if( pRecord->nDyWrapDistTop || pRecord->nDyWrapDistBottom )
    {
        SvxULSpaceItem aUL;
        aUL.SetUpper(   (USHORT)pRecord->nDyWrapDistTop );
        aUL.SetLower(   (USHORT)pRecord->nDyWrapDistBottom );
        pFlyFmt->SetAttr( aUL );
    }
}


void SwWW8ImplReader::SetCropAtGrfNode( SvxMSDffImportRec* pRecord,
                                        SwFrmFmt*          pFlyFmt,
                                        WW8_FSPA*          pF )
{
    if( pRecord->nCropFromTop ||
        pRecord->nCropFromBottom ||
        pRecord->nCropFromLeft ||
        pRecord->nCropFromRight )
    {

        const SwNodeIndex* pIdx = pFlyFmt->GetCntnt( FALSE ).GetCntntIdx();
        SwGrfNode* pGrfNd;
        if( pIdx && 0 != (pGrfNd = rDoc.GetNodes()[ pIdx->GetIndex()
                                                        + 1 ]->GetGrfNode() ))
        {
            Size aSz( pGrfNd->GetTwipSize() );
            ULONG rHeight = aSz.Height();
            ULONG rWidth  = aSz.Width();
            if( !rWidth )
                rWidth  = pF->nXaRight  - pF->nXaLeft;
            else if( !rHeight )
                rHeight = pF->nYaBottom - pF->nYaTop;
#if SUPD>601

            SwCropGrf aCrop;            // Cropping is stored in 'fixed floats'
                                        // 16.16 (it est fraction times total
            if( pRecord->nCropFromTop ) //        image width or height resp.)
                aCrop.SetTop(
                (   ( (pRecord->nCropFromTop    >> 16   ) * rHeight )
                  + (((pRecord->nCropFromTop    & 0xffff) * rHeight ) >> 16) ));
            if( pRecord->nCropFromBottom )
                aCrop.SetBottom(
                (   ( (pRecord->nCropFromBottom >> 16   ) * rHeight )
                  + (((pRecord->nCropFromBottom & 0xffff) * rHeight ) >> 16) ));
            if( pRecord->nCropFromLeft )
                aCrop.SetLeft(
                (   ( (pRecord->nCropFromLeft   >> 16   ) * rWidth  )
                  + (((pRecord->nCropFromLeft   & 0xffff) * rWidth  ) >> 16) ));
            if( pRecord->nCropFromRight )
                aCrop.SetRight(
                (   ( (pRecord->nCropFromRight  >> 16   ) * rWidth  )
                  + (((pRecord->nCropFromRight  & 0xffff) * rWidth  ) >> 16) ));

            pGrfNd->SetAttr( aCrop );
#endif
        }
    }
}



SdrObject* SwWW8ImplReader::CreateContactObject( SwFlyFrmFmt* pFlyFmt )
{
    if( pFlyFmt )
    {
        SdrObject* pNewObject = pFlyFmt->FindSdrObject();
        if( !pNewObject )
        {
            SwFlyDrawContact* pContactObject
                = new SwFlyDrawContact( pFlyFmt, pDrawModel);
            pNewObject = pContactObject->GetMaster();
        }
        return pNewObject;
    }
    return 0;
}


void SwWW8ImplReader::ProcessEscherAlign( SvxMSDffImportRec* pRecord,
                                          WW8_FSPA&          rFSPA,
                                          SfxItemSet&        rFlySet )
{
    if( pRecord )
    {
        // nXAlign - abs. Position, Left,  Centered,  Right,  Inside, Outside
        // nYAlign - abs. Position, Top,   Centered,  Bottom, Inside, Outside

        // nXRelTo - Page printable area, Page,  Column,    Character
        // nYRelTo - Page printable area, Page,  Paragraph, Line

        const int nCntXAlign = 6;
        const int nCntYAlign = 6;

        const int nCntRelTo  = 4;

/*
        // our anchor settings
        static const RndStdIds aAnchorTab[] = {
            FLY_AT_CNTNT,   // Frame bound to paragraph
            FLY_IN_CNTNT,   //             to character
            FLY_PAGE,       //             to page
            FLY_AT_FLY,     //             to another fly ( LAYER_IMPL )
            FLY_AUTO_CNTNT, // automat. positioned frame bound to paragraph
        };
*/

        // horizontal Adjustment
        static const SwHoriOrient aHoriOriTab[ nCntXAlign ] = {
            HORI_NONE,      // Value of nXPos defined RelPos directly.

            HORI_LEFT,      // automatical adjustment
            HORI_CENTER,    // automatical adjustment
            HORI_RIGHT,     // automatical adjustment

            HORI_LEFT,      // will be converted to HORI_INSIDE when SetPosToggle() called
            HORI_RIGHT      // will be converted to HORI_OUTSIDE...
        };


        // vertical Adjustment
        static const SwVertOrient aVertOriTab[ nCntYAlign ] = {
            VERT_NONE,          // Value of nXPos defined RelPos directly.
            VERT_TOP,           // automatical adjustment
            VERT_CENTER,        // automatical adjustment
            VERT_BOTTOM,        // automatical adjustment
            VERT_LINE_TOP,      // automatical adjustment
            VERT_LINE_BOTTOM    // automatical adjustment
        };

        // Adjustment is relative to...
        static const SwRelationOrient aRelOriTab[ nCntRelTo ] = {
            REL_PG_PRTAREA, // Page printable area, when bound to page. identical with PRTAREA
            REL_PG_FRAME,   // Page,                when bound to page. identical with FRAME
//          FRAME,          // Paragraph printable area
            PRTAREA,        // Paragraph
            REL_CHAR        // to a Character

//          REL_PG_LEFT,    // in left page-border
//          REL_PG_RIGHT,   // in right page-border
//          REL_FRM_LEFT,   // in left paragraph-border
//          REL_FRM_RIGHT,  // in right paragraph-border
        };


        UINT16 nXAlign = nCntXAlign > pRecord->nXAlign ? pRecord->nXAlign : 1;
        UINT16 nYAlign = nCntYAlign > pRecord->nYAlign ? pRecord->nYAlign : 1;

        UINT16 nXRelTo = nCntRelTo > pRecord->nXRelTo ? pRecord->nXRelTo : 1;
        UINT16 nYRelTo = nCntRelTo > pRecord->nYRelTo ? pRecord->nYRelTo : 1;

        RndStdIds        eAnchor;
        SwHoriOrient     eHoriOri;
        SwVertOrient     eVertOri;
        SwRelationOrient eHoriRel;
        SwRelationOrient eVertRel;

        eAnchor = 3 == nXRelTo  ?  FLY_AUTO_CNTNT
                                :  2 <= nYRelTo  ?  FLY_AT_CNTNT
                                                 :  FLY_PAGE;
        eHoriOri = aHoriOriTab[ nXAlign ];
        eVertOri = aVertOriTab[ nYAlign ];

        eHoriRel = aRelOriTab[  nXRelTo ];
        eVertRel = FLY_AUTO_CNTNT == eAnchor ? REL_CHAR : aRelOriTab[  nYRelTo ];

        SwFmtAnchor aAnchor( eAnchor );
        aAnchor.SetAnchor( pPaM->GetPoint() );

        SwFmtHoriOrient aHoriOri( rFSPA.nXaLeft, eHoriOri, eHoriRel );
        if( 4 <= nXAlign )
            aHoriOri.SetPosToggle( TRUE );

        rFlySet.Put( aAnchor );
        rFlySet.Put( aHoriOri );
        rFlySet.Put( SwFmtVertOrient( rFSPA.nYaTop,  eVertOri, eVertRel ) );
    }
}

#pragma optimize("",off)
SwFrmFmt* SwWW8ImplReader::Read_GrafLayer( long nGrafAnchorCp )
{
    SwFrmFmt* pRetFrmFmt = 0;
    if( nIniFlags & WW8FL_NO_GRAFLAYER )
        return pRetFrmFmt;

    ::SetProgressState( nProgress, rDoc.GetDocShell() );     // Update

    nDrawCpO =    pWwFib->ccpText + pWwFib->ccpFtn
                + pWwFib->ccpHdr  + pWwFib->ccpMcr
                + pWwFib->ccpAtn  + pWwFib->ccpEdn;
    if( pPlcxMan->GetManType() == MAN_HDFT ) nDrawCpO += pWwFib->ccpTxbx;

    if( bVer67 )
    {
        if(  bHdFtFtnEdn )          // kann der Writer z.Zt. leider nicht
            return pRetFrmFmt;
    //  if( pFlyPara )              // sicherheitshalber
    //      return;
    //  if( bTable )                // sicherheitshalber
    //      return;

    //  if( bTable )
    //      return;                         // geht z.Zt. nicht


        if( !pDrawModel )           // 1. GrafikObjekt des Docs
            GrafikCtor();

        WW8PLCFspecial* pPF = pPlcxMan->GetFdoa();
        if( !pPF ){
            ASSERT( !this, "+Wo ist die Grafik (1) ?" );
            return pRetFrmFmt;
        }
        long nOldPos = pStrm->Tell();
        if( !pDrawHeight ){                             // 1. Aufruf
            pDrawHeight = new SvShorts;
            nDrawObjOfs = pDrawPg->GetObjCount();
        }
        nDrawXOfs = nDrawYOfs = 0;
        pDrawFmt = rDoc.MakeDrawFrmFmt( WW8_ASCII2STR( "DrawObject" ),
                                        rDoc.GetDfltFrmFmt() );
        ReadGrafLayer1( pPF, nGrafAnchorCp );
        pStrm->Seek( nOldPos );
    }
    else
    {
        WW8PLCFspecial* pPF = pPlcxMan->GetFdoa();
        if( !pPF ){
            ASSERT( !this, "+Wo ist die Grafik (1) ?" );
            return pRetFrmFmt;
        }
        pPF->SeekPos( nGrafAnchorCp );

        long nStartFc;
        void* pF0;
        if( !pPF->Get( nStartFc, pF0 ) ){
            ASSERT( !this, "+Wo ist die Grafik (2) ?" );
            return pRetFrmFmt;
        }

        WW8_FSPA_SHADOW* pFS = (WW8_FSPA_SHADOW*)pF0;
        WW8_FSPA*        pF;
#ifdef __WW8_NEEDS_COPY
        WW8_FSPA aFSFA;
        pF = &aFSFA;
        WW8FSPAShadowToReal( pFS, pF );
#else
        pF = (WW8_FSPA*)pFS;
#endif // defined __WW8_NEEDS_COPY
        if( !pF->nSpId )
        {
            ASSERT( !this, "+Wo ist die Grafik (3) ?" );
            return pRetFrmFmt;
        }

        if( !pDrawModel )// 1. GrafikObjekt des Docs
            GrafikCtor();

        if( !pMSDffManager->GetModel() )
             pMSDffManager->SetModel(pDrawModel, 1440);

        SdrObject* pObject = 0;

        Rectangle aRect(pF->nXaLeft,  pF->nYaTop,
                        pF->nXaRight, pF->nYaBottom);
        SvxMSDffImportData aData( aRect );

        if(     pMSDffManager->GetShape( pF->nSpId, pObject, aData )
            &&  pObject )
        {
            const SdrObject* pOrgShapeObject = pObject;
            SvxMSDffImportRec* pRecord;
            BOOL bDone = FALSE;
            SdrObject* pOurNewObject = 0;
            BOOL bSetCrop     = FALSE;
            BOOL bOrgObjectWasReplace
                = (    (SdrInventor == pObject->GetObjInventor())
                    && (    (UINT16( OBJ_GRAF ) == pObject->GetObjIdentifier())
                         || (UINT16( OBJ_OLE2 ) == pObject->GetObjIdentifier()) ) );

            //FormControl Text, cmc
            UINT32 nTextId =
                pMSDffManager->GetPropertyValue(DFF_Prop_pictureId,0);
            if (nTextId)
                bOrgObjectWasReplace=TRUE;


            // Umfluss-Modus ermitteln
            SfxItemSet aFlySet( rDoc.GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
            SwSurround eSurround = SURROUND_PARALLEL;
            BOOL bContour = FALSE;
            switch( pF->nwr )
            {
            case 0: ;                                       //0 like 2, but doesn't require absolute object
            case 2: eSurround = SURROUND_PARALLEL;  break;  //2 wrap around absolute object
            case 1: eSurround = SURROUND_NONE;      break;  //1 no text next to shape
            case 3: eSurround = SURROUND_THROUGHT;  break;  //3 wrap as if no object present
            case 4: ;                                       //4 wrap tightly around object
            case 5:
                {
                    eSurround = SURROUND_PARALLEL;          //5 wrap tightly, but allow holes

                    // ensure the object will be conterted
                    // into a Writer structure!
                    // the Writer doesn't support Contour on Sdr objects
                    if( bOrgObjectWasReplace )
                        bContour = TRUE;
                }
                break;
            }
            // bei Modus 2 oder 4 auch den Zusatzparameter beruecksichtigen
            if( ( 2 == pF->nwr ) || ( 4 == pF->nwr ) )
            {
                switch( pF->nwrk )
                {
                case 0: eSurround = SURROUND_PARALLEL;  break;  //0 wrap both sides
                case 1: eSurround = SURROUND_LEFT;      break;  //1 wrap only on left
                case 2: eSurround = SURROUND_RIGHT;     break;  //2 wrap only on right
                case 3: eSurround = SURROUND_IDEAL;     break;  //3 wrap only on largest side
                }
            }
            SwFmtSurround aSur( eSurround );
            aSur.SetContour( bContour );
            aSur.SetOutside( TRUE ); // Winword kann nur Aussen-Konturen
            aFlySet.Put( aSur );

            if( bOrgObjectWasReplace )
            {
                pRecord = (    aData.HasRecords()
                            && (1 == aData.GetRecCount() ) )
                        ? aData.GetRecord( 0 ) : 0;

                long nWidthTw  = pF->nXaRight -pF->nXaLeft;
                if( 0>nWidthTw ) nWidthTw =0;
                long nHeightTw = pF->nYaBottom - pF->nYaTop;
                if( 0>nHeightTw) nHeightTw=0;

                if( nbxRelPageBorder == pF->nbx )
                {
                    pF->nXaLeft -= nPgLeft;
                    if( bTable )
                        pF->nXaLeft -= GetTableLeft();
                }
/*
                RndStdIds        eAnchor;
                SwHoriOrient     eHori;
                SwVertOrient     eVert;
                SwRelationOrient eRel;

                aFlySet.Put( SwFmtAnchor( eAnchor ) );//FLY_AT_CNTNT ) );

                aFlySet.Put(
                    SwFmtHoriOrient( pF->nXaLeft, eHori, eRel ) );//HORI_NONE, FRAME ) );
                aFlySet.Put(
                    SwFmtVertOrient( pF->nYaTop,  eVert, eRel ) );//VERT_NONE, FRAME ) );
*/
                ProcessEscherAlign( pRecord, *pF, aFlySet );

                aFlySet.Put( SwFmtFrmSize( ATT_VAR_SIZE, nWidthTw, nHeightTw ));

                if( pRecord )
                {
                    Rectangle aInnerDist(   pRecord->nDxTextLeft,
                                            pRecord->nDyTextTop,
                                            pRecord->nDxTextRight,
                                            pRecord->nDyTextBottom  );

                    MatchSdrItemsIntoFlySet( pObject,
                                             aFlySet,
                                             pRecord->eLineStyle,
                                             aInnerDist,
                                             !pRecord->bLastBoxInChain );
                }

                XubString aObjectName( pObject->GetName() );
                SwFrmFmt *pControl=NULL;
                if( UINT16( OBJ_OLE2 ) == pObject->GetObjIdentifier() )
                {
                    SvInPlaceObjectRef xIPRef(
                                ((SdrOle2Obj*)pObject)->GetObjRef() );

                    // kein GrafSet uebergeben, da nur fuer Cropping sinnvoll, was die
                    // UI derzeit (27.1.99) noch nicht kann khz.
                    pRetFrmFmt = rDoc.Insert( *pPaM, &xIPRef, &aFlySet );
                }
                else if (nTextId) //i.e. This is possibly a FormControl or graphic
                {
                    UINT16 nTxBxS = (UINT16)(nTextId >> 16);
                    UINT16 nSequence = (UINT16)nTextId;
                    pRecord = aData.GetRecord(0);

                    SdrObject* pTrueObject
                        = (    bOrgObjectWasReplace
                            && (pOrgShapeObject == pRecord->pObj) )
                        ? pOurNewObject
                        : pRecord->pObj;

                    if( bOrgObjectWasReplace && pTrueObject )
                    {
                        if( pF->bBelowText )
                            pTrueObject->SetLayer( nDrawHell );
                        else
                            pTrueObject->SetLayer( nDrawHeaven );
                    }

                    if( pTrueObject == pRecord->pObj )
                    {
                        MatchWrapDistancesIntoFlyFmt(    pRecord, pRetFrmFmt );
                        if( bSetCrop )
                            SetCropAtGrfNode( pRecord, pRetFrmFmt, pF );
                    }

                    SdrTextObj* pSdrTextObj =
                        PTR_CAST(SdrTextObj,pRecord->pObj);

                    if( pSdrTextObj )
                    {
                        Size aObjSize(
                            pSdrTextObj->GetSnapRect().GetWidth(),
                            pSdrTextObj->GetSnapRect().GetHeight());

                        BOOL bEraseThisObject=FALSE;
                        BOOL bOldFloatingCtrl=bFloatingCtrl;
                        bFloatingCtrl=TRUE;
                        pControl = InsertTxbxText(
                            pSdrTextObj,
                            &aObjSize,
                            nTxBxS,
                            nSequence,
                            nGrafAnchorCp,
                            pRetFrmFmt,
                            FALSE,
                            bEraseThisObject,
                            FALSE,0,0,
                            pRecord );

                        if ((pControl) && (RES_DRAWFRMFMT==pControl->Which()))
                        {
                            SdrObject *pObj = pControl->FindSdrObject();
                            if ((pObj) && (FmFormInventor == pObj->GetObjInventor()))
                            {
                                /*have a control*/
                                pControl->SetAttr(aFlySet.Get(RES_VERT_ORIENT));
                                pControl->SetAttr(aFlySet.Get(RES_HORI_ORIENT));
                            }
                        }
                        bFloatingCtrl=bOldFloatingCtrl;
                    bDone=TRUE;
                    }
                }


                if ((UINT16( OBJ_GRAF ) == pObject->GetObjIdentifier()) && (!pControl))
                {
                    const Graphic& rGraph = ((SdrGrafObj*)pObject)->GetGraphic();
                    bSetCrop = TRUE;
                    BOOL bDone2 = FALSE;

                    if( ((SdrGrafObj*)pObject)->IsLinkedGraphic() )
                    {
                        String aGrfName(
                            INetURLObject::RelToAbs(
                                ((SdrGrafObj*)pObject)->GetFileName() ) );

                        BOOL bExist = FALSE;
                        INetURLObject aGrURL(URIHelper::SmartRelToAbs(aGrfName));
                        try
                        {
                            ::ucb::Content aTestContent(
                                aGrURL.GetMainURL(),
                                uno::Reference< XCommandEnvironment >());
                            bExist = aTestContent.isDocument();
                        }
                        catch(...){}


                        if(    bExist
                            || (GRAPHIC_NONE == rGraph.GetType()))
                        {
                            pRetFrmFmt = rDoc.Insert( *pPaM,
                                                aGrfName,
                                                aEmptyStr,
                                                0,          // Graphic*
                                                &aFlySet,
                                                0 );        // SwFrmFmt*
                            bDone2 = TRUE;
                        }
                    }
                    if( !bDone2 )
                        pRetFrmFmt = rDoc.Insert( *pPaM,
                                            aEmptyStr,
                                            aEmptyStr,
                                            &rGraph,
                                            &aFlySet,
                                            0 );        // SwFrmFmt*
                    bDone = TRUE;
                }


                if( pRetFrmFmt )
                {
                    if( pRecord )
                    {
                        MatchWrapDistancesIntoFlyFmt(    pRecord, pRetFrmFmt );
                        if( bSetCrop ) SetCropAtGrfNode( pRecord, pRetFrmFmt, pF );
                    }
                    // mehrfaches Auftreten gleicher Grafik-Namen vermeiden
                    if( aObjectName.Len() )
                    {
                        String aName;
                        if( MakeUniqueGraphName( aName, aObjectName ))
                            pRetFrmFmt->SetName( aName );
                    }
                }
                // falls alles Ok, Zeiger auf neues Objekt ermitteln und
                // Z-Order-Liste entsprechend korrigieren (oder Eintrag loeschen)
                pOurNewObject = CreateContactObject( (SwFlyFrmFmt*)pRetFrmFmt );

                // altes Objekt aus der Z-Order-Liste entfernen
                pMSDffManager->RemoveFromShapeOrder( pObject );
                // aus der Drawing-Page rausnehmen
                if( pObject->GetPage() )
                    pDrawPg->RemoveObject( pObject->GetOrdNum() );

                // und das Objekt loeschen
                DELETEZ( pObject );
                /*
                    Achtung: ab jetzt nur noch pOrgShapeObject abfragen!
                */


                // Kontakt-Objekt in die Z-Order-Liste und die Page aufnehmen
                if( pOurNewObject )
                {
                    if( !bHdFtFtnEdn )
                        pMSDffManager->StoreShapeOrder( pF->nSpId, 0, pOurNewObject, 0 );
                    // Das Kontakt-Objekt MUSS in die Draw-Page gesetzt werden,
                    // damit in SwWW8ImplReader::LoadDoc1() die Z-Order
                    // festgelegt werden kann !!!
                    pDrawPg->InsertObject( pOurNewObject );
                }
            }
            else
            {
                // eingelesenes Objekt (kann eine ganze Gruppe sein)
                // jetzt korrekt positionieren usw.
                pRecord = (    aData.HasRecords()
                            && (1 == aData.GetRecCount() ) )
                        ? aData.GetRecord( 0 ) : 0;


                if( pF->bRcaSimple )
                {
                    pF->nbx = nbxRelPageBorder;
                    pF->nby = nbyRelPageBorder;
                }

                RndStdIds eAnchor = FLY_AT_CNTNT;  //FLY_PAGE
                if( (nbyRelText != pF->nby) )
                {
                    if( bIsHeader || bIsFooter)
                    {
                        if( bIsHeader && (nPgTop < pF->nYaTop))
                            pF->nYaTop -= nPgTop;
                        else
                            pF->nYaTop = 0;
                        pNode_FLY_AT_CNTNT = &pPaM->GetPoint()->nNode.GetNode();
                    }
                    else
                        eAnchor = FLY_PAGE;
                }


/*
                SwHoriOrient     eHori;
                SwVertOrient     eVert;
                SwRelationOrient eRel;
                ProcessEscherAlign( pRecord, eAnchor, eHori, eVert, eRel );
*/
                ProcessEscherAlign( pRecord, *pF, aFlySet );
/*


                SwFmtAnchor aAnchor( eAnchor );
                aAnchor.SetAnchor( pPaM->GetPoint() );

                aFlySet.Put( aAnchor );



                //  Hilfs-Attribute setzen, damit MA die Werte im Layout umrechnen kann
                //  ( bugdoc:59640 )

                static SwRelationOrient __READONLY_DATA aRelOrientTab[] = {
                        REL_PG_PRTAREA,         // == nbxRelPgMargin
                        REL_PG_FRAME,           // == nbxRelPageBorder
                        FRAME                   // == nbxRelText
                };

                aFlySet.Put( SwFmtVertOrient( pF->nYaTop, eVert, eRel ));

                SwFmtHoriOrient aHoriOri( pF->nXaLeft, eHori, eRel );
                if( HORI_INSIDE <= eHori )
                    aHoriOri.SetPosToggle( TRUE );
                aFlySet.Put( aHoriOri );
*/


                if( !(nIniFlags1 & WW8FL_NO_FLY_FOR_TXBX) ) // Wer nicht will, der hat gewollt!
                {
                    BOOL bTextThere = FALSE;
                    long nStartCpFly;
                    long nEndCpFly;
                    if( pRecord && pRecord->bReplaceByFly )
                    {
                        // Pruefen, ob in dieser Textbox-Kette denn Text enthalten ist.
                        //   ( Umwandeln einer leeren Kette in Rahmen waere Unsinn. )
                        //
                        bTextThere = TxbxChainContainsRealText( pRecord->aTextId.nTxBxS,
                                                                nStartCpFly,
                                                                nEndCpFly );
                        if( bTextThere )
                        {
                            // Der Text wird nicht in das SdrTextObj eingelesen!
                            // Stattdessen wird ein Rahmen eingefuegt und der Text
                            // von nStartCpFly bis nEndCpFy dort hinein gelesen.
                            //
                            // Vorteil: im Rahmen sind viel mehr Attribute moeglich
                            //          als in der Edit-Enging, und es koennen
                            //          auch Felder, OLEs oder Grafiken darin sein...

                            Rectangle aInnerDist(   pRecord->nDxTextLeft,
                                                    pRecord->nDyTextTop,
                                                    pRecord->nDxTextRight,
                                                    pRecord->nDyTextBottom  );
                            MatchSdrItemsIntoFlySet( pObject,
                                                     aFlySet,
                                                     pRecord->eLineStyle,
                                                     aInnerDist,
                                                     !pRecord->bLastBoxInChain );

                            pRetFrmFmt = rDoc.MakeFlySection( eAnchor, pPaM->GetPoint(), &aFlySet );

                            MatchWrapDistancesIntoFlyFmt( pRecord, pRetFrmFmt );

                            // falls alles Ok, Zeiger auf neues Objekt ermitteln und
                            // Z-Order-Liste entsprechend korrigieren (oder Eintrag loeschen)
                            pOurNewObject = CreateContactObject( (SwFlyFrmFmt*)pRetFrmFmt );

                            // altes Objekt aus der Z-Order-Liste entfernen
                            pMSDffManager->RemoveFromShapeOrder( pObject );

                            // und das Objekt loeschen
                            DELETEZ( pObject );
                            /*
                                Achtung: ab jetzt nur noch pOrgShapeObject abfragen!
                            */


                            if( pOurNewObject )
                            {
                                pMSDffManager->StoreShapeOrder(
                                    pF->nSpId,
                                      (((ULONG)pRecord->aTextId.nTxBxS) << 16)
                                    + pRecord->aTextId.nSequence,
                                    pOurNewObject,
                                    (SwFlyFrmFmt*)pRetFrmFmt,
                                      nActSectionNo
                                    + bIsHeader ? 1 : 0
                                    + bIsFooter ? 2 : 0 );
                                // Das Kontakt-Objekt MUSS in die Draw-Page gesetzt werden,
                                // damit in SwWW8ImplReader::LoadDoc1() die Z-Order
                                // festgelegt werden kann !!!
                                pDrawPg->InsertObject( pOurNewObject );
                            }

                            // Damit die Frames bei Einfuegen in existierendes Doc erzeugt werden,
                            // wird in fltshell.cxx beim Setzen des FltAnchor-Attributes
                            // pFlyFrm->MakeFrms() gerufen
                            if( FLY_IN_CNTNT != eAnchor )
                                pCtrlStck->NewAttr( *pPaM->GetPoint(), SwFltAnchor( pRetFrmFmt ) );

                            // Box-0 erhaelt den Text fuer die ganze Kette!
                            if( !pRecord->aTextId.nSequence )
                            {
#if 0
                                // merke Pos im Haupttext
                                SwPosition aMainTextPos( *pPaM->GetPoint() );

                                // schliesse Attribute auf dem End-Stack
                                pEndStck->SetAttr( *pPaM->GetPoint(), 0, FALSE );

                                SwWW8FltControlStack* pOldStck = pCtrlStck;
                                pCtrlStck = new SwWW8FltControlStack( &rDoc, nFieldFlags );

                                // rette die Attributverwaltung
                                WW8PLCFxSaveAll aPLCFxSave;
                                pPlcxMan->SaveAllPLCFx( &aPLCFxSave );
                                WW8PLCFMan* pOldPlcxMan = pPlcxMan;

#endif
                                // rette Flags u.ae. und setze sie zurueck
                                WW8ReaderSave aSave( this );
                                // setze Pam in den FlyFrame
                                const SwFmtCntnt& rCntnt = pRetFrmFmt->GetCntnt();
                                ASSERT( rCntnt.GetCntntIdx(), "Kein Inhalt vorbereitet." );
                                pPaM->GetPoint()->nNode = rCntnt.GetCntntIdx()->GetIndex() + 1;
                                pPaM->GetPoint()->nContent.Assign( pPaM->GetCntntNode(), 0 );


                                // lies den Text ein
                                bTxbxFlySection = TRUE;
                                ReadText( nStartCpFly,
                                          (nEndCpFly-nStartCpFly),
                                          MAN_MAINTEXT == pPlcxMan->GetManType()
                                        ? MAN_TXBX
                                        : MAN_TXBX_HDFT );

#if 0
                                // schliesse alle Attribute, da sonst Attribute
                                // entstehen koennen, die aus dem Fly rausragen
                                pCtrlStck->SetAttr( *pPaM->GetPoint(), 0, FALSE );
                                pEndStck->SetAttr(  *pPaM->GetPoint(), 0, FALSE );

                                // stelle  die Writer-Flags wieder her
                                aSave.Restore( this );
                                DeleteCtrlStk();
                                pCtrlStck = pOldStck;

                                *pPaM->GetPoint() = aMainTextPos;

                                // restauriere die Attributverwaltung
                                pPlcxMan = pOldPlcxMan;
                                pPlcxMan->RestoreAllPLCFx( &aPLCFxSave );
#else
                                aSave.Restore( this );
#endif
                            }
                            bDone = TRUE;
                        }
                    }
                }
                if( !bDone )
                {
                    if( pF->bBelowText )
                        pObject->SetLayer( nDrawHell );
                    else
                        pObject->SetLayer( nDrawHeaven );

                    pDrawPg->InsertObject( pObject );
                    pRetFrmFmt = rDoc.Insert( *pPaM, *pObject, &aFlySet );
                }
            }


            /*
                Innen- und Aussen-Rand-Abstaende einstellen
                und ggfs. Text in enthaltene Textbox(en) einlesen
            */
            if( !bDone && aData.HasRecords() )
            {
                USHORT nRecCount = aData.GetRecCount();
                for(USHORT nTxbx=0; nTxbx < nRecCount; nTxbx++ )
                {
                    pRecord = aData.GetRecord( nTxbx );
                    if( pRecord && pRecord->pObj )
                    {
                        SdrObject* pTrueObject
                            = (    bOrgObjectWasReplace
                                && (pOrgShapeObject == pRecord->pObj) )
                            ? pOurNewObject
                            : pRecord->pObj;

                        if( bOrgObjectWasReplace && pTrueObject )
                        {
                            if( pF->bBelowText )
                                pTrueObject->SetLayer( nDrawHell );
                            else
                                pTrueObject->SetLayer( nDrawHeaven );
                        }

                        if( pTrueObject == pRecord->pObj )
                        {
                            MatchWrapDistancesIntoFlyFmt(    pRecord, pRetFrmFmt );
                            if( bSetCrop ) SetCropAtGrfNode( pRecord, pRetFrmFmt, pF );

                            /*
                            if( pRecord->pClientDataBuffer )
                            {
                                //Auswertung des  Client Data Puffers

                            }
                            */
                        }

                        if( pRecord->aTextId.nTxBxS && !bOrgObjectWasReplace )
                        {
                            SdrTextObj* pSdrTextObj;
                            // Pruefen, ob Gruppenobjekt (z.B. zwei Klammern) vorliegt
                            SdrObjGroup* pThisGroup
                                = PTR_CAST(SdrObjGroup, pRecord->pObj);
                            //if(pRecord->pObj->ISA(SdrObjGroup))
                            if( pThisGroup )
                            {
                                // Gruppenobjekte haben keinen Text. Fuege ein Textobjekt in die
                                // Gruppe ein, um den Text zu halten.
                                pSdrTextObj =
                                    new SdrRectObj(OBJ_TEXT, pThisGroup->GetBoundRect());

                                SfxItemSet aSet(pDrawModel->GetItemPool());
                                aSet.Put(XFillStyleItem(XFILL_NONE));
                                aSet.Put(XLineStyleItem(XLINE_NONE));
                                //SdrFitToSizeType eFTS = SDRTEXTFIT_PROPORTIONAL;
                                /*
                                aSet.Put(SdrTextVertAdjustItem( SDRTEXTVERTADJUST_TOP ));
                                aSet.Put(SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_LEFT ));
                                */
                                aSet.Put(SdrTextFitToSizeTypeItem( SDRTEXTFIT_NONE ));
                                aSet.Put(SdrTextAutoGrowHeightItem( FALSE ));
                                aSet.Put(SdrTextAutoGrowWidthItem(  FALSE ));
//-/                                pSdrTextObj->NbcSetAttributes(aSet, FALSE);
                                pSdrTextObj->SetItemSet(aSet);

                                long nAngle = pRecord->nTextRotationAngle;
                                if(  nAngle )
                                {
                                    double a = nAngle*nPi180;
                                    pSdrTextObj->NbcRotate( pSdrTextObj->GetBoundRect().Center(),
                                                            nAngle, sin(a), cos(a) );
                                }

                                pSdrTextObj->NbcSetLayer( pThisGroup->GetLayer() );
                                pThisGroup->GetSubList()->NbcInsertObject( pSdrTextObj );
                            }
                            else
                            {
                                pSdrTextObj = PTR_CAST(SdrTextObj, pRecord->pObj);
                                /*
                                    Die Frage: was tun, wenn hier FALSE hereuskommt,
                                                z.B. bei 3D-Objekten (nicht von SdrTextObj abgeleitet)

                                    Wunsch: neues SdrTextObj hinzufuegen, das mit dem alten
                                            in einer neu zu schaffenden Gruppe geklammert wird.

                                    Implementierung: nicht zur 5.1 (jp und khz am 11.02.1999)

                                if( !pSdrTextObj )
                                {
                                    ...
                                }
                                */
                            }

                            if( pSdrTextObj )
                            {
                                Size aObjSize(
                                        pSdrTextObj->GetSnapRect().GetWidth(),
                                        pSdrTextObj->GetSnapRect().GetHeight());

                                // Objekt ist Bestandteil einer Gruppe?
                                SdrObject* pGroupObject = pSdrTextObj->GetUpGroup();

                                UINT32 nOrdNum = pSdrTextObj->GetOrdNum();
                                BOOL bEraseThisObject;
                                InsertTxbxText( pSdrTextObj,
                                                &aObjSize,
                                                pRecord->aTextId.nTxBxS,
                                                pRecord->aTextId.nSequence,
                                                nGrafAnchorCp,
                                                pRetFrmFmt,
                                                   (pSdrTextObj != pTrueObject)
                                                || (0 != pGroupObject),
                                                bEraseThisObject,
                                                FALSE,0,0,
                                                pRecord );

                                // wurde dieses Objekt ersetzt ??
                                if( bEraseThisObject )
                                {
                                    if(    pGroupObject
                                        || (pSdrTextObj != pTrueObject) )
                                    {
                                        // Objekt wurde bereits (in der Gruppe und)
                                        // der Drawing-Page durch ein neues
                                        // SdrGrafObj ersetzt.

                                        SdrObject* pNewObj =
                                              pGroupObject
                                            ? pGroupObject->GetSubList()->GetObj(
                                                nOrdNum )
                                            : pTrueObject;
                                        // Objekt in der Z-Order-Liste ersetzen
                                        pMSDffManager->ExchangeInShapeOrder(
                                            pSdrTextObj, 0,0, pNewObj );
                                        // Objekt jetzt noch loeschen
                                        delete pRecord->pObj;
                                        // und das neue Objekt merken.
                                        pRecord->pObj = pNewObj;
                                    }
                                    else
                                    {
                                        // Objekt aus der Z-Order-Liste loeschen
                                        pMSDffManager->RemoveFromShapeOrder( pSdrTextObj );
                                        // Objekt aus der Drawing-Page rausnehmen
                                        if( pSdrTextObj->GetPage() )
                                            pDrawPg->RemoveObject( pSdrTextObj->GetOrdNum() );
                                        // und FrameFormat entfernen, da durch Grafik
                                        // ersetzt (dies loescht auch das Objekt)
                                        rDoc.DelFrmFmt( pRetFrmFmt );
                                        // auch den Objektmerker loeschen
                                        pRecord->pObj = 0;
                                    }
                                }
                                else
                                {
                                    // ww8-default Randabstand einsetzen
                                    SfxItemSet aItemSet( pDrawModel->GetItemPool(),
                                                         SDRATTR_TEXT_LEFTDIST,
                                                         SDRATTR_TEXT_LOWERDIST );
                                    aItemSet.Put( SdrTextLeftDistItem(  pRecord->nDxTextLeft   ) );
                                    aItemSet.Put( SdrTextRightDistItem( pRecord->nDxTextRight  ) );
                                    aItemSet.Put( SdrTextUpperDistItem( pRecord->nDyTextTop    ) );
                                    aItemSet.Put( SdrTextLowerDistItem( pRecord->nDyTextBottom ) );
//-/                                    pSdrTextObj->SetAttributes( aItemSet, FALSE );
                                    pSdrTextObj->SetItemSetAndBroadcast(aItemSet);
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            ASSERT( !this, "+Wo ist das Shape ?" );
            return 0;
        }
    }
    return pRetFrmFmt;
}
#pragma optimize("",on)


void SwWW8ImplReader::GrafikCtor()  // Fuer SVDraw und VCControls und Escher
{
    rDoc.MakeDrawModel( );
    pDrawModel  = rDoc.GetDrawModel();ASSERT( pDrawModel,
                                              "Kann DrawModel nicht anlegen" );
    pDrawPg     = pDrawModel->GetPage( 0 );
    nDrawHeaven = rDoc.GetHeavenId();
    nDrawHell   = rDoc.GetHellId();
}

void SwWW8ImplReader::GrafikDtor()
{
    DELETEZ( pDrawEditEngine ); // evtl. von Grafik angelegt
    DELETEZ( pDrawHeight );     // dito
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/ww8graf.cxx,v 1.5 2000-11-23 16:18:18 sj Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.4  2000/11/03 09:35:25  khz
      fault tolerant Winword parameter reading

      Revision 1.3  2000/10/30 12:07:14  aw
      change SdrObjects to use SfxItemSet instead of SfxSetItems.
      Removed TakeAttributes() and SetAttributes(), new ItemSet
      modification methods (GetItem[Set], SetItem[Set], ClearItem,...)

      Revision 1.2  2000/10/16 10:35:05  khz
      read extended WW9-Frame-Alignment (stored in Escher record 0xF122)

      Revision 1.1.1.1  2000/09/18 17:14:58  hr
      initial import

      Revision 1.93  2000/09/18 16:04:59  willem.vandorp
      OpenOffice header added.

      Revision 1.92  2000/08/28 14:55:42  khz
      #64941# store Hd./Ft./Section info to prevent prohibited TxBx linkage

      Revision 1.91  2000/08/24 14:39:04  khz
      #78052# don't store SwFlyDrawContact in ShapeOrder array if bHdFtFtnEdn set

      Revision 1.90  2000/08/24 12:32:54  os
      Outliner/EditEngine changes

      Revision 1.89  2000/08/24 10:48:52  jp
      crop export to svx

      Revision 1.88  2000/06/26 12:59:11  os
      INetURLObject::SmartRelToAbs removed

      Revision 1.87  2000/06/15 15:23:38  cmc
      #75669# 97Controls Import fixes

      Revision 1.86  2000/06/13 08:24:46  os
      using UCB

      Revision 1.85  2000/05/16 12:12:53  jp
      ASS_FALSE define removed

      Revision 1.84  2000/05/16 11:14:00  khz
      Unicode code-conversion

      Revision 1.83  2000/03/08 10:20:26  khz
      Task #73362# WW6: restore stream position after readind content of textbox

      Revision 1.82  2000/03/03 15:20:01  os
      StarView remainders removed

      Revision 1.81  2000/02/22 16:00:02  khz
      Task #72647# Import attributes of winword 6 TEXT OBJECTS

      Revision 1.80  2000/02/22 13:54:32  khz
      Task #73096# look for OLE- or Graphic-Indicator-Sprms

      Revision 1.79  2000/02/22 11:59:02  khz
      Task #73096# erase TextObj only when OLE or Graphic propperly imported

      Revision 1.78  2000/02/18 09:35:05  cmc
      #69372# Improved Hyperlink Importing for WW97


      Revision 1.77  2000/02/14 14:36:32  jp
      #70473# changes for unicode

      Revision 1.76  2000/02/14 08:25:21  cmc
      #72579 TxtBox NULL test crash fix, revealed by glossary addition

      Revision 1.74  2000/02/09 08:57:16  khz
      Task #72647# Read SPRMs that are stored in piece table grpprls (2)

      Revision 1.73  2000/01/14 11:28:03  khz
      Task #68832# -- CORRECTION: Task Number of rev below should have been: 68832 !

      Revision 1.72  2000/01/14 11:11:38  khz
      Task #71343# look for Sprm 37 (not 5) to recognize APO

      Revision 1.71  2000/01/07 12:29:46  khz
      Task #64574# believe Layerflag in Winword Struct rather than the one in Escher Data (2)

      Revision 1.70  2000/01/07 12:27:19  khz
      Task #64574# believe Layerflag in Winword Struct rather than the one in Escher Data

      Revision 1.69  1999/12/14 14:22:45  khz
      Task #70815# Avoid typecast when calling MatchSdrItemsIntoFlySet()

      Revision 1.68  1999/12/06 16:38:08  khz
      Task #67435# Import FillStyle on OLE and Graphic viaSdrAttrObj::GetFillAttrSetItem()

      Revision 1.67  1999/12/01 10:18:36  khz
      Task #67435# Use GetItemState() to find border of graphic

      Revision 1.66  1999/11/23 18:56:17  khz
      Task #67963# New method SvxMSDffManager::ImportObj() used by Draw, Writer, Calc

      Revision 1.65  1999/11/19 15:02:42  khz
      Special Task! Use new method to import Shapes: SvxMSDffManager::ImportObj()

      Revision 1.64  1999/09/20 14:24:27  khz
      Mussaenderung wg. TH apichange 10.09.1999

      Revision 1.63  1999/08/30 19:53:00  JP
      Bug #68219#: no static members - be reentrant


      Rev 1.62   30 Aug 1999 21:53:00   JP
   Bug #68219#: no static members - be reentrant

      Rev 1.61   09 Aug 1999 14:16:02   JP
   read W95 format from stream

      Rev 1.60   27 Jul 1999 14:41:56   KHZ
   Task #67886# Import Textboxes without #14 in it's field code

      Rev 1.59   15 Jul 1999 10:56:00   KHZ
   Task #67659# Do *not* set FillStyle on box with Default fill style

      Rev 1.58   06 Jul 1999 16:38:14   KHZ
   Task #67435# Borderattributes of graphics that are connected to a #1 char

      Rev 1.57   22 Jun 1999 17:40:22   KHZ
   Task #64574# adjust z-order for grouped objects2

      Rev 1.56   09 Jun 1999 18:25:04   KHZ
   use FOUR different inner distances with SvxBoxItem

      Rev 1.55   02 Jun 1999 09:32:24   KHZ
   Task #66227# a) kein Unicode bei Ver67 ;-)  b) Grafik in grupp. Textbox

      Rev 1.54   29 Apr 1999 12:45:40   KHZ
   Task #65561# (versehentlich war Zuweisung von pRecord auskommentiert)

      Rev 1.53   26 Apr 1999 17:40:22   KHZ
   Task #61381# Attribute von Textboxen in denen NUR EINE Grafik steht

      Rev 1.52   10 Mar 1999 18:23:12   KHZ
   Task #63093# Bei FLY_IN_CNTNT nach MakeFlySection *kein* Ankerattribut setzen!

      Rev 1.51   10 Mar 1999 15:11:18   KHZ
   Task #62521# Einlesen von Sonderzeichen

      Rev 1.50   18 Feb 1999 20:43:48   KHZ
   Task #61681# in InsertObjekt() immer SURROUND_TROUGH setzen

      Rev 1.49   18 Feb 1999 20:24:38   KHZ
   Task #61659# Umlauf im Hintergrund

      Rev 1.48   18 Feb 1999 18:52:38   JP
   Bug #61659#/#61660#: Objecte in einer Gruppe NIE im LayoutConnecten

      Rev 1.47   17 Feb 1999 17:34:26   KHZ
   Task #61982# Zugriff auf freigegebenen Speicher nach ReplaceObject

      Rev 1.46   17 Feb 1999 15:24:16   KHZ
   Task #61382# jetzt auch Textbox-Grafiken im Hintergrund ( SvxOpaqItem(FALSE) )

      Rev 1.45   16 Feb 1999 13:03:24   KHZ
   Task #60326# nur das erste von mehreren PicLoc-Attributen einer Char-Pos ist ok

      Rev 1.44   15 Feb 1999 21:40:04   KHZ
   Task #61381# jetzt include von boxitem.hxx fuer WW8_DECL_LINETAB_ARRAY

      Rev 1.43   12 Feb 1999 16:51:06   KHZ
   Task #61381# Ersetzen von Sdr-Text-Objekten im Writer durch Rahmen (3)

      Rev 1.42   05 Feb 1999 19:37:02   KHZ
   Task #61381# Ersetzen von Sdr-Text-Objekten im Writer durch Rahmen (2)

      Rev 1.41   05 Feb 1999 10:33:48   KHZ
   Task #61381# Ersetzen von Sdr-Text-Objekten im Writer durch Rahmen

      Rev 1.40   28 Jan 1999 17:10:46   KHZ
   Task #60715# in SdrTextObj verankerte Grafiken und OLEs (4)

      Rev 1.39   27 Jan 1999 10:58:02   KHZ
   Task #60715# in SdrTextObj verankerte Grafiken und OLEs (2)

      Rev 1.38   25 Jan 1999 10:25:46   KHZ
   Task #60715# in Textobjekt verankerte Grafik als Grafik importieren

      Rev 1.37   19 Jan 1999 10:53:06   KHZ
   Task #60715# in Textbox verankerte Grafiken und OLEs (1)

      Rev 1.36   06 Jan 1999 10:51:52   KHZ
   Task #59858# Testreihenfolge der While-Schleifen geaendert

      Rev 1.35   22 Dec 1998 09:08:38   KHZ
   Task #60395# (4) StoreShapeOrder wieder angeklemmt

      Rev 1.34   21 Dec 1998 16:00:42   KHZ
   StoreShapeOrder voruebergehend abgeklemmt

      Rev 1.33   18 Dec 1998 12:27:22   KHZ
   Task #60395# (3) jetzt DOCH Layer und Z-Order fuer Grafik oder OLE-Objekt  :-)

      Rev 1.32   17 Dec 1998 10:41:26   KHZ
   Task #60395# (2) KEINE Positionierung bei geloeschten Objekten (Grafik, OLE)

      Rev 1.31   15 Dec 1998 19:54:06   KHZ
   Task #60395# Positionierung vor/hinter dem Text je nach pRecord->bDrawHell

      Rev 1.30   07 Dec 1998 16:29:38   JP
   Bug #58766#/#59640#: Zeichenobjecte nach WW97 Import richtig positionieren

      Rev 1.29   05 Dec 1998 17:11:26   KHZ
   Task #59580# Unicode (3)

      Rev 1.28   04 Dec 1998 20:19:46   KHZ
   Task #58766# Textboxen mit Unicode-Inhalt

      Rev 1.27   30 Nov 1998 17:46:52   JP
   Task #59822#: OLE-Objecte importieren

      Rev 1.26   27 Nov 1998 09:45:22   JP
   Task #59822#: OLE-Objecte importieren - Namen korrekt ermitteln

      Rev 1.25   25 Nov 1998 18:45:40   JP
   Compilerfehler unter UNX

      Rev 1.24   24 Nov 1998 20:45:04   JP
   Task #59822#: OLE-Objecte einlesen

      Rev 1.23   20 Nov 1998 22:04:52   JP
   Task #59476#: Crop-Werte bei Grafiken setzen

      Rev 1.22   20 Nov 1998 16:35:30   KHZ
   Task #59640# Hilfsattribute SwFmtVertOrient und SwFmtHoriOrient (2)

      Rev 1.21   20 Nov 1998 16:18:10   KHZ
   Task #59735# Innen- und Aussenabstaende zwischen Text und Shape (bzw. Grafik)

      Rev 1.20   20 Nov 1998 09:44:54   KHZ
   Task #59640# Hilfsattribute SwFmtVertOrient und SwFmtHoriOrient eingesetzt

      Rev 1.19   29 Oct 1998 15:28:54   KHZ
   Task #58199# gedrehter Text in Shapes und Text in Gruppen jetzt moeglich

      Rev 1.18   28 Oct 1998 10:49:30   KHZ
   Task #55189# Text jetzt auch bei Group-Autoformen (Doppelklammer...) moeglich

      Rev 1.17   21 Oct 1998 13:55:52   KHZ
   Task #55189# Abstand des Textbereich vom Shape-Rand ist jetzt WW8 Default

      Rev 1.16   20 Oct 1998 17:21:56   KHZ
   Task #57704# Text-Umlauf um Grafiken und Escher-Objekte

      Rev 1.15   03 Sep 1998 22:14:22   KHZ
   Task #55189# Textboxen

      Rev 1.14   27 Aug 1998 10:12:30   KHZ
   Task #55189# Escher-Import fuer Draw, Calc und Writer

      Rev 1.13   05 Aug 1998 17:16:24   KHZ
   Task #53614# komprimierte WMF, Enh.Metafile und PICT Grafiken

      Rev 1.12   31 Jul 1998 20:43:06   KHZ
   Task #52607# Fehler wg. Compiler-Optimierung!

      Rev 1.11   30 Jul 1998 23:15:02   KHZ
   Task #53614# Grafiken in Data und Dokumentstream gemischt

      Rev 1.10   30 Jul 1998 20:52:36   KHZ
   Task #52607# GPF unter Unix behoben

      Rev 1.9   28 Jul 1998 23:48:20   KHZ
   Task #52607# Grafik-Textboxen und Absatzformatierung verbessert

      Rev 1.8   28 Jul 1998 11:01:48   KHZ
   Task #52607# nummerierte Listen (Teil 1)

      Rev 1.7   22 Jul 1998 15:36:18   KHZ
   Task #52607#

      Rev 1.6   21 Jul 1998 14:51:50   KHZ
   Task #52607# (WW 97 Import)

      Rev 1.5   21 Jul 1998 12:33:38   KHZ
   als MSDrawingObject eingebettete Grafik (Teil 1)

      Rev 1.4   13 Jul 1998 15:29:16   KHZ
   Task #52607# embeded Grafiken (NICHT UEBER den Text gelegte)

      Rev 1.3   30 Jun 1998 21:33:24   KHZ
   Header/Footer/Footnotes weitgehend ok

      Rev 1.2   23 Jun 1998 11:24:26   KHZ
   Zwischenstand: die meisten Zeichenattribute Ok!

      Rev 1.1   18 Jun 1998 09:47:14   KHZ
   Zwischenstand fur 396c

      Rev 1.0   16 Jun 1998 11:06:34   KHZ
   Initial revision.

*************************************************************************/



