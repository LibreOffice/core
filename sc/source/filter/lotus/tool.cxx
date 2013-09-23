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

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/zforlist.hxx>
#include <tools/solar.h>

#include "rangenam.hxx"
#include "compiler.hxx"

#include "tool.h"
#include "decl.h"
#include "root.hxx"
#include "lotrange.hxx"
#include "namebuff.hxx"
#include "ftools.hxx"
#include "stringutil.hxx"
#include "tokenarray.hxx"

#include <math.h>

//--------------------------------------------------------- EXTERNE VARIABLEN -
extern WKTYP                eTyp;           // -> filter.cxx, aktueller Dateityp
extern ScDocument*          pDoc;           // -> filter.cxx, Aufhaenger zum Dokumentzugriff

//--------------------------------------------------------- GLOBALE VARIABLEN -
sal_uInt8                       nDefaultFormat; // -> op.cpp, Standard-Zellenformat

extern SvxHorJustifyItem    *pAttrRight, *pAttrLeft, *pAttrCenter, *pAttrRepeat, *pAttrStandard;
extern ScProtectionAttr*    pAttrUnprot;
extern SfxUInt32Item**      pAttrValForms;

SvxHorJustifyItem           *pAttrRight, *pAttrLeft, *pAttrCenter, *pAttrRepeat, *pAttrStandard;
                                                    // -> in memory.cxx initialisiert
ScProtectionAttr*           pAttrUnprot;            // ->  " memory.cxx    "

extern FormCache*           pValueFormCache;        // -> in memory.cxx initialisiert
FormCache*                  pValueFormCache;

SCCOL                       LotusRangeList::nEingCol;
SCROW                       LotusRangeList::nEingRow;




void PutFormString( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Char* pString )
{
    // Label-Format-Auswertung
    OSL_ENSURE( pString != NULL, "PutFormString(): pString == NULL" );
    if (!pString)
        return;

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

    pDoc->ApplyAttr( nCol, nRow, nTab, *pJustify );
    ScSetStringParam aParam;
    aParam.setTextInput();
    pDoc->SetString(ScAddress(nCol,nRow,nTab), String(pString, pLotusRoot->eCharsetQ), &aParam);
}




void SetFormat( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt8 nFormat, sal_uInt8 nSt )
{
    //  PREC:   nSt = Standard-Dezimalstellenanzahl
    pDoc->ApplyAttr( nCol, nRow, nTab, *( pValueFormCache->GetAttr( nFormat, nSt ) ) );

    ScProtectionAttr aAttr;

    aAttr.SetProtection( nFormat & 0x80 );

    pDoc->ApplyAttr( nCol, nRow, nTab, aAttr );
}

void InitPage( void )
{   // Seitenformat initialisieren, d.h. Default-Werte von SC holen
    //scGetPageFormat( 0, &aPage );
}


double SnumToDouble( sal_Int16 nVal )
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
        fVal *= ( sal_Int16 ) ( nVal >> 4 );
    }
    else
        fVal = ( sal_Int16 ) ( nVal >> 1 );

    return fVal;
}

double Snum32ToDouble( sal_uInt32 nValue )
{
    double fValue, temp;

    fValue = nValue >> 6;
    temp = nValue & 0x0f;
    if (temp)
    {
        if (nValue & 0x00000010)
                fValue /= pow((double)10, temp);
        else
        fValue *= pow((double)10, temp);
    }

    if ((nValue & 0x00000020))
        fValue = -fValue;
    return fValue;
}


FormCache::FormCache( ScDocument* pDoc1, sal_uInt8 nNewDefaultFormat )
{   // Default-Format ist 'Default'
    nDefaultFormat = nNewDefaultFormat;
    pFormTable = pDoc1->GetFormatTable();
    for( sal_uInt16 nC = 0 ; nC < __nSize ; nC++ )
        bValid[ nC ] = false;
    eLanguage = ScGlobal::eLnge;
}


FormCache::~FormCache()
{
    for( sal_uInt16 nC = 0 ; nC < __nSize ; nC++ )
        delete aIdents[ nC ].GetAttr();
}


