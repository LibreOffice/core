/*************************************************************************
 *
 *  $RCSfile: ww8par2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:58 $
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
#include <tools/solar.h>
#include "pam.hxx"              // fuer SwPam
#include "doc.hxx"

#ifndef _DOCARY_HXX
#include "docary.hxx"
#endif

#include "ndtxt.hxx"            // class SwTxtNode
#include "hintids.hxx"
#include "paratr.hxx"           // SwNumRuleItem

// Folgende sollten demnaechst ins Precompiled-Header-File
#include "poolfmt.hxx"          // RES_POOLCOLL_STANDARD
#include "swtable.hxx"          // class SwTableLines, ...
#include "tblsel.hxx"           // class _SwSelBox
#include "mdiexp.hxx"           // Progress

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SVX_ORPHITEM_HXX //autogen
#include <svx/orphitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <svx/widwitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifdef DEBUG
#ifndef _SV_SOUND_HXX //autogen
 #include <vcl/sound.hxx>
#endif
#endif

// Folgende bleiben hier
#include "fltshell.hxx"         // fuer den Attribut Stack
#include "ww8struc.hxx"         // struct TC
#include "ww8par.hxx"
#include "ww8par2.hxx"

#ifndef _SV_FONTTYPE_HXX //autogen
#include <vcl/fonttype.hxx>
#endif
#ifndef _SV_FONT_HXX //autogen
#include <vcl/font.hxx>
#endif


#define DELETEAZ( p ) { delete[]( p ); p = 0; };




//-----------------------------------------
//              Tabellen
//-----------------------------------------


#define MAX_COL 64  // WW6-Beschreibung: 32, WW6-UI: 31
                                        // WW8-UI: 63!

struct WW8SelBoxInfo: public SwSelBoxes_SAR
{
    short nGroupXStart;
    short nGroupWidth;

    WW8SelBoxInfo( short nXCenter, short nWidth ): nGroupXStart( nXCenter ), nGroupWidth( nWidth ){}
};

typedef WW8SelBoxInfo* WW8SelBoxInfoPtr;

SV_DECL_PTRARR_DEL(WW8MergeGroups, WW8SelBoxInfoPtr, 16,16);
SV_IMPL_PTRARR(    WW8MergeGroups, WW8SelBoxInfoPtr       );

struct WW8TabBandDesc{
    WW8TabBandDesc* pNextBand;
    short nGapHalf;
    short nLineHeight;
    short nRows;
    short nCenter[MAX_COL + 1]; // X-Rand aller Zellen dieses Bandes
    short nWidth[MAX_COL + 1];  // Laenge aller Zellen dieses Bandes
    short nWwCols;      // BYTE wuerde reichen, alignment -> short
    short nSwCols;      // SW: so viele Spalten fuer den Writer
    BOOL bLEmptyCol;    // SW: Links eine leere Zusatz-Spalte
    BOOL bREmptyCol;    // SW: dito rechts
    WW8_TCell* pTCs;
    WW8_SHD* pSHDs;


    // nur fuer WW6-7: diese Zelle hat WW-Flag bMerged (horizontal) gesetzt
    //BOOL bWWMergedVer6[MAX_COL];


    BOOL bExist[MAX_COL];           // Existiert diese Zelle ?
    UINT8 nTransCell[MAX_COL + 2];  // UEbersetzung WW-Index -> SW-Index

    WW8TabBandDesc() {  memset( this, 0, sizeof( *this ) ); };
    WW8TabBandDesc( WW8TabBandDesc& rBand );    // tief kopieren
    ~WW8TabBandDesc() { delete[]( pTCs ); pTCs = 0; delete[]( pSHDs ); };
    void ReadDef( BOOL bVer67, BYTE* pS );
    void ProcessSprmTSetBRC( BOOL bVer67, BYTE* pParamsTSetBRC );
    void ProcessSprmTDxaCol( BOOL bVer67, BYTE* pParamsTDxaCol );
    void ProcessSprmTDelete( BOOL bVer67, BYTE* pParamsTDelete );
    void ReadShd( SVBT16* pS );
};

class WW8TabDesc
{
    WW8_BRC aDefBrcs[6];

    SvStringsDtor aNumRuleNames;

    SwWW8ImplReader* pIo;

    WW8TabBandDesc* pFirstBand;
    WW8TabBandDesc* pActBand;

    SwPosition* pTmpPos;

    const SwTable* pTable;          // Tabelle
    SwTableNode* pTblNd;            // Tabellen-Node
    const SwTableLines* pTabLines;  // Zeilen-Array davon
    SwTableLine* pTabLine;          // akt. Zeile
    SwTableBoxes* pTabBoxes;        // Boxen-Array in akt. Zeile
    SwTableBox* pTabBox;            // akt. Zelle

    WW8MergeGroups* pMergeGroups;   // Listen aller zu verknuepfenden Zellen

    WW8_TCell* pAktWWCell;

    short nRows;
    short nDefaultSwCols;
    short nBands;
    short nMinLeft;
    short nMaxRight;
    short nSwWidth;

    BOOL bOk;
    BOOL bHeader;
    BOOL bBorderDefaults;
    BOOL bClaimLineFmt;
    SwHoriOrient eOri;
                                // 2. allgemeine Verwaltungsinfo
    short nAktRow;
    short nAktBandRow;          // SW: in dieser Zeile des akt. Bandes bin ich
                                // 3. Verwaltungsinfo fuer Writer
    short nAktCol;


    // 4. Methoden

    USHORT GetLogicalWWCol() const;
    void SetTabBorders( SwTableBox* pBox, short nIdx );
    void SetTabShades( SwTableBox* pBox, short nWwIdx );
    void CalcDefaults();
    BOOL SetPamInCell( short nWwCol, BOOL bPam, BOOL bCheckForMerge );
    void DeleteCells( short nDelete );
    void InsertCells( short nIns );
    void AdjustNewBand( SwWW8ImplReader* pReader );

    // durchsucht pMergeGroups, meldet Index der ersten, passenden Gruppe bzw. -1
    // Details siehe bei der Implementierung
    BOOL FindMergeGroup( short nX1, short nWidth, BOOL bExact, short& nMGrIdx );

    // einzelne Box ggfs. in eine Merge-Gruppe aufnehmen
    // (die Merge-Gruppen werden dann spaeter auf einen Schlag abgearbeitet)
    SwTableBox* UpdateTableMergeGroup(  WW8_TCell&     rCell,
                                                                            WW8SelBoxInfo* pActGroup,
                                                                            SwTableBox*    pActBox,
                                                                            USHORT         nCol );
public:
    BOOL IsValidCell( short nCol ) const;

    WW8TabDesc( SwWW8ImplReader* pIoClass );
    BOOL Ok() const { return bOk; }
    void CreateSwTable();
    void TableCellEnd();
    void FinishSwTable();
    short GetMinLeft() const { return nMinLeft; }
    ~WW8TabDesc();

    const WW8_TCell* GetAktWWCell() const { return pAktWWCell; }
    const short GetAktCol() const { return nAktCol; }
    const String& GetNumRuleName() const;  // find name of numrule valid for current WW-COL
    void SetNumRuleName( const String& rName );
};






//---------------------------------------------------------------------
//   Hilfroutinen fuer Kapitelnummerierung und Aufzaehlung / Gliederung
//---------------------------------------------------------------------

static void SetBaseAnlv( SwNumFmt* pNum, WW8_ANLV* pAV )
{
    static SvxExtNumType eNumA[8] = { SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC };

    static SvxAdjust eAdjA[4] = { SVX_ADJUST_LEFT,
        SVX_ADJUST_RIGHT, SVX_ADJUST_LEFT, SVX_ADJUST_LEFT };
//          eigentlich folgende 2, aber Writer-UI bietet es nicht an
//      SVX_ADJUST_CENTER, SVX_ADJUST_BLOCKLINE };

    pNum->eType = ( SVBT8ToByte( pAV->nfc ) < 8 ) ?
                    eNumA[SVBT8ToByte( pAV->nfc ) ] : SVX_NUM_NUMBER_NONE;
//  pNum->bInclUpperLevel = pAV->fPrev;
    pNum->SetInclUpperLevel( ( SVBT8ToByte( pAV->aBits1 ) & 0x4 ) >> 2 );
    pNum->SetStartValue( (USHORT)SVBT16ToShort( pAV->iStartAt ) );
//  pNum->eNumAdjust = eAdjA[pAV->jc];
    pNum->SetAdjust( eAdjA[SVBT8ToByte( pAV->aBits1 ) & 0x3] );

    pNum->SetCharTextOffset( (USHORT)SVBT16ToShort( pAV->dxaSpace ) );
    INT16 nIndent = Abs((INT16)SVBT16ToShort( pAV->dxaIndent ));
    if( SVBT8ToByte( pAV->aBits1 ) & 0x08 ){    // fHang
        pNum->SetFirstLineOffset( -nIndent );
        pNum->SetLSpace( nIndent );
        pNum->SetAbsLSpace( nIndent );
    }else{
        pNum->SetCharTextOffset( nIndent );     // Breite der Nummer fehlt
    }
    if( SVBT8ToByte( pAV->nfc ) == 5 || SVBT8ToByte( pAV->nfc ) == 7 )
    {
        String sP( pNum->GetPostfix() );
        sP.Insert( '.', 0 );
        pNum->SetPostfix( sP ); // Ordinalzahlen
    }
}

void SwWW8ImplReader::SetAnlvStrings( SwNumFmt* pNum, WW8_ANLV* pAV, BYTE* pTxt,
                            BOOL bOutline )
{
    BOOL bInsert = FALSE;                       // Default
    CharSet eCharSet = eStructCharSet;

    const WW8_FFN* pF = pFonts->GetFont( SVBT16ToShort( pAV->ftc ) ); // FontInfo
    BOOL bSymbol = pF && ( pF->chs == 2 );      // Symbol/WingDings/...

    String sTxt( (sal_Char*)pTxt,  SVBT8ToByte( pAV->cbTextBefore )
                                 + SVBT8ToByte( pAV->cbTextAfter  ), eCharSet );

    if( bOutline )
    {                             // Gliederung
        if( !pNum->IsInclUpperLevel()           // es sind  <= 1 Nummern anzuzeigen
            || pNum->eType == SVX_NUM_NUMBER_NONE ){    // oder dieser Level hat keine
                                                // eigenen Ziffern
            bInsert = TRUE;                     // -> dann uebernehme Zeichen

            // replace by simple Bullet ?
            if( bSymbol )
                //JP 14.08.96: cBulletChar benutzen, damit auf dem MAC
                //              richtig gemappt wird
                sTxt.Fill(  SVBT8ToByte( pAV->cbTextBefore )
                          + SVBT8ToByte( pAV->cbTextAfter  ), cBulletChar );
            }
    }
    else
    {                                       // Nummerierung / Aufzaehlung
        bInsert = TRUE;
//      if( SVBT16ToShort( pAV->ftc ) == 1
//          || SVBT16ToShort( pAV->ftc ) == 3 ){    // Symbol / WingDings
        if( bSymbol )
        {
            FontFamily eFamily;
            String aName;
            FontPitch ePitch;

            if( GetFontParams( SVBT16ToShort( pAV->ftc ), eFamily, aName,
                                ePitch, eCharSet ) ){
//              USHORT nSiz = ( SVBT16ToShort( pAV->hps ) ) ?
//                          SVBT16ToShort( pAV->hps ) : 24; // Groesse in 1/2 Pt
//                      darf nach JP nicht gesetzt werden, da immer die Size
//                      genommen wird, die am ZeilenAnfang benutzt wird
                Font aFont;
                aFont.SetName( aName );
                aFont.SetFamily( eFamily );
//              aFont.SetPitch( ePitch );       // darf nach JP nicht
                aFont.SetCharSet( eCharSet );
                pNum->eType = SVX_NUM_CHAR_SPECIAL;
//              if( pAV->ico )      // geht in UI und SWG-Writer/Reader nicht
//                  aFont.SetColor( Color( GetCol( pAV->ico ) ) );
                pNum->SetBulletFont( &aFont );

                // take only the very first character
                if( pAV->cbTextBefore || pAV->cbTextAfter)
                    pNum->SetBulletChar( sTxt.GetChar( 0 ) );
                else
                    pNum->SetBulletChar( 0xF000 + 183 );
            }
        }
    }
    if( bInsert )
    {
        if( pAV->cbTextBefore )
        {
            String sP( sTxt.Copy( 0, SVBT8ToByte( pAV->cbTextBefore ) ) );
            pNum->SetPrefix( sP );
        }
        if( SVBT8ToByte( pAV->cbTextAfter ) )
        {
            String sP( pNum->GetPostfix() );
            sP.Insert( sTxt.Copy( SVBT8ToByte( pAV->cbTextBefore ),
                                  SVBT8ToByte( pAV->cbTextAfter  ) ) );
            pNum->SetPostfix( sP );
        }
// Die Zeichen vor und hinter mehreren Ziffern koennen leider nicht uebernommen
// werden, da sie der Writer ganz anders behandelt und das Ergebnis i.A.
// schlechter als ohne waere.
    }
}

// SetAnld bekommt einen WW-ANLD-Descriptor und einen Level und modifiziert
// die durch pNumR anggebeben NumRules. Wird benutzt fuer alles ausser
// Gliederung im Text
void SwWW8ImplReader::SetAnld( SwNumRule* pNumR, WW8_ANLD* pAD, BYTE nSwLevel,
                       BOOL bOutLine )
{
    SwNumFmt aNF;
    if( pAD )
    {                                   // Es gibt einen Anld-Sprm
        bAktAND_fNumberAcross = 0 != SVBT8ToByte( pAD->fNumberAcross );
        WW8_ANLV* pAV = &pAD->eAnlv;
        SetBaseAnlv( &aNF, pAV );       // Setze Basis-Format
        SetAnlvStrings( &aNF, pAV, pAD->rgchAnld, bOutLine );// und Rest
    }
    pNumR->Set( nSwLevel, aNF );
}



//-------------------------------------------------------
//          Kapitelnummerierung und Kapitelbullets
//-------------------------------------------------------
// Kapitelnummerierung findet in Styledefinionen statt. Sprm 13 gibt den Level
// an, Sprm 12 den Inhalt

SwNumRule* SwWW8ImplReader::GetStyRule()
{
    if( pStyles->pStyRule )         // Bullet-Style bereits vorhanden
        return pStyles->pStyRule;

    const String aBaseName(WW8_ASCII2STR( "WW8StyleNum" ));
    const String aName( rDoc.GetUniqueNumRuleName( &aBaseName, FALSE ) );

    USHORT nRul = rDoc.MakeNumRule( aName );
    pStyles->pStyRule = rDoc.GetNumRuleTbl()[nRul];
    pStyles->pStyRule->SetAutoRule( FALSE );// Auto == FALSE -> Nummerierungsvorlage

    return pStyles->pStyRule;
}

void SwWW8ImplReader::Read_ANLevelNo( USHORT, BYTE* pData, short nLen ) // Sprm 13
{
    nSwNumLevel = 0xff; // Default: ungueltig

    if( nLen <= 0 )
        return;

    // StyleDef ?
    if( pAktColl )
    {
        if(      pCollA[nAktColl].bColl     // nur fuer SwTxtFmtColl, nicht CharFmt
                && *pData                                       // WW: 0 = no Numbering
                && !( nIniFlags & WW8FL_NO_OUTLINE ) )
        {
        if(      *pData <= MAXLEVEL     // Bereich WW:1..9 -> SW:0..8
                    && *pData <= 9 )                // keine Aufzaehlung / Nummerierung
            {
                nSwNumLevel = *pData - 1;
                if( !bNoAttrImport )
                    ((SwTxtFmtColl*)pAktColl)->SetOutlineLevel( nSwNumLevel );
                    // Bei WW-NoNumbering koennte auch NO_NUMBERING gesetzt
                    // werden. ( Bei normaler Nummerierung muss NO_NUM gesetzt
                    // werden: NO_NUM : Nummerierungs-Pause,
                    // NO_NUMBERING : ueberhaupt keine Nummerierung )

            }
            else if( *pData == 10 || *pData == 11 )
            {
                pStyles->nWwNumLevel = *pData;      // Typ merken, der Rest
                                                                                    // geschieht bei Sprm 12
            }
        }
    }
    else
    {                               // !StyleDef
        if( !bAnl && !( nIniFlags & WW8FL_NO_NUMRULE ) )
        {
            StartAnl( pData );      // Anfang der Gliederung / Aufzaehlung
        }
    }
}

void SwWW8ImplReader::Read_ANLevelDesc( USHORT, BYTE* pData, short nLen ) // Sprm 12
{
    if( !pAktColl || nLen <= 0                  // nur bei Styledef
        || !pCollA[nAktColl].bColl              // CharFmt -> ignorieren
        || ( nIniFlags & WW8FL_NO_OUTLINE ) ){
        nSwNumLevel = 0xff;
        return;
    }
    if( nSwNumLevel <= MAXLEVEL         // Bereich WW:1..9 -> SW:0..8
        && nSwNumLevel <= 9 ){      // keine Aufzaehlung / Nummerierung

                                        // Falls bereits direkt oder durch
                                        // Vererbung NumruleItems gesetzt sind,
                                        // dann jetzt ausschalten #56163
        pAktColl->SetAttr( SwNumRuleItem() );

        String aName(WW8_ASCII2STR( "Outline" ));
        SwNumRule aNR( rDoc.GetUniqueNumRuleName(&aName), OUTLINE_RULE );
        aNR = *rDoc.GetOutlineNumRule();

        SetAnld( &aNR, (WW8_ANLD*)pData, nSwNumLevel, TRUE );

            // fehlende Level muessen nicht aufgefuellt werden

        rDoc.SetOutlineNumRule( aNR );
    }else if( pStyles->nWwNumLevel == 10 || pStyles->nWwNumLevel == 11 ){
        SwNumRule* pNR = GetStyRule();
        SetAnld( pNR, (WW8_ANLD*)pData, 0, FALSE );
        pAktColl->SetAttr( SwNumRuleItem( pNR->GetName() ) );
        pCollA[nAktColl].bHasStyNumRule = TRUE;
    }
}

//-----------------------------------------
//      Nummerierung / Aufzaehlung
//-----------------------------------------

// SetNumOlst() traegt die Numrules fuer diese Zeile ins SwNumFmt ein
// ( nur fuer Gliederungen im Text; Aufzaehlungen / Nummerierungen laufen
// ueber ANLDs )
// dabei wird die Info aus dem OLST geholt und nicht aus dem ANLD ( s.u. )
void SwWW8ImplReader::SetNumOlst( SwNumRule* pNumR, WW8_OLST* pO, BYTE nSwLevel )
{
    SwNumFmt aNF;
    WW8_ANLV* pAV = &pO->rganlv[nSwLevel];
    SetBaseAnlv( &aNF, pAV );               // Setze Basis-Format
                                            // ... und nun die Strings
    int i, nTxtOfs = 0;
    register WW8_ANLV* pAV1;                 // suche String-Positionen
    for( i = 0, pAV1 = pO->rganlv; i < nSwLevel; i++, pAV1++ ){
        nTxtOfs += SVBT8ToByte( pAV1->cbTextBefore )
                    + SVBT8ToByte( pAV1->cbTextAfter );
    }

    SetAnlvStrings( &aNF, pAV, pO->rgch + nTxtOfs, TRUE ); // und rein
    pNumR->Set( nSwLevel, aNF );
}




// der OLST kommt am Anfang jeder Section, die Gliederungen enthaelt. Die ANLDs,
// die an jeder Gliederungszeile haengen, enthalten nur Stuss, also werden die
// OLSTs waehrend der Section gemerkt, damit die Informationen beim Auftreten
// von Gliederungsabsaetzen zugreifbar ist.
void SwWW8ImplReader::Read_OLST( USHORT, BYTE* pData, short nLen )
{
    if( nIniFlags & WW8FL_NO_NUMRULE )
        return;
    if( nLen <= 0 ){
        DELETEZ( pNumOlst );
        return;
    }
    if( pNumOlst )
        delete( pNumOlst );                 // nur sicherheitshalber
    pNumOlst = new WW8_OLST;
    if( nLen < sizeof( WW8_OLST ) )         // auffuellen, falls zu kurz
        memset( pNumOlst, 0, sizeof( *pNumOlst ) );
    *pNumOlst = *(WW8_OLST*)pData;
}

// StartAnl wird am Anfang eines Zeilenbereichs gerufen,
//  der Gliederung / Nummerierung / Aufzaehlung enthaelt
#define MAX_ANLV_NUM 12
void SwWW8ImplReader::StartAnl( BYTE* pSprm13 )
{
    bAktAND_fNumberAcross = FALSE;

    BYTE nT = GetNumType( *pSprm13 );
    if( nT == WW8_Pause || nT == WW8_None )
        return;

    nWwNumType = nT;

    // check for COL numbering:
    BYTE* pS12 = 0;// sprmAnld
    String sNumRule;
    if( pTableDesc )
    {
        sNumRule = pTableDesc->GetNumRuleName();
        if( sNumRule.Len() )
        {
            pNumRule = rDoc.FindNumRulePtr( sNumRule );
            if( !pNumRule )
                sNumRule.Erase();
            else
            {
                // this is ROW numbering ?
                pS12 = pPlcxMan->HasParaSprm( 12 ); // sprmAnld
                if( pS12 && 0 != SVBT8ToByte( ((WW8_ANLD*)pS12)->fNumberAcross) )
                    sNumRule.Erase();
            }
        }
    }
    if( !sNumRule.Len() )
    {
        //sNumRule = rDoc.GetUniqueNumRuleName();
        pNumRule = rDoc.GetNumRuleTbl()[ rDoc.MakeNumRule( sNumRule ) ];
        if( pTableDesc )
        {
            if( !pS12 )
                pS12 = pPlcxMan->HasParaSprm( 12 ); // sprmAnld
            if( !pS12 || !SVBT8ToByte( ((WW8_ANLD*)pS12)->fNumberAcross ) )
                pTableDesc->SetNumRuleName( pNumRule->GetName() );
        }
    }

    bAnl = TRUE;

    NextAnlLine( pSprm13, pS12 );                       // Einstellungen fuer 1. Zeile

                                                // NumRules ueber Stack setzen
    pCtrlStck->NewAttr( *pPaM->GetPoint(),
            SfxStringItem( RES_FLTR_NUMRULE, pNumRule->GetName() ) );
}


// NextAnlLine() wird fuer jede Zeile einer
// Gliederung / Nummerierung / Aufzaehlung einmal gerufen
void SwWW8ImplReader::NextAnlLine( BYTE* pSprm13, BYTE* pS12 )
{
    if( !bAnl )
        return;         // pNd->UpdateNum ohne Regelwerk gibt GPF spaetestens
                        // beim Speichern als sdw3

    if( *pSprm13 == 10 || *pSprm13 == 11 )      // WW:10 = Nummerierung -> SW:0
                                                // WW:11 = Auffzaehlung -> SW:0
    {
        nSwNumLevel = 0;
        if( !pNumRule->GetNumFmt( nSwNumLevel ) )
        {                                               // noch nicht definiert
            if( !pS12 )
                pS12 = pPlcxMan->HasParaSprm( 12 ); // sprmAnld o. 0
            SetAnld( pNumRule, (WW8_ANLD*)pS12, nSwNumLevel, FALSE );
        }
    }
    else if( *pSprm13 <= MAXLEVEL )             // Bereich WW:1..9 -> SW:0..8
    {
        nSwNumLevel = *pSprm13 - 1;             // Gliederung
        if( !pNumRule->GetNumFmt( nSwNumLevel ) )           // noch nicht definiert
        {
            if( pNumOlst )                      // es gab ein OLST
            {
                SetNumOlst( pNumRule, pNumOlst , nSwNumLevel );
            }
            else                                // kein Olst, nimm Anld
            {
                if( !pS12 )
                    pS12 = pPlcxMan->HasParaSprm( 12 ); // sprmAnld
                SetAnld( pNumRule, (WW8_ANLD*)pS12, nSwNumLevel, FALSE );
            }
        }
    }
    else
    {
        nSwNumLevel = 0xff;                 // keine Nummer
    }
    BYTE nLevel = ( nSwNumLevel < MAXLEVEL ) ? nSwNumLevel : NO_NUMLEVEL;
    SwNodeNum aNum( nLevel );
    SwTxtNode* pNd = pPaM->GetNode()->GetTxtNode();
    pNd->UpdateNum( aNum );
}

// StopAnl() wird am Ende des Zeilenbereiches gerufen
void SwWW8ImplReader::StopAnl( BOOL bGoBack )
{
    if( bGoBack )
    {
        SwPosition aTmpPos( *pPaM->GetPoint() );    // eine Zeile zu spaet
        // gehe zurueck
        pPaM->Move( fnMoveBackward, fnGoCntnt );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_FLTR_NUMRULE ); // Setze NumRules in Stack
        // restore Pam
        *pPaM->GetPoint() = aTmpPos;
    }
    else
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_FLTR_NUMRULE ); // Setze NumRules in Stack

    nSwNumLevel = 0xff;
    nWwNumType = WW8_None;
    bAnl = FALSE;
}


// Bei der Zaehlung der Tabellenzellen gibt es einige Merkwuerdigkeiten:
// 1) Ist in einer Zeile eine besondere Zellenumrandung vorhanden, dann werden
//    die WW8_TCell's vom WW generiert, sonst nicht.
// 2) Existieren die TCs, dann ist bei nicht existenten Zellen fMerged gesetzt.
//    Im Text existieren diese Zellen nicht, d.h. die Indices muessen nicht
//    korrigiert werden. ( Im sprmTDefTable sind die nicht existenten Zellen
//    jedoch vorhanden ). Fuer diese Zellen ist bExist[] TRUE, da keine
//    Anpassung der Indices beim Einlesen der Inhalte passieren muss.
// 3) Existieren die TCs einer Zeile nicht, dann werden nicht existente Zellen
//    als Zellen mit 0-Breite dargestellt, die auch jeweils einen leeren Absatz
//    beinhalten. Werden diese Zellen mit Breite 0 nicht importiert ( was sehr
//    empfehlenswert ist ), dann muessen die ZellenNummern korrigiert werden.
//    Der Index kann ueber nTransCell[] nachgesehen werden. Fuer diese Zellen
//    ist bExist[] FALSE.
// 4) Da WW Zellen mit ausgefransten Raendern darstellen kann, der SW aber
//    nicht, werden hier ggfs. leere Zellen links und rechts angefuegt. Auch
//    hier muessen die Zellennummern korrigiert werden.
//
// -> bExist[] enthaelt Info darueber, ob im WW vorhandene Zellen im SW fehlen
//    ( Fall 3 )
// -> nTransCell[] ist die UEbersetzung von WW-Indices auf SW-Indizes
//    ( unterschiedlich bei Fall 3 und 4 )

static BOOL IsEqual( WW8TabBandDesc* p1, WW8TabBandDesc* p2 )
{
    if( !p1 || !p2 )
        return FALSE;
#if 0   // dieses waere der schnellste Vergleich, aber ich bin mir nicht sicher,
        // ob gewisse Systeme die struct-Elemente nicht umsortieren oder
        // Luecken lassen
    if( !memcmp( &p1->nGapHalf, &p2->nGapHalf,
        (BYTE*)&p1->pTCs - (BYTE*)&p1->nGapHalf ) )
        return FALSE;
#endif
    if( p1->nWwCols != p2->nWwCols )
        return FALSE;

    if( p1->nLineHeight != p2->nLineHeight )
        return FALSE;

    if( p1->nGapHalf != p2->nGapHalf )  // koennte auch mit Toleranz arbeiten ( noetig ? )
        return FALSE;

    for( int i = 0; i <= p1->nWwCols; i++ )     // nCols+1 Angaben
        if( p1->nCenter[i] != p2->nCenter[i] )  // Toleranz bei nCenter ?
            return FALSE;

    if( p1->pTCs || p2->pTCs ){         // mindestens einer mit TCs
        if( !p1->pTCs || !p2->pTCs )
            return FALSE;                   // einer ohne TCs

        if( memcmp( p1->pTCs, p2->pTCs, ( p1->nWwCols ) * sizeof( WW8_TCell ) ) )
            return FALSE;
    }
    if( p1->pSHDs || p2->pSHDs ){           // mindestens einer mit SHDs
        if( !p1->pSHDs || !p2->pSHDs )
            return FALSE;                   // einer ohne SHDs

        if( memcmp( p1->pSHDs, p2->pSHDs, ( p1->nWwCols ) * sizeof( WW8_SHD ) ) )
            return FALSE;
    }
    return TRUE;
}

WW8TabBandDesc::WW8TabBandDesc( WW8TabBandDesc& rBand )
{
    *this = rBand;
    if( rBand.pTCs ){
        pTCs = new WW8_TCell[nWwCols];
        memcpy( pTCs, rBand.pTCs, nWwCols * sizeof( WW8_TCell ) );
    }
    if( rBand.pSHDs ){
        pSHDs = new WW8_SHD[nWwCols];
        memcpy( pSHDs, rBand.pSHDs, nWwCols * sizeof( WW8_SHD ) );
    }
}

// ReadDef liest die Zellenpositionen und ggfs die Umrandungen eines Bandes ein
void WW8TabBandDesc::ReadDef( BOOL bVer67, BYTE* pS )
{
    int i;

    if( !bVer67 ) pS++;

    short nLen = (INT16)SVBT16ToShort( pS - 2 ); // nicht schoen
    BYTE nCols = *pS;                       // Anzahl der Zellen
    short nOldCols = nWwCols;

    if( nCols > MAX_COL )
        return;

    nWwCols = nCols;

    BYTE* pT = &pS[1];
    nLen --;
    for( i=0; i<=nCols; i++, pT+=2 ){
        nCenter[i] = (INT16)SVBT16ToShort( pT );    // X-Raender
    }
    nLen -= 2 * ( nCols + 1 );
    if( nCols != nOldCols ){            // andere Spaltenzahl
        DELETEAZ( pTCs );
        DELETEAZ( pSHDs );
    }

    short nFileCols = nLen / ( bVer67 ? 10 : 20 );      // wirklich abgespeichert


    if( !pTCs )
    {
        // lege leere TCs an
        pTCs = new WW8_TCell[nCols];
        memset( pTCs, 0, nCols * sizeof( WW8_TCell ) );
    }


    if( nFileCols )
    {
        // lies TCs ein

        /*
            Achtung: ab Ver8 ist ein reserve-ushort je TC eingefuegt und auch
                     der Border-Code ist doppelt so gross, daher ist hier
                             kein simples kopieren moeglich,
                             d.h.: pTCs[i] = *pTc;  geht leider nicht.
            ---
            Vorteil: Arbeitstruktur ist jetzt viel bequemer zu handhaben!
        */
        WW8_TCell* pAktTC  = pTCs;
        if( bVer67 )
        {
            WW8_TCellVer6* pTc = (WW8_TCellVer6*)pT;
            for( i=0; i<nCols; i++, ++pAktTC )
            {
                memset( pAktTC, 0, sizeof( WW8_TCell ) );
                if( i < nFileCols )
                {               // TC aus File ?
                    BYTE aBits1 = SVBT8ToByte( pTc->aBits1Ver6 );
                    pAktTC->bFirstMerged    = ( ( aBits1 & 0x01 ) != 0 );
                    pAktTC->bMerged     = ( ( aBits1 & 0x02 ) != 0 );
                    memcpy( pAktTC->rgbrc[ WW8_TOP      ].aBits1,
                                    pTc->rgbrcVer6[ WW8_TOP     ].aBits1, sizeof( SVBT16 ) );
                    memcpy( pAktTC->rgbrc[ WW8_LEFT     ].aBits1,
                                    pTc->rgbrcVer6[ WW8_LEFT    ].aBits1, sizeof( SVBT16 ) );
                    memcpy( pAktTC->rgbrc[ WW8_BOT      ].aBits1,
                                    pTc->rgbrcVer6[ WW8_BOT     ].aBits1, sizeof( SVBT16 ) );
                    memcpy( pAktTC->rgbrc[ WW8_RIGHT    ].aBits1,
                                    pTc->rgbrcVer6[ WW8_RIGHT   ].aBits1, sizeof( SVBT16 ) );
                    if(    ( pAktTC->bMerged )
                            && ( i > 0             ) )
                    {
                        // Cell gemerged -> merken
                        //bWWMergedVer6[i] = TRUE;
                        memcpy( pTCs[i-1].rgbrc[ WW8_RIGHT ].aBits1,
                                pTc->rgbrcVer6[  WW8_RIGHT ].aBits1, sizeof( SVBT16 ) );
                            // right Border in vorige Zelle uebernehmen
                            // Hier darf bExist nicht auf FALSE gesetzt werden, da WW
                            // in den Textboxen diese Zellen nicht mitzaehlt....
                    }
                    if( i+1 < nFileCols )
                        pTc++;
                }
            }
        }
        else
        {
            WW8_TCellVer8* pTc = (WW8_TCellVer8*)pT;
            for( i=0; i<nCols; i++, ++pAktTC )
            {
                memset( pAktTC, 0, sizeof( WW8_TCell ) );
                if( i < nFileCols )
                {               // TC aus File ?
                    UINT16 aBits1 = (UINT16)SVBT16ToShort( pTc->aBits1Ver8 );
                    pAktTC->bFirstMerged    = ( ( aBits1 & 0x0001 ) != 0 );
                    pAktTC->bMerged         = ( ( aBits1 & 0x0002 ) != 0 );
                    pAktTC->bVertical       = ( ( aBits1 & 0x0004 ) != 0 );
                    pAktTC->bBackward       = ( ( aBits1 & 0x0008 ) != 0 );
                    pAktTC->bRotateFont     = ( ( aBits1 & 0x0010 ) != 0 );
                    pAktTC->bVertMerge      = ( ( aBits1 & 0x0020 ) != 0 );
                    pAktTC->bVertRestart    = ( ( aBits1 & 0x0040 ) != 0 );
                    pAktTC->nVertAlign      = ( ( aBits1 & 0x0180 ) >> 7 );
                    // am Rande: im aBits1 verstecken sich noch 7 Reserve-Bits,
                    //           anschliessend folgen noch 16 weitere Reserve-Bits

                    // In Version 8 koennen wir alle Bordercodes auf einmal kopieren!
                    memcpy( pAktTC->rgbrc, pTc->rgbrcVer8, 4 * sizeof( WW8_BRC ) );
                    // Zellen-Mergeinfo braucht hier nicht extra vermerkt zu werden,
                    // da dies ab Ver8 ja ueber Merge-Gruppen verwaltet wird.
                    if( i+1 < nFileCols )
                        pTc++;
                }
            }
        }
    }
}


