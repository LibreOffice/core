/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <osl/mutex.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/msgbox.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>

#include <map>
#include <vector>
#include <iterator>

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


/**************************************************************************/

SmSym::SmSym() :
    m_aName(C2S("unknown")),
    m_aSetName(C2S("unknown")),
    m_cChar('\0'),
    m_bPredefined(false),
    m_bDocSymbol(false)
{
    m_aExportName = m_aName;
    m_aFace.SetTransparent(true);
    m_aFace.SetAlign(ALIGN_BASELINE);
}


SmSym::SmSym(const SmSym& rSymbol)
{
    *this = rSymbol;
}


SmSym::SmSym(const String& rName, const Font& rFont, sal_Unicode cChar,
             const String& rSet, bool bIsPredefined)
{
    m_aName     = m_aExportName   = rName;

    m_aFace     = rFont;
    m_aFace.SetTransparent(true);
    m_aFace.SetAlign(ALIGN_BASELINE);

    m_cChar         = cChar;
    m_aSetName      = rSet;
    m_bPredefined   = bIsPredefined;
    m_bDocSymbol    = false;
}


SmSym& SmSym::operator = (const SmSym& rSymbol)
{
    m_aName         = rSymbol.m_aName;
    m_aExportName   = rSymbol.m_aExportName;
    m_cChar         = rSymbol.m_cChar;
    m_aFace         = rSymbol.m_aFace;
    m_aSetName      = rSymbol.m_aSetName;
    m_bPredefined   = rSymbol.m_bPredefined;
    m_bDocSymbol    = rSymbol.m_bDocSymbol;

    SmSymbolManager * pSymSetManager = &SM_MOD()->GetSymbolManager();
    if (pSymSetManager)
        pSymSetManager->SetModified(true);

    return *this;
}


bool SmSym::IsEqualInUI( const SmSym& rSymbol ) const
{
    return  m_aName == rSymbol.m_aName &&
            m_aFace == rSymbol.m_aFace &&
            m_cChar == rSymbol.m_cChar;
}

/**************************************************************************/

void SmSymbolManager::SFX_NOTIFY(SfxBroadcaster& /*rBC*/, const TypeId& rBCType,
                              const SfxHint& /*rHint*/, const TypeId& rHintType)
{
}


void SmSymbolManager::Init()
{
    SmModule *pp = SM_MOD();
    StartListening(*pp->GetConfig());
}


void SmSymbolManager::Exit()
{
    SmModule *pp = SM_MOD();
    EndListening(*pp->GetConfig());
}


SmSymbolManager::SmSymbolManager()
{
    m_bModified     = false;
}


SmSymbolManager::SmSymbolManager(const SmSymbolManager& rSymbolSetManager) :
    SfxListener()
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


SmSym *SmSymbolManager::GetSymbolByName(const String& rSymbolName)
{
    SmSym *pRes = NULL;
    SymbolMap_t::iterator aIt( m_aSymbols.find( rSymbolName ) );
    if (aIt != m_aSymbols.end())
        pRes = &aIt->second;
    return pRes;
}


const SymbolPtrVec_t SmSymbolManager::GetSymbols() const
{
    SymbolPtrVec_t aRes;
    SymbolMap_t::const_iterator aIt( m_aSymbols.begin() );
    for ( ; aIt != m_aSymbols.end(); ++aIt)
        aRes.push_back( &aIt->second );
//    OSL_ENSURE( sSymbols.size() == m_aSymbols.size(), "number of symbols mismatch " );
    return aRes;
}


bool SmSymbolManager::AddOrReplaceSymbol( const SmSym &rSymbol, bool bForceChange )
{
    bool bAdded = false;

    const String aSymbolName( rSymbol.GetName() );
    if (aSymbolName.Len() > 0 && rSymbol.GetSymbolSetName().Len() > 0)
    {
        const SmSym *pFound = GetSymbolByName( aSymbolName );
        const bool bSymbolConflict = pFound && !pFound->IsEqualInUI( rSymbol );

        // avoid having the same symbol name twice but with different symbols in use
        if (!pFound || bForceChange)
        {
            m_aSymbols[ aSymbolName ] = rSymbol;
            bAdded = true;
        }
        else if (pFound && !bForceChange && bSymbolConflict)
        {
                // TODO: but what ...
                OSL_ENSURE( 0, "symbol conflict, different symbol with same name found!" );
        }
    }

    OSL_ENSURE( bAdded, "failed to add symbol" );
    if (bAdded)
        m_bModified = true;

    return bAdded;
}


