/*************************************************************************
 *
 *  $RCSfile: dpdimsave.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:28:38 $
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

#ifndef SC_DPDIMSAVE_HXX
#define SC_DPDIMSAVE_HXX

#include <vector>

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef SC_DPGROUP_HXX
#include "dpgroup.hxx"      // for ScDPNumGroupInfo
#endif

class ScDPGroupTableData;
class ScDPGroupDimension;
class ScDPObject;
class StrCollection;
class SvNumberFormatter;

class ScDPSaveGroupDimension;

// --------------------------------------------------------------------
//
//  Classes to save Data Pilot settings that create new dimensions (fields).
//  These have to be applied before the other ScDPSaveData settings.
//

class ScDPSaveGroupItem
{
    String                  aGroupName;     // name of group
    ::std::vector<String>   aElements;      // names of items in original dimension

public:
                ScDPSaveGroupItem( const String& rName );
                ScDPSaveGroupItem( const ScDPSaveGroupItem& r );
                ~ScDPSaveGroupItem();

    void    AddToData( ScDPGroupDimension& rDataDim, SvNumberFormatter* pFormatter ) const;

    void    AddElement( const String& rName );
    void    AddElementsFromGroup( const ScDPSaveGroupItem& rGroup );
    const String& GetGroupName() const   { return aGroupName; }
    bool    RemoveElement( const String& rName );   // returns true if found (removed)

    bool    IsEmpty() const;
    size_t  GetElementCount() const;
    const String* GetElementByIndex( size_t nIndex ) const;

    void    Rename( const String& rNewName );

    // remove this group's elements from their groups in rDimension
    // (rDimension must be a different dimension from the one which contains this)
    void    RemoveElementsFromGroups( ScDPSaveGroupDimension& rDimension ) const;
};

typedef ::std::vector<ScDPSaveGroupItem> ScDPSaveGroupItemVec;

class ScDPSaveGroupDimension
{
    String                  aSourceDim;     // always the real source from the original data
    String                  aGroupDimName;
    ScDPSaveGroupItemVec    aGroups;
    ScDPNumGroupInfo        aDateInfo;
    sal_Int32               nDatePart;

public:
                ScDPSaveGroupDimension( const String& rSource, const String& rName );
                ScDPSaveGroupDimension( const ScDPSaveGroupDimension& r );
                ~ScDPSaveGroupDimension();

    void    AddToData( ScDPGroupTableData& rData ) const;

    void    SetDateInfo( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart );

    void    AddGroupItem( const ScDPSaveGroupItem& rItem );
    const String& GetGroupDimName() const   { return aGroupDimName; }
    const String& GetSourceDimName() const  { return aSourceDim; }

    sal_Int32   GetDatePart() const             { return nDatePart; }
    const ScDPNumGroupInfo& GetDateInfo() const { return aDateInfo; }

    String  CreateGroupName( const String& rPrefix );
    const ScDPSaveGroupItem* GetNamedGroup( const String& rGroupName ) const;
    ScDPSaveGroupItem* GetNamedGroupAcc( const String& rGroupName );
    void    RemoveFromGroups( const String& rItemName );
    void    RemoveGroup( const String& rGroupName );
    bool    IsEmpty() const;
    bool    HasOnlyHidden( const StrCollection& rVisible );

    long    GetGroupCount() const;
    const ScDPSaveGroupItem* GetGroupByIndex( long nIndex ) const;
    ScDPSaveGroupItem* GetGroupAccByIndex( long nIndex );

    void    Rename( const String& rNewName );
};

typedef ::std::vector<ScDPSaveGroupDimension> ScDPSaveGroupDimensionVec;

class ScDPSaveNumGroupDimension
{
    String              aDimensionName;
    ScDPNumGroupInfo    aGroupInfo;
    ScDPNumGroupInfo    aDateInfo;
    sal_Int32           nDatePart;

public:
                ScDPSaveNumGroupDimension( const String& rName, const ScDPNumGroupInfo& rInfo );
                ScDPSaveNumGroupDimension( const ScDPSaveNumGroupDimension& r );
                ~ScDPSaveNumGroupDimension();

    void        AddToData( ScDPGroupTableData& rData ) const;

    const String& GetDimensionName() const  { return aDimensionName; }
    const ScDPNumGroupInfo& GetInfo() const { return aGroupInfo; }

    sal_Int32   GetDatePart() const             { return nDatePart; }
    const ScDPNumGroupInfo& GetDateInfo() const { return aDateInfo; }

    void        SetGroupInfo( const ScDPNumGroupInfo& rNew );
    void        SetDateInfo( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart );
};

typedef ::std::vector<ScDPSaveNumGroupDimension> ScDPSaveNumGroupDimensionVec;

class ScDPDimensionSaveData
{
    ScDPSaveGroupDimensionVec    aGroupDimensions;
    ScDPSaveNumGroupDimensionVec aNumGroupDimensions;


                            // not implemented
    ScDPDimensionSaveData&  operator=( const ScDPDimensionSaveData& );

public:
            ScDPDimensionSaveData();
            ScDPDimensionSaveData( const ScDPDimensionSaveData& r );
            ~ScDPDimensionSaveData();

    bool    operator==( const ScDPDimensionSaveData& r ) const;

    void    WriteToData( ScDPGroupTableData& rData ) const;

    String  CreateGroupDimName( const String& rSourceName, const ScDPObject& rObject, bool bAllowSource,
                                const std::vector<String>* pDeletedNames );
    void    AddGroupDimension( const ScDPSaveGroupDimension& rGroup );
    void    RemoveGroupDimension( const String& rDimensionName );

    void    AddNumGroupDimension( const ScDPSaveNumGroupDimension& rGroup );
    void    RemoveNumGroupDimension( const String& rDimensionName );

    const ScDPSaveGroupDimension* GetGroupDimForBase( const String& rBaseName ) const;
    const ScDPSaveGroupDimension* GetNamedGroupDim( const String& rGroupDim ) const;
    const ScDPSaveNumGroupDimension* GetNumGroupDim( const String& rName ) const;

    ScDPSaveGroupDimension* GetGroupDimAccForBase( const String& rBaseName );
    ScDPSaveGroupDimension* GetNamedGroupDimAcc( const String& rGroupDim );
    ScDPSaveNumGroupDimension* GetNumGroupDimAcc( const String& rName );

    bool    HasGroupDimensions() const;

    sal_Int32 CollectDateParts( const String& rBaseDimName ) const;
};

#endif

