/*************************************************************************
 *
 *  $RCSfile: ww8par6.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-16 17:15:59 $
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

#include <stdlib.h>

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#include <svtools/svstdarr.hxx>
#endif

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_WRLMITEM_HXX //autogen
#include <svx/wrlmitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_CNTRITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_ORPHITEM_HXX //autogen
#include <svx/orphitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <svx/widwitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_AKRNITEM_HXX //autogen
#include <svx/akrnitem.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_EMPHITEM_HXX //autogen
#include <svx/emphitem.hxx>
#endif
#ifndef _SVX_FORBIDDENRULEITEM_HXX
#include <svx/forbiddenruleitem.hxx>
#endif
#ifndef _SVX_TWOLINESITEM_HXX
#include <svx/twolinesitem.hxx>
#endif
#ifndef _SVX_SCRIPSPACEITEM_HXX
#include <svx/scriptspaceitem.hxx>
#endif
#ifndef _SVX_HNGPNCTITEM_HXX
#include <svx/hngpnctitem.hxx>
#endif
#ifndef _SVX_PBINITEM_HXX
#include <svx/pbinitem.hxx>
#endif
#ifndef _SVX_CHARSCALEITEM_HXX
#include <svx/charscaleitem.hxx>
#endif
#ifndef _SVX_CHARROTATEITEM_HXX
#include <svx/charrotateitem.hxx>
#endif
#ifndef _SVX_CHARRELIEFITEM_HXX
#include <svx/charreliefitem.hxx>
#endif

#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx> // SwTxtNode, siehe unten: JoinNode()
#endif
#ifndef _PAM_HXX
#include <pam.hxx>              // fuer SwPam
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>         // class SwPageDesc
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTFTNTX_HXX
#include <fmtftntx.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FLTSHELL_HXX
#include <fltshell.hxx>         // fuer den Attribut Stack
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef SW_LINEINFO_HXX
#include <lineinfo.hxx>
#endif
#ifndef SW_FMTLINE_HXX
#include <fmtline.hxx>
#endif
#ifndef _WW8SCAN_HXX
#include <ww8scan.hxx>
#endif
#ifndef _WW8PAR2_HXX
#include <ww8par2.hxx>          // class WW8RStyle, class WwAnchorPara
#endif


#define NEW_MINHDSIZ            // Definieren zum Ausnutzen der minimalen
                                // Header- und Footerhoehe


static ColorData __FAR_DATA eSwWW8ColA[] = {
        COL_BLACK, COL_BLACK, COL_LIGHTBLUE,
        COL_LIGHTCYAN, COL_LIGHTGREEN, COL_LIGHTMAGENTA, COL_LIGHTRED,
        COL_YELLOW, COL_WHITE, COL_BLUE, COL_CYAN, COL_GREEN,
        COL_MAGENTA, COL_RED, COL_BROWN, COL_GRAY, COL_LIGHTGRAY };


#ifdef HP9000
#define INLINE_AUSSER_HP
#else
#define INLINE_AUSSER_HP inline
#endif


//-----------------------------------------
//              diverses
//-----------------------------------------

#define MM_250 1417             // WW-Default fuer Hor. Seitenraender: 2.5 cm
#define MM_200 1134             // WW-Default fuer u.Seitenrand: 2.0 cm
#define MM_125 709              // WW-Default fuer vert. K/F-Pos: 1.25 cm

BOOL lcl_ReadBorders( BOOL bVer67, WW8_BRC* brc,
                            WW8PLCFx_Cp_FKP* pPap,
                            const WW8RStyle* pSty = 0,
                            const WW8PLCFx_SEPX* pSep = 0 );


ColorData SwWW8ImplReader::GetCol( BYTE nIco )
{
    return eSwWW8ColA[nIco];
}

/***************************************************************************
#  Seiten - Attribute, die nicht ueber die Attribut-Verwaltung, sondern
#   ueber ...->HasSprm abgearbeitet werden
#   ( ausser OLST, dass weiterhin ein normales Attribut ist )
#**************************************************************************/

static short ReadSprm( const WW8PLCFx_SEPX* pSep, USHORT nId, short nDefaultVal )
{
    BYTE* pS = pSep->HasSprm( nId );            // sprm da ?
    short nVal = ( pS ) ? SVBT16ToShort( pS ) : nDefaultVal;
    return nVal;
}

static USHORT ReadUSprm( const WW8PLCFx_SEPX* pSep, USHORT nId, short nDefaultVal )
{
    BYTE* pS = pSep->HasSprm( nId );            // sprm da ?
    USHORT nVal = ( pS ) ? SVBT16ToShort( pS ) : nDefaultVal;
    return nVal;
}

static BYTE ReadBSprm( const WW8PLCFx_SEPX* pSep, USHORT nId, BYTE nDefaultVal )
{
    BYTE* pS = pSep->HasSprm( nId );            // sprm da ?
    BYTE nVal = ( pS ) ? SVBT8ToByte( pS ) : nDefaultVal;
    return nVal;
}

// dito, aber mit Return, ob vorhanden
static BOOL ReadBSprmRet( BYTE& rnRet, const WW8PLCFx_SEPX* pSep, USHORT nId,
                       BYTE nDefaultVal )
{
    BYTE* pS = pSep->HasSprm( nId );            // sprm da ?
    if( pS ){
        rnRet = SVBT8ToByte( pS );
        return TRUE;
    }else{
        rnRet = nDefaultVal;
        return FALSE;
    }
}

static short ReadULSprm( const WW8PLCFx_SEPX* pSep, USHORT nId, short nDefaultVal )
{
    BYTE* pS = pSep->HasSprm( nId );            // sprm da ?
    short nVal = ( pS ) ? SVBT16ToShort( pS ) : nDefaultVal;
    if( nVal < 0 )      // < 0 bedeutet: nicht verschieben, wenns nicht passt
        nVal = -nVal;
    return nVal;
}

static short ReadLRSprm( const WW8PLCFx_SEPX* pSep, USHORT nId, short nDefaultVal )
{
    BYTE* pS = pSep->HasSprm( nId );            // sprm da ?
    short nVal = ( pS ) ? SVBT16ToShort( pS ) : nDefaultVal;
    if( nVal < 0 )      // < 0 ist beim SW nicht erlaubt
        nVal = 0;
    return nVal;
}


// WW nimmt es mit den Groessen nicht so genau. Wird dieses nicht
// korrigiert, dann erkennt es der Writer nur als Benutzergroesse
// statt als richtiges Papierformat
// AdjustSize() sucht nahe Papiergroessen raus, stark abweichende
// Werte bleiben unveraendert.
static SwTwips AdjustSize( USHORT nPara )
{
    static SwTwips aSizA[] = { lA3Height, lA3Width, lA4Width, lA5Width,
            lB4Height, lB4Width, lB5Width, lB6Width, lC4Width, lC4Height,
            lC5Width, lC6Width, lC65Width, lDLWidth, lDLHeight,lLetterWidth,
            lLetterHeight, lLegalHeight, lTabloidWidth, lTabloidHeight };


    SwTwips nSi = nPara;
    for( USHORT i = 0; i < sizeof( aSizA ) / sizeof( aSizA[0] ); i++ ){
        if( ( nSi - aSizA[i] < 5 )
            && ( nSi - aSizA[i] > -5 ) ){
            nSi = aSizA[i];
            break;
        }
    }
    return nSi;
}


void SwWW8ImplReader::RemoveCols( SwPageDesc& rPageDesc, SwFmtCol*& rpCol )
{
    SwFrmFmt &rFmt0 = rPageDesc.GetMaster();
    const SwFmtCol rCol = rFmt0.GetCol();
    if( rCol.GetNumCols() )
    {
        rpCol = new SwFmtCol( rCol );
        SwFmtCol aCol;
        rFmt0.SetAttr( aCol );
        rPageDesc.GetLeft().SetAttr( aCol );
    }
    else
        rpCol = 0;
}


BOOL SwWW8ImplReader::SetCols( SwFrmFmt* pFmt, const WW8PLCFx_SEPX* pSep,
    long nNettoWidth, BOOL bTestOnly )
{
    if( nIniFlags & WW8FL_NO_COLS )         // ausgeschaltet
        return FALSE;

    //sprmSCcolumns - Anzahl der Spalten - 1
    USHORT nCols = ReadSprm( pSep, (bVer67 ? 144 : 0x500B), 0 );

    nCols ++;                           // Zahl der SW-Spalten
    if( nCols < 2 )
        return FALSE;                   // keine oder bloedsinnige Spalten

    if( bTestOnly )
        return TRUE;

    if( !pFmt )
    {
        ASSERT( !this, "code error:  pFmt hat Zero value!" );
        return FALSE;
    }

    SwFmtCol aCol;                      // Erzeuge SwFmtCol

    //sprmSDxaColumns   - Default-Abstand 1.25 cm
    USHORT nColSpace = ReadUSprm( pSep, (bVer67 ? 145 : 0x900C), 708 );

    // sprmSLBetween
    if( ReadBSprm( pSep, (bVer67 ? 158 : 0x3019), 0 ) )
    {
        aCol.SetLineAdj( COLADJ_TOP );      // Line
        aCol.SetLineHeight( 100 );
        aCol.SetLineColor( Color( COL_BLACK ));
        aCol.SetLineWidth( 1 );
    }

    // sprmSFEvenlySpaced
    BOOL bEven = ReadBSprm( pSep, (bVer67 ? 138 : 0x3005), 1 );

    if( bEven )                         // alle Spalten sind gleich
        aCol.Init( nCols, nColSpace, nNettoWidth );
    else
    {
        aCol.Init( nCols, nColSpace, USHRT_MAX );
        // Spalten unterschiedlich breit: fein, das kann der Writer inzwischen!
        USHORT nWishWidth = 0, nLeftDist = 0, nRightDist = 0;
        USHORT i;
        for( i = 0; i < nCols; i++ )
        {
            SwColumn* pCol = aCol.GetColumns()[ i ];
            pCol->SetLeft( nLeftDist );

            if( i < nCols-1 )
            {
                //sprmSDxaColSpacing
                BYTE* pSD = pSep->HasSprm( (bVer67 ? 137 : 0xF204), BYTE( i ) );

                ASSERT( pSD, "+Sprm 137 (bzw. 0xF204) (Colspacing) fehlt" );
                if( pSD )
                {
                    USHORT nSp = SVBT16ToShort( pSD + 1 );
                    nRightDist = nSp / 2;

                    pCol->SetRight( nSp - nRightDist );
                }
            }
            else
                nRightDist = 0; // letzte Spalte hat keinen Zwischenraum mehr

            //sprmSDxaColWidth
            BYTE* pSW = pSep->HasSprm( (bVer67 ? 136 : 0xF203), BYTE( i ) );

            ASSERT( pSW, "+Sprm 136 (bzw. 0xF203) (ColWidth) fehlt" );
            if( pSW )
                pCol->SetWishWidth(   SVBT16ToShort( pSW + 1 )
                                    + nLeftDist
                                    + pCol->GetRight() );
            // aufsummierte Spaltenbreiten ergeben Gesamtbreite
            nWishWidth += pCol->GetWishWidth();
            // Halber Abstand ist an naechster Spalte noch zu setzen
            nLeftDist = nRightDist;
        }
        aCol.SetWishWidth( nWishWidth );

#ifdef niemalsdef
        // beim RTF-Import:
        aCol._SetOrtho( FALSE );
        USHORT nWishWidth = 0, nHalfPrev = 0;
        for( USHORT n = 0, i = 0; n < aColumns.Count(); n += 2, ++i )
        {
            SwColumn* pCol = aCol.GetColumns()[ i ];
            pCol->SetLeft( nHalfPrev );
            USHORT nSp = aColumns[ n+1 ];
            nHalfPrev = nSp / 2;
            pCol->SetRight( nSp - nHalfPrev );
            pCol->SetWishWidth( aColumns[ n ] +
                                pCol->GetLeft() + pCol->GetRight() );
            nWishWidth += pCol->GetWishWidth();
        }
        aCol.SetWishWidth( nWishWidth );
#endif
    }
    pFmt->SetAttr( aCol );
    return TRUE;
}

// SetPage1() setzt Orientierung, Papiergroesse, LRRaender, Spalten
void SwWW8ImplReader::SetPage1( SwPageDesc* pPageDesc, SwFrmFmt &rFmt,
                                const WW8PLCFx_SEPX* pSep, USHORT nLIdx,
                                BOOL bIgnoreCols )
{
    if( nIniFlags & WW8FL_NO_LRUL )             // Abgeschaltet
        return;

    static USHORT __READONLY_DATA aVer67Ids[] = {
/*sprmSBOrientation*/               162,
/*sprmSXaPage*/                     164,
/*sprmSYaPage*/                     165,
/*sprmSDxaLeft*/                    166,
/*sprmSDxaRight*/                   167,
/*sprmSDzaGutter*/                  170
        };

    static USHORT __READONLY_DATA aVer8Ids[] = {
/*sprmSBOrientation*/               0x301d,
/*sprmSXaPage*/                     0xB01F,
/*sprmSYaPage*/                     0xB020,
/*sprmSDxaLeft*/                    0xB021,
/*sprmSDxaRight*/                   0xB022,
/*sprmSDzaGutter*/                  0xB025
        };

    const USHORT* pIds = bVer67 ? aVer67Ids : aVer8Ids;

                                            // 1. Orientierung
    pPageDesc->SetLandscape( 0 != ReadBSprm( pSep, pIds[0], 0 ) );

                                                // 2. Papiergroesse
    SwFmtFrmSize aSz( rFmt.GetFrmSize() );
    aSz.SetWidth( AdjustSize( ReadUSprm( pSep,  pIds[1], (USHORT)lLetterWidth )));

    nPgWidth = (short)aSz.GetWidth();       // Merken fuer Tabellen u. ae.
    aSz.SetHeight( AdjustSize( ReadUSprm( pSep, pIds[2], (USHORT)lLetterHeight )));

    rFmt.SetAttr( aSz );

    // 3. LR-Raender
    // Default-Raender fuer verschiedene nationale Versionen
    static USHORT __READONLY_DATA nLef[] = { MM_250, 1800 };
    static USHORT __READONLY_DATA nRig[] = { MM_250, 1800 };

    short nWWLe = ReadULSprm( pSep, pIds[3], nLef[nLIdx] );
    short nWWRi = ReadULSprm( pSep, pIds[4], nRig[nLIdx] );
    short nWWGu = ReadULSprm( pSep, pIds[5], 0 );

    nWWLe += nWWGu;

                                            // Left / Right
    rFmt.SetAttr( SvxLRSpaceItem( nWWLe, nWWRi ) );

    nPgLeft = nWWLe;
    nPgRight = nWWRi;

    if( !bIgnoreCols )
    {
        // 4. Spalten
        SetCols( &rFmt, pSep, aSz.GetWidth() - nWWLe - nWWRi );
    }
}




struct WW8ULSpaceData
{
    BOOL  bHasHeader, bHasFooter;
    short nSwHLo, nHdUL,
          nSwFUp, nFtUL,
          nSwUp,  nSwLo;
    WW8ULSpaceData(): bHasHeader( FALSE ), bHasFooter( FALSE ){}
};

void SwWW8ImplReader::GetPageULData( const  WW8PLCFx_SEPX* pSep,
                                     USHORT nLIdx,
                                     BOOL   bFirst,
                                     WW8ULSpaceData& rData )
{
    if( nIniFlags & WW8FL_NO_LRUL )         // abgeschaltet
        return;

            // Default-Raender fuer verschiedene nationale Versionen
    static USHORT __READONLY_DATA nTop[] = { MM_250, 1440 };
    static USHORT __READONLY_DATA nBot[] = { MM_200, 1440 };

    // Einlesen der WW-Einstellungen

    static USHORT __READONLY_DATA aVer67Ids[] = {
/*sprmSDyaTop*/         168,
/*sprmSDyaBottom*/      169,
/*sprmSDyaHdrTop*/      156,
/*sprmSDyaHdrBottom*/   157
        };
    static USHORT __READONLY_DATA aVer8Ids[] = {
/*sprmSDyaTop*/         0x9023,
/*sprmSDyaBottom*/      0x9024,
/*sprmSDyaHdrTop*/      0xB017,
/*sprmSDyaHdrBottom*/   0xB018
        };

    const USHORT* pIds = bVer67 ? aVer67Ids : aVer8Ids;

    short nWWUp = ReadULSprm( pSep, pIds[0], nTop[nLIdx] );
    short nWWLo = ReadULSprm( pSep, pIds[1], nBot[nLIdx] );
    short nWWHTop = ReadULSprm( pSep, pIds[2], MM_125 );
    short nWWFBot = ReadULSprm( pSep, pIds[3], MM_125 );

    if( bFirst )
        rData.bHasHeader = (nCorrIhdt &   WW8_HEADER_FIRST                )!=0;
    else
        rData.bHasHeader = (nCorrIhdt & (WW8_HEADER_EVEN | WW8_HEADER_ODD))!=0;

    if( rData.bHasHeader )
    {
        rData.nSwUp  = nWWHTop;             // Header -> umrechnen
        rData.nSwHLo = nWWUp - nWWHTop;
#ifdef NEW_MINHDSIZ
        if( rData.nSwHLo < MINLAY )
            rData.nSwHLo = MINLAY;
#else //  NEW_MINHDSIZ
        if( nIniHdSiz )
            rData.nSwHLo -= (short)nIniHdSiz;
        else if( nHdTextHeight )
            rData.nSwHLo -= (short)nHdTextHeight;
        else
            rData.nSwHLo -= 240;

        if( rData.nSwHLo < 0 )
            rData.nSwHLo = 0;
#endif // NEW_MINHDSIZ
    }
    else // kein Header -> Up einfach uebernehmen
        rData.nSwUp = nWWUp;

    if( bFirst )
        rData.bHasFooter = (nCorrIhdt &  WW8_FOOTER_FIRST                 )!=0;
    else
        rData.bHasFooter = (nCorrIhdt & (WW8_FOOTER_EVEN | WW8_FOOTER_ODD))!=0;

    if( rData.bHasFooter )
    {
        rData.nSwLo = nWWFBot;              // Footer -> Umrechnen
        rData.nSwFUp = nWWLo - nWWFBot;

#if 0
        if( rData.nSwFUp < MINLAY )
            rData.nSwFUp = MINLAY;
#else //  0
        if( nIniFtSiz )
            rData.nSwFUp -= (short)nIniFtSiz;
        else if( nFtTextHeight )
            rData.nSwFUp -= (short)nFtTextHeight;
        else
            rData.nSwFUp -= 240;

        if( rData.nSwFUp < 0 )
            rData.nSwFUp = 0;
#endif // !0
    }
    else // kein Footer -> Lo einfach uebernehmen
        rData.nSwLo = nWWLo;

    nPgTop = rData.nSwUp;
}


void SwWW8ImplReader::SetPageULSpaceItems( SwFrmFmt &rFmt, WW8ULSpaceData& rData )
{
    if( nIniFlags & WW8FL_NO_LRUL )         // deactivated ?
        return;

    if( rData.bHasHeader )              // ... und Header-Lower setzen
    {
        SwFrmFmt* pHdFmt = (SwFrmFmt*)rFmt.GetHeader().GetHeaderFmt();
        if( pHdFmt )
        {
#ifdef NEW_MINHDSIZ
//              Kopfzeilenhoehe minimal sezten
            pHdFmt->SetAttr( SwFmtFrmSize( ATT_MIN_SIZE, 0, rData.nSwHLo ) );
#else //  NEW_MINHDSIZ
            SvxULSpaceItem aHdUL( pHdFmt->GetULSpace() );
            aHdUL.SetLower(  rData.nSwHLo );
            pHdFmt->SetAttr( aHdUL );
#endif // NEW_MINHDSIZ
        }
    }

    if( rData.bHasFooter )              // ... und Footer-Upper setzen
    {
        SwFrmFmt* pFtFmt = (SwFrmFmt*)rFmt.GetFooter().GetFooterFmt();
        if( pFtFmt )
        {
#if 0
//              Fusszeilenhoehe minimal sezten
            pFtFmt->SetAttr( SwFmtFrmSize( ATT_MIN_SIZE, 0, rData.nSwFUp ) );
#else // 0
            SvxULSpaceItem aFtUL( pFtFmt->GetULSpace() );
            aFtUL.SetUpper(  rData.nSwFUp );
            pFtFmt->SetAttr( aFtUL );
#endif // !0
        }
    }
    SvxULSpaceItem aUL( rData.nSwUp, rData.nSwLo ); // Page-UL setzen
    rFmt.SetAttr( aUL );
}




void SwWW8ImplReader::SetPageBorder( SwPageDesc*    pPageDesc0,
                                     SwPageDesc*    pPageDesc1,
                               const WW8PLCFx_SEPX* pSep,
                                     USHORT         nLIdx )
{
    WW8_BRC brc[4];
    if( !bVer67 && ::lcl_ReadBorders( bVer67, brc, 0, 0, pSep ) &&
        IsBorder( brc ))
    {
        short nPgbProp = ReadSprm( pSep, 0x522F, 0 );   //sprmSPgbProp

        // ogbProp - pgbApplyTo
        //  0 all Pages in this Section
        //  1 first Page in this Section
        //  2 all Pages in this Section but first
        //  3 whole document (all Sections)
        if( !pPageDesc0 && pPageDesc1 &&
            ( 2 == (nPgbProp & 0x7 ) || 1 == (nPgbProp & 0x7 )) )
        {
            // dann muss aber einer angelegt werden
            SwPaM* pPageDeskPaM = 0;
            pPageDesc0 = CreatePageDesc( 0, &pPageDeskPaM );



            // if PageDesc was inserted check for cols
            // and insert section instead
            if( pPageDeskPaM )
            {
                SwFrmFmt &rFmt = pPageDesc0->GetMaster();
                SetPage1( pPageDesc0, rFmt, pSep, nLIdx, FALSE );
                const SwFmtCol& rCol = rFmt.GetCol();
                // if PageDesc has been inserted and has cols
                // insert a *section* with cols instead
                if( rCol.GetNumCols() )
                {
                    InsertSectionWithWithoutCols( *pPaM, &rCol );
                    // remove columns from PageDesc
                    SwFmtCol aCol;
                    rFmt.SetAttr( aCol );
                }
                delete pPageDeskPaM;
            }



            rDoc.CopyPageDesc( *pPageDesc1, *pPageDesc0, FALSE );
            pPageDesc0->SetFollow( pPageDesc1 );
            pPageDesc1->SetFollow( pPageDesc1 );
        }

        SwFrmFmt* pFmt0 = 0, *pFmt1 = 0;
        if( 2 != (nPgbProp & 0x7 ) && pPageDesc0 )
            pFmt0 = &pPageDesc0->GetMaster();

        if( 1 != (nPgbProp & 0x7 ) && pPageDesc1 )
            pFmt1 = &pPageDesc1->GetMaster();

        SvxBoxItem aBox;
        SetBorder( aBox, brc );

        Rectangle aInnerDist;
        GetBorderDistance( brc, aInnerDist );

        if(    aInnerDist.Left()  || aInnerDist.Top()
            || aInnerDist.Right() || aInnerDist.Bottom() )
        {
            // das muss natuerlich von den Raendern abgezogen werden!
            SwFrmFmt* pFmt = pFmt0;
            for( int i = 0; i < 2; ++i, pFmt = pFmt1 )
                if( pFmt )
                {
                    SvxLRSpaceItem aLR( pFmt->GetLRSpace() );
                    SvxULSpaceItem aUL( pFmt->GetULSpace() );

                    if( 0x20 == ( nPgbProp & 0xe0 ))        // distance from pageborder
                    {
                        // Left
                        if( aInnerDist.Left() < aLR.GetLeft() )
                        {
                            aLR.SetLeft( (USHORT)(aLR.GetLeft()
                                            - aInnerDist.Left() ) );
                            aBox.SetDistance( (USHORT)aInnerDist.Left(),
                                                BOX_LINE_LEFT );
                        }
                        else
                        {
                            aBox.SetDistance( aLR.GetLeft(), BOX_LINE_LEFT );
                            aLR.SetLeft( 0 );
                        }
                        // Right
                        if( aInnerDist.Right() < aLR.GetRight() )
                        {
                            aLR.SetRight( (USHORT)(aLR.GetRight()
                                            - aInnerDist.Right() ) );
                            aBox.SetDistance( (USHORT)aInnerDist.Right(),
                                                BOX_LINE_RIGHT );
                        }
                        else
                        {
                            aBox.SetDistance( aLR.GetRight(), BOX_LINE_RIGHT );
                            aLR.SetRight( 0 );
                        }
                        // Top
                        if( aInnerDist.Top() < aUL.GetUpper() )
                        {
                            aUL.SetUpper( (USHORT)(aUL.GetUpper()
                                            - aInnerDist.Top() ) );
                            aBox.SetDistance( (USHORT)aInnerDist.Top(),
                                                BOX_LINE_TOP );
                        }
                        else
                        {
                            aBox.SetDistance( aUL.GetUpper(), BOX_LINE_TOP );
                            aUL.SetUpper( 0 );
                        }
                        // Bottom
                        if( aInnerDist.Bottom() < aUL.GetLower() )
                        {
                            aUL.SetLower( (USHORT)(aUL.GetLower()
                                            - aInnerDist.Bottom() ) );
                            aBox.SetDistance( (USHORT)aInnerDist.Bottom(),
                                                BOX_LINE_BOTTOM );
                        }
                        else
                        {
                            aBox.SetDistance( aUL.GetLower(), BOX_LINE_BOTTOM );
                            aUL.SetLower( 0 );
                        }

                        pFmt->SetAttr( aLR );
                        pFmt->SetAttr( aUL );
                        pFmt->SetAttr( aBox );
                    }
                    else                                    // distance from text
                    {
                        // Left
                        aBox.SetDistance( (USHORT)aInnerDist.Left(),
                                            BOX_LINE_LEFT );
                        aLR.SetLeft( Max((long)0, (long)(aLR.GetLeft()
                                            - (USHORT)aInnerDist.Left() )));
                        // Right
                        aBox.SetDistance( (USHORT)aInnerDist.Right(),
                                            BOX_LINE_RIGHT );
                        aLR.SetRight(Max((long)0, (long)(aLR.GetRight()
                                            - (USHORT)aInnerDist.Right() )));
                        // Top
                        aBox.SetDistance( (USHORT)aInnerDist.Top(),
                                            BOX_LINE_TOP );
                        aUL.SetUpper(Max(0, aUL.GetUpper()
                                            - (USHORT)aInnerDist.Top() ));
                        // Bottom
                        aBox.SetDistance( (USHORT)aInnerDist.Bottom(),
                                            BOX_LINE_BOTTOM );
                        aUL.SetLower(Max(0, aUL.GetLower()
                                            - (USHORT)aInnerDist.Bottom() ));

                        pFmt->SetAttr( aLR );
                        pFmt->SetAttr( aUL );
                        pFmt->SetAttr( aBox );
                    }
                }

        }
        else
        {
            if( pFmt0 ) pFmt0->SetAttr( aBox );
            if( pFmt1 ) pFmt1->SetAttr( aBox );
        }


        SvxShadowItem aS;
        if( SetShadow( aS, aBox, brc))
        {
            if( pFmt0 ) pFmt0->SetAttr( aS );
            if( pFmt1 ) pFmt1->SetAttr( aS );
        }
    }

//  if( nSwUp > rBox.GetDistance()) nSwUp -= rBox.GetDistance(); else nSwUp = 0;
//  if( nSwLo > rBox.GetDistance()) nSwLo -= rBox.GetDistance(); else nSwLo = 0;
}

void SwWW8ImplReader::SetUseOn( SwPageDesc* pPageDesc0, SwPageDesc* pPageDesc1,
                                const WW8PLCFx_SEPX* pSep, BYTE nCorrIhdt )
{
    BOOL bEven = nCorrIhdt & ( WW8_HEADER_EVEN |  WW8_FOOTER_EVEN );

    UseOnPage eUseBase = ( pWDop->fMirrorMargins ) ? PD_MIRROR : PD_ALL;
    UseOnPage eUse = eUseBase;
    if( !bEven )
        eUse = (UseOnPage)( eUse | PD_HEADERSHARE | PD_FOOTERSHARE );

    if( !pPageDesc1 ){                      // 1 Pagedesc reicht

        pPageDesc0->WriteUseOn( eUse );     // alle Seiten
    }else{                                  // 2 Pagedescs noetig
        pPageDesc0->WriteUseOn( (UseOnPage) // 1. Seite
                    ( eUseBase | PD_HEADERSHARE | PD_FOOTERSHARE ) );
        pPageDesc1->WriteUseOn( eUse );     // Folgeseiten
    }
}