SfxUInt32Item* FormCache::NewAttr( sal_uInt8 nFormat, sal_uInt8 nSt )
{
    // neues Format erzeugen
    sal_uInt8       nL, nH; // Low-/High-Nibble
    sal_uInt8       nForm = nFormat;
    OUString          aFormString;
    sal_Int16       eType = NUMBERFORMAT_ALL;
    sal_uInt32      nIndex1;
    sal_uInt32      nHandle;
    NfIndexTableOffset eIndexTableOffset = NF_NUMERIC_START;
    sal_Bool        bDefault = false;

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
            nIndex1 = pFormTable->GetStandardFormat(
                NUMBERFORMAT_NUMBER, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL, 1);
            break;
        case 0x01:  // Exponentdarstellung (scientific notation)
            //fExponent;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                NUMBERFORMAT_SCIENTIFIC, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL, 1);
            break;
        case 0x02:  // Waehrungsdarstellung (currency)
            //fMoney;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                NUMBERFORMAT_CURRENCY, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL, 1);
            break;
        case 0x03:  // Prozent
            //fPercent;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                NUMBERFORMAT_PERCENT, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL, 1);
            break;
        case 0x04:  // Komma
            //fStandard;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                NUMBERFORMAT_NUMBER, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, sal_True, false, nL, 1);
            break;
        case 0x05:  // frei
            //fStandard;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                NUMBERFORMAT_NUMBER, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL, 1);
            break;
        case 0x06:  // frei
            //fStandard;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                NUMBERFORMAT_NUMBER, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL, 1);
            nIndex1 = 0;
            break;
        case 0x07:  // Spezialformat
            switch( nL )
            {
                case 0x00:  // +/-
                    //fStandard;nSt;
                    nIndex1 = pFormTable->GetStandardFormat(
                        NUMBERFORMAT_NUMBER, eLanguage );
                    aFormString = pFormTable->GenerateFormat(nIndex1,
                        eLanguage, false, sal_True, nSt, 1);
                    break;
                case 0x01:  // generelles Format
                    //fStandard;nSt;
                    nIndex1 = pFormTable->GetStandardFormat(
                        NUMBERFORMAT_NUMBER, eLanguage );
                    aFormString = pFormTable->GenerateFormat(nIndex1,
                        eLanguage, false, false, nSt, 1);
                    break;
                case 0x02:  // Datum: Tag, Monat, Jahr
                    //fDate;dfDayMonthYearLong;
                    eType = NUMBERFORMAT_DATE;
                    eIndexTableOffset = NF_DATE_SYS_DDMMYYYY;
                    break;
                case 0x03:  // Datum: Tag, Monat
                    //fDate;dfDayMonthLong;
                    eType = NUMBERFORMAT_DATE;
                    aFormString = pFormTable->GetKeyword( eLanguage, NF_KEY_DD);
                    aFormString += pFormTable->GetDateSep();    // matches last eLanguage
                    aFormString += pFormTable->GetKeyword( eLanguage, NF_KEY_MMMM);
                    break;
                case 0x04:  // Datum: Monat, Jahr
                    //fDate;dfMonthYearLong;
                    eType = NUMBERFORMAT_DATE;
                    aFormString = pFormTable->GetKeyword( eLanguage, NF_KEY_MM);
                    aFormString += pFormTable->GetDateSep();    // matches last eLanguage
                    aFormString += pFormTable->GetKeyword( eLanguage, NF_KEY_YYYY);
                    break;
                case 0x05:  // Textformate
                    //fString;nSt;
                    eType = NUMBERFORMAT_TEXT;
                    eIndexTableOffset = NF_TEXT;
                    break;
                case 0x06:  // versteckt
                    //wFlag |= paHideAll;bSetFormat = sal_False;
                    eType = NUMBERFORMAT_NUMBER;
                    aFormString = "\"\"";
                    break;
                case 0x07:  // Time: hour, min, sec
                    //fTime;tfHourMinSec24;
                    eType = NUMBERFORMAT_TIME;
                    eIndexTableOffset = NF_TIME_HHMMSS;
                    break;
                case 0x08:  // Time: hour, min
                    //fTime;tfHourMin24;
                    eType = NUMBERFORMAT_TIME;
                    eIndexTableOffset = NF_TIME_HHMM;
                    break;
                case 0x09:  // Date, intern sal_Int32 1
                    //fDate;dfDayMonthYearLong;
                    eType = NUMBERFORMAT_DATE;
                    eIndexTableOffset = NF_DATE_SYS_DDMMYYYY;
                    break;
                case 0x0A:  // Date, intern sal_Int32 2
                    //fDate;dfDayMonthYearLong;
                    eType = NUMBERFORMAT_DATE;
                    eIndexTableOffset = NF_DATE_SYS_DDMMYYYY;
                    break;
                case 0x0B:  // Time, intern sal_Int32 1
                    //fTime;tfHourMinSec24;
                    eType = NUMBERFORMAT_TIME;
                    eIndexTableOffset = NF_TIME_HHMMSS;
                    break;
                case 0x0C:  // Time, intern sal_Int32 2
                    //fTime;tfHourMinSec24;
                    eType = NUMBERFORMAT_TIME;
                    eIndexTableOffset = NF_TIME_HHMMSS;
                    break;
                case 0x0F:  // Standardeinstellung
                    //fStandard;nSt;
                    bDefault = sal_True;
                    break;
                default:
                    //fStandard;nSt;
                    bDefault = sal_True;
                    break;
            }
            break;
        default:
            //fStandard;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                NUMBERFORMAT_NUMBER, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL, 1);
            nIndex1 = 0;
            break;
    }

    // Format in Table schieben
    if( bDefault )
        nHandle = 0;
    else if (eIndexTableOffset != NF_NUMERIC_START)
        nHandle = pFormTable->GetFormatIndex( eIndexTableOffset, eLanguage);
    else
    {
        sal_Int32  nDummy;
        pFormTable->PutEntry( aFormString, nDummy, eType, nHandle, eLanguage );
    }

    return new SfxUInt32Item( ATTR_VALUE_FORMAT, ( sal_uInt32 ) nHandle );
}




