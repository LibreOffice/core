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

#include <scitems.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/zforlist.hxx>
#include <sal/log.hxx>

#include <attrib.hxx>
#include <document.hxx>

#include <tool.h>
#include <root.hxx>
#include <lotrange.hxx>
#include <namebuff.hxx>
#include <stringutil.hxx>
#include <tokenarray.hxx>
#include "lotfilter.hxx"

#include <math.h>

void PutFormString(LotusContext& rContext, SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Char* pString)
{
    // evaluate Label-Format
    SAL_WARN_IF( pString == nullptr, "sc.filter", "PutFormString(): pString == NULL" );
    if (!pString)
        return;

    sal_Char            cForm;
    SvxHorJustifyItem*  pJustify = nullptr;

    cForm = *pString;

    switch( cForm )
    {
        case '"':   // align-right
            pJustify = rContext.pAttrRight;
            pString++;
            break;
        case '\'':  // align-left
            pJustify = rContext.pAttrLeft;
            pString++;
            break;
        case '^':   // centered
            pJustify = rContext.pAttrCenter;
            pString++;
            break;
        case '|':   // printer command
            pString = nullptr;
            break;
        case '\\':  // repetition
            pJustify = rContext.pAttrRepeat;
            pString++;
            break;
        default:    // undefined case!
            pJustify = rContext.pAttrStandard;
    }

    if (!pString)
        return;

    nCol = SanitizeCol(nCol);
    nRow = SanitizeRow(nRow);
    nTab = SanitizeTab(nTab);

    rContext.pDoc->ApplyAttr( nCol, nRow, nTab, *pJustify );
    ScSetStringParam aParam;
    aParam.setTextInput();
    rContext.pDoc->SetString(ScAddress(nCol,nRow,nTab), OUString(pString, strlen(pString), rContext.pLotusRoot->eCharsetQ), &aParam);
}

void SetFormat(LotusContext& rContext, SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt8 nFormat, sal_uInt8 nSt)
{
    nCol = SanitizeCol(nCol);
    nRow = SanitizeRow(nRow);
    nTab = SanitizeTab(nTab);

    //  PREC:   nSt = default number of decimal places
    rContext.pDoc->ApplyAttr(nCol, nRow, nTab, *(rContext.pValueFormCache->GetAttr(nFormat, nSt)));

    ScProtectionAttr aAttr;

    aAttr.SetProtection( nFormat & 0x80 );

    rContext.pDoc->ApplyAttr( nCol, nRow, nTab, aAttr );
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
        fVal *= static_cast<sal_Int16>( nVal >> 4 );
    }
    else
        fVal = static_cast<sal_Int16>( nVal >> 1 );

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
                fValue /= pow(double(10), temp);
        else
        fValue *= pow(double(10), temp);
    }

    if (nValue & 0x00000020)
        fValue = -fValue;
    return fValue;
}

FormCache::FormCache( const ScDocument* pDoc1 )
    : nIndex(0)
{
    pFormTable = pDoc1->GetFormatTable();
    for(bool & rb : bValid)
        rb = false;
    eLanguage = ScGlobal::eLnge;
}

FormCache::~FormCache()
{
}

