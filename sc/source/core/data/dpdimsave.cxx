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
#include "precompiled_sc.hxx"

#include "dpdimsave.hxx"
#include "dpgroup.hxx"
#include "dpobject.hxx"
#include "document.hxx"

#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>

#include <svl/zforlist.hxx>
#include <tools/debug.hxx>
#include <rtl/math.hxx>
#include <algorithm>

// ============================================================================

ScDPSaveGroupItem::ScDPSaveGroupItem( const String& rName ) :
    aGroupName( rName )
{
}

ScDPSaveGroupItem::~ScDPSaveGroupItem()
{
}

void ScDPSaveGroupItem::AddElement( const String& rName )
{
    aElements.push_back( rName );
}

void ScDPSaveGroupItem::AddElementsFromGroup( const ScDPSaveGroupItem& rGroup )
{
    // add all elements of the other group (used for nested grouping)

    for ( std::vector<String>::const_iterator aIter(rGroup.aElements.begin());
                                aIter != rGroup.aElements.end(); aIter++ )
        aElements.push_back( *aIter );
}

bool ScDPSaveGroupItem::RemoveElement( const String& rName )
{
    for ( std::vector<String>::iterator aIter(aElements.begin()); aIter != aElements.end(); aIter++ )
        if ( *aIter == rName )          //! ignore case
        {
            aElements.erase( aIter );   // found -> remove
            return true;                // don't have to look further
        }

    return false;   // not found
}

bool ScDPSaveGroupItem::IsEmpty() const
{
    return aElements.empty();
}

size_t ScDPSaveGroupItem::GetElementCount() const
{
    return aElements.size();
}

const String* ScDPSaveGroupItem::GetElementByIndex( size_t nIndex ) const
{
    return (nIndex < aElements.size()) ? &aElements[ nIndex ] : 0;
}

void ScDPSaveGroupItem::Rename( const String& rNewName )
{
    aGroupName = rNewName;
}

void ScDPSaveGroupItem::RemoveElementsFromGroups( ScDPSaveGroupDimension& rDimension ) const
{
    // remove this group's elements from their groups in rDimension
    // (rDimension must be a different dimension from the one which contains this)

    for ( std::vector<String>::const_iterator aIter(aElements.begin()); aIter != aElements.end(); aIter++ )
        rDimension.RemoveFromGroups( *aIter );
}

void ScDPSaveGroupItem::AddToData( ScDPGroupDimension& rDataDim, SvNumberFormatter* pFormatter ) const
{
    ScDPGroupItem aGroup( aGroupName );
    ScDPItemData aData;

    for ( std::vector<String>::const_iterator aIter(aElements.begin()); aIter != aElements.end(); aIter++ )
    {
        sal_uInt32 nFormat = 0;      //! ...
        double fValue;
        if ( pFormatter->IsNumberFormat( *aIter, nFormat, fValue ) )
            aData = ScDPItemData( *aIter, fValue, sal_True );
        else
            aData.SetString( *aIter );

        aGroup.AddElement( aData );
        //! for numeric data, look at source members?
    }

    rDataDim.AddItem( aGroup );
}

// ============================================================================

ScDPSaveGroupDimension::ScDPSaveGroupDimension( const String& rSource, const String& rName ) :
    aSourceDim( rSource ),
    aGroupDimName( rName ),
    nDatePart( 0 )
{
}

ScDPSaveGroupDimension::ScDPSaveGroupDimension( const String& rSource, const String& rName, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nPart ) :
    aSourceDim( rSource ),
    aGroupDimName( rName ),
    aDateInfo( rDateInfo ),
    nDatePart( nPart )
{
}

ScDPSaveGroupDimension::~ScDPSaveGroupDimension()
{
}

void ScDPSaveGroupDimension::SetDateInfo( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart )
{
    aDateInfo = rInfo;
    nDatePart = nPart;
}

void ScDPSaveGroupDimension::AddGroupItem( const ScDPSaveGroupItem& rItem )
{
    aGroups.push_back( rItem );
}