void SmSymbolManager::RemoveSymbol( const String & rSymbolName )
{
    if (rSymbolName.Len() > 0)
    {
        size_t nOldSize = m_aSymbols.size();
        m_aSymbols.erase( rSymbolName );
        m_bModified = nOldSize != m_aSymbols.size();
    }
}


std::set< String > SmSymbolManager::GetSymbolSetNames() const
{
    std::set< String >  aRes;
    SymbolMap_t::const_iterator aIt( m_aSymbols.begin() );
    for ( ; aIt != m_aSymbols.end(); ++aIt )
        aRes.insert( aIt->second.GetSymbolSetName() );
    return aRes;
}


const SymbolPtrVec_t SmSymbolManager::GetSymbolSet( const String& rSymbolSetName )
{
    SymbolPtrVec_t aRes;
    if (rSymbolSetName.Len() > 0)
    {
        SymbolMap_t::const_iterator aIt( m_aSymbols.begin() );
        for ( ; aIt != m_aSymbols.end(); ++aIt )
        {
            if (aIt->second.GetSymbolSetName() == rSymbolSetName)
                aRes.push_back( &aIt->second );
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
        OSL_ENSURE( rSym.GetName().Len() > 0, "symbol without name!" );
        if (rSym.GetName().Len() > 0)
            AddOrReplaceSymbol( rSym );
    }
    m_bModified = true;

    if (0 == nSymbolCount)
    {
        OSL_FAIL( "no symbol set found" );
        m_bModified = false;
    }

    // now add a %i... symbol to the 'iGreek' set for every symbol found in the 'Greek' set.
    SmLocalizedSymbolData   aLocalizedData;
    const String aGreekSymbolSetName( aLocalizedData.GetUiSymbolSetName( A2OU("Greek") ) );
    const SymbolPtrVec_t    aGreekSymbols( GetSymbolSet( aGreekSymbolSetName ) );
    String aSymbolSetName( (sal_Unicode) 'i' );
    aSymbolSetName += aGreekSymbolSetName;
    size_t nSymbols = aGreekSymbols.size();
    for (size_t i = 0;  i < nSymbols;  ++i)
    {
        // make the new symbol a copy but with ITALIC_NORMAL, and add it to iGreek
        const SmSym &rSym = *aGreekSymbols[i];
        Font aFont( rSym.GetFace() );
        OSL_ENSURE( aFont.GetItalic() == ITALIC_NONE, "expected Font with ITALIC_NONE, failed." );
        aFont.SetItalic( ITALIC_NORMAL );
        String aSymbolName( (sal_Unicode)'i' );
        aSymbolName += rSym.GetName();
        SmSym aSymbol( aSymbolName, aFont, rSym.GetCharacter(),
                aSymbolSetName, true /*bIsPredefined*/ );

        AddOrReplaceSymbol( aSymbol );
    }
}

void SmSymbolManager::Save()
{
    if (m_bModified)
    {
        SmMathConfig &rCfg = *SM_MOD()->GetConfig();

        // prepare to skip symbols from iGreek on saving
        SmLocalizedSymbolData   aLocalizedData;
        String aSymbolSetName( (sal_Unicode) 'i' );
        aSymbolSetName += aLocalizedData.GetUiSymbolSetName( A2OU("Greek") );

        SymbolPtrVec_t aTmp( GetSymbols() );
        std::vector< SmSym > aSymbols;
        for (size_t i = 0; i < aTmp.size(); ++i)
        {
            // skip symbols from iGreek set since those symbols always get added
            // by computational means in SmSymbolManager::Load
            if (aTmp[i]->GetSymbolSetName() != aSymbolSetName)
                aSymbols.push_back( *aTmp[i] );
        }
        rCfg.SetSymbols( aSymbols );

        m_bModified = false;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
