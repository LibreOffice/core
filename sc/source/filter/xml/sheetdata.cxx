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

#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <xmloff/families.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/nmspmap.hxx>

#include "sheetdata.hxx"

ScSheetSaveData::ScSheetSaveData() :
    mnStartTab( -1 ),
    mnStartOffset( -1 ),
    maPreviousNote( OUString(), OUString(), ScAddress::INITIALIZE_INVALID ),
    mbInSupportedSave( false )
{
}

ScSheetSaveData::~ScSheetSaveData()
{
}

void ScSheetSaveData::AddCellStyle( const OUString& rName, const ScAddress& rCellPos )
{
    maCellStyles.push_back( ScCellStyleEntry( rName, rCellPos ) );
}

void ScSheetSaveData::AddColumnStyle( const OUString& rName, const ScAddress& rCellPos )
{
    maColumnStyles.push_back( ScCellStyleEntry( rName, rCellPos ) );
}

void ScSheetSaveData::AddRowStyle( const OUString& rName, const ScAddress& rCellPos )
{
    maRowStyles.push_back( ScCellStyleEntry( rName, rCellPos ) );
}

void ScSheetSaveData::AddTableStyle( const OUString& rName, const ScAddress& rCellPos )
{
    maTableStyles.push_back( ScCellStyleEntry( rName, rCellPos ) );
}

void ScSheetSaveData::HandleNoteStyles( const OUString& rStyleName, const OUString& rTextName, const ScAddress& rCellPos )
{
    // only consecutive duplicates (most common case) are filtered out here,
    // the others are found when the styles are created

    if ( rStyleName == maPreviousNote.maStyleName &&
         rTextName  == maPreviousNote.maTextStyle &&
         rCellPos.Tab() == maPreviousNote.maCellPos.Tab() )
    {
        // already stored for the same sheet - ignore
        return;
    }

    ScNoteStyleEntry aNewEntry( rStyleName, rTextName, rCellPos );
    maPreviousNote = aNewEntry;
    maNoteStyles.push_back( aNewEntry );
}

void ScSheetSaveData::AddNoteContentStyle( sal_uInt16 nFamily, const OUString& rName, const ScAddress& rCellPos, const ESelection& rSelection )
{
    if ( nFamily == XML_STYLE_FAMILY_TEXT_PARAGRAPH )
        maNoteParaStyles.push_back( ScTextStyleEntry( rName, rCellPos, rSelection ) );
    else
        maNoteTextStyles.push_back( ScTextStyleEntry( rName, rCellPos, rSelection ) );
}

void ScSheetSaveData::AddTextStyle( const OUString& rName, const ScAddress& rCellPos, const ESelection& rSelection )
{
    maTextStyles.push_back( ScTextStyleEntry( rName, rCellPos, rSelection ) );
}

void ScSheetSaveData::BlockSheet( SCTAB nTab )
{
    if ( nTab >= static_cast<SCTAB>(maBlocked.size()) )
        maBlocked.resize( nTab + 1, false );        // fill vector with "false" entries

    maBlocked[nTab] = true;
}

bool ScSheetSaveData::IsSheetBlocked( SCTAB nTab ) const
{
    if ( nTab < static_cast<SCTAB>(maBlocked.size()) )
        return maBlocked[nTab];
    else
        return false;
}

void ScSheetSaveData::AddStreamPos( SCTAB nTab, sal_Int32 nStartOffset, sal_Int32 nEndOffset )
{
    if ( nTab >= static_cast<SCTAB>(maStreamEntries.size()) )
        maStreamEntries.resize( nTab + 1 );

    maStreamEntries[nTab] = ScStreamEntry( nStartOffset, nEndOffset );
}

void ScSheetSaveData::StartStreamPos( SCTAB nTab, sal_Int32 nStartOffset )
{
    OSL_ENSURE( mnStartTab < 0, "StartStreamPos without EndStreamPos" );

    mnStartTab = nTab;
    mnStartOffset = nStartOffset;
}

void ScSheetSaveData::EndStreamPos( sal_Int32 nEndOffset )
{
    if ( mnStartTab >= 0 )
    {
        AddStreamPos( mnStartTab, mnStartOffset, nEndOffset );
        mnStartTab = -1;
        mnStartOffset = -1;
    }
}

void ScSheetSaveData::GetStreamPos( SCTAB nTab, sal_Int32& rStartOffset, sal_Int32& rEndOffset ) const
{
    if ( nTab < static_cast<SCTAB>(maStreamEntries.size()) )
    {
        const ScStreamEntry& rEntry = maStreamEntries[nTab];
        rStartOffset = rEntry.mnStartOffset;
        rEndOffset = rEntry.mnEndOffset;
    }
    else
        rStartOffset = rEndOffset = -1;
}