void WW8TabBandDesc::ProcessSprmTSetBRC( BOOL bVer67, BYTE* pParamsTSetBRC )
{
    if( pParamsTSetBRC && pTCs ) // set one or more cell border(s)
    {
        BYTE nitcFirst= pParamsTSetBRC[0];// first col to be changed
        BYTE nitcLim  = pParamsTSetBRC[1];// (last col to be changed)+1
        BYTE nFlag    = *(pParamsTSetBRC+2);

        BOOL bChangeRight  = nFlag & 0x08;
        BOOL bChangeBottom = nFlag & 0x04;
        BOOL bChangeLeft   = nFlag & 0x02;
        BOOL bChangeTop    = nFlag & 0x01;

        WW8_TCell* pAktTC  = pTCs + nitcFirst;
        if( bVer67 )
        {
            WW8_BRCVer6* pBRC = (WW8_BRCVer6*)(pParamsTSetBRC+3);

            for( int i = nitcFirst; i < nitcLim; i++, ++pAktTC )
            {
                if( bChangeTop )
                    memcpy( pAktTC->rgbrc[ WW8_TOP  ].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                if( bChangeLeft )
                    memcpy( pAktTC->rgbrc[ WW8_LEFT ].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                if( bChangeBottom )
                    memcpy( pAktTC->rgbrc[ WW8_BOT  ].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                if( bChangeRight )
                    memcpy( pAktTC->rgbrc[ WW8_RIGHT].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
            }
        }
        else
        {
            WW8_BRC* pBRC = (WW8_BRC*)(pParamsTSetBRC+3);

            for( int i = nitcFirst; i < nitcLim; i++, ++pAktTC )
            {
                if( bChangeTop )
                    memcpy( pAktTC->rgbrc[ WW8_TOP  ].aBits1,
                            pBRC->aBits1,
                            sizeof( WW8_BRC ) );
                if( bChangeLeft )
                    memcpy( pAktTC->rgbrc[ WW8_LEFT ].aBits1,
                            pBRC->aBits1,
                            sizeof( WW8_BRC ) );
                if( bChangeBottom )
                    memcpy( pAktTC->rgbrc[ WW8_BOT  ].aBits1,
                            pBRC->aBits1,
                            sizeof( WW8_BRC ) );
                if( bChangeRight )
                    memcpy( pAktTC->rgbrc[ WW8_RIGHT].aBits1,
                            pBRC->aBits1,
                            sizeof( WW8_BRC ) );
            }



        }
    }
}


void WW8TabBandDesc::ProcessSprmTDxaCol( BOOL bVer67, BYTE* pParamsTDxaCol )
{
    // sprmTDxaCol (opcode 0x7623) changes the width of cells
    // whose index is within a certain range to be a certain value.

    if( nWwCols && pParamsTDxaCol ) // set one or more cell length(s)
    {
        BYTE nitcFirst= pParamsTDxaCol[0]; // first col to be changed
        BYTE nitcLim  = pParamsTDxaCol[1]; // (last col to be changed)+1
        short nDxaCol = (INT16)SVBT16ToShort( pParamsTDxaCol + 2 );
        short nOrgWidth;
        short nDelta;

        for( int i = nitcFirst; i < nitcLim, i < nWwCols; i++ )
        {
            nOrgWidth  = nCenter[i+1] - nCenter[i];
            nDelta     = nDxaCol - nOrgWidth;
            for( int j = i+1; j < nWwCols; j++ )
            {
                nCenter[j] += nDelta;
            }
        }
    }
}


void WW8TabBandDesc::ProcessSprmTDelete( BOOL bVer67, BYTE* pParamsTDelete )
{
    // sprmTDxaCol (opcode 0x7623) changes the width of cells
    // whose index is within a certain range to be a certain value.

    if( nWwCols && pParamsTDelete )        // set one or more cell length(s)
    {
        BYTE nitcFirst= pParamsTDelete[0]; // first col to be deleted
        BYTE nitcLim  = pParamsTDelete[1]; // (last col to be deleted)+1

        BYTE nShlCnt  = nWwCols - nitcLim; // count of cells to be shifted


        WW8_TCell* pAktTC  = pTCs + nitcFirst;
        int i = 0;
        for( ; i < nShlCnt; i++, ++pAktTC )
        {
            // adjust the left x-position
            nCenter[nitcFirst + i] = nCenter[nitcLim + i];

            // adjust the cell's borders
            *pAktTC = pTCs[ nitcLim + i];
        }
        // adjust the left x-position of the dummy at the very end
        nCenter[nitcFirst + i] = nCenter[nitcLim + i];

        nWwCols -= (nitcLim - nitcFirst);
    }
}

void SwWW8ImplReader::Read_TabCellDelete( USHORT, BYTE* pData, short nLen )
{
    // implementation missing: #76673#
    //
    // could we use part of ProcessSprmTDelete()  ???
}

void SwWW8ImplReader::Read_TabCellInsert( USHORT, BYTE* pData, short nLen )
{
    // implementation missing, analoguous to #76673#
}



// ReadShd liest ggfs die Hintergrundfarben einer Zeile ein.
// Es muss vorher ReadDef aufgerufen worden sein
void WW8TabBandDesc::ReadShd( SVBT16* pS )
{
    BYTE nLen = ( pS ) ? *( (BYTE*)pS - 1 ) : 0 ;
    if( !nLen )
        return;

#if 0
    if( !pS || *( pS - 1 ) < nWwCols * sizeof( WW8_SHD )  )
    {
        DELETEAZ( pSHDs );              // nicht vollstaendig -> ignorieren
        return;
    }
#endif

    if( !pSHDs )
    {
        pSHDs = new WW8_SHD[nWwCols];
        memset( pSHDs, 0, nWwCols * sizeof( WW8_SHD ) );
    }

    BYTE nAnz = nLen >> 1;
    if( nAnz > nWwCols )
        nAnz = (BYTE)nWwCols;
    int i;
    SVBT16* pShd;
    for( i=0, pShd = pS; i<nAnz; i++, pShd++ )
        pSHDs[i].SetWWValue( *pShd );

    //pSHDs[i]  = *pShd;
}



static BOOL SearchRowEnd( BOOL bVer67, BOOL bComplex, WW8PLCFx_Cp_FKP* pPap )
{
    BYTE* pB;

    while(     (    ( bVer67 && !bComplex )
                 || ( pPap->GetPCDIdx() < pPap->GetPCDIMax()) )
            && pPap->HasFkp() )
    {
        if( pPap->Where() != LONG_MAX )
        {
            pB = pPap->HasSprm( bVer67 ? 25 : 0x2417 );
            if(     pB
                && *pB == 1 )
                return TRUE;    // RowEnd found
        }
        (*pPap)++;
    }
    return FALSE;
}



WW8TabDesc::WW8TabDesc( SwWW8ImplReader* pIoClass )
    : pIo( pIoClass ),
    pFirstBand( 0 ),
    pActBand( 0 ),
    pTmpPos( 0 ),
    pTable( 0 ),
    pTblNd( 0 ),
    pTabLines( 0 ),
    pTabLine( 0 ),
    pTabBoxes( 0 ),
    pTabBox( 0 ),
    pMergeGroups( 0 ),
    pAktWWCell( 0 ),
    nRows( 0 ),
    nDefaultSwCols( 0 ),
    nBands( 0 ),
    nMinLeft( 0 ),
    nMaxRight( 0 ),
    nSwWidth( 0 ),
    bOk( TRUE ),
    bHeader( FALSE ),
    bBorderDefaults( FALSE ),
    bClaimLineFmt( FALSE ),
    eOri( HORI_NONE ),
    nAktRow( 0 ),
    nAktBandRow( 0 ),
    nAktCol( 0 )
{
    pIo->bAktAND_fNumberAcross = FALSE;

    static SwHoriOrient aOriArr[] = {
                        HORI_LEFT, HORI_CENTER, HORI_RIGHT, HORI_CENTER };

    BOOL bVer67   = pIo->bVer67;
    BOOL bComplex = pIo->pWwFib->fComplex;

    WW8TabBandDesc* pNewBand = new WW8TabBandDesc;


                            // Dies ist auch eine moegliche Erkennung
                            // fuer Dok-Start ( hat immer PageDesc ).
                            // Allerdings werden die PageDescs
                            // bei Bereichswechsel nicht erkannt
//  bStartDoc = ( pIo->pPaM->GetPoint()->nNode.GetIndex()
//               == pIo->rDoc.GetNodes().EndOfIcons.GetIndex() + 2 );

    WW8PLCFxSave1 aSave;
    pIoClass->pPlcxMan->GetPap()->Save( aSave );

    WW8PLCFx_Cp_FKP* pPap = pIoClass->pPlcxMan->GetPapPLCF();

    // process pPap until end of table found
    do{
        short nTabeDxaNew     = SHRT_MAX;
        eOri                  = HORI_LEFT;
        pNewBand->nGapHalf    = 0;
        pNewBand->nLineHeight = 0;
        BOOL bTabRowJustRead  = FALSE;
        BYTE* pShadeSprm      = 0;
        BYTE* pTabDxaColSprm  = 0;


        if( !SearchRowEnd( bVer67, bComplex, pPap ) )   // Suche Ende einer TabZeile
        {
            bOk = FALSE;
            break;
        }


        // Get the SPRM chains:
        // first from PAP and then from PCD (of the Piece Table)
        WW8PLCFxDesc aDesc;
        pPap->GetSprms( &aDesc );
        WW8SprmIter aSprmIter( aDesc.pMemPos, aDesc.nSprmsLen,
                               pIo->GetFib().nVersion );

        BYTE* pParams = aSprmIter.GetAktParams();
        for( int nLoop = 0; nLoop < 2; ++nLoop )
        {
            while(     aSprmIter.GetSprms()
                    && (0 != (pParams = aSprmIter.GetAktParams())) )
            {
                switch( aSprmIter.GetAktId() )
                {
                case 187:
                case 0xD605:
                    if( !nRows )
                    {
                        // sprmTTableBorders
                        if( bVer67 )
                            for( int i = 0; i < 6; ++i )
                            {
                                aDefBrcs[i].aBits1[0] = pParams[   2*i ];
                                aDefBrcs[i].aBits1[1] = pParams[ 1+2*i ];
                            }
                        else
                            memcpy( aDefBrcs, pParams, 24 ); // aDefBrcs = *(BRC(*)[6])pS;
                        bBorderDefaults = TRUE;
                    }
                    break;
                case 186:
                case 0x3404:
                    if( nRows == 0 )  // only 1. Line is Header
                    {
                        // sprmTTableHeader
                        bHeader = TRUE;
                    }
                    break;
                case 182:
                case 0x5400:
                    if( nRows == 0 )
                    {
                        // sprmTJc  -  Justification Code
                        eOri = aOriArr[ *pParams & 0x3 ];
                    }
                    break;
                case 184:
                case 0x9602:
                    {
                        // sprmTDxaGapHalf
                        pNewBand->nGapHalf = (INT16)SVBT16ToShort( pParams );
                    }
                    break;
                case 189:
                case 0x9407:
                    {
                        // sprmTDyaRowHeight
                        pNewBand->nLineHeight = (INT16)SVBT16ToShort( pParams ); // DyaRowHeight
                        bClaimLineFmt = TRUE;
                    }
                    break;
                case 190:
                case 0xD608:
                    {
                        // DefineTabRow
                        pNewBand->ReadDef( bVer67, pParams );
                        bTabRowJustRead = TRUE;
                    }
                    break;
                case 191:
                case 0xD609:
                    {
                        // TableShades
                        pShadeSprm = pParams;
                    }
                    break;
                //
                // * * * * Bug #69885#: the following codes were added * * * *
                //                      (khz, 1.Feb.2000)
                case 183:
                case 0x9601:
                    {
                        // sprmTDxaLeft
                        //
                        // our Writer cannot shift single table lines horizontally
                        // so we have to find the smallest parameter (meaning the
                        // left-most position) and then
                        // shift the whole table to that margin (see below)
                        short nDxaNew = (INT16)SVBT16ToShort( pParams );
                        if( nDxaNew < nTabeDxaNew )
                            nTabeDxaNew = nDxaNew;
                    }
                    break;
                case 193:
                case 0xD620:
                    {
                        // sprmTSetBrc
                        pNewBand->ProcessSprmTSetBRC( bVer67, pParams );
                    }
                    break;
                case 196:
                case 0x7623:
                    {
                        // sprmTDxaCol
                        pTabDxaColSprm = pParams;
                    }
                    break;
                /*              the following is processed by a separate methode
                case 195:       while reading the table's content...
                case 0x5622:
                    {
                        // sprmTDelete
                    }
                    break;
                */
                }
                aSprmIter++;
            }

            if( !nLoop )
            {
                pPap->GetPCDSprms(  aDesc );
                aSprmIter.SetSprms( aDesc.pMemPos, aDesc.nSprmsLen );
            }
        }


                            // #55171: WW-Tabellen koennen Fly-Wechsel
                            // beinhalten
                            // daher hier Tabellen abbrechen und neu beginnen
                            // noch steht *pPap noch vor TabRowEnd, daher
                            // kann TestApo() mit letztem Parameter FALSE
                            // und damit wirksam gerufen werden.


        if( bTabRowJustRead && pShadeSprm )
            pNewBand->ReadShd( (SVBT16*)pShadeSprm );

        if( nTabeDxaNew < SHRT_MAX )
        {
            short* pCenter  = pNewBand->nCenter;
            for( int i = 0; i < pNewBand->nWwCols; i++, ++pCenter )
                // Shift left x-position
                *pCenter += nTabeDxaNew;
        }

        if( pTabDxaColSprm )
            pNewBand->ProcessSprmTDxaCol( bVer67, pTabDxaColSprm );


     // Bei Unterschieden gibt es ein neues Band
     // Zweite Zeile bekommt immer ein neues Band, damit die erste allein ist
        if( nRows == 1 || !IsEqual( pNewBand, pActBand ) )
        {
            if( !pActBand )
            {
                pActBand = pFirstBand = pNewBand;
            }
            else
            {
                pActBand->pNextBand = pNewBand;
                pActBand = pNewBand;
            }
            pNewBand = new WW8TabBandDesc;
            nBands++;
        }
        else
        {
            delete pNewBand;
            pNewBand = new WW8TabBandDesc;
        }
        nRows++;
        pActBand->nRows++;

        (*pPap)++;
        if(    ( pPap->Where() == LONG_MAX )
            || (    ( !bVer67 || bComplex )
                 && ( pPap->GetPCDIdx() >= pPap->GetPCDIMax() ) ) )
        {
            bOk = FALSE;
            break;
        }

        // InTable
        if(     (0 == (pParams = pPap->HasSprm( bVer67 ? 24 : 0x2416 ) ) )
            ||  (1 != *pParams) )
            break;


        BOOL bStartApo, bStopApo, bNowStyleApo;
        pIoClass->TestApo( bStartApo, bStopApo,
                           bNowStyleApo, TRUE, FALSE, TRUE );
        if( bStopApo )  // Wenn Apo-Ende oder Apo-Wechsel, dann
            break;      // hoert die Tabelle hier auf

    }while( 1 );

    if( bOk )
    {
        if( pActBand->nRows > 1 )
        {   // Letztes Band hat mehr als 1 Zeile
            delete( pNewBand );
            pNewBand = new WW8TabBandDesc( *pActBand ); // neues machen
            pActBand->nRows--;      // wegen Sonderbehandlung Raender-Defaults
            pNewBand->nRows = 1;
            pActBand->pNextBand = pNewBand; // am Ende einschleifen
            nBands++;
            pNewBand = 0;                   // nicht loeschen
        }

        CalcDefaults();
    }
    delete( pNewBand );

    pIoClass->pPlcxMan->GetPap()->Restore( aSave );
}

WW8TabDesc::~WW8TabDesc()
{
    WW8TabBandDesc* pR = pFirstBand;
    WW8TabBandDesc* pR2;

    while( pR ){
        pR2 = pR->pNextBand;
        delete( pR );
        pR = pR2;
    }
    if( pMergeGroups ) DELETEZ( pMergeGroups );
}

void WW8TabDesc::CalcDefaults()
{
    short nMinCols = SHRT_MAX;
    register WW8TabBandDesc* pR;

    nMinLeft = SHRT_MAX;
    nMaxRight = SHRT_MIN;

                        // 1. Durchlauf: aeusserste L- und R-Grenzen finden
    for( pR = pFirstBand; pR; pR = pR->pNextBand ){
        if( pR->nCenter[0] < nMinLeft )
            nMinLeft = pR->nCenter[0];
        if( pR->nCenter[pR->nWwCols] > nMaxRight )
            nMaxRight = pR->nCenter[pR->nWwCols];
    }
    nSwWidth = nMaxRight - nMinLeft;
                        // 2. Durchlauf: Zahl der Writer-Spalten feststellen
                        // Die Zahl der Writer Spalten kann um bis zu 2 hoeher
                        // sein als im WW, da der SW im Gegensatz zu WW keine
                        // ausgefransten linken und rechten Raender kann und
                        // diese durch leere Boxen aufgefuellt werden.
                        // Durch nichtexistente Zellen koennen auch Zellen
                        // wegfallen

    for( pR = pFirstBand; pR; pR = pR->pNextBand ){
        pR->nSwCols = pR->nWwCols;
        pR->bLEmptyCol = pR->nCenter[0] - nMinLeft > MINLAY;
        pR->bREmptyCol = nMaxRight - pR->nCenter[pR->nWwCols] > MINLAY;

        short nAddCols = pR->bLEmptyCol + pR->bREmptyCol;
        USHORT i;
        USHORT j = ( pR->bLEmptyCol ) ? 1 : 0;
        for( i = 0; i < pR->nWwCols; i++ ){
            pR->nTransCell[i] = (INT8)j;
            if(    ( pR->nCenter[i] < pR->nCenter[i+1] )
                    //&& !pR->bWWMergedVer6[i]
                    )
            {       //  Zelle existiert wirklich
                pR->bExist[i] = TRUE;
                j++;
            }
            else
            {
                pR->bExist[i] = FALSE;
                nAddCols--;
            }
        }
        pR->nTransCell[i++] = (INT8)(j++);  // Wird u.a. wegen bREmptyCol um
        pR->nTransCell[i] = (INT8)j;        // max. 2 ueberindiziert

        pR->nSwCols += nAddCols;
        if( pR->nSwCols < nMinCols )
            nMinCols = pR->nSwCols;
    }
                        // 3. Durchlauf: Wo noetig die Umrandungen durch die
                        // Defaults ersetzen
    if( bBorderDefaults ){
        for( pR = pFirstBand ; pR; pR = pR->pNextBand ){
            if( !pR->pTCs ){
                pR->pTCs = new WW8_TCell[ pR->nWwCols ];
                memset( pR->pTCs, 0, pR->nWwCols * sizeof( WW8_TCell ) );
            }
            int k;
            for( k = 0; k < pR->nWwCols; k++ ){
                register WW8_TCell* pT = &pR->pTCs[k];
                int i, j;
                for( i = 0; i < 4; i ++ ){
#ifdef DEBUG
// nur fuer Ver67
                    USHORT nBBrc = SVBT16ToShort( pT->rgbrc[i].aBits1 );
                    USHORT nBWidth = nBBrc >> 0 & 0x7;
                    USHORT nBType = nBBrc >> 3 & 0x3;
                    USHORT nBShadow = nBBrc >> 5 & 0x1;
                    USHORT nBIco = nBBrc >> 6 & 0x1f;
                    USHORT nBSpace = nBBrc >> 11 & 0x1f;
#endif
                    if(   pIo->bVer67
                        ? (( SVBT16ToShort( pT->rgbrc[i].aBits1 ) >> 3 & 0x3 ) == 0)
                        : (( SVBT16ToShort( pT->rgbrc[i].aBits1 )   & 0xFF00 ) == 0) )
                    {   // brcType == 0 nichts explizit gesetzt
                        j = i;
                        switch( i )
                        {
                        case 0: j = (pR == pFirstBand    ) ? 0 : 4; // Aussen oben  / Innen waagerecht
                                break;
                        case 1: j = ( k                  ) ? 5 : 1; // Aussen links / Innen senkrecht
                                break;
                        case 2: j = ( pR->pNextBand      ) ? 4 : 2; // Aussen unten / Innen waagerecht
                                break;
                        case 3: j = (k == pR->nWwCols - 1) ? 3 : 5; // Aussen rechts/ Innen senkrecht
                                break;
                        }
#ifdef DEBUG
// nur fuer Ver67
                        USHORT nDBrc = SVBT16ToShort( aDefBrcs[j].aBits1 );
                        USHORT nDWidth = nDBrc >> 0 & 0x7;
                        USHORT nDType = nDBrc >> 3 & 0x3;
                        USHORT nDShadow = nDBrc >> 5 & 0x1;
                        USHORT nDIco = nDBrc >> 6 & 0x1f;
                        USHORT nDSpace = nDBrc >> 11 & 0x1f;
#endif
                        pT->rgbrc[i] = aDefBrcs[j]; // mangel mit Defaults ueber
                    }
                }
            }
        }
    }
    // 4. Ausrichtung der Tabelle
    long nMidTab  = ( (long)nMinLeft  + nMaxRight ) / 2; // TabellenMitte
    long nRight   = pIo->nPgWidth - pIo->nPgRight - pIo->nPgLeft;

     // set Position if not on adjusted to left border
    if((MINLAY < nMinLeft) && (HORI_LEFT == eOri))
    {
        if(MINLAY > abs(nMidTab - nRight/2))// very near the center IS centered
            eOri = HORI_CENTER;
        else
        if(MINLAY > abs(nMaxRight - nRight))// very near r.border IS r.adjusted
            eOri = HORI_RIGHT;
        else
            eOri = HORI_LEFT_AND_WIDTH;     //  absolutely positioned
    }

    nDefaultSwCols = nMinCols;  // da Zellen einfuegen billiger ist als Mergen
    if( nDefaultSwCols == 0 )
        bOk = FALSE;
    pActBand = pFirstBand;
    nAktBandRow = 0;
    ASSERT( pActBand, "pActBand ist 0" );
}

void WW8TabDesc::CreateSwTable()
{

    ::SetProgressState( pIo->nProgress, pIo->rDoc.GetDocShell() );   // Update
#ifdef DEBUG
//  Sound::Beep();
#endif

    // if there is already some content on the Node
    // append new node to ensure that this content remains
    // ABOVE the table
    SwPosition* pPoint = pIo->pPaM->GetPoint();
    BOOL bInsNode = ( pPoint->nContent.GetIndex() );
    if(    !bInsNode
        && pIo->pNode_FLY_AT_CNTNT == &pPoint->nNode.GetNode() )
    {
        bInsNode = TRUE;
        // minimize Fontsize to minimize height growth of the header/footer
        SvxFontHeightItem aSz( 20 );
        pIo->NewAttr( aSz );
        pIo->pCtrlStck->SetAttr( *pPoint, RES_CHRATR_FONTSIZE );
    }
    if( bInsNode )
    {
        // set font size to 1 point to minimize y-growth of Hd/Ft
        pIo->rDoc.AppendTxtNode( *pPoint );
    }


    pTmpPos = new SwPosition( *pIo->pPaM->GetPoint() );


    // Abfrage, ob im Node, in dem die Tabelle eingefuegt werden soll,
    // bereits ein Pagedesc steht. Dann wuerde der PageDesc in die
    // naechste Zeile hinter der Tabelle rutschen, wo er nichts zu
    // suchen hat.
    // -> loeschen und spaeter an das Tabellenformat setzen
    SfxPoolItem *pSetAttr = 0;
    SwTxtNode* pNd = pIo->rDoc.GetNodes()[pTmpPos->nNode]->GetTxtNode();
    if ( pNd ){
        const SfxItemSet* pSet = pNd->GetpSwAttrSet();
        const SfxPoolItem* pItem;
        if( pSet ){
            if( SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, FALSE, &pItem ))
            {
                pSetAttr = new SwFmtPageDesc( *(SwFmtPageDesc*)pItem );
                pNd->ResetAttr( RES_PAGEDESC );
            }
            if( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, FALSE, &pItem ) )
            {
                pSetAttr = new SvxFmtBreakItem( *(SvxFmtBreakItem*)pItem );
                pNd->ResetAttr( RES_BREAK );
            }
        }
    }


    // Die Tabelle ist beim Einfuegen noch recht klein:
    // Zahl der Spalten ist die kleinste Spaltenanzahl des Originals,
    // da Spalten einfuegen schneller geht als Loeschen
    // Zahl der Zeilen ist die Zahl der Baender, da sich die
    // (identischen) Zeilen eines Bandes prima duplizieren lassen
    pTable = pIo->rDoc.InsertTable( *pTmpPos, nBands, nDefaultSwCols, eOri );
    SwFrmFmt* pFrmFmt = pTable->GetFrmFmt();

    // evtl den PageDesc/Break jetzt an der Tabelle setzen
    if( pSetAttr )
    {
        pFrmFmt->SetAttr( *pSetAttr );
        delete pSetAttr;
    }

    if( nMaxRight - nMinLeft > MINLAY * nDefaultSwCols ) // Gesamtbreite der Tabelle
    {
        pFrmFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, nSwWidth ) );//FIX
    }

    if(HORI_LEFT_AND_WIDTH == eOri)
    {
        SvxLRSpaceItem aL;
        aL.SetLeft( nMinLeft );
        pFrmFmt->SetAttr( aL );
    }

    if( pIo->pSFlyPara ){   // Ist Tabelle im Rahmen ?
                            // ok, Rahmen auf Bildgroesse vergroessern
                            //  ( nur wenn Auto-Breite )
        pIo->pSFlyPara->BoxUpWidth( nSwWidth );
    }

    // globale Varis initialisieren
    pTabLines = &pTable->GetTabLines();
    nAktRow = nAktCol
            = nAktBandRow
            = 0;

    pTblNd  = (SwTableNode*)(*pTabLines)[0]->GetTabBoxes()[0]->GetSttNd()->FindTableNode();
    ASSERT( pTblNd, "wo ist mein TabellenNode" );
    pTblNd->GetTable().SetHeadlineRepeat( bHeader );

    // ggfs. Zusatz-Zellen einfuegen u.dgl.
    AdjustNewBand( pIo );

    // jetzt den PaM korrekt setzen und ggfs. erste Mergegruppe vorbereiten...
    SetPamInCell( nAktCol, TRUE, TRUE );

    pIo->bWasTabRowEnd = FALSE;
}

