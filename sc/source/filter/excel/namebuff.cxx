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

#include "namebuff.hxx"

#include "document.hxx"
#include "compiler.hxx"
#include "scextopt.hxx"

#include "root.hxx"
#include "tokstack.hxx"
#include "xltools.hxx"
#include "xiroot.hxx"

#include <string.h>

sal_uInt32 StringHashEntry::MakeHashCode( const OUString& r )
{
    sal_uInt32             n = 0;
    const sal_Unicode*     pAkt = r.getStr();
    sal_Unicode            cAkt = *pAkt;

    while( cAkt )
    {
        n *= 70;
        n += ( sal_uInt32 ) cAkt;
        pAkt++;
        cAkt = *pAkt;
    }

    return n;
}

NameBuffer::~NameBuffer()
{
    std::vector<StringHashEntry*>::iterator pIter;
    for ( pIter = maHashes.begin(); pIter != maHashes.end(); ++pIter )
        delete *pIter;
}

//void NameBuffer::operator <<( const SpString &rNewString )
void NameBuffer::operator <<( const OUString &rNewString )
{
    OSL_ENSURE( maHashes.size() + nBase < 0xFFFF,
        "*NameBuffer::GetLastIndex(): Ich hab' die Nase voll!" );

    maHashes.push_back( new StringHashEntry( rNewString ) );
}

SharedFormulaBuffer::SharedFormulaBuffer( RootData* pRD ) : ExcRoot(pRD) {}

SharedFormulaBuffer::~SharedFormulaBuffer()
{
    Clear();
}

void SharedFormulaBuffer::Clear()
{
    TokenArraysType::iterator it = maTokenArrays.begin(), itEnd = maTokenArrays.end();
    for (; it != itEnd; ++it)
        delete it->second;

    maTokenArrays.clear();
}

void SharedFormulaBuffer::Store( const ScAddress& rPos, const ScTokenArray& rArray )
{
    ScTokenArray* pCode = rArray.Clone();
    pCode->GenHash();
    maTokenArrays.insert(TokenArraysType::value_type(rPos, pCode));
}

const ScTokenArray* SharedFormulaBuffer::Find( const ScAddress& rRefPos ) const
{
    TokenArraysType::const_iterator it = maTokenArrays.find(rRefPos);
    if (it == maTokenArrays.end())
        return nullptr;

    return it->second;
}

sal_Int16 ExtSheetBuffer::Add( const OUString& rFPAN, const OUString& rTN, const bool bSWB )
{
    maEntries.push_back( Cont( rFPAN, rTN, bSWB ) );
    // return 1-based index of EXTERNSHEET
    return static_cast< sal_Int16 >( maEntries.size() );
}

bool ExtSheetBuffer::GetScTabIndex( sal_uInt16 nExcIndex, sal_uInt16& rScIndex )
{
    OSL_ENSURE( nExcIndex,
        "*ExtSheetBuffer::GetScTabIndex(): Sheet-Index == 0!" );

    if ( !nExcIndex || nExcIndex > maEntries.size() )
        return false;

    Cont*       pCur = &maEntries[ nExcIndex - 1 ];
    sal_uInt16&     rTabNum = pCur->nTabNum;

    if( rTabNum < 0xFFFD )
    {
        rScIndex = rTabNum;
        return true;
    }

    if( rTabNum == 0xFFFF )
    {// create new table
        SCTAB   nNewTabNum;
        if( pCur->bSWB )
        {// table is in the same workbook!
            if( pExcRoot->pIR->GetDoc().GetTable( pCur->aTab, nNewTabNum ) )
            {
                rScIndex = rTabNum = static_cast<sal_uInt16>(nNewTabNum);
                return true;
            }
            else
                rTabNum = 0xFFFD;
        }
        else if( pExcRoot->pIR->GetDocShell() )
        {// table is 'really' external
            if( pExcRoot->pIR->GetExtDocOptions().GetDocSettings().mnLinkCnt == 0 )
            {
                OUString      aURL( ScGlobal::GetAbsDocName( pCur->aFile,
                                    pExcRoot->pIR->GetDocShell() ) );
                OUString      aTabName( ScGlobal::GetDocTabName( aURL, pCur->aTab ) );
                if( pExcRoot->pIR->GetDoc().LinkExternalTab( nNewTabNum, aTabName, aURL, pCur->aTab ) )
                {
                    rScIndex = rTabNum = static_cast<sal_uInt16>(nNewTabNum);
                    return true;
                }
                else
                    rTabNum = 0xFFFE;       // no table is created for now -> and likely
                                            //  will not be created later...
            }
            else
                rTabNum = 0xFFFE;

        }
    }

    return false;
}

bool ExtSheetBuffer::IsLink( const sal_uInt16 nExcIndex ) const
{
    OSL_ENSURE( nExcIndex > 0, "*ExtSheetBuffer::IsLink(): Index has to be >0!" );

    if (!nExcIndex || nExcIndex > maEntries.size() )
        return false;

    return maEntries[ nExcIndex -1 ].bLink;
}

bool ExtSheetBuffer::GetLink( const sal_uInt16 nExcIndex, OUString& rAppl, OUString& rDoc ) const
{
    OSL_ENSURE( nExcIndex > 0, "*ExtSheetBuffer::GetLink(): Index has to be >0!" );

    if (!nExcIndex || nExcIndex > maEntries.size() )
        return false;

    const Cont &rRet = maEntries[ nExcIndex -1 ];

    rAppl = rRet.aFile;
    rDoc = rRet.aTab;

    return true;
}

void ExtSheetBuffer::Reset()
{
    maEntries.clear();
}

bool ExtName::IsDDE() const
{
    return ( nFlags & 0x0001 ) != 0;
}

bool ExtName::IsOLE() const
{
    return ( nFlags & 0x0002 ) != 0;
}

ExtNameBuff::ExtNameBuff( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void ExtNameBuff::AddDDE( const OUString& rName, sal_Int16 nRefIdx )
{
    ExtName aNew( rName, 0x0001 );
    maExtNames[ nRefIdx ].push_back( aNew );
}

void ExtNameBuff::AddOLE( const OUString& rName, sal_Int16 nRefIdx, sal_uInt32 nStorageId )
{
    ExtName aNew( rName, 0x0002 );
    aNew.nStorageId = nStorageId;
    maExtNames[ nRefIdx ].push_back( aNew );
}

void ExtNameBuff::AddName( const OUString& rName, sal_Int16 nRefIdx )
{
    ExtName aNew( GetScAddInName( rName ), 0x0004 );
    maExtNames[ nRefIdx ].push_back( aNew );
}

const ExtName* ExtNameBuff::GetNameByIndex( sal_Int16 nRefIdx, sal_uInt16 nNameIdx ) const
{
    OSL_ENSURE( nNameIdx > 0, "ExtNameBuff::GetNameByIndex() - invalid name index" );
    ExtNameMap::const_iterator aIt = maExtNames.find( nRefIdx );
    return ((aIt != maExtNames.end()) && (0 < nNameIdx) && (nNameIdx <= aIt->second.size())) ? &aIt->second[ nNameIdx - 1 ] : nullptr;
}

void ExtNameBuff::Reset()
{
    maExtNames.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
