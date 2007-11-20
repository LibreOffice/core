/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dpsave.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 17:40:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_DPSAVE_HXX
#define SC_DPSAVE_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _LIST_HXX //autogen wg. List
#include <tools/list.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XDIMENSIONSSUPPLIER_HPP_
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#endif

#ifndef INCLUDED_SCDLLAPI_H
#include "scdllapi.h"
#endif
#include <hash_map>
#include <list>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldReference;
    struct DataPilotFieldSortInfo;
    struct DataPilotFieldAutoShowInfo;
    struct DataPilotFieldLayoutInfo;
} } } }

class SvStream;
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
                            ScDPSaveMember(SvStream& rStream);
                            ~ScDPSaveMember();

    BOOL                    operator== ( const ScDPSaveMember& r ) const;

    const String&           GetName() const { return aName; }
    BOOL                    HasIsVisible() const;
    void                    SetIsVisible(BOOL bSet);
    BOOL                    GetIsVisible() const { return BOOL(nVisibleMode); }
    BOOL                    HasShowDetails() const;
    void                    SetShowDetails(BOOL bSet);
    BOOL                    GetShowDetails() const { return BOOL(nShowDetailsMode); }

    void                    SetName( const String& rNew );  // used if the source member was renamed (groups)

    void                    WriteToSource( const com::sun::star::uno::Reference<
                                            com::sun::star::uno::XInterface>& xMember,
                                            sal_Int32 nPosition );

    void                    Store( SvStream& rStream ) const;
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
                            ScDPSaveDimension(SvStream& rStream);
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
    void                    SetSubTotals(BOOL bSet);        // to be removed!
    void                    SetSubTotals(long nCount, const USHORT* pFuncs);
    long                    GetSubTotalsCount() const { return nSubTotalCount; }
    USHORT                  GetSubTotalFunc(long nIndex) const { return pSubTotalFuncs[nIndex]; }
    void                    SetShowEmpty(BOOL bSet);
    BOOL                    GetShowEmpty() const { return BOOL(nShowEmptyMode); }
    void                    SetFunction(USHORT nNew);       // enum GeneralFunction
    USHORT                  GetFunction() const { return nFunction; }
    void                    SetUsedHierarchy(long nNew);
    long                    GetUsedHierarchy() const { return nUsedHierarchy; }
    void                    SetLayoutName(const String* pName);
    const String&           GetLayoutName() const;
    BOOL                    HasLayoutName() const;
    void                    ResetLayoutName();

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

    void                    Store( SvStream& rStream ) const;
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
                            ScDPSaveData();
                            ScDPSaveData(const ScDPSaveData& r);
                            ~ScDPSaveData();

    ScDPSaveData&           operator= ( const ScDPSaveData& r );

    BOOL                    operator== ( const ScDPSaveData& r ) const;

    const List&             GetDimensions() const { return aDimList; }
    void                    AddDimension(ScDPSaveDimension* pDim) { aDimList.Insert(pDim, LIST_APPEND); }

    ScDPSaveDimension*      GetDimensionByName(const String& rName);
    ScDPSaveDimension*      GetDataLayoutDimension();

    ScDPSaveDimension*      DuplicateDimension(const String& rName);
    ScDPSaveDimension&      DuplicateDimension(const ScDPSaveDimension& rDim);

    SC_DLLPUBLIC ScDPSaveDimension*     GetExistingDimensionByName(const String& rName);
    ScDPSaveDimension*      GetNewDimensionByName(const String& rName);

    void                    RemoveDimensionByName(const String& rName);

    ScDPSaveDimension*      GetInnermostDimension(USHORT nOrientation);
    long                    GetDataDimensionCount() const;


    void                    SetPosition( ScDPSaveDimension* pDim, long nNew );
    void                    SetColumnGrand( BOOL bSet );
    BOOL                    GetColumnGrand() const { return BOOL(nColumnGrandMode); }
    void                    SetRowGrand( BOOL bSet );
    BOOL                    GetRowGrand() const { return BOOL(nRowGrandMode); }
    void                    SetIgnoreEmptyRows( BOOL bSet );
    BOOL                    GetIgnoreEmptyRows() const { return BOOL(nIgnoreEmptyMode); }
    void                    SetRepeatIfEmpty( BOOL bSet );
    BOOL                    GetRepeatIfEmpty() const { return BOOL(nRepeatEmptyMode); }

    void                    SetFilterButton( BOOL bSet );
    BOOL                    GetFilterButton() const { return bFilterButton; }
    void                    SetDrillDown( BOOL bSet );
    BOOL                    GetDrillDown() const { return bDrillDown; }

    void                    WriteToSource( const com::sun::star::uno::Reference<
                                            com::sun::star::sheet::XDimensionsSupplier>& xSource );

    void                    Store( SvStream& rStream ) const;
    void                    Load( SvStream& rStream );

    BOOL                    IsEmpty() const;

    const ScDPDimensionSaveData* GetExistingDimensionData() const   { return pDimensionData; }
    ScDPDimensionSaveData*  GetDimensionData();     // create if not there
    void                    SetDimensionData( const ScDPDimensionSaveData* pNew );      // copied
};


#endif