void SwWW8ImplReader::InsertSectionWithWithoutCols( SwPaM& rMyPaM,
    const SwFmtCol* pCol )
{
    // if this Node is not empty create a new Node befor inserting the Section
    const SwPosition* pPos  = rMyPaM.GetPoint();
    const SwTxtNode* pSttNd = pPos->nNode.GetNode().GetTxtNode();
    USHORT nCntPos          = pPos->nContent.GetIndex();
    if( nCntPos && pSttNd->GetTxt().Len() )
    {
        if( rMyPaM.HasMark() ) // do we have a SELECTION ?
        {
            SwNodeIndex aMarkNd( rMyPaM.GetMark()->nNode, -1 );
            xub_StrLen nMarkCnt = rMyPaM.GetMark()->nContent.GetIndex();
            rDoc.SplitNode( *pPos );

            rMyPaM.Move( fnMoveBackward );
            aMarkNd++;
            rMyPaM.GetMark()->nNode = aMarkNd;
            rMyPaM.GetMark()->nContent.Assign(aMarkNd.GetNode().GetCntntNode(),
                nMarkCnt );
        }
        else
            rDoc.SplitNode( *pPos );
    }

    SwSection aSection( CONTENT_SECTION, rDoc.GetUniqueSectionName() );

    SfxItemSet aSet( rDoc.GetAttrPool(), aFrmFmtSetRange );

    if( pCol )
        aSet.Put( *pCol );

    if( 2 == pWDop->fpc )
        aSet.Put( SwFmtFtnAtTxtEnd( FTNEND_ATTXTEND ));
    if( 0 == pWDop->epc )
        aSet.Put( SwFmtEndAtTxtEnd( FTNEND_ATTXTEND ));

    pNewSection = rDoc.Insert( rMyPaM, aSection, &aSet );
    ASSERT( !pBehindSection, "pBehindSection ungleich Null! why Recursion?");

    // set PaM into first Node of the new Section
    const SwSectionNode* pSectionNode =
        pNewSection->GetFmt()->GetSectionNode();
    ASSERT( pSectionNode, "Kein Inhalt vorbereitet." );

    pBehindSection = new SwNodeIndex( *pSectionNode->EndOfSectionNode(), 1 );

    rMyPaM.GetPoint()->nNode =
        pSectionNode->GetIndex()+1;
    rMyPaM.GetPoint()->nContent.Assign(
        rMyPaM.GetCntntNode(), 0 );
}



// Bei jedem Abschnittswechsel ( auch am Anfang eines Dokuments ) wird
// CreateSep gerufen, dass dann den / die Pagedesc(s) erzeugt und
// mit Attributen un KF-Texten fuellt.
// Dieses Vorgehen ist noetig geworden, da die UEbersetzung der verschiedenen
// Seiten-Attribute zu stark verflochten ist.

void SwWW8ImplReader::CreateSep(const long nTxtPos)
{/*static BYTE __READONLY_DATA nHdFtType[] =
        { WW8_HEADER_EVEN,  WW8_HEADER_ODD,
            WW8_FOOTER_EVEN,  WW8_FOOTER_ODD,
            WW8_HEADER_FIRST, WW8_FOOTER_FIRST };*/

    if( bTxbxFlySection || bDontCreateSep )
    {
        return;
    }


    BYTE nLastSectionCorrIhdt      = nCorrIhdt;
    BOOL bLastSectionHadATitlePage = bSectionHasATitlePage;

    // Might we have to close a section first?
    BOOL bSectionWasJustClosed = pBehindSection && nTxtPos;
    if( bSectionWasJustClosed )
    {
        // remove preceeding Node
        // if a  0x0d  came immediately before
        if( 0 == pPaM->GetPoint()->nContent.GetIndex() )
            JoinNode( pPaM );
        // set PaM behind section
        pPaM->GetPoint()->nNode = *pBehindSection;
        pPaM->GetPoint()->nContent.Assign(
            pPaM->GetCntntNode(), 0 );

        DELETEZ( pBehindSection );
        DELETEZ( pLastPgDeskIdx );
    }

    SwPageDesc* pOldPageDesc = pPageDesc;
    SwPageDesc* pPageDesc1   = 0;
    WW8PLCFx_SEPX* pSep = pPlcxMan->GetSepPLCF();


    // check if Line Numbering must be activated or resetted
    BYTE* pSprmSNLnnMod = bNew ? pSep->HasSprm( bVer67 ? 154 : 0x5015 ) : 0;
    if( pSprmSNLnnMod && *pSprmSNLnnMod )
    {
        // restart-numbering-mode: 0 per page, 1 per section, 2 never restart
        BYTE* pSprmSLnc = pSep->HasSprm( bVer67 ? 152 : 0x3013 );
        bRestartLnNumPerSection =  pSprmSLnc && 1 == *pSprmSLnc;

        if( bNoLnNumYet )
        {
            SwLineNumberInfo aInfo( rDoc.GetLineNumberInfo() );

            aInfo.SetPaintLineNumbers( TRUE );

            aInfo.SetRestartEachPage( !pSprmSLnc || 0 == *pSprmSLnc );

            BYTE* pSprmSDxaLnn = pSep->HasSprm( bVer67 ? 155:0x9016 );
            if( pSprmSDxaLnn )
            {
                INT16 nSDxaLnn = SVBT16ToShort( pSprmSDxaLnn );
                aInfo.SetPosFromLeft( nSDxaLnn );
            }

            aInfo.SetCountBy( *pSprmSNLnnMod ); //Paint only for every n line


            // to be defaulted features ( HARDCODED in MS Word 6,7,8,9 )
            aInfo.SetCountBlankLines(  TRUE );
            aInfo.SetCountInFlys(      FALSE );
            aInfo.SetPos( LINENUMBER_POS_LEFT );
            SvxNumberType aNumType; // this sets SVX_NUM_ARABIC per default
            aInfo.SetNumType( aNumType );


            // to be ingnored features ( NOT existing in MS Word 6,7,8,9 )
    //      aInfo.SetDividerCountBy( nDividerCountBy );
    //      aInfo.SetDivider( sDivider );
    //      aInfo.SetCharFmt( pChrFmt );

            rDoc.SetLineNumberInfo( aInfo );
            bNoLnNumYet = FALSE;
        }

        BYTE* pSprmSLnnMin = pSep->HasSprm( bVer67 ? 160:0x501B );
        if(    (    pSprmSLnnMin
                 && 0 < *pSprmSLnnMin )
            || (    bRestartLnNumPerSection
                 && !bNoLnNumYet ) )
        {
            SwFmtLineNumber aLN;
            if( pSprmSLnnMin )
                aLN.SetStartValue( 1 + *pSprmSLnnMin );
            else
                aLN.SetStartValue( 1 );

            NewAttr( aLN );
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_LINENUMBER );
        }
        bNoLnNumYet = FALSE;
    }


    // pruefen, ob und wie umzubrechen ist          break code: 0 No break
                                                            //  1 New column
    BYTE* pSprmBkc = pSep->HasSprm( bVer67 ? 142 : 0x3009 );//  2 New page
    BYTE nBreakCode = pSprmBkc ? *pSprmBkc : 2; //              3 Even page
    BOOL bContinuousBreak=(0 == nBreakCode); //                 4 Odd page

    // im wievielten Abschnitt stehen wir denn eigentlich?
    nActSectionNo = (short)( pSep->GetIdx() & 0x00007fff );



    /*
        Welche Kopf-/Fuss-Bereiche sind
        identisch mit dem Vorgaenger-Abschnitt?
    */                                                // sprmSFTitlePage
    bSectionHasATitlePage = (0 != ReadBSprm( pSep, bVer67 ? 143 : 0x300A, 0 ));

    BYTE nIPara;

    BYTE nJustCopyHdFt = 0; // HdFt that are stored in previous WW section prop
                            //           and were NOT USED but may be copied now
    BYTE nSameHdFt     = 0; // HdFt that WERE the same in previous WW section

    short aOldSectionNo[ 6 ];
    memset( &aOldSectionNo, 0, sizeof( aOldSectionNo ) );

    if( bVer67 )
    {
        // sprmSGprfIhdt
        BOOL bSameHdFt = ( !ReadBSprmRet( nIPara, pSep, 153, 0 ) );

        // Leere Hd/Ft will ich nicht
        nCorrIhdt = pHdFt ? HdFtCorrectPara( nIPara ) : 0;
        if( bSameHdFt )
        {
            nSameHdFt = 0xff;
            nJustCopyHdFt = nSameHdFt;
        }
    }
    else
    {
        if( !pHdFt )
            nCorrIhdt = 0;
        else
        {
            // nCorrIhdt ermitteln: WELCHE Hd/Ft sind ueberhaupt zu aktivieren?
            //
            nCorrIhdt = WW8_HEADER_ODD | WW8_FOOTER_ODD;

            if( bSectionHasATitlePage )
                nCorrIhdt |=  WW8_HEADER_FIRST | WW8_FOOTER_FIRST;

            if( pWDop->fFacingPages )
                nCorrIhdt |= WW8_HEADER_EVEN   | WW8_FOOTER_EVEN;

            // den PLCF analysieren:
            //
            // Hd/Ft ist gleich, wenn die Laenge des Eintrags NULL ist
            //                   oder die Ziel-Offsets identisch sind.
            //
            WW8_CP nStart;
            WW8_CP nOldStart;
            long nLen;
            long nOldLen;
            BYTE nHdFtInfosStored=0;

            for( BYTE nI = 0, nMask = 1; nI < 6; nI++, nMask <<= 1 )
            {
                // 1st find ALL Hd/Ft infos that are stored in this WW section
                //     -- or in the sections before this one --
                //     regardless whether they are used in this section or not
                pHdFt->GetTextPosExact( nI+ (nActSectionNo+1)*6, nStart, nLen );
                if( !nLen || nActSectionNo )
                {
                    short nOldSectionNo = nActSectionNo;
                    do
                    {
                        --nOldSectionNo;
                        pHdFt->GetTextPosExact( nI + (nOldSectionNo+1)*6,
                                                nOldStart,
                                                nOldLen);
                        if( nOldLen )
                            aOldSectionNo[ nI ] = nOldSectionNo;
                    }
                    while( nOldSectionNo && !nOldLen );
                }
                if( nLen || nOldLen )
                    nHdFtInfosStored |= nMask;


                if( nCorrIhdt & nMask )
                {
                    if( nActSectionNo )
                    {
                        if(    !nLen
                            || (    (nOldStart == nStart)
                                 && (nOldLen   == nLen  ) ) )
                        {
                            // same Hd/Ft as in previous Section or NO Hd/Ft
                            if( nHdFtInfosStored & nMask )
                            {
                                // prev. sect. really DID have and USE the Hd/Ft
                                if( nLastSectionCorrIhdt & nMask )
                                    nJustCopyHdFt |= nMask;
                                // Hd/Ft may be found in one of prev. sections
                                nSameHdFt |= nMask;
                            }
                            else
                                nCorrIhdt &= ~nMask;// NO prev. Hd/Ft at all
                        }
                    }
                    else
                        if( !nLen )
                            nCorrIhdt &= ~nMask;// 0 in 1.Sect.: Hd/Ft undefined
                }
            }
        }
        // Einlese-Flag auf Use-On-Flag setzen
        nIPara = nCorrIhdt;
    }


    USHORT nLIdx = ( ( pWwFib->lid & 0xff ) == 0x9 ) ? 1 : 0;

    // sprmSNfcPgn
    BYTE nLastNfcPgn = nNfcPgn;
    nNfcPgn = ReadBSprm( pSep, (bVer67 ? 147 : 0x300E), 0 );
    if( nNfcPgn > 4 ) nNfcPgn = 0;


    /*
        Pruefen, ob wir uns den neuen Abschnitt schenken koennen, da kein
        Umbruch erforderlich ist.
    */
    if( pSep->GetIMax() > 1 )
    {
        if( nActSectionNo )
        {
            // Index eins zurueck setzen
            pSep->SetIdx( nActSectionNo-1 );
            // Attribute von vorigem Abschnitt einlesen
            WW8PLCFxDesc aOther;
            pSep->GetSprms( &aOther );
            long nOtherSprmsLen = aOther.nSprmsLen;
            // Kopie der vorigen Attr. anlegen, da sie gleich ueberschrieben werden
            BYTE* pOtherMem = new BYTE[ nOtherSprmsLen ];
            memcpy( pOtherMem, aOther.pMemPos, nOtherSprmsLen );
            // Index wieder richtig setzen
            pSep->SetIdx( nActSectionNo );
            // aktuelle Attribute einlesen
            WW8PLCFxDesc aCur;
            pSep->GetSprms( &aCur );
            // zu ignorierende Attribute sammeln
            SvUShortsSort aIgnore(9, 1);
            if( bContinuousBreak )
            {
                /*
                    zu 'sprmSBkc':
                    Wir sehen zwei WW-Abschnitte auch dann als gleich an,
                    wenn sie sich lediglich im break code unterscheiden.
                    Natuerlich muessen die Kopf/Fuss-Bereiche identisch sein.

                    Ignoriert werden auch die folgenden,
                    spaltenbezogene Flags:
                    SCcolumns, SDxaColumns, SDxaColWidth,
                    SDxaColSpacing, SFEvenlySpaced, SLBetween
                    und: SFFacingCol (nur bei Ver8)

                    We will also ignore a different formatting of the page number here.

                    We will also ignore different line numbering settings here
                    since only the very 1st line numbering settings are taken
                    into account anyway, see: bNoLnNum
                */
                static USHORT __READONLY_DATA aVer67Ids[ 13 ] =
                    {136, 137, 138, 139, 142, 144, 145, 147, 152, 154, 155, 158, 160};//sortiert!
                static USHORT __READONLY_DATA aVer8Ids[  14] =
                    {0x3005, 0x3006, 0x3009, 0x300E, 0x3013, 0x3019, 0x3229, 0x500B, 0x5015, 0x501B,
                     0x900C, 0x9016, 0xF203, 0xF204};//sortiert!
                if( bVer67 )
                    aIgnore.Insert( aVer67Ids, 13);
                else
                    aIgnore.Insert( aVer8Ids,  14);
            }

            // nachschauen, ob die nicht zu ignor. Attr. gleich sind
            BOOL bEqual =    (bSectionHasATitlePage ==bLastSectionHadATitlePage)
                          && (nCorrIhdt == nLastSectionCorrIhdt)
                          && (nCorrIhdt == (nCorrIhdt & nJustCopyHdFt))
                          && (nNfcPgn   == nLastNfcPgn)
                          && pSep->CompareSprms(pOtherMem,
                                                nOtherSprmsLen,
                                                &aIgnore);
            // Kopie der vorigen Attr. wieder freigeben
            delete pOtherMem;

            if( bEqual )
            {
                switch( nBreakCode )
                {
                case 0:
#if 1
                    InsertSectionWithWithoutCols( *pPaM, 0 );
                    if( pPageDesc )
                    {
                        SwFrmFmt& rFmt = pPageDesc->GetMaster();
                        const SwFmtFrmSize&   rSz = rFmt.GetFrmSize();
                        const SvxLRSpaceItem& rLR = rFmt.GetLRSpace();
                        SwTwips nWidth = rSz.GetWidth();
                        long nLeft  = rLR.GetTxtLeft();
                        long nRight = rLR.GetRight();
                        SetCols( pNewSection->GetFmt(), pSep,
                            nWidth - nLeft - nRight );
                    }
#else   //Old (seemingly) duplicate code
                    {
                        if( 0 < pPaM->GetPoint()->nContent.GetIndex() )
                            rDoc.AppendTxtNode( *pPaM->GetPoint() );
                        // Abschnittsweschsel: neuer Bereich
                        SwSection aSection( CONTENT_SECTION,
                                            rDoc.GetUniqueSectionName() );

                        SfxItemSet aSet( rDoc.GetAttrPool(), aFrmFmtSetRange );
                        if( 2 == pWDop->fpc )
                            aSet.Put( SwFmtFtnAtTxtEnd( FTNEND_ATTXTEND ));
                        if( 3 == pWDop->epc )
                            aSet.Put( SwFmtEndAtTxtEnd( FTNEND_ATTXTEND ));

                        pNewSection = rDoc.Insert( *pPaM, aSection, &aSet );

                        // Anzahl der Spalten einstellen
                        if( pPageDesc )
                        {
                            SwFrmFmt& rFmt = pPageDesc->GetMaster();
                            const SwFmtFrmSize&   rSz = rFmt.GetFrmSize();
                            const SvxLRSpaceItem& rLR = rFmt.GetLRSpace();
                            SwTwips nWidth = rSz.GetWidth();
                            USHORT  nLeft  = rLR.GetTxtLeft();
                            USHORT  nRight = rLR.GetRight();
                            SetCols( pNewSection->GetFmt(), pSep, nWidth - nLeft - nRight );
                        }
                        // PaM in Node der Section setzen
                        const SwSectionNode* pSectionNode =
                            pNewSection->GetFmt()->GetSectionNode();
                        ASSERT( pSectionNode, "Kein Inhalt vorbereitet." );
                        pBehindSection = new SwNodeIndex( pPaM->GetPoint()->nNode );

                        pPaM->GetPoint()->nNode =
                            pSectionNode->GetIndex()+1;
                        pPaM->GetPoint()->nContent.Assign(
                            pPaM->GetCntntNode(), 0 );
                    }
#endif
                    break;
                case 1:
                    if( bNew )
                    {
                        rDoc.Insert(*pPaM, SvxFmtBreakItem(
                            SVX_BREAK_COLUMN_BEFORE ));
                    }
                    break;
                //case 2:
                //case 3:   // alle drei Faelle -> PgDesc-Format-Einfuegung
                //case 4:
                default:
                    /*
                        Wir koennen den aktuellen PgDesk einsetzen, da dies
                        immer der 1st-Page Deskriptor ist, nie der Follow!

                        So geht es, auch wenn der Break auf Seite 7 kommt,
                        wieder mit einer ERSTEN Seite weiter.
                    */
                    if( bNew )
                    {
                        if( 0 < pPaM->GetPoint()->nContent.GetIndex() )
                            rDoc.AppendTxtNode( *pPaM->GetPoint() );

                        if( pPageDesc )
                            rDoc.Insert(*pPaM, SwFmtPageDesc( pPageDesc ));
                        else
                            // kein voriger PgDesc vorhanden?
                            rDoc.Insert(*pPaM,
                                SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE ));
                        SetLastPgDeskIdx();
                    }
                    break;
                }
                return;
            // ========  Das war's Freunde, jetzt nichts wie weg hier!
            }
        }
        if(    bNew
            && (    bSectionWasJustClosed
                 || (pPageDesc != &rDoc._GetPageDesc( 0 )) ) )
        {
            // Create and *insert* PageDesc
            SwPaM* pPageDeskPaM = 0;
            pPageDesc = CreatePageDesc( 0, &pPageDeskPaM );
            SwFrmFmt &rFmt = pPageDesc->GetMaster();
            SetPage1( pPageDesc, rFmt, pSep, nLIdx, FALSE );
            // if PageDesc has been inserted and has cols
            // insert a *section* with cols instead
            if( pPageDeskPaM )
            {
                const SwFmtCol& rCol = rFmt.GetCol();
                if( rCol.GetNumCols() )
                {
                    InsertSectionWithWithoutCols( *pPaM, &rCol );
                    // remove columns from PageDesc
                    SwFmtCol aCol;
                    rFmt.SetAttr( aCol );
                }
                delete pPageDeskPaM;
            }
        }
    }
    else
        if( bNew )
            pPageDesc = &rDoc._GetPageDesc( 0 );    // Standard

    if( !bNew )
        return;


    // Seitennummernformat speichern
    {
        static SvxExtNumType __READONLY_DATA aNumTyp[ 5 ] = {
            SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
            SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N };

        SvxNumberType aType; aType.SetNumberingType(aNumTyp[ nNfcPgn ]);
        pPageDesc->SetNumType( aType );
    }

    // Page Number Restarts - sprmSFPgnRestart
    BYTE nfPgnRestart = ReadBSprm( pSep, (bVer67 ? 150 : 0x3011), 0 );

    if( nfPgnRestart )
    {
        const SfxPoolItem* pItem;
        const SfxItemSet* pSet;
        if(     ( 0 != (pSet = pPaM->GetCntntNode()->GetpSwAttrSet()) )
             && ( SFX_ITEM_SET ==
                  pSet->GetItemState(RES_PAGEDESC, FALSE, &pItem) ) )
        {
            // read Pagination Start attribute - sprmSPgnStart
            BYTE nPgnStart = ReadBSprm( pSep, (bVer67 ? 161 : 0x501C), 0 );
            ((SwFmtPageDesc*)pItem)->SetNumOffset( nPgnStart );
        }
        else if( pPageDesc == &rDoc.GetPageDesc( 0 ) )
        {
            // read Pagination Start attribute - sprmSPgnStart
            BYTE nPgnStart = ReadBSprm( pSep, (bVer67 ? 161 : 0x501C), 0 );
            SwFmtPageDesc aPgDesc( pPageDesc );
            aPgDesc.SetNumOffset( nPgnStart );
            rDoc.Insert( *pPaM, aPgDesc );
            SetLastPgDeskIdx();
        }

        // Chapterlevel und Chapterdelimiter ? (sprmScnsPgn & sprmSiHeadingPgn)
        BYTE* p = pSep->HasSprm( bVer67 ? 132 : 0x3001 );
        if( p && *p )
        {
            bPgChpLevel = TRUE;
            nPgChpLevel = *p - 1;
            if( MAXLEVEL <= nPgChpLevel )
                nPgChpLevel = MAXLEVEL - 1;

            if( 0 != (p = pSep->HasSprm( bVer67 ? 131 : 0x3000 )) )
                nPgChpDelim = *p;
            else
                nPgChpDelim = 0;
        }
        else
            bPgChpLevel = FALSE;
    }

    // Vorsicht: gibt es ueberhaupt einen vorigen Page Descriptor?
    if( !pOldPageDesc )
        nSameHdFt = 0;

    SwFrmFmt &rFmt0 = pPageDesc->GetMaster();

    if( !bSectionHasATitlePage )
    {
        // Gegebenenfalls Einstellungen des vorigen PgDesc uebernehmen
        // und das Einlese-Flag entsprechend korrigieren,
        // damit nur noch die Eintraege gelesen werden, die NICHT
        // vom vorigen PgDesc zu uebernehmen sind.
        //
        if( 0 < nSameHdFt )
        {
            CopyPageDescHdFt( pOldPageDesc, pPageDesc, nSameHdFt );
            if( bVer67 )
                nIPara  =  0;
            else
                nIPara &= ~nSameHdFt;
        }

        WW8ULSpaceData aULData;
        GetPageULData( pSep, nLIdx, FALSE, aULData );

        // dann Header / Footer lesen, falls noch noetig
        //
        if( nIPara )
        {
            SetHdFt( pPageDesc, 0, pSep, nIPara );
        }
        // und uebrige Einstellungen updaten
        //

        SetPage1( pPageDesc, // Orientierung, Hoehe, Breite, Vertikale Formatierung
                  rFmt0,
                  pSep,
                  nLIdx,
                  TRUE );

        SetPageULSpaceItems( rFmt0, aULData );
        SetPageBorder( 0, pPageDesc, pSep, nLIdx );
    }
    else
    {
        // Hier beachten:
        // ==============
        //
        //      pPageDesc  = erste Seite
        //      pPageDesc1 = Folge-Seiten
        //


        // erst folgende PageDesc-Werte einstellen:
        //
        //    Orientierung, Hoehe, Breite
        //
        SetPage1( pPageDesc, // Orientierung, Hoehe, Breite, Vertikale Formatierung
                    rFmt0,
                    pSep,
                    nLIdx,
                    TRUE );

        // dann den PageDesc1 anlegen fuer Folge-Seiten
        //
        pPageDesc1 = CreatePageDesc( pPageDesc );

        // Gegebenenfalls Einstellungen des/der vorigen PgDesc uebernehmen
        //
        if( 0 < nSameHdFt )
        {
            // ggfs. erst den alten 1st-Seite-PgDesc suchen und dessen
            // Einstellungen uebernehmen
            //
            if( nSameHdFt & (WW8_HEADER_FIRST | WW8_FOOTER_FIRST) )
            {

/*
                // suchen
                const SwPageDesc* pOld1stPageDesc = 0;

                USHORT nSize = rDoc.GetPageDescCnt();
                if( nSize )
                {
                    for( USHORT n = 0; n < nSize; ++n )
                    {
                        const SwPageDesc& rPageDesc = rDoc.GetPageDesc( n );
                        if(    (&rPageDesc            != pOldPageDesc)
                            && (rPageDesc.GetFollow() == pOldPageDesc) )
                        {
                            pOld1stPageDesc = &rPageDesc;
                            break;
                        }
                    }
                }
                // uebernehmen
                if( pOld1stPageDesc )
                {
                    CopyPageDescHdFt( pOld1stPageDesc, pPageDesc,
                        nSameHdFt & (WW8_HEADER_FIRST | WW8_FOOTER_FIRST) );
                }
                else
                {
*/

                if( pOldPageDesc == pOldPageDesc->GetFollow() )
                {
                    // hoppla, die vorige Section hatte keine 1st Page?
                    //
                    // also: auslesen der in WW8 fuer die nicht vorhandene
                    //       1st Page gespeicherten Hd/Ft-Einstellungen...
                    //
                    // Beachte: pPageDesc1 MUSS hier mit uebergeben werden,
                    //          obwohl wir dessen Werte ja eigentlich hier
                    //          gar nicht einlesen wollen!
                    //          Das macht aber nichts, denn wir stellen den
                    //          "SetHdFt( nIPara )"-Parameter so ein, dass
                    //          nur der 1.-Seite-PgDesc eingelesen wird.
                    //
                    short nSaveSectionNo = nActSectionNo;
                    if( bVer67 )
                        --nActSectionNo;
                    else
                        nActSectionNo =   aOldSectionNo[ 4 ]
                                        ? aOldSectionNo[ 4 ]    // Hd 1st
                                        : aOldSectionNo[ 5 ];   // Ft 1st
                    SetHdFt( pPageDesc, pPageDesc1, pSep,
                             nSameHdFt & (WW8_HEADER_FIRST | WW8_FOOTER_FIRST) );
                    nActSectionNo = nSaveSectionNo;
                }
                else
                    CopyPageDescHdFt( pOldPageDesc, pPageDesc,
                        nSameHdFt & (WW8_HEADER_FIRST | WW8_FOOTER_FIRST) );
            }

            // ggfs. Werte des vorigen PgDesc fuer Folge-Seiten uebernehmen
            //
            if( nSameHdFt & (   WW8_HEADER_EVEN  | WW8_HEADER_ODD
                              | WW8_FOOTER_EVEN  | WW8_FOOTER_ODD ) )
            {
                CopyPageDescHdFt( pOldPageDesc, pPageDesc1,
                    nSameHdFt & (   WW8_HEADER_EVEN  | WW8_HEADER_ODD
                                  | WW8_FOOTER_EVEN  | WW8_FOOTER_ODD ) );
            }
            // das Einlese-Flag entsprechend korrigieren,
            // damit nur noch die Eintraege gelesen werden, die NICHT
            // von vorigem/n PgDesc(s) zu uebernehmen sind.
            //
            if( bVer67 )
                nIPara  =  0;
            else
                nIPara &= ~nSameHdFt;
        }

        SwFrmFmt &rFmt1 = pPageDesc1->GetMaster();

        WW8ULSpaceData aULData0, aULData1;
        GetPageULData( pSep, nLIdx, TRUE,  aULData0 ); // Vertikale Formatierung
        GetPageULData( pSep, nLIdx, FALSE, aULData1 ); // einzeln, da KF evtl. verschieden

        // dann Header / Footer lesen, falls noch noetig
        //
        if( nIPara )
        {
            SetHdFt( pPageDesc, pPageDesc1, pSep, nIPara );
        }

        // und uebrige Einstellungen updaten
        //
        SetPageULSpaceItems( rFmt0, aULData0 ); // Vertikale Formatierung
        SetPageULSpaceItems( rFmt1, aULData1 ); // einzeln, da KF evtl. verschieden
        SetPageBorder( pPageDesc, pPageDesc1, pSep, nLIdx );
    }


    SetUseOn( pPageDesc, pPageDesc1, pSep, nCorrIhdt );


    static BYTE __READONLY_DATA aPaperBinIds[ 17 ] =
        {5,2,4,0,3,0,0,0,0,0,0,0,0,0,0,0,1};
        // WW                          SW
        // ------------------------------
        //  0 == default                5
        //  1 == Upper paper tray       2
        //  2 == Lower paper tray       4
        //  4 == Manual paper feed      3
        // 15 == Automatically select   0
        // 16 == First tray available   1

    BYTE* pSprmSDmBinFirst = pSep->HasSprm( bVer67 ? 140 : 0x5007 );
    BYTE* pSprmSDmBinOther = pSep->HasSprm( bVer67 ? 141 : 0x5008  );
    if( pSprmSDmBinFirst && (17 > *pSprmSDmBinFirst) )
    {
        SvxPaperBinItem aItem( ITEMID_PAPERBIN, aPaperBinIds[ *pSprmSDmBinFirst ] );
        pPageDesc->GetMaster().SetAttr( aItem );
    }
    if( pSprmSDmBinOther && (17 > *pSprmSDmBinOther) )
    {
        SvxPaperBinItem aItem( ITEMID_PAPERBIN, aPaperBinIds[ *pSprmSDmBinOther ] );
        SwFrmFmt &rFmtOther = pPageDesc1 ? pPageDesc1->GetMaster() : pPageDesc->GetLeft();
        rFmtOther.SetAttr( aItem );
    }

    // Kopf / Fuss - Index Updaten
    if( pHdFt )
        pHdFt->UpdateIndex( nCorrIhdt );    // Damit der Index auch spaeter noch stimmt

    // Die Attribute OLST
    // werden weiterhin ueber die WWSprmTab abgehandelt
}