void WW8TabDesc::FinishSwTable()
{
    *pIo->pPaM->GetPoint() = *pTmpPos;
    delete( pTmpPos );
    pTmpPos = 0;
    pIo->bWasTabRowEnd = FALSE;
    if( pIo->rDoc.GetRootFrm() )
    {
        // exitiert schon ein Layout, dann muss an dieser Tabelle die
        // BoxFrames neu erzeugt werden.
        pTblNd->DelFrms();
        pTblNd->MakeFrms( &pIo->pPaM->GetPoint()->nNode );
    }
    // falls noetig, zu mergende Zellen gruppenweise zusammenfassen
    if( pMergeGroups )
    {
        // bearbeite alle Merge-Gruppen nacheinander
        SwTableBox*    pTargetBox;
        WW8SelBoxInfo* pActMGroup;
        USHORT         nActBoxCount;
        for( USHORT iGr = 0; iGr < pMergeGroups->Count(); iGr++ )
        {
            pActMGroup   = (*pMergeGroups)[ iGr ];
            // Anzahl der zu mergenden Boxen PLUS Eins (fuer die TargetBox)
            nActBoxCount = pActMGroup->Count();

            if( ( 1 < nActBoxCount ) && pActMGroup && (*pActMGroup)[ 0 ] )
            {
                // beachten: der 0. Eintrag ist die ebenso wie
                // die obere, linke Box formatierte Target-Box !!!
                pTargetBox = (*pActMGroup)[ 0 ];
                //
                // Target-Box verschoenern: unteren Rand der letzten Box nehmen
                //
                SwFrmFmt* pTargetFrmFmt = pTargetBox->GetFrmFmt();
                SvxBoxItem aBoxItem( pTargetFrmFmt->GetBox() );
                aBoxItem.SetLine(
                    (*pActMGroup)[ nActBoxCount-1 ]->GetFrmFmt()->GetBox().GetBottom(),
                    BOX_LINE_BOTTOM );
                pTargetFrmFmt->SetAttr( aBoxItem );

                // oeffne Merge-Array mit passender Groesse
                SwSelBoxes aBoxes( nActBoxCount-1 );

                // alle Boxen dieser Gruppe in aBoxes versammeln
                aBoxes.Insert( pActMGroup->GetData()+1, nActBoxCount-1 );

                // Sicherheitspruefung: Gruppe mit nur EINER Box fuehrt beim Mergen
                // zum Programmabsturz, deshalb diesen unsinnigen Fall abfangen!
                USHORT nMergeTest;
                if( 2 == nActBoxCount )
                    // nur 1 Zelle in der Merge-Gruppe: wahrscheinlich Word-Bug
                    nMergeTest = TBLMERGE_TOOCOMPLEX;
                else
                {   //
                    // Vorsicht: erstmal austesten,
                    // ob sich die aBoxes ueberhaupt mergen lassen wuerden...
                    //
                    nMergeTest = CheckMergeSel( aBoxes );   // siehe   /SW/INC/TBLSEL.HXX
                    BYTE  nTry = 1;
                    //
                    // Falls das Merge so nicht machbar ist,
                    // pruefen, ob es ginge, wenn wir die letzte Box
                    // oder die beiden letzten Boxen
                    // oder gar die drei letzten Boxen
                    // aus der Merge-Gruppe rausnehmen wuerden
                    while(     ( TBLMERGE_TOOCOMPLEX == nMergeTest )
                                    && ( 4 > nTry                          )
                                    && ( 3 < nActBoxCount /* wichtig ! */  ) )
                    {
                        nTry++;
                        nActBoxCount--;
                        // letzte Box aus der Gruppe rausnehmen
                        aBoxes.Remove( nActBoxCount-1 );
                        // und nachsehen, ob Merge nun moeglich waere
                        nMergeTest = CheckMergeSel( aBoxes );
                    }
                }
                //
                // Genug geprueft - wir schreiten zur Tat...
                //
                switch( nMergeTest )
                {
                case TBLMERGE_OK:
                    //
                    // You succeed at last...
                    //
                    ((SwTable*)pTable)->Merge( &pIo->rDoc, aBoxes, pTargetBox );
                    break;
                case TBLMERGE_NOSELECTION:
                    /*NOP*/     // war wohl nix?
                    break;
                case TBLMERGE_TOOCOMPLEX:
                    // Pech: Trotz aller Probiererei geht das Merge immer noch nicht!
                    //
                    // Beachten: die Daten der Gruppe schlummern bereits
                    // in der Target-Box. Da kein Merge moeglich ist,
                    // setzen wir die Target-Box in die Tabelle an
                    // die Stelle der oberen linken Box.
                    {
                        const SwTableBox* pBox  = (*pActMGroup)[ 1 ];
                        SwTableLine*      pLine = (SwTableLine*)pBox->GetUpper();

                        USHORT nPos = pLine->GetTabBoxes().GetPos( pBox );

                        ASSERT( USHRT_MAX != nPos, "GetPos fehlgeschlagen");

                        SwStartNode* pSttNd = (SwStartNode*)pBox->GetSttNd();

                        ASSERT( pSttNd, "Box ohne Start-Node ?!");

                        pTargetBox->ChgFrmFmt(
                            (SwTableBoxFmt*)pBox->GetFrmFmt() );
                        pTargetBox->SetUpper(  pLine );

                        // erst die Box loeschen!!
                        pLine->GetTabBoxes().DeleteAndDestroy( nPos );
                        // dann die pTargetBox einfuegen
                        pLine->GetTabBoxes().C40_INSERT( SwTableBox, pTargetBox, nPos );
                        // dann die Nodes loeschen!!
                        pIo->rDoc.DeleteSection( pSttNd );
                    }
                    break;
                default:            // was wollen wir denn hier ???
                    ASSERT( !this, "CheckMergeSel() with undefined return value" );
                    break;
                }
            }
        }
        DELETEZ( pMergeGroups );
    }
    // nun noch ggfs. die doppelten Innenraender korrigieren (Bug #53525#)
    if( pTable )
        ((SwTable*)pTable)->GCBorderLines();    // Garbage Collect
}