bool ScSheetSaveData::HasStreamPos( SCTAB nTab ) const
{
    sal_Int32 nStartOffset = -1;
    sal_Int32 nEndOffset = -1;
    GetStreamPos( nTab, nStartOffset, nEndOffset );
    return ( nStartOffset >= 0 && nEndOffset >= 0 );
}

void ScSheetSaveData::ResetSaveEntries()
{
    maSaveEntries.clear();
}

void ScSheetSaveData::AddSavePos( SCTAB nTab, sal_Int32 nStartOffset, sal_Int32 nEndOffset )
{
    if ( nTab >= static_cast<SCTAB>(maSaveEntries.size()) )
        maSaveEntries.resize( nTab + 1 );

    maSaveEntries[nTab] = ScStreamEntry( nStartOffset, nEndOffset );
}

void ScSheetSaveData::UseSaveEntries()
{
    maStreamEntries = maSaveEntries;
}

void ScSheetSaveData::StoreInitialNamespaces( const SvXMLNamespaceMap& rNamespaces )
{
    // the initial namespaces are just removed from the list of loaded namespaces,
    // so only a unordered_map of the prefixes is needed.

    const NameSpaceHash& rNameHash = rNamespaces.GetAllEntries();
    NameSpaceHash::const_iterator aIter = rNameHash.begin(), aEnd = rNameHash.end();
    while (aIter != aEnd)
    {
        maInitialPrefixes.insert( aIter->first );
        ++aIter;
    }
}

void ScSheetSaveData::StoreLoadedNamespaces( const SvXMLNamespaceMap& rNamespaces )
{
    // store the loaded namespaces, so the prefixes in copied stream fragments remain valid

    const NameSpaceHash& rNameHash = rNamespaces.GetAllEntries();
    NameSpaceHash::const_iterator aIter = rNameHash.begin(), aEnd = rNameHash.end();
    while (aIter != aEnd)
    {
        // ignore the initial namespaces
        if ( maInitialPrefixes.find( aIter->first ) == maInitialPrefixes.end() )
        {
            const NameSpaceEntry& rEntry = *(aIter->second);
            maLoadedNamespaces.push_back( ScLoadedNamespaceEntry( rEntry.sPrefix, rEntry.sName, rEntry.nKey ) );
        }
        ++aIter;
    }
}

static bool lcl_NameInHash( const NameSpaceHash& rNameHash, const OUString& rName )
{
    NameSpaceHash::const_iterator aIter = rNameHash.begin(), aEnd = rNameHash.end();
    while (aIter != aEnd)
    {
        if ( aIter->second->sName == rName )
            return true;

        ++aIter;
    }
    return false;   // not found
}

bool ScSheetSaveData::AddLoadedNamespaces( SvXMLNamespaceMap& rNamespaces ) const
{
    // Add the loaded namespaces to the name space map.

    // first loop: only look for conflicts
    // (if the loaded namespaces were added first, this might not be necessary)
    const NameSpaceHash& rNameHash = rNamespaces.GetAllEntries();
    std::vector<ScLoadedNamespaceEntry>::const_iterator aIter = maLoadedNamespaces.begin();
    std::vector<ScLoadedNamespaceEntry>::const_iterator aEnd = maLoadedNamespaces.end();
    while (aIter != aEnd)
    {
        NameSpaceHash::const_iterator aHashIter = rNameHash.find( aIter->maPrefix );
        if ( aHashIter == rNameHash.end() )
        {
            if ( lcl_NameInHash( rNameHash, aIter->maName ) )
            {
                // a second prefix for the same name would confuse SvXMLNamespaceMap lookup,
                // so this is also considered a conflict
                return false;
            }
        }
        else if ( aHashIter->second->sName != aIter->maName )
        {
            // same prefix, but different name: loaded namespaces can't be used
            return false;
        }
        ++aIter;
    }

    // only if there were no conflicts, add the entries that aren't in the map already
    // (the key is needed if the same namespace is added later within an element)
    aIter = maLoadedNamespaces.begin();
    while (aIter != aEnd)
    {
        NameSpaceHash::const_iterator aHashIter = rNameHash.find( aIter->maPrefix );
        if ( aHashIter == rNameHash.end() )
            rNamespaces.Add( aIter->maPrefix, aIter->maName, aIter->mnKey );
        ++aIter;
    }

    return true;    // success
}

void ScSheetSaveData::SetInSupportedSave( bool bSet )
{
    mbInSupportedSave = bSet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