void LotusRange::MakeHash( void )
{
    // 33222222222211111111110000000000
    // 10987654321098765432109876543210
    //                         ******** nColS
    //                   ********       nColE
    //     ****************             nRowS
    // ****************                 nRowE
    nHash =  static_cast<sal_uInt32>(nColStart);
    nHash += static_cast<sal_uInt32>(nColEnd) << 6;
    nHash += static_cast<sal_uInt32>(nRowStart) << 12;
    nHash += static_cast<sal_uInt32>(nRowEnd ) << 16;
}


LotusRange::LotusRange( SCCOL nCol, SCROW nRow )
{
    nColStart = nColEnd = nCol;
    nRowStart = nRowEnd = nRow;
    nId = ID_FAIL;
    MakeHash();
}


LotusRange::LotusRange( SCCOL nCS, SCROW nRS, SCCOL nCE, SCROW nRE )
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

    ScSingleRefData*    pSingRef;
    nIdCnt = 1;

    pSingRef = &aComplRef.Ref1;
    pSingRef->SetRelTab(0);
    pSingRef->SetColRel( false );
    pSingRef->SetRowRel( false );
    pSingRef->SetFlag3D( false );

    pSingRef = &aComplRef.Ref2;
    pSingRef->SetRelTab(0);
    pSingRef->SetColRel( false );
    pSingRef->SetRowRel( false );
    pSingRef->SetFlag3D( false );
}

LotusRangeList::~LotusRangeList ()
{
    std::vector<LotusRange*>::iterator pIter;
    for (pIter = maRanges.begin(); pIter != maRanges.end(); ++pIter)
        delete (*pIter);
}

LR_ID LotusRangeList::GetIndex( const LotusRange &rRef )
{
    std::vector<LotusRange*>::iterator pIter;
    for (pIter = maRanges.begin(); pIter != maRanges.end(); ++pIter)
    {
        if (rRef == *(*pIter))
            return (*pIter)->nId;
    }

    return ID_FAIL;
}


