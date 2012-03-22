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


#ifdef _MSC_VER
#pragma optimize("",off)
#endif

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/boxitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svx/algitem.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "globstr.hrc"
#include "subtotal.hxx"
#include "rangeutl.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "document.hxx"
#include "userlist.hxx"
#include "pivot.hxx"
#include "rechead.hxx"
#include "formula/errorcodes.hxx"                           // fuer errNoValue
#include "refupdat.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"

using ::com::sun::star::sheet::DataPilotFieldReference;
using ::rtl::OUString;
using ::std::vector;

namespace {

bool equals(const DataPilotFieldReference& left, const DataPilotFieldReference& right)
{
    return (left.ReferenceType     == right.ReferenceType)
        && (left.ReferenceField    == right.ReferenceField)
        && (left.ReferenceItemType == right.ReferenceItemType)
        && (left.ReferenceItemName == right.ReferenceItemName);
}

}

// ============================================================================

ScDPName::ScDPName(const OUString& rName, const OUString& rLayoutName) :
    maName(rName), maLayoutName(rLayoutName) {}

// ============================================================================

ScDPLabelData::Member::Member() :
    mbVisible(true),
    mbShowDetails(true)
{
}

OUString ScDPLabelData::Member::getDisplayName() const
{
    if (!maLayoutName.isEmpty())
        return maLayoutName;

    return maName;
}

ScDPLabelData::ScDPLabelData() :
    mnCol(-1),
    mnOriginalDim(-1),
    mnFuncMask(PIVOT_FUNC_NONE),
    mnUsedHier(0),
    mnFlags(0),
    mbShowAll(false),
    mbIsValue(false),
    mbDataLayout(false)
{
}

ScDPLabelData::ScDPLabelData(const rtl::OUString& rName, SCCOL nCol, bool bIsValue) :
    maName( rName ),
    mnCol( nCol ),
    mnOriginalDim(-1),
    mnFuncMask( PIVOT_FUNC_NONE ),
    mnUsedHier( 0 ),
    mnFlags( 0 ),
    mbShowAll( false ),
    mbIsValue( bIsValue ),
    mbDataLayout(false)
{
}

OUString ScDPLabelData::getDisplayName() const
{
    if (!maLayoutName.isEmpty())
        return maLayoutName;

    return maName;
}

PivotField::PivotField(SCCOL nNewCol, sal_uInt16 nNewFuncMask) :
    nCol( nNewCol ),
    mnOriginalDim(-1),
    nFuncMask( nNewFuncMask ),
    mnDupCount(0)
{
}

PivotField::PivotField( const PivotField& r ) :
    nCol(r.nCol),
    mnOriginalDim(r.mnOriginalDim),
    nFuncMask(r.nFuncMask),
    mnDupCount(r.mnDupCount),
    maFieldRef(r.maFieldRef) {}

long PivotField::getOriginalDim() const
{
    return mnOriginalDim >= 0 ? mnOriginalDim : static_cast<long>(nCol);
}

bool PivotField::operator==( const PivotField& r ) const
{
    return (nCol          == r.nCol)
        && (mnOriginalDim == r.mnOriginalDim)
        && (nFuncMask     == r.nFuncMask)
        && equals(maFieldRef, r.maFieldRef);
}

ScPivotParam::ScPivotParam()
    :   nCol(0), nRow(0), nTab(0),
        bIgnoreEmptyRows(false), bDetectCategories(false),
        bMakeTotalCol(true), bMakeTotalRow(true)
{
}

ScPivotParam::ScPivotParam( const ScPivotParam& r )
    :   nCol( r.nCol ), nRow( r.nRow ), nTab( r.nTab ),
        maPageFields(r.maPageFields),
        maColFields(r.maColFields),
        maRowFields(r.maRowFields),
        maDataFields(r.maDataFields),
        bIgnoreEmptyRows(r.bIgnoreEmptyRows),
        bDetectCategories(r.bDetectCategories),
        bMakeTotalCol(r.bMakeTotalCol),
        bMakeTotalRow(r.bMakeTotalRow)
{
    SetLabelData(r.maLabelArray);
}

ScPivotParam::~ScPivotParam()
{
}

void ScPivotParam::SetLabelData(const ScDPLabelDataVec& r)
{
    ScDPLabelDataVec aNewArray;
    aNewArray.reserve(r.size());
    for (ScDPLabelDataVec::const_iterator itr = r.begin(), itrEnd = r.end();
         itr != itrEnd; ++itr)
        aNewArray.push_back(new ScDPLabelData(*itr));

    maLabelArray.swap(aNewArray);
}

ScPivotParam& ScPivotParam::operator=( const ScPivotParam& r )
{
    nCol              = r.nCol;
    nRow              = r.nRow;
    nTab              = r.nTab;
    bIgnoreEmptyRows  = r.bIgnoreEmptyRows;
    bDetectCategories = r.bDetectCategories;
    bMakeTotalCol     = r.bMakeTotalCol;
    bMakeTotalRow     = r.bMakeTotalRow;

    maPageFields = r.maPageFields;
    maColFields  = r.maColFields;
    maRowFields  = r.maRowFields;
    maDataFields = r.maDataFields;
    SetLabelData(r.maLabelArray);
    return *this;
}

bool ScPivotParam::operator==( const ScPivotParam& r ) const
{
    bool bEqual = (nCol == r.nCol)
                 && (nRow == r.nRow)
                 && (nTab == r.nTab)
                 && (bIgnoreEmptyRows  == r.bIgnoreEmptyRows)
                 && (bDetectCategories == r.bDetectCategories)
                 && (bMakeTotalCol == r.bMakeTotalCol)
                 && (bMakeTotalRow == r.bMakeTotalRow)
                 && (maLabelArray.size() == r.maLabelArray.size())
                 && maPageFields == r.maPageFields
                 && maColFields == r.maColFields
                 && maRowFields == r.maRowFields
                 && maDataFields == r.maDataFields;

    return bEqual;
}

// ============================================================================

ScDPFuncData::ScDPFuncData( SCCOL nCol, sal_uInt16 nFuncMask ) :
    mnCol( nCol ),
    mnOriginalDim(-1),
    mnFuncMask( nFuncMask ),
    mnDupCount(0)
{
}

ScDPFuncData::ScDPFuncData(
    SCCOL nCol, long nOriginalDim, sal_uInt16 nFuncMask, sal_uInt8 nDupCount,
    const DataPilotFieldReference& rFieldRef) :
    mnCol( nCol ),
    mnOriginalDim(nOriginalDim),
    mnFuncMask( nFuncMask ),
    mnDupCount(nDupCount),
    maFieldRef( rFieldRef )
{
}

bool ScDPFuncData::operator== (const ScDPFuncData& r) const
{
    if (mnCol != r.mnCol || mnOriginalDim != r.mnOriginalDim || mnFuncMask != r.mnFuncMask || mnDupCount != r.mnDupCount)
        return false;

    return equals(maFieldRef, r.maFieldRef);
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
