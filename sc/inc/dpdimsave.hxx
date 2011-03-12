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

#ifndef SC_DPDIMSAVE_HXX
#define SC_DPDIMSAVE_HXX

#include <vector>
#include <map>
#include <tools/string.hxx>
#include "dpgroup.hxx"      // for ScDPNumGroupInfo
#include "scdllapi.h"

class ScDPGroupTableData;
class ScDPGroupDimension;
class ScDPObject;
class ScStrCollection;
class SvNumberFormatter;

class ScDPSaveGroupDimension;

//
//  Classes to save Data Pilot settings that create new dimensions (fields).
//  These have to be applied before the other ScDPSaveData settings.
//

class SC_DLLPUBLIC ScDPSaveGroupItem
{
    String                  aGroupName;     // name of group
    ::std::vector<String>   aElements;      // names of items in original dimension

public:
                ScDPSaveGroupItem( const String& rName );
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

class SC_DLLPUBLIC ScDPSaveGroupDimension
{
    String                  aSourceDim;     // always the real source from the original data
    String                  aGroupDimName;
    ScDPSaveGroupItemVec    aGroups;
    ScDPNumGroupInfo        aDateInfo;
    sal_Int32               nDatePart;

public:
                ScDPSaveGroupDimension( const String& rSource, const String& rName );
                ScDPSaveGroupDimension( const String& rSource, const String& rName, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nPart );
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
    bool    HasOnlyHidden( const ScStrCollection& rVisible );

    long    GetGroupCount() const;
    const ScDPSaveGroupItem* GetGroupByIndex( long nIndex ) const;
    ScDPSaveGroupItem* GetGroupAccByIndex( long nIndex );

    void    Rename( const String& rNewName );
};

class SC_DLLPUBLIC ScDPSaveNumGroupDimension
{
    String              aDimensionName;
    ScDPNumGroupInfo    aGroupInfo;
    ScDPNumGroupInfo    aDateInfo;
    sal_Int32           nDatePart;

public:
                ScDPSaveNumGroupDimension( const String& rName, const ScDPNumGroupInfo& rInfo );
                ScDPSaveNumGroupDimension( const String& rName, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nPart );
                ~ScDPSaveNumGroupDimension();

    void        AddToData( ScDPGroupTableData& rData ) const;

    const String& GetDimensionName() const  { return aDimensionName; }
    const ScDPNumGroupInfo& GetInfo() const { return aGroupInfo; }

    sal_Int32   GetDatePart() const             { return nDatePart; }
    const ScDPNumGroupInfo& GetDateInfo() const { return aDateInfo; }

    void        SetGroupInfo( const ScDPNumGroupInfo& rNew );
    void        SetDateInfo( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart );
};

class SC_DLLPUBLIC ScDPDimensionSaveData
{
public:
            ScDPDimensionSaveData();
            ~ScDPDimensionSaveData();

    bool    operator==( const ScDPDimensionSaveData& r ) const;

    void    WriteToData( ScDPGroupTableData& rData ) const;

    String  CreateGroupDimName( const String& rSourceName, const ScDPObject& rObject, bool bAllowSource, const ::std::vector< String >* pDeletedNames );
    String  CreateDateGroupDimName( sal_Int32 nDatePart, const ScDPObject& rObject, bool bAllowSource, const ::std::vector< String >* pDeletedNames );

    void    AddGroupDimension( const ScDPSaveGroupDimension& rGroupDim );
    void    ReplaceGroupDimension( const ScDPSaveGroupDimension& rGroupDim );
    void    RemoveGroupDimension( const String& rGroupDimName );

    void    AddNumGroupDimension( const ScDPSaveNumGroupDimension& rGroupDim );
    void    ReplaceNumGroupDimension( const ScDPSaveNumGroupDimension& rGroupDim );
    void    RemoveNumGroupDimension( const String& rGroupDimName );

    const ScDPSaveGroupDimension* GetGroupDimForBase( const String& rBaseDimName ) const;
    const ScDPSaveGroupDimension* GetNamedGroupDim( const String& rGroupDimName ) const;
    const ScDPSaveGroupDimension* GetFirstNamedGroupDim( const String& rBaseDimName ) const;
    const ScDPSaveGroupDimension* GetNextNamedGroupDim( const String& rGroupDimName ) const;
    const ScDPSaveNumGroupDimension* GetNumGroupDim( const String& rGroupDimName ) const;

    ScDPSaveGroupDimension* GetGroupDimAccForBase( const String& rBaseDimName );
    ScDPSaveGroupDimension* GetNamedGroupDimAcc( const String& rGroupDimName );
    ScDPSaveGroupDimension* GetFirstNamedGroupDimAcc( const String& rBaseDimName );
    ScDPSaveGroupDimension* GetNextNamedGroupDimAcc( const String& rGroupDimName );

    ScDPSaveNumGroupDimension* GetNumGroupDimAcc( const String& rGroupDimName );

    bool    HasGroupDimensions() const;

    sal_Int32 CollectDateParts( const String& rBaseDimName ) const;

private:
    typedef ::std::vector< ScDPSaveGroupDimension >         ScDPSaveGroupDimVec;
    typedef ::std::map< String, ScDPSaveNumGroupDimension > ScDPSaveNumGroupDimMap;

    ScDPDimensionSaveData& operator=( const ScDPDimensionSaveData& );

    ScDPSaveGroupDimVec maGroupDims;
    ScDPSaveNumGroupDimMap maNumGroupDims;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