void SwWW8ImplReader::CopyPageDescHdFt( const SwPageDesc* pOrgPageDesc,
                                        SwPageDesc* pNewPageDesc, BYTE nCode )
{
    // copy first header content section
    if( nCode & WW8_HEADER_FIRST )
    {
        rDoc.CopyHeader(pOrgPageDesc->GetMaster(),
                        pNewPageDesc->GetMaster());
    }
    // copy first footer content section
    if( nCode & WW8_FOOTER_FIRST )
    {
        rDoc.CopyFooter(pOrgPageDesc->GetMaster(),
                        pNewPageDesc->GetMaster());
    }
    if( nCode & (   WW8_HEADER_ODD  | WW8_FOOTER_ODD
                  | WW8_HEADER_EVEN | WW8_FOOTER_EVEN ) )
    {
        // determine PageDesc of follow pages
        const SwPageDesc* pOrgFollowPageDesc = pOrgPageDesc->GetFollow();
        // copy odd header content section
        if( nCode & WW8_HEADER_ODD )
        {
            rDoc.CopyHeader(pOrgFollowPageDesc->GetMaster(),
                            pNewPageDesc->GetMaster() );
        }
        // copy odd footer content section
        if( nCode & WW8_FOOTER_ODD )
        {
            rDoc.CopyFooter(pOrgFollowPageDesc->GetMaster(),
                            pNewPageDesc->GetMaster());
        }
        // copy even header content section
        if( nCode & WW8_HEADER_EVEN )
        {
            rDoc.CopyHeader(pOrgFollowPageDesc->GetLeft(),
                            pNewPageDesc->GetLeft());
        }
        // copy even footer content section
        if( nCode & WW8_FOOTER_EVEN )
        {
            rDoc.CopyFooter(pOrgFollowPageDesc->GetLeft(),
                            pNewPageDesc->GetLeft());
        }
    }
}


//------------------------------------------------------
//   Hilfsroutinen fuer Grafiken und Apos und Tabellen
//------------------------------------------------------

static BOOL _SetWW8_BRC( BOOL bVer67, WW8_BRC& rVar, BYTE* pS )
{
    if( pS )
        if( bVer67 )
            (WW8_BRCVer6&)rVar = *((WW8_BRCVer6*)pS);
        else
            rVar = *((WW8_BRC*)pS);
    else
    {
        *(USHORT*)rVar.aBits1 = 0;
        *(USHORT*)rVar.aBits2 = 0;
    }

    return 0 != pS;
}

BOOL lcl_ReadBorders( BOOL bVer67, WW8_BRC* brc,
                            WW8PLCFx_Cp_FKP* pPap,
                            const WW8RStyle* pSty,
                            const WW8PLCFx_SEPX* pSep )
{
// Ausgegend von diesen defines:
//      #define WW8_TOP 0
//      #define WW8_LEFT 1
//      #define WW8_BOT 2
//      #define WW8_RIGHT 3
//      #define WW8_BETW 4

    BOOL bBorder = FALSE;
    if( pSep )
    {
        if( !bVer67 )
        {
             BYTE* pSprm[4];

            //  sprmSBrcTop, sprmSBrcLeft, sprmSBrcBottom, sprmSBrcRight
             if( pSep->Find4Sprms(  0x702B,   0x702C,   0x702D,   0x702E,
                                    pSprm[0], pSprm[1], pSprm[2], pSprm[3] ) )
             {
                for( int i = 0; i < 4; ++i )
                    bBorder |= _SetWW8_BRC( bVer67, brc[ i ], pSprm[ i ] );
             }
        }
    }
    else
    {

        static USHORT __READONLY_DATA aVer67Ids[5] =
                { 38, 39, 40, 41, 42 };

        static USHORT __READONLY_DATA aVer8Ids[5] =
                { 0x6424, 0x6425, 0x6426, 0x6427, 0x6428 };

        const USHORT* pIds = bVer67 ? aVer67Ids : aVer8Ids;

        if( pPap )
        {
            for( int i = 0; i < 5; ++i, ++pIds )
                bBorder |= _SetWW8_BRC( bVer67, brc[ i ], pPap->HasSprm( *pIds ));
        }
        else if( pSty )
        {
            for( int i = 0; i < 5; ++i, ++pIds )
                bBorder |= _SetWW8_BRC( bVer67, brc[ i ], pSty->HasParaSprm( *pIds ));
        }
        else
            ASSERT( pSty || pPap, "WW8PLCFx_Cp_FKP and WW8RStyle "
                               "and WW8PLCFx_SEPX is 0" );
    }

    return bBorder;
}



static SvxBorderLine& Set1Border( BOOL bVer67, SvxBorderLine& rLine,
                                    const WW8_BRC& rBor )
{
    // Deklarationen gemaess BOXITEM.HXX
    WW8_DECL_LINETAB_ARRAY

    // Match-Tabelle: verwandelt Ver67 Border Codes in Ver8-Typen
    static USHORT __READONLY_DATA nTabBorderCode67ToCode8[] = // Aussenlinie
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1,   2, 2, 3, 0, 0, 0, 1, 2, 3, 4, 4, 1, 1,
        WW8_DECL_LINETAB_OFS_DOUBLE +0,
        WW8_DECL_LINETAB_OFS_DOUBLE +1,
        WW8_DECL_LINETAB_OFS_DOUBLE +1,
        WW8_DECL_LINETAB_OFS_DOUBLE +2,
        WW8_DECL_LINETAB_OFS_DOUBLE +2,
        WW8_DECL_LINETAB_OFS_DOUBLE +3,
        WW8_DECL_LINETAB_OFS_DOUBLE +0,
        WW8_DECL_LINETAB_OFS_DOUBLE +0
    };


        // Vor Aufruf muss sichergestellt werden, dass aBor & 0x1f != 0 ist,
        // d.h. ueberhaupt eine Linie vorhanden ist

//  short nIdx = ( aBrc1 & 0x7 )        // aBor.dxpLineWidth ( Liniendicke )
//          + 8 * ( aBrc1 >> 3 & 0x3 ); // brcType, 0 = none / dotted / dashed
                                        // 1 = single width, 2 = double width
                                        // 3 = doppelte Linie ( = 3-fache Dicke )

    BYTE nCol;
    short nIdx;

    if( bVer67 )
    {
        UINT16 aBrc1 = SVBT16ToShort( rBor.aBits1 );
        nIdx = nTabBorderCode67ToCode8[ aBrc1 & 0x1f ];
        nCol = ( (aBrc1 >> 6) & 0x1f ); // aBor.ico
    }
    else
    {
        // Sicherheitsmassnahme (Kodes ueber 25 sind undokumentiert)
        nIdx = rBor.aBits1[1];
        if( 25 < nIdx )
            nIdx = 1;

        // Angabe in 8tel Punkten, also mal 2.5, da 1 Punkt = 20 Twips
        short nMSLineWidth  = rBor.aBits1[ 0 ] * 20 / 8;
        short nMSTotalWidth = nMSLineWidth;

        // erst die Gesamt-Randbreite errechnen, zur weiteren Entscheidungsgrundlage
        switch( nIdx )
        {
            /*
            // Einzel-Linien sind Wurst
            case  1:
            case  2:
            case  5:
            case  6:
            case 22:
            case  7:
            case  8:
            case  9:
            // Schraffurbalken dito
            case 23:
            */
            // Mehrfach- und Spezial-Linien, die wir wie besonders behandeln
            // (in der Reihenfolge, in der sie im Winword-97-Dialog erscheinen)
            case  3: nMSTotalWidth = 3*nMSLineWidth;break;
            case 10: nMSTotalWidth = 5*nMSLineWidth;break;
            case 18:
            case 17: nMSTotalWidth = 7*nMSLineWidth;break;
            case 16:
            case 19: nMSTotalWidth =12*nMSLineWidth;break;
            case 20: nMSTotalWidth = 4*nMSLineWidth;break;
            case 12:
            case 11:
            case 21: nMSTotalWidth = 6*nMSLineWidth;break;
            case 13:
            case 15:
            case 14:
            //case 24: ##158## setting relief to this multiple of its width
            //is overkill, lets leave it at its original width.
            case 25: nMSTotalWidth = 8*nMSLineWidth;break;
        }

        // dann auf unsere Randtypen mappen
        switch( nIdx )
        {
            // zuerst die Einzel-Linien
            case  1:
            case  2:
            case  5:
            case  6:
            case 22:
            case  7:
            case  8:
            case  9:
            // UND die Sonderfaelle, die wir durch eine Einzel-Linie
            case 13:                // bzw. notfalls durch eine Doppel-Linie darstellen
            case 16:
            case 19:
            case 24:
            case 25:         if( nMSTotalWidth < 11) nIdx =            0;//   1 Twip bei uns
                            else if( nMSTotalWidth < 46) nIdx =            1;//  20 Twips
                            else if( nMSTotalWidth < 66) nIdx =            2;//  50
                            else if( nMSTotalWidth < 91) nIdx =            3;//  80
                            else if( nMSTotalWidth <126) nIdx =            4;// 100
                            // Pfusch: fuer die ganz dicken Linien muessen
                            //         wir doppelte Linien malen, weil unsere
                            //               Einfach-Linie nicht dicker als 5 Punkt wird
                            else if( nMSTotalWidth <166) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+2;// 150
                            else                         nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+5;// 180
                            break;
            // dann den Schraffurbalken, den wir durch eine doppelte Linie darstellen
            case 23:    nIdx =  6;
                                break;
            // dann die Doppel-Linien, fuer die wir feine Entsprechungen haben :-)))
            case  3:
            case 10:         if( nMSTotalWidth <  46) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 0;//  22 Twips bei uns
                            else if( nMSTotalWidth < 106) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 1;//  60
                            else                          nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 2;// 150
                            break;
            case 12:         if( nMSTotalWidth <  87) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 8;//  71 Twips bei uns
                            else if( nMSTotalWidth < 117) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 9;// 101
                            else if( nMSTotalWidth < 166) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+10;// 131
                            else                          nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 5;// 180
                            break;
            case 11:         if( nMSTotalWidth < 137) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 4;//  90 Twips bei uns
                            else                          nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 6;// 180
                            break;
            case 15:         if( nMSTotalWidth <  46) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 0;//  22 Twips bei uns
                            else if( nMSTotalWidth < 106) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 1;//  60
                            else if( nMSTotalWidth < 166) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 2;// 150
                            else                          nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 5;// 180
                            break;
            case 14:         if( nMSTotalWidth <  46) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 0;//  22 Twips bei uns
                            else if( nMSTotalWidth <  76) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 1;//  60
                            else if( nMSTotalWidth < 121) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 4;//  90
                            else if( nMSTotalWidth < 166) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 2;// 150
                            else                          nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 6;// 180
                            break;
            case 18:         if( nMSTotalWidth <  46) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 0;//  22 Twips bei uns
                            else if( nMSTotalWidth <  62) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 7;//  52
                            else if( nMSTotalWidth <  87) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 8;//  71
                            else if( nMSTotalWidth < 117) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 9;// 101
                            else if( nMSTotalWidth < 156) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+10;// 131
                            else                          nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 5;// 180
                            break;
            case 17:         if( nMSTotalWidth <  46) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 0;//  22 Twips bei uns
                            else if( nMSTotalWidth <  72) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 7;//  52
                            else if( nMSTotalWidth < 137) nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 4;//  90
                            else                          nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 6;// 180
                            break;
            case 20:         if( nMSTotalWidth <  46) nIdx =             1;//  20 Twips bei uns
                            else                          nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 1;//  60
                            break;
            case 21:                                  nIdx = WW8_DECL_LINETAB_OFS_DOUBLE+ 1;//  60 Twips bei uns
                            break;

            default: nIdx = 0;
        }

        nCol  = rBor.aBits2[0]; // aBor.ico
    }

    const WW8_BordersSO& rBorders = nLineTabVer8[ nIdx ];
    rLine.SetOutWidth( rBorders.Out  );
    rLine.SetInWidth ( rBorders.In   );
    rLine.SetDistance( rBorders.Dist );
    rLine.SetColor( eSwWW8ColA[ nCol ] );

    /*
        Achtung:  noch zu tun!!!
        ========
        eigentlich sollten wir uns jetzt die kumulierte Randbreite merken
        ( als da waere:  rBorders.Out + rBorders.In + rBorders.Dist )
        und sie nach draussen melden, damit die aufrufenden Methoden ggfs.
        die Objekt-Breiten entsprechend vergroessern!

        Warum?
        ======
        Weil Winword die Raender  a u s s e n  drauf malt, wir hingegen sie
        innen rein zeichnen, was bei uns zu einem kleineren Innenraum fuehrt!
    */

    return rLine;
}


BOOL lcl_IsBorder( BOOL bVer67, const WW8_BRC* pbrc, BOOL bChkBtwn=FALSE )
{
    if( bVer67  )
        return ( pbrc[WW8_TOP  ].aBits1[0] & 0x18 ) ||  // brcType  != 0
               ( pbrc[WW8_LEFT ].aBits1[0] & 0x18 ) ||
               ( pbrc[WW8_BOT  ].aBits1[0] & 0x18 ) ||
               ( pbrc[WW8_RIGHT].aBits1[0] & 0x18 ) ||
               ( bChkBtwn && ( pbrc[WW8_BETW ].aBits1[0] & 0x18 ));
                    // Abfrage auf 0x1f statt 0x18 ist noetig, da zumindest einige
                    // WW-Versionen ( 6.0 US ) bei dotted brcType auf 0 setzen
    else
        return pbrc[WW8_TOP  ].aBits1[1] ||         // brcType  != 0
               pbrc[WW8_LEFT ].aBits1[1] ||
               pbrc[WW8_BOT  ].aBits1[1] ||
               pbrc[WW8_RIGHT].aBits1[1] ||
               (bChkBtwn && pbrc[WW8_BETW ].aBits1[1]);
}

BOOL SwWW8ImplReader::IsBorder( const WW8_BRC* pbrc, BOOL bChkBtwn )
{
    return lcl_IsBorder( bVer67, pbrc, bChkBtwn );
}

BOOL SwWW8ImplReader::SetBorder( SvxBoxItem& rBox, const WW8_BRC* pbrc,
                                    BOOL bChkBtwn )
{
    SvxBorderLine aLine;
    BOOL bChange = FALSE;
    static USHORT __READONLY_DATA aIdArr[ 10 ] = {
            WW8_TOP,    BOX_LINE_TOP,
            WW8_LEFT,   BOX_LINE_LEFT,
            WW8_RIGHT,  BOX_LINE_RIGHT,
            WW8_BOT,    BOX_LINE_BOTTOM,
            WW8_BETW,   BOX_LINE_BOTTOM };


    for( int i = 0, nEnd = 8; i < nEnd; i += 2 )
    {
        // ungueltige Borders ausfiltern
        const WW8_BRC& rB = pbrc[ aIdArr[ i ] ];
        if( !(( rB.aBits1[0] == 0xff && rB.aBits1[1] == 0xff ) ||
                0 == ( bVer67
                        ? ( rB.aBits1[0] & 0x001f )     // Version 6/7
                        : rB.aBits1[1] ) ))             // Version 8
        {
            rBox.SetLine( &Set1Border( bVer67, aLine, rB ), aIdArr[ i+1 ] );
            bChange = TRUE;
        }
        else if( 6 == i && bChkBtwn )   // wenn Botton nichts war,
            nEnd += 2;                  // dann ggfs. auch Between befragen
    }
    return bChange;
}


BOOL SwWW8ImplReader::SetShadow( SvxShadowItem& rShadow,
                                const SvxBoxItem& rBox, const WW8_BRC pbrc[4] )
{
    BOOL bRet = ( bVer67 ? (pbrc[WW8_RIGHT].aBits1[ 1 ] & 0x20 )
                         : (pbrc[WW8_RIGHT].aBits2[ 1 ] & 0x20 ) ) &&
                rBox.GetRight();
    if( bRet )
    {
        rShadow.SetColor( Color( COL_BLACK ));

//          aS.SetWidth( 28 );
// JP 19.11.98: abhaengig von der Breite der rechten Kante der Box
        const SvxBorderLine& rLine = *rBox.GetRight();
        rShadow.SetWidth( ( rLine.GetOutWidth() + rLine.GetInWidth() +
                        rLine.GetDistance() ) );

        rShadow.SetLocation( SVX_SHADOW_BOTTOMRIGHT );
        bRet = TRUE;
    }
    return bRet;
}

void SwWW8ImplReader::GetBorderDistance( WW8_BRC* pbrc, Rectangle& rInnerDist )
{
    // 'dptSpace' is stored in 3 bits of 'Border Code (BRC)'
    if( bVer67 )
        rInnerDist = Rectangle(((pbrc[ 1 ].aBits1[1] >> 3) & 0x1f) * 20,
                               ((pbrc[ 0 ].aBits1[1] >> 3) & 0x1f) * 20,
                               ((pbrc[ 3 ].aBits1[1] >> 3) & 0x1f) * 20,
                               ((pbrc[ 2 ].aBits1[1] >> 3) & 0x1f) * 20 );
    else
        rInnerDist = Rectangle( (pbrc[ 1 ].aBits2[1]       & 0x1f) * 20,
                                (pbrc[ 0 ].aBits2[1]       & 0x1f) * 20,
                                (pbrc[ 3 ].aBits2[1]       & 0x1f) * 20,
                                (pbrc[ 2 ].aBits2[1]       & 0x1f) * 20 );
}


void SwWW8ImplReader::SetFlyBordersShadow(  SfxItemSet& rFlySet,
                                            const WW8_BRC pbrc[4],
                                            USHORT nInnerMgn )
{
    if( IsBorder( pbrc ) )
    {
        SvxBoxItem aBox;
        SetBorder( aBox, pbrc );

        if( nInnerMgn )
            aBox.SetDistance( nInnerMgn );          // Rand innen
        rFlySet.Put( aBox );

        // fShadow
        SvxShadowItem aShadow;
        if( SetShadow( aShadow, aBox, pbrc ))
            rFlySet.Put( aShadow );
    }
}



//-----------------------------------------
//              APOs
//-----------------------------------------
                            // fuer Berechnung der minimalen FrameSize
#define MAX_BORDER_SIZE 210         // so breit ist max. der Border
#define MAX_EMPTY_BORDER 10         // fuer +-1-Fehler, mindestens 1


static short GetLineWidth( BOOL bVer67, const WW8_BRC& rBor )
{
/*
    register UINT16 nBits = SVBT16ToShort( rBor.aBits1 );
    register short n = nBits & 0x7;      // dxpLineWidth
    if( n >= 6 )
        n = 1;
    return n * ( ( nBits >> 3 & 0x3 ) + 1 ) * 15;   // aBor.brcType
                            // brcType Linien mit je n * 3/4 point Dicke
*/

/*
JP 19.11.98: wenn dann muss es so richtig sein, die Version MUSS beachtet werden
    UINT16 nIdx = bVer67 ? ((rBor.aBits1[ 0 ] & 0x18) >> 3) + 1
                         : rBor.aBits1[ 1 ];
    UINT16 nWidth = bVer67  ? ((rBor.aBits1[ 0 ] & 0x7) * 15 )  // 3/4pt
                            : (rBor.aBits1[ 0 ] * 5 / 2);       // 1/8pt
    return nIdx * nWidth;
*/

//JP 19.11.98: aber warum werden nicht die SW-Borders zu Berechnung
//              herangezogen??
    SvxBorderLine aLine;
    ::Set1Border( bVer67, aLine, rBor );
    return aLine.GetOutWidth() + aLine.GetInWidth() + aLine.GetDistance();
}

static void FlySecur1( short& rSize, const short nMgn1, const short nMgn2,
                       const BOOL bBorder )
{
    register short nMin = MINFLY
            + ( bBorder ) ? MAX_BORDER_SIZE : MAX_EMPTY_BORDER;

    if ( rSize < nMin )
        rSize = nMin;
}

INLINE_AUSSER_HP BOOL SetValSprm( short* pVar, WW8PLCFx_Cp_FKP* pPap,
                                  USHORT nId )
{
    register BYTE* pS = pPap->HasSprm( nId );
    if( pS )
        *pVar = (INT16)SVBT16ToShort( pS );
    return ( pS != 0 );
}

INLINE_AUSSER_HP BOOL SetValSprm( short* pVar, const WW8RStyle* pStyle,
                                  USHORT nId )
{
    register BYTE* pS = pStyle->HasParaSprm( nId );
    if( pS )
        *pVar = (INT16)SVBT16ToShort( pS );
    return ( pS != 0 );
}

WW8FlyPara::WW8FlyPara( BOOL bIsVer67, const WW8FlyPara* pSrc /* = 0 */ )
{
    if ( pSrc )
        memcpy( this, pSrc, sizeof( WW8FlyPara ) ); // Copy-Ctor
    else
    {
        memset( this, 0, sizeof( WW8FlyPara ) );    // Default-Ctor
        nSp37 = 2;                                  // Default: Umfluss
    }
    bVer67 = bIsVer67;
}


// WW8FlyPara::operator == vergleicht alles, was in der Definition vor
// den Borders steht!
// dieses wird u.a. fuer TestSameApo benoetigt.
int WW8FlyPara::operator == ( const WW8FlyPara& rSrc ) const
{
    ASSERT( ( (BYTE*)rSrc.brc - (BYTE*)&rSrc < sizeof( WW8FlyPara ) ),
            "WW8FlyPara::operator == geht schief" );
    return !memcmp( this, &rSrc, (BYTE*)rSrc.brc - (BYTE*)&rSrc );
            // memcmp ist moeglich, da die gesamte Struktur beim Initialisieren
            // incl. Luecken mit 0 gefuellt wird und sich damit durch die
            // Luecken keine falschen Unterschiede ergeben koennen.
            // Ausserdem sind alle Elemente aligned, so dass keine Luecken
            // existieren
}


// Read fuer normalen Text

BOOL WW8FlyPara::Read( BYTE* pSprm29, WW8PLCFx_Cp_FKP* pPap )
{
//  WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();

    if( pSprm29 )
        nSp29 = *pSprm29;                           // PPC ( Bindung )

    BYTE* pS = 0;

    if( bVer67 )
    {
        SetValSprm( &nSp26,         pPap, 26 ); // X-Position       //sprmPDxaAbs
        SetValSprm( &nSp27,         pPap, 27 ); // Y-Position       //sprmPDyaAbs
        SetValSprm( (short*)&nSp45, pPap, 45 ); // Hoehe                //sprmPWHeightAbs
        SetValSprm( (short*)&nSp28, pPap, 28 ); // Breite               //sprmPDxaWidth
        SetValSprm( &nSp49,         pPap, 49 ); // L/R-Raender  //sprmPDxaFromText
        SetValSprm( &nSp48,         pPap, 48 ); // U/L-Raender  //sprmPDyaFromText

        pS = pPap->HasSprm( 37 );                               // Umfluss          //sprmPWr
        if( pS )
            nSp37 = *pS;

        pS = pPap->HasSprm( 46 );                               // DropCap          //sprmPDcs
    }
    else
    {
        SetValSprm( &nSp26,         pPap, 0x8418 ); // X-Position
        SetValSprm( &nSp27,         pPap, 0x8419 ); // Y-Position
        SetValSprm( (short*)&nSp45, pPap, 0x442B ); // Hoehe
        SetValSprm( (short*)&nSp28, pPap, 0x841A ); // Breite
        SetValSprm( &nSp49,         pPap, 0x842F ); // L/R-Raender
        SetValSprm( &nSp48,         pPap, 0x842E ); // U/L-Raender

        pS = pPap->HasSprm( 0x2423 );                               // Umfluss
        if( pS )
            nSp37 = *pS;

        pS = pPap->HasSprm( 0x442C );                               // DropCap
    }

    bDropCap = pS != 0;

    if( !nSp29 && !nSp27 && !nSp49 && !nSp37 )      // alles 0 heisst
        return FALSE;                               // Apo ist nicht vorhanden

    if( ::lcl_ReadBorders( bVer67, brc, pPap ))     // Umrandung
    {
        bBorderLines = ::lcl_IsBorder( bVer67, brc );
        bBorder = TRUE;
    }
    else
        bBorderLines = bBorder = FALSE;
    return TRUE;
}

BOOL WW8FlyPara::ReadFull( BYTE* pSprm29, SwWW8ImplReader* pIo )
{
    WW8PLCFMan* pPlcxMan = pIo->pPlcxMan;
    WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();

    BOOL bOk = Read( pSprm29, pPap );   // Lies Apo-Parameter

    do{             // Block zum rausspringen
        if( !bOk )                      // schiefgegangen
            break;
        if( nSp45 != 0 /* || nSp28 != 0 */ )
            break;                      // bGrafApo nur bei Hoehe automatisch
        if( pIo->pWwFib->fComplex )
            break;                      // (*pPap)++ geht bei FastSave schief
                                        // -> bei FastSave kein Test auf Grafik-APO
        SvStream* pIoStrm = pIo->pStrm;
        ULONG nPos = pIoStrm->Tell();
        WW8PLCFxSave1 aSave;
        pPlcxMan->GetPap()->Save( aSave );
        bGrafApo = FALSE;

        do{             // Block zum rausspringen

            BYTE nTxt[2];

            pIoStrm->Read( nTxt, 2 );                   // lies Text
            if( nTxt[0] != 0x01 || nTxt[1] != 0x0d )// nur Grafik + CR ?
                break;                              // Nein

            (*pPap)++;                              // Naechste Zeile

            // In APO ?
            //sprmPPc
            BYTE* pS = pPap->HasSprm( bVer67 ? 29 : 0x261B );

            // Nein -> Grafik-Apo
            if( !pS ){
                bGrafApo = TRUE;
                break;                              // Ende des APO
            }
            USHORT nColl = pPap->GetIstd();
            if( nColl >= pIo->nColls || !pIo->pCollA[nColl].pFmt
                || !pIo->pCollA[nColl].bColl )
                nColl = 0;                          // Unguelige Style-Id

            BOOL bNowStyleApo = pIo->pCollA[nColl].pWWFly != 0; // Apo in StyleDef
    //      BOOL bNowApo = bNowStyleApo || ( pS != 0 ); // hier Apo
            WW8FlyPara aF( bVer67, bNowStyleApo ? pIo->pCollA[nColl].pWWFly : 0 );
                                                    // Neuer FlaPara zum Vergleich
            aF.Read( pS, pPap );                    // WWPara fuer neuen Para
            if( !( aF == *this ) )                  // selber APO ? ( oder neuer ? )
                bGrafApo = TRUE;                    // nein -> 1-zeiliger APO
                                                    //      -> Grafik-APO
        }
        while( 0 );                                 // Block zum rausspringen

        pPlcxMan->GetPap()->Restore( aSave );
        pIoStrm->Seek( nPos );
    }while( 0 );                                    // Block zum rausspringen
    return bOk;
}


