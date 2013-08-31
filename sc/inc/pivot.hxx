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

/*
    IMPORTANT:
    Strictly adhere to the following sequence when creating a pivot table:

    pPivot->SetColFields(aColArr, aColCount)
    pPivot->SetRowFields(aRowArr, aRowCount)
    pPivot->SetDataFields(aDataArr, aDataCount)
    if (pPivot->CreateData())
    {
        pPivotDrawData();
        pPivotReleaseData();
    }

    Make sure that either ColArr or RowArr contains a PivotDataField entry.
*/

#ifndef SC_PIVOT_HXX
#define SC_PIVOT_HXX

#include "global.hxx"
#include "address.hxx"
#include "dpglobal.hxx"
#include "calcmacros.hxx"

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

#define PIVOT_DATA_FIELD        (MAXCOLCOUNT)
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>

struct ScDPName
{
    OUString     maName;         ///< Original name of the dimension.
    OUString     maLayoutName;   ///< Layout name (display name)

    explicit ScDPName(const OUString& rName, const OUString& rLayoutName);
};

struct ScDPLabelData
{
    OUString       maName;              ///< Original name of the dimension.
    OUString       maLayoutName;        ///< Layout name (display name)
    OUString       maSubtotalName;
    SCCOL               mnCol;          ///< 0-based field index (not the source column index)
    long                mnOriginalDim;  ///< original dimension index (>= 0 for duplicated dimension)
    sal_uInt16          mnFuncMask;     ///< Page/Column/Row subtotal function.
    sal_Int32           mnUsedHier;     ///< Used hierarchy.
    sal_Int32           mnFlags;        ///< Flags from the DataPilotSource dimension
    bool                mbShowAll:1;    ///< true = Show all (also empty) results.
    bool                mbIsValue:1;    ///< true = Sum or count in data field.
    bool                mbDataLayout:1;

    struct Member
    {
        OUString maName;
        OUString maLayoutName;
        bool mbVisible;
        bool mbShowDetails;

        Member();

        /**
         * @return the name that should be displayed in the dp dialogs i.e.
         * when the layout name is present, use it, or else use the original
         * name.
         */
        OUString SC_DLLPUBLIC getDisplayName() const;
    };
    ::std::vector<Member>                               maMembers;
    ::com::sun::star::uno::Sequence< OUString >  maHiers;               ///< Hierarchies.
    ::com::sun::star::sheet::DataPilotFieldSortInfo     maSortInfo;     ///< Sorting info.
    ::com::sun::star::sheet::DataPilotFieldLayoutInfo   maLayoutInfo;   ///< Layout info.
    ::com::sun::star::sheet::DataPilotFieldAutoShowInfo maShowInfo;     ///< AutoShow info.

    ScDPLabelData();

    /**
     * @return the name that should be displayed in the dp dialogs i.e. when
     * the layout name is present, use it, or else use the original name.
     */
    OUString SC_DLLPUBLIC getDisplayName() const;
};

typedef boost::ptr_vector<ScDPLabelData> ScDPLabelDataVector;

struct ScPivotField
{
    SCCOL               nCol;          ///< 0-based dimension index (not source column index)
    long                mnOriginalDim; ///< >= 0 for duplicated field.
    sal_uInt16          nFuncMask;
    sal_uInt8           mnDupCount;
    ::com::sun::star::sheet::DataPilotFieldReference maFieldRef;

    explicit ScPivotField( SCCOL nNewCol = 0, sal_uInt16 nNewFuncMask = PIVOT_FUNC_NONE );
    ScPivotField( const ScPivotField& r );

    long getOriginalDim() const;
    bool                operator==( const ScPivotField& r ) const;
};

typedef ::std::vector< ScPivotField > ScPivotFieldVector;

struct ScPivotParam
{
    SCCOL           nCol;           ///< Cursor Position /
    SCROW           nRow;           ///< or start of destination area
    SCTAB           nTab;
    ScDPLabelDataVector maLabelArray;
    ScPivotFieldVector maPageFields;
    ScPivotFieldVector maColFields;
    ScPivotFieldVector maRowFields;
    ScPivotFieldVector maDataFields;
    bool            bIgnoreEmptyRows;
    bool            bDetectCategories;
    bool            bMakeTotalCol;
    bool            bMakeTotalRow;

    ScPivotParam();
    ScPivotParam( const ScPivotParam& r );
    ~ScPivotParam();

    ScPivotParam&   operator=       ( const ScPivotParam& r );
    bool            operator==      ( const ScPivotParam& r ) const;
    void SetLabelData(const ScDPLabelDataVector& r);
};

struct ScPivotFuncData
{
    SCCOL               mnCol;
    long                mnOriginalDim;
    sal_uInt16          mnFuncMask;
    sal_uInt8           mnDupCount;
    ::com::sun::star::sheet::DataPilotFieldReference maFieldRef;

    explicit ScPivotFuncData( SCCOL nCol, sal_uInt16 nFuncMask );
    explicit ScPivotFuncData(
        SCCOL nCol, long nOriginalDim, sal_uInt16 nFuncMask, sal_uInt8 nDupCount,
        const ::com::sun::star::sheet::DataPilotFieldReference& rFieldRef );

    bool operator== (const ScPivotFuncData& r) const;

#if DEBUG_PIVOT_TABLE
    void Dump() const;
#endif
};

typedef ::std::vector< ScPivotFuncData > ScPivotFuncDataVector;
typedef std::vector<ScDPName> ScDPNameVec;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