void LotusRangeList::Append( LotusRange* pLR, const String& rName )
{
    OSL_ENSURE( pLR, "*LotusRangeList::Append(): das wird nichts!" );
    maRanges.push_back(pLR);

    ScTokenArray    aTokArray;

    ScSingleRefData*    pSingRef = &aComplRef.Ref1;

    pSingRef->SetAbsCol(pLR->nColStart);
    pSingRef->SetAbsRow(pLR->nRowStart);

    if( pLR->IsSingle() )
        aTokArray.AddSingleReference( *pSingRef );
    else
    {
        pSingRef = &aComplRef.Ref2;
        pSingRef->SetAbsCol(pLR->nColEnd);
        pSingRef->SetAbsRow(pLR->nRowEnd);
        aTokArray.AddDoubleReference( aComplRef );
    }

    ScRangeData*    pData = new ScRangeData(
        pLotusRoot->pDoc, rName, aTokArray );

    pLotusRoot->pScRangeName->insert( pData );

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
    delete pScTokenArray;
}


void RangeNameBufferWK3::Add( const String& rOrgName, const ScComplexRefData& rCRD )
{
    String aScName = ScfTools::ConvertToScDefinedName(rOrgName);

    Entry aInsert( rOrgName, aScName, rCRD );

    pScTokenArray->Clear();

    const ScSingleRefData& rRef1 = rCRD.Ref1;
    const ScSingleRefData& rRef2 = rCRD.Ref2;
    ScAddress aAbs1 = rRef1.toAbs(ScAddress());
    ScAddress aAbs2 = rRef2.toAbs(ScAddress());
    if (aAbs1 == aAbs2)
    {
        pScTokenArray->AddSingleReference( rCRD.Ref1 );
        aInsert.bSingleRef = true;
    }
    else
    {
        pScTokenArray->AddDoubleReference( rCRD );
        aInsert.bSingleRef = false;
    }

    ScRangeData*        pData = new ScRangeData( pLotusRoot->pDoc, aScName, *pScTokenArray );

    aInsert.nRelInd = nIntCount;
    pData->SetIndex( nIntCount );
    nIntCount++;

    maEntries.push_back( aInsert );
    pLotusRoot->pScRangeName->insert( pData );
}


sal_Bool RangeNameBufferWK3::FindRel( const String& rRef, sal_uInt16& rIndex )
{
    StringHashEntry     aRef( rRef );

    std::vector<Entry>::const_iterator itr;
    for ( itr = maEntries.begin(); itr != maEntries.end(); ++itr )
    {
        if ( aRef == itr->aStrHashEntry )
        {
            rIndex = itr->nRelInd;
            return true;
        }
    }

    return false;
}


sal_Bool RangeNameBufferWK3::FindAbs( const String& rRef, sal_uInt16& rIndex )
{
    String              aTmp( rRef );
    StringHashEntry     aRef( aTmp.Erase( 0, 1 ) ); // ohne '$' suchen!

    std::vector<Entry>::iterator itr;
    for ( itr = maEntries.begin(); itr != maEntries.end(); ++itr )
    {
        if ( aRef == itr->aStrHashEntry )
        {
            // eventuell neuen Range Name aufbauen
            if( itr->nAbsInd )
                rIndex = itr->nAbsInd;
            else
            {
                ScSingleRefData*        pRef = &itr->aScComplexRefDataRel.Ref1;
                pScTokenArray->Clear();

                pRef->SetColRel( false );
                pRef->SetRowRel( false );
                pRef->SetTabRel( sal_True );

                if( itr->bSingleRef )
                    pScTokenArray->AddSingleReference( *pRef );
                else
                {
                    pRef = &itr->aScComplexRefDataRel.Ref2;
                    pRef->SetColRel( false );
                    pRef->SetRowRel( false );
                    pRef->SetTabRel( sal_True );
                    pScTokenArray->AddDoubleReference( itr->aScComplexRefDataRel );
                }

                ScRangeData*    pData = new ScRangeData( pLotusRoot->pDoc, itr->aScAbsName, *pScTokenArray );

                rIndex = itr->nAbsInd = nIntCount;
                pData->SetIndex( rIndex );
                nIntCount++;

                pLotusRoot->pScRangeName->insert( pData );
            }

            return true;
        }
    }

    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