// Read fuer Apo-Defs in Styledefs
BOOL WW8FlyPara::Read( BYTE* pSprm29, WW8RStyle* pStyle )
{
    if( pSprm29 )
        nSp29 = *pSprm29;                           // PPC ( Bindung )

    BYTE* pS = 0;
    if( bVer67 )
    {
        SetValSprm( &nSp26,         pStyle, 26 );   // X-Position
        SetValSprm( &nSp27,         pStyle, 27 );   // Y-Position
        SetValSprm( (short*)&nSp45, pStyle, 45 );   // Hoehe
        SetValSprm( (short*)&nSp28, pStyle, 28 );   // Breite
        SetValSprm( &nSp49,         pStyle, 49 );   // L/R-Raender
        SetValSprm( &nSp48,         pStyle, 48 );   // U/L-Raender

        pS = pStyle->HasParaSprm( 37 );             // Umfluss
        if( pS )
            nSp37 = *pS;

        pS = pStyle->HasParaSprm( 46 );             // DropCap
    }
    else
    {
        SetValSprm( &nSp26,         pStyle, 0x8418 );   // X-Position
        SetValSprm( &nSp27,         pStyle, 0x8419 );   // Y-Position
        SetValSprm( (short*)&nSp45, pStyle, 0x442B );   // Hoehe
        SetValSprm( (short*)&nSp28, pStyle, 0x841A );   // Breite
        SetValSprm( &nSp49,         pStyle, 0x842F );   // L/R-Raender
        SetValSprm( &nSp48,         pStyle, 0x842E );   // U/L-Raender

        pS = pStyle->HasParaSprm( 0x2423 );             // Umfluss
        if( pS )
            nSp37 = *pS;

        pS = pStyle->HasParaSprm( 0x442C );             // DropCap
    }

    bDropCap = pS != 0;

    if( !nSp29 && !nSp27 && !nSp49 && !nSp37 )      // alles 0 heisst
        return FALSE;                               // Apo ist nicht vorhanden

    if( ::lcl_ReadBorders( bVer67, brc, 0, pStyle ) )       // Umrandung
    {
        bBorderLines = ::lcl_IsBorder( bVer67, brc );
        bBorder = TRUE;
    }
    else
        bBorderLines = bBorder = FALSE;
    return TRUE;
}




WW8SwFlyPara::WW8SwFlyPara( SwPaM& rPaM,
                            SwWW8ImplReader& rIo,
                            WW8FlyPara& rWW,
                            short nPgTop,
                            short nPgLeft,
                            short nPgWidth,
                            INT32 nIniFlyDx, INT32 nIniFlyDy )
{
    memset( this, 0, sizeof( WW8SwFlyPara ) );  // Initialisieren
    nNewNettoWidth = MINFLY;                    // Minimum

    eSurround = ( rWW.nSp37 > 1 ) ? SURROUND_PARALLEL : SURROUND_NONE;

    /*
     #83307# These old style WinWord textboxes are a terrible problem for
     headers and footers. They can be anchored in a footer and moved to just
     about anyplace. Also they have flexible heights and can be higher than
     the actual header/footer area, if we do not set wrap to NONE then boxes
     narrower than a header/footer but taller will be forced into the
     header/footer area and the look of the word original will be lost
    */
    if (rIo.bIsHeader || rIo.bIsFooter)
        eSurround = SURROUND_NONE;

    nHeight = rWW.nSp45;
    if( nHeight & 0x8000 )
    {
        nHeight &= 0x7fff;
        eHeightFix = ATT_MIN_SIZE;
    }
    else
        eHeightFix = ATT_FIX_SIZE;

    if( nHeight <= MINFLY )
    {                           // keine Angabe oder Stuss
        eHeightFix = ATT_MIN_SIZE;
        nHeight = MINFLY;
    }

    nWidth = nNettoWidth = rWW.nSp28;
    if( nWidth <= 10 )                              // Auto-Breite
    {
        bAutoWidth = TRUE;
        nWidth = nNettoWidth =
            rWW.bDropCap ? MINFLY : (nPgWidth ? nPgWidth : 2268); // 4 cm
    }
    if( nWidth <= MINFLY )
        nWidth = nNettoWidth = MINFLY;              // Minimale Breite

    eVAlign = VERT_NONE;                            // Defaults
    eHAlign = HORI_NONE;
    nYPos = 0;
    nXPos = 0;

    nRiMgn = nLeMgn = rWW.nSp49;
    nLoMgn = nUpMgn = rWW.nSp48;

                // Wenn der Fly links, rechts, oben oder unten aligned ist,
                // wird der aeussere Textabstand ignoriert, da sonst
                // der Fly an falscher Position landen wuerde
                // JP 18.11.98: Problematisch wird es nur bei Innen/Aussen

    switch( rWW.nSp27 )             // besondere Y-Positionen ?
    {
    case -4:  eVAlign = VERT_TOP;       nUpMgn = 0; break;  // oben
    case -8:  eVAlign = VERT_CENTER;                break;  // zentriert
    case -12: eVAlign = VERT_BOTTOM;    nLoMgn = 0; break;  // unten
    default:  nYPos = rWW.nSp27 + (short)nIniFlyDy; break;  // Korrekturen per Ini-Datei
    }

    switch( rWW.nSp26 )                 // besondere X-Positionen ?
    {
    case 0:     eHAlign = HORI_LEFT;        nLeMgn = 0; break;  // links
    case -4:    eHAlign = HORI_CENTER;                  break;  // zentriert
    case -8:    eHAlign = HORI_RIGHT;       nRiMgn = 0; break;  // rechts
    case -12:   eHAlign = HORI_LEFT; bToggelPos = TRUE; break;  // innen
    case -16:   eHAlign = HORI_RIGHT; bToggelPos = TRUE;break;  // aussen
    default:  nXPos = rWW.nSp26 + (short)nIniFlyDx;     break;  // Korrekturen per Ini-Datei
    }

    // Bindung
    BYTE nYBind = (( rWW.nSp29 & 0x30 ) >> 4);
    switch ( nYBind )
    {                                       // Y - Bindung bestimmt Sw-Bindung
    case 0: eAnchor = FLY_PAGE;             // Vert Margin
            eVRel = REL_PG_PRTAREA;
            break;
    case 1: eAnchor = FLY_PAGE;             // Vert Page
            eVRel = REL_PG_FRAME;
            break;                          // 2=Vert. Paragraph, 3=Use Default
    default:eAnchor = FLY_AT_CNTNT;
            eVRel = PRTAREA;
            if( nYPos < 0 )
                nYPos = 0;                  // koennen wir nicht
            break;
    }

    if( (rIo.bIsHeader || rIo.bIsFooter) && (FLY_AT_CNTNT != eAnchor) )
    {
        eAnchor = FLY_AT_CNTNT;
        if( rIo.bIsHeader )
        {
            nYPos -= nPgTop;
            // nXPos will be decreased at the very end of this C'tor
        }
        else
        {
            nYPos = 0;
        }
        rIo.pNode_FLY_AT_CNTNT = &rPaM.GetPoint()->nNode.GetNode();
    }

    BYTE nXBind = ( rWW.nSp29 & 0xc0 ) >> 6;
    switch ( nXBind )           // X - Bindung -> Koordinatentransformation
    {
    case 0:                                 // Hor. Spalte
    case 1:                                 // Hor. Absatz
            eHRel = (FLY_PAGE == eAnchor) ? REL_PG_PRTAREA : PRTAREA;
            break;
/*  case 2:*/                               // Hor. Seite
/*  case 3:*/                               // Use Default
    default:{
                eHRel = (FLY_PAGE == eAnchor) ? REL_PG_FRAME : FRAME;

                // important: allways set REL_PG_FRAME in sections with columns
                if( eHRel != REL_PG_FRAME )
                {
                    const SwSectionNode* pSectNd
                        = rPaM.GetPoint()->nNode.GetNode().FindSectionNode();
                    if( pSectNd )
                    {
                        const SwSectionFmt* pFmt
                            = pSectNd->GetSection().GetFmt();
                        if( pFmt )
                        {
                            if( 1 < pFmt->GetCol().GetNumCols() )
                                eHRel = REL_PG_FRAME;
                        }
                    }
                }
            }
            break;
    }

    INT16 nLeLMgn = 0, nRiLMgn = 0;     // aus Liniendicke entstehende Raender

    if( rWW.bBorder )                   // Raender innerhalb der Umrandung
    {
        WW8_BRC5& rBrc = rWW.brc;

        // dxpSpace aller 4 Borders bestimmen, zum InnerMargin aufaddieren
        // und die linken/rechten zur Groesse dazu rechnen
        // Die Angaben sind in Point
        short nLeft, nRight;
        if( rWW.bVer67 )
        {
            nLeft  = (rBrc[WW8_LEFT ].aBits1[1] >> 3) & 0x1f;   // dxpSpace
            nRight = (rBrc[WW8_RIGHT].aBits1[1] >> 3) & 0x1f;   // dxpSpace
            // muessen obere / untere Liniendicken auch beruecksichtigt werden ???

            nInnerMgn = (( rBrc[WW8_TOP].aBits1[1] ) >> 3 & 0x1f) +
                        (( rBrc[WW8_BOT].aBits1[1] ) >> 3 & 0x1f);
        }
        else
        {
            nLeft  = rBrc[WW8_LEFT ].aBits2[1] & 0x1f;      // dxpSpace
            nRight = rBrc[WW8_RIGHT].aBits2[1] & 0x1f;      // dxpSpace
            // muessen obere / untere Liniendicken auch beruecksichtigt werden ???

            nInnerMgn = (rBrc[WW8_TOP].aBits2[1] & 0x1f) +
                        (rBrc[WW8_BOT].aBits2[1] & 0x1f);
        }

        nLeLMgn += 20 * nLeft;          // dxpSpace
        nRiLMgn += 20 * nRight;         // dxpSpace

        nLeLMgn += GetLineWidth( rWW.bVer67, rBrc[WW8_LEFT] );
        nRiLMgn += GetLineWidth( rWW.bVer67, rBrc[WW8_RIGHT] );

        nInnerMgn += nLeft + nRight;
        nInnerMgn *= 5;     // Mittelwert in Twips (20 * 4 Kanten)
    }
                                            // Raender ausserhalb der Umrandung
    nWidth += nLeLMgn + nRiLMgn;
//  nHeight += nUpLMgn + nLoLMgn;

    FlySecur1( nWidth, nLeMgn, nRiMgn, rWW.bBorderLines );          // passen Raender ?
    FlySecur1( nHeight, nUpMgn, nLoMgn, rWW.bBorderLines );

            // Seitenrand-Bindung: Wenn die Position mit der jetzigen
            // SW-Bindung nicht zu erreichen ist, dann waehle andere Bindung
            // Absatz-Bindung: Nicht anfassen
    if(     (eAnchor == FLY_PAGE      ) // Bindung: Seitenrand
        &&  (eHRel   == REL_PG_PRTAREA) // und abs. positioniert
        &&  (eVAlign == VERT_NONE     )
        &&  (eHAlign == HORI_NONE     ) )
    {
        if( nYPos < nPgTop )            // echter neg. Y-Wert
            eHRel = eVRel = REL_PG_FRAME;
        if( nXPos < nPgLeft )           // echter neg. X-Wert
            eHRel = eVRel = REL_PG_FRAME;
    }

    /*
        // eine Writer-Kuriositaet: auch wenn Abstaende vom Seitenrand
        // gezaehlt werden sollen, muessen die Positionen als Abstaende vom
        // Papierrand angegeben werden
        // bei Absatzgebundenen Frames geht die Zaehlung immer von
        // der Printarea aus
    */
    if( (FRAME == eHRel) && (FLY_AT_CNTNT == eAnchor) )
    {
        // hier duerfen neg. Werte bis minimal -nPgLeft entstehen
        nXPos -= nPgLeft;
        if( rIo.bTable )
            nXPos -= rIo.GetTableLeft();
    }
}

// hat ein Fly in WW eine automatische Breite, dann muss das durch
// nachtraegliches Anpassen der ( im SW festen ) Fly-Breite simuliert werden.
// Dabei kann die Fly-Breite groesser oder kleiner werden, da der Default-Wert
// ohne Wissen ueber den Inhalt eingesetzt wird.
void WW8SwFlyPara::BoxUpWidth( long nWidth )
{
    if( bAutoWidth && nWidth > nNewNettoWidth )
        nNewNettoWidth = nWidth;
};

// Die Klasse WW8FlySet ist von SfxItemSet abgeleitet und stellt auch
// im Prizip nicht mehr zur Verfuegung, ist aber fuer mich besser
// zu handeln
// WW8FlySet-ctor fuer Apos und Graf-Apos
WW8FlySet::WW8FlySet( SwWW8ImplReader& rReader, /*const*/ WW8FlyPara* pFW,
                    /*const*/ WW8SwFlyPara* pFS, BOOL bGraf )
:SfxItemSet( rReader.rDoc.GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1 )

{
    if( !rReader.bNew )
        Reader::ResetFrmFmtAttrs( *this );  // Abstand/Umrandung raus

                                            // Position
    Put( SwFmtHoriOrient( pFS->nXPos, pFS->eHAlign, pFS->eHRel, pFS->bToggelPos ));
    Put( SwFmtVertOrient( pFS->nYPos, pFS->eVAlign, pFS->eVRel ) );

    if( pFS->nLeMgn || pFS->nRiMgn )        // Raender setzen
    {
        SvxLRSpaceItem aLR;
        aLR.SetTxtLeft( pFS->nLeMgn );
        aLR.SetRight( pFS->nRiMgn );
        Put( aLR );
    }
    if( pFS->nUpMgn || pFS->nLoMgn )
    {
        SvxULSpaceItem aUL;
        aUL.SetUpper( pFS->nUpMgn );
        aUL.SetLower( pFS->nLoMgn );
        Put( aUL );
    }


    SwFmtSurround aSur( pFS->eSurround );   // Umfluss

//  GoldCut umfliesst inzwischen nur dann auf beiden Seiten, wenn der Fly
//  schmaler als ca. 1.5cm ist. Also entspricht normales Parallel besser
//  dem WW-Verhalten.
//  aSur.SetGoldCut( pFS->eSurround == SURROUND_PARALLEL );

    Put( aSur );

    rReader.SetFlyBordersShadow( *this, (WW8_BRC*)pFW->brc,
                                          pFS->nInnerMgn );
            // der 5. Parameter ist immer 0, daher geht beim Cast nix verloren

    if( !bGraf ){           // Textrahmen->Anker und Groesse einstellen
        Put( SwFmtAnchor( pFS->eAnchor, 1 ) );
                                            // Groesse einstellen
        Put( SwFmtFrmSize( pFS->eHeightFix, pFS->nWidth, pFS->nHeight ));
    }
}

// WW8FlySet-ctor fuer zeichengebundene Grafiken
WW8FlySet::WW8FlySet( SwWW8ImplReader& rReader, const SwPaM* pPaM,
                    const WW8_PIC& rPic, long nWidth, long nHeight )
:SfxItemSet( rReader.rDoc.GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1 )
{
    if( !rReader.bNew )
        Reader::ResetFrmFmtAttrs( *this );  // Abstand/Umrandung raus

    SwFmtAnchor aAnchor( FLY_IN_CNTNT );
    aAnchor.SetAnchor( pPaM->GetPoint() );
    Put( aAnchor );
    Put( SwFmtFrmSize( ATT_FIX_SIZE, nWidth, nHeight ) );

//  Put( SwFmtHoriOrient( 0, HORI_NONE, REL_CHAR, FALSE ));
//  Put( SwFmtVertOrient( 0, VERT_NONE, REL_CHAR ) );
    Put( SwFmtVertOrient( 0, VERT_TOP, FRAME ));

    rReader.SetFlyBordersShadow( *this, rPic.rgbrc, 0 );
}


BOOL SwWW8ImplReader::StartApo( BYTE* pSprm29, BOOL bNowStyleApo )
{
    pWFlyPara = new WW8FlyPara ( bVer67, bNowStyleApo  ?  pCollA[nAktColl].pWWFly : 0 );

    // APO-Parameter ermitteln und Test auf bGrafApo
    if( !pWFlyPara->ReadFull( pSprm29, this ) )
    {
        DELETEZ( pWFlyPara );
        return FALSE;
    }


    pSFlyPara = new WW8SwFlyPara( *pPaM,
                                  *this,
                                  *pWFlyPara,
                                  nPgTop,
                                  nPgLeft,
                                  (nPgWidth - nPgRight - nPgLeft),
                                  nIniFlyDx,
                                  nIniFlyDy );
        // nPg... nicht aus PageDesc, da bei Defaultraendern noch nicht gesetzt

    if( !pWFlyPara->bGrafApo )
    {

        // Innerhalb des GrafApo muessen Textattribute
        // ignoriert werden, da sie sonst auf den
        // folgenden Zeilen landen.
        // Der Rahmen wird nur eingefuegt, wenn er
        // *nicht* nur zum Positionieren einer einzelnen
        // Grafik dient.
        // Ist es ein Grafik-Rahmen, dann werden
        // pWFlyPara und pSFlyPara behalten und die
        // daraus resultierenden Attribute beim
        // Einfuegen der Grafik auf die Grafik angewendet.

//      SwAttrSet aFlySet( rDoc.GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );
//      SetApoCharacteristics( aFlySet, pWFlyPara, pSFlyPara );
        WW8FlySet aFlySet( *this, pWFlyPara, pSFlyPara, FALSE );

        pSFlyPara->pFlyFmt = rDoc.MakeFlySection( pSFlyPara->eAnchor,
                                              pPaM->GetPoint(), &aFlySet );
        if( FLY_IN_CNTNT != pSFlyPara->eAnchor )
            pCtrlStck->NewAttr( *pPaM->GetPoint(), SwFltAnchor( pSFlyPara->pFlyFmt ) );

                                // merke Pos im Haupttext
        pSFlyPara->pMainTextPos = new SwPosition( *pPaM->GetPoint() );
        nLastFlyNode = pSFlyPara->pMainTextPos->nNode.GetIndex();

                                // Alle Attribute schliessen, da sonst
                                // Attribute entstehen koennen, die
                                // in Flys reinragen
        pCtrlStck->SetAttr( *pPaM->GetPoint(), 0, FALSE );
        pEndStck->SetAttr( *pPaM->GetPoint(), 0, FALSE );


                                // Setze Pam in den FlyFrame
        const SwFmtCntnt& rCntnt = pSFlyPara->pFlyFmt->GetCntnt();
        ASSERT( rCntnt.GetCntntIdx(), "Kein Inhalt vorbereitet." );
        pPaM->GetPoint()->nNode = rCntnt.GetCntntIdx()->GetIndex() + 1;
        pPaM->GetPoint()->nContent.Assign( pPaM->GetCntntNode(), 0 );

        // 1) ReadText() wird nicht wie beim W4W-Reader rekursiv aufgerufen,
        //    da die Laenge des Apo zu diesen Zeitpunkt noch nicht feststeht,
        //    ReadText() diese Angabe aber braucht.
        // 2) Der CtrlStck wird nicht neu erzeugt.
        //    die Char-Attribute laufen weiter ( AErger mit SW-Attributen )
        //    Paraattribute muessten am Ende jeden Absatzes zurueckgesetzt
        //    sein, d.h. es duerften am Absatzende keine Paraattribute
        //    auf dem Stack liegen
    }
    return TRUE;
}

BOOL SwWW8ImplReader::JoinNode( SwPaM* pPam, BOOL bStealAttr )
{
    BOOL bRet = FALSE;
    pPam->GetPoint()->nContent = 0;         // an den Anfang der Zeile gehen

    SwNodeIndex aPref( pPam->GetPoint()->nNode, -1 );

    SwTxtNode* pNode = aPref.GetNode().GetTxtNode();
    if( pNode )
    {
        pPaM->GetPoint()->nNode = aPref;
        pPaM->GetPoint()->nContent.Assign( pNode, pNode->GetTxt().Len() );

        if( bStealAttr )
            pCtrlStck->StealAttr( pPam->GetPoint() );
        pNode->JoinNext();
        bRet = TRUE;
    }
    return bRet;
}

void SwWW8ImplReader::StopApo()
{
    if( pWFlyPara->bGrafApo ){  // Grafik-Rahmen, der *nicht* eingefuegt wurde
                                // leeren Absatz incl. Attributen entfernen
        JoinNode( pPaM, TRUE );

    }
    else
    {                           // Der Rahmen wurde nur eingefuegt, wenn er
                                // *nicht* nur zum Positionieren einer einzelnen
                                // Grafik dient.
        JoinNode( pPaM, FALSE );// UEberfluessigen Absatz entfernen

        if( !pSFlyPara->pMainTextPos || !pWFlyPara ){
            ASSERT( pSFlyPara->pMainTextPos, "StopApo: pMainTextPos ist 0" );
            ASSERT( pWFlyPara, "StopApo: pWFlyPara ist 0" );
            return;
        }
                                // Alle Attribute schliessen, da sonst
                                // Attribute entstehen koennen, die
                                // aus Flys rausragen
        pCtrlStck->SetAttr( *pPaM->GetPoint(), 0, FALSE );
        pEndStck->SetAttr( *pPaM->GetPoint(), 0, FALSE );


// Ist die Fly-Breite durch eine innenliegende Grafik vergroessert worden
// ( bei automatischer Breite des Flys ), dann muss die Breite des SW-Flys
// entsprechend umgesetzt werden, da der SW keine automatische Breite kennt.

        if( pSFlyPara->nNewNettoWidth > MINFLY )    // BoxUpWidth ?
        {
            long nW = pSFlyPara->nNewNettoWidth;
            nW += pSFlyPara->nWidth - pSFlyPara->nNettoWidth;   // Rand dazu
            pSFlyPara->pFlyFmt->SetAttr(
                SwFmtFrmSize( pSFlyPara->eHeightFix, nW, pSFlyPara->nHeight ) );
        }
        /*
        #83307# Word set *no* width meaning its an automatic width. The
        SwFlyPara reader will have already set a fallback width of the
        printable regions width, so we should reuse it. Despite the related
        problems with layout addressed with a hack in WW8FlyPara's constructor
        */
        else if( !pWFlyPara->nSp28 )
        {
            SfxItemSet aFlySet( pSFlyPara->pFlyFmt->GetAttrSet() );
            aFlySet.ClearItem( RES_FRM_SIZE );
            CalculateFlySize( aFlySet, pSFlyPara->pMainTextPos->nNode,
                pSFlyPara->nWidth );
            pSFlyPara->pFlyFmt->SetAttr( aFlySet.Get( RES_FRM_SIZE ) );
        }

        *pPaM->GetPoint() = *pSFlyPara->pMainTextPos;
        DELETEZ( pSFlyPara->pMainTextPos );

// Damit die Frames bei Einfuegen in existierendes Doc erzeugt werden,
// wird in fltshell.cxx beim Setzen des FltAnchor-Attributes
// pFlyFrm->MakeFrms() gerufen

    }
    DELETEZ( pSFlyPara );
    DELETEZ( pWFlyPara );
}

// TestSameApo() beantwortet die Frage, ob es dasselbe APO oder ein neues ist
BOOL SwWW8ImplReader::TestSameApo( BYTE* pSprm29, BOOL bNowStyleApo )
{
    if( !pWFlyPara )
    {
        ASSERT( pWFlyPara, " Wo ist mein pWFlyPara ? " );
        return TRUE;
    }

                        // Es muss ein kompletter Vergleich ( ausser Borders )
                        // stattfinden, um alle Kombinationen Style / Hart
                        // richtig einzuordnen. Deshalb wird ein temporaerer
                        // WW8FlyPara angelegt ( abh. ob Style oder nicht ),
                        // darauf die harten Attrs angewendet, und
                        // dann verglichen

    WW8FlyPara aF( bVer67, bNowStyleApo ? pCollA[nAktColl].pWWFly : 0 ); // Zum Vergleich
    aF.Read( pSprm29, pPlcxMan->GetPapPLCF() );         // WWPara fuer akt. Para
    return aF == *pWFlyPara;
}



/***************************************************************************
#       Attribut - Verwaltung
#**************************************************************************/

void SwWW8ImplReader::NewAttr( const SfxPoolItem& rAttr )
{
    if( !bNoAttrImport ){   // zum Ignorieren von Styles beim Doc-Einfuegen
        if( pAktColl )
            pAktColl->SetAttr( rAttr );
        else
        if( pAktItemSet )
            pAktItemSet->Put( rAttr );
        else
            pCtrlStck->NewAttr( *pPaM->GetPoint(), rAttr );
    }
}

// holt Attribut aus der FmtColl / Stack / Doc
const SfxPoolItem* SwWW8ImplReader::GetFmtAttr( USHORT nWhich )
{
    if( pAktColl )
        return &pAktColl->GetAttr( nWhich );
    else
    if( pAktItemSet )
    {
        const SfxPoolItem* pRet = pAktItemSet->GetItem( nWhich );
        return pRet ? pRet : &pStandardFmtColl->GetAttr( nWhich );
    }
    else
        return pCtrlStck->GetFmtAttr( *pPaM->GetPoint(), nWhich );
}

#if 0
// holt Attribut aus der FmtColl / Stack, return 0 wenn nicht gefunden
const SfxPoolItem* SwWW8ImplReader::GetFmtStkAttr( USHORT nWhich )
{
    if( pAktColl )
        return &pAktColl->GetAttr( nWhich );
    else
        return pCtrlStck->GetFmtStkAttr( *pPaM->GetPoint(), nWhich );
}
#endif

/***************************************************************************
#       eigentliche Attribute
#
# Die Methoden erhalten die Token-Id und die Laenge der noch folgenden
# Parameter gemaess Tabelle in WWScan.cxx als Parameter
#**************************************************************************/

/***************************************************************************
#  Spezial WW - Attribute
#**************************************************************************/

void SwWW8ImplReader::Read_Special( USHORT, BYTE* pData, short nLen )
{
    if( nLen < 0 ){
        bSpec = FALSE;
        return;
    }
    bSpec = ( *pData != 0 );
}

// Read_Obj wird fuer fObj und fuer fOle2 benutzt !
void SwWW8ImplReader::Read_Obj( USHORT nId, BYTE* pData, short nLen )
{
    if( nLen < 0 )
        bObj = FALSE;
    else
    {
        bObj = 0 != *pData;

        if( bObj && nPicLocFc && bEmbeddObj )
            nObjLocFc = nPicLocFc;
    }
}

void SwWW8ImplReader::Read_PicLoc( USHORT nId, BYTE* pData, short nLen )
{
    if( nLen < 0 )
    {
        nPicLocFc = 0;
        bSpec = FALSE;  // Stimmt das immer ?
    }
    else
    {
        nPicLocFc = SVBT32ToLong( pData );
        bSpec = TRUE;

        if( bObj && nPicLocFc && bEmbeddObj )
            nObjLocFc = nPicLocFc;
    }
}


void SwWW8ImplReader::Read_POutLvl( USHORT nId, BYTE* pData, short nLen )
{
    if( pAktColl && (0 < nLen) )
    {
        SwWW8StyInf* pSI = &pCollA[nAktColl];
        pSI->nOutlineLevel =
              ( (1 <= pSI->GetWWStyleId()) && (9 >= pSI->GetWWStyleId()) )
            ? pSI->GetWWStyleId()-1
            : (pData ? *pData : 0);
    }
}


