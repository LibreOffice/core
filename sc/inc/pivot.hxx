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
/*
    WICHTIG:
    Folgende Reihenfolge beim Aufbau der Pivot-Tabelle unbedingt einzuhalten:

    pPivot->SetColFields(aColArr, aColCount)
    pPivot->SetRowFields(aRowArr, aRowCount)
    pPivot->SetDataFields(aDataArr, aDataCount)
    if (pPivot->CreateData())
    {
        pPivotDrawData();
        pPivotReleaseData();
    }

    ausserdem ist sicherzustellen, dass entweder das ColArr oder das RowArr
    einen PivotDataField Eintrag enthalten

*/


#ifndef SC_PIVOT_HXX
#define SC_PIVOT_HXX

#include "global.hxx"
#include "address.hxx"
#include "dpglobal.hxx"

#include <vector>
#include <boost/shared_ptr.hpp>

class SubTotal;
#include "collect.hxx"

#define PIVOT_DATA_FIELD        (MAXCOLCOUNT)
#define PIVOT_FUNC_REF          (MAXCOLCOUNT)
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>

class SvStream;
class ScDocument;
class ScUserListData;
class ScProgress;
struct ScDPLabelData;

typedef ::boost::shared_ptr<ScDPLabelData> ScDPLabelDataRef;

// -----------------------------------------------------------------------

struct PivotField
{
    SCsCOL              nCol;
    USHORT              nFuncMask;
    USHORT              nFuncCount;
    ::com::sun::star::sheet::DataPilotFieldReference maFieldRef;

    explicit            PivotField( SCsCOL nNewCol = 0, USHORT nNewFuncMask = PIVOT_FUNC_NONE );
    PivotField( const PivotField& r );

    bool                operator==( const PivotField& r ) const;
};

// -----------------------------------------------------------------------

// implementation still in global2.cxx
struct ScPivotParam
{
    SCCOL           nCol;           // Cursor Position /
    SCROW           nRow;           // bzw. Anfang des Zielbereiches
    SCTAB           nTab;
    ::std::vector<ScDPLabelDataRef> maLabelArray;
    ::std::vector<PivotField> maPageFields;
    ::std::vector<PivotField> maColFields;
    ::std::vector<PivotField> maRowFields;
    ::std::vector<PivotField> maDataFields;
    BOOL            bIgnoreEmptyRows;
    BOOL            bDetectCategories;
    BOOL            bMakeTotalCol;
    BOOL            bMakeTotalRow;

    ScPivotParam();
    ScPivotParam( const ScPivotParam& r );
    ~ScPivotParam();

    ScPivotParam&   operator=       ( const ScPivotParam& r );
    BOOL            operator==      ( const ScPivotParam& r ) const;
    void            ClearPivotArrays();
    void            SetLabelData    (const ::std::vector<ScDPLabelDataRef>& r);
};

// -----------------------------------------------------------------------

typedef PivotField          PivotFieldArr[PIVOT_MAXFIELD];
typedef PivotField          PivotPageFieldArr[PIVOT_MAXPAGEFIELD];

//------------------------------------------------------------------------

struct ScDPName
{
    ::rtl::OUString     maName;         /// Original name of the dimension.
    ::rtl::OUString     maLayoutName;   /// Layout name (display name)

    explicit ScDPName(const ::rtl::OUString& rName, const ::rtl::OUString& rLayoutName);
};

// ============================================================================

struct ScDPLabelData
{
    ::rtl::OUString     maName;         /// Original name of the dimension.
    ::rtl::OUString     maLayoutName;   /// Layout name (display name)
    SCsCOL              mnCol;
    USHORT              mnFuncMask;     /// Page/Column/Row subtotal function.
    sal_Int32           mnUsedHier;     /// Used hierarchy.
    sal_Int32           mnFlags;        /// Flags from the DataPilotSource dimension
    bool                mbShowAll;      /// true = Show all (also empty) results.
    bool                mbIsValue;      /// true = Sum or count in data field.

    struct Member
    {
        ::rtl::OUString maName;
        ::rtl::OUString maLayoutName;
        bool mbVisible;
        bool mbShowDetails;

        Member();

        /**
         * return the name that should be displayed in the dp dialogs i.e.
         * when the layout name is present, use it, or else use the original
         * name.
         */
        ::rtl::OUString SC_DLLPUBLIC getDisplayName() const;
    };
    ::std::vector<Member>                               maMembers;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  maHiers;        /// Hierarchies.
    ::com::sun::star::sheet::DataPilotFieldSortInfo     maSortInfo;     /// Sorting info.
    ::com::sun::star::sheet::DataPilotFieldLayoutInfo   maLayoutInfo;   /// Layout info.
    ::com::sun::star::sheet::DataPilotFieldAutoShowInfo maShowInfo;     /// AutoShow info.

    explicit            ScDPLabelData( const String& rName, short nCol, bool bIsValue );

    /**
     * return the name that should be displayed in the dp dialogs i.e. when
     * the layout name is present, use it, or else use the original name.
     */
    ::rtl::OUString SC_DLLPUBLIC getDisplayName() const;
};

// ============================================================================

struct ScDPFuncData
{
    short               mnCol;
    USHORT              mnFuncMask;
    ::com::sun::star::sheet::DataPilotFieldReference maFieldRef;

    explicit            ScDPFuncData( short nNewCol, USHORT nNewFuncMask );
    explicit            ScDPFuncData( short nNewCol, USHORT nNewFuncMask,
                            const ::com::sun::star::sheet::DataPilotFieldReference& rFieldRef );
};

// ============================================================================

typedef std::vector< ScDPLabelData > ScDPLabelDataVec;
typedef std::vector<ScDPName> ScDPNameVec;

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
