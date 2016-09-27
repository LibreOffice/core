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

#ifndef INCLUDED_SC_INC_DPSAVE_HXX
#define INCLUDED_SC_INC_DPSAVE_HXX

#include <list>
#include <memory>
#include <vector>

#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "scdllapi.h"
#include "calcmacros.hxx"

#include <unordered_map>
#include <unordered_set>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldReference;
    struct DataPilotFieldSortInfo;
    struct DataPilotFieldAutoShowInfo;
    struct DataPilotFieldLayoutInfo;
} } } }

class ScDPDimensionSaveData;
class ScDPTableData;

// classes to save Data Pilot settings

class ScDPSaveMember
{
private:
    OUString aName;
    std::unique_ptr<OUString> mpLayoutName; // custom name to be displayed in the table.
    sal_uInt16 nVisibleMode;
    sal_uInt16 nShowDetailsMode;

public:
    ScDPSaveMember(const OUString& rName);
    ScDPSaveMember(const ScDPSaveMember& r);
    ~ScDPSaveMember();

    bool operator== ( const ScDPSaveMember& r ) const;

    const OUString& GetName() const
        { return aName; }

    SC_DLLPUBLIC bool HasIsVisible() const;
    SC_DLLPUBLIC void SetIsVisible(bool bSet);
    bool GetIsVisible() const
        { return bool(nVisibleMode); }

    SC_DLLPUBLIC bool HasShowDetails() const;
    SC_DLLPUBLIC void SetShowDetails(bool bSet);
    bool GetShowDetails() const
        { return bool(nShowDetailsMode); }

    void SetName( const OUString& rNew ); // used if the source member was renamed (groups)

    SC_DLLPUBLIC void SetLayoutName( const OUString& rName );
    SC_DLLPUBLIC const OUString*  GetLayoutName() const;
    void RemoveLayoutName();

    void WriteToSource( const css::uno::Reference<css::uno::XInterface>& xMember,
                            sal_Int32 nPosition );

#if DEBUG_PIVOT_TABLE
    void Dump(int nIndent = 0) const;
#endif
};

bool operator == (const css::sheet::DataPilotFieldSortInfo &l, const css::sheet::DataPilotFieldSortInfo &r );
bool operator == (const css::sheet::DataPilotFieldAutoShowInfo &l, const css::sheet::DataPilotFieldAutoShowInfo &r );
bool operator == (const css::sheet::DataPilotFieldReference &l, const css::sheet::DataPilotFieldReference &r );

class SC_DLLPUBLIC ScDPSaveDimension
{
private:
    OUString aName;
    std::unique_ptr<OUString> mpLayoutName;
    std::unique_ptr<OUString> mpSubtotalName;
    bool bIsDataLayout;
    bool bDupFlag;
    sal_uInt16 nOrientation;
    sal_uInt16 nFunction; // enum GeneralFunction, for data dimensions
    long nUsedHierarchy;
    sal_uInt16 nShowEmptyMode; //! at level
    bool bRepeatItemLabels; //! at level
    bool bSubTotalDefault; //! at level
    long nSubTotalCount;
    sal_uInt16* pSubTotalFuncs; // enum GeneralFunction
    css::sheet::DataPilotFieldReference* pReferenceValue;
    css::sheet::DataPilotFieldSortInfo* pSortInfo; // (level)
    css::sheet::DataPilotFieldAutoShowInfo* pAutoShowInfo; // (level)
    css::sheet::DataPilotFieldLayoutInfo* pLayoutInfo; // (level)

public:
    typedef std::unordered_set<OUString, OUStringHash> MemberSetType;
    typedef std::unordered_map <OUString, ScDPSaveMember*, OUStringHash> MemberHash;
    typedef std::list <ScDPSaveMember*> MemberList;

private:
    MemberHash maMemberHash;
    MemberList maMemberList;

public:
    ScDPSaveDimension(const OUString& rName, bool bDataLayout);
    ScDPSaveDimension(const ScDPSaveDimension& r);
    ~ScDPSaveDimension();

    bool operator== ( const ScDPSaveDimension& r ) const;

    const MemberList& GetMembers() const
        { return maMemberList; }

    void AddMember(ScDPSaveMember* pMember);

    void SetDupFlag(bool bSet)
        { bDupFlag = bSet; }

    bool GetDupFlag() const
        { return bDupFlag; }

    const OUString& GetName() const
        { return aName; }

    bool IsDataLayout() const
        { return bIsDataLayout; }

    void SetName( const OUString& rNew ); // used if the source dim was renamed (groups)

    void SetOrientation(sal_uInt16 nNew);
    void SetSubTotals(long nCount, const sal_uInt16* pFuncs);
    long GetSubTotalsCount() const
        { return nSubTotalCount; }