String ScDPSaveGroupDimension::CreateGroupName( const String& rPrefix )
{
    // create a name for a new group, using "Group1", "Group2" etc. (translated prefix in rPrefix)

    //! look in all dimensions, to avoid clashes with automatic groups (=name of base element)?
    //! (only dimensions for the same base)

    sal_Int32 nAdd = 1;                                 // first try is "Group1"
    const sal_Int32 nMaxAdd = nAdd + aGroups.size();    // limit the loop
    while ( nAdd <= nMaxAdd )
    {
        String aGroupName( rPrefix );
        aGroupName.Append( String::CreateFromInt32( nAdd ) );
        bool bExists = false;

        // look for existing groups
        for ( ScDPSaveGroupItemVec::const_iterator aIter(aGroups.begin());
                                    aIter != aGroups.end() && !bExists; aIter++ )
            if ( aIter->GetGroupName() == aGroupName )         //! ignore case
                bExists = true;

        if ( !bExists )
            return aGroupName;          // found a new name

        ++nAdd;                         // continue with higher number
    }

    DBG_ERROR("CreateGroupName: no valid name found");
    return EMPTY_STRING;
}

const ScDPSaveGroupItem* ScDPSaveGroupDimension::GetNamedGroup( const String& rGroupName ) const
{
    return const_cast< ScDPSaveGroupDimension* >( this )->GetNamedGroupAcc( rGroupName );
}

ScDPSaveGroupItem* ScDPSaveGroupDimension::GetNamedGroupAcc( const String& rGroupName )
{
    for ( ScDPSaveGroupItemVec::iterator aIter(aGroups.begin()); aIter != aGroups.end(); aIter++ )
        if ( aIter->GetGroupName() == rGroupName )         //! ignore case
            return &*aIter;

    return NULL;        // none found
}

long ScDPSaveGroupDimension::GetGroupCount() const
{
    return aGroups.size();
}

const ScDPSaveGroupItem* ScDPSaveGroupDimension::GetGroupByIndex( long nIndex ) const
{
    return const_cast< ScDPSaveGroupDimension* >( this )->GetGroupAccByIndex( nIndex );
}

ScDPSaveGroupItem* ScDPSaveGroupDimension::GetGroupAccByIndex( long nIndex )
{
    return &aGroups[nIndex];
}

void ScDPSaveGroupDimension::RemoveFromGroups( const String& rItemName )
{
    //  if the item is in any group, remove it from the group,
    //  also remove the group if it is empty afterwards

    for ( ScDPSaveGroupItemVec::iterator aIter(aGroups.begin()); aIter != aGroups.end(); aIter++ )
        if ( aIter->RemoveElement( rItemName ) )
        {
            if ( aIter->IsEmpty() )         // removed last item from the group?
                aGroups.erase( aIter );     // then remove the group

            return;     // don't have to look further
        }
}

void ScDPSaveGroupDimension::RemoveGroup( const String& rGroupName )
{
    for ( ScDPSaveGroupItemVec::iterator aIter(aGroups.begin()); aIter != aGroups.end(); aIter++ )
        if ( aIter->GetGroupName() == rGroupName )          //! ignore case
        {
            aGroups.erase( aIter );
            return;                     // don't have to look further
        }
}

bool ScDPSaveGroupDimension::IsEmpty() const
{
    return aGroups.empty();
}

bool ScDPSaveGroupDimension::HasOnlyHidden( const ScStrCollection& rVisible )
{
    // check if there are only groups that don't appear in the list of visible names

    bool bAllHidden = true;
    for ( ScDPSaveGroupItemVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end() && bAllHidden; aIter++ )
    {
        StrData aSearch( aIter->GetGroupName() );
        sal_uInt16 nCollIndex;
        if ( rVisible.Search( &aSearch, nCollIndex ) )
            bAllHidden = false;                             // found one that is visible
    }
    return bAllHidden;
}

void ScDPSaveGroupDimension::Rename( const String& rNewName )
{
    aGroupDimName = rNewName;
}

void ScDPSaveGroupDimension::AddToData( ScDPGroupTableData& rData ) const
{
    long nSourceIndex = rData.GetDimensionIndex( aSourceDim );
    if ( nSourceIndex >= 0 )
    {
        ScDPGroupDimension aDim( nSourceIndex, aGroupDimName );
        if ( nDatePart )
        {
            // date grouping

            aDim.MakeDateHelper( aDateInfo, nDatePart );
        }
        else
        {
            // normal (manual) grouping

            SvNumberFormatter* pFormatter = rData.GetDocument()->GetFormatTable();

            for ( ScDPSaveGroupItemVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end(); aIter++ )
                aIter->AddToData( aDim, pFormatter );
        }

        rData.AddGroupDimension( aDim );
    }
}