SfxUInt32Item* FormCache::NewAttr( sal_uInt8 nFormat, sal_uInt8 nSt )
{
    // setup new Format
    sal_uInt8       nL, nH; // Low-/High-Nibble
    OUString        aFormString;
    SvNumFormatType eType = SvNumFormatType::ALL;
    sal_uInt32      nIndex1;
    sal_uInt32      nHandle;
    NfIndexTableOffset eIndexTableOffset = NF_NUMERIC_START;
    bool            bDefault = false;

    // split into Low and High byte
    nL = nFormat & 0x0F;
    nH = ( nFormat & 0xF0 ) / 16;

    nH &= 0x07;     // extract bits 4-6
    switch( nH )
    {
        case 0x00:  // fixed-point number
            //fStandard;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                SvNumFormatType::NUMBER, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL);
            break;
        case 0x01:  // scientific notation
            //fExponent;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                SvNumFormatType::SCIENTIFIC, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL);
            break;
        case 0x02:  // currency
            //fMoney;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                SvNumFormatType::CURRENCY, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL);
            break;
        case 0x03:  // percentage
            //fPercent;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                SvNumFormatType::PERCENT, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL);
            break;
        case 0x04:  // Decimal
            //fStandard;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                SvNumFormatType::NUMBER, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, true, false, nL);
            break;
        case 0x05:  // unspecified
            //fStandard;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                SvNumFormatType::NUMBER, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL);
            break;
        case 0x06:  // unspecified
            //fStandard;nL;
            nIndex1 = pFormTable->GetStandardFormat(
                SvNumFormatType::NUMBER, eLanguage );
            aFormString = pFormTable->GenerateFormat(nIndex1,
                eLanguage, false, false, nL);
            break;
        case 0x07:  // Special format
            switch( nL )
            {
                case 0x00:  // +/-
                    //fStandard;nSt;
                    nIndex1 = pFormTable->GetStandardFormat(
                        SvNumFormatType::NUMBER, eLanguage );
                    aFormString = pFormTable->GenerateFormat(nIndex1,
                        eLanguage, false, true, nSt);
                    break;
                case 0x01:  // general Format
                    //fStandard;nSt;
                    nIndex1 = pFormTable->GetStandardFormat(
                        SvNumFormatType::NUMBER, eLanguage );
                    aFormString = pFormTable->GenerateFormat(nIndex1,
                        eLanguage, false, false, nSt);
                    break;
                case 0x02:  // Date: Day, Month, Year
                    //fDate;dfDayMonthYearLong;
                    eType = SvNumFormatType::DATE;
                    eIndexTableOffset = NF_DATE_SYS_DDMMYYYY;
                    break;
                case 0x03:  // Date: Day, Month
                    //fDate;dfDayMonthLong;
                    eType = SvNumFormatType::DATE;
                    aFormString = pFormTable->GetKeyword( eLanguage, NF_KEY_DD);
                    aFormString += pFormTable->GetDateSep();    // matches last eLanguage
                    aFormString += pFormTable->GetKeyword( eLanguage, NF_KEY_MMMM);
                    break;
                case 0x04:  // Date: Month, Year
                    //fDate;dfMonthYearLong;
                    eType = SvNumFormatType::DATE;
                    aFormString = pFormTable->GetKeyword( eLanguage, NF_KEY_MM);
                    aFormString += pFormTable->GetDateSep();    // matches last eLanguage
                    aFormString += pFormTable->GetKeyword( eLanguage, NF_KEY_YYYY);
                    break;
                case 0x05:  // Text formats
                    //fString;nSt;
                    eType = SvNumFormatType::TEXT;
                    eIndexTableOffset = NF_TEXT;
                    break;
                case 0x06:  // hidden
                    //wFlag |= paHideAll;bSetFormat = sal_False;
                    eType = SvNumFormatType::NUMBER;
                    aFormString = "\"\"";
                    break;
                case 0x07:  // Time: hour, min, sec
                    //fTime;tfHourMinSec24;
                    eType = SvNumFormatType::TIME;
                    eIndexTableOffset = NF_TIME_HHMMSS;
                    break;
                case 0x08:  // Time: hour, min
                    //fTime;tfHourMin24;
                    eType = SvNumFormatType::TIME;
                    eIndexTableOffset = NF_TIME_HHMM;
                    break;
                case 0x09:  // Date, intern sal_Int32 1
                    //fDate;dfDayMonthYearLong;
                    eType = SvNumFormatType::DATE;
                    eIndexTableOffset = NF_DATE_SYS_DDMMYYYY;
                    break;
                case 0x0A:  // Date, intern sal_Int32 2
                    //fDate;dfDayMonthYearLong;
                    eType = SvNumFormatType::DATE;
                    eIndexTableOffset = NF_DATE_SYS_DDMMYYYY;
                    break;
                case 0x0B:  // Time, intern sal_Int32 1
                    //fTime;tfHourMinSec24;
                    eType = SvNumFormatType::TIME;
                    eIndexTableOffset = NF_TIME_HHMMSS;
                    break;
                case 0x0C:  // Time, intern sal_Int32 2
                    //fTime;tfHourMinSec24;
                    eType = SvNumFormatType::TIME;
                    eIndexTableOffset = NF_TIME_HHMMSS;
                    break;
                case 0x0F:  // Default
                    //fStandard;nSt;
                    bDefault = true;
                    break;
                default:
                    //fStandard;nSt;
                    bDefault = true;
                    break;
            }
            break;
    }

    // push Format into table
    if( bDefault )
        nHandle = 0;
    else if (eIndexTableOffset != NF_NUMERIC_START)
        nHandle = pFormTable->GetFormatIndex( eIndexTableOffset, eLanguage);
    else
    {
        sal_Int32  nDummy;
        pFormTable->PutEntry( aFormString, nDummy, eType, nHandle, eLanguage );
    }

    return new SfxUInt32Item( ATTR_VALUE_FORMAT, nHandle );
}

