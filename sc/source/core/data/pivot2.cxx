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
#include <formula/errorcodes.hxx>
#include "refupdat.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include <o3tl/make_unique.hxx>

#if DEBUG_PIVOT_TABLE
using std::cout;
using std::endl;
#endif

using css::sheet::DataPilotFieldReference;
using std::vector;

namespace
{

bool equals(const DataPilotFieldReference& left, const DataPilotFieldReference& right)
{
    return (left.ReferenceType     == right.ReferenceType)
        && (left.ReferenceField    == right.ReferenceField)
        && (left.ReferenceItemType == right.ReferenceItemType)
        && (left.ReferenceItemName == right.ReferenceItemName);
}

} // namespace

// ScDPName

ScDPName::ScDPName() : mnDupCount(0)
{}

ScDPName::ScDPName(const OUString& rName, const OUString& rLayoutName, sal_uInt8 nDupCount) :
    maName(rName), maLayoutName(rLayoutName), mnDupCount(nDupCount)
{}

// ScDPLabelData

ScDPLabelData::Member::Member() :
    mbVisible(true),
    mbShowDetails(true)
{}

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
    mnDupCount(0),
    mbShowAll(false),
    mbIsValue(false),
    mbDataLayout(false),
    mbRepeatItemLabels(false)
{}

OUString ScDPLabelData::getDisplayName() const
{
    if (!maLayoutName.isEmpty())
        return maLayoutName;

    return maName;
}

// ScPivotField

ScPivotField::ScPivotField(SCCOL nNewCol, sal_uInt16 nNewFuncMask) :
    nCol(nNewCol),
    mnOriginalDim(-1),
    nFuncMask(nNewFuncMask),
    mnDupCount(0)
{}

ScPivotField::ScPivotField( const ScPivotField& rPivotField ) :
    nCol(rPivotField.nCol),
    mnOriginalDim(rPivotField.mnOriginalDim),
    nFuncMask(rPivotField.nFuncMask),
    mnDupCount(rPivotField.mnDupCount),
    maFieldRef(rPivotField.maFieldRef)
{}

long ScPivotField::getOriginalDim() const
{
    return mnOriginalDim >= 0 ? mnOriginalDim : static_cast<long>(nCol);
}

bool ScPivotField::operator==( const ScPivotField& r ) const
{
    return (nCol          == r.nCol)
        && (mnOriginalDim == r.mnOriginalDim)
        && (nFuncMask     == r.nFuncMask)
        && equals(maFieldRef, r.maFieldRef);
}

// ScPivotParam

ScPivotParam::ScPivotParam() :
    nCol(0), nRow(0), nTab(0),
    bIgnoreEmptyRows(false), bDetectCategories(false),
    bMakeTotalCol(true), bMakeTotalRow(true)
{}

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
{}

void ScPivotParam::SetLabelData(const ScDPLabelDataVector& rVector)
{
    ScDPLabelDataVector aNewArray;
    aNewArray.reserve(rVector.size());
    ScDPLabelDataVector::const_iterator it;
    for (it = rVector.begin(); it != rVector.end(); ++it)
    {
        aNewArray.push_back(o3tl::make_unique<ScDPLabelData>(*it->get()));
    }
    maLabelArray.swap(aNewArray);
}

ScPivotParam& ScPivotParam::operator=( const ScPivotParam& rPivotParam )
{
    nCol              = rPivotParam.nCol;
    nRow              = rPivotParam.nRow;
    nTab              = rPivotParam.nTab;
    bIgnoreEmptyRows  = rPivotParam.bIgnoreEmptyRows;
    bDetectCategories = rPivotParam.bDetectCategories;
    bMakeTotalCol     = rPivotParam.bMakeTotalCol;
    bMakeTotalRow     = rPivotParam.bMakeTotalRow;

    maPageFields = rPivotParam.maPageFields;
    maColFields  = rPivotParam.maColFields;
    maRowFields  = rPivotParam.maRowFields;
    maDataFields = rPivotParam.maDataFields;

    SetLabelData(rPivotParam.maLabelArray);
    return *this;
}

bool ScPivotParam::operator==( const ScPivotParam& rPivotParam ) const
{
    bool bEqual = (nCol == rPivotParam.nCol &&
                  nRow == rPivotParam.nRow &&
                  nTab == rPivotParam.nTab &&
                  bIgnoreEmptyRows  == rPivotParam.bIgnoreEmptyRows &&
                  bDetectCategories == rPivotParam.bDetectCategories &&
                  bMakeTotalCol == rPivotParam.bMakeTotalCol &&
                  bMakeTotalRow == rPivotParam.bMakeTotalRow &&
                  maLabelArray.size() == rPivotParam.maLabelArray.size() &&
                  maPageFields == rPivotParam.maPageFields &&
                  maColFields == rPivotParam.maColFields &&
                  maRowFields == rPivotParam.maRowFields &&
                  maDataFields == rPivotParam.maDataFields);
    return bEqual;
}

// ScPivotFuncData

ScPivotFuncData::ScPivotFuncData( SCCOL nCol, sal_uInt16 nFuncMask ) :
    mnCol( nCol ),
    mnOriginalDim(-1),
    mnFuncMask(nFuncMask),
    mnDupCount(0)
{}

bool ScPivotFuncData::operator== (const ScPivotFuncData& rFuncData) const
{
    if (mnCol != rFuncData.mnCol ||
        mnOriginalDim != rFuncData.mnOriginalDim ||
        mnFuncMask != rFuncData.mnFuncMask ||
        mnDupCount != rFuncData.mnDupCount)
    {
        return false;
    }
    return equals(maFieldRef, rFuncData.maFieldRef);
}

#if DEBUG_PIVOT_TABLE
void ScPivotFuncData::Dump() const
{
    cout << "ScPivotFuncData: (col=" << mnCol << ", original dim=" << mnOriginalDim
        << ", func mask=" << mnFuncMask << ", duplicate count=" << static_cast<int>(mnDupCount)
        << ")" << endl;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