// ============================================================================

ScDPSaveNumGroupDimension::ScDPSaveNumGroupDimension( const String& rName, const ScDPNumGroupInfo& rInfo ) :
    aDimensionName( rName ),
    aGroupInfo( rInfo ),
    nDatePart( 0 )
{
}

ScDPSaveNumGroupDimension::ScDPSaveNumGroupDimension( const String& rName, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nPart ) :
    aDimensionName( rName ),
    aDateInfo( rDateInfo ),
    nDatePart( nPart )
{
}

ScDPSaveNumGroupDimension::~ScDPSaveNumGroupDimension()
{
}

void ScDPSaveNumGroupDimension::AddToData( ScDPGroupTableData& rData ) const
{
    long nSource = rData.GetDimensionIndex( aDimensionName );
    if ( nSource >= 0 )
    {
        ScDPNumGroupDimension aDim( aGroupInfo );           // aGroupInfo: value grouping
        if ( nDatePart )
            aDim.MakeDateHelper( aDateInfo, nDatePart );    // date grouping

        rData.SetNumGroupDimension( nSource, aDim );
    }
}

void ScDPSaveNumGroupDimension::SetGroupInfo( const ScDPNumGroupInfo& rNew )
{
    aGroupInfo = rNew;
}

void ScDPSaveNumGroupDimension::SetDateInfo( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart )
{
    aDateInfo = rInfo;
    nDatePart = nPart;
}

// ============================================================================

namespace {

struct ScDPSaveGroupDimNameFunc
{
    String              maDimName;
    inline explicit     ScDPSaveGroupDimNameFunc( const String& rDimName ) : maDimName( rDimName ) {}
    inline bool         operator()( const ScDPSaveGroupDimension& rGroupDim ) const { return rGroupDim.GetGroupDimName() == maDimName; }
};

struct ScDPSaveGroupSourceNameFunc
{
    String              maSrcDimName;
    inline explicit     ScDPSaveGroupSourceNameFunc( const String& rSrcDimName ) : maSrcDimName( rSrcDimName ) {}
    inline bool         operator()( const ScDPSaveGroupDimension& rGroupDim ) const { return rGroupDim.GetSourceDimName() == maSrcDimName; }
};

} // namespace

// ----------------------------------------------------------------------------

ScDPDimensionSaveData::ScDPDimensionSaveData()
{
}

ScDPDimensionSaveData::~ScDPDimensionSaveData()
{
}

bool ScDPDimensionSaveData::operator==( const ScDPDimensionSaveData& ) const
{
    return false;
}

void ScDPDimensionSaveData::AddGroupDimension( const ScDPSaveGroupDimension& rGroupDim )
{
    DBG_ASSERT( ::std::find_if( maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupDimNameFunc( rGroupDim.GetGroupDimName() ) ) == maGroupDims.end(),
        "ScDPDimensionSaveData::AddGroupDimension - group dimension exists already" );
    // ReplaceGroupDimension() adds new or replaces existing
    ReplaceGroupDimension( rGroupDim );
}

void ScDPDimensionSaveData::ReplaceGroupDimension( const ScDPSaveGroupDimension& rGroupDim )
{
    ScDPSaveGroupDimVec::iterator aIt = ::std::find_if(
        maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupDimNameFunc( rGroupDim.GetGroupDimName() ) );
    if( aIt == maGroupDims.end() )
        maGroupDims.push_back( rGroupDim );
    else
        *aIt = rGroupDim;
}

void ScDPDimensionSaveData::RemoveGroupDimension( const String& rGroupDimName )
{
    ScDPSaveGroupDimVec::iterator aIt = ::std::find_if(
        maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupDimNameFunc( rGroupDimName ) );
    if( aIt != maGroupDims.end() )
        maGroupDims.erase( aIt );
}