void SwWW8ImplReader::Read_Symbol( USHORT nId, BYTE* pData, short nLen )
{
    if( !bIgnoreText )
    {
        if( nLen < 0 )
            bSymbol = FALSE;
        else
        {
            // neues Font-Atribut aufmachen
            // (wird in SwWW8ImplReader::ReadChars() geschlossen)
            if( SetNewFontAttr( SVBT16ToShort( pData ), FALSE, RES_CHRATR_FONT ))
            {
                if( bVer67 )
                    cSymbol = ByteString::ConvertToUnicode(
                                *(sal_Char*)(pData+2), RTL_TEXTENCODING_MS_1252 );
                else
                    cSymbol = SVBT16ToShort( pData+2 );
                bSymbol = TRUE;
            }
        }
    }
}

/***************************************************************************
#  Zeichen - Attribute
#**************************************************************************/

static USHORT __FAR_DATA nEndIds[ 8 + 2 ] = {
        RES_CHRATR_WEIGHT,          RES_CHRATR_POSTURE,
        RES_CHRATR_CROSSEDOUT,      RES_CHRATR_CONTOUR,
        RES_CHRATR_SHADOWED,        RES_CHRATR_CASEMAP,
        RES_CHRATR_CASEMAP,         RES_CHRATR_CROSSEDOUT,

        RES_CHRATR_CJK_WEIGHT,      RES_CHRATR_CJK_POSTURE
    };

// Read_BoldUsw fuer Italic, Bold, Kapitaelchen, Versalien, durchgestrichen,
// Contour und Shadow
void SwWW8ImplReader::Read_BoldUsw( USHORT nId, BYTE* pData, short nLen )
{
    BYTE nI;
    // die Attribut-Nr fuer "doppelt durchgestrichen" tanzt aus der Reihe
    if( 0x2A53 != nId )
        nI = bVer67 ? nId - 85 : nId - 0x0835;  // Index 0..6
    else
        nI = 7;                         // Index 7 (Doppelt durchgestrichen)

    BYTE nMask = 1 << nI;

    if( nLen < 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), nEndIds[ nI ] );
        // reset the CJK Weight and Posture
        if( nI < 2 )
            pCtrlStck->SetAttr( *pPaM->GetPoint(), nEndIds[ 8 + nI ] );
        pCtrlStck->SetToggleAttr( nI, FALSE );
        return;
    }
                            // Wert: 0 = Aus, 1 = An,
                            //       128 = Wie Style, 129 entgegen Style
    BOOL bOn = *pData & 1;
    SwWW8StyInf* pSI = &pCollA[nAktColl];

    if( pAktColl )                          // StyleDef -> Flags merken
    {
        if( pSI->nBase < nColls             // Style Based on
            && ( *pData & 0x80 )            // Bit 7 gesetzt ?
            && ( pCollA[pSI->nBase].n81Flags & nMask ) ) // BasisMaske ?
            bOn = !bOn;                     // umdrehen

        if( bOn )
            pSI->n81Flags |= nMask;         // Flag setzen
        else
            pSI->n81Flags &= ~nMask;        // Flag loeschen
    }
    else
    {                                       // im Text -> Flags abfragen
        if( *pData & 0x80 )                 // Bit 7 gesetzt ?
        {
            if( pSI->n81Flags & nMask )     // und in StyleDef an ?
                bOn = !bOn;                 // dann invertieren
            // am Stack vermerken, das dieses ein Toggle-Attribut ist
            pCtrlStck->SetToggleAttr( nI, TRUE );
        }
    }

    SetToggleAttr( nI, bOn );
}

void SwWW8ImplReader::SetToggleAttr( BYTE nAttrId, BOOL bOn )
{
    switch( nAttrId )
    {
    case 0:
        {
            SvxWeightItem aAttr( bOn ? WEIGHT_BOLD : WEIGHT_NORMAL );
            NewAttr( aAttr );
            aAttr.SetWhich( RES_CHRATR_CJK_WEIGHT );
            NewAttr( aAttr );
        }
        break;
    case 1:
        {
            SvxPostureItem aAttr( bOn ? ITALIC_NORMAL : ITALIC_NONE );
            NewAttr( aAttr );
            aAttr.SetWhich( RES_CHRATR_CJK_POSTURE );
            NewAttr( aAttr );
        }
        break;
    case 2: NewAttr( SvxCrossedOutItem( bOn ? STRIKEOUT_SINGLE : STRIKEOUT_NONE ) );
            break;
    case 3: NewAttr( SvxContourItem( bOn ) );
            break;
    case 4: NewAttr( SvxShadowedItem( bOn ) );
            break;
    case 5: NewAttr( SvxCaseMapItem( bOn ? SVX_CASEMAP_KAPITAELCHEN
                                          : SVX_CASEMAP_NOT_MAPPED ) );
            break;
    case 6: NewAttr( SvxCaseMapItem( bOn ? SVX_CASEMAP_VERSALIEN
                                         : SVX_CASEMAP_NOT_MAPPED ) );
            break;
    case 7: NewAttr( SvxCrossedOutItem( bOn ? STRIKEOUT_DOUBLE
                                            : STRIKEOUT_NONE ) );
            break;
    }
}

void SwWW8ImplReader::_ChkToggleAttr( USHORT nOldStyle81Mask,
                                        USHORT nNewStyle81Mask )
{
    USHORT i = 1, nToggleAttrFlags = pCtrlStck->GetToggleAttrFlags();
    BYTE n = 0;
    for( ; n < 7; ++n, i <<= 1 )
        if( (i & nToggleAttrFlags) &&
            ( (i & nOldStyle81Mask) != (i & nNewStyle81Mask)))
        {
            SetToggleAttr( n, 0 != (i & nOldStyle81Mask ) );
        }
}

void SwWW8ImplReader::Read_SubSuper( USHORT, BYTE* pData, short nLen )
{
    if( nLen < 0 ){
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_ESCAPEMENT );
        return;
    }

    short nEs;
    BYTE nProp;
    switch( *pData ){
    case 1:  nEs = DFLT_ESC_AUTO_SUPER; nProp = DFLT_ESC_PROP; break;
    case 2:  nEs = DFLT_ESC_AUTO_SUB; nProp = DFLT_ESC_PROP; break;
    default: nEs = 0; nProp = 100; break;
    }
    NewAttr( SvxEscapementItem( nEs, nProp ) );
}

void SwWW8ImplReader::Read_SubSuperProp( USHORT, BYTE* pData, short nLen )
{
    if( nLen < 0 ){
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_ESCAPEMENT );
        return;
    }

    short nPos = SVBT16ToShort( pData );    // Font-Position in HalfPoints
    INT32 nPos2 = nPos * ( 10 * 100 );      // HalfPoints in 100 * tw
    SvxFontHeightItem* pF
        = (SvxFontHeightItem*)GetFmtAttr( RES_CHRATR_FONTSIZE );
    nPos2 /= (INT32)pF->GetHeight();        // ... nun in % ( gerundet )
    if( nPos2 > 100 )                       // zur Sicherheit
        nPos2 = 100;
    if( nPos2 < -100 )
        nPos2 = -100;
    SvxEscapementItem aEs( (short)nPos2, 100 );
    NewAttr( aEs );
}

void SwWW8ImplReader::Read_Underline( USHORT, BYTE* pData, short nLen )
{
    FontUnderline eUnderline = UNDERLINE_NONE;
    BOOL bWordLine = FALSE;
    if( pData )
    {
        // Parameter:  0 = none,    1 = single,  2 = by Word,
                    // 3 = double,  4 = dotted,  5 = hidden
                    // 6 = thick,   7 = dash,    8 = dot(not used)
                    // 9 = dotdash 10 = dotdotdash 11 = wave


        // pruefe auf Sonderfall "fett+unterstrichen"
        BOOL bAlsoBold = /*( 6 == b )*/FALSE;
        // erst mal ggfs. *bold* einschalten!
        if( bAlsoBold )
        {
            BYTE nOn = 1;
            Read_BoldUsw( 0x0835, &nOn, nLen );
            eUnderline = UNDERLINE_SINGLE;
        }
        else
        {
            switch( *pData )
            {
            case 2: bWordLine = TRUE;       // no break;
            case 1: eUnderline = (FontUnderline)UNDERLINE_SINGLE;       break;
            case 3: eUnderline = (FontUnderline)UNDERLINE_DOUBLE;       break;
            case 4: eUnderline = (FontUnderline)UNDERLINE_DOTTED;       break;
            case 7: eUnderline = (FontUnderline)UNDERLINE_DASH;         break;
            case 9: eUnderline = (FontUnderline)UNDERLINE_DASHDOT;      break;
            case 10:eUnderline = (FontUnderline)UNDERLINE_DASHDOTDOT;   break;
            case 6: eUnderline = (FontUnderline)UNDERLINE_BOLD;         break;
            case 11:eUnderline = (FontUnderline)UNDERLINE_WAVE;         break;
            case 20:eUnderline = (FontUnderline)UNDERLINE_BOLDDOTTED;   break;
            case 23:eUnderline = (FontUnderline)UNDERLINE_BOLDDASH;     break;
            case 39:eUnderline = (FontUnderline)UNDERLINE_LONGDASH;     break;
            case 55:eUnderline = (FontUnderline)UNDERLINE_BOLDLONGDASH; break;
            case 25:eUnderline = (FontUnderline)UNDERLINE_BOLDDASHDOT;  break;
            case 26:eUnderline = (FontUnderline)UNDERLINE_BOLDDASHDOTDOT;break;
            case 27:eUnderline = (FontUnderline)UNDERLINE_BOLDWAVE;     break;
            case 43:eUnderline = (FontUnderline)UNDERLINE_DOUBLEWAVE;   break;
            }
        }
    }

    // dann Stack ggfs. verwursteln und exit!
    if( nLen < 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_UNDERLINE );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_WORDLINEMODE );
    }
    else
    {
        NewAttr( SvxUnderlineItem( eUnderline ));
        if( bWordLine )
            NewAttr( SvxWordLineModeItem( TRUE ));
    }
}

/*
//The last three vary, measurements, rotation ? ?
NoBracket   78 CA 06 -  02 00 00 02 34 52
()          78 CA 06 -  02 01 00 02 34 52
[]          78 CA 06 -  02 02 00 02 34 52
<>          78 CA 06 -  02 03 00 02 34 52
{}          78 CA 06 -  02 04 00 02 34 52
*/
void SwWW8ImplReader::Read_DoubleLine_Rotate( USHORT, BYTE* pData, short nLen )
{
    if( nLen < 0 ) // close the tag
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_TWO_LINES );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_ROTATE );
    }
    else if( pData && 6 == nLen )
    {
        switch( *pData )
        {
        case 2:                     // double line
            {
                sal_Unicode cStt = 0, cEnd = 0;
                switch( SVBT16ToShort( pData+1 ) )
                {
                case 1: cStt = '(', cEnd = ')'; break;
                case 2: cStt = '[', cEnd = ']'; break;
                case 3: cStt = '<', cEnd = '>'; break;
                case 4: cStt = '{', cEnd = '}'; break;
                }
                NewAttr( SvxTwoLinesItem( sal_True, cStt, cEnd ));
            }
            break;

        case 1:                         // rotated characters
            {
                BOOL bFitToLine = 0 != *(pData+1);
                NewAttr( SvxCharRotateItem( 900, bFitToLine ));
            }
            break;
        }
    }
}

void SwWW8ImplReader::Read_TxtColor( USHORT, BYTE* pData, short nLen )
{
    if( nLen < 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );
        bTxtCol = FALSE;

        if( bCharShdTxtCol || bShdTxtCol )
            // dann muss die wieder eingeschaltet werden!!
            NewAttr( SvxColorItem( Color( COL_WHITE ) ) );  // -> weisse Schrift
    }
    else
    {
        BYTE b = *pData;            // Parameter: 0 = Auto, 1..16 Farben

        if( b > 16 )                // unbekannt -> Black
            b = 0;                  // Auto -> Black

        NewAttr( SvxColorItem( Color( eSwWW8ColA[b] ) ) );
        bTxtCol = TRUE;                         // SHD darf nicht Farbe einschalten
    }
}

BOOL SwWW8ImplReader::GetFontParams( USHORT nFCode, FontFamily& reFamily,
        String& rName, FontPitch& rePitch, CharSet& reCharSet )
{
    // Die Defines, aus denen diese Tabellen erzeugt werden, stehen in windows.h
    static FontPitch __READONLY_DATA ePitchA[]
    = { PITCH_DONTKNOW, PITCH_FIXED, PITCH_VARIABLE, PITCH_DONTKNOW };

    static FontFamily __READONLY_DATA eFamilyA[]
    = { FAMILY_DONTKNOW, FAMILY_ROMAN, FAMILY_SWISS, FAMILY_MODERN,
        FAMILY_SCRIPT, FAMILY_DECORATIVE };


    const WW8_FFN* pF = pFonts->GetFont( nFCode );  // Info dazu
    if( !pF )                                   // FontNummer unbekannt ?
        return FALSE;                           // dann ignorieren

    rName = String( pF->sFontname );

#if defined( OS2 )
            if (   rName.EqualsIgnoreCaseAscii( "Helv"  )
                || rName.EqualsIgnoreCaseAscii( "Arial" ) )
                rName.AssignAscii( "Helvetica" );
            else if (    rName.EqualsIgnoreCaseAscii( "Tms Rmn"         )
                      || rName.EqualsIgnoreCaseAscii( "Times New Roman" ) )
                rName.AssignAscii( "Times New Roman" );
            else if ( rName.EqualsIgnoreCaseAscii( "Courier New" ) )
                rName.AssignAscii( "Courier" );
            else if ( rName.EqualsIgnoreCaseAscii( "Symbol" ) )
                rName.AssignAscii( "Symbol Set" );
#endif // OS2
#if defined( MAC )
            if (   rName.EqualsIgnoreCaseAscii( "Helv"  )
                || rName.EqualsIgnoreCaseAscii( "Arial" ))
                rName.AssignAscii( "Helvetica" );
            else if (  rName.EqualsIgnoreCaseAscii( "Tms Rmn" )
                    || rName.EqualsIgnoreCaseAscii( "Times New Roman" ) )
                rName.AssignAscii( "Times" );
            else if ( rName.EqualsIgnoreCaseAscii( "Courier New"  ) )
                rName.AssignAscii( "Courier" );
#endif // MAC

    // pF->prg : Pitch
    rePitch = ePitchA[pF->prg];

    // pF->chs: Charset
    if( 77 == pF->chs )             // Mac-Font im Mac-Charset oder
        reCharSet = eTextCharSet;   // auf ANSI-Charset uebersetzt
    else
        reCharSet = rtl_getTextEncodingFromWindowsCharset( pF->chs );

    // pF->ff : Family
    BYTE b = pF->ff;

    // make sure Font Family Code is set correctly
    // at least for the most important fonts
    // ( might be set wrong when Doc was not created by
    //   Winword but by third party program like Applixware... )
        /*
        0: FAMILY_DONTKNOW
        1: FAMILY_ROMAN
        2: FAMILY_SWISS
        3: FAMILY_MODERN
        4: FAMILY_SCRIPT
        5: FAMILY_DECORATIVE
    */
#define FONTNAMETAB_SZ    14
#define MAX_FONTNAME_ROMAN 6
    static sal_Char __READONLY_DATA
        // first comes ROMAN
        sFontName0[] = "\x07""Tms Rmn",
        sFontName1[] = "\x07""Timmons",
        sFontName2[] = "\x08""CG Times",
        sFontName3[] = "\x08""MS Serif",
        sFontName4[] = "\x08""Garamond",
        sFontName5[] = "\x11""Times Roman",
        sFontName6[] = "\x15""Times New Roman",
        // from here SWISS --> see above: #define MAX_FONTNAME_ROMAN 6
        sFontName7[] = "\x04""Helv",
        sFontName8[] = "\x05""Arial",
        sFontName9[] = "\x07""Univers",
        sFontName10[]= "\x11""LinePrinter",
        sFontName11[]= "\x11""Lucida Sans",
        sFontName12[]= "\x11""Small Fonts",
        sFontName13[]= "\x13""MS Sans Serif";
    static const sal_Char* __READONLY_DATA aFontNameTab[ FONTNAMETAB_SZ ] =
    {
        sFontName0,  sFontName1,  sFontName2,  sFontName3,
        sFontName4,  sFontName5,  sFontName6,  sFontName7,
        sFontName8,  sFontName9,  sFontName10, sFontName11,
        sFontName12, sFontName13
    };

    for( USHORT n = 0;  n < FONTNAMETAB_SZ; n++ )
    {
        const sal_Char* pCmp = aFontNameTab[ n ];
        int nLen = *pCmp++;
        if( rName.EqualsIgnoreCaseAscii(pCmp, 0, nLen) )
        {
            b = n <= MAX_FONTNAME_ROMAN ? 1 : 2;
            break;
        }
    }
    if( b < sizeof( eFamilyA ) )
        reFamily = eFamilyA[b];
    else
        reFamily = FAMILY_DONTKNOW;

    return TRUE;
}

BOOL SwWW8ImplReader::SetNewFontAttr( USHORT nFCode, BOOL bSetEnums,
                                        USHORT nWhich )
{
    FontFamily eFamily;
    String aName;
    FontPitch ePitch;
    CharSet eSrcCharSet;

    if( !GetFontParams( nFCode, eFamily, aName, ePitch, eSrcCharSet ) )
        return FALSE;

    CharSet eDstCharSet = eSrcCharSet;

    SvxFontItem aFont( eFamily, aName, aEmptyStr, ePitch, eDstCharSet, nWhich);

    if( bSetEnums )
    {
        if( pAktColl )
        {                                   // StyleDef
            pCollA[nAktColl].eFontSrcCharSet = eSrcCharSet;
        }
        else
        {
            eFontSrcCharSet = eSrcCharSet;
        }
    }

    NewAttr( aFont );                       // ...und 'reinsetzen

    return TRUE;
}

void SwWW8ImplReader::ResetCharSetVars()
{
    if( nCharFmt >= 0 )
    {
        eFontSrcCharSet = pCollA[nCharFmt].eFontSrcCharSet; // aus C-Style
    }
    else
    {
        eFontSrcCharSet = RTL_TEXTENCODING_DONTKNOW;
    }
    if( eFontSrcCharSet == RTL_TEXTENCODING_DONTKNOW )
        eFontSrcCharSet = pCollA[nAktColl].eFontSrcCharSet; // aus P-Style
}

/*
    Font ein oder ausschalten:
*/
void SwWW8ImplReader::Read_FontCode( USHORT nId, BYTE* pData, short nLen )
{
    if( !bSymbol && !bIgnoreText )  // falls bSymbol, gilt der am Symbol
    {                               // (siehe sprmCSymbol) gesetzte Font !
        switch( nId )
        {
        case 0x93:
        case 0x4a51:
        case 0x4a4f:    nId = RES_CHRATR_FONT;      break;
        case 0x4a50:    nId = RES_CHRATR_CJK_FONT;  break;
//          nId = RES_CHRATR_CTL_FONT;  break;
        default:
            return ;
        }

        if( nLen < 0 )
        {                   // Ende des Attributes
            pCtrlStck->SetAttr( *pPaM->GetPoint(), nId );
            ResetCharSetVars();
        }
        else
        {
            USHORT nFCode = SVBT16ToShort( pData );     // Font-Nummer
            if( SetNewFontAttr( nFCode, TRUE, nId )     // Lies Inhalt
                && pAktColl && pStyles )                // Style-Def ?
            {
                // merken zur Simulation Default-Font
                if( RES_CHRATR_CJK_FONT == nId )
                    pStyles->bCJKFontChanged = TRUE;
                else
                    pStyles->bFontChanged = TRUE;
            }
        }
    }
}




void SwWW8ImplReader::Read_FontSize( USHORT nId, BYTE* pData, short nLen )
{
    switch( nId )
    {
    case 99:
    case 0x4a43:    nId = RES_CHRATR_FONTSIZE;      break;
//  case 0x4a61:    nId = RES_CHRATR_CTL_FONTSIZE;  break;
    default:
        return ;
    }

    if( nLen < 0 )          // Ende des Attributes
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), nId  );
        if( RES_CHRATR_FONTSIZE == nId )  // reset additional the CJK size
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_CJK_FONTSIZE );
    }
    else
    {
        USHORT nFSize = SVBT16ToShort( pData ) * 10;         // Font-Groesse in halben Point
                                                //  10 = 1440 / ( 72 * 2 )

        SvxFontHeightItem aSz( nFSize, 100, nId );
        NewAttr( aSz );
        if( RES_CHRATR_FONTSIZE == nId )  // set additional the CJK size
        {
            aSz.SetWhich( RES_CHRATR_CJK_FONTSIZE );
            NewAttr( aSz );
        }
        if( pAktColl && pStyles )           // Style-Def ?
            pStyles->bFSizeChanged = TRUE;  // merken zur Simulation Default-FontSize
    }
}



void SwWW8ImplReader::Read_CharSet( USHORT nId, BYTE* pData, short nLen )
{
    if( nLen < 0 ){                 // Ende des Attributes
        eHardCharSet = RTL_TEXTENCODING_DONTKNOW;
        return;
    }
    BYTE nfChsDiff = SVBT8ToByte( pData );

    if( nfChsDiff )
        eHardCharSet = rtl_getTextEncodingFromWindowsCharset( *(pData + 1) );
    else
        eHardCharSet = RTL_TEXTENCODING_DONTKNOW;
}

void SwWW8ImplReader::Read_Language( USHORT nId, BYTE* pData, short nLen )
{
    switch( nId )
    {
    case 97:
    case 0x486d:    nId = RES_CHRATR_LANGUAGE;      break;
    case 0x486e:    nId = RES_CHRATR_CJK_LANGUAGE;  break;
    default:
        return ;
    }

    if( nLen < 0 )                  // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), nId );
    else
    {
        USHORT nLang = SVBT16ToShort( pData );  // Language-Id
        NewAttr( SvxLanguageItem( (const LanguageType)nLang, nId ));
    }
}



/*
    Einschalten des Zeichen-Styles:
*/
void SwWW8ImplReader::Read_CColl( USHORT, BYTE* pData, short nLen )
{
    if( nLen < 0 ){                 // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_TXTATR_CHARFMT );
        nCharFmt = -1;
        return;
    }
    USHORT nId = SVBT16ToShort( pData );    // Style-Id (NICHT Sprm-Id!)

    if( nId >= nColls || !pCollA[nId].pFmt  // ungueltige Id ?
        || pCollA[nId].bColl )              // oder Para-Style ?
        return;                             // dann ignorieren

    NewAttr( SwFmtCharFmt( (SwCharFmt*)pCollA[nId].pFmt ) );
    nCharFmt = (short) nId;
}


/*
    enger oder weiter als normal:
*/
void SwWW8ImplReader::Read_Kern( USHORT, BYTE* pData, short nLen )
{
    if( nLen < 0 ){                 // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_KERNING );
        return;
    }
    INT16 nKern = SVBT16ToShort( pData );    // Kerning in Twips
    NewAttr( SvxKerningItem( nKern ) );
}

void SwWW8ImplReader::Read_FontKern( USHORT, BYTE* pData, short nLen )
{
    if( nLen < 0 )
                        // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_AUTOKERN );
    else
        NewAttr( SvxAutoKernItem( TRUE ) );
}

void SwWW8ImplReader::Read_CharShadow(  USHORT, BYTE* pData, short nLen )
{
    if( nLen <= 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_BACKGROUND );
        if( bCharShdTxtCol )
        {
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );  // Zeichenfarbe auch
            bCharShdTxtCol = FALSE;
        }
    }
    else
    {
        WW8_SHD aSHD;
        aSHD.SetWWValue( *(SVBT16*)pData );
        SwWW8Shade aSh( bVer67, aSHD );

        NewAttr( SvxBrushItem( aSh.aColor, RES_CHRATR_BACKGROUND ));

        // weisse Schrift und nicht ueberattributiert
        if( aSh.bWhiteText && !bTxtCol && !bShdTxtCol )
        {
            NewAttr( SvxColorItem( Color( COL_WHITE ) ) );  // -> weisse Schrift
            bCharShdTxtCol = TRUE;
        }
    }
}

void SwWW8ImplReader::Read_CharHighlight( USHORT, BYTE* pData, short nLen )
{
    if( nLen <= 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_BACKGROUND );
        if( bCharShdTxtCol )
        {
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );  // Zeichenfarbe auch
            bCharShdTxtCol = FALSE;
        }
    }
    else
    {
        BYTE b = *pData;            // Parameter: 0 = Auto, 1..16 Farben

        if( b > 16 )                // unbekannt -> Black
            b = 0;                  // Auto -> Black

        Color aCol( eSwWW8ColA[b] );
        NewAttr( SvxBrushItem( aCol , RES_CHRATR_BACKGROUND ));

        // weisse Schrift und nicht ueberattributiert
        if( COL_BLACK == aCol.GetColor() && !bTxtCol && !bShdTxtCol )
        {
            NewAttr( SvxColorItem( Color( COL_WHITE ) ) );  // -> weisse Schrift
            bCharShdTxtCol = TRUE;
        }
    }
}


/***************************************************************************
#  Absatz - Attribute
#**************************************************************************/

void SwWW8ImplReader::Read_NoLineNumb( USHORT nId, BYTE* pData, short nLen )
{
    if( nLen < 0 )  // Ende des Attributes
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_LINENUMBER );
        return;
    }
    SwFmtLineNumber aLN;
    aLN.SetCountLines( pData && (0 == *pData) );
    NewAttr( aLN );
}

void SwWW8ImplReader::Read_LR( USHORT nId, BYTE* pData, short nLen ) // Sprm 16, 17
{
    if( nIniFlags & WW8FL_NO_LRUL )
        return;

    if( nLen < 0 )  // Ende des Attributes
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_LR_SPACE );
        return;
    }

    short nPara = SVBT16ToShort( pData );

    SvxLRSpaceItem aLR;
    const SfxPoolItem* pLR = GetFmtAttr( RES_LR_SPACE );
    if( pLR )
        aLR = *(const SvxLRSpaceItem*)pLR;

    switch( nId )
    {
    //sprmPDxaLeft
    case     17:
    case 0x840F:
        if( !aLR.GetTxtFirstLineOfst() )
            aLR.SetTxtFirstLineOfst( 1 );

        aLR.SetTxtLeft( nPara );
        if( pAktColl )
        {
            pCollA[nAktColl].nLeftParaMgn      = nPara; // fuer Tabs merken
            pCollA[nAktColl].nTxtFirstLineOfst = aLR.GetTxtFirstLineOfst();
        }
        else
        {
            nLeftParaMgn      = nPara; // fuer Tabs merken
            nTxtFirstLineOfst = aLR.GetTxtFirstLineOfst();
        }

        // adjust tabs that were set at this paragraph/style resp. before
        // we encountered the LR Space Attribute. Tabstops in winword
        // are relative to page, tabstops in OOo are relative to para
        // edge, so adjust them, and remove negative ones.
        SvxTabStopItem* pTStop;
        pTStop = (SvxTabStopItem*)GetFmtAttr( RES_PARATR_TABSTOP );
        if( pTStop )
        {
            for( USHORT nCnt = 0; nCnt < pTStop->Count(); ++nCnt )
            {
                SvxTabStop& rTab = (SvxTabStop&)((*pTStop)[ nCnt ]);
                if(SVX_TAB_ADJUST_DEFAULT != rTab.GetAdjustment())
                {
                    if (rTab.GetTabPos() >= nLeftParaMgn )
                        rTab.GetTabPos() -= nLeftParaMgn;
                    else
                    {
                        pTStop->Remove( nCnt );
                        --nCnt;
                    }
                }
            }
            if (pTStop->Count())
                NewAttr( *pTStop );
        }
        break;
    //sprmPDxaLeft1
    case     19:
    case 0x8411:
        aLR.SetTxtFirstLineOfst( nPara );
        if( pAktColl )
        {
            pCollA[nAktColl].nTxtFirstLineOfst = nPara; // fuer Tabs merken
        }
        else
        {
            nTxtFirstLineOfst = nPara; // fuer Tabs merken
        }
        break;
    //sprmPDxaRight
    case     16:
    case 0x840E:
        aLR.SetRight( nPara );
        break;
    default: return;
    }
    NewAttr( aLR );
}