    sal_uInt16 GetSubTotalFunc(long nIndex) const
        { return pSubTotalFuncs[nIndex]; }

    bool HasShowEmpty() const;
    void SetShowEmpty(bool bSet);
    bool GetShowEmpty() const
        { return bool(nShowEmptyMode); }

    void SetRepeatItemLabels(bool bSet);
    bool GetRepeatItemLabels() const
        { return bRepeatItemLabels; }

    void SetFunction(sal_uInt16 nNew); // enum GeneralFunction
    sal_uInt16 GetFunction() const
        { return nFunction; }

    void SetUsedHierarchy(long nNew);
    long GetUsedHierarchy() const
        { return nUsedHierarchy; }

    void SetLayoutName(const OUString& rName);
    const OUString* GetLayoutName() const;
    void RemoveLayoutName();
    void SetSubtotalName(const OUString& rName);
    const OUString* GetSubtotalName() const;
    void RemoveSubtotalName();

    bool IsMemberNameInUse(const OUString& rName) const;

    const css::sheet::DataPilotFieldReference* GetReferenceValue() const
        { return pReferenceValue; }

    void SetReferenceValue(const css::sheet::DataPilotFieldReference* pNew);

    const css::sheet::DataPilotFieldSortInfo* GetSortInfo() const
        { return pSortInfo; }

    void SetSortInfo(const css::sheet::DataPilotFieldSortInfo* pNew);
    const css::sheet::DataPilotFieldAutoShowInfo* GetAutoShowInfo() const
        { return pAutoShowInfo; }

    void SetAutoShowInfo(const css::sheet::DataPilotFieldAutoShowInfo* pNew);
    const css::sheet::DataPilotFieldLayoutInfo* GetLayoutInfo() const
        { return pLayoutInfo; }

    void SetLayoutInfo(const css::sheet::DataPilotFieldLayoutInfo* pNew);

    void SetCurrentPage( const OUString* pPage ); // NULL = no selection (all)
    OUString GetCurrentPage() const; // only for ODF compatibility

    sal_uInt16 GetOrientation() const
        { return nOrientation; }

    ScDPSaveMember* GetExistingMemberByName(const OUString& rName);

    /**
     * Get a member object by its name.  If one doesn't exist, create a new
     * object and return it.  This class manages the life cycle of all member
     * objects belonging to it, so <i>don't delete the returned instance.</i>
     *
     * @param rName member name
     *
     * @return pointer to the member object.
     */
    ScDPSaveMember* GetMemberByName(const OUString& rName);

    void SetMemberPosition( const OUString& rName, sal_Int32 nNewPos );

    void WriteToSource( const css::uno::Reference<css::uno::XInterface>& xDim );

    void UpdateMemberVisibility(const std::unordered_map< OUString, bool, OUStringHash>& rData);

    bool HasInvisibleMember() const;

    void RemoveObsoleteMembers(const MemberSetType& rMembers);

#if DEBUG_PIVOT_TABLE
    void Dump(int nIndent = 0) const;
#endif
};

class ScDPSaveData
{
    typedef std::unordered_map<OUString, size_t, OUStringHash> DupNameCountType;
public:
    typedef std::unordered_map<OUString, size_t, OUStringHash> DimOrderType;
    typedef std::vector<std::unique_ptr<ScDPSaveDimension>> DimsType;

private:
    DimsType m_DimList;
    DupNameCountType maDupNameCounts; /// keep track of number of duplicates in each name.
    ScDPDimensionSaveData* pDimensionData; // settings that create new dimensions
    sal_uInt16 nColumnGrandMode;
    sal_uInt16 nRowGrandMode;
    sal_uInt16 nIgnoreEmptyMode;
    sal_uInt16 nRepeatEmptyMode;
    bool bFilterButton; // not passed to DataPilotSource
    bool bDrillDown; // not passed to DataPilotSource

    /** if true, all dimensions already have all of their member instances
     *  created. */
    bool mbDimensionMembersBuilt;

    std::unique_ptr<OUString> mpGrandTotalName;
    mutable std::unique_ptr<DimOrderType> mpDimOrder; // dimension order for row and column dimensions, to traverse result tree.

public:
    SC_DLLPUBLIC ScDPSaveData();
    ScDPSaveData(const ScDPSaveData& r);
    SC_DLLPUBLIC ~ScDPSaveData();

    ScDPSaveData& operator= ( const ScDPSaveData& r );

    bool operator== ( const ScDPSaveData& r ) const;

    SC_DLLPUBLIC void SetGrandTotalName(const OUString& rName);
    SC_DLLPUBLIC const OUString* GetGrandTotalName() const;

    const DimsType& GetDimensions() const { return m_DimList; }

