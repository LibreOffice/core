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

#include <tools/string.hxx>
#include <tools/list.hxx>
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include "scdllapi.h"
#include <hash_map>
#include <list>
#include <memory>

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
//  classes to save Data Pilot settings
//


class ScDPSaveMember
{
private:
    String      aName;
    ::std::auto_ptr<rtl::OUString> mpLayoutName; // custom name to be displayed in the table.
    USHORT      nVisibleMode;
    USHORT      nShowDetailsMode;

public:
                            ScDPSaveMember(const String& rName);
                            ScDPSaveMember(const ScDPSaveMember& r);
                            ~ScDPSaveMember();

    BOOL                    operator== ( const ScDPSaveMember& r ) const;

    const String&           GetName() const { return aName; }
    BOOL                    HasIsVisible() const;
    SC_DLLPUBLIC void                   SetIsVisible(BOOL bSet);
    BOOL                    GetIsVisible() const { return BOOL(nVisibleMode); }
    BOOL                    HasShowDetails() const;
    SC_DLLPUBLIC void                   SetShowDetails(BOOL bSet);
    BOOL                    GetShowDetails() const { return BOOL(nShowDetailsMode); }

    void                    SetName( const String& rNew );  // used if the source member was renamed (groups)

    SC_DLLPUBLIC void                    SetLayoutName( const ::rtl::OUString& rName );
    SC_DLLPUBLIC const ::rtl::OUString*  GetLayoutName() const;
    void                    RemoveLayoutName();

    void                    WriteToSource( const com::sun::star::uno::Reference<
                                            com::sun::star::uno::XInterface>& xMember,
                                            sal_Int32 nPosition );
};


bool operator == (const ::com::sun::star::sheet::DataPilotFieldSortInfo &l, const ::com::sun::star::sheet::DataPilotFieldSortInfo &r );
bool operator == (const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo &l, const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo &r );
bool operator == (const ::com::sun::star::sheet::DataPilotFieldReference &l, const ::com::sun::star::sheet::DataPilotFieldReference &r );
class SC_DLLPUBLIC ScDPSaveDimension
{
private:
    String      aName;
    String*     pSelectedPage;
    ::std::auto_ptr<rtl::OUString> mpLayoutName;
    ::std::auto_ptr<rtl::OUString> mpSubtotalName;
    BOOL        bIsDataLayout;
    BOOL        bDupFlag;
    USHORT      nOrientation;
    USHORT      nFunction;          // enum GeneralFunction, for data dimensions
    long        nUsedHierarchy;
    USHORT      nShowEmptyMode;     //! at level
    BOOL        bSubTotalDefault;   //! at level
    long        nSubTotalCount;
    USHORT*     pSubTotalFuncs;     // enum GeneralFunction
    ::com::sun::star::sheet::DataPilotFieldReference* pReferenceValue;
    ::com::sun::star::sheet::DataPilotFieldSortInfo*  pSortInfo;            // (level)
    ::com::sun::star::sheet::DataPilotFieldAutoShowInfo* pAutoShowInfo;     // (level)
    ::com::sun::star::sheet::DataPilotFieldLayoutInfo* pLayoutInfo;         // (level)

public:
    typedef std::hash_map <String, ScDPSaveMember*, rtl::OUStringHash> MemberHash;
    typedef std::list <ScDPSaveMember*>                                MemberList;
private:
    MemberHash maMemberHash;
    MemberList maMemberList;
public:
                            ScDPSaveDimension(const String& rName, BOOL bDataLayout);
                            ScDPSaveDimension(const ScDPSaveDimension& r);
                            ~ScDPSaveDimension();

    BOOL                    operator== ( const ScDPSaveDimension& r ) const;

    const MemberList&       GetMembers() const { return maMemberList; }
    void                    AddMember(ScDPSaveMember* pMember);

    void                    SetDupFlag(BOOL bSet)   { bDupFlag = bSet; }
    BOOL                    GetDupFlag() const      { return bDupFlag; }

