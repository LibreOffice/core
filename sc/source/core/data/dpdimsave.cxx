/*************************************************************************
 *
 *  $RCSfile: dpdimsave.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:30:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "dpdimsave.hxx"
#include "dpgroup.hxx"
#include "dpobject.hxx"
#include "document.hxx"

#include <svtools/zforlist.hxx>
#include <tools/debug.hxx>
#include <rtl/math.hxx>
#include <algorithm>

// -----------------------------------------------------------------------

ScDPSaveGroupItem::ScDPSaveGroupItem( const String& rName ) :
    aGroupName( rName )
{
}

ScDPSaveGroupItem::ScDPSaveGroupItem( const ScDPSaveGroupItem& r ) :
    aGroupName( r.aGroupName ),
    aElements( r.aElements )
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
        ULONG nFormat = 0;      //! ...
        double fValue;
        if ( pFormatter->IsNumberFormat( *aIter, nFormat, fValue ) )
            aData = ScDPItemData( *aIter, fValue, TRUE );
        else
            aData.SetString( *aIter );

        aGroup.AddElement( aData );
        //! for numeric data, look at source members?
    }

    rDataDim.AddItem( aGroup );
}

// -----------------------------------------------------------------------

ScDPSaveGroupDimension::ScDPSaveGroupDimension( const String& rSource, const String& rName ) :
    aSourceDim( rSource ),
    aGroupDimName( rName ),
    nDatePart( 0 )
{
}

ScDPSaveGroupDimension::ScDPSaveGroupDimension( const ScDPSaveGroupDimension& r ) :
    aSourceDim( r.aSourceDim ),
    aGroupDimName( r.aGroupDimName ),
    aGroups( r.aGroups ),
    aDateInfo( r.aDateInfo ),
    nDatePart( r.nDatePart )
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

bool ScDPSaveGroupDimension::HasOnlyHidden( const StrCollection& rVisible )
{
    // check if there are only groups that don't appear in the list of visible names

    bool bAllHidden = true;
    for ( ScDPSaveGroupItemVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end() && bAllHidden; aIter++ )
    {
        StrData aSearch( aIter->GetGroupName() );
        USHORT nCollIndex;
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

// -----------------------------------------------------------------------

ScDPSaveNumGroupDimension::ScDPSaveNumGroupDimension( const String& rName, const ScDPNumGroupInfo& rInfo ) :
    aDimensionName( rName ),
    aGroupInfo( rInfo ),
    nDatePart( 0 )
{
}

ScDPSaveNumGroupDimension::ScDPSaveNumGroupDimension( const ScDPSaveNumGroupDimension& r ) :
    aDimensionName( r.aDimensionName ),
    aGroupInfo( r.aGroupInfo ),
    aDateInfo( r.aDateInfo ),
    nDatePart( r.nDatePart )
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

// -----------------------------------------------------------------------

ScDPDimensionSaveData::ScDPDimensionSaveData()
{
}

ScDPDimensionSaveData::ScDPDimensionSaveData( const ScDPDimensionSaveData& r ) :
    aGroupDimensions( r.aGroupDimensions ),
    aNumGroupDimensions( r.aNumGroupDimensions )
{
}

ScDPDimensionSaveData::~ScDPDimensionSaveData()
{
}

bool ScDPDimensionSaveData::operator==( const ScDPDimensionSaveData& r ) const
{
    return false;
}

void ScDPDimensionSaveData::AddGroupDimension( const ScDPSaveGroupDimension& rGroup )
{
    aGroupDimensions.push_back( rGroup );
}

void ScDPDimensionSaveData::AddNumGroupDimension( const ScDPSaveNumGroupDimension& rGroup )
{
    aNumGroupDimensions.push_back( rGroup );
}

void ScDPDimensionSaveData::RemoveGroupDimension( const String& rDimensionName )
{
    for ( ScDPSaveGroupDimensionVec::iterator aIter(aGroupDimensions.begin());
                                        aIter != aGroupDimensions.end(); aIter++ )
    {
        if ( aIter->GetGroupDimName() == rDimensionName )       //! ignore case
        {
            aGroupDimensions.erase( aIter );    // found -> remove
            return;                             // don't have to look further
        }
    }
}

void ScDPDimensionSaveData::RemoveNumGroupDimension( const String& rDimensionName )
{
    for ( ScDPSaveNumGroupDimensionVec::iterator aIter(aNumGroupDimensions.begin());
                                           aIter != aNumGroupDimensions.end(); aIter++ )
    {
        if ( aIter->GetDimensionName() == rDimensionName )      //! ignore case
        {
            aNumGroupDimensions.erase( aIter );    // found -> remove
            return;                                // don't have to look further
        }
    }
}

void ScDPDimensionSaveData::WriteToData( ScDPGroupTableData& rData ) const
{
    //  rData is assumed to be empty
    //  AddToData also handles date grouping

    for ( ScDPSaveGroupDimensionVec::const_iterator aIter(aGroupDimensions.begin());
                                     aIter != aGroupDimensions.end(); aIter++ )
        aIter->AddToData( rData );

    for ( ScDPSaveNumGroupDimensionVec::const_iterator aNumIter(aNumGroupDimensions.begin());
                                        aNumIter != aNumGroupDimensions.end(); aNumIter++ )
        aNumIter->AddToData( rData );
}

ScDPSaveGroupDimension* ScDPDimensionSaveData::GetGroupDimAccForBase( const String& rBaseName )
{
    String aOrigBase( rBaseName );      // the original base dim in the case of nested group dims

    for ( ScDPSaveGroupDimensionVec::iterator aIter(aGroupDimensions.begin());
                                     aIter != aGroupDimensions.end(); aIter++ )
    {
        if ( aIter->GetSourceDimName() == aOrigBase )         //! ignore case
        {
            return &*aIter;             // return the first one directly - it's the immediate derivative
        }
        else if ( aIter->GetGroupDimName() == aOrigBase )
        {
            // found the definition of rBaseName in the array - look for the next occurrence
            // of this definitions's base

            aOrigBase = aIter->GetSourceDimName();      // and continue the loop
        }
    }

    return NULL;    // none
}

const ScDPSaveGroupDimension* ScDPDimensionSaveData::GetGroupDimForBase( const String& rBaseName ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetGroupDimAccForBase( rBaseName );
}

const ScDPSaveGroupDimension* ScDPDimensionSaveData::GetNamedGroupDim( const String& rGroupDim ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetNamedGroupDimAcc( rGroupDim );
}

const ScDPSaveNumGroupDimension* ScDPDimensionSaveData::GetNumGroupDim( const String& rName ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetNumGroupDimAcc( rName );
}

ScDPSaveGroupDimension* ScDPDimensionSaveData::GetNamedGroupDimAcc( const String& rGroupDim )
{
    for ( ScDPSaveGroupDimensionVec::iterator aIter(aGroupDimensions.begin());
                                     aIter != aGroupDimensions.end(); aIter++ )
        if ( aIter->GetGroupDimName() == rGroupDim )         //! ignore case
            return &*aIter;            // there should be only one - use it directly

    return NULL;    // none
}

ScDPSaveNumGroupDimension* ScDPDimensionSaveData::GetNumGroupDimAcc( const String& rName )
{
    for ( ScDPSaveNumGroupDimensionVec::iterator aIter(aNumGroupDimensions.begin());
                                        aIter != aNumGroupDimensions.end(); aIter++ )
        if ( aIter->GetDimensionName() == rName )         //! ignore case
            return &*aIter;            // there should be only one - use it directly

    return NULL;    // none
}

bool ScDPDimensionSaveData::HasGroupDimensions() const
{
    return !aGroupDimensions.empty() || !aNumGroupDimensions.empty();
}

sal_Int32 ScDPDimensionSaveData::CollectDateParts( const String& rBaseDimName ) const
{
    sal_Int32 nParts = 0;
    const ScDPSaveNumGroupDimension* pNumDim = GetNumGroupDim( rBaseDimName );
    if ( pNumDim )
        nParts |= pNumDim->GetDatePart();

    for ( ScDPSaveGroupDimensionVec::const_iterator aIter(aGroupDimensions.begin());
                                     aIter != aGroupDimensions.end(); aIter++ )
        if ( aIter->GetSourceDimName() == rBaseDimName )         //! ignore case
        {
            nParts |= aIter->GetDatePart();
            // continue loop, collect parts from all matching group dimensions
        }

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
        for ( ScDPSaveGroupDimensionVec::const_iterator aIter(aGroupDimensions.begin());
                                         aIter != aGroupDimensions.end() && !bExists; aIter++ )
            if ( aIter->GetGroupDimName() == aDimName )         //! ignore case
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