void LotusRange::MakeHash()
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

LotusRangeList::LotusRangeList()
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
}

LR_ID LotusRangeList::GetIndex( const LotusRange &rRef )
{
    auto pIter = std::find_if(maRanges.begin(), maRanges.end(),
        [&rRef](const std::unique_ptr<LotusRange>& pRange) { return rRef == *pRange; });
    if (pIter != maRanges.end())
        return (*pIter)->nId;

    return ID_FAIL;
}

void LotusRangeList::Append( std::unique_ptr<LotusRange> pLR )
{
    assert( pLR );
    auto pLRTmp = pLR.get();
    maRanges.push_back(std::move(pLR));

    ScTokenArray    aTokArray;

    ScSingleRefData*    pSingRef = &aComplRef.Ref1;

    pSingRef->SetAbsCol(pLRTmp->nColStart);
    pSingRef->SetAbsRow(pLRTmp->nRowStart);

    if( pLRTmp->IsSingle() )
        aTokArray.AddSingleReference( *pSingRef );
    else
    {
        pSingRef = &aComplRef.Ref2;
        pSingRef->SetAbsCol(pLRTmp->nColEnd);
        pSingRef->SetAbsRow(pLRTmp->nRowEnd);
        aTokArray.AddDoubleReference( aComplRef );
    }

    pLRTmp->SetId( nIdCnt );

    nIdCnt++;
}

RangeNameBufferWK3::RangeNameBufferWK3()
    : pScTokenArray( new ScTokenArray )
{
    nIntCount = 1;
}

RangeNameBufferWK3::~RangeNameBufferWK3()
{
}

void RangeNameBufferWK3::Add( const OUString& rOrgName, const ScComplexRefData& rCRD )
{
    Entry aInsert( rOrgName, rCRD );

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

    aInsert.nRelInd = nIntCount;
    nIntCount++;

    maEntries.push_back( aInsert );
}

bool RangeNameBufferWK3::FindRel( const OUString& rRef, sal_uInt16& rIndex )
{
    StringHashEntry     aRef( rRef );

    std::vector<Entry>::const_iterator itr = std::find_if(maEntries.begin(), maEntries.end(),
        [&aRef](const Entry& rEntry) { return aRef == rEntry.aStrHashEntry; });
    if (itr != maEntries.end())
    {
        rIndex = itr->nRelInd;
        return true;
    }

    return false;
}

bool RangeNameBufferWK3::FindAbs( const OUString& rRef, sal_uInt16& rIndex )
{
    OUString            aTmp( rRef );
    aTmp = aTmp.copy(1);
    StringHashEntry     aRef( aTmp ); // search w/o '$'!

    std::vector<Entry>::iterator itr = std::find_if(maEntries.begin(), maEntries.end(),
        [&aRef](const Entry& rEntry) { return aRef == rEntry.aStrHashEntry; });
    if (itr != maEntries.end())
    {
        // setup new range if needed
        if( itr->nAbsInd )
            rIndex = itr->nAbsInd;
        else
        {
            ScSingleRefData*        pRef = &itr->aScComplexRefDataRel.Ref1;
            pScTokenArray->Clear();

            pRef->SetColRel( false );
            pRef->SetRowRel( false );
            pRef->SetTabRel( true );

            if( itr->bSingleRef )
                pScTokenArray->AddSingleReference( *pRef );
            else
            {
                pRef = &itr->aScComplexRefDataRel.Ref2;
                pRef->SetColRel( false );
                pRef->SetRowRel( false );
                pRef->SetTabRel( true );
                pScTokenArray->AddDoubleReference( itr->aScComplexRefDataRel );
            }

            rIndex = itr->nAbsInd = nIntCount;
            nIntCount++;
        }

        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
