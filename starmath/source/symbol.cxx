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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_starmath.hxx"


#include <vector>
#include <osl/mutex.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/msgbox.hxx>

#ifndef _SV_RESARY_HXX
#include <tools/resary.hxx>
#endif
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>

#include "symbol.hxx"
#include "view.hxx"
#include "utility.hxx"
#include "dialog.hxx"
#include "config.hxx"
#include "cfgitem.hxx"
#include "smmod.hxx"
#include "starmath.hrc"


using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

// Das hier muss auch mal alles "uberarbeitet werden. Insbesondere die nicht
// funktionierende und bei l"oschen/"andern von Symbolen nicht gepflegte
// Hash Tabelle!!!  Diese aktualisert sich erst im Wertzuweisungsoperator
// beim Verlassen des 'SmSymDefineDialog's!

/**************************************************************************/
/*
**
**  MACRO DEFINTION
**
**/

#define SF_SM20IDENT 0x03031963L
#define SF_IDENT     0x30334D53L


SV_IMPL_PTRARR( SymbolArray, SmSym * );

/**************************************************************************/
/*
**
**  DATA DEFINITION
**
**/

long                SF_Ident = SF_IDENT;

/**************************************************************************/
/*
**
**  CLASS IMPLEMENTATION
**
**/

SmSym::SmSym() :
    Name(C2S("unknown")),
    aSetName(C2S("unknown")),
    pHashNext(0),
    pSymSetManager(0),
    Character('\0'),
    bPredefined(FALSE),
    bDocSymbol(FALSE)
{
    aExportName = Name;
    Face.SetTransparent(TRUE);
    Face.SetAlign(ALIGN_BASELINE);
}


SmSym::SmSym(const SmSym& rSymbol)
{
    pSymSetManager = 0;
    *this = rSymbol;
}


SmSym::SmSym(const String& rName, const Font& rFont, sal_Unicode aChar,
             const String& rSet, BOOL bIsPredefined)
{
    Name        = aExportName   = rName;

    Face        = rFont;
    Face.SetTransparent(TRUE);
    Face.SetAlign(ALIGN_BASELINE);

    Character   = aChar;
//! according to HDU this should not be used anymore now
//! since this was necessary in the early days but should
//! not be done now since this is handled now at a more
//! bottom layer by HDU.
//! He can still imagine scenarios where this will be wrong
//! now though, for example when importing *some* old documents.
//! But overall it should be a large improvement, and
//! likely everything will still work... #_- (eyes shut and "go"!)
//
//    if (RTL_TEXTENCODING_SYMBOL == rFont.GetCharSet())
//        Character |= 0xF000;
    aSetName    = rSet;
    bPredefined = bIsPredefined;
    bDocSymbol  = FALSE;

    pHashNext      = 0;
    pSymSetManager = 0;
}


SmSym& SmSym::operator = (const SmSym& rSymbol)
{
    Name        = rSymbol.Name;
    Face        = rSymbol.Face;
    Character   = rSymbol.Character;
    aSetName    = rSymbol.aSetName;
    bPredefined = rSymbol.bPredefined;
    bDocSymbol  = rSymbol.bDocSymbol;
    aExportName = rSymbol.aExportName;

    pHashNext = 0;

    if (pSymSetManager)
        pSymSetManager->SetModified(TRUE);

    return *this;
}

/**************************************************************************/

SmSymSet::SmSymSet() :
    Name(C2S("unknown")),
    pSymSetManager(0)
{
    SymbolList.Clear();
}

SmSymSet::SmSymSet(const SmSymSet& rSymbolSet)
{
    pSymSetManager = 0;
    *this = rSymbolSet;
}

SmSymSet::SmSymSet(const String& rName)
{
    Name = rName;
    SymbolList.Clear();

    pSymSetManager = 0;
}

SmSymSet::~SmSymSet()
{
    for (USHORT i = 0; i < GetCount(); i++)
        delete SymbolList.GetObject(i);
}

SmSymSet& SmSymSet::operator = (const SmSymSet& rSymbolSet)
{
    USHORT i;
    for (i = 0; i < GetCount(); i++)
        delete SymbolList.GetObject(i);

    Name = rSymbolSet.Name;
    SymbolList.Clear();
    for (i = 0; i < rSymbolSet.GetCount(); i++)
        AddSymbol(new SmSym(rSymbolSet.GetSymbol(i)));

    if (pSymSetManager)
        pSymSetManager->SetModified(TRUE);

    return *this;
}

