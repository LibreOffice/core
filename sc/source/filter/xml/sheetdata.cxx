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

#include <rtl/ustring.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/nmspmap.hxx>

#include "sheetdata.hxx"

// -----------------------------------------------------------------------

ScSheetSaveData::ScSheetSaveData() :
    mnStartTab( -1 ),
    mnStartOffset( -1 ),
    maPreviousNote( rtl::OUString(), rtl::OUString(), ScAddress(ScAddress::INITIALIZE_INVALID) ),
    mbInSupportedSave( false )
{
}

ScSheetSaveData::~ScSheetSaveData()
{
}

void ScSheetSaveData::AddCellStyle( const rtl::OUString& rName, const ScAddress& rCellPos )
{
    maCellStyles.push_back( ScCellStyleEntry( rName, rCellPos ) );
}

void ScSheetSaveData::AddColumnStyle( const rtl::OUString& rName, const ScAddress& rCellPos )
{
    maColumnStyles.push_back( ScCellStyleEntry( rName, rCellPos ) );
}

void ScSheetSaveData::AddRowStyle( const rtl::OUString& rName, const ScAddress& rCellPos )
{
    maRowStyles.push_back( ScCellStyleEntry( rName, rCellPos ) );
}

void ScSheetSaveData::AddTableStyle( const rtl::OUString& rName, const ScAddress& rCellPos )
{
    maTableStyles.push_back( ScCellStyleEntry( rName, rCellPos ) );
}

void ScSheetSaveData::HandleNoteStyles( const rtl::OUString& rStyleName, const rtl::OUString& rTextName, const ScAddress& rCellPos )
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

void ScSheetSaveData::AddNoteContentStyle( sal_uInt16 nFamily, const rtl::OUString& rName, const ScAddress& rCellPos, const ESelection& rSelection )
{
    if ( nFamily == XML_STYLE_FAMILY_TEXT_PARAGRAPH )
        maNoteParaStyles.push_back( ScTextStyleEntry( rName, rCellPos, rSelection ) );
    else
        maNoteTextStyles.push_back( ScTextStyleEntry( rName, rCellPos, rSelection ) );
}

void ScSheetSaveData::AddTextStyle( const rtl::OUString& rName, const ScAddress& rCellPos, const ESelection& rSelection )
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
    // so only a boost::unordered_map of the prefixes is needed.

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

static bool lcl_NameInHash( const NameSpaceHash& rNameHash, const rtl::OUString& rName )
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

bool ScSheetSaveData::IsInSupportedSave() const
{
    return mbInSupportedSave;
}

void ScSheetSaveData::SetInSupportedSave( bool bSet )
{
    mbInSupportedSave = bSet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
