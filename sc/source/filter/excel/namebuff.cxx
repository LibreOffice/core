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

sal_uInt32 StringHashEntry::MakeHashCode( const String& r )
{
    sal_uInt32                 n = 0;
    const sal_Unicode*              pAkt = r.GetBuffer();
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
void NameBuffer::operator <<( const String &rNewString )
{
    OSL_ENSURE( maHashes.size() + nBase < 0xFFFF,
        "*NameBuffer::GetLastIndex(): Ich hab' die Nase voll!" );

    maHashes.push_back( new StringHashEntry( rNewString ) );
}

SharedFormulaBuffer::SharedFormulaBuffer( RootData* pRD ) : ExcRoot(pRD) {}

SharedFormulaBuffer::~SharedFormulaBuffer() {}

void SharedFormulaBuffer::Clear()
{
    maFormulaGroups.clear();
}

void SharedFormulaBuffer::Store( const ScRange& rRange, const ScTokenArray& rArray )
{
    SCROW nGroupLen = rRange.aEnd.Row() - rRange.aStart.Row() + 1;
    for (SCCOL i = rRange.aStart.Col(); i <= rRange.aEnd.Col(); ++i)
    {
        // Create one group per column.
        ScAddress aPos = rRange.aStart;
        aPos.SetCol(i);

        ScFormulaCellGroupRef xNewGroup(new ScFormulaCellGroup);
        xNewGroup->mnStart = rRange.aStart.Row();
        xNewGroup->mnLength = nGroupLen;
        xNewGroup->mpCode = rArray.Clone();
        xNewGroup->mbInvariant = rArray.IsInvariant();
        xNewGroup->setCode(rArray);
        maFormulaGroups.insert(FormulaGroupsType::value_type(aPos, xNewGroup));
    }
}

ScFormulaCellGroupRef SharedFormulaBuffer::Find( const ScAddress& rRefPos ) const
{
    FormulaGroupsType::const_iterator it = maFormulaGroups.find(rRefPos);
    if (it == maFormulaGroups.end())
        return ScFormulaCellGroupRef();

    return it->second;
}

sal_Int16 ExtSheetBuffer::Add( const String& rFPAN, const String& rTN, const sal_Bool bSWB )
{
    maEntries.push_back( Cont( rFPAN, rTN, bSWB ) );
    // return 1-based index of EXTERNSHEET
    return static_cast< sal_Int16 >( maEntries.size() );
}


sal_Bool ExtSheetBuffer::GetScTabIndex( sal_uInt16 nExcIndex, sal_uInt16& rScIndex )
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
        return sal_True;
    }

    if( rTabNum == 0xFFFF )
    {// neue Tabelle erzeugen
        SCTAB   nNewTabNum;
        if( pCur->bSWB )
        {// Tabelle ist im selben Workbook!
            if( pExcRoot->pIR->GetDoc().GetTable( pCur->aTab, nNewTabNum ) )
            {
                rScIndex = rTabNum = static_cast<sal_uInt16>(nNewTabNum);
                return sal_True;
            }
            else
                rTabNum = 0xFFFD;
        }
        else if( pExcRoot->pIR->GetDocShell() )
        {// Tabelle ist 'echt' extern
            if( pExcRoot->pIR->GetExtDocOptions().GetDocSettings().mnLinkCnt == 0 )
            {
                String      aURL( ScGlobal::GetAbsDocName( pCur->aFile,
                                    pExcRoot->pIR->GetDocShell() ) );
                String      aTabName( ScGlobal::GetDocTabName( aURL, pCur->aTab ) );
                if( pExcRoot->pIR->GetDoc().LinkExternalTab( nNewTabNum, aTabName, aURL, pCur->aTab ) )
                {
                    rScIndex = rTabNum = static_cast<sal_uInt16>(nNewTabNum);
                    return sal_True;
                }
                else
                    rTabNum = 0xFFFE;       // Tabelle einmal nicht angelegt -> wird
                                            //  wohl auch nicht mehr gehen...
            }
            else
                rTabNum = 0xFFFE;

        }
    }

    return false;
}


sal_Bool ExtSheetBuffer::IsLink( const sal_uInt16 nExcIndex ) const
{
    OSL_ENSURE( nExcIndex > 0, "*ExtSheetBuffer::IsLink(): Index has to be >0!" );

    if (!nExcIndex || nExcIndex > maEntries.size() )
        return false;

    return maEntries[ nExcIndex -1 ].bLink;
}


sal_Bool ExtSheetBuffer::GetLink( const sal_uInt16 nExcIndex, String& rAppl, String& rDoc ) const
{
    OSL_ENSURE( nExcIndex > 0, "*ExtSheetBuffer::GetLink(): Index has to be >0!" );

    if (!nExcIndex || nExcIndex > maEntries.size() )
        return false;

    const Cont &rRet = maEntries[ nExcIndex -1 ];

    rAppl = rRet.aFile;
    rDoc = rRet.aTab;

    return true;
}


void ExtSheetBuffer::Reset( void )
{
    maEntries.clear();
}




sal_Bool ExtName::IsDDE( void ) const
{
    return ( nFlags & 0x0001 ) != 0;
}


sal_Bool ExtName::IsOLE( void ) const
{
    return ( nFlags & 0x0002 ) != 0;
}


ExtNameBuff::ExtNameBuff( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}


void ExtNameBuff::AddDDE( const String& rName, sal_Int16 nRefIdx )
{
    ExtName aNew( rName, 0x0001 );
    maExtNames[ nRefIdx ].push_back( aNew );
}


void ExtNameBuff::AddOLE( const String& rName, sal_Int16 nRefIdx, sal_uInt32 nStorageId )
{
    ExtName aNew( rName, 0x0002 );
    aNew.nStorageId = nStorageId;
    maExtNames[ nRefIdx ].push_back( aNew );
}


void ExtNameBuff::AddName( const String& rName, sal_Int16 nRefIdx )
{
    ExtName aNew( GetScAddInName( rName ), 0x0004 );
    maExtNames[ nRefIdx ].push_back( aNew );
}


const ExtName* ExtNameBuff::GetNameByIndex( sal_Int16 nRefIdx, sal_uInt16 nNameIdx ) const
{
    OSL_ENSURE( nNameIdx > 0, "ExtNameBuff::GetNameByIndex() - invalid name index" );
    ExtNameMap::const_iterator aIt = maExtNames.find( nRefIdx );
    return ((aIt != maExtNames.end()) && (0 < nNameIdx) && (nNameIdx <= aIt->second.size())) ? &aIt->second[ nNameIdx - 1 ] : 0;
}


void ExtNameBuff::Reset( void )
{
    maExtNames.clear();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