USHORT SmSymSet::AddSymbol(SmSym* pSymbol)
{
    DBG_ASSERT(pSymbol, "Kein Symbol");

    if (pSymbol)
        pSymbol->SetSetName( GetName() );
    SymbolList.Insert(pSymbol, LIST_APPEND);
    DBG_ASSERT(SymbolList.GetPos(pSymbol) == SymbolList.Count() - 1,
        "Sm : ... ergibt falschen return Wert");

    if (pSymSetManager)
        pSymSetManager->SetModified(TRUE);

    return (USHORT) SymbolList.Count() - 1;
}

void SmSymSet::DeleteSymbol(USHORT SymbolNo)
{
    delete RemoveSymbol(SymbolNo);
}

SmSym * SmSymSet::RemoveSymbol(USHORT SymbolNo)
{
    DBG_ASSERT(SymbolList.GetObject(SymbolNo), "Symbol nicht vorhanden");

    SmSym *pSym = SymbolList.GetObject(SymbolNo);
    SymbolList.Remove(SymbolNo);

    if (pSymSetManager)
        pSymSetManager->SetModified(TRUE);

    return pSym;
}

USHORT SmSymSet::GetSymbolPos(const String& rName)
{
    for (USHORT i = 0; i < GetCount(); i++)
        if (SymbolList.GetObject(i)->GetName() == rName)
            return (i);

    return SYMBOL_NONE;
}

/**************************************************************************/

SmSymSetManager_Impl::SmSymSetManager_Impl(
        SmSymSetManager &rMgr, USHORT HashTableSize ) :

    rSymSetMgr    (rMgr)
{
    NoSymbolSets    = 0;
    NoHashEntries   = HashTableSize;
    HashEntries     = new SmSym *[NoHashEntries];
    memset( HashEntries, 0, sizeof(SmSym *) * NoHashEntries );
    Modified        = FALSE;
}


SmSymSetManager_Impl::~SmSymSetManager_Impl()
{
    for (USHORT i = 0;  i < NoSymbolSets;  ++i)
        delete SymbolSets.Get(i);
    SymbolSets.Clear();

    NoSymbolSets = 0;
    if (HashEntries)
    {
        delete[] HashEntries;
        HashEntries = 0;
    }
    NoHashEntries = 0;
    Modified = FALSE;
}


SmSymSetManager_Impl & SmSymSetManager_Impl::operator = ( const SmSymSetManager_Impl &rImpl )
{
    //! rMySymSetMgr remains unchanged

    NoHashEntries   = rImpl.NoHashEntries;
    if (HashEntries)
        delete [] HashEntries;
    HashEntries = new SmSym *[NoHashEntries];
    memset( HashEntries, 0, sizeof(SmSym *) * NoHashEntries );

    NoSymbolSets    = 0;
    SymbolSets.Clear();
    for (USHORT i = 0;  i < rImpl.NoSymbolSets;  ++i)
    {
        rSymSetMgr.AddSymbolSet( new SmSymSet( *rImpl.rSymSetMgr.GetSymbolSet(i) ) );
    }
    DBG_ASSERT( NoSymbolSets == rImpl.NoSymbolSets,
            "incorrect number of symbolsets" );

    Modified        = TRUE;
    return *this;
}

/**************************************************************************/

void SmSymSetManager::SFX_NOTIFY(SfxBroadcaster& /*rBC*/, const TypeId& rBCType,
                              const SfxHint& /*rHint*/, const TypeId& rHintType)
{
}


UINT32 SmSymSetManager::GetHashIndex(const String& rSymbolName)
{
    UINT32 x = 1;
    for (xub_StrLen i = 0; i < rSymbolName.Len(); i++)
        x += x * rSymbolName.GetChar(i) + i;

    return x % pImpl->NoHashEntries;
}


void SmSymSetManager::EnterHashTable(SmSym& rSymbol)
{
    int j = GetHashIndex( rSymbol.GetName() );
    if (pImpl->HashEntries[j] == 0)
        pImpl->HashEntries[j] = &rSymbol;
    else
    {
        SmSym *p = pImpl->HashEntries[j];
        while (p->pHashNext)
            p = p->pHashNext;
        p->pHashNext = &rSymbol;
    }
    rSymbol.pHashNext = 0;
}


void SmSymSetManager::EnterHashTable(SmSymSet& rSymbolSet)
{
    for (USHORT i = 0; i < rSymbolSet.GetCount(); i++)
        EnterHashTable( *rSymbolSet.SymbolList.GetObject(i) );
}