    const String&           GetName() const         { return aName; }
    BOOL                    IsDataLayout() const    { return bIsDataLayout; }

    void                    SetName( const String& rNew );  // used if the source dim was renamed (groups)

    void                    SetOrientation(USHORT nNew);
    void                    SetSubTotals(long nCount, const USHORT* pFuncs);
    long                    GetSubTotalsCount() const { return nSubTotalCount; }
    USHORT                  GetSubTotalFunc(long nIndex) const { return pSubTotalFuncs[nIndex]; }
    bool                    HasShowEmpty() const;
    void                    SetShowEmpty(BOOL bSet);
    BOOL                    GetShowEmpty() const { return BOOL(nShowEmptyMode); }
    void                    SetFunction(USHORT nNew);       // enum GeneralFunction
    USHORT                  GetFunction() const { return nFunction; }
    void                    SetUsedHierarchy(long nNew);
    long                    GetUsedHierarchy() const { return nUsedHierarchy; }

    void                    SetLayoutName(const ::rtl::OUString& rName);
    const ::rtl::OUString*  GetLayoutName() const;
    void                    RemoveLayoutName();
    void                    SetSubtotalName(const ::rtl::OUString& rName);
    const ::rtl::OUString*  GetSubtotalName() const;

    bool                    IsMemberNameInUse(const ::rtl::OUString& rName) const;

    const ::com::sun::star::sheet::DataPilotFieldReference* GetReferenceValue() const   { return pReferenceValue; }
    void                    SetReferenceValue(const ::com::sun::star::sheet::DataPilotFieldReference* pNew);

    const ::com::sun::star::sheet::DataPilotFieldSortInfo* GetSortInfo() const          { return pSortInfo; }
    void                    SetSortInfo(const ::com::sun::star::sheet::DataPilotFieldSortInfo* pNew);
    const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo* GetAutoShowInfo() const  { return pAutoShowInfo; }
    void                    SetAutoShowInfo(const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo* pNew);
    const ::com::sun::star::sheet::DataPilotFieldLayoutInfo* GetLayoutInfo() const      { return pLayoutInfo; }
    void                    SetLayoutInfo(const ::com::sun::star::sheet::DataPilotFieldLayoutInfo* pNew);

    void                    SetCurrentPage( const String* pPage );      // NULL = no selection (all)
    BOOL                    HasCurrentPage() const;
    const String&           GetCurrentPage() const;

    USHORT                  GetOrientation() const  { return nOrientation; }

    ScDPSaveMember*         GetExistingMemberByName(const String& rName);
    ScDPSaveMember*         GetMemberByName(const String& rName);

    void                    SetMemberPosition( const String& rName, sal_Int32 nNewPos );

    void                    WriteToSource( const com::sun::star::uno::Reference<
                                            com::sun::star::uno::XInterface>& xDim );
    void                    Refresh( const com::sun::star::uno::Reference<
                                    com::sun::star::sheet::XDimensionsSupplier>& xSource ,
                                    const   std::list<String> & deletedDims);

    void                    UpdateMemberVisibility(const ::std::hash_map< ::rtl::OUString, bool, ::rtl::OUStringHash>& rData);

    bool                    HasInvisibleMember() const;
};


class ScDPSaveData
{
private:
    List        aDimList;
    ScDPDimensionSaveData* pDimensionData;      // settings that create new dimensions
    USHORT      nColumnGrandMode;
    USHORT      nRowGrandMode;
    USHORT      nIgnoreEmptyMode;
    USHORT      nRepeatEmptyMode;
    BOOL        bFilterButton;      // not passed to DataPilotSource
    BOOL        bDrillDown;         // not passed to DataPilotSource
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    long      mnCacheId;
    // End Comments

    /** if true, all dimensions already have all of their member instances
     *  created. */
    bool        mbDimensionMembersBuilt;