// durchsucht pMergeGroups, meldet Index der ersten, passenden Gruppe bzw. -1
//
// Parameter: nXcenter  = Mittenposition der anfragenden Box
//            nWidth    = Breite der anfragenden Box
//            bExact    = Flag, ob Box in dieser Gruppe passen muss,
//                          oder diese nur zu tangieren braucht
//
BOOL WW8TabDesc::FindMergeGroup( short nX1, short nWidth, BOOL bExact, short& nMGrIdx )
{
    nMGrIdx = -1;
    if( pMergeGroups )
    {
        // noch als gueltig angesehener Bereich in der Naehe der Grenzen
        const short nToleranz = 4;
        // die aktuell untersuchte Gruppe
        WW8SelBoxInfoPtr pActGroup;
        // Boxgrenzen
        short nX2 = nX1 + nWidth;
        // ungefaehre Gruppengrenzen
        short nGrX1;
        short nGrX2;

        for( USHORT iGr = 0; iGr < pMergeGroups->Count(); iGr++ )
        {
            // die aktuell untersuchte Gruppe
            pActGroup = (*pMergeGroups)[ iGr ];
            if( -999 < pActGroup->nGroupXStart  )
            {
                // ungefaehre Gruppengrenzen mit Toleranz nach *aussen* hin
                nGrX1 = pActGroup->nGroupXStart - nToleranz;
                nGrX2 = pActGroup->nGroupXStart
                             +pActGroup->nGroupWidth  + nToleranz;
                //
                // Falls Box reinpasst, melde auf jeden Fall den Erfolg
                //
                if( ( nX1 > nGrX1 ) && ( nX2 < nGrX2 ) )
                {
                    nMGrIdx = iGr;  break;
                }
                //
                // hat die Box Bereiche mit der Gruppe gemeinsam?
                //
                if( !bExact )
                {
                    // melde Erfolg, wenn nX1 *oder* nX2 innerhalb der Gruppe liegen
                    if(    (     ( nX1 > nGrX1 )
                                        && ( nX1 < nGrX2 - 2*nToleranz ) )
                            || (     ( nX2 > nGrX1 + 2*nToleranz )
                                        && ( nX2 < nGrX2 ) )
                            // oder nX1 und nX2 die Gruppe umfassen
                            || (     ( nX1 <=nGrX1 )
                                        && ( nX2 >=nGrX2 ) ) )
                    {
                        nMGrIdx = iGr;  break;
                    }
                }
            }
        }
    }
    return BOOL( -1 < nMGrIdx );
}