void SwWW8ImplReader::Read_LineSpace( USHORT, BYTE* pData, short nLen ) // Sprm 20
{
// Kommentear siehe Read_UL()
    if( bStyNormal && ( bWWBugNormal || ( nIniFlags & WW8FL_NO_STD_STY_DYA ) ) )
        return;

    if( nLen < 0 ){
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_LINESPACING );
        if( !( nIniFlags & WW8FL_NO_IMPLPASP ) )
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_UL_SPACE );
        return;
    }
    short nSpace = SVBT16ToShort( pData );
    short nMulti = SVBT16ToShort( pData + 2 );

    SvxLineSpace eLnSpc;
    if( 0 > nSpace )
    {
        nSpace = -nSpace;
        eLnSpc = SVX_LINE_SPACE_FIX;
    }
    else
        eLnSpc = SVX_LINE_SPACE_MIN;

// WW hat einen impliziten zusaetzlichen Absatzabstand abhaengig vom
// Zeilenabstand. Er betraegt bei "genau", 0.8*Zeilenabstand "vor" und
// 0.2*Zeilenabstand "nach".
// Bei "Mindestens" sind es 1*Zeilenabstand "vor" und 0*Zeilenabstand "nach".
// Bei Mehrfach sind es 0 "vor" und min( 0cm, FontSize*(nFach-1) ) "nach".
//
// SW hat auch einen impliziten Zeilenabstand. er betraegt bei "mindestens"
// 1*Zeilenabstand "vor" und 0 "nach"
// bei proportional betraegt er min( 0cm, FontSize*(nFach-1) ) sowohl "vor"
// wie auch "nach"

    USHORT nWwPre = 0;
    USHORT nWwPost = 0;
    USHORT nSwPre = 0;
    USHORT nSwPost = 0;
    USHORT nSpaceTw = 0;

    SvxLineSpacingItem aLSpc;

    if( 1 == nMulti )               // MultilineSpace ( proportional )
    {
        long n = nSpace * 10 / 24;  // WW: 240 = 100%, SW: 100 = 100%

//JP 03.12.98: nach Absprache mit AMA ist die Begrenzung unsinnig
        if( n>200 ) n = 200;        // SW_UI-Maximum
        aLSpc.SetPropLineSpace( (const BYTE)n );
        SvxFontHeightItem* pH = (SvxFontHeightItem*)
            GetFmtAttr( RES_CHRATR_FONTSIZE );
        nSpaceTw = (USHORT)( n * pH->GetHeight() / 100 );

        if( n > 100 )
            nWwPost = nSwPre = nSwPost = (USHORT)( ( n - 100 )
                                                    * pH->GetHeight() / 100 );
    }
    else                            // Fixed / Minimum
    {
        // bei negativen Space ist der Abstand exakt, sonst minimum
        nSpaceTw = (USHORT)nSpace;
        aLSpc.SetLineHeight( nSpaceTw );
        aLSpc.GetLineSpaceRule() = eLnSpc;
        nSwPre = nSpace;

        if( SVX_LINE_SPACE_FIX == eLnSpc )                  // Genau
        {
            nWwPre = (USHORT)( 8L * nSpace / 10 );
            nWwPost = (USHORT)( 2L * nSpace / 10 );
            nSwPre = nSpace;
        }
        else                                                // Minimum
        {
            nWwPre = (USHORT)( 129L * nSpace / 100 - 95 );// erst bei groesseren
                                                          // Zeilenabstaenden
        }
    }
    NewAttr( aLSpc );
    if( pSFlyPara )
        pSFlyPara->nLineSpace = nSpaceTw;   // LineSpace fuer Graf-Apos

    if( ( nWwPre > nSwPre || nWwPost > nSwPost )
            && !( nIniFlags & WW8FL_NO_IMPLPASP ) )
    {
        SvxULSpaceItem aUL( *(SvxULSpaceItem*)GetFmtAttr( RES_UL_SPACE ));
        short nDU = aUL.GetUpper() + nWwPre - nSwPre;
        short nDL = aUL.GetLower() + nWwPost - nSwPost;
        if( nDU > 0 )
            aUL.SetUpper( nDU );
        else
            nDL += nDU;

        if( nDL > 0 )
            aUL.SetLower( nDL );
        NewAttr( aUL );
    }
}

void SwWW8ImplReader::Read_UL( USHORT nId, BYTE* pData, short nLen ) // Sprm 21, 22
{
    if( nIniFlags & WW8FL_NO_LRUL )
        return;

// Nun eine Umpopelung eines WW-Fehlers: Bei nProduct == 0c03d wird
// faelschlicherweise ein DyaAfter 240 ( delta y abstand after, amn.d.Åb.)
// im Style "Normal" eingefuegt, der
// gar nicht da ist. Ueber das IniFlag WW8FL_NO_STY_DYA laesst sich dieses
// Verhalten auch fuer andere WW-Versionen erzwingen
//  ASSERT( !bStyNormal || bWWBugNormal, "+Dieses Doc deutet evtl. auf einen \
//Fehler in der benutzten WW-Version hin. Wenn sich die Styles <Standard> bzw. \
//<Normal> zwischen WW und SW im Absatz- oder Zeilenabstand unterscheiden, \
//dann bitte dieses Doc SH zukommen lassen." );

//  if( bStyNormal && ( bWWBugNormal || ( nIniFlags & WW8FL_NO_STD_STY_DYA ) ) )
//      return;
// bWWBugNormal ist kein hinreichendes Kriterium dafuer, dass der
// angegebene Abstand falsch ist

    if( bStyNormal && ( nIniFlags & WW8FL_NO_STD_STY_DYA ) )
        return;

    if( nLen < 0 ){                 // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_UL_SPACE );
        return;
    }
    short nPara = SVBT16ToShort( pData );
    if( nPara < 0 )
        nPara = -nPara;

    SvxULSpaceItem aUL( *(SvxULSpaceItem*)GetFmtAttr( RES_UL_SPACE ));

    switch( nId ){                // keine Versuche
        //sprmPDyaBefore
        case     21:
        case 0xA413: aUL.SetUpper( nPara ); break;
        //sprmPDyaAfter
        case     22:
        case 0xA414: aUL.SetLower( nPara ); break;
        default: return;
    };
#if 0
    // nIniFlags stehen in c:\winnt40\soffice.ini[user]
    // siehe wwpar.hxx
    // und SwWW8ImplReader::LoadDoc( SwPaM& rPaM )
    //
    if( nIniFlags & WW8FL_NO_IMPLPASP ){
        switch( nId ){                // keine Versuche
            case 21:
            case 0xA413: aUL.SetUpper( nPara ); break;
            case 22:
            case 0xA414: aUL.SetLower( nPara ); break;
            default: return;
        };
    }else{
    // auf alte Werte addieren wg. implizitem Absatzabstand
    // ( siehe Read_LineSpace() )
        switch( nId ){
            case 21:
            case 0xA413: aUL.SetUpper( aUL.GetUpper() + nPara ); break;
            case 22:
            case 0xA414: aUL.SetLower( aUL.GetLower() + nPara ); break;
            default: return;
        };
    }
#endif
    NewAttr( aUL );
}

void SwWW8ImplReader::Read_Justify( USHORT, BYTE* pData, short nLen )
{
    static SvxAdjust aAdjArr[] = { SVX_ADJUST_LEFT,  SVX_ADJUST_CENTER,
                                   SVX_ADJUST_RIGHT, SVX_ADJUST_BLOCK };

    if( nLen < 0 ){
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_ADJUST );
        return;
    }
    BYTE b = *pData;

    NewAttr( SvxAdjustItem( aAdjArr[b&0x3] ) ); // "&0x3 gegen Tabellenueberlauf
}                                               // bei Stuss-Werten


void SwWW8ImplReader::Read_BoolItem( USHORT nId, BYTE* pData, short nLen )
{
    switch( nId )
    {
    case 0x2433:    nId = RES_PARATR_FORBIDDEN_RULES;       break;
    case 0x2435:    nId = RES_PARATR_HANGINGPUNCTUATION;    break;
    case 0x2437:    nId = RES_PARATR_SCRIPTSPACE;           break;
    default:
        ASSERT( !this, "wrong Id" );
        return ;
    }

    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), nId );
    else
    {
        SfxBoolItem* pI = (SfxBoolItem*)GetDfltAttr( nId )->Clone();
        pI->SetValue( 0 != *pData );
        NewAttr( *pI );
        delete pI;
    }
}

void SwWW8ImplReader::Read_Emphasis( USHORT, BYTE* pData, short nLen )
{
    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_EMPHASIS_MARK );
    else
    {
        sal_uInt16 nVal;
        switch( *pData )
        {
        case 0:     nVal = EMPHASISMARK_NONE;           break;
        case 2:     nVal = EMPHASISMARK_SIDE_DOTS;      break;
        case 3:     nVal = EMPHASISMARK_CIRCLE_ABOVE;   break;
        case 4:     nVal = EMPHASISMARK_DOTS_BELOW;     break;
//      case 1:
        default:    nVal = EMPHASISMARK_DOTS_ABOVE;     break;
        }

        NewAttr( SvxEmphasisMarkItem( nVal ) );
    }
}

void SwWW8ImplReader::Read_ScaleWidth( USHORT, BYTE* pData, short nLen )
{
    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_SCALEW );
    else
    {
        sal_uInt16 nVal = SVBT16ToShort( pData );
        NewAttr( SvxCharScaleWidthItem( nVal ) );
    }
}

void SwWW8ImplReader::Read_Relief( USHORT nId, BYTE* pData, short nLen )
{
    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_RELIEF );
    else
    {
        if( *pData )
        {
// JP 16.03.2001 - not so eays because this is also a toggle attribute!
//  2 x emboss on -> no emboss !!!
// the actual value must be searched over the stack / template

            const SvxCharReliefItem* pOld = (SvxCharReliefItem*)
                                            GetFmtAttr( RES_CHRATR_RELIEF );
            FontRelief nNewValue = 0x854 == nId ? RELIEF_ENGRAVED
                                         : ( 0x858 == nId ? RELIEF_EMBOSSED
                                                            : RELIEF_NONE );
            if( pOld->GetValue() == nNewValue )
            {
                if( RELIEF_NONE != nNewValue )
                    nNewValue = RELIEF_NONE;
            }
            NewAttr( SvxCharReliefItem( nNewValue ));
        }
    }
}


SwWW8Shade::SwWW8Shade( BOOL bVer67, const WW8_SHD& rSHD )
{
static ULONG __READONLY_DATA eMSGrayScale[] = {
        // Nul-Brush
           0,   // 0
        // Solid-Brush
        1000,   // 1
        // promillemaessig abgestufte Schattierungen
          50,   // 2
         100,   // 3
         200,   // 4
         250,   // 5
         300,   // 6
         400,   // 7
         500,   // 8
         600,   // 9
         700,   // 10
         750,   // 11
         800,   // 12
         900,   // 13
         333, // 14 Dark Horizontal
         333, // 15 Dark Vertical
         333, // 16 Dark Forward Diagonal
         333, // 17 Dark Backward Diagonal
         333, // 18 Dark Cross
         333, // 19 Dark Diagonal Cross
         333, // 20 Horizontal
         333, // 21 Vertical
         333, // 22 Forward Diagonal
         333, // 23 Backward Diagonal
         333, // 24 Cross
         333, // 25 Diagonal Cross
         // neun Nummern ohne Bedeutung in Ver8
         500, // 26
         500, // 27
         500, // 28
         500, // 29
         500, // 30
         500, // 31
         500, // 32
         500, // 33
         500, // 34
         // und weiter gehts mit tollen Schattierungen ;-)
          25,   // 35
          75,   // 36
         125,   // 37
         150,   // 38
         175,   // 39
         225,   // 40
         275,   // 41
         325,   // 42
         350,   // 43
         375,   // 44
         425,   // 45
         450,   // 46
         475,   // 47
         525,   // 48
         550,   // 49
         575,   // 50
         625,   // 51
         650,   // 52
         675,   // 53
         725,   // 54
         775,   // 55
         825,   // 56
         850,   // 57
         875,   // 58
         925,   // 59
         950,   // 60
         975,   // 61
         // und zu guter Letzt:
         970};// 62
    ColorData nFore;
    ColorData nBack;
    ULONG     nWW8BrushStyle;
    short     b;

    b = rSHD.GetFore();
    if ( b >= 17 )
        b = 0;
    nFore = eSwWW8ColA[b];
                                        // Vordergrund: Auto = Schwarz
    b = rSHD.GetBack();
    if( b >=  17 )
        b = 0;
    if( b == 0 )
        b = 8;                          // Hintergrund: Auto = Weiss
    nBack = eSwWW8ColA[b];

    b = rSHD.GetStyle( bVer67 );

    if( b >= sizeof( eMSGrayScale ) / sizeof ( eMSGrayScale[ 0 ] ) )
        b = 0;

    nWW8BrushStyle = eMSGrayScale[b];

    switch( nWW8BrushStyle )
    {
        case 0: // Null-Brush
            aColor.SetColor( nBack );
            break;
        case 1000:
            aColor.SetColor( nFore );
            break;
        default:
            {
                Color aForeColor = Color( nFore );
                Color aBackColor = Color( nBack );
                ULONG   nRed        = aForeColor.GetRed()    *      nWW8BrushStyle;
                ULONG   nGreen  = aForeColor.GetGreen()  *      nWW8BrushStyle;
                ULONG   nBlue       = aForeColor.GetBlue()   *      nWW8BrushStyle;
                nRed   += (ULONG)(aBackColor.GetRed()  *(1000-nWW8BrushStyle));
                nGreen += (ULONG)(aBackColor.GetGreen()*(1000-nWW8BrushStyle));
                nBlue  += (ULONG)(aBackColor.GetBlue() *(1000-nWW8BrushStyle));

                aColor.SetColor( RGB_COLORDATA( nRed/1000, nGreen/1000, nBlue/1000 ) );
            }
            break;
    }
    // schwarzer Hintergrund -> weisse Schrift
    bWhiteText =    (nFore == COL_BLACK) && ( 800 <= nWW8BrushStyle )
                 || (nBack == COL_BLACK) && ( 200 >= nWW8BrushStyle );
}


void SwWW8ImplReader::Read_Shade( USHORT, BYTE* pData, short nLen )
{
    if( nLen <= 0 )
    {       // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_BACKGROUND );
        if( bShdTxtCol )
        {
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );  // Zeichenfarbe auch
            bShdTxtCol = FALSE;
        }
    }
    else
    {
        WW8_SHD aSHD;
        aSHD.SetWWValue( *(SVBT16*)pData );
        SwWW8Shade aSh( bVer67, aSHD );

        NewAttr( SvxBrushItem( aSh.aColor ) );

        // weisse Schrift und nicht ueberattributiert
        if( aSh.bWhiteText && !bTxtCol )
        {
            NewAttr( SvxColorItem( Color( COL_WHITE ) ) );  // -> weisse Schrift
            bShdTxtCol = TRUE;
        }
    }
}

void SwWW8ImplReader::Read_Border( USHORT nId, BYTE* pData, short nLen )
{
    if( nLen < 0 )
    {
        if( bHasBorder )
        {
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_BOX );
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_SHADOW );
            bHasBorder = FALSE;
        }
    }
    else if( !bHasBorder )
    {
                            // die Borders auf allen 4 Seiten werden gebuendelt.
                            // dieses vereinfacht die Verwaltung, d.h. die
                            // Box muss nicht 4 mal auf den CtrlStack und wieder
                            // runter
        bHasBorder = TRUE;

        WW8_BRC5 aBrcs; // Top, Left, Bottom, Right, Between
        BOOL bBorder;

        if( pAktColl )
            bBorder = ::lcl_ReadBorders( bVer67, aBrcs, 0, pStyles );
        else
            bBorder = ::lcl_ReadBorders( bVer67, aBrcs, pPlcxMan->GetPapPLCF() );

        if( bBorder )                                   // Border
        {
            BOOL bIsB = IsBorder( aBrcs, TRUE );
            if( !bApo || !bIsB || ( pWFlyPara && !pWFlyPara->bBorderLines ))
            {
                // in Apo keine Umrandungen *ein*-schalten, da ich
                // sonst die Flyumrandungen doppelt bekomme
                // JP 04.12.98: aber nur wenn am Fly ein gesetzt ist, keine
                //              uebernehmen. Sonst wird gar keine gesetzt!
                //              Bug #59619#

                // auch wenn kein Rand gesetzt ist, muss das Attribut gesetzt
                // werden, sonst ist kein hartes Ausschalten von Style-Attrs
                // moeglich
                SvxBoxItem* pBox
                    = (SvxBoxItem*)GetFmtAttr( RES_BOX );
                SvxBoxItem aBox;
                if( pBox )
                    aBox = *pBox;
                SetBorder( aBox, aBrcs, TRUE );

                Rectangle aInnerDist;
                GetBorderDistance( aBrcs, aInnerDist );

                aBox.SetDistance( (USHORT)aInnerDist.Left(), BOX_LINE_LEFT );
                aBox.SetDistance( (USHORT)aInnerDist.Top(), BOX_LINE_TOP );
                aBox.SetDistance( (USHORT)aInnerDist.Right(), BOX_LINE_RIGHT );
                aBox.SetDistance( (USHORT)aInnerDist.Bottom(), BOX_LINE_BOTTOM );

                NewAttr( aBox );

                SvxShadowItem aS;
                if( SetShadow( aS, aBox, aBrcs ) )
                    NewAttr( aS );
            }
        }
    }
}

void SwWW8ImplReader::Read_Hyphenation( USHORT, BYTE* pData, short nLen )
{
    // set Hyphenation flag
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_HYPHENZONE );
    else
    {
        SvxHyphenZoneItem aAttr(
            *(SvxHyphenZoneItem*)GetFmtAttr( RES_PARATR_HYPHENZONE ) );

        aAttr.SetHyphen( 0 == *pData ); // sic !

        if( !*pData )
        {
            aAttr.GetMinLead()    = 2;
            aAttr.GetMinTrail()   = 2;
            aAttr.GetMaxHyphens() = 0;
        }

        NewAttr( aAttr );
    }
}

void SwWW8ImplReader::Read_WidowControl( USHORT, BYTE* pData, short nLen )
{
    if( nLen <= 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_WIDOWS );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_ORPHANS );
    }
    else
    {
        BYTE nL = ( *pData & 1 ) ? 2 : 0;

        NewAttr( SvxWidowsItem( nL ) );     // Aus -> nLines = 0
        NewAttr( SvxOrphansItem( nL ) );

        if( pAktColl && pStyles )           // Style-Def ?
            pStyles->bWidowsChanged = TRUE; // merken zur Simulation
                                            // Default-Widows
    }
}

void SwWW8ImplReader::Read_KeepLines( USHORT, BYTE* pData, short nLen )
{
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_SPLIT );
    else
        NewAttr( SvxFmtSplitItem( ( *pData & 1 ) == 0 ) );
}

void SwWW8ImplReader::Read_KeepParas( USHORT, BYTE* pData, short nLen )
{
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_KEEP );
    else
        NewAttr( SvxFmtKeepItem( ( *pData & 1 ) != 0 ) );
}

void SwWW8ImplReader::Read_BreakBefore( USHORT, BYTE* pData, short nLen )
{
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_BREAK );
    else
        NewAttr( SvxFmtBreakItem(
                ( *pData & 1 ) ? SVX_BREAK_PAGE_BEFORE : SVX_BREAK_NONE ) );
}

void SwWW8ImplReader::Read_ApoPPC( USHORT, BYTE* pData, short nLen )
{
    if( pAktColl ){                 // nur fuer Styledef, sonst anders geloest
        SwWW8StyInf& rSI = pCollA[nAktColl];
        WW8FlyPara* pFly = rSI.pWWFly;
        if( !pFly ){
            pFly = ( rSI.nBase >= nColls ) ?
                        new WW8FlyPara( bVer67 )                                                        // !based on
                    : new WW8FlyPara( bVer67, pCollA[rSI.nBase].pWWFly );   // based on
            pCollA[nAktColl].pWWFly = pFly;
            if( !pFly->Read( pData, pStyles ) )     // Lese Style-Apo-Parameter
                DELETEZ( pCollA[nAktColl].pWWFly );
        }
    }
}

/***************************************************************************
#  Seiten - Attribute werden nicht mehr als Attribute gehandhabt
#   ( ausser OLST )
#**************************************************************************/


/***************************************************************************
#       Arrays zum Lesen der erweiterten ( selbstdefinierten ) SPRMs
#**************************************************************************/
typedef long (SwWW8ImplReader:: *FNReadRecordExt)( WW8PLCFManResult*, BOOL );

static FNReadRecordExt aWwSprmTab2[] = {
/* 0 (256) */   &SwWW8ImplReader::Read_Ftn,     // FootNote
/* 1 (257) */   &SwWW8ImplReader::Read_Ftn,     // EndNote
/* 2 (258) */   &SwWW8ImplReader::Read_Field,   // Feld
/* 3 (259) */   &SwWW8ImplReader::Read_Book,    // Bookmark
/* 4 (260) */   &SwWW8ImplReader::Read_Piece,   // Piece-Anfang / Ende
/* 5 (261) */   &SwWW8ImplReader::Read_And      // Annotation
};

long SwWW8ImplReader::ImportExtSprm( WW8PLCFManResult* pRes, BOOL bStart )
{
    if( pRes->nSprmId < 280 )
    {
        BYTE nIdx = pRes->nSprmId - 256;
        if( nIdx < sizeof( aWwSprmTab2 ) / sizeof( *aWwSprmTab2 )
            && aWwSprmTab2[nIdx] )
            return (this->*aWwSprmTab2[nIdx])( pRes, bStart );
        else
            return 0;
    }
    else
        return 0;
}



/***************************************************************************
#       Arrays zum Lesen der SPRMs
#**************************************************************************/

// Funktion zum Einlesen von Sprms. Par1: SprmId
//typedef void (SwWW8ImplReader:: *FNReadRecord) ( BYTE, BYTE*, short );




typedef void (SwWW8ImplReader:: *FNReadRecord)( USHORT, BYTE*, short );

struct SprmReadInfo
{
    USHORT       nId;
    FNReadRecord pReadFnc;
};

SprmReadInfo& WW8GetSprmReadInfo( USHORT nId );



