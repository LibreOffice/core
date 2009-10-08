/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dpsave.hxx,v $
 * $Revision: 1.11.32.3 $
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
#include "scdllapi.h"
#include <hash_map>
#include <list>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldReference;
    struct DataPilotFieldSortInfo;
    struct DataPilotFieldAutoShowInfo;
    struct DataPilotFieldLayoutInfo;
} } } }

class ScDPDimensionSaveData;

// --------------------------------------------------------------------
//
//  classes to save Data Pilot settings
//


class ScDPSaveMember
{
private:
    String      aName;
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

    void                    WriteToSource( const com::sun::star::uno::Reference<
                                            com::sun::star::uno::XInterface>& xMember,
                                            sal_Int32 nPosition );
};


class ScDPSaveDimension
{
private:
    String      aName;
    String*     pLayoutName;        // alternative name for layout, not used (yet)
    String*     pSelectedPage;
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

    SC_DLLPUBLIC void                   SetOrientation(USHORT nNew);
    void                    SetSubTotals(BOOL bSet);        // to be removed!
    SC_DLLPUBLIC void                   SetSubTotals(long nCount, const USHORT* pFuncs);
    long                    GetSubTotalsCount() const { return nSubTotalCount; }
    USHORT                  GetSubTotalFunc(long nIndex) const { return pSubTotalFuncs[nIndex]; }
    SC_DLLPUBLIC void                   SetShowEmpty(BOOL bSet);
    BOOL                    GetShowEmpty() const { return BOOL(nShowEmptyMode); }
    SC_DLLPUBLIC void                   SetFunction(USHORT nNew);       // enum GeneralFunction
    USHORT                  GetFunction() const { return nFunction; }
    void                    SetUsedHierarchy(long nNew);
    long                    GetUsedHierarchy() const { return nUsedHierarchy; }
    SC_DLLPUBLIC void                   SetLayoutName(const String* pName);
    SC_DLLPUBLIC const String&          GetLayoutName() const;
    SC_DLLPUBLIC BOOL                   HasLayoutName() const;

    const ::com::sun::star::sheet::DataPilotFieldReference* GetReferenceValue() const   { return pReferenceValue; }
    SC_DLLPUBLIC void                   SetReferenceValue(const ::com::sun::star::sheet::DataPilotFieldReference* pNew);

    const ::com::sun::star::sheet::DataPilotFieldSortInfo* GetSortInfo() const          { return pSortInfo; }
    SC_DLLPUBLIC void                    SetSortInfo(const ::com::sun::star::sheet::DataPilotFieldSortInfo* pNew);
    const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo* GetAutoShowInfo() const  { return pAutoShowInfo; }
    SC_DLLPUBLIC void                    SetAutoShowInfo(const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo* pNew);
    const ::com::sun::star::sheet::DataPilotFieldLayoutInfo* GetLayoutInfo() const      { return pLayoutInfo; }
    SC_DLLPUBLIC void                    SetLayoutInfo(const ::com::sun::star::sheet::DataPilotFieldLayoutInfo* pNew);

    SC_DLLPUBLIC void                   SetCurrentPage( const String* pPage );      // NULL = no selection (all)
    SC_DLLPUBLIC BOOL                   HasCurrentPage() const;
    SC_DLLPUBLIC const String&          GetCurrentPage() const;

    USHORT                  GetOrientation() const  { return nOrientation; }

    ScDPSaveMember*         GetExistingMemberByName(const String& rName);
    SC_DLLPUBLIC ScDPSaveMember*            GetMemberByName(const String& rName);

    void                    SetMemberPosition( const String& rName, sal_Int32 nNewPos );

    void                    WriteToSource( const com::sun::star::uno::Reference<
                                            com::sun::star::uno::XInterface>& xDim );
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

public:
    SC_DLLPUBLIC                        ScDPSaveData();
                            ScDPSaveData(const ScDPSaveData& r);
    SC_DLLPUBLIC                        ~ScDPSaveData();

    ScDPSaveData&           operator= ( const ScDPSaveData& r );

    BOOL                    operator== ( const ScDPSaveData& r ) const;

    const List&             GetDimensions() const { return aDimList; }
    void                    AddDimension(ScDPSaveDimension* pDim) { aDimList.Insert(pDim, LIST_APPEND); }

    ScDPSaveDimension*      GetDimensionByName(const String& rName);
    SC_DLLPUBLIC ScDPSaveDimension*     GetDataLayoutDimension();

    ScDPSaveDimension*      DuplicateDimension(const String& rName);
    SC_DLLPUBLIC ScDPSaveDimension&      DuplicateDimension(const ScDPSaveDimension& rDim);

    SC_DLLPUBLIC ScDPSaveDimension*     GetExistingDimensionByName(const String& rName);
    SC_DLLPUBLIC ScDPSaveDimension*     GetNewDimensionByName(const String& rName);

    void                    RemoveDimensionByName(const String& rName);

    ScDPSaveDimension*      GetInnermostDimension(USHORT nOrientation);
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

    BOOL                    IsEmpty() const;

    const ScDPDimensionSaveData* GetExistingDimensionData() const   { return pDimensionData; }
    SC_DLLPUBLIC ScDPDimensionSaveData*  GetDimensionData();     // create if not there
    void                    SetDimensionData( const ScDPDimensionSaveData* pNew );      // copied
};


#endif

