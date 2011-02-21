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

#ifndef SC_DPSAVE_HXX
#define SC_DPSAVE_HXX

#include <list>
#include <memory>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_map.hpp>

#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <tools/string.hxx>

#include "scdllapi.h"

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldReference;
    struct DataPilotFieldSortInfo;
    struct DataPilotFieldAutoShowInfo;
    struct DataPilotFieldLayoutInfo;
} } } }

class ScDPDimensionSaveData;
class ScDPTableData;

// --------------------------------------------------------------------
//
// classes to save Data Pilot settings
//


class ScDPSaveMember
{
private:
    ::rtl::OUString aName;
    ::std::auto_ptr<rtl::OUString> mpLayoutName; // custom name to be displayed in the table.
    sal_uInt16 nVisibleMode;
    sal_uInt16 nShowDetailsMode;

public:
    ScDPSaveMember(const ::rtl::OUString& rName);
    ScDPSaveMember(const ScDPSaveMember& r);
    ~ScDPSaveMember();

    bool operator== ( const ScDPSaveMember& r ) const;

    const ::rtl::OUString& GetName() const
        { return aName; }

    bool HasIsVisible() const;
    SC_DLLPUBLIC void SetIsVisible(bool bSet);
    bool GetIsVisible() const
        { return bool(nVisibleMode); }

    bool HasShowDetails() const;
    SC_DLLPUBLIC void SetShowDetails(bool bSet);
    bool GetShowDetails() const
        { return bool(nShowDetailsMode); }

    void SetName( const ::rtl::OUString& rNew ); // used if the source member was renamed (groups)

    SC_DLLPUBLIC void SetLayoutName( const ::rtl::OUString& rName );
    SC_DLLPUBLIC const ::rtl::OUString*  GetLayoutName() const;
    void RemoveLayoutName();

    void WriteToSource( const com::sun::star::uno::Reference<com::sun::star::uno::XInterface>& xMember,
                            sal_Int32 nPosition );
};


bool operator == (const ::com::sun::star::sheet::DataPilotFieldSortInfo &l, const ::com::sun::star::sheet::DataPilotFieldSortInfo &r );
bool operator == (const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo &l, const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo &r );
bool operator == (const ::com::sun::star::sheet::DataPilotFieldReference &l, const ::com::sun::star::sheet::DataPilotFieldReference &r );

class SC_DLLPUBLIC ScDPSaveDimension
{
private:
    ::rtl::OUString aName;
    ::rtl::OUString* pSelectedPage;
    ::std::auto_ptr<rtl::OUString> mpLayoutName;
    ::std::auto_ptr<rtl::OUString> mpSubtotalName;
    bool bIsDataLayout;
    bool bDupFlag;
    sal_uInt16 nOrientation;
    sal_uInt16 nFunction; // enum GeneralFunction, for data dimensions
    long nUsedHierarchy;
    sal_uInt16 nShowEmptyMode; //! at level
    bool bSubTotalDefault; //! at level
    long nSubTotalCount;
    sal_uInt16* pSubTotalFuncs; // enum GeneralFunction
    ::com::sun::star::sheet::DataPilotFieldReference* pReferenceValue;
    ::com::sun::star::sheet::DataPilotFieldSortInfo* pSortInfo; // (level)
    ::com::sun::star::sheet::DataPilotFieldAutoShowInfo* pAutoShowInfo; // (level)
    ::com::sun::star::sheet::DataPilotFieldLayoutInfo* pLayoutInfo; // (level)

public:
    typedef boost::unordered_map <rtl::OUString, ScDPSaveMember*, rtl::OUStringHash> MemberHash;
    typedef std::list <ScDPSaveMember*> MemberList;

private:
    MemberHash maMemberHash;
    MemberList maMemberList;

public:
    ScDPSaveDimension(const ::rtl::OUString& rName, bool bDataLayout);
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

    const ::rtl::OUString& GetName() const
        { return aName; }

    bool IsDataLayout() const
        { return bIsDataLayout; }

    void SetName( const ::rtl::OUString& rNew ); // used if the source dim was renamed (groups)

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

    void SetFunction(sal_uInt16 nNew); // enum GeneralFunction
    sal_uInt16 GetFunction() const
        { return nFunction; }

    void SetUsedHierarchy(long nNew);
    long GetUsedHierarchy() const
        { return nUsedHierarchy; }

    void SetLayoutName(const ::rtl::OUString& rName);
    const ::rtl::OUString* GetLayoutName() const;
    void RemoveLayoutName();
    void SetSubtotalName(const ::rtl::OUString& rName);
    const ::rtl::OUString* GetSubtotalName() const;

    bool IsMemberNameInUse(const ::rtl::OUString& rName) const;

    const ::com::sun::star::sheet::DataPilotFieldReference* GetReferenceValue() const
        { return pReferenceValue; }

    void SetReferenceValue(const ::com::sun::star::sheet::DataPilotFieldReference* pNew);

    const ::com::sun::star::sheet::DataPilotFieldSortInfo* GetSortInfo() const
        { return pSortInfo; }

    void SetSortInfo(const ::com::sun::star::sheet::DataPilotFieldSortInfo* pNew);
    const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo* GetAutoShowInfo() const
        { return pAutoShowInfo; }