SprmReadInfo aSprmReadTab[] = {
   0, (FNReadRecord)0, // "??0" Default bzw. Error wird uebersprungen! ,
   2, &SwWW8ImplReader::Read_StyleCode, // "sprmPIstd",  // pap.istd (style code)

   3, (FNReadRecord)0, // "sprmPIstdPermute", // pap.istd   permutation
   4, (FNReadRecord)0, //  "sprmPIncLv1", // pap.istddifference
   5, &SwWW8ImplReader::Read_Justify, //  "sprmPJc", // pap.jc (justification)
   6, (FNReadRecord)0, //  "sprmPFSideBySide", // pap.fSideBySide
   7, &SwWW8ImplReader::Read_KeepLines, //  "sprmPFKeep", // pap.fKeep
   8, &SwWW8ImplReader::Read_KeepParas, //  "sprmPFKeepFollow ", // pap.fKeepFollow
   9, &SwWW8ImplReader::Read_BreakBefore, //  "sprmPPageBreakBefore", // pap.fPageBreakBefore

  10, (FNReadRecord)0, // "sprmPBrcl", //   pap.brcl
  11, (FNReadRecord)0, // "sprmPBrcp ", // pap.brcp
  12, &SwWW8ImplReader::Read_ANLevelDesc, // "sprmPAnld", //    pap.anld (ANLD structure)
  13, &SwWW8ImplReader::Read_ANLevelNo, //  "sprmPNLvlAnm", // pap.nLvlAnm nn
  14, &SwWW8ImplReader::Read_NoLineNumb, //  "sprmPFNoLineNumb", //ap.fNoLnn

//??
  15, &SwWW8ImplReader::Read_Tab, //   "?sprmPChgTabsPapx", // pap.itbdMac, ...
  16, &SwWW8ImplReader::Read_LR, //  "sprmPDxaRight", // pap.dxaRight
  17, &SwWW8ImplReader::Read_LR, //  "sprmPDxaLeft", // pap.dxaLeft
  18, (FNReadRecord)0, //  "sprmPNest", // pap.dxaLeft
  19, &SwWW8ImplReader::Read_LR, //  "sprmPDxaLeft1", // pap.dxaLeft1

  20, &SwWW8ImplReader::Read_LineSpace, // "sprmPDyaLine", // pap.lspd  an LSPD
  21, &SwWW8ImplReader::Read_UL, //  "sprmPDyaBefore", // pap.dyaBefore
  22, &SwWW8ImplReader::Read_UL, //  "sprmPDyaAfter", //    pap.dyaAfter

//??
  23, (FNReadRecord)0, // "?sprmPChgTabs", // pap.itbdMac, pap.rgdxaTab, ...
  24, (FNReadRecord)0, //  "sprmPFInTable", // pap.fInTable
  25, &SwWW8ImplReader::Read_TabRowEnd, //  "sprmPTtp", // pap.fTtp
  26, (FNReadRecord)0, //  "sprmPDxaAbs", // pap.dxaAbs
  27, (FNReadRecord)0, //  "sprmPDyaAbs", // pap.dyaAbs
  28, (FNReadRecord)0, // "sprmPDxaWidth", // pap.dxaWidth
  29, &SwWW8ImplReader::Read_ApoPPC, // "sprmPPc", // pap.pcHorz, pap.pcVert

  30, (FNReadRecord)0, // "sprmPBrcTop10", // pap.brcTop BRC10
  31, (FNReadRecord)0, // "sprmPBrcLeft10", // pap.brcLeft BRC10
  32, (FNReadRecord)0, // "sprmPBrcBottom10", // pap.brcBottom BRC10

  33, (FNReadRecord)0, // "sprmPBrcRight10", // pap.brcRight BRC10
  34, (FNReadRecord)0, // "sprmPBrcBetween10", // pap.brcBetween BRC10
  35, (FNReadRecord)0, // "sprmPBrcBar10", // pap.brcBar BRC10
  36, (FNReadRecord)0, // "sprmPFromText10", // pap.dxaFromText dxa
  37, (FNReadRecord)0, // "sprmPWr", // pap.wr wr
  38, &SwWW8ImplReader::Read_Border, // "sprmPBrcTop", // pap.brcTop BRC
  39, &SwWW8ImplReader::Read_Border, // "sprmPBrcLeft", // pap.brcLeft BRC

  40, &SwWW8ImplReader::Read_Border, // "sprmPBrcBottom", // pap.brcBottom BRC
  41, &SwWW8ImplReader::Read_Border, // "sprmPBrcRight", // pap.brcRight BRC
  42, &SwWW8ImplReader::Read_Border, // "sprmPBrcBetween", // pap.brcBetween BRC
  43, (FNReadRecord)0, // "sprmPBrcBar",//pap.brcBar BRC word
  44, &SwWW8ImplReader::Read_Hyphenation, // "sprmPFNoAutoHyph",//pap.fNoAutoHyph
  45, (FNReadRecord)0, // "sprmPWHeightAbs",//pap.wHeightAbs w
  46, (FNReadRecord)0, // "sprmPDcs",//pap.dcs DCS
  47, &SwWW8ImplReader::Read_Shade, // "sprmPShd",//pap.shd SHD
  48, (FNReadRecord)0, //  "sprmPDyaFromText",//pap.dyaFromText dya
  49, (FNReadRecord)0, //  "sprmPDxaFromText",//pap.dxaFromText dxa

  50, (FNReadRecord)0, //  "sprmPFLocked", // pap.fLocked 0 or 1 byte
  51, &SwWW8ImplReader::Read_WidowControl, //  "sprmPFWidowControl", // pap.fWidowControl 0 or 1 byte

//??
  52, (FNReadRecord)0, // "?sprmPRuler 52", //???
  53, (FNReadRecord)0, // "??53",
  54, (FNReadRecord)0, // "??54",
  55, (FNReadRecord)0, // "??55",
  56, (FNReadRecord)0, // "??56",
  57, (FNReadRecord)0, // "??57",
  58, (FNReadRecord)0, // "??58",
  59, (FNReadRecord)0, // "??59",

  60, (FNReadRecord)0, // "??60",
  61, (FNReadRecord)0, // "??61",
  62, (FNReadRecord)0, // "??62",
  63, (FNReadRecord)0, // "??63",
  64, (FNReadRecord)0, // "??64",
  65, &SwWW8ImplReader::Read_CFRMarkDel, //  "sprmCFStrikeRM", // chp.fRMarkDel 1 or 0 bit
  66, &SwWW8ImplReader::Read_CFRMark,    //  "sprmCFRMark", // chp.fRMark 1 or 0 bit
  67, &SwWW8ImplReader::Read_FldVanish, //  "sprmCFFldVanish", // chp.fFldVanish 1 or 0 bit
  68, &SwWW8ImplReader::Read_PicLoc, // "sprmCPicLocation", // chp.fcPic and chp.fSpec
  69, (FNReadRecord)0, // "sprmCIbstRMark", // chp.ibstRMark index into sttbRMark

  70, (FNReadRecord)0, // "sprmCDttmRMark", // chp.dttm DTTM long
  71, (FNReadRecord)0, //  "sprmCFData", // chp.fData 1 or 0 bit
  72, (FNReadRecord)0, // "sprmCRMReason", // chp.idslRMReason an index to a table
  73, &SwWW8ImplReader::Read_CharSet, // "sprmCChse", // chp.fChsDiff and chp.chse see below 3 bytes
  74, &SwWW8ImplReader::Read_Symbol, // "sprmCSymbol", // chp.fSpec, chp.chSym and chp.ftcSym
  75, &SwWW8ImplReader::Read_Obj, //  "sprmCFOle2", // chp.fOle2 1 or 0 bit
  76, (FNReadRecord)0, // "??76",
  77, (FNReadRecord)0, // "??77",
  78, (FNReadRecord)0, // "??78",
  79, (FNReadRecord)0, // "??79",

  80, &SwWW8ImplReader::Read_CColl, // "sprmCIstd", // chp.istd istd, see stylesheet definition short
  81, (FNReadRecord)0, // "sprmCIstdPermute", // chp.istd permutation vector (see below)
  82, (FNReadRecord)0, // "sprmCDefault", // whole CHP (see below) none variable length
  83, (FNReadRecord)0, // "sprmCPlain", // whole CHP (see below) none 0
  84, (FNReadRecord)0, // "??84",
  85, &SwWW8ImplReader::Read_BoldUsw, // "sprmCFBold", // chp.fBold 0,1, 128, or 129 (see below) byte
  86, &SwWW8ImplReader::Read_BoldUsw, // "sprmCFItalic", // chp.fItalic 0,1, 128, or 129 (see below) byte
  87, &SwWW8ImplReader::Read_BoldUsw, // "sprmCFStrike", // chp.fStrike 0,1, 128, or 129 (see below) byte
  88, &SwWW8ImplReader::Read_BoldUsw, // "sprmCFOutline", // chp.fOutline 0,1, 128, or 129 (see below) byte
  89, &SwWW8ImplReader::Read_BoldUsw, // "sprmCFShadow", // chp.fShadow 0,1, 128, or 129 (see below) byte

  90, &SwWW8ImplReader::Read_BoldUsw, // "sprmCFSmallCaps", // chp.fSmallCaps 0,1, 128, or 129 (see below) byte
  91, &SwWW8ImplReader::Read_BoldUsw, // "sprmCFCaps", // chp.fCaps 0,1, 128, or 129 (see below) byte

  92, &SwWW8ImplReader::Read_Invisible, // "sprmCFVanish", // chp.fVanish 0,1, 128, or 129 (see below) byte

  93, &SwWW8ImplReader::Read_FontCode, // "sprmCFtc", // chp.ftc ftc word
  94, &SwWW8ImplReader::Read_Underline, // "sprmCKul", // chp.kul kul byte
  95, (FNReadRecord)0, // "sprmCSizePos", // chp.hps, chp.hpsPos (see below) 3 bytes
  96, &SwWW8ImplReader::Read_Kern,   //  "sprmCDxaSpace", // chp.dxaSpace dxa word
  97, &SwWW8ImplReader::Read_Language, //  "sprmCLid", // chp.lid LID word
  98, &SwWW8ImplReader::Read_TxtColor, //  "sprmCIco", // chp.ico ico byte
  99, &SwWW8ImplReader::Read_FontSize, // "sprmCHps", // chp.hps hps word!


 100, (FNReadRecord)0, //  "sprmCHpsInc", // chp.hps (see below) byte
 101, &SwWW8ImplReader::Read_SubSuperProp, //  "sprmCHpsPos", // chp.hpsPos hps byte
 102, (FNReadRecord)0, //  "sprmCHpsPosAdj", // chp.hpsPos hps (see below) byte

//??
 103, &SwWW8ImplReader::Read_Majority, // "?sprmCMajority", // chp.fBold, chp.fItalic, chp.fSmallCaps, ...
 104, &SwWW8ImplReader::Read_SubSuper, //  "sprmCIss", // chp.iss iss byte
 105, (FNReadRecord)0, // "sprmCHpsNew50", // chp.hps hps variable width, length always recorded as 2
 106, (FNReadRecord)0, // "sprmCHpsInc1", // chp.hps complex (see below) variable width, length always recorded as 2
 107, &SwWW8ImplReader::Read_FontKern, // "sprmCHpsKern", // chp.hpsKern hps short
 108, &SwWW8ImplReader::Read_Majority, // "sprmCMajority50", // chp.fBold, chp.fItalic, chp.fSmallCaps, chp.fVanish, ...
 109, (FNReadRecord)0, // "sprmCHpsMul", // chp.hps percentage to grow hps short

 110, (FNReadRecord)0, // "sprmCCondHyhen", // chp.ysri ysri short
 111, (FNReadRecord)0, // "??111",
 112, (FNReadRecord)0, // "??112",
 113, (FNReadRecord)0, // "??113",
 114, (FNReadRecord)0, // "??114",
 115, (FNReadRecord)0, // "??115",
 116, (FNReadRecord)0, // "??116",
 117, &SwWW8ImplReader::Read_Special, //  "sprmCFSpec", // chp.fSpec  1 or 0 bit
 118, &SwWW8ImplReader::Read_Obj,       //  "sprmCFObj", // chp.fObj 1 or 0 bit
 119, (FNReadRecord)0, // "sprmPicBrcl", // pic.brcl brcl (see PIC structure definition) byte

 120,   (FNReadRecord)0, // "sprmPicScale", // pic.mx, pic.my, pic.dxaCropleft,
 121, (FNReadRecord)0, // "sprmPicBrcTop", // pic.brcTop BRC word
 122, (FNReadRecord)0, // "sprmPicBrcLeft", // pic.brcLeft BRC word
 123, (FNReadRecord)0, // "sprmPicBrcBottom", // pic.brcBottom BRC word
 124, (FNReadRecord)0, // "sprmPicBrcRight", // pic.brcRight BRC word
 125, (FNReadRecord)0, // "??125",
 126, (FNReadRecord)0, // "??126",
 127, (FNReadRecord)0, // "??127",
 128, (FNReadRecord)0, // "??128",
 129, (FNReadRecord)0, // "??129",

 130, (FNReadRecord)0, // "??130",
 131, (FNReadRecord)0, //  "sprmSScnsPgn", // sep.cnsPgn cns byte
 132, (FNReadRecord)0, // "sprmSiHeadingPgn", // sep.iHeadingPgn heading number level byte
 133, &SwWW8ImplReader::Read_OLST, // "sprmSOlstAnm", // sep.olstAnm OLST variable length
 134, (FNReadRecord)0, // "??135",
 135, (FNReadRecord)0, // "??135",
 136, (FNReadRecord)0, // "sprmSDxaColWidth", // sep.rgdxaColWidthSpacing complex (see below) 3 bytes
 137, (FNReadRecord)0, // "sprmSDxaColSpacing", // sep.rgdxaColWidthSpacing complex (see below) 3 bytes
 138, (FNReadRecord)0, // "sprmSFEvenlySpaced", // sep.fEvenlySpaced 1 or 0 byte
 139, (FNReadRecord)0, // "sprmSFProtected", // sep.fUnlocked 1 or 0 byte
 140, (FNReadRecord)0, // "sprmSDmBinFirst", // sep.dmBinFirst  word
 141, (FNReadRecord)0, // "sprmSDmBinOther", // sep.dmBinOther  word
 142, (FNReadRecord)0, // "sprmSBkc", // sep.bkc bkc byte BreakCode
 143, (FNReadRecord)0, // "sprmSFTitlePage", // sep.fTitlePage 0 or 1 byte
 144, (FNReadRecord)0, // "sprmSCcolumns", // sep.ccolM1 # of cols - 1 word
 145, (FNReadRecord)0, // "sprmSDxaColumns", // sep.dxaColumns dxa word
 146, (FNReadRecord)0, // "sprmSFAutoPgn", // sep.fAutoPgn obsolete byte
 147, (FNReadRecord)0, // "sprmSNfcPgn", // sep.nfcPgn nfc byte
 148, (FNReadRecord)0, // "sprmSDyaPgn", // sep.dyaPgn dya short
 149, (FNReadRecord)0, // "sprmSDxaPgn", // sep.dxaPgn dya short

 150, (FNReadRecord)0, // "sprmSFPgnRestart", // sep.fPgnRestart 0 or 1 byte
 151, (FNReadRecord)0, // "sprmSFEndnote", // sep.fEndnote 0 or 1 byte
 152, (FNReadRecord)0, // "sprmSLnc", // sep.lnc lnc byte
 153, (FNReadRecord)0, // "sprmSGprfIhdt", // sep.grpfIhdt grpfihdt (see Headers and Footers topic) byte
 154, (FNReadRecord)0, // "sprmSNLnnMod", // sep.nLnnMod non-neg int. word

 155, (FNReadRecord)0, // "sprmSDxaLnn", // sep.dxaLnn dxa word
 156, (FNReadRecord)0, // "sprmSDyaHdrTop", // sep.dyaHdrTop dya word
 157, (FNReadRecord)0, // "sprmSDyaHdrBottom", // sep.dyaHdrBottom dya word
 158, (FNReadRecord)0, // "sprmSLBetween", // sep.fLBetween 0 or 1 byte
 159, (FNReadRecord)0, // "sprmSVjc", // sep.vjc vjc byte

 160, (FNReadRecord)0, // "sprmSLnnMin", // sep.lnnMin lnn word
 161, (FNReadRecord)0, // "sprmSPgnStart", // sep.pgnStart pgn word
 162, (FNReadRecord)0, // "sprmSBOrientation", // sep.dmOrientPage dm byte

//??
 163, (FNReadRecord)0, // "?SprmSBCustomize 163", //???
 164, (FNReadRecord)0, // "sprmSXaPage", // sep.xaPage xa word
 165, (FNReadRecord)0, // "sprmSYaPage", // sep.yaPage ya word
 166, (FNReadRecord)0, // "sprmSDxaLeft", // sep.dxaLeft dxa word
 167, (FNReadRecord)0, // "sprmSDxaRight", // sep.dxaRight dxa word
 168, (FNReadRecord)0, // "sprmSDyaTop", // sep.dyaTop dya word
 169, (FNReadRecord)0, // "sprmSDyaBottom", // sep.dyaBottom dya word

 170, (FNReadRecord)0, // "sprmSDzaGutter", // sep.dzaGutter dza word
 171, (FNReadRecord)0, // "sprmSDMPaperReq", // sep.dmPaperReq dm word
 172, (FNReadRecord)0, // "??172",
 173, (FNReadRecord)0, // "??173",
 174, (FNReadRecord)0, // "??174",
 175, (FNReadRecord)0, // "??175",
 176, (FNReadRecord)0, // "??176",
 177, (FNReadRecord)0, // "??177",
 178, (FNReadRecord)0, // "??178",
 179, (FNReadRecord)0, // "??179",

 180, (FNReadRecord)0, // "??180",
 181, (FNReadRecord)0, // "??181",
 182, (FNReadRecord)0, //  "sprmTJc", // tap.jc jc word (low order byte is significant)
 183, (FNReadRecord)0, //  "sprmTDxaLeft", // tap.rgdxaCenter (see below) dxa word
 184, (FNReadRecord)0, //  "sprmTDxaGapHalf", // tap.dxaGapHalf, tap.rgdxaCenter (see below) dxa word
 185, (FNReadRecord)0, //  "sprmTFCantSplit", // tap.fCantSplit 1 or 0 byte
 186, (FNReadRecord)0, //  "sprmTTableHeader", // tap.fTableHeader 1 or 0 byte
 187, (FNReadRecord)0, // "sprmTTableBorders", // tap.rgbrcTable complex(see below) 12 bytes
 188, (FNReadRecord)0, // "sprmTDefTable10", // tap.rgdxaCenter, tap.rgtc complex (see below) variable length
 189, (FNReadRecord)0, //  "sprmTDyaRowHeight", // tap.dyaRowHeight dya word


//??
 190, (FNReadRecord)0, // "?sprmTDefTable", // tap.rgtc complex (see below)

//??
 191, (FNReadRecord)0, // "?sprmTDefTableShd", // tap.rgshd complex (see below)
 192, (FNReadRecord)0, // "sprmTTlp", // tap.tlp TLP 4 bytes
 193, (FNReadRecord)0, // "sprmTSetBrc", // tap.rgtc[].rgbrc complex (see below) 5 bytes
 194, (FNReadRecord)0, // "sprmTInsert", // tap.rgdxaCenter,tap.rgtc complex (see below) 4 bytes
 195, (FNReadRecord)0, // "sprmTDelete", // tap.rgdxaCenter, tap.rgtc complex (see below) word
 196, (FNReadRecord)0, // "sprmTDxaCol", // tap.rgdxaCenter complex (see below) 4 bytes
 197, (FNReadRecord)0, // "sprmTMerge", // tap.fFirstMerged, tap.fMerged complex (see below) word
 198, (FNReadRecord)0, // "sprmTSplit", // tap.fFirstMerged, tap.fMerged complex (see below) word
 199, (FNReadRecord)0, // "sprmTSetBrc10", // tap.rgtc[].rgbrc complex (see below) 5 bytes

 200, (FNReadRecord)0, // "sprmTSetShd", // tap.rgshd complex (see below) 4 bytes

#if 0
// ab hier Selbstdefinierte Ids

/* 0 (256) */   &SwWW8ImplReader::Read_Ftn,     // FootNote
/* 1 (257) */   &SwWW8ImplReader::Read_Ftn,     // EndNote
/* 2 (258) */   &SwWW8ImplReader::Read_Field,   // Feld
/* 3 (259) */   &SwWW8ImplReader::Read_Book,    // Bookmark
/* 4 (260) */   &SwWW8ImplReader::Read_Piece    // Piece-Anfang / Ende


// ab hier Selbstdefinierte Ids

    256, (FnOut)0, "FootNote",
    257, (FnOut)0, "EndNote",
    258, (FnOut)0, "??258 selbstdef.",
    259, (FnOut)0, "??259 selbstdef.",
    260, (FnOut)0, "Field",
#endif
//- neue ab Ver8 ------------------------------------------------------------

    0x4600, &SwWW8ImplReader::Read_StyleCode, //"sprmPIstd" // pap.istd;istd (style code);short;
    0xC601, (FNReadRecord)0, //"sprmPIstdPermute" // pap.istd;permutation vector (see below);variable length;
    0x2602, (FNReadRecord)0, //"sprmPIncLvl" // pap.istd, pap.lvl;difference between istd of base PAP and istd of PAP to be produced (see below);byte;
    0x2403, &SwWW8ImplReader::Read_Justify, //"sprmPJc" // pap.jc;jc (justification);byte;
    0x2404, (FNReadRecord)0, //"sprmPFSideBySide" // pap.fSideBySide;0 or 1;byte;
    0x2405, &SwWW8ImplReader::Read_KeepLines, //"sprmPFKeep" // pap.fKeep;0 or 1;byte;
    0x2406, &SwWW8ImplReader::Read_KeepParas, //"sprmPFKeepFollow" // pap.fKeepFollow;0 or 1;byte;
    0x2407, &SwWW8ImplReader::Read_BreakBefore, //"sprmPFPageBreakBefore" // pap.fPageBreakBefore;0 or 1;byte;
    0x2408, (FNReadRecord)0, //"sprmPBrcl" // pap.brcl;brcl;byte;
    0x2409, (FNReadRecord)0, //"sprmPBrcp" // pap.brcp;brcp;byte;
    0x260A, &SwWW8ImplReader::Read_ListLevel,       //"sprmPIlvl" // pap.ilvl;ilvl;byte;
    0x460B, &SwWW8ImplReader::Read_LFOPosition, //"sprmPIlfo" // pap.ilfo;ilfo (list index) ;short;
    0x240C, &SwWW8ImplReader::Read_NoLineNumb, //"sprmPFNoLineNumb" // pap.fNoLnn;0 or 1;byte;
    0xC60D, &SwWW8ImplReader::Read_Tab, //"sprmPChgTabsPapx" // pap.itbdMac, pap.rgdxaTab, pap.rgtbd;complex - see below;variable length
    0x840E, &SwWW8ImplReader::Read_LR, //"sprmPDxaRight" // pap.dxaRight;dxa;word;
    0x840F, &SwWW8ImplReader::Read_LR, //"sprmPDxaLeft" // pap.dxaLeft;dxa;word;
    0x4610, (FNReadRecord)0, //"sprmPNest" // pap.dxaLeft;dxa-see below;word;
    0x8411, &SwWW8ImplReader::Read_LR, //"sprmPDxaLeft1" // pap.dxaLeft1;dxa;word;
    0x6412, &SwWW8ImplReader::Read_LineSpace, //"sprmPDyaLine" // pap.lspd;an LSPD, a long word structure consisting of a short of dyaLine followed by a short of fMultLinespace - see below;long;
    0xA413, &SwWW8ImplReader::Read_UL, //"sprmPDyaBefore" // pap.dyaBefore;dya;word;
    0xA414, &SwWW8ImplReader::Read_UL, //"sprmPDyaAfter" // pap.dyaAfter;dya;word;
    0xC615, (FNReadRecord)0, //"sprmPChgTabs" // pap.itbdMac, pap.rgdxaTab, pap.rgtbd;complex - see below;variable length;
    0x2416, (FNReadRecord)0, //"sprmPFInTable" // pap.fInTable;0 or 1;byte;
    0x2417, &SwWW8ImplReader::Read_TabRowEnd, //"sprmPFTtp" // // pap.fTtp;0 or 1;byte;
    0x8418, (FNReadRecord)0, //"sprmPDxaAbs" // pap.dxaAbs;dxa;word;
    0x8419, (FNReadRecord)0, //"sprmPDyaAbs" // pap.dyaAbs;dya;word;
    0x841A, (FNReadRecord)0, //"sprmPDxaWidth" // pap.dxaWidth;dxa;word;
    0x261B, &SwWW8ImplReader::Read_ApoPPC, //"sprmPPc" // pap.pcHorz, pap.pcVert;complex - see below;byte;
    0x461C, (FNReadRecord)0, //"sprmPBrcTop10" // pap.brcTop;BRC10;word;
    0x461D, (FNReadRecord)0, //"sprmPBrcLeft10" // pap.brcLeft;BRC10;word;
    0x461E, (FNReadRecord)0, //"sprmPBrcBottom10" // pap.brcBottom;BRC10;word;
    0x461F, (FNReadRecord)0, //"sprmPBrcRight10" // pap.brcRight;BRC10;word;
    0x4620, (FNReadRecord)0, //"sprmPBrcBetween10" // pap.brcBetween;BRC10;word;
    0x4621, (FNReadRecord)0, //"sprmPBrcBar10" // pap.brcBar;BRC10;word;
    0x4622, (FNReadRecord)0, //"sprmPDxaFromText10" // pap.dxaFromText;dxa;word;
    0x2423, (FNReadRecord)0, //"sprmPWr" // pap.wr;wr (see description of PAP for definition;byte;
    0x6424, &SwWW8ImplReader::Read_Border, //"sprmPBrcTop" // pap.brcTop;BRC;long;
    0x6425, &SwWW8ImplReader::Read_Border, //"sprmPBrcLeft" // pap.brcLeft;BRC;long;
    0x6426, &SwWW8ImplReader::Read_Border, //"sprmPBrcBottom" // pap.brcBottom;BRC;long;
    0x6427, &SwWW8ImplReader::Read_Border, //"sprmPBrcRight" // pap.brcRight;BRC;long;
    0x6428, &SwWW8ImplReader::Read_Border, //"sprmPBrcBetween" // pap.brcBetween;BRC;long;
    0x6629, (FNReadRecord)0, //"sprmPBrcBar" // pap.brcBar;BRC;long;
    0x242A, &SwWW8ImplReader::Read_Hyphenation, //"sprmPFNoAutoHyph" // pap.fNoAutoHyph;0 or 1;byte;
    0x442B, (FNReadRecord)0, //"sprmPWHeightAbs" // pap.wHeightAbs;w;word;
    0x442C, (FNReadRecord)0, //"sprmPDcs" // pap.dcs;DCS;short;
    0x442D, &SwWW8ImplReader::Read_Shade, //"sprmPShd" // pap.shd;SHD;word;
    0x842E, (FNReadRecord)0, //"sprmPDyaFromText" // pap.dyaFromText;dya;word;
    0x842F, (FNReadRecord)0, //"sprmPDxaFromText" // pap.dxaFromText;dxa;word;
    0x2430, (FNReadRecord)0, //"sprmPFLocked" // pap.fLocked;0 or 1;byte;
    0x2431, &SwWW8ImplReader::Read_WidowControl, //"sprmPFWidowControl" // pap.fWidowControl;0 or 1;byte;
    0xC632, (FNReadRecord)0, //"sprmPRuler" // ;;variable length;
    0x2433, &SwWW8ImplReader::Read_BoolItem, //"sprmPFKinsoku" // pap.fKinsoku;0 or 1;byte;
    0x2434, (FNReadRecord)0, //"sprmPFWordWrap" // pap.fWordWrap;0 or 1;byte;
    0x2435, &SwWW8ImplReader::Read_BoolItem, //"sprmPFOverflowPunct" // pap.fOverflowPunct;0 or 1;byte;
    0x2436, (FNReadRecord)0, //"sprmPFTopLinePunct" // pap.fTopLinePunct;0 or 1;byte;
    0x2437, &SwWW8ImplReader::Read_BoolItem, //"sprmPFAutoSpaceDE" // pap.fAutoSpaceDE;0 or 1;byte;
    0x2438, (FNReadRecord)0, //"sprmPFAutoSpaceDN" // pap.fAutoSpaceDN;0 or 1;byte;
    0x4439, (FNReadRecord)0, //"sprmPWAlignFont" // pap.wAlignFont;iFa (see description of PAP for definition);word;
    0x443A, (FNReadRecord)0, //"sprmPFrameTextFlow" // pap.fVertical pap.fBackward pap.fRotateFont;complex (see description of PAP for definition);word;
    0x243B, (FNReadRecord)0, //"sprmPISnapBaseLine" // obsolete: not applicable in Word97 and later versions;;byte;
    0xC63E, &SwWW8ImplReader::Read_ANLevelDesc, //"sprmPAnld" // pap.anld;;variable length;
    0xC63F, (FNReadRecord)0, //"sprmPPropRMark" // pap.fPropRMark;complex (see below);variable length;
    0x2640,  &SwWW8ImplReader::Read_POutLvl, //"sprmPOutLvl" // pap.lvl;has no effect if pap.istd is < 1 or is > 9;byte;
    0x2441, (FNReadRecord)0, //"sprmPFBiDi" // ;;byte;
    0x2443, (FNReadRecord)0, //"sprmPFNumRMIns" // pap.fNumRMIns;1 or 0;bit;
    0x2444, (FNReadRecord)0, //"sprmPCrLf" // ;;byte;
    0xC645, (FNReadRecord)0, //"sprmPNumRM" // pap.numrm;;variable length;
    0x6645, (FNReadRecord)0, //"sprmPHugePapx" // see below;fc in the data stream to locate the huge grpprl (see below);long;
    0x6646, (FNReadRecord)0, //"sprmPHugePapx" // see below;fc in the data stream to locate the huge grpprl (see below);long;
    0x2447, (FNReadRecord)0, //"sprmPFUsePgsuSettings" // pap.fUsePgsuSettings;1 or 0;byte;
    0x2448, (FNReadRecord)0, //"sprmPFAdjustRight" // pap.fAdjustRight;1 or 0;byte;
    0x0800, &SwWW8ImplReader::Read_CFRMarkDel, //"sprmCFRMarkDel" // chp.fRMarkDel;1 or 0;bit;
    0x0801, &SwWW8ImplReader::Read_CFRMark,    //"sprmCFRMark" // chp.fRMark;1 or 0;bit;
    0x0802, &SwWW8ImplReader::Read_FldVanish, //"sprmCFFldVanish" // chp.fFldVanish;1 or 0;bit;
    0x6A03, &SwWW8ImplReader::Read_PicLoc, //"sprmCPicLocation" // chp.fcPic and chp.fSpec;see below;variable length, length recorded is always 4;
    0x4804, (FNReadRecord)0, //"sprmCIbstRMark" // chp.ibstRMark;index into sttbRMark;short;
    0x6805, (FNReadRecord)0, //"sprmCDttmRMark" // chp.dttmRMark;DTTM;long;
    0x0806, (FNReadRecord)0, //"sprmCFData" // chp.fData;1 or 0;bit;
    0x4807, (FNReadRecord)0, //"sprmCIdslRMark" // chp.idslRMReason;an index to a table of strings defined in Word 6.0 executables;short;
    0xEA08, &SwWW8ImplReader::Read_CharSet, //"sprmCChs" // chp.fChsDiff and chp.chse;see below;3 bytes;
    0x6A09, &SwWW8ImplReader::Read_Symbol, //"sprmCSymbol" // chp.fSpec, chp.xchSym and chp.ftcSym;see below;variable length, length recorded is always 4;
    0x080A, &SwWW8ImplReader::Read_Obj, //"sprmCFOle2" // chp.fOle2;1 or 0;bit;
//0x480B, obsolete,"sprmCIdCharType", // obsolete: not applicable in Word97 and later versions;;;
    0x2A0C, &SwWW8ImplReader::Read_CharHighlight, //"sprmCHighlight" // chp.fHighlight, chp.icoHighlight;ico (fHighlight is set to 1 iff ico is not 0);byte;
    0x680E, &SwWW8ImplReader::Read_PicLoc, //"sprmCObjLocation" // chp.fcObj;FC;long;
//0x2A10, ? ? ?  , "sprmCFFtcAsciSymb", // ;;;
    0x4A30, &SwWW8ImplReader::Read_CColl, //"sprmCIstd" // chp.istd;istd, see stylesheet definition;short;
    0xCA31, (FNReadRecord)0, //"sprmCIstdPermute" // chp.istd;permutation vector (see below);variable length;
    0x2A32, (FNReadRecord)0, //"sprmCDefault" // whole CHP (see below);none;variable length;
    0x2A33, (FNReadRecord)0, //"sprmCPlain" // whole CHP (see below);none; Laenge: 0;

    0x2A34, &SwWW8ImplReader::Read_Emphasis, // "sprmCKcd", // ;;;

    0x0835, &SwWW8ImplReader::Read_BoldUsw, //"sprmCFBold" // chp.fBold;0,1, 128, or 129 (see below);byte;
    0x0836, &SwWW8ImplReader::Read_BoldUsw, //"sprmCFItalic" // chp.fItalic;0,1, 128, or 129 (see below);byte;
    0x0837, &SwWW8ImplReader::Read_BoldUsw, //"sprmCFStrike" // chp.fStrike;0,1, 128, or 129 (see below);byte;
    0x0838, &SwWW8ImplReader::Read_BoldUsw, //"sprmCFOutline" // chp.fOutline;0,1, 128, or 129 (see below);byte;
    0x0839, &SwWW8ImplReader::Read_BoldUsw, //"sprmCFShadow" // chp.fShadow;0,1, 128, or 129 (see below);byte;
    0x083A, &SwWW8ImplReader::Read_BoldUsw, //"sprmCFSmallCaps" // chp.fSmallCaps;0,1, 128, or 129 (see below);byte;
    0x083B, &SwWW8ImplReader::Read_BoldUsw, //"sprmCFCaps" // chp.fCaps;0,1, 128, or 129 (see below);byte;

    0x083C, &SwWW8ImplReader::Read_Invisible, //"sprmCFVanish" // chp.fVanish;0,1, 128, or 129 (see below);byte;

//0x4A3D, (FNReadRecord)0, //"sprmCFtcDefault" // ;ftc, only used internally, never stored in file;word;
    0x2A3E, &SwWW8ImplReader::Read_Underline, //"sprmCKul" // chp.kul;kul;byte;
    0xEA3F, (FNReadRecord)0, //"sprmCSizePos" // chp.hps, chp.hpsPos;(see below);3 bytes;
    0x8840, &SwWW8ImplReader::Read_Kern, //"sprmCDxaSpace" // chp.dxaSpace;dxa;word;
    0x4A41, &SwWW8ImplReader::Read_Language, //"sprmCLid" // ;only used internally never stored;word;
    0x2A42, &SwWW8ImplReader::Read_TxtColor, //"sprmCIco" // chp.ico;ico;byte;
    0x4A43, &SwWW8ImplReader::Read_FontSize, //"sprmCHps" // chp.hps;hps;byte;
    0x2A44, (FNReadRecord)0, //"sprmCHpsInc" // chp.hps;(see below);byte;
    0x4845, &SwWW8ImplReader::Read_SubSuperProp, //"sprmCHpsPos" // chp.hpsPos;hps;byte;
    0x2A46, (FNReadRecord)0, //"sprmCHpsPosAdj" // chp.hpsPos;hps (see below);byte;
    0xCA47, &SwWW8ImplReader::Read_Majority, //"sprmCMajority" // chp.fBold, chp.fItalic, chp.fSmallCaps, chp.fVanish, chp.fStrike, chp.fCaps, chp.rgftc, chp.hps, chp.hpsPos, chp.kul, chp.dxaSpace, chp.ico, chp.rglid;complex (see below);variable length, length byte plus size of following grpprl;
    0x2A48, &SwWW8ImplReader::Read_SubSuper, //"sprmCIss" // chp.iss;iss;byte;
    0xCA49, (FNReadRecord)0, //"sprmCHpsNew50" // chp.hps;hps;variable width, length always recorded as 2;
    0xCA4A, (FNReadRecord)0, //"sprmCHpsInc1" // chp.hps;complex (see below);variable width, length always recorded as 2;
    0x484B, &SwWW8ImplReader::Read_FontKern, //"sprmCHpsKern" // chp.hpsKern;hps;short;
    0xCA4C, &SwWW8ImplReader::Read_Majority, //"sprmCMajority50" // chp.fBold, chp.fItalic, chp.fSmallCaps, chp.fVanish, chp.fStrike, chp.fCaps, chp.ftc, chp.hps, chp.hpsPos, chp.kul, chp.dxaSpace, chp.ico,;complex (see below);variable length;
    0x4A4D, (FNReadRecord)0, //"sprmCHpsMul" // chp.hps;percentage to grow hps;short;
    0x484E, (FNReadRecord)0, //"sprmCYsri" // chp.ysri;ysri;short;
    0x4A4F, &SwWW8ImplReader::Read_FontCode, //"sprmCRgFtc0" // chp.rgftc[0];ftc for ASCII text (see below);short;
    0x4A50, &SwWW8ImplReader::Read_FontCode, //"sprmCRgFtc1" // chp.rgftc[1];ftc for Far East text (see below);short;
    0x4A51, &SwWW8ImplReader::Read_FontCode, //"sprmCRgFtc2" // chp.rgftc[2];ftc for non-Far East text (see below);short;
    0x4852, &SwWW8ImplReader::Read_ScaleWidth,  // ? ? ?  , "sprmCCharScale", // ;;;
    0x2A53, &SwWW8ImplReader::Read_BoldUsw, //"sprmCFDStrike" // chp.fDStrike;;byte;
    0x0854, &SwWW8ImplReader::Read_Relief, //"sprmCFImprint" // chp.fImprint;1 or 0;bit;
    0x0855, &SwWW8ImplReader::Read_Special, //"sprmCFSpec" // chp.fSpec ;1 or 0;bit;
    0x0856, &SwWW8ImplReader::Read_Obj, //"sprmCFObj" // chp.fObj;1 or 0;bit;
    0xCA57, &SwWW8ImplReader::Read_CPropRMark, //"sprmCPropRMark" // chp.fPropRMark, chp.ibstPropRMark, chp.dttmPropRMark;Complex (see below);variable length always recorded as 7 bytes;
    0x0858, &SwWW8ImplReader::Read_Relief, //"sprmCFEmboss" // chp.fEmboss;1 or 0;bit;
    0x2859, (FNReadRecord)0, //"sprmCSfxText" // chp.sfxtText;text animation;byte;
//0x085A, ? ? ?  , "sprmCFBiDi", // ;;;
//0x085B, ? ? ?  , "sprmCFDiacColor", // ;;;
//0x085C, ? ? ?  , "sprmCFBoldBi", // ;;;
//0x085D, ? ? ?  , "sprmCFItalicBi", // ;;;
//0x4A5E, ? ? ?  , "sprmCFtcBi", // ;;;
//0x485F, ? ? ?  , "sprmCLidBi", // ;;;
//0x4A60, ? ? ?  , "sprmCIcoBi", // ;;;
//0x4A61, &SwWW8ImplReader::Read_FontSize,  // "sprmCHpsBi", // ;;;
    0xCA62, (FNReadRecord)0, //"sprmCDispFldRMark" // chp.fDispFldRMark, chp.ibstDispFldRMark, chp.dttmDispFldRMark ;Complex (see below);variable length always recorded as 39 bytes;
    0x4863, (FNReadRecord)0, //"sprmCIbstRMarkDel" // chp.ibstRMarkDel;index into sttbRMark;short;
    0x6864, (FNReadRecord)0, //"sprmCDttmRMarkDel" // chp.dttmRMarkDel;DTTM;long;
    0x6865, (FNReadRecord)0, //"sprmCBrc" // chp.brc;BRC;long;
    0x4866, &SwWW8ImplReader::Read_CharShadow, //"sprmCShd" // chp.shd;SHD;short;
    0x4867, (FNReadRecord)0, //"sprmCIdslRMarkDel" // chp.idslRMReasonDel;an index to a table of strings defined in Word 6.0 executables;short;
    0x0868, (FNReadRecord)0, //"sprmCFUsePgsuSettings" // chp.fUsePgsuSettings;1 or 0;bit;
    0x486B, (FNReadRecord)0, //"sprmCCpg" // ;;word;
    0x486D, &SwWW8ImplReader::Read_Language, //"sprmCRgLid0" // chp.rglid[0];LID: for non-Far East text;word;
    0x486E, &SwWW8ImplReader::Read_Language, //"sprmCRgLid1" // chp.rglid[1];LID: for Far East text;word;
    0x286F, (FNReadRecord)0, //"sprmCIdctHint" // chp.idctHint;IDCT: (see below);byte;
    0x2E00, (FNReadRecord)0, //"sprmPicBrcl" // pic.brcl;brcl (see PIC structure definition);byte;
    0xCE01, (FNReadRecord)0, //"sprmPicScale" // pic.mx, pic.my, pic.dxaCropleft, pic.dyaCropTop pic.dxaCropRight, pic.dyaCropBottom;Complex (see below);length byte plus 12 bytes;
    0x6C02, (FNReadRecord)0, //"sprmPicBrcTop" // pic.brcTop;BRC;long;
    0x6C03, (FNReadRecord)0, //"sprmPicBrcLeft" // pic.brcLeft;BRC;long;
    0x6C04, (FNReadRecord)0, //"sprmPicBrcBottom" // pic.brcBottom;BRC;long;
    0x6C05, (FNReadRecord)0, //"sprmPicBrcRight" // pic.brcRight;BRC;long;
    0x3000, (FNReadRecord)0, //"sprmScnsPgn" // sep.cnsPgn;cns;byte;
    0x3001, (FNReadRecord)0, //"sprmSiHeadingPgn" // sep.iHeadingPgn;heading number level;byte;
    0xD202, &SwWW8ImplReader::Read_OLST, //"sprmSOlstAnm" // sep.olstAnm;OLST;variable length;
    0xF203, (FNReadRecord)0, //"sprmSDxaColWidth" // sep.rgdxaColWidthSpacing;complex (see below);3 bytes;
    0xF204, (FNReadRecord)0, //"sprmSDxaColSpacing" // sep.rgdxaColWidthSpacing;complex (see below);3 bytes;
    0x3005, (FNReadRecord)0, //"sprmSFEvenlySpaced" // sep.fEvenlySpaced;1 or 0;byte;
    0x3006, (FNReadRecord)0, //"sprmSFProtected" // sep.fUnlocked;1 or 0;byte;
    0x5007, (FNReadRecord)0, //"sprmSDmBinFirst" // sep.dmBinFirst;;word;
    0x5008, (FNReadRecord)0, //"sprmSDmBinOther" // sep.dmBinOther;;word;
    0x3009, (FNReadRecord)0, //"sprmSBkc" // sep.bkc;bkc;byte;
    0x300A, (FNReadRecord)0, //"sprmSFTitlePage" // sep.fTitlePage;0 or 1;byte;
    0x500B, (FNReadRecord)0, //"sprmSCcolumns" // sep.ccolM1;# of cols - 1;word;
    0x900C, (FNReadRecord)0, //"sprmSDxaColumns" // sep.dxaColumns;dxa;word;
    0x300D, (FNReadRecord)0, //"sprmSFAutoPgn" // sep.fAutoPgn;obsolete;byte;
    0x300E, (FNReadRecord)0, //"sprmSNfcPgn" // sep.nfcPgn;nfc;byte;
    0xB00F, (FNReadRecord)0, //"sprmSDyaPgn" // sep.dyaPgn;dya;short;
    0xB010, (FNReadRecord)0, //"sprmSDxaPgn" // sep.dxaPgn;dya;short;
    0x3011, (FNReadRecord)0, //"sprmSFPgnRestart" // sep.fPgnRestart;0 or 1;byte;
    0x3012, (FNReadRecord)0, //"sprmSFEndnote" // sep.fEndnote;0 or 1;byte;
    0x3013, (FNReadRecord)0, //"sprmSLnc" // sep.lnc;lnc;byte;
    0x3014, (FNReadRecord)0, //"sprmSGprfIhdt" // sep.grpfIhdt;grpfihdt (see Headers and Footers topic);byte;
    0x5015, (FNReadRecord)0, //"sprmSNLnnMod" // sep.nLnnMod;non-neg int.;word;
    0x9016, (FNReadRecord)0, //"sprmSDxaLnn" // sep.dxaLnn;dxa;word;
    0xB017, (FNReadRecord)0, //"sprmSDyaHdrTop" // sep.dyaHdrTop;dya;word;
    0xB018, (FNReadRecord)0, //"sprmSDyaHdrBottom" // sep.dyaHdrBottom;dya;word;
    0x3019, (FNReadRecord)0, //"sprmSLBetween" // sep.fLBetween;0 or 1;byte;
    0x301A, (FNReadRecord)0, //"sprmSVjc" // sep.vjc;vjc;byte;
    0x501B, (FNReadRecord)0, //"sprmSLnnMin" // sep.lnnMin;lnn;word;
    0x501C, (FNReadRecord)0, //"sprmSPgnStart" // sep.pgnStart;pgn;word;
    0x301D, (FNReadRecord)0, //"sprmSBOrientation" // sep.dmOrientPage;dm;byte;
//0x301E, ? ? ?  , "sprmSBCustomize", // ;;;
    0xB01F, (FNReadRecord)0, //"sprmSXaPage" // sep.xaPage;xa;word;
    0xB020, (FNReadRecord)0, //"sprmSYaPage" // sep.yaPage;ya;word;
    0x2205, (FNReadRecord)0, //"sprmSDxaLeft" // sep.dxaLeft;dxa;word;
    0xB022, (FNReadRecord)0, //"sprmSDxaRight" // sep.dxaRight;dxa;word;
    0x9023, (FNReadRecord)0, //"sprmSDyaTop" // sep.dyaTop;dya;word;
    0x9024, (FNReadRecord)0, //"sprmSDyaBottom" // sep.dyaBottom;dya;word;
    0xB025, (FNReadRecord)0, //"sprmSDzaGutter" // sep.dzaGutter;dza;word;
    0x5026, (FNReadRecord)0, //"sprmSDmPaperReq" // sep.dmPaperReq;dm;word;
    0xD227, (FNReadRecord)0, //"sprmSPropRMark" // sep.fPropRMark, sep.ibstPropRMark, sep.dttmPropRMark ;complex (see below);variable length always recorded as 7 bytes;
//0x3228, ? ? ?  , "sprmSFBiDi", // ;;;
//0x3229, ? ? ?  , "sprmSFFacingCol", // ;;;
//0x322A, ? ? ?  , "sprmSFRTLGutter", // ;;;
    0x702B, (FNReadRecord)0, //"sprmSBrcTop" // sep.brcTop;BRC;long;
    0x702C, (FNReadRecord)0, //"sprmSBrcLeft" // sep.brcLeft;BRC;long;
    0x702D, (FNReadRecord)0, //"sprmSBrcBottom" // sep.brcBottom;BRC;long;
    0x702E, (FNReadRecord)0, //"sprmSBrcRight" // sep.brcRight;BRC;long;
    0x522F, (FNReadRecord)0, //"sprmSPgbProp" // sep.pgbProp;;word;
    0x7030, (FNReadRecord)0, //"sprmSDxtCharSpace" // sep.dxtCharSpace;dxt;long;
    0x9031, (FNReadRecord)0, //"sprmSDyaLinePitch" // sep.dyaLinePitch;dya;  WRONG:long;  RIGHT:short;  !!!
//0x5032, ? ? ?  , "sprmSClm", // ;;;
    0x5033, (FNReadRecord)0, //"sprmSTextFlow" // sep.wTextFlow;complex (see below);short;
    0x5400, (FNReadRecord)0, //"sprmTJc" // tap.jc;jc;word (low order byte is significant);
    0x9601, (FNReadRecord)0, //"sprmTDxaLeft" // tap.rgdxaCenter (see below);dxa;word;
    0x9602, (FNReadRecord)0, //"sprmTDxaGapHalf" // tap.dxaGapHalf, tap.rgdxaCenter (see below);dxa;word;
    0x3403, (FNReadRecord)0, //"sprmTFCantSplit" // tap.fCantSplit;1 or 0;byte;
    0x3404, (FNReadRecord)0, //"sprmTTableHeader" // tap.fTableHeader;1 or 0;byte;
    0xD605, (FNReadRecord)0, //"sprmTTableBorders" // tap.rgbrcTable;complex(see below);24 bytes;
    0xD606, (FNReadRecord)0, //"sprmTDefTable10" // tap.rgdxaCenter, tap.rgtc;complex (see below);variable length;
    0x9407, (FNReadRecord)0, //"sprmTDyaRowHeight" // tap.dyaRowHeight;dya;word;
    0xD608, (FNReadRecord)0, //"sprmTDefTable" // tap.rgtc;complex (see below);;
    0xD609, (FNReadRecord)0, //"sprmTDefTableShd" // tap.rgshd;complex (see below);;
    0x740A, (FNReadRecord)0, //"sprmTTlp" // tap.tlp;TLP;4 bytes;
//0x560B, ? ? ?  , "sprmTFBiDi", // ;;;
//0x740C, ? ? ?  , "sprmTHTMLProps", // ;;;
    0xD620, (FNReadRecord)0, //"sprmTSetBrc" // tap.rgtc[].rgbrc;complex (see below);5 bytes;
    0x7621, &SwWW8ImplReader::Read_TabCellInsert, //"sprmTInsert" // tap.rgdxaCenter, tap.rgtc;complex (see below);4 bytes;
    0x5622, &SwWW8ImplReader::Read_TabCellDelete, //"sprmTDelete" // tap.rgdxaCenter, tap.rgtc;complex (see below);word;
    0x7623, (FNReadRecord)0, //"sprmTDxaCol" // tap.rgdxaCenter;complex (see below);4 bytes;
    0x5624, (FNReadRecord)0, //"sprmTMerge" // tap.fFirstMerged, tap.fMerged;complex (see below);word;
    0x5625, (FNReadRecord)0, //"sprmTSplit" // tap.fFirstMerged, tap.fMerged;complex (see below);word;
    0xD626, (FNReadRecord)0, //"sprmTSetBrc10" // tap.rgtc[].rgbrc;complex (see below);5 bytes;
    0x7627, (FNReadRecord)0, //"sprmTSetShd" // tap.rgshd;complex (see below);4 bytes;
    0x7628, (FNReadRecord)0, //"sprmTSetShdOdd" // tap.rgshd;complex (see below);4 bytes;
    0x7629, (FNReadRecord)0, //"sprmTTextFlow" // tap.rgtc[].fVerticaltap.rgtc[].fBackwardtap.rgtc[].fRotateFont;0 or 10 or 10 or 1;word;
//0xD62A, ? ? ?  , "sprmTDiagLine", // ;;;
    0xD62B, (FNReadRecord)0, //"sprmTVertMerge" // tap.rgtc[].vertMerge;complex (see below);variable length always recorded as 2 bytes;
    0xD62C, (FNReadRecord)0, //"sprmTVertAlign" // tap.rgtc[].vertAlign;complex (see below);variable length always recorded as 3 byte;
    0xCA78, &SwWW8ImplReader::Read_DoubleLine_Rotate,
    0x6649, (FNReadRecord)0, //undocumented 4 byte pap sprm (special though)
    0x6649, (FNReadRecord)0, //undocumented 4 byte pap sprm (special though)
    0xF614, (FNReadRecord)0, //undocumented 4 byte pap sprm (special though)
    0xD61A, (FNReadRecord)0, //undocumented 4 byte pap sprm (special though)
    0xD61B, (FNReadRecord)0, //undocumented 4 byte pap sprm (special though)
    0xD61C, (FNReadRecord)0, //undocumented 4 byte pap sprm (special though)
    0xD61D, (FNReadRecord)0, //undocumented 4 byte pap sprm (special though)
    0xD634, (FNReadRecord)0, //undocumented 4 byte pap sprm (special though)
    0xF661, (FNReadRecord)0  //undocumented 4 byte pap sprm (special though)
};