    /**
     * Get sort order map to sort row and column dimensions in order of
     * appearance. Row dimensions get sorted before column dimensions.  This
     * is used to traverse result tree, which is structured following this
     * order.
     */
    const DimOrderType& GetDimensionSortOrder() const;

    /**
     * Get all dimensions in a given orientation.  The order represents the
     * actual order of occurrence.  The returned list also includes data
     * layout dimension.
     *
     * @param eOrientation orientation
     * @param rDims (out) list of dimensions for specified orientation
     */
    SC_DLLPUBLIC void GetAllDimensionsByOrientation(
        css::sheet::DataPilotFieldOrientation eOrientation,
        std::vector<const ScDPSaveDimension*>& rDims) const;

    void AddDimension(ScDPSaveDimension* pDim);

    /**
     * Get a dimension object by its name.  <i>If one doesn't exist for the
     * given name, it creates a new one.</i>
     *
     * @param rName dimension name
     *
     * @return pointer to the dimension object.  The ScDPSaveData instance
     *         manages its life cycle; hence the caller must
     *         <i>not</i> delete this object.
     */
    SC_DLLPUBLIC ScDPSaveDimension* GetDimensionByName(const OUString& rName);
    SC_DLLPUBLIC ScDPSaveDimension* GetDataLayoutDimension();
    SC_DLLPUBLIC ScDPSaveDimension* GetExistingDataLayoutDimension() const;

    ScDPSaveDimension* DuplicateDimension(const OUString& rName);
    SC_DLLPUBLIC ScDPSaveDimension& DuplicateDimension(const ScDPSaveDimension& rDim);

    SC_DLLPUBLIC ScDPSaveDimension* GetExistingDimensionByName(const OUString& rName) const;
    SC_DLLPUBLIC ScDPSaveDimension* GetNewDimensionByName(const OUString& rName);

    void RemoveDimensionByName(const OUString& rName);

    ScDPSaveDimension* GetInnermostDimension(sal_uInt16 nOrientation);
    ScDPSaveDimension* GetFirstDimension(css::sheet::DataPilotFieldOrientation eOrientation);
    long GetDataDimensionCount() const;

    void SetPosition( ScDPSaveDimension* pDim, long nNew );
    SC_DLLPUBLIC void SetColumnGrand( bool bSet );
    bool GetColumnGrand() const
        { return bool(nColumnGrandMode); }

    SC_DLLPUBLIC void SetRowGrand( bool bSet );
    bool GetRowGrand() const
        { return bool(nRowGrandMode); }

    void SetIgnoreEmptyRows( bool bSet );
    bool GetIgnoreEmptyRows() const
        { return bool(nIgnoreEmptyMode); }

    void SetRepeatIfEmpty( bool bSet );
    bool GetRepeatIfEmpty() const
        { return bool(nRepeatEmptyMode); }

    SC_DLLPUBLIC void SetFilterButton( bool bSet );
    bool GetFilterButton() const
        { return bFilterButton; }

    SC_DLLPUBLIC void SetDrillDown( bool bSet );
    bool GetDrillDown() const
        { return bDrillDown; }

    void WriteToSource( const css::uno::Reference<css::sheet::XDimensionsSupplier>& xSource );
    bool IsEmpty() const;

    const ScDPDimensionSaveData* GetExistingDimensionData() const
        { return pDimensionData; }

    void RemoveAllGroupDimensions( const OUString& rSrcDimName, std::vector<OUString>* pDeletedNames = nullptr );

    SC_DLLPUBLIC ScDPDimensionSaveData* GetDimensionData(); // create if not there
    SC_DLLPUBLIC void SetDimensionData( const ScDPDimensionSaveData* pNew ); // copied
    void BuildAllDimensionMembers(ScDPTableData* pData);
    void SyncAllDimensionMembers(ScDPTableData* pData);

    /**
     * Check whether a dimension has one or more invisible members.
     *
     * @param rDimName dimension name
     */
    SC_DLLPUBLIC bool HasInvisibleMember(const OUString& rDimName) const;

#if DEBUG_PIVOT_TABLE
    void Dump() const;
#endif

private:
    void CheckDuplicateName(ScDPSaveDimension& rDim);
    void RemoveDuplicateNameCount(const OUString& rName);

    /**
     * Append a new original dimension. Not to be called to insert a duplicate
     * dimension.
     *
     * @param rName Dimension name. The name must be the original dimension
     *              name; not a duplicate dimension name.
     * @param bDataLayout true if this is a data layout dimension, false
     *                    otherwise.
     *
     * @return pointer to the new dimension just inserted.
     */
    ScDPSaveDimension* AppendNewDimension(const OUString& rName, bool bDataLayout);

    void DimensionsChanged();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
