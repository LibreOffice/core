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

#ifndef INCLUDED_SC_INC_DPDIMSAVE_HXX
#define INCLUDED_SC_INC_DPDIMSAVE_HXX

#include <vector>
#include <map>
#include "dpitemdata.hxx"
#include "dpnumgroupinfo.hxx"
#include "scdllapi.h"
#include "dptypes.hxx"

class ScDPGroupTableData;
class ScDPGroupDimension;
class ScDPObject;
class ScDPCache;
class SvNumberFormatter;

class ScDPSaveGroupDimension;

/**
 * Classes to save Data Pilot settings that create new dimensions (fields).
 * These have to be applied before the other ScDPSaveData settings.
 */

class SC_DLLPUBLIC ScDPSaveGroupItem
{
    OUString aGroupName;                        ///< name of group
    std::vector<OUString> aElements;            ///< names of items in original dimension
    mutable std::vector<ScDPItemData> maItems;  ///< items converted from the strings.

public:
    ScDPSaveGroupItem( const OUString& rName );
    ~ScDPSaveGroupItem();

    void AddToData(ScDPGroupDimension& rDataDim) const;

    void    AddElement( const OUString& rName );
    void    AddElementsFromGroup( const ScDPSaveGroupItem& rGroup );
    const OUString& GetGroupName() const { return aGroupName; }

    /// @return true if found (removed)
    bool    RemoveElement( const OUString& rName );

    bool    IsEmpty() const;
    size_t  GetElementCount() const;
    const OUString* GetElementByIndex(size_t nIndex) const;

    void Rename( const OUString& rNewName );

    /** remove this group's elements from their groups in rDimension
     (rDimension must be a different dimension from the one which contains this)*/
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
    OUString           aSourceDim;     ///< always the real source from the original data
    OUString           aGroupDimName;
    ScDPSaveGroupItemVec    aGroups;
    mutable ScDPNumGroupInfo aDateInfo;
    sal_Int32               nDatePart;

public:
                ScDPSaveGroupDimension( const OUString& rSource, const OUString& rName );
                ScDPSaveGroupDimension( const OUString& rSource, const OUString& rName, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nPart );

    void    AddToData( ScDPGroupTableData& rData ) const;
    void AddToCache(ScDPCache& rCache) const;
    void    SetDateInfo( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart );

    void    AddGroupItem( const ScDPSaveGroupItem& rItem );
    const OUString& GetGroupDimName() const { return aGroupDimName; }
    const OUString& GetSourceDimName() const { return aSourceDim; }

    sal_Int32   GetDatePart() const             { return nDatePart; }
    const ScDPNumGroupInfo& GetDateInfo() const { return aDateInfo; }

    OUString CreateGroupName( const OUString& rPrefix );
    const ScDPSaveGroupItem* GetNamedGroup( const OUString& rGroupName ) const;
    ScDPSaveGroupItem* GetNamedGroupAcc( const OUString& rGroupName );
    void    RemoveFromGroups( const OUString& rItemName );
    void RemoveGroup(const OUString& rGroupName);
    bool    IsEmpty() const;
    bool HasOnlyHidden(const ScDPUniqueStringSet& rVisible);

    long    GetGroupCount() const;
    const ScDPSaveGroupItem& GetGroupByIndex( long nIndex ) const;

    void    Rename( const OUString& rNewName );

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
    OUString       aDimensionName;
    mutable ScDPNumGroupInfo aGroupInfo;
    mutable ScDPNumGroupInfo aDateInfo;
    sal_Int32           nDatePart;

public:
                ScDPSaveNumGroupDimension( const OUString& rName, const ScDPNumGroupInfo& rInfo );
                ScDPSaveNumGroupDimension( const OUString& rName, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nPart );

    void        AddToData( ScDPGroupTableData& rData ) const;
    void AddToCache(ScDPCache& rCache) const;

    const OUString& GetDimensionName() const  { return aDimensionName; }
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

    bool    operator==( const ScDPDimensionSaveData& r ) const;

    void    WriteToData( ScDPGroupTableData& rData ) const;

    void WriteToCache(ScDPCache& rCache) const;

    OUString CreateGroupDimName(
        const OUString& rSourceName, const ScDPObject& rObject, bool bAllowSource,
        const ::std::vector<OUString>* pDeletedNames );

    OUString CreateDateGroupDimName(
        sal_Int32 nDatePart, const ScDPObject& rObject, bool bAllowSource,
        const ::std::vector<OUString>* pDeletedNames );

    void    AddGroupDimension( const ScDPSaveGroupDimension& rGroupDim );
    void    ReplaceGroupDimension( const ScDPSaveGroupDimension& rGroupDim );
    void    RemoveGroupDimension( const OUString& rGroupDimName );

    void    AddNumGroupDimension( const ScDPSaveNumGroupDimension& rGroupDim );
    void    ReplaceNumGroupDimension( const ScDPSaveNumGroupDimension& rGroupDim );
    void    RemoveNumGroupDimension( const OUString& rGroupDimName );

    const ScDPSaveGroupDimension* GetGroupDimForBase( const OUString& rBaseDimName ) const;
    const ScDPSaveGroupDimension* GetNamedGroupDim( const OUString& rGroupDimName ) const;
    const ScDPSaveGroupDimension* GetFirstNamedGroupDim( const OUString& rBaseDimName ) const;
    const ScDPSaveGroupDimension* GetNextNamedGroupDim( const OUString& rGroupDimName ) const;
    const ScDPSaveNumGroupDimension* GetNumGroupDim( const OUString& rGroupDimName ) const;

    ScDPSaveGroupDimension* GetGroupDimAccForBase( const OUString& rBaseDimName );
    ScDPSaveGroupDimension* GetNamedGroupDimAcc( const OUString& rGroupDimName );
    ScDPSaveGroupDimension* GetFirstNamedGroupDimAcc( const OUString& rBaseDimName );
    ScDPSaveGroupDimension* GetNextNamedGroupDimAcc( const OUString& rGroupDimName );

    ScDPSaveNumGroupDimension* GetNumGroupDimAcc( const OUString& rGroupDimName );

    bool    HasGroupDimensions() const;

    sal_Int32 CollectDateParts( const OUString& rBaseDimName ) const;

private:
    typedef ::std::vector< ScDPSaveGroupDimension >         ScDPSaveGroupDimVec;
    typedef ::std::map<OUString, ScDPSaveNumGroupDimension> ScDPSaveNumGroupDimMap;

    ScDPDimensionSaveData& operator=( const ScDPDimensionSaveData& ) = delete;

    ScDPSaveGroupDimVec maGroupDims;
    ScDPSaveNumGroupDimMap maNumGroupDims;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