//-----------------------------------------
//      Hilfsroutinen : SPRM finden
//-----------------------------------------

static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC )
 _Optlink
#endif
    CompSprmReadId( const void *pFirst, const void *pSecond)
{
    return( ((SprmReadInfo*)pFirst )->nId - ((SprmReadInfo*)pSecond)->nId );
}


SprmReadInfo& WW8GetSprmReadInfo( USHORT nId )
{
    // ggfs. Tab sortieren
    static BOOL bInit = FALSE;
    if( !bInit )
    {
        qsort( (void*)aSprmReadTab,
            sizeof( aSprmReadTab      ) / sizeof (aSprmReadTab[ 0 ]),
            sizeof( aSprmReadTab[ 0 ] ),
            CompSprmReadId );
        bInit = TRUE;
    }
    // Sprm heraussuchen
    void* pFound;
    SprmReadInfo aSrch;
    aSrch.nId = nId;
    if( 0 == ( pFound = bsearch( (BYTE *) &aSrch,
                        (void*) aSprmReadTab,
                        sizeof( aSprmReadTab      ) / sizeof (aSprmReadTab[ 0 ]),
                        sizeof( aSprmReadTab[ 0 ] ),
                        CompSprmReadId )))
    {
        // im Fehlerfall auf Nulltes Element verweisen
        pFound = (void*)aSprmReadTab;
    }
    return *(SprmReadInfo*) pFound;
}



//-----------------------------------------
//      Hilfsroutinen : SPRMs
//-----------------------------------------

void SwWW8ImplReader::EndSprm( USHORT nId )
{
    if( ( nId > 255 ) && ( nId < 0x0800 ) ) return;

    SprmReadInfo& rSprm = WW8GetSprmReadInfo( nId );
    /*
    if ( aWwSprmTab[nId] == 0 )
        return;

    (this->*SprmReadInfo)( nId, 0, -1 );
    */
    if( rSprm.pReadFnc )
        (this->*rSprm.pReadFnc)( nId, 0, -1 );
}

short SwWW8ImplReader::ImportSprm( BYTE* pPos, short nSprmsLen, USHORT nId )
{
    BYTE nDelta;
    BYTE nV = pWwFib->nVersion;

    if( nId )
        nDelta = bVer67 ? 0 : 1;
    else
        nId = WW8GetSprmId( nV, pPos, &nDelta );

#ifdef DEBUG
    ASSERT( nId != 0xff, "Sprm FF !!!!" );
#endif //DEBUG

    SprmReadInfo& rSprm = WW8GetSprmReadInfo( nId );

    short nFixedLen = 1 + nDelta + WW8SprmDataOfs( nId );
    short nL = WW8GetSprmSizeBrutto( nV, pPos, &nId );

    if( rSprm.pReadFnc )
    {
        (this->*rSprm.pReadFnc)( nId, pPos + nFixedLen, nL - nFixedLen );
    }
    return nL;
}




/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/ww8par6.cxx,v 1.17 2001-03-16 17:15:59 jp Exp $


      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.16  2001/03/16 14:19:41  cmc
      Add some undocumented sprms

      Revision 1.15  2001/03/13 16:21:22  cmc
      ##503##, #84126#. Incorrect endnote setting, and duplicate code

      Revision 1.14  2001/02/23 12:45:26  os
      Complete use of DefaultNumbering component

      Revision 1.13  2001/02/20 10:34:57  cmc
      #83546# Don't create a tab stop description without any tab stops

      Revision 1.12  2001/02/15 20:08:10  jp
      im-/export the Rotate-/ScaleWidth-Character attribut

      Revision 1.11  2001/02/07 16:15:13  cmc
      #83307# Change automatic width handling for frames, with special care for header/footer problems

      Revision 1.10  2001/02/07 11:12:31  cmc
      #83308# Allow negative frame positions

      Revision 1.9  2001/02/06 17:28:21  cmc
      #83581# CJK Two Lines in One {Im|Ex}port for Word

      Revision 1.8  2001/02/06 13:13:07  cmc
      #83356# Support Explicit Page Start Number for WW6/7

      Revision 1.7  2001/01/26 10:57:04  cmc
      ##158## Table relief Border width hack modified

      Revision 1.6  2000/12/01 11:22:52  jp
      Task #81077#: im-/export of CJK documents

      Revision 1.5  2000/11/28 15:22:49  khz
      #79657# avoid accessing NULL pointer when reading LVL properties

      Revision 1.4  2000/10/26 12:23:38  khz
      add paragraph's left border to TabStops (as WW does)

      Revision 1.3  2000/10/25 14:10:36  khz
      Now supporting negative horizontal indentation of paragrahps and tables

      Revision 1.2  2000/10/17 15:06:36  khz
      Bug #79439# WW border ordering is different than StarWriter's

      Revision 1.1.1.1  2000/09/18 17:14:59  hr
      initial import

      Revision 1.104  2000/09/18 16:05:01  willem.vandorp
      OpenOffice header added.

      Revision 1.103  2000/08/24 13:20:54  kz
      Max (long, long)

      Revision 1.102  2000/08/21 15:03:09  khz
      #77692# import page orientation

      Revision 1.101  2000/08/18 09:48:33  khz
      Import Line Numbering (restart on new section)

      Revision 1.100  2000/08/18 06:47:29  khz
      Import Line Numbering

      Revision 1.99  2000/07/25 15:16:29  khz
      #76811# read/write AutoHyphenation flag from/into Document Properties

      Revision 1.98  2000/07/17 13:47:04  khz
      #73987# check if sprmSNfcPgn should cause section change or not

      Revision 1.97  2000/07/11 11:30:59  khz
      #76673# prepare implementation of sprmTDelete and sprmTInsert

      Revision 1.96  2000/06/28 08:07:48  khz
      #70915# Insert Section if end-note with flag 'on end of section' found

      Revision 1.95  2000/05/25 08:06:57  khz
      Piece Table optimization, Unicode changes, Bugfixes

      Revision 1.94  2000/05/16 12:03:50  jp
      Changes for unicode

      Revision 1.93  2000/05/16 11:23:49  khz
      Unicode code-conversion

      Revision 1.92  2000/03/21 10:37:08  khz
      Task #74319# - do NOT close just inserted (still empty!) section in CreateSep()
                   - closing of old section may cause the creation of a new one
      Task #66529# improve Header/Footer import

      Revision 1.91  2000/03/10 14:27:15  khz
      Task #65529# improve detection if Section is identical with previous

      Revision 1.90  2000/03/03 15:20:02  os
      StarView remainders removed

      Revision 1.89  2000/02/23 17:43:03  cmc
      #68832# Consider endnotes for header footer code

      Revision 1.88  2000/02/22 14:55:19  khz
      partial fix of #66832#

      Revision 1.87  2000/02/21 13:08:30  jp
      #70473# changes for unicode

      Revision 1.86  2000/02/15 16:09:28  jp
      #70473# changes for unicode

      Revision 1.85  2000/02/09 11:43:31  khz
      no Task Id: code cleaned for of annoying warning

      Revision 1.84  2000/01/21 16:42:44  khz
      Task #69155# set pPaM when inserting section with/without columns

      Revision 1.83  2000/01/19 17:08:51  khz
      Task #69178# never insert Fontattribute when skipping text content

      Revision 1.82  2000/01/14 11:27:59  khz
      Task #68832# -- CORRECTION: Task Number of rev below should have been: 68832 !

      Revision 1.81  2000/01/14 11:11:34  khz
      Task #71343# look for Sprm 37 (not 5) to recognize APO

      Revision 1.80  2000/01/06 15:23:49  khz
      Task #71411# Let last Section be unbalanced

      Revision 1.79  1999/12/23 15:10:46  jp
      Task #70915#: set Ftn-/End-TxtAtEnd attributes at sections

      Revision 1.78  1999/12/23 14:03:42  khz
      Task #68143# avoid attributes atached solely to Cell-End marks

      Revision 1.77  1999/12/23 10:24:53  khz
      Task #71209# avoid accessing pPageDesk if it's 0 ;-)

      Revision 1.76  1999/12/22 18:03:35  khz
      Task #70919# look if ParaStyle is different behind filed than it was before

      Revision 1.75  1999/12/09 16:46:36  khz
      Task #69180# allow Tabs if right of 1st-line-start OR right of paragraph margin

      Revision 1.74  1999/12/07 14:28:56  khz
      Task #69508# import sprmPHugePapx by reading DATA stream

      Revision 1.73  1999/12/01 14:35:52  khz
      Task #68488# Graphics in Sections with more than one Column

      Revision 1.72  1999/11/25 12:28:37  khz
      Task #68482# Store data in member *after* memset(this,0,...) in WW8FlyPara()

      Revision 1.71  1999/11/24 12:33:15  khz
      Task #66194# don't consider Sect-Properties to be unequal when one is protected

      Revision 1.70  1999/11/12 15:29:25  khz
      Task #69204#: prevent from inserting '?' when ConvertFromUnicode() fails

      Revision 1.69  1999/11/12 13:05:03  jp
      new: read sprmCHighlight

      Revision 1.68  1999/11/02 15:59:49  khz
      import new TOX_CONTENT and TOX_INDEX features (2)

      Revision 1.67  1999/10/13 21:06:12  khz
      Import Redlining (3)

      Revision 1.66  1999/10/08 09:26:23  khz
      Import Redlining

      Revision 1.65  1999/09/10 15:36:35  khz
      CharSet matching made by TENCINFO.H::rtl_getTextEncodingFromWindowsCharset()

      Revision 1.64  1999/09/09 18:16:25  khz
      CharSet matching now done in central methode WW8SCAN.HXX::WW8GetCharSet()

      Revision 1.63  1999/08/10 13:16:26  KHZ
      Task #66019# corrction of Font Family by analyzing Font Name

*************************************************************************/