    void SetAutoShowInfo(const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo* pNew);
    const ::com::sun::star::sheet::DataPilotFieldLayoutInfo* GetLayoutInfo() const
        { return pLayoutInfo; }

    void SetLayoutInfo(const ::com::sun::star::sheet::DataPilotFieldLayoutInfo* pNew);

    void SetCurrentPage( const ::rtl::OUString* pPage ); // NULL = no selection (all)
    bool HasCurrentPage() const;
    const ::rtl::OUString& GetCurrentPage() const;

    sal_uInt16 GetOrientation() const
        { return nOrientation; }

    ScDPSaveMember* GetExistingMemberByName(const ::rtl::OUString& rName);

    /**
     * Get a member object by its name.  If one doesn't exist, creat a new
     * object and return it.  This class manages the life cycle of all member
     * objects belonging to it, so <i>don't delete the returned instance.</i>
     *
     * @param rName member name
     *
     * @return pointer to the member object.
     */
    ScDPSaveMember* GetMemberByName(const ::rtl::OUString& rName);

    void SetMemberPosition( const ::rtl::OUString& rName, sal_Int32 nNewPos );

    void WriteToSource( const com::sun::star::uno::Reference<com::sun::star::uno::XInterface>& xDim );
    void Refresh( const com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier>& xSource ,
                      const std::list<rtl::OUString> & deletedDims);

    void UpdateMemberVisibility(const ::boost::unordered_map< ::rtl::OUString, bool, ::rtl::OUStringHash>& rData);

    bool HasInvisibleMember() const;
};


class ScDPSaveData
{
private:
    boost::ptr_vector<ScDPSaveDimension> aDimList;
    ScDPDimensionSaveData* pDimensionData; // settings that create new dimensions
    sal_uInt16 nColumnGrandMode;
    sal_uInt16 nRowGrandMode;
    sal_uInt16 nIgnoreEmptyMode;
    sal_uInt16 nRepeatEmptyMode;
    bool bFilterButton; // not passed to DataPilotSource
    bool bDrillDown; // not passed to DataPilotSource
    long mnCacheId;

    /** if true, all dimensions already have all of their member instances
     *  created. */
    bool mbDimensionMembersBuilt;

    ::std::auto_ptr<rtl::OUString> mpGrandTotalName;

public:
    SC_DLLPUBLIC ScDPSaveData();
    ScDPSaveData(const ScDPSaveData& r);
    SC_DLLPUBLIC ~ScDPSaveData();

    ScDPSaveData& operator= ( const ScDPSaveData& r );

    bool operator== ( const ScDPSaveData& r ) const;

    SC_DLLPUBLIC void SetGrandTotalName(const ::rtl::OUString& rName);
    SC_DLLPUBLIC const ::rtl::OUString* GetGrandTotalName() const;

    const boost::ptr_vector<ScDPSaveDimension>& GetDimensions() const
        { return aDimList; }

    void AddDimension(ScDPSaveDimension* pDim)
        { aDimList.push_back(pDim); }

    /**
     * Get a dimension object by its name.  <i>If one doesn't exist for the
     * given name, it creats a new one.</i>
     *
     * @param rName dimension name
     *
     * @return pointer to the dimension object.  The ScDPSaveData instance
     *         manages its life cycle; hence the caller must
     *         <i>not</i> delete this object.
     */
    ScDPSaveDimension* GetDimensionByName(const ::rtl::OUString& rName);
    SC_DLLPUBLIC ScDPSaveDimension* GetDataLayoutDimension();
    SC_DLLPUBLIC ScDPSaveDimension* GetExistingDataLayoutDimension() const;

    ScDPSaveDimension* DuplicateDimension(const ::rtl::OUString& rName);
    SC_DLLPUBLIC ScDPSaveDimension& DuplicateDimension(const ScDPSaveDimension& rDim);

    SC_DLLPUBLIC ScDPSaveDimension* GetExistingDimensionByName(const ::rtl::OUString& rName) const;
    SC_DLLPUBLIC ScDPSaveDimension* GetNewDimensionByName(const ::rtl::OUString& rName);

    void RemoveDimensionByName(const ::rtl::OUString& rName);

    ScDPSaveDimension* GetInnermostDimension(sal_uInt16 nOrientation);
    ScDPSaveDimension* GetFirstDimension(::com::sun::star::sheet::DataPilotFieldOrientation eOrientation);
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

    void WriteToSource( const com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier>& xSource );
    void Refresh( const com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier>& xSource );
    bool IsEmpty() const;
    inline long GetCacheId() const
        { return mnCacheId; }

    inline void SetCacheId( long nCacheId )
        { mnCacheId = nCacheId; }

    const ScDPDimensionSaveData* GetExistingDimensionData() const
        { return pDimensionData; }

    SC_DLLPUBLIC ScDPDimensionSaveData* GetDimensionData(); // create if not there
    void SetDimensionData( const ScDPDimensionSaveData* pNew ); // copied
    void BuildAllDimensionMembers(ScDPTableData* pData);

    /**
     * Check whether a dimension has one or more invisible members.
     *
     * @param rDimName dimension name
     */
    SC_DLLPUBLIC bool HasInvisibleMember(const ::rtl::OUString& rDimName) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