void ScDPDimensionSaveData::AddNumGroupDimension( const ScDPSaveNumGroupDimension& rGroupDim )
{
    DBG_ASSERT( maNumGroupDims.count( rGroupDim.GetDimensionName() ) == 0,
        "ScDPDimensionSaveData::AddNumGroupDimension - numeric group dimension exists already" );
    // ReplaceNumGroupDimension() adds new or replaces existing
    ReplaceNumGroupDimension( rGroupDim );
}

void ScDPDimensionSaveData::ReplaceNumGroupDimension( const ScDPSaveNumGroupDimension& rGroupDim )
{
    ScDPSaveNumGroupDimMap::iterator aIt = maNumGroupDims.find( rGroupDim.GetDimensionName() );
    if( aIt == maNumGroupDims.end() )
        maNumGroupDims.insert( ScDPSaveNumGroupDimMap::value_type( rGroupDim.GetDimensionName(), rGroupDim ) );
    else
        aIt->second = rGroupDim;
}

void ScDPDimensionSaveData::RemoveNumGroupDimension( const String& rGroupDimName )
{
    maNumGroupDims.erase( rGroupDimName );
}

void ScDPDimensionSaveData::WriteToData( ScDPGroupTableData& rData ) const
{
    //  rData is assumed to be empty
    //  AddToData also handles date grouping

    for( ScDPSaveGroupDimVec::const_iterator aIt = maGroupDims.begin(), aEnd = maGroupDims.end(); aIt != aEnd; ++aIt )
        aIt->AddToData( rData );

    for( ScDPSaveNumGroupDimMap::const_iterator aIt = maNumGroupDims.begin(), aEnd = maNumGroupDims.end(); aIt != aEnd; ++aIt )
        aIt->second.AddToData( rData );
}

const ScDPSaveGroupDimension* ScDPDimensionSaveData::GetGroupDimForBase( const String& rBaseDimName ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetGroupDimAccForBase( rBaseDimName );
}

const ScDPSaveGroupDimension* ScDPDimensionSaveData::GetNamedGroupDim( const String& rGroupDimName ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetNamedGroupDimAcc( rGroupDimName );
}

const ScDPSaveGroupDimension* ScDPDimensionSaveData::GetFirstNamedGroupDim( const String& rBaseDimName ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetFirstNamedGroupDimAcc( rBaseDimName );
}

const ScDPSaveGroupDimension* ScDPDimensionSaveData::GetNextNamedGroupDim( const String& rGroupDimName ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetNextNamedGroupDimAcc( rGroupDimName );
}

const ScDPSaveNumGroupDimension* ScDPDimensionSaveData::GetNumGroupDim( const String& rGroupDimName ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetNumGroupDimAcc( rGroupDimName );
}

ScDPSaveGroupDimension* ScDPDimensionSaveData::GetGroupDimAccForBase( const String& rBaseDimName )
{
    ScDPSaveGroupDimension* pGroupDim = GetFirstNamedGroupDimAcc( rBaseDimName );
    return pGroupDim ? pGroupDim : GetNextNamedGroupDimAcc( rBaseDimName );
}

ScDPSaveGroupDimension* ScDPDimensionSaveData::GetNamedGroupDimAcc( const String& rGroupDimName )
{
    ScDPSaveGroupDimVec::iterator aIt = ::std::find_if(
        maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupDimNameFunc( rGroupDimName ) );
    return (aIt == maGroupDims.end()) ? 0 : &*aIt;
}

ScDPSaveGroupDimension* ScDPDimensionSaveData::GetFirstNamedGroupDimAcc( const String& rBaseDimName )
{
    ScDPSaveGroupDimVec::iterator aIt = ::std::find_if(
        maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupSourceNameFunc( rBaseDimName ) );
    return (aIt == maGroupDims.end()) ? 0 : &*aIt;
}

ScDPSaveGroupDimension* ScDPDimensionSaveData::GetNextNamedGroupDimAcc( const String& rGroupDimName )
{
    // find the group dimension with the passed name
    ScDPSaveGroupDimVec::iterator aIt = ::std::find_if(
        maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupDimNameFunc( rGroupDimName ) );
    // find next group dimension based on the same source dimension name
    if( aIt != maGroupDims.end() )
        aIt = ::std::find_if( aIt + 1, maGroupDims.end(), ScDPSaveGroupSourceNameFunc( aIt->GetSourceDimName() ) );
    return (aIt == maGroupDims.end()) ? 0 : &*aIt;
}