BOOL WW8TabDesc::IsValidCell( short nCol ) const
{
    return     pActBand->bExist[ nCol ]
           && (USHORT)nAktRow < pTabLines->Count();
}


BOOL WW8TabDesc::SetPamInCell( short nWwCol, BOOL bPam, BOOL bCheckForMerge )
{
    ASSERT( pActBand, "pActBand ist 0" );

    if( !IsValidCell( nWwCol ) )
        return FALSE;

    USHORT nCol = pActBand->nTransCell[nWwCol];

    pTabLine = (*pTabLines)[nAktRow];
    pTabBoxes = &pTabLine->GetTabBoxes();
    if ( nCol >= pTabBoxes->Count() )
        return FALSE;
    pTabBox = (*pTabBoxes)[nCol];
    if( !pTabBox->GetSttNd() )
    {
        ASSERT(pTabBox->GetSttNd(), "Probleme beim Aufbau der Tabelle");
        return FALSE;
    }
    if( bPam )
    {
        pAktWWCell = &pActBand->pTCs[ nWwCol ];
        pIo->pPaM->GetPoint()->nNode = pTabBox->GetSttIdx() + 1;
        pIo->pPaM->GetPoint()->nContent.Assign( pIo->pPaM->GetCntntNode(), 0 );

        // Zur Sicherheit schon jetzt setzen, da bei den Zellen, die
        // zum Randausgleich eingefuegt werden, sonst der Style
        // nicht gesetzt wird.
        pIo->rDoc.SetTxtFmtColl( *pIo->pPaM, (SwTxtFmtColl*)pIo->pDfltTxtFmtColl );
        // uebrigens: da diese Zellen unsichtbare Hilfskonstruktionen sind,
        //            und nur dazu dienen, zerfranste Aussehen der WW-Tabelle
        //            nachzuahmen, braucht NICHT SetTxtFmtCollAndListLevel()
        //            verwendet zu werden.
    }
    //
    // ggfs. aktuelle Box in entsprechende Merge-Gruppe eintragen
    //
    if( bCheckForMerge && pActBand->pTCs /*&& !pIo->bVer67*/ )
    {
        SwTableBox* pTargetBox;
        WW8SelBoxInfoPtr pActMGroup = 0;
        //
        // ggfs. eine neue Merge-Gruppe beginnen
        //
        WW8_TCell& rCell = pAktWWCell ? *pAktWWCell : pActBand->pTCs[ nWwCol ];
        // ist dies die obere, linke-Zelle einer Merge-Gruppe ?
        if( rCell.bFirstMerged || ( rCell.bVertRestart && !rCell.bMerged ) )
        {
            short nX1    = pActBand->nCenter[ nWwCol ];
            short nWidth = pActBand->nWidth[  nWwCol ];
            if( !pMergeGroups )
                // 0. falls noetig das Array fuer die Merge-Gruppen anlegen
                pMergeGroups = new WW8MergeGroups;
            else
            {
                // 1. ggfs. alte Mergegruppe(n) schliessen, die den von
                //    unserer neuen Gruppe betroffenen X-Bereich ueberdecken
                short nMGrIdx;
                while( FindMergeGroup( nX1, nWidth, FALSE, nMGrIdx ) )
                {
                    (*pMergeGroups)[ nMGrIdx ]->nGroupXStart = -999;
                }
            }

            // 2. aktuelle Merge-Gruppe anlegen
            pActMGroup = new WW8SelBoxInfo( nX1, nWidth );

            // 3. und in Gruppen-Array eintragen
            pMergeGroups->Insert( pActMGroup, pMergeGroups->Count() );

            // 4. Target-Box anlegen und als 0. in Merge-Group setzen
            pIo->rDoc.GetNodes().InsBoxen(pTblNd,
                                pTabLine,
                                (SwTableBoxFmt*)pTabBox->GetFrmFmt(),
                                (SwTxtFmtColl*)pIo->rDoc.GetDfltTxtFmtColl(),
                                0,
                                nCol );

            pTargetBox = (*pTabBoxes)[ nCol ];

            // eingefuegte Box wieder aus der Row entfernen
            // (wird vom Merge() dann endgueltig richtig eingebaut)
            pTabBoxes->Remove( nCol );

            // und ab damit in die Merge-Group
            pActMGroup->Insert( pTargetBox, pActMGroup->Count() );
            //
            // 5. Target-Box formatieren
            //
            pTargetBox->SetUpper( 0 );

            // eigenes Frame-Format fuer diese Box anlegen
            SwFrmFmt* pNewFrmFmt = pTargetBox->ClaimFrmFmt();

            // Border der O-L-Box der Gruppe wird Border der Targetbox
            pNewFrmFmt->SetAttr( pTabBox->GetFrmFmt()->GetBox() );

            // Gesamtbreite ermitteln und zuweisen
            short nSizCell = pActBand->nWidth[  nWwCol ];
            for(USHORT i = nWwCol+1; i < pActBand->nWwCols; i++ )
                if(     pActBand->pTCs[ i ].bMerged
                        && !pActBand->pTCs[ i ].bFirstMerged    )
                    nSizCell += pActBand->nWidth[ i ];
                else
                    break;
            pActMGroup->nGroupWidth = nSizCell;
            pNewFrmFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, pActMGroup->nGroupWidth ));
        }

        // ggfs. akt. Box zu einer Merge-Gruppe hinzufuegen
        // (dies kann eine soeben angelegte, oder eine andere Gruppe sein)
        pTargetBox = UpdateTableMergeGroup( rCell, pActMGroup, pTabBox, nWwCol );
        if( pTargetBox )
        {
            // den pCurPaM jetzt ans Ende der Target-Box setzen,
            // damit der Text direkt in die richtige Box gelesen wird.

            const SwNode* pEndNd =
                    pTargetBox->GetSttNd()->EndOfSectionNode();

            ASSERT(pEndNd, "Gruppen-TargetBox ohne Start-Node ?");

            if( bPam )
            {
                pIo->pPaM->GetPoint()->nNode = pEndNd->GetIndex();
                pIo->pPaM->Move( fnMoveBackward );
            }
        }
    }
    return TRUE;
}