void SmSymSetManager::FillHashTable()
{
    if (pImpl->HashEntries)
    {
        memset( pImpl->HashEntries, 0, pImpl->NoHashEntries * sizeof(SmSym *) );

        for (UINT32 i = 0; i < pImpl->NoSymbolSets; i++)
            EnterHashTable( *GetSymbolSet( (USHORT) i ) );
    }
}

void SmSymSetManager::Init()
{
    SmModule *pp = SM_MOD1();
    StartListening(*pp->GetConfig());
}


void SmSymSetManager::Exit()
{
    SmModule *pp = SM_MOD1();
    EndListening(*pp->GetConfig());
}


SmSymSetManager::SmSymSetManager(USHORT HashTableSize)
{
    pImpl = new SmSymSetManager_Impl( *this, HashTableSize );
}


SmSymSetManager::SmSymSetManager(const SmSymSetManager& rSymbolSetManager) :
    SfxListener()
{
    pImpl = new SmSymSetManager_Impl( *this, rSymbolSetManager.pImpl->NoHashEntries );
    *pImpl = *rSymbolSetManager.pImpl;
}


SmSymSetManager::~SmSymSetManager()
{
    delete pImpl;
    pImpl = 0;
}

SmSymSetManager& SmSymSetManager::operator = (const SmSymSetManager& rSymbolSetManager)
{
    *pImpl = *rSymbolSetManager.pImpl;
    return *this;
}

USHORT SmSymSetManager::AddSymbolSet(SmSymSet* pSymbolSet)
{
    if (pImpl->NoSymbolSets >= pImpl->SymbolSets.GetSize())
        pImpl->SymbolSets.SetSize(pImpl->NoSymbolSets + 1);

    pImpl->SymbolSets.Put(pImpl->NoSymbolSets++, pSymbolSet);

    pSymbolSet->pSymSetManager = this;

    for (USHORT i = 0; i < pSymbolSet->GetCount(); i++)
        pSymbolSet->SymbolList.GetObject(i)->pSymSetManager = this;

    FillHashTable();
    pImpl->Modified = TRUE;

    return (USHORT) (pImpl->NoSymbolSets - 1);
}

void SmSymSetManager::ChangeSymbolSet(SmSymSet* pSymbolSet)
{
    if (pSymbolSet)
    {
        FillHashTable();
        pImpl->Modified = TRUE;
    }
}

void SmSymSetManager::DeleteSymbolSet(USHORT SymbolSetNo)
{
    delete pImpl->SymbolSets.Get(SymbolSetNo);
    pImpl->NoSymbolSets--;

    for (UINT32 i = SymbolSetNo; i < pImpl->NoSymbolSets; i++)
        pImpl->SymbolSets.Put(i, pImpl->SymbolSets.Get(i + 1));

    FillHashTable();

    pImpl->Modified = TRUE;
}


USHORT SmSymSetManager::GetSymbolSetPos(const String& rSymbolSetName) const
{
    for (USHORT i = 0; i < pImpl->NoSymbolSets; i++)
        if (pImpl->SymbolSets.Get(i)->GetName() == rSymbolSetName)
            return (i);

    return SYMBOLSET_NONE;
}

SmSym *SmSymSetManager::GetSymbolByName(const String& rSymbolName)
{
    SmSym *pSym = pImpl->HashEntries[GetHashIndex(rSymbolName)];
    while (pSym)
    {
        if (pSym->Name == rSymbolName)
            break;
        pSym = pSym->pHashNext;
    }

    return pSym;
}


void SmSymSetManager::AddReplaceSymbol( const SmSym &rSymbol )
{
    SmSym *pSym = GetSymbolByName( rSymbol.GetName() );
    if (pSym)
    {
        *pSym = rSymbol;
    }
    else
    {
        USHORT nPos = GetSymbolSetPos( rSymbol.GetSetName() );
        if (SYMBOLSET_NONE == nPos)
        {
            AddSymbolSet( new SmSymSet( rSymbol.GetSetName() ) );
            nPos = GetSymbolSetPos( rSymbol.GetSetName() );
        }
        DBG_ASSERT( nPos != SYMBOLSET_NONE, "SymbolSet not found");
        SmSym *pTmpSym = new SmSym( rSymbol );
        GetSymbolSet( nPos )->AddSymbol( pTmpSym );
        EnterHashTable( *pTmpSym );
    }
    SetModified( TRUE );
}


