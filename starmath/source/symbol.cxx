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

#include <vector>

#include <symbol.hxx>
#include <utility.hxx>
#include "cfgitem.hxx"
#include <smmod.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>


SmSym::SmSym() :
    m_aName(OUString("unknown")),
    m_aSetName(OUString("unknown")),
    m_cChar('\0'),
    m_bPredefined(false)
{
    m_aExportName = m_aName;
    m_aFace.SetTransparent(true);
    m_aFace.SetAlignment(ALIGN_BASELINE);
}


SmSym::SmSym(const SmSym& rSymbol)
{
    *this = rSymbol;
}


SmSym::SmSym(const OUString& rName, const vcl::Font& rFont, sal_UCS4 cChar,
             const OUString& rSet, bool bIsPredefined)
{
    m_aName     = m_aExportName   = rName;

    m_aFace     = rFont;
    m_aFace.SetTransparent(true);
    m_aFace.SetAlignment(ALIGN_BASELINE);

    m_cChar         = cChar;
    m_aSetName      = rSet;
    m_bPredefined   = bIsPredefined;
}


SmSym& SmSym::operator = (const SmSym& rSymbol)
{
    m_aName         = rSymbol.m_aName;
    m_aExportName   = rSymbol.m_aExportName;
    m_cChar         = rSymbol.m_cChar;
    m_aFace         = rSymbol.m_aFace;
    m_aSetName      = rSymbol.m_aSetName;
    m_bPredefined   = rSymbol.m_bPredefined;

    SM_MOD()->GetSymbolManager().SetModified(true);

    return *this;
}


bool SmSym::IsEqualInUI( const SmSym& rSymbol ) const
{
    return  m_aName == rSymbol.m_aName &&
            m_aFace == rSymbol.m_aFace &&
            m_cChar == rSymbol.m_cChar;
}

/**************************************************************************/


SmSymbolManager::SmSymbolManager()
{
    m_bModified     = false;
}


SmSymbolManager::SmSymbolManager(const SmSymbolManager& rSymbolSetManager)
{
    m_aSymbols      = rSymbolSetManager.m_aSymbols;
    m_bModified     = true;
}


SmSymbolManager::~SmSymbolManager()
{
}


SmSymbolManager& SmSymbolManager::operator = (const SmSymbolManager& rSymbolSetManager)
{
    m_aSymbols      = rSymbolSetManager.m_aSymbols;
    m_bModified     = true;
    return *this;
}


SmSym *SmSymbolManager::GetSymbolByName(const OUString& rSymbolName)
{
    SmSym *pRes = nullptr;
    SymbolMap_t::iterator aIt( m_aSymbols.find( rSymbolName ) );
    if (aIt != m_aSymbols.end())
        pRes = &aIt->second;
    return pRes;
}


const SymbolPtrVec_t SmSymbolManager::GetSymbols() const
{
    SymbolPtrVec_t aRes;
    for (const auto& rEntry : m_aSymbols)
        aRes.push_back( &rEntry.second );
//    OSL_ENSURE( sSymbols.size() == m_aSymbols.size(), "number of symbols mismatch " );
    return aRes;
}


bool SmSymbolManager::AddOrReplaceSymbol( const SmSym &rSymbol, bool bForceChange )
{
    bool bAdded = false;

    const OUString& aSymbolName( rSymbol.GetName() );
    if (!aSymbolName.isEmpty() && !rSymbol.GetSymbolSetName().isEmpty())
    {
        const SmSym *pFound = GetSymbolByName( aSymbolName );
        const bool bSymbolConflict = pFound && !pFound->IsEqualInUI( rSymbol );

        // avoid having the same symbol name twice but with different symbols in use
        if (!pFound || bForceChange)
        {
            m_aSymbols[ aSymbolName ] = rSymbol;
            bAdded = true;
        }
        else if (bSymbolConflict)
        {
            // TODO: to solve this a document owned symbol manager would be required ...
                SAL_WARN("starmath", "symbol conflict, different symbol with same name found!");
            // symbols in all formulas. A copy of the global one would be needed here
            // and then the new symbol has to be forcefully applied. This would keep
            // the current formula intact but will leave the set of symbols in the
            // global symbol manager somewhat to chance.
        }

        OSL_ENSURE( bAdded, "failed to add symbol" );
        if (bAdded)
            m_bModified = true;
        OSL_ENSURE( bAdded || (pFound && !bSymbolConflict), "AddOrReplaceSymbol: unresolved symbol conflict" );
    }

    return bAdded;
}