#if 0               // wird fuer FastSave-Files noch gebraucht
void WW8TabDesc::DeleteCells( short nDelete )
{
    short nMergeCol = nDefaultSwCols - nDelete - 1;

    SwSelBoxes aBoxes;
    short i;

    for (i = nMergeCol; i < nMergeCol + nDelete + 1; i++ ){
        if( !SetPamInCell( i, FALSE, FALSE ) )
            return;
        _SwSelBox a_SelBox( pTabBox->GetSttIdx(), pTabBox );
        aBoxes.Insert( a_SelBox );
    }

    SetPamInCell( nMergeCol, FALSE, FALSE ); // Cursor u.AE. an alte Pos
    pIo->rDoc.GetNodes().InsBoxen( pTblNd, pTabLine, pTabBox->GetFrmFmt(),
                            (SwTxtFmtColl*)pIo->rDoc.GetDfltTxtFmtColl(),
                            0, nMergeCol );
    SwTableBox* pNewBox = (*pTabBoxes)[nMergeCol];
    pTabBoxes->Remove( nMergeCol );     // wieder austragen
    pNewBox->SetUpper( 0 );
    pNewBox->ClaimFrmFmt();

    pIo->pPaM->GetPoint()->nContent.Assign( 0, 0 ); // Melde ContentIndex pCurPaM ab

    ((SwTable*)pTable)->Merge( &pIo->rDoc, aBoxes, pNewBox );
}
#endif

void WW8TabDesc::InsertCells( short nIns )
{
        pTabLine = (*pTabLines)[nAktRow];
        pTabBoxes = &pTabLine->GetTabBoxes();
        pTabBox = (*pTabBoxes)[0];

    pIo->rDoc.GetNodes().InsBoxen( pTblNd, pTabLine, (SwTableBoxFmt*)pTabBox->GetFrmFmt(),
                            (SwTxtFmtColl*)pIo->pDfltTxtFmtColl, 0, pTabBoxes->Count(), nIns );
    // mit dem Dritten Parameter wird das FrmFmt der Boxen angegeben.
    // hier kann man auch noch optimieren, um FrmFmts zu sparen
}

void WW8TabDesc::SetTabBorders( SwTableBox* pBox, short nWwIdx )
{
    if( nWwIdx < 0 || nWwIdx >= pActBand->nWwCols )
        return;                 // kuenstlich erzeugte Zellen -> Kein Rand

    SvxBoxItem aFmtBox;

    // nGapHalf bedeutet bei WW ein *horizontaler* Abstand zwischen
    // Tabellenzelle und -Inhalt

    short nHorDist  = (short)pActBand->nGapHalf;
    short nVertDist = 0;

    if( pActBand->pTCs ) // neither Cell Border nor Default Border defined ?
    {
        WW8_TCell* pT = &pActBand->pTCs[nWwIdx];
        if( pIo->IsBorder( pT->rgbrc ) )
        {
            pIo->SetBorder( aFmtBox, pT->rgbrc );
            if ( nHorDist < MIN_BORDER_DIST )
                nHorDist = MIN_BORDER_DIST; // Border -> min. MIN_BORDER_DIST
        }
    }

    // Vertikal benutzt WW den Absatzabstand oben und unten, um einen Abstand
    // zu schaffen. Dieser ist hier leider noch unbekannt.  :-(


    // Frueher stand hier das Hypo-Flag:
    // Hypo-Tabellen alles einheitlich !
    // if ( SwFltGetFlag( pIo->GetFieldFlags(), SwFltControlStack::HYPO ) )

    // jetzt musz das alte Verhalten auch von HYPO gesondert aktiviert werden !
    if( SwFltGetFlag( pIo->GetIniFlags1(), EQUAL_TAB_BORDERDISTS ) )
    {
        if ( nHorDist < MIN_BORDER_DIST )
            nHorDist = MIN_BORDER_DIST; // Border -> min. MIN_BORDER_DIST
        nVertDist = nHorDist;
    }
    else
    {
        if( !nVertDist )            // kein Abstand gesetzt
            nVertDist = 18;         // ca. 0.03 cm
    }

    aFmtBox.SetDistance( nHorDist, BOX_LINE_LEFT    );
    aFmtBox.SetDistance( nVertDist, BOX_LINE_TOP    );

    aFmtBox.SetDistance( nHorDist, BOX_LINE_RIGHT   );
    aFmtBox.SetDistance( nVertDist, BOX_LINE_BOTTOM );

    pBox->GetFrmFmt()->SetAttr( aFmtBox );
}

void WW8TabDesc::SetTabShades( SwTableBox* pBox, short nWwIdx )
{
    if( nWwIdx < 0 || nWwIdx >= pActBand->nWwCols )
        return;                 // kuenstlich erzeugte Zellen -> Keine Farbe
    WW8_SHD& rSHD = pActBand->pSHDs[nWwIdx];
    if( !rSHD.GetValue() )      // alles weiss
        return;

    SwWW8Shade aSh( pIo->bVer67, rSHD );

    pBox->GetFrmFmt()->SetAttr( SvxBrushItem( aSh.aColor ) );
}

void WW8TabDesc::AdjustNewBand( SwWW8ImplReader* pReader )
{
    if( pActBand->nSwCols > nDefaultSwCols )        // Zellen splitten
        InsertCells( pActBand->nSwCols - nDefaultSwCols );

    SetPamInCell( 0, FALSE, FALSE );
    ASSERT( pTabBoxes && pTabBoxes->Count() == (USHORT)pActBand->nSwCols,
            "Falsche Spaltenzahl in Tabelle" )

    if( bClaimLineFmt ){
        pTabLine->ClaimFrmFmt();            // noetig wg. Zeilenhoehe
#if 1

        SwFmtFrmSize aF( ATT_MIN_SIZE, 0, 0 );  // default

        if( pActBand->nLineHeight == 0 ){       // 0 = Auto
            aF.SetSizeType( ATT_VAR_SIZE );
        }else{
            if( pActBand->nLineHeight < 0 ){    // Pos = min, Neg = exakt
                aF.SetSizeType( ATT_FIX_SIZE );
                pActBand->nLineHeight = -pActBand->nLineHeight;
            }
            if( pActBand->nLineHeight < MINLAY ) // nicht erlaubte Zeilenhoehe
                pActBand->nLineHeight = MINLAY;

            aF.SetHeight( pActBand->nLineHeight );// Min- / Exakt-Hoehe setzen
        }
        pTabLine->GetFrmFmt()->SetAttr( aF );
#else
        SwFrmSize eSiz = ATT_MIN_SIZE;      // Default

        if( pActBand->nLineHeight < 0 ){    // Pos = min; Neg = exakt
            pActBand->nLineHeight = -pActBand->nLineHeight;
            eSiz = ATT_FIX_SIZE;
        }

        if( pActBand->nLineHeight >= MINLAY ){  // erlaubte Zeilenhoehe, nicht auto
            pTabLine->GetFrmFmt()->SetAttr(
                SwFmtFrmSize( eSiz, 0, pActBand->nLineHeight ) );
        }else{                              // Auto oder ungueltig
            pTabLine->GetFrmFmt()->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, 0 ) );
        }
#endif
    }

    short i;    // SW-Index
    short j;    // WW-Index
    short nW;   // Breite

    SwFmtFrmSize aFS( ATT_FIX_SIZE );
    j = -pActBand->bLEmptyCol;
    for( i = 0; i < pActBand->nSwCols; i++ )
    {
        // setze Zellenbreite
        if( j < 0 )
            nW = pActBand->nCenter[0] - nMinLeft;
        else
        {
            if( j < pActBand->nWwCols )
                nW = pActBand->nCenter[j+1] - pActBand->nCenter[j];
            else
                nW = nMaxRight - pActBand->nCenter[j];
            pActBand->nWidth[ j ] = nW;
        }

        register SwTableBox* pBox = (*pTabBoxes)[i];
        pBox->ClaimFrmFmt();    // liesse sich durch intelligentes Umhaengen
                                                    // der FrmFmts noch weiter verringern
        // naemlich so: pFrmFmt->Add( pBox );
        // gibt der Box pBox das FrameFormat pFrmFmt


        //pMergeCtrl[ j ].nRowWidth = nW;
        aFS.SetWidth( nW );
        pBox->GetFrmFmt()->SetAttr( aFS );

                                            // setze Umrandung und Hintergrund
        SetTabBorders( pBox, j );           // und Abstand
        if( pActBand->pSHDs )
            SetTabShades( pBox, j );
        j++;
        // ueberspringe nicht existente Zellen
        while( ( j < pActBand->nWwCols ) && !pActBand->bExist[j] )
        {
            //pActBand->nWidth[  j ] =    0;
            pActBand->nWidth[  j ] = pActBand->nCenter[j+1] - pActBand->nCenter[j];
            pActBand->nCenter[ j ] = -999;
            j++;
        }
    }
}

void WW8TabDesc::TableCellEnd()
{
    ::SetProgressState( pIo->nProgress, pIo->rDoc.GetDocShell() );   // Update

    // neue Zeile
//  BOOL bRowEnded = pIo->bWasTabRowEnd;
    if( pIo->bWasTabRowEnd )  // bWasTabRowEnd will be deactivated
    {                         // in SwWW8ImplReader::ProcessSpecial()

        USHORT iCol = GetLogicalWWCol();
        if( iCol < aNumRuleNames.Count() )
            aNumRuleNames.DeleteAndDestroy( iCol, aNumRuleNames.Count()-iCol );

        pIo->pCtrlStck->SetAttr( *pIo->pPaM->GetPoint(), 0, FALSE );

        nAktCol = 0;
        nAktRow++;
        nAktBandRow++;
        ASSERT( pActBand , "pActBand ist 0" );
        if( pActBand )
        {
            if( nAktRow >= nRows )  // am Tabellenende gibt's nichts sinnvolles
                return;                 // mehr zu tun

            BOOL bNewBand = nAktBandRow >= pActBand->nRows;
            if( bNewBand )
            {                       // neues Band noetig ?
                pActBand = pActBand->pNextBand; //
                nAktBandRow = 0;
                ASSERT( pActBand, "pActBand ist 0" );
                AdjustNewBand( pIo );
            }
            else
            {
                SwTableBox* pBox = (*pTabBoxes)[0];
                SwSelBoxes aBoxes;
                pIo->rDoc.InsertRow( pTable->SelLineFromBox( pBox, aBoxes ) );
            }
        }
    }
    else
    {                       // neue Spalte ( Zelle )
        nAktCol++;
    }
    SetPamInCell( nAktCol, TRUE, TRUE );

    // finish Annotated Level Numbering ?
    if( pIo->bAnl && !pIo->bAktAND_fNumberAcross )
        pIo->StopAnl( IsValidCell( nAktCol ) );//FALSE );//!bRowEnded ); //
}


// ggfs. die Box in fuer diese Col offene Merge-Gruppe eintragen
SwTableBox* WW8TabDesc::UpdateTableMergeGroup(  WW8_TCell&     rCell,
                                                WW8SelBoxInfo* pActGroup,
                                                SwTableBox*    pActBox,
                                                USHORT         nCol )
{
    // Rueckgabewert defaulten
    SwTableBox* pResult = 0;

    // pruefen, ob die Box zu mergen ist
    if(    rCell.bFirstMerged
        || rCell.bMerged
        || rCell.bVertMerge
        || rCell.bVertRestart )
    {
        // passende Merge-Gruppe ermitteln
        WW8SelBoxInfo* pTheMergeGroup = 0;
        if( pActGroup )
            // Gruppe uebernehmen
            pTheMergeGroup = pActGroup;
        else
        {
            // Gruppe finden
            short nMGrIdx;
            if( FindMergeGroup( pActBand->nCenter[ nCol ],
                                pActBand->nWidth[  nCol ], TRUE, nMGrIdx ) )
                pTheMergeGroup = (*pMergeGroups)[ nMGrIdx ];
        }
        if( pTheMergeGroup )
        {
            // aktuelle Box der Merge-Gruppe hinzufuegen
            pTheMergeGroup->Insert( pActBox, pTheMergeGroup->Count() );

            // Target-Box zurueckmelden
            pResult = (*pTheMergeGroup)[ 0 ];
        }
    }
    return pResult;
}


USHORT WW8TabDesc::GetLogicalWWCol() const // returns number of col as INDICATED within WW6 UI status line -1
{
    USHORT nCol = 0;
    if( pActBand && pActBand->pTCs)
    {
        for( USHORT iCol = 1; iCol <= nAktCol; ++iCol )
        {
            if( !pActBand->pTCs[ iCol-1 ].bMerged )
                ++nCol;
        }
    }
    return nCol;
}

const String& WW8TabDesc::GetNumRuleName() const // find name of numrule valid for current WW-COL
{
    USHORT nCol = GetLogicalWWCol();
    const String*   pName =   aNumRuleNames.Count()  > nCol
                            ? aNumRuleNames.GetObject( nCol )
                            : 0;
    return pName ? *pName : aEmptyStr;
}

void WW8TabDesc::SetNumRuleName( const String& rName )
{
    USHORT nCol = GetLogicalWWCol();
    String* pName = 0;
    while( aNumRuleNames.Count() <= nCol )
    {
        aNumRuleNames.Insert( pName, aNumRuleNames.Count() );
    }
    String* pOldName = aNumRuleNames.GetObject( nCol );
    pName = new String( rName );
    aNumRuleNames.Replace( pName, nCol );
    if( pOldName )
        delete pOldName;
}




