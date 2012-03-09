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
#include "dptypes.hxx"

class ScDPGroupTableData;
class ScDPGroupDimension;
class ScDPObject;
class ScDPCache;
class SvNumberFormatter;

class ScDPSaveGroupDimension;

//
//  Classes to save Data Pilot settings that create new dimensions (fields).
//  These have to be applied before the other ScDPSaveData settings.
//

class SC_DLLPUBLIC ScDPSaveGroupItem
{
    rtl::OUString aGroupName;     // name of group
    std::vector<rtl::OUString> aElements;      // names of items in original dimension
    mutable std::vector<ScDPItemData> maItems; // items converted from the strings.

public:
    ScDPSaveGroupItem( const rtl::OUString& rName );
    ~ScDPSaveGroupItem();

    void AddToData(ScDPGroupDimension& rDataDim) const;

    void    AddElement( const rtl::OUString& rName );
    void    AddElementsFromGroup( const ScDPSaveGroupItem& rGroup );
    const rtl::OUString& GetGroupName() const { return aGroupName; }
    bool    RemoveElement( const rtl::OUString& rName );   // returns true if found (removed)

    bool    IsEmpty() const;
    size_t  GetElementCount() const;
    const rtl::OUString* GetElementByIndex(size_t nIndex) const;

    void Rename( const rtl::OUString& rNewName );

    // remove this group's elements from their groups in rDimension
    // (rDimension must be a different dimension from the one which contains this)
    void    RemoveElementsFromGroups( ScDPSaveGroupDimension& rDimension ) const;

    void ConvertElementsToItems(SvNumberFormatter* pFormatter) const;
    bool HasInGroup(const ScDPItemData& rItem) const;
};

typedef ::std::vector<ScDPSaveGroupItem> ScDPSaveGroupItemVec;

/**
 * Represents a new group dimension whose dimension ID is higher than the
 * highest source dimension ID.
 */
class SC_DLLPUBLIC ScDPSaveGroupDimension
{
    rtl::OUString           aSourceDim;     // always the real source from the original data
    rtl::OUString           aGroupDimName;
    ScDPSaveGroupItemVec    aGroups;
    mutable ScDPNumGroupInfo aDateInfo;
    sal_Int32               nDatePart;

public:
                ScDPSaveGroupDimension( const rtl::OUString& rSource, const rtl::OUString& rName );
                ScDPSaveGroupDimension( const rtl::OUString& rSource, const rtl::OUString& rName, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nPart );
                ~ScDPSaveGroupDimension();

    void    AddToData( ScDPGroupTableData& rData ) const;
    void AddToCache(ScDPCache& rCache) const;
    void    SetDateInfo( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart );

    void    AddGroupItem( const ScDPSaveGroupItem& rItem );
    const rtl::OUString& GetGroupDimName() const { return aGroupDimName; }
    const rtl::OUString& GetSourceDimName() const { return aSourceDim; }

    sal_Int32   GetDatePart() const             { return nDatePart; }
    const ScDPNumGroupInfo& GetDateInfo() const { return aDateInfo; }

    rtl::OUString CreateGroupName( const rtl::OUString& rPrefix );
    const ScDPSaveGroupItem* GetNamedGroup( const rtl::OUString& rGroupName ) const;
    ScDPSaveGroupItem* GetNamedGroupAcc( const rtl::OUString& rGroupName );
    void    RemoveFromGroups( const rtl::OUString& rItemName );
    void RemoveGroup(const rtl::OUString& rGroupName);
    bool    IsEmpty() const;
    bool HasOnlyHidden(const ScDPUniqueStringSet& rVisible);

    long    GetGroupCount() const;
    const ScDPSaveGroupItem* GetGroupByIndex( long nIndex ) const;
    ScDPSaveGroupItem* GetGroupAccByIndex( long nIndex );

    void    Rename( const rtl::OUString& rNewName );

private:
    bool IsInGroup(const ScDPItemData& rItem) const;
};

/**
 * Represents a group dimension that introduces a new hierarchy for an
 * existing dimension.  Unlike the ScDPSaveGroupDimension counterpart, it
 * re-uses the source dimension name and ID.
 */