void SmSymbolManager::RemoveSymbol( const OUString & rSymbolName )
{
    if (!rSymbolName.isEmpty())
    {
        size_t nOldSize = m_aSymbols.size();
        m_aSymbols.erase( rSymbolName );
        m_bModified = nOldSize != m_aSymbols.size();
    }
}


std::set< OUString > SmSymbolManager::GetSymbolSetNames() const
{
    std::set< OUString >  aRes;
    for (const auto& rEntry : m_aSymbols)
        aRes.insert( rEntry.second.GetSymbolSetName() );
    return aRes;
}


const SymbolPtrVec_t SmSymbolManager::GetSymbolSet( const OUString& rSymbolSetName )
{
    SymbolPtrVec_t aRes;
    if (!rSymbolSetName.isEmpty())
    {
        for (const auto& rEntry : m_aSymbols)
        {
            if (rEntry.second.GetSymbolSetName() == rSymbolSetName)
                aRes.push_back( &rEntry.second );
        }
    }
    return aRes;
}


void SmSymbolManager::Load()
{
    std::vector< SmSym > aSymbols;
    SmMathConfig &rCfg = *SM_MOD()->GetConfig();
    rCfg.GetSymbols( aSymbols );
    size_t nSymbolCount = aSymbols.size();

    m_aSymbols.clear();
    for (size_t i = 0;  i < nSymbolCount;  ++i)
    {
        const SmSym &rSym = aSymbols[i];
        OSL_ENSURE( !rSym.GetName().isEmpty(), "symbol without name!" );
        if (!rSym.GetName().isEmpty())
            AddOrReplaceSymbol( rSym );
    }
    m_bModified = true;

    if (0 == nSymbolCount)
    {
        SAL_WARN("starmath", "no symbol set found");
        m_bModified = false;
    }

    // now add a %i... symbol to the 'iGreek' set for every symbol found in the 'Greek' set.
    const OUString aGreekSymbolSetName(SmLocalizedSymbolData::GetUiSymbolSetName("Greek"));
    const SymbolPtrVec_t    aGreekSymbols( GetSymbolSet( aGreekSymbolSetName ) );
    OUString aSymbolSetName('i');
    aSymbolSetName += aGreekSymbolSetName;
    size_t nSymbols = aGreekSymbols.size();
    for (size_t i = 0;  i < nSymbols;  ++i)
    {
        // make the new symbol a copy but with ITALIC_NORMAL, and add it to iGreek
        const SmSym &rSym = *aGreekSymbols[i];
        vcl::Font aFont( rSym.GetFace() );
        OSL_ENSURE( aFont.GetItalic() == ITALIC_NONE, "expected Font with ITALIC_NONE, failed." );
        aFont.SetItalic( ITALIC_NORMAL );
        OUString aSymbolName('i');
        aSymbolName += rSym.GetName();
        SmSym aSymbol( aSymbolName, aFont, rSym.GetCharacter(),
                aSymbolSetName, true /*bIsPredefined*/ );

        AddOrReplaceSymbol( aSymbol );
    }
}

void SmSymbolManager::Save()
{
    if (!m_bModified)
        return;

    SmMathConfig &rCfg = *SM_MOD()->GetConfig();

    // prepare to skip symbols from iGreek on saving
    OUString aSymbolSetName('i');
    aSymbolSetName += SmLocalizedSymbolData::GetUiSymbolSetName("Greek");

    SymbolPtrVec_t aTmp( GetSymbols() );
    std::vector< SmSym > aSymbols;
    for (const SmSym* i : aTmp)
    {
        // skip symbols from iGreek set since those symbols always get added
        // by computational means in SmSymbolManager::Load
        if (i->GetSymbolSetName() != aSymbolSetName)
            aSymbols.push_back( *i );
    }
    rCfg.SetSymbols( aSymbols );

    m_bModified = false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