BOOL SwWW8ImplReader::StartTable()
{
    if(    pTableDesc       // keine rekursiven Tabellen
        || bReadNoTbl )     // Nicht bei EinfuegenDatei in Tabelle
        return FALSE;       // oder Fussnote

    pTableDesc = new WW8TabDesc( this );
    if( pTableDesc->Ok() )
        pTableDesc->CreateSwTable();
    else
    {
        delete pTableDesc;
        pTableDesc = 0;
    }
    return 0 != pTableDesc;
}

void SwWW8ImplReader::TabCellEnd()
{
    if( bTable && pTableDesc )
        pTableDesc->TableCellEnd();
}

void SwWW8ImplReader::Read_TabRowEnd( USHORT, BYTE* pData, short nLen ) // Sprm25
{
    if( ( nLen > 0 ) && ( *pData == 1 ) )
    {
        bWasTabRowEnd = TRUE;
        pSBase->SetNoAttrScan( 1 );
    }
}

void SwWW8ImplReader::StopTable()
{
    pTableDesc->FinishSwTable();
    delete( pTableDesc );
    pTableDesc = 0;
}

// GetTableLeft() wird fuer absatzgebundene Grafikobjekte in Tabellen
// gebraucht.
// WW nimmt bei eingerueckten Tabellen den Absatzrand, der ohne Tabelle
// gueltig waere, als Basis; SW benutzt den linken Tabellenrand.
short SwWW8ImplReader::GetTableLeft()
{
    return (pTableDesc) ? pTableDesc->GetMinLeft() : 0;
}


BOOL SwWW8ImplReader::IsInvalidOrToBeMergedTabCell() const
{
    if( !pTableDesc )
        return FALSE;

    const WW8_TCell* pCell = pTableDesc->GetAktWWCell();

    return     !pTableDesc->IsValidCell( pTableDesc->GetAktCol() )
            || (    pCell
                 && (    !pCell->bFirstMerged
                      && (    pCell->bMerged
                           || (    pCell->bVertMerge
                                && !pCell->bVertRestart
                              )
                         )
                    )
                );
}


const USHORT SwWW8ImplReader::StyleUsingLFO( USHORT nLFOIndex ) const
{
    USHORT nRes = USHRT_MAX;
    if( pCollA )
    {
        for(USHORT nI = 0; nI < pStyles->GetCount(); nI++ )
            if(    pCollA[ nI ].bValid
                && (nLFOIndex == pCollA[ nI ].nLFOIndex) )
                nRes = nI;
    }
    return nRes;
}


const SwFmt* SwWW8ImplReader::GetStyleWithOrgWWName( String& rName ) const
{
    SwFmt* pRet = 0;
    if( pCollA )
    {
        for(USHORT nI = 0; nI < pStyles->GetCount(); nI++ )
            if(    pCollA[ nI ].bValid
                && (rName.Equals( pCollA[ nI ].GetOrgWWName())) )
            {
                pRet = pCollA[ nI ].pFmt;
                break;
            }
    }
    return pRet;
}



//-----------------------------------------
//          class WW8RStyle
//-----------------------------------------

BYTE* WW8RStyle::HasParaSprm( USHORT nId ) const
{
    if( !pParaSprms || !nSprmsLen )
        return 0;

    BYTE* pSprms = pParaSprms;
    BYTE nDelta;
    short i, x;

    for( i=0; i<nSprmsLen;   )
    {
        USHORT nAktId = WW8GetSprmId( rFib.nVersion, pSprms, &nDelta );
        // Sprm gefunden?
        if( nAktId == nId )
            //alt: if( *pSprms == nId )
            return pSprms + 1 + nDelta + WW8SprmDataOfs( nId ); // Zeiger auf Daten
        x = WW8GetSprmSizeBrutto( rFib.nVersion, pSprms, &nAktId );
        i += x;
        pSprms += x;
    }
    return 0;                               // Sprm nicht gefunden
}


void WW8RStyle::ImportSprms( long nPosFc, short nLen, BOOL bPap )
{
    if( pStStrm->IsEof() )
        return;

    BYTE *pSprms0 = new BYTE[nLen];
    if( bPap )
    {
        pParaSprms = pSprms0;   // fuer HasParaSprms()
        nSprmsLen = nLen;
    }
    BYTE* pSprms1 = pSprms0;
    pStStrm->Seek( nPosFc );
    pStStrm->Read( pSprms0, nLen );

    while ( nLen > 0 )
    {
        USHORT nL1 = pIo->ImportSprm( pSprms1, nLen );
        nLen -= nL1;
        pSprms1 += nL1;
    }
    delete[]( pSprms0 );
    pParaSprms = 0;
    nSprmsLen = 0;
}


short WW8RStyle::ImportUPX( short nLen, BOOL bPAP )
{
    INT16 cbUPX;

    if( 0 < nLen ) // Empty ?
    {
        nLen -= WW8SkipOdd( pStStrm );

        *pStStrm >> cbUPX;

        nLen-=  2;

        if ( cbUPX > nLen )
            cbUPX = nLen;       // !cbUPX auf nLen verkleinert!

        if( (1 < cbUPX) || ( (0 < cbUPX) && !bPAP ) )
        {
            if( bPAP )
            {
                UINT16 id;
                *pStStrm >> id;

                cbUPX-=  2;
                nLen-=  2;
            }

            if( 0 < cbUPX )
            {
                ULONG nPos = pStStrm->Tell();   // falls etwas falsch interpretiert
                                                // wird, gehts danach wieder richtig
                ImportSprms( nPos, cbUPX, bPAP );

                if ( pStStrm->Tell() != nPos + cbUPX )
                    pStStrm->Seek( nPos+cbUPX );

                nLen -= cbUPX;
            }
        }
    }
    return nLen;
}


void WW8RStyle::ImportGrupx( short nLen, BOOL bPara )
{
    if( nLen <= 0 )
        return;

//  if( rFib.nVersion < 8 )
        nLen -= WW8SkipOdd( pStStrm );

    if( bPara ) nLen = ImportUPX( nLen, TRUE ); // Grupx.Papx
    ImportUPX( nLen, FALSE );                   // Grupx.Chpx
}

WW8RStyle::WW8RStyle( WW8Fib& rFib, SwWW8ImplReader* pI )
    : WW8Style( *pI->pTableStream, rFib ), pIo( pI ),
    pStStrm( pI->pTableStream ),
nWwNumLevel( 0 ), pStyRule( 0 )
{
    pIo->pCollA = new SwWW8StyInf[ cstd ]; // Style-UEbersetzung WW->SW
    pIo->nColls = cstd;
}


void WW8RStyle::Set1StyleDefaults()
{
    if( !bFontChanged )                         // Style hat keinen Font ?
    {
        short nWwDefaultFont = ftcStandardChpStsh;
        pIo->SetNewFontAttr( nWwDefaultFont );  // ... dann hart 'reinsetzen
    }
    if( pIo->bNoAttrImport )
        return;
    if( !bFSizeChanged )
    {                                           // Style hat keine FontSize ?
        SvxFontHeightItem aSz( 200 );           // WW-Default: 10pt
        pIo->pAktColl->SetAttr( aSz );
    }
    if( pIo->pWDop->fWidowControl && !bWidowsChanged )  // Widows ?
    {
        pIo->pAktColl->SetAttr( SvxWidowsItem( 2 ) );
        pIo->pAktColl->SetAttr( SvxOrphansItem( 2 ) );
    }
}

//-----------------------------------------
//      Zeichenvorlagen
//-----------------------------------------
SwCharFmt* WW8RStyle::SearchCharFmt( const String& rName )
{
    USHORT n;
                            // Suche zuerst in den Doc-Styles
    for( n = pIo->rDoc.GetCharFmts()->Count(); n > 0; )
        if( (*pIo->rDoc.GetCharFmts())[ --n ]->GetName().Equals( rName ) )
            return (*pIo->rDoc.GetCharFmts())[ n ];

                    // Collection noch nicht gefunden, vielleicht im Pool ?
    n = pIo->rDoc.GetPoolId( rName , GET_POOLID_CHRFMT );
    if ( n != USHRT_MAX )       // gefunden oder Standard
        return pIo->rDoc.GetCharFmtFromPool( n );

    return 0;
}

SwCharFmt* WW8RStyle::MakeNewCharFmt( WW8_STD* pStd, const String& rName )
{
    String aName( rName );
    SwCharFmt* pFmt = 0;

    if( pStd->sti != STI_USER           // eingebauter Style, den wir nicht kennen ?
        || SearchCharFmt( aName ) ){    // oder Namen gibts schon ?

        if( !aName.EqualsAscii( "WW-", 0, 3 ) ) // noch kein "WW-"
            aName.InsertAscii( "WW-", 0 );      // dann aender ihn

        if( SearchCharFmt( aName ) )                // Namen gibt's immer noch ?
            for( USHORT n = 1; n < 1000; n++ ){     // dann bastel neuen
                String aName1( aName );
                aName1 += String::CreateFromInt32( n );
                if( ( pFmt = SearchCharFmt( aName1 ) ) == 0 ){
                    aName = aName1;
                    break;                          // unbenutzten Namen gefunden
                }
            }
    }
    if( !pFmt )   // unbenutzter Collection-Name gefunden, erzeuge neue Coll
        pFmt = pIo->rDoc.MakeCharFmt( aName,
            ( SwCharFmt*)pIo->rDoc.GetDfltCharFmt() ); // const Wegcasten

    return pFmt;
}


SwCharFmt* WW8RStyle::MakeOrGetCharFmt( BOOL* pbStyExist, WW8_STD* pStd, const String& rName )
{
#define RES_NO RES_POOLCHR_END

    static USHORT __READONLY_DATA aArr1[]={
        RES_POOLCHR_FOOTNOTE, 0, RES_POOLCHR_LINENUM,
        RES_POOLCHR_PAGENO, RES_POOLCHR_ENDNOTE };

    static USHORT __READONLY_DATA aArr2[]={
        RES_POOLCHR_INET_NORMAL, RES_POOLCHR_INET_VISIT,
        RES_POOLCHR_HTML_STRONG, RES_POOLCHR_HTML_EMPHASIS };

    if(     pIo->bNew               // Einfuegen: immer neue Styles generieren
        && !( pIo->nIniFlags & WW8FL_NO_DEFSTYLES ) )   // nicht abgeschaltet
    {
        SwCharFmt* pFmt = 0;

        // Default-Style bekannt ?
        short nIdx = pStd->sti - 38;
        if(    nIdx >= 0 && nIdx < 5
            && aArr1[nIdx]!=0 )
            pFmt = pIo->rDoc.GetCharFmtFromPool( aArr1[ nIdx ] );
        else
        {
            nIdx = pStd->sti - 85;
            if(    nIdx >= 0
                && nIdx <  4 )
                pFmt = pIo->rDoc.GetCharFmtFromPool( aArr2[ nIdx ] );
        }
        if( pFmt )
        {
            *pbStyExist = TRUE;
            return pFmt;
        }
    }
    {
        *pbStyExist = FALSE;
        String aName( rName );
        USHORT nPos = aName.Search( ',' );
        if( nPos )  // mehrere Namen mit Komma getrennt gibts im SW nicht
            aName.Erase( nPos );
        return MakeNewCharFmt( pStd, aName );   // nicht gefunden
    }
}

//-----------------------------------------
//      Absatzvorlagen
//-----------------------------------------

SwTxtFmtColl* WW8RStyle::SearchFmtColl( const String& rName )
{
                            // Suche zuerst in den Doc-Styles
    SwTxtFmtColl* pColl = pIo->rDoc.FindTxtFmtCollByName( rName );
    if( !pColl )
    {
                    // Collection noch nicht gefunden, vielleicht im Pool ?
        USHORT n = pIo->rDoc.GetPoolId( rName , GET_POOLID_TXTCOLL );
        if ( n != USHRT_MAX )       // gefunden oder Standard
            pColl = pIo->rDoc.GetTxtCollFromPool( n );
    }
    return pColl;
}

SwTxtFmtColl* WW8RStyle::MakeNewFmtColl( WW8_STD* pStd, const String& rName )
{
    String aName( rName );
    SwTxtFmtColl* pColl = 0;

    if( pStd->sti != STI_USER           // eingebauter Style, den wir nicht kennen
        || SearchFmtColl( aName ) ){    // oder Namen gibts schon ?

        if( !aName.EqualsIgnoreCaseAscii( "WW-", 0, 3 ) )   // noch kein "WW-"
            aName.Insert( WW8_ASCII2STR( "WW-" ), 0 );  // dann AEnder ihn

        if( SearchFmtColl( aName ) )                    // Namen gibt's immer noch ?
            for( USHORT n = 1; n < 1000; n++ ){     // dann bastel neuen
                String aName1( aName );
                aName += String::CreateFromInt32( n );
                if( ( pColl = SearchFmtColl( aName1 ) ) == 0 ){
                    aName = aName1;
                    break;                          // unbenutzten Namen gefunden
                }
            }
    }
    if( !pColl )   // unbenutzter Collection-Name gefunden, erzeuge neue Coll
        pColl = pIo->rDoc.MakeTxtFmtColl( aName,
            ( SwTxtFmtColl*)pIo->rDoc.GetDfltTxtFmtColl() ); // const Wegcasten

    return pColl;
}