    ::std::auto_ptr<rtl::OUString> mpGrandTotalName;

public:
    SC_DLLPUBLIC                        ScDPSaveData();
                            ScDPSaveData(const ScDPSaveData& r);
    SC_DLLPUBLIC                        ~ScDPSaveData();

    ScDPSaveData&           operator= ( const ScDPSaveData& r );

    BOOL                    operator== ( const ScDPSaveData& r ) const;

    SC_DLLPUBLIC void                    SetGrandTotalName(const ::rtl::OUString& rName);
    SC_DLLPUBLIC const ::rtl::OUString*  GetGrandTotalName() const;

    const List&             GetDimensions() const { return aDimList; }
    void                    AddDimension(ScDPSaveDimension* pDim) { aDimList.Insert(pDim, LIST_APPEND); }

    ScDPSaveDimension*      GetDimensionByName(const String& rName);
    SC_DLLPUBLIC ScDPSaveDimension*     GetDataLayoutDimension();
    SC_DLLPUBLIC ScDPSaveDimension*      GetExistingDataLayoutDimension() const;

    ScDPSaveDimension*      DuplicateDimension(const String& rName);
    SC_DLLPUBLIC ScDPSaveDimension&      DuplicateDimension(const ScDPSaveDimension& rDim);

    SC_DLLPUBLIC ScDPSaveDimension*     GetExistingDimensionByName(const String& rName) const;
    SC_DLLPUBLIC ScDPSaveDimension*     GetNewDimensionByName(const String& rName);

    void                    RemoveDimensionByName(const String& rName);

    ScDPSaveDimension*      GetInnermostDimension(USHORT nOrientation);
    ScDPSaveDimension*      GetFirstDimension(::com::sun::star::sheet::DataPilotFieldOrientation eOrientation);
    long                    GetDataDimensionCount() const;


    void                    SetPosition( ScDPSaveDimension* pDim, long nNew );
    SC_DLLPUBLIC void                   SetColumnGrand( BOOL bSet );
    BOOL                    GetColumnGrand() const { return BOOL(nColumnGrandMode); }
    SC_DLLPUBLIC void                   SetRowGrand( BOOL bSet );
    BOOL                    GetRowGrand() const { return BOOL(nRowGrandMode); }
    void                    SetIgnoreEmptyRows( BOOL bSet );
    BOOL                    GetIgnoreEmptyRows() const { return BOOL(nIgnoreEmptyMode); }
    void                    SetRepeatIfEmpty( BOOL bSet );
    BOOL                    GetRepeatIfEmpty() const { return BOOL(nRepeatEmptyMode); }

    SC_DLLPUBLIC void                    SetFilterButton( BOOL bSet );
    BOOL                    GetFilterButton() const { return bFilterButton; }
    SC_DLLPUBLIC void                    SetDrillDown( BOOL bSet );
    BOOL                    GetDrillDown() const { return bDrillDown; }

    void                    WriteToSource( const com::sun::star::uno::Reference<
        com::sun::star::sheet::XDimensionsSupplier>& xSource );
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    void                    Refresh( const com::sun::star::uno::Reference<
                                            com::sun::star::sheet::XDimensionsSupplier>& xSource );
    BOOL                    IsEmpty() const;
    inline long GetCacheId() const{ return mnCacheId; }
    inline void SetCacheId( long nCacheId ){ mnCacheId = nCacheId; }
    // End Comments
    const ScDPDimensionSaveData* GetExistingDimensionData() const   { return pDimensionData; }
    SC_DLLPUBLIC ScDPDimensionSaveData*  GetDimensionData();     // create if not there
    void                    SetDimensionData( const ScDPDimensionSaveData* pNew );      // copied
    void                    BuildAllDimensionMembers(ScDPTableData* pData);

    /**
     * Check whether a dimension has one or more invisible members.
     *
     * @param rDimName dimension name
     */
    SC_DLLPUBLIC bool       HasInvisibleMember(const ::rtl::OUString& rDimName) const;
};


#endif