ScDPSaveNumGroupDimension* ScDPDimensionSaveData::GetNumGroupDimAcc( const String& rGroupDimName )
{
    ScDPSaveNumGroupDimMap::iterator aIt = maNumGroupDims.find( rGroupDimName );
    return (aIt == maNumGroupDims.end()) ? 0 : &aIt->second;
}

bool ScDPDimensionSaveData::HasGroupDimensions() const
{
    return !maGroupDims.empty() || !maNumGroupDims.empty();
}

sal_Int32 ScDPDimensionSaveData::CollectDateParts( const String& rBaseDimName ) const
{
    sal_Int32 nParts = 0;
    // start with part of numeric group
    if( const ScDPSaveNumGroupDimension* pNumDim = GetNumGroupDim( rBaseDimName ) )
        nParts |= pNumDim->GetDatePart();
    // collect parts from all matching group dimensions
    for( const ScDPSaveGroupDimension* pGroupDim = GetFirstNamedGroupDim( rBaseDimName ); pGroupDim; pGroupDim = GetNextNamedGroupDim( pGroupDim->GetGroupDimName() ) )
        nParts |= pGroupDim->GetDatePart();

    return nParts;
}

String ScDPDimensionSaveData::CreateGroupDimName( const String& rSourceName,
                                        const ScDPObject& rObject, bool bAllowSource,
                                        const std::vector<String>* pDeletedNames )
{
    // create a name for the new dimension by appending a number to the original
    // dimension's name

    bool bUseSource = bAllowSource;     // if set, try the unchanged original name first

    sal_Int32 nAdd = 2;                 // first try is "Name2"
    const sal_Int32 nMaxAdd = 1000;     // limit the loop
    while ( nAdd <= nMaxAdd )
    {
        String aDimName( rSourceName );
        if ( !bUseSource )
            aDimName.Append( String::CreateFromInt32( nAdd ) );
        bool bExists = false;

        // look for existing group dimensions
        for( ScDPSaveGroupDimVec::const_iterator aIt = maGroupDims.begin(), aEnd = maGroupDims.end(); (aIt != aEnd) && !bExists; ++aIt )
            if( aIt->GetGroupDimName() == aDimName )         //! ignore case
                bExists = true;

        // look for base dimensions that happen to have that name
        if ( !bExists && rObject.IsDimNameInUse( aDimName ) )
        {
            if ( pDeletedNames &&
                 std::find( pDeletedNames->begin(), pDeletedNames->end(), aDimName ) != pDeletedNames->end() )
            {
                // allow the name anyway if the name is in pDeletedNames
            }
            else
                bExists = true;
        }

        if ( !bExists )
            return aDimName;            // found a new name

        if ( bUseSource )
            bUseSource = false;
        else
            ++nAdd;                     // continue with higher number
    }
    DBG_ERROR("CreateGroupDimName: no valid name found");
    return EMPTY_STRING;
}

String ScDPDimensionSaveData::CreateDateGroupDimName( sal_Int32 nDatePart, const ScDPObject& rObject, bool bAllowSource, const ::std::vector< String >* pDeletedNames )
{
    using namespace ::com::sun::star::sheet::DataPilotFieldGroupBy;
    String aPartName;
    switch( nDatePart )
    {
        //! use translated strings from globstr.src
        case SECONDS:  aPartName = String::CreateFromAscii( "Seconds" );    break;
        case MINUTES:  aPartName = String::CreateFromAscii( "Minutes" );    break;
        case HOURS:    aPartName = String::CreateFromAscii( "Hours" );      break;
        case DAYS:     aPartName = String::CreateFromAscii( "Days" );       break;
        case MONTHS:   aPartName = String::CreateFromAscii( "Months" );     break;
        case QUARTERS: aPartName = String::CreateFromAscii( "Quarters" );   break;
        case YEARS:    aPartName = String::CreateFromAscii( "Years" );      break;
    }
    DBG_ASSERT( aPartName.Len() > 0, "ScDPDimensionSaveData::CreateDateGroupDimName - invalid date part" );
    return CreateGroupDimName( aPartName, rObject, bAllowSource, pDeletedNames );
}

// ============================================================================