SwTxtFmtColl* WW8RStyle::MakeOrGetFmtColl( BOOL* pbStyExist, WW8_STD* pStd, const String& rName )
{
#define RES_NONE RES_POOLCOLL_DOC_END

    static const RES_POOL_COLLFMT_TYPE aArr[]={
        RES_POOLCOLL_STANDARD, RES_POOLCOLL_HEADLINE1, RES_POOLCOLL_HEADLINE2,                                  // Ueberschrift 2
        RES_POOLCOLL_HEADLINE3, RES_POOLCOLL_HEADLINE4, RES_POOLCOLL_HEADLINE5,
        RES_POOLCOLL_HEADLINE6, RES_POOLCOLL_HEADLINE7, RES_POOLCOLL_HEADLINE8,
        RES_POOLCOLL_HEADLINE9,

        RES_POOLCOLL_TOX_IDX1, RES_POOLCOLL_TOX_IDX2, RES_POOLCOLL_TOX_IDX3,
        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,
        RES_POOLCOLL_TOX_CNTNT1,

        RES_POOLCOLL_TOX_CNTNT2, RES_POOLCOLL_TOX_CNTNT3, RES_POOLCOLL_TOX_CNTNT4,
        RES_POOLCOLL_TOX_CNTNT5, RES_POOLCOLL_TOX_CNTNT6, RES_POOLCOLL_TOX_CNTNT7,
        RES_POOLCOLL_TOX_CNTNT8, RES_POOLCOLL_TOX_CNTNT9,
        RES_NONE, RES_POOLCOLL_FOOTNOTE,

        RES_NONE, RES_POOLCOLL_HEADER, RES_POOLCOLL_FOOTER, RES_POOLCOLL_TOX_IDXH,
        RES_NONE, RES_NONE, RES_POOLCOLL_JAKETADRESS, RES_POOLCOLL_SENDADRESS,
        RES_NONE, RES_NONE,

        RES_NONE, RES_NONE, RES_NONE, RES_POOLCOLL_ENDNOTE, RES_NONE, RES_NONE, RES_NONE,
        RES_POOLCOLL_LISTS_BEGIN, RES_NONE, RES_NONE,

        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,
        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,

        RES_NONE,RES_NONE, RES_POOLCOLL_DOC_TITEL, RES_NONE, RES_POOLCOLL_SIGNATURE, RES_NONE,
        RES_POOLCOLL_TEXT, RES_POOLCOLL_TEXT_MOVE, RES_NONE, RES_NONE,

        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_POOLCOLL_DOC_SUBTITEL };


    ASSERT( ( sizeof( aArr ) / sizeof( RES_POOL_COLLFMT_TYPE ) == 75 ),
            "Style-UEbersetzungstabelle hat falsche Groesse" );

#if 1
    if( pStd->sti < sizeof( aArr ) / sizeof( RES_POOL_COLLFMT_TYPE )
        && aArr[pStd->sti]!=RES_NONE    // Default-Style bekannt
        && !( pIo->nIniFlags & WW8FL_NO_DEFSTYLES ) ){ // nicht abgeschaltet

        SwTxtFmtColl* pCol = pIo->rDoc.GetTxtCollFromPool( aArr[pStd->sti] );
        if( pCol ){
            *pbStyExist = TRUE;
            return pCol;
        }
    }
    {
        String aName( rName );
        USHORT nPos = aName.Search( ',' );
        if( nPos )  // mehrere Namen mit Komma getrennt gibts im SW nicht
            aName.Erase( nPos );
        SwTxtFmtColl* pCol = SearchFmtColl( aName );
        if( pCol ){
            *pbStyExist = TRUE;
            return pCol;
        }
        *pbStyExist = FALSE;
        return MakeNewFmtColl( pStd, aName );   // nicht gefunden
    }
#else // 1
    if( pIo->bNew                   // Einfuegen: immer neue Styles generieren
        && pStd->sti < sizeof( aArr ) / sizeof( RES_POOL_COLLFMT_TYPE )
        && aArr[pStd->sti]!=RES_NONE    // Default-Style bekannt
        && !( pIo->nIniFlags & WW8FL_NO_DEFSTYLES ) ){ // nicht abgeschaltet

        SwTxtFmtColl* pCol = pIo->rDoc.GetTxtCollFromPool( aArr[pStd->sti] );
        if( pCol ){
            *pbStyExist = TRUE;
            return pCol;
        }
    }
    {
        *pbStyExist = FALSE;
        String aName( rName );
        USHORT nPos = aName.Search( ',' );
        if( nPos )  // mehrere Namen mit Komma getrennt gibts im SW nicht
            aName.Erase( nPos );
        return MakeNewFmtColl( pStd, aName );   // nicht gefunden
    }
#endif // !1
}

void WW8RStyle::Import1Style( USHORT nNr )
{
    SwWW8StyInf* pSI = &pIo->pCollA[nNr];

    if( pSI->bImported || !pSI->bValid )
        return;

    pSI->bImported = TRUE;                      // jetzt schon Flag setzen
                                                                        // verhindert endlose Rekursion
    if( pSI->nBase < cstd                           // gueltig und nicht NIL
        && !pIo->pCollA[pSI->nBase].bImported ) // und noch nicht Importiert
        Import1Style( pSI->nBase );

    pStStrm->Seek( pSI->nFilePos );

    short nSkip, cbStd;
    String sName;

    WW8_STD* pStd = Read1Style( nSkip, &sName, &cbStd );// lies Style

    if( pStd )
        pSI->SetOrgWWIdent( sName, pStd->sti );

    if(    !pStd
        || (0 == sName.Len())                       // no Name or unused Slot
        || ((1 != pStd->sgc) && (2 != pStd->sgc)) ) // unknown Style
    {
        pStStrm->SeekRel( nSkip );
        return;
    }
                                                // echter Para- oder Char-Style
    SwFmt* pColl;
    BOOL bStyExist;
    if( pStd->sgc == 1 )                // Para-Style
        pColl = MakeOrGetFmtColl( &bStyExist, pStd, sName );
    else                                // Char-Style
        pColl = MakeOrGetCharFmt( &bStyExist, pStd, sName );

    BOOL bImport = !bStyExist || pIo->bNew; // Inhalte Importieren ?
    BOOL bOldNoImp = pIo->bNoAttrImport;
    pSI->bImportSkipped = !bImport;

    if( !bImport ){
        pIo->bNoAttrImport = TRUE;
    }
    else
    {
        if( bStyExist )
            pColl->ResetAllAttr();
        pColl->SetAuto( FALSE );            // nach Empfehlung JP
    }                                   // macht die UI aber anders
    pIo->pAktColl = pColl;
    pSI->pFmt = pColl;                  // UEbersetzung WW->SW merken
    pSI->bImportSkipped = !bImport;

    // Setze Based on
    USHORT j = pSI->nBase;
    if(     j != nNr                                // sinnvoller Index ?
        &&  j < cstd )
    {
        SwWW8StyInf* pj = &pIo->pCollA[j];
        if (
                    pSI->pFmt && pj->pFmt           // Beide Formate ok ?
                &&  pSI->bColl == pj->bColl )       // beides gleicher Typ ?
        {
            pSI->pFmt->SetDerivedFrom( pj->pFmt );  // ok, Based on eintragen
            pSI->eFontSrcCharSet = pj->eFontSrcCharSet; // CharSet als Default
        }
    }
    else if( pIo->bNew && bStyExist )
    {
        if( pStd->sgc == 1 )
            pSI->pFmt->SetDerivedFrom( pIo->pStandardFmtColl );
        else
            pSI->pFmt->SetDerivedFrom( pIo->rDoc.GetDfltCharFmt() );
    }

    pSI->nFollow = pStd->istdNext;              // Follow merken

    long nPos = pStStrm->Tell();        // falls etwas falsch interpretiert
                                    // wird, gehts danach wieder richtig

    pStyRule = 0;                   // falls noetig, neu anlegen
    bFontChanged = bFSizeChanged = bWidowsChanged = FALSE;
    pIo->SetNAktColl( nNr );
    pIo->bStyNormal = nNr == 0;

    if( pStd && ( pStd->sgc == 1 || pStd->sgc == 2 ) ){
        ImportGrupx( nSkip, pStd->sgc == 1 ); // Import des Style-Inhalts
                        // Alle moeglichen Attribut-Flags zuruecksetzen,
                        // da es in Styles keine Attr-Enden gibt
        pIo->bHasBorder = pIo->bTxtCol = pIo->bShdTxtCol = pIo->bCharShdTxtCol
        = pIo->bSpec = pIo->bObj = pIo->bSymbol = FALSE;
        pIo->nCharFmt = -1;
    }

//  if( pStd && pStd->istdBase == 0xfff     // Style basiert auf Nichts
//      && pStd->sgc == 1 ){                //! Char-Styles funktionieren aus

    if( pStd
        && ( pSI->nBase >= cstd // Style basiert auf Nichts
          || pIo->pCollA[pSI->nBase].bImportSkipped ) // oder Basis ignoriert
        && pStd->sgc == 1 ){                //! Char-Styles funktionieren aus
                                            // unerfindlichen Gruenden nicht
                                            // -> dann evtl. harte WW-Defaults
                                            // reinsetzen
        Set1StyleDefaults();
    }

    pStyRule = 0;                   // zur Sicherheit
    pIo->bStyNormal = FALSE;
    pIo->SetNAktColl( 0 );
    pIo->bNoAttrImport = bOldNoImp;
    // rasch nochmal die Listen-Merk-Felder zuruecksetzen,
    // fuer den Fall dass sie beim einlesen des Styles verwendet wurden
    pIo->nLFOPosition = USHRT_MAX;
    pIo->nListLevel   = nWW8MaxListLevel;

    pStStrm->Seek( nPos+nSkip );

    DELETEZ( pStd );
}


void WW8RStyle::RegisterNumFmts()
{
    for(USHORT i=0; i < cstd; i++)
    {
        SwWW8StyInf* pSI = &pIo->pCollA[ i ];
        if( pSI )
            pIo->RegisterNumFmtOnStyle( *pSI );
    }
}


void WW8RStyle::ScanStyles()        // untersucht Style-Abhaengigkeiten
{                               // und ermittelt die Filepos fuer jeden Style
/*
    WW8_FC nStyleStart = rFib.fcStshf;
    pStStrm->Seek( nStyleStart );
    */
    USHORT i;
    for( i=0; i<cstd; i++ ){
        short nSkip;
        SwWW8StyInf* pSI = &pIo->pCollA[i];

        pSI->nFilePos = pStStrm->Tell();        // merke FilePos
        WW8_STD* pStd = Read1Style( nSkip, 0, 0 );  // read STD
        pSI->bValid   = ( 0 != pStd );
        if( pSI->bValid
            && !(pStd->sgc == 2 && ( pIo->nIniFlags & WW8FL_NO_ZSTYLES ) ) ){
            pSI->nBase = pStd->istdBase;        // merke Basis
            pSI->bColl = ( pStd->sgc == 1 );    // Para-Style
            pSI->bValid = TRUE;
        }
        else
            memset( &pSI , 0, sizeof( &pSI ) );

        delete pStd;
        pStStrm->SeekRel( nSkip );              // ueberlese Namen und Sprms
    }
}

void WW8RStyle::Import()
{
    pIo->pDfltTxtFmtColl  = pIo->rDoc.GetDfltTxtFmtColl();
    pIo->pStandardFmtColl = pIo->rDoc.GetTxtCollFromPool( RES_POOLCOLL_STANDARD );

    if( pIo->nIniFlags & WW8FL_NO_STYLES )
        return;

    ScanStyles();                       // Scanne Based On

    USHORT i;
    for( i=0; i<cstd; i++ ) // Styles importieren
        if( pIo->pCollA[i].bValid )
            Import1Style( i );

    for( i=0; i<cstd; i++ )
    {   // Follow einstellen
        SwWW8StyInf* pi = &pIo->pCollA[i];
        USHORT j = pi->nFollow;
        if( j < cstd )
        {
            SwWW8StyInf* pj = &pIo->pCollA[j];
            if ( j != i                             // sinnvoller Index ?
                 && pi->pFmt                        // Format ok ?
                 && pj->pFmt                        // Derived-Format ok ?
                 && pi->bColl                       // geht nur bei Absatz-Vorlagen (WW)
                 && pj->bColl ){                    // beides gleicher Typ ?
                    ( (SwTxtFmtColl*)pi->pFmt )->SetNextTxtFmtColl(
                     *(SwTxtFmtColl*)pj->pFmt );    // ok, eintragen
            }
        }
    }
// Die Sonderbehandlung zur Setzen der
// Default-Zeichenvorlage "Absatz-Standardschriftart" ( Style-ID 65 ) fehlt
// Sie ist aber defaultmaessig leer ( WW6 dt und US ) und von der
// WW-UI nicht zu veraendern, so dass das nicht stoert.
// Der Mechanismus waere folgender:
//  if( bNew ) rDoc.SetDefault( pDefCharFmt->GetAttrSet() );
//
    // fuer z.B. Tabellen wird ein immer gueltiger Std-Style gebraucht

    if( pIo->pCollA[0].pFmt && pIo->pCollA[0].bColl && pIo->pCollA[0].bValid )
        pIo->pDfltTxtFmtColl = (SwTxtFmtColl*)pIo->pCollA[0].pFmt;
    else
        pIo->pDfltTxtFmtColl = pIo->rDoc.GetDfltTxtFmtColl();


    // set Hyphenation flag on BASIC para-style
    if(    pIo->bNew
        && pIo->pWDop->fAutoHyphen
        && pIo->pStandardFmtColl
        && SFX_ITEM_SET != pIo->pStandardFmtColl->GetItemState(
                                            RES_PARATR_HYPHENZONE, FALSE ) )
    {
        SvxHyphenZoneItem aAttr( TRUE );
        aAttr.GetMinLead()    = 2;
        aAttr.GetMinTrail()   = 2;
        aAttr.GetMaxHyphens() = 0;

        pIo->pStandardFmtColl->SetAttr( aAttr );
    }

    // wir sind jetzt nicht mehr beim Style einlesen:
    pIo->pAktColl = 0;
}


//-----------------------------------------
//      Document Info
//-----------------------------------------

// ReadDocInfo() liegt hier und nicht nicht in wwpar.cxx::LoadDoc1(),
// da hier das noetige sfxcore.hxx bereits includet ist.

void SwWW8ImplReader::ReadDocInfo()
{
    if( pStg )
    {
        SfxDocumentInfo* pNeu;
        if( rDoc.GetpInfo() )                           // soo ein Umstand......
            pNeu = new SfxDocumentInfo( *rDoc.GetpInfo() );
        else
            pNeu = new SfxDocumentInfo();

        pNeu->LoadPropertySet( pStg );  // DocInfo laden
        rDoc.SetInfo( *pNeu );
        delete( pNeu );
    }
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/ww8par2.cxx,v 1.1.1.1 2000-09-18 17:14:58 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.80  2000/09/18 16:05:00  willem.vandorp
      OpenOffice header added.

      Revision 1.79  2000/08/17 13:49:28  khz
      #77316# set Default Char Format if no Base Format known

      Revision 1.78  2000/07/28 15:37:27  khz
      #73796# don't delete NumRule from Attr but set it into pDoc

      Revision 1.77  2000/07/27 10:21:36  khz
      #73796# stop ANList when opening next cell in a row and !bAktAND_fNumberAcross

      Revision 1.76  2000/07/25 15:17:45  khz
      #76811# read/write AutoHyphenation flag from/into Document Properties

      Revision 1.75  2000/07/24 12:56:38  jp
      Bug #76561#: convert Bulletchar to unicode

      Revision 1.74  2000/07/11 11:39:06  khz
      #76673# prepare implementation of sprmTDelete and sprmTInsert

      Revision 1.73  2000/07/11 11:30:55  khz
      #76673# prepare implementation of sprmTDelete and sprmTInsert

      Revision 1.72  2000/06/08 16:12:55  khz
      Piece table optimization only when piece table exists ;-)

      Revision 1.71  2000/05/31 12:23:09  khz
      Changes for Unicode

      Revision 1.70  2000/05/25 08:06:48  khz
      Piece Table optimization, Unicode changes, Bugfixes

      Revision 1.69  2000/05/18 10:58:59  jp
      Changes for Unicode

      Revision 1.68  2000/05/16 12:13:04  jp
      ASS_FALSE define removed

      Revision 1.67  2000/05/16 11:06:21  khz
      Unicode code-conversion

      Revision 1.66  2000/05/04 07:40:47  khz
      Task #75379# use new Save() and Restore()

      Revision 1.65  2000/04/17 12:01:32  khz
      Task #74837# prevent from breaking table (in Apo) when Apo-flag is missing

*************************************************************************/