USHORT SmSymSetManager::GetSymbolCount() const
{
    USHORT nRes = 0;
    USHORT nSets = GetSymbolSetCount();
    for (USHORT i = 0;  i < nSets;  ++i)
        nRes = nRes + GetSymbolSet(i)->GetCount();
    return nRes;
}


const SmSym * SmSymSetManager::GetSymbolByPos( USHORT nPos ) const
{
    const SmSym *pRes = 0;

    INT16 nIdx = 0;
    USHORT nSets = GetSymbolSetCount();
    USHORT i = 0;
    while (i < nSets  &&  !pRes)
    {
        USHORT nEntries = GetSymbolSet(i)->GetCount();
        if (nPos < nIdx + nEntries)
            pRes = &GetSymbolSet(i)->GetSymbol( nPos - nIdx );
        else
            nIdx = nIdx + nEntries;
        ++i;
    }

    return pRes;
}


void SmSymSetManager::GetSymbols( std::vector< SmSym > &rSymbols ) const
{
    INT32 nCount = GetSymbolCount();
    rSymbols.resize( nCount );
    USHORT nPos = 0;
    std::vector< SmSym >::iterator aIt( rSymbols.begin() );
    std::vector< SmSym >::iterator aEnd( rSymbols.end() );
    while (aIt != aEnd)
    {
        const SmSym *pSym = GetSymbolByPos( nPos++ );
        DBG_ASSERT( pSym, "symbol missing" );
        if (pSym)
            *aIt++ = *pSym;
    }
    DBG_ASSERT( nPos == nCount, "index out of range?" );
}


void SmSymSetManager::Load()
{
    std::vector< SmSym > aSymbols;
    SmMathConfig &rCfg = *SM_MOD1()->GetConfig();
    rCfg.GetSymbols( aSymbols );
    INT32 nSymbolCount = aSymbols.size();

    USHORT i;
    for (i = 0;  i < nSymbolCount;  ++i)
    {
        const SmSym &rSym = aSymbols[i];
        DBG_ASSERT( rSym.Name.Len() > 0, "symbol without name!" );
        if (rSym.Name.Len() > 0)
        {
            SmSymSet *pSymSet = 0;
            const String &rSetName = rSym.GetSetName();
            USHORT nSetPos = GetSymbolSetPos( rSetName );
            if (SYMBOLSET_NONE != nSetPos)
                pSymSet = GetSymbolSet( nSetPos );
            else
            {
                pSymSet = new SmSymSet( rSetName );
                AddSymbolSet( pSymSet );
            }

            pSymSet->AddSymbol( new SmSym( rSym ) );
        }
    }
    // build HashTables
    INT32 nSymbolSetCount = GetSymbolSetCount();
    for (i = 0;  i < nSymbolSetCount;  ++i)
        ChangeSymbolSet( GetSymbolSet( i ) );

    if (0 == nSymbolCount)
    {
        DBG_ERROR( "no symbol set found" );
        pImpl->Modified = FALSE;
    }
}

void SmSymSetManager::Save()
{
    SmMathConfig &rCfg = *SM_MOD1()->GetConfig();

    // get number of Symbols
    USHORT nSymbolCount = 0;
    USHORT nSetCount = GetSymbolSetCount();
    USHORT i;
    for (i = 0;  i < nSetCount;  ++i)
        nSymbolCount = nSymbolCount + GetSymbolSet( i )->GetCount();

    if (nSymbolCount)
    {
        USHORT nSaveSymbolCnt = 0;
        const SmSym **pSymbols = new const SmSym* [ nSymbolCount ];
        const SmSym **pSym = pSymbols;
        for (i = 0;  i < nSetCount;  ++i)
        {
            const SmSymSet *pSymSet = GetSymbolSet( i );
            USHORT n = pSymSet->GetCount();
            for (USHORT j = 0;  j < n;  ++j)
            {
                const SmSym &rSym = pSymSet->GetSymbol( j );
                if (!rSym.IsDocSymbol())
                {
                    *pSym++ = &rSym;
                    ++nSaveSymbolCnt;
                }
            }
        }
        DBG_ASSERT(pSym - pSymbols == nSaveSymbolCnt, "wrong number of symbols" );

        std::vector< SmSym > aSymbols;
        GetSymbols( aSymbols );
        rCfg.SetSymbols( aSymbols );
        delete [] pSymbols;
    }
}