class SC_DLLPUBLIC ScDPSaveNumGroupDimension
{
    rtl::OUString       aDimensionName;
    mutable ScDPNumGroupInfo aGroupInfo;
    mutable ScDPNumGroupInfo aDateInfo;
    sal_Int32           nDatePart;

public:
                ScDPSaveNumGroupDimension( const rtl::OUString& rName, const ScDPNumGroupInfo& rInfo );
                ScDPSaveNumGroupDimension( const rtl::OUString& rName, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nPart );
                ~ScDPSaveNumGroupDimension();

    void        AddToData( ScDPGroupTableData& rData ) const;
    void AddToCache(ScDPCache& rCache) const;

    const rtl::OUString& GetDimensionName() const  { return aDimensionName; }
    const ScDPNumGroupInfo& GetInfo() const { return aGroupInfo; }

    sal_Int32   GetDatePart() const             { return nDatePart; }
    const ScDPNumGroupInfo& GetDateInfo() const { return aDateInfo; }

    void        SetGroupInfo( const ScDPNumGroupInfo& rNew );
    void        SetDateInfo( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart );
};

/**
 * This class has to do with handling exclusively grouped dimensions?  TODO:
 * Find out what this class does and document it here.
 */
class SC_DLLPUBLIC ScDPDimensionSaveData
{
public:
            ScDPDimensionSaveData();
            ~ScDPDimensionSaveData();

    bool    operator==( const ScDPDimensionSaveData& r ) const;

    void    WriteToData( ScDPGroupTableData& rData ) const;

    void WriteToCache(ScDPCache& rCache) const;

    rtl::OUString CreateGroupDimName(
        const rtl::OUString& rSourceName, const ScDPObject& rObject, bool bAllowSource,
        const ::std::vector<rtl::OUString>* pDeletedNames );

    rtl::OUString CreateDateGroupDimName(
        sal_Int32 nDatePart, const ScDPObject& rObject, bool bAllowSource,
        const ::std::vector<rtl::OUString>* pDeletedNames );

    void    AddGroupDimension( const ScDPSaveGroupDimension& rGroupDim );
    void    ReplaceGroupDimension( const ScDPSaveGroupDimension& rGroupDim );
    void    RemoveGroupDimension( const rtl::OUString& rGroupDimName );

    void    AddNumGroupDimension( const ScDPSaveNumGroupDimension& rGroupDim );
    void    ReplaceNumGroupDimension( const ScDPSaveNumGroupDimension& rGroupDim );
    void    RemoveNumGroupDimension( const rtl::OUString& rGroupDimName );

    const ScDPSaveGroupDimension* GetGroupDimForBase( const rtl::OUString& rBaseDimName ) const;
    const ScDPSaveGroupDimension* GetNamedGroupDim( const rtl::OUString& rGroupDimName ) const;
    const ScDPSaveGroupDimension* GetFirstNamedGroupDim( const rtl::OUString& rBaseDimName ) const;
    const ScDPSaveGroupDimension* GetNextNamedGroupDim( const rtl::OUString& rGroupDimName ) const;
    const ScDPSaveNumGroupDimension* GetNumGroupDim( const rtl::OUString& rGroupDimName ) const;

    ScDPSaveGroupDimension* GetGroupDimAccForBase( const rtl::OUString& rBaseDimName );
    ScDPSaveGroupDimension* GetNamedGroupDimAcc( const rtl::OUString& rGroupDimName );
    ScDPSaveGroupDimension* GetFirstNamedGroupDimAcc( const rtl::OUString& rBaseDimName );
    ScDPSaveGroupDimension* GetNextNamedGroupDimAcc( const rtl::OUString& rGroupDimName );

    ScDPSaveNumGroupDimension* GetNumGroupDimAcc( const rtl::OUString& rGroupDimName );

    bool    HasGroupDimensions() const;

    sal_Int32 CollectDateParts( const rtl::OUString& rBaseDimName ) const;

private:
    typedef ::std::vector< ScDPSaveGroupDimension >         ScDPSaveGroupDimVec;
    typedef ::std::map<rtl::OUString, ScDPSaveNumGroupDimension> ScDPSaveNumGroupDimMap;

    ScDPDimensionSaveData& operator=( const ScDPDimensionSaveData& );

    ScDPSaveGroupDimVec maGroupDims;
    ScDPSaveNumGroupDimMap maNumGroupDims;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
