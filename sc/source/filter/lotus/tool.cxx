/*************************************************************************
 *
 *  $RCSfile: tool.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2001-02-26 06:58:59 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <svtools/zforlist.hxx>
#include <tools/solar.h>

#include "cell.hxx"
#include "rangenam.hxx"
#include "compiler.hxx"

#include "tool.h"
#include "decl.h"
#include "root.hxx"
#include "lotrange.hxx"
#include "namebuff.hxx"

#ifndef _FLTTOOLS_HXX
#include "flttools.hxx"
#endif

#pragma optimize("",off)

//--------------------------------------------------------- EXTERNE VARIABLEN -
extern WKTYP                eTyp;           // -> filter.cxx, aktueller Dateityp
extern sal_Char*            pDummy2;        // -> memory.cxx
extern ScDocument*          pDoc;           // -> filter.cxx, Aufhaenger zum Dokumentzugriff
extern CharSet              eCharNach;      // -> filter.cxx, Zeichenkonvertierung von->nach

extern BOOL                 bFormInit;      // -> memory.cxx, fuer GetFormHandle()

//--------------------------------------------------------- GLOBALE VARIABLEN -
BYTE                        nDefaultFormat; // -> op.cpp, Standard-Zellenformat

extern SvxHorJustifyItem    *pAttrRight, *pAttrLeft, *pAttrCenter, *pAttrRepeat, *pAttrStandard;
extern ScProtectionAttr*    pAttrUnprot;
extern SfxUInt32Item**      pAttrValForms;

SvxHorJustifyItem           *pAttrRight, *pAttrLeft, *pAttrCenter, *pAttrRepeat, *pAttrStandard;
                                                    // -> in memory.cxx initialisiert
ScProtectionAttr*           pAttrUnprot;            // ->  " memory.cxx    "

extern FormCache*           pValueFormCache;        // -> in memory.cxx initialisiert
FormCache*                  pValueFormCache;

UINT16                      LotusRangeList::nEingCol;
UINT16                      LotusRangeList::nEingRow;




void PutFormString( UINT16 nCol, UINT16 nRow, UINT16 nTab, sal_Char* pString )
{
    // Label-Format-Auswertung
    DBG_ASSERT( pString != NULL, "PutFormString(): pString == NULL" );

    sal_Char            cForm;
    SvxHorJustifyItem*  pJustify = NULL;

    cForm = *pString;

    switch( cForm )
    {
        case '"':   // rechtsbuendig
            pJustify = pAttrRight;
            pString++;
            break;
        case '\'':  // linksbuendig
            pJustify = pAttrLeft;
            pString++;
            break;
        case '^':   // zentriert
            pJustify = pAttrCenter;
            pString++;
            break;
        case '|':   // printer command
            pString = NULL;
            break;
        case '\\':  // Wiederholung
            pJustify = pAttrRepeat;
            pString++;
            break;
        default:    // kenn' ich nicht!
            pJustify = pAttrStandard;
    }

    if( pString )
    {
        pDoc->ApplyAttr( nCol, nRow, nTab, *pJustify );
        ScStringCell*   pZelle = new ScStringCell( String( pString, pLotusRoot->eCharsetQ ) );
        pDoc->PutCell( nCol, nRow, nTab, pZelle, ( BOOL ) TRUE );
    }
}




void SetFormat( UINT16 nCol, UINT16 nRow, UINT16 nTab, BYTE nFormat, BYTE nSt )
{
    //  PREC:   nSt = Standard-Dezimalstellenanzahl
    pDoc->ApplyAttr( nCol, nRow, nTab, *( pValueFormCache->GetAttr( nFormat, nSt ) ) );

    ScProtectionAttr aAttr;

    aAttr.SetProtection( nFormat & 0x80 );

    pDoc->ApplyAttr( nCol, nRow, nTab, aAttr );
}




sal_Char* ReplaceWith( sal_Char* pString, sal_Char cVon, sal_Char cNach )
{   // Ersetzt alle Vorkommen von cVon durch cNach
    //  cNach = 0 -> cVon wird aus String herausgeloescht!
    // PREC:    pString:    Nullterminierter String
    // POST:    cNach = 0:  aus String werden alle Vorkommen von cVon
    //                      herausgeloescht (String wird kuerzer)
    //          cNach !=0:  alle Vorkommen von cVon im String werden durch
    //                      cNach ersetzt
    DBG_ASSERT( pString != NULL, "ReplaceWith(): pString == NULL" );

    sal_Char *pAkt, *pCpy;
    if( cNach != 0 )
    {   // Zeichen erstzen
        for(  pAkt = pString ; *pAkt != 0 ; pAkt++ )
        {
            if( *pAkt == cVon )
                *pAkt = cNach;
        }
    }
    else
    {   // Zeichen loeschen
        for( pAkt = pCpy =pString ; *pAkt != 0 ; pAkt++ )
        if( *pAkt != cVon )
        {
            *pCpy = *pAkt;
            pCpy++;
        }
        *pCpy = 0;  // Nullterminator anhaengen
    }

    return pString;
}




void InitPage( void )
{   // Seitenformat initialisieren, d.h. Default-Werte von SC holen
    //scGetPageFormat( 0, &aPage );
}




String DosToSystem( sal_Char* p )
{   // Umwandlung vom internen in externen Zeichensatz
    return String( p, pLotusRoot->eCharsetQ );
}




double SnumToDouble( INT16 nVal )
{
    const double pFacts[ 8 ] = {
        5000.0,
        500.0,
        0.05,
        0.005,
        0.0005,
        0.00005,
        0.0625,
        0.015625 };

    double      fVal;

    if( nVal & 0x0001 )
    {
        fVal = pFacts[ ( nVal >> 1 ) & 0x0007 ];
        fVal *= ( INT16 ) ( nVal >> 4 );
    }
    else
        fVal = ( INT16 ) ( nVal >> 1 );

    return fVal;
}




FormCache::FormCache( ScDocument* pDoc, BYTE nNewDefaultFormat )
{   // Default-Format ist 'Default'
    nDefaultFormat = nNewDefaultFormat;
    pFormTable = pDoc->GetFormatTable();
    for( UINT16 nC = 0 ; nC < __nSize ; nC++ )
        bValid[ nC ] = FALSE;
    eLanguage = ScGlobal::eLnge;
}


FormCache::~FormCache()
{
    for( UINT16 nC = 0 ; nC < __nSize ; nC++ )
        delete aIdents[ nC ].GetAttr();
}


SfxUInt32Item* FormCache::NewAttr( BYTE nFormat, BYTE nSt )
{
    // neues Format erzeugen
    BYTE        nL, nH; // Low-/High-Nibble
    BYTE        nForm = nFormat;
    String      aFormString;
    sal_Char*   pFormString = NULL;
    INT16       eType = NUMBERFORMAT_ALL;
    UINT32      nIndex;
    UINT32      nHandle;
    BOOL        bDefault = FALSE;
    //void GenerateFormat( aFormString, eType, COUNTRY_SYSTEM, LANGUAGE_SYSTEM,
    //  BOOL bThousand, BOOL IsRed, UINT16 nPrecision, UINT16 nAnzLeading );

    if( nForm == 0xFF ) // Default-Format?
        nForm = nDefaultFormat;

    // Aufdroeseln in Low- und High-Nibble
    nL = nFormat & 0x0F;
    nH = ( nFormat & 0xF0 ) / 16;

    nH &= 0x07;     // Bits 4-6 'rausziehen
    switch( nH )
    {
        case 0x00:  // Festkommaformat (fixed)
            //fStandard;nL;
            nIndex = pFormTable->GetStandardFormat(
                NUMBERFORMAT_NUMBER, eLanguage );
            pFormTable->GenerateFormat( aFormString, nIndex,
                eLanguage, FALSE, FALSE, nL, 1 );
            break;
        case 0x01:  // Exponentdarstellung (scientific notation)
            //fExponent;nL;
            nIndex = pFormTable->GetStandardFormat(
                NUMBERFORMAT_SCIENTIFIC, eLanguage );
            pFormTable->GenerateFormat( aFormString, nIndex,
                eLanguage, FALSE, FALSE, nL, 1 );
            break;
        case 0x02:  // Waehrungsdarstellung (currency)
            //fMoney;nL;
            nIndex = pFormTable->GetStandardFormat(
                NUMBERFORMAT_CURRENCY, eLanguage );
            pFormTable->GenerateFormat( aFormString, nIndex,
                eLanguage, FALSE, FALSE, nL, 1 );
            break;
        case 0x03:  // Prozent
            //fPercent;nL;
            nIndex = pFormTable->GetStandardFormat(
                NUMBERFORMAT_PERCENT, eLanguage );
            pFormTable->GenerateFormat( aFormString, nIndex,
                eLanguage, FALSE, FALSE, nL, 1 );
            break;
        case 0x04:  // Komma
            //fStandard;nL;
            nIndex = pFormTable->GetStandardFormat(
                NUMBERFORMAT_NUMBER, eLanguage );
            pFormTable->GenerateFormat( aFormString, nIndex,
                eLanguage, TRUE, FALSE, nL, 1 );
            break;
        case 0x05:  // frei
            //fStandard;nL;
            nIndex = pFormTable->GetStandardFormat(
                NUMBERFORMAT_NUMBER, eLanguage );
            pFormTable->GenerateFormat( aFormString, nIndex,
                eLanguage, FALSE, FALSE, nL, 1 );
            break;
        case 0x06:  // frei
            //fStandard;nL;
            nIndex = pFormTable->GetStandardFormat(
                NUMBERFORMAT_NUMBER, eLanguage );
            pFormTable->GenerateFormat( aFormString, nIndex,
                eLanguage, FALSE, FALSE, nL, 1 );
            nIndex = 0;
            break;
        case 0x07:  // Spezialformat
            switch( nL )
            {
                case 0x00:  // +/-
                    //fStandard;nSt;
                    nIndex = pFormTable->GetStandardFormat(
                        NUMBERFORMAT_NUMBER, eLanguage );
                    pFormTable->GenerateFormat( aFormString, nIndex,
                        eLanguage, FALSE, TRUE, nSt, 1 );
                    break;
                case 0x01:  // generelles Format
                    //fStandard;nSt;
                    nIndex = pFormTable->GetStandardFormat(
                        NUMBERFORMAT_NUMBER, eLanguage );
                    pFormTable->GenerateFormat( aFormString, nIndex,
                        eLanguage, FALSE, FALSE, nSt, 1 );
                    break;
                case 0x02:  // Datum: Tag, Monat, Jahr
                    //fDate;dfDayMonthYearLong;
                    eType = NUMBERFORMAT_DATE;
                    pFormString = "TT.MM.JJJJ";
                    break;
                case 0x03:  // Datum: Tag, Monat
                    //fDate;dfDayMonthLong;
                    eType = NUMBERFORMAT_DATE;
                    pFormString = "TT.MMMM";
                    break;
                case 0x04:  // Datum: Monat, Jahr
                    //fDate;dfMonthYearLong;
                    eType = NUMBERFORMAT_DATE;
                    pFormString = "MM.JJJJ";
                    break;
                case 0x05:  // Textformate
                    //fString;nSt;
                    eType = NUMBERFORMAT_TEXT;
                    pFormString = "@";
                    break;
                case 0x06:  // versteckt
                    //wFlag |= paHideAll;bSetFormat = FALSE;
                    eType = NUMBERFORMAT_NUMBER;
                    pFormString = "";
                    break;
                case 0x07:  // Time: hour, min, sec
                    //fTime;tfHourMinSec24;
                    eType = NUMBERFORMAT_TIME;
                    pFormString = "HH:MM:SS";
                    break;
                case 0x08:  // Time: hour, min
                    //fTime;tfHourMin24;
                    eType = NUMBERFORMAT_TIME;
                    pFormString = "HH:MM";
                    break;
                case 0x09:  // Date, intern INT32 1
                    //fDate;dfDayMonthYearLong;
                    eType = NUMBERFORMAT_DATE;
                    pFormString = "TT.MM.JJJJ";
                    break;
                case 0x0A:  // Date, intern INT32 2
                    //fDate;dfDayMonthYearLong;
                    eType = NUMBERFORMAT_DATE;
                    pFormString = "TT.MM.JJJJ";
                    break;
                case 0x0B:  // Time, intern INT32 1
                    //fTime;tfHourMinSec24;
                    eType = NUMBERFORMAT_TIME;
                    pFormString = "HH:MM:SS";
                    break;
                case 0x0C:  // Time, intern INT32 2
                    //fTime;tfHourMinSec24;
                    eType = NUMBERFORMAT_TIME;
                    pFormString = "HH:MM:SS";
                    break;
                case 0x0F:  // Standardeinstellung
                    //fStandard;nSt;
                    bDefault = TRUE;
                    break;
            }
            break;
    }

    // Format in Table schieben
    if( bDefault )
        nHandle = 0;
    else
    {
        if( pFormString )
            aFormString.AssignAscii( pFormString );

        xub_StrLen  nDummy;
        pFormTable->PutEntry( aFormString, nDummy, eType, nHandle, eLanguage );
    }

    return new SfxUInt32Item( ATTR_VALUE_FORMAT, ( UINT32 ) nHandle );
}




void LotusRange::MakeHash( void )
{
    // 33222222222211111111110000000000
    // 10987654321098765432109876543210
    //                         ******** nColS
    //                   ********       nColE
    //     ****************             nRowS
    // ****************                 nRowE
    nHash = nColStart;
    nHash += nColEnd << 6;
    nHash += ( ( UINT32 ) nRowStart ) << 12;
    nHash += ( ( UINT32 ) nRowEnd ) << 16;
}


LotusRange::LotusRange( UINT16 nCol, UINT16 nRow )
{
    nColStart = nColEnd = nCol;
    nRowStart = nRowEnd = nRow;
    nId = ID_FAIL;
    MakeHash();
}


LotusRange::LotusRange( UINT16 nCS, UINT16 nRS, UINT16 nCE, UINT16 nRE )
{
    nColStart = nCS;
    nColEnd = nCE;
    nRowStart = nRS;
    nRowEnd = nRE;
    nId = ID_FAIL;
    MakeHash();
}


LotusRange::LotusRange( const LotusRange& rCpy )
{
    Copy( rCpy );
}





LotusRangeList::LotusRangeList( void )
{
    aComplRef.InitFlags();

    SingleRefData*  pSingRef;
    nIdCnt = 1;

    pSingRef = &aComplRef.Ref1;
    pSingRef->nTab = pSingRef->nRelTab = 0;
    pSingRef->SetColRel( FALSE );
    pSingRef->SetRowRel( FALSE );
    pSingRef->SetTabRel( TRUE );
    pSingRef->SetFlag3D( FALSE );

    pSingRef = &aComplRef.Ref2;
    pSingRef->nTab = pSingRef->nRelTab = 0;
    pSingRef->SetColRel( FALSE );
    pSingRef->SetRowRel( FALSE );
    pSingRef->SetTabRel( TRUE );
    pSingRef->SetFlag3D( FALSE );
}


LotusRangeList::~LotusRangeList( void )
    {
    LotusRange *pDel = ( LotusRange * ) List::First();

    while( pDel )
        {
        delete pDel;
        pDel = ( LotusRange * ) List::Next();
        }
    }


LR_ID LotusRangeList::GetIndex( const LotusRange &rRef )
{
    LotusRange*     pComp = ( LotusRange* ) List::First();

    while( pComp )
    {
        if( *pComp == rRef )
            return pComp->nId;
        pComp = ( LotusRange* ) List::Next();
    }

    return ID_FAIL;
}


void LotusRangeList::Append( LotusRange* pLR, const String& rName )
{
    DBG_ASSERT( pLR, "*LotusRangeList::Append(): das wird nichts!" );
    List::Insert( pLR, CONTAINER_APPEND );

    ScTokenArray    aTokArray;

    SingleRefData*  pSingRef = &aComplRef.Ref1;

    pSingRef->nCol = pLR->nColStart;
    pSingRef->nRow = pLR->nRowStart;

    if( pLR->IsSingle() )
        aTokArray.AddSingleReference( *pSingRef );
    else
    {
        pSingRef = &aComplRef.Ref2;
        pSingRef->nCol = pLR->nColEnd;
        pSingRef->nRow = pLR->nRowEnd;
        aTokArray.AddDoubleReference( aComplRef );
    }

    ScRangeData*    pData = new ScRangeData(
        pLotusRoot->pDoc, rName, aTokArray );

    pLotusRoot->pScRangeName->Insert( pData );

    pLR->SetId( nIdCnt );

    nIdCnt++;
}




RangeNameBufferWK3::RangeNameBufferWK3( void )
{
    pScTokenArray = new ScTokenArray;
    nIntCount = 1;
}


RangeNameBufferWK3::~RangeNameBufferWK3()
{
    ENTRY*      pDel = ( ENTRY* ) List::First();

    while( pDel )
    {
        delete pDel;
        pDel = ( ENTRY* ) List::Next();
    }

    delete pScTokenArray;
}


void RangeNameBufferWK3::Add( const String& rOrgName, const ComplRefData& rCRD )
{
    static UINT16       nDouble = 0;

    String              aScName( rOrgName );
    ScFilterTools::ConvertName( aScName );

    register ENTRY*     pInsert = new ENTRY( rOrgName, aScName, rCRD );

    List::Insert( pInsert, CONTAINER_APPEND );

    pScTokenArray->Clear();

    register const SingleRefData&   rRef1 = rCRD.Ref1;
    register const SingleRefData&   rRef2 = rCRD.Ref2;

    if( rRef1.nCol == rRef2.nCol && rRef1.nRow == rRef2.nRow && rRef1.nTab == rRef2.nTab )
    {
        pScTokenArray->AddSingleReference( rCRD.Ref1 );
        pInsert->bSingleRef = TRUE;
    }
    else
    {
        pScTokenArray->AddDoubleReference( rCRD );
        pInsert->bSingleRef = FALSE;
    }

    ScRangeData*        pData = new ScRangeData( pLotusRoot->pDoc, aScName, *pScTokenArray );

    pInsert->nRelInd = nIntCount;
    pData->SetIndex( nIntCount );
    nIntCount++;

    pLotusRoot->pScRangeName->Insert( pData );
}


BOOL RangeNameBufferWK3::FindRel( const String& rRef, UINT16& rIndex )
{
    StringHashEntry     aRef( rRef );

    ENTRY*              pFind = ( ENTRY* ) List::First();

    while( pFind )
    {
        if( aRef == pFind->aStrHashEntry )
        {
            rIndex = pFind->nRelInd;
            return TRUE;
        }
        pFind = ( ENTRY* ) List::Next();
    }

    return FALSE;
}


BOOL RangeNameBufferWK3::FindAbs( const String& rRef, UINT16& rIndex )
{
    String              aTmp( rRef );
    StringHashEntry     aRef( aTmp.Erase( 0, 1 ) ); // ohne '$' suchen!

    ENTRY*              pFind = ( ENTRY* ) List::First();

    while( pFind )
    {
        if( aRef == pFind->aStrHashEntry )
        {
            // eventuell neuen Range Name aufbauen
            if( pFind->nAbsInd )
                rIndex = pFind->nAbsInd;
            else
            {
                SingleRefData*      pRef = &pFind->aComplRefDataRel.Ref1;
                pScTokenArray->Clear();

                pRef->SetColRel( FALSE );
                pRef->SetRowRel( FALSE );
                pRef->SetTabRel( TRUE );

                if( pFind->bSingleRef )
                    pScTokenArray->AddSingleReference( *pRef );
                else
                {
                    pRef = &pFind->aComplRefDataRel.Ref2;
                    pRef->SetColRel( FALSE );
                    pRef->SetRowRel( FALSE );
                    pRef->SetTabRel( TRUE );
                    pScTokenArray->AddDoubleReference( pFind->aComplRefDataRel );
                }

                ScRangeData*    pData = new ScRangeData( pLotusRoot->pDoc, pFind->aScAbsName, *pScTokenArray );

                rIndex = pFind->nAbsInd = nIntCount;
                pData->SetIndex( rIndex );
                nIntCount++;

                pLotusRoot->pScRangeName->Insert( pData );
            }

            return TRUE;
        }
        pFind = ( ENTRY* ) List::Next();
    }

    return FALSE;
}


