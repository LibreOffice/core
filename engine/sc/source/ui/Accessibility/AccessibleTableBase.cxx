/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <AccessibleTableBase.hxx>
#include <document.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <table.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::cpo;

ScAccessibleTableBase::ScAccessibleTableBase(const uno::Reference<XAccessible>& rxParent,
                                             ScDocument* pDoc, const ScRange& rRange)
    : ImplInheritanceHelper(rxParent, AccessibleRole::TABLE)
    , maRange(rRange)
    , mpDoc(pDoc)
{
}

ScAccessibleTableBase::~ScAccessibleTableBase()
{
}

void ScAccessibleTableBase::disposing()
{
    SolarMutexGuard aGuard;
    mpDoc = nullptr;

    ScAccessibleContextBase::disposing();
}

    //=====  XAccessibleTable  ================================================

sal_Int32 ScAccessibleTableBase::getAccessibleRowCount(  )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return maRange.aEnd.Row() - maRange.aStart.Row() + 1;
}

sal_Int32 ScAccessibleTableBase::getAccessibleColumnCount(  )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return maRange.aEnd.Col() - maRange.aStart.Col() + 1;
}

OUString ScAccessibleTableBase::getAccessibleRowDescription( sal_Int32 nRow )
{
    OSL_FAIL("Here should be an implementation to fill the description");

    if ((nRow > (maRange.aEnd.Row() - maRange.aStart.Row())) || (nRow < 0))
        throw lang::IndexOutOfBoundsException();

    //setAccessibleRowDescription(nRow, xAccessible); // to remember the created Description
    return OUString();
}

OUString ScAccessibleTableBase::getAccessibleColumnDescription( sal_Int32 nColumn )
{
    OSL_FAIL("Here should be an implementation to fill the description");

    if ((nColumn > (maRange.aEnd.Col() - maRange.aStart.Col())) || (nColumn < 0))
        throw lang::IndexOutOfBoundsException();

    //setAccessibleColumnDescription(nColumn, xAccessible); // to remember the created Description
    return OUString();
}

sal_Int32 ScAccessibleTableBase::getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMutexGuard aGuard;
    ensureAlive();

    if ((nColumn > (maRange.aEnd.Col() - maRange.aStart.Col())) || (nColumn < 0) ||
        (nRow > (maRange.aEnd.Row() - maRange.aStart.Row())) || (nRow < 0))
        throw lang::IndexOutOfBoundsException();

    sal_Int32 nCount(1); // the same cell
    nRow += maRange.aStart.Row();
    nColumn += maRange.aStart.Col();

    if (mpDoc)
    {
        ScTable* pTab = mpDoc->FetchTable(maRange.aStart.Tab());
        if (pTab)
        {
            SCROW nStartRow = static_cast<SCROW>(nRow);
            SCROW nEndRow   = nStartRow;
            SCCOL nStartCol = static_cast<SCCOL>(nColumn);
            SCCOL nEndCol   = nStartCol;
            if (pTab->ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, false))
            {
                if (nEndRow > nStartRow)
                    nCount = nEndRow - nStartRow + 1;
            }
        }
    }

    return nCount;
}

sal_Int32 ScAccessibleTableBase::getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMutexGuard aGuard;
    ensureAlive();

    if ((nColumn > (maRange.aEnd.Col() - maRange.aStart.Col())) || (nColumn < 0) ||
        (nRow > (maRange.aEnd.Row() - maRange.aStart.Row())) || (nRow < 0))
        throw lang::IndexOutOfBoundsException();

    sal_Int32 nCount(1); // the same cell
    nRow += maRange.aStart.Row();
    nColumn += maRange.aStart.Col();

    if (mpDoc)
    {
        ScTable* pTab = mpDoc->FetchTable(maRange.aStart.Tab());
        if (pTab)
        {
            SCROW nStartRow = static_cast<SCROW>(nRow);
            SCROW nEndRow   = nStartRow;
            SCCOL nStartCol = static_cast<SCCOL>(nColumn);
            SCCOL nEndCol   = nStartCol;
            if (pTab->ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, false))
            {
                if (nEndCol > nStartCol)
                    nCount = nEndCol - nStartCol + 1;
            }
        }
    }

    return nCount;
}

uno::Reference< XAccessibleTable > ScAccessibleTableBase::getAccessibleRowHeaders(  )
{
    uno::Reference< XAccessibleTable > xAccessibleTable;
    OSL_FAIL("Here should be an implementation to fill the row headers");

    //CommitChange
    return xAccessibleTable;
}

uno::Reference< XAccessibleTable > ScAccessibleTableBase::getAccessibleColumnHeaders(  )
{
    uno::Reference< XAccessibleTable > xAccessibleTable;
    OSL_FAIL("Here should be an implementation to fill the column headers");

    //CommitChange
    return xAccessibleTable;
}

cpo::uno::Sequence< sal_Int32 > ScAccessibleTableBase::getSelectedAccessibleRows(  )
{
    OSL_FAIL("not implemented yet");
    cpo::uno::Sequence< sal_Int32 > aSequence;
    return aSequence;
}

cpo::uno::Sequence< sal_Int32 > ScAccessibleTableBase::getSelectedAccessibleColumns(  )
{
    OSL_FAIL("not implemented yet");
    cpo::uno::Sequence< sal_Int32 > aSequence;
    return aSequence;
}

bool ScAccessibleTableBase::isAccessibleRowSelected( sal_Int32 /* nRow */ )
{
    OSL_FAIL("not implemented yet");
    return false;
}

bool ScAccessibleTableBase::isAccessibleColumnSelected( sal_Int32 /* nColumn */ )
{
    OSL_FAIL("not implemented yet");
    return false;
}

uno::Reference< XAccessible > ScAccessibleTableBase::getAccessibleCellAt( sal_Int32 /* nRow */, sal_Int32 /* nColumn */ )
{
    OSL_FAIL("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

uno::Reference< XAccessible > ScAccessibleTableBase::getAccessibleCaption(  )
{
    OSL_FAIL("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

uno::Reference< XAccessible > ScAccessibleTableBase::getAccessibleSummary(  )
{
    OSL_FAIL("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

bool ScAccessibleTableBase::isAccessibleSelected( sal_Int32 /* nRow */, sal_Int32 /* nColumn */ )
{
    OSL_FAIL("not implemented yet");
    return false;
}

// =====  XAccessibleExtendedTable  ========================================

sal_Int64 ScAccessibleTableBase::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMutexGuard aGuard;
    ensureAlive();

    if (nRow > (maRange.aEnd.Row() - maRange.aStart.Row()) ||
        nRow < 0 ||
        nColumn > (maRange.aEnd.Col() - maRange.aStart.Col()) ||
        nColumn < 0)
        throw lang::IndexOutOfBoundsException();

    nRow -= maRange.aStart.Row();
    nColumn -= maRange.aStart.Col();
    return (static_cast<sal_Int64>(nRow) * static_cast<sal_Int64>(maRange.aEnd.Col() + 1)) + nColumn;
}

sal_Int32 ScAccessibleTableBase::getAccessibleRow( sal_Int64 nChildIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();

    if (nChildIndex >= getAccessibleChildCount() || nChildIndex < 0)
        throw lang::IndexOutOfBoundsException();

    return nChildIndex / (maRange.aEnd.Col() - maRange.aStart.Col() + 1);
}

sal_Int32 ScAccessibleTableBase::getAccessibleColumn( sal_Int64 nChildIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();

    if (nChildIndex >= getAccessibleChildCount() || nChildIndex < 0)
        throw lang::IndexOutOfBoundsException();

    return nChildIndex % static_cast<sal_Int32>(maRange.aEnd.Col() - maRange.aStart.Col() + 1);
}

// =====  XAccessibleContext  ==============================================

sal_Int64 ScAccessibleTableBase::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;
    ensureAlive();

    // FIXME: representing rows & columns this way is a plain and simple madness.
    // this needs a radical re-think.
    sal_Int64 nMax = static_cast<sal_Int64>(maRange.aEnd.Row() - maRange.aStart.Row() + 1) *
                     static_cast<sal_Int64>(maRange.aEnd.Col() - maRange.aStart.Col() + 1);
    if (nMax < 0)
        return 0;
    return nMax;
}

uno::Reference< XAccessible >
    ScAccessibleTableBase::getAccessibleChild(sal_Int64 nIndex)
{
    SolarMutexGuard aGuard;
    ensureAlive();

    if (nIndex >= getAccessibleChildCount() || nIndex < 0)
        throw lang::IndexOutOfBoundsException();

    // FIXME: representing rows & columns this way is a plain and simple madness.
    // this needs a radical re-think.

    sal_Int32 nRow(0);
    sal_Int32 nColumn(0);
    sal_Int32 nTemp(maRange.aEnd.Col() - maRange.aStart.Col() + 1);
    nRow = nIndex / nTemp;
    nColumn = nIndex % nTemp;
    return getAccessibleCellAt(nRow, nColumn);
}

OUString
    ScAccessibleTableBase::createAccessibleDescription()
{
    return STR_ACC_TABLE_DESCR;
}

OUString ScAccessibleTableBase::createAccessibleName()
{
    OUString sName(ScResId(STR_ACC_TABLE_NAME));
    OUString sCoreName;
    if (mpDoc && mpDoc->GetName( maRange.aStart.Tab(), sCoreName ))
        sName = sName.replaceFirst("%1", sCoreName);
    return sName;
}

uno::Reference<XAccessibleRelationSet>
    ScAccessibleTableBase::getAccessibleRelationSet()
{
    OSL_FAIL("should be implemented in the abbreviated class");
    return uno::Reference<XAccessibleRelationSet>();
}

sal_Int64 ScAccessibleTableBase::getAccessibleStateSet()
{
    OSL_FAIL("should be implemented in the abbreviated class");
    return 0;
}

    ///=====  XAccessibleSelection  ===========================================

void ScAccessibleTableBase::selectAccessibleChild( sal_Int64 /* nChildIndex */ )
{
}

bool
        ScAccessibleTableBase::isAccessibleChildSelected( sal_Int64 nChildIndex )
{
    // I don't need to guard, because the called functions have a guard
    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();
    return isAccessibleSelected(getAccessibleRow(nChildIndex), getAccessibleColumn(nChildIndex));
}

void
        ScAccessibleTableBase::clearAccessibleSelection(  )
{
}

void ScAccessibleTableBase::selectAllAccessibleChildren()
{
}

sal_Int64
        ScAccessibleTableBase::getSelectedAccessibleChildCount(  )
{
    return 0;
}

uno::Reference<XAccessible >
        ScAccessibleTableBase::getSelectedAccessibleChild( sal_Int64 /* nSelectedChildIndex */ )
{
    uno::Reference < XAccessible > xAccessible;
    return xAccessible;
}

void ScAccessibleTableBase::deselectAccessibleChild( sal_Int64 /* nSelectedChildIndex */ )
{
}

void ScAccessibleTableBase::CommitTableModelChange(sal_Int32 nStartRow, sal_Int32 nStartCol, sal_Int32 nEndRow, sal_Int32 nEndCol, sal_uInt16 nId)
{
    AccessibleTableModelChange aModelChange;
    aModelChange.FirstRow = nStartRow;
    aModelChange.FirstColumn = nStartCol;
    aModelChange.LastRow = nEndRow;
    aModelChange.LastColumn = nEndCol;
    aModelChange.Type = nId;

    CommitChange(AccessibleEventId::TABLE_MODEL_CHANGED, cpo::uno::Any(), cpo::uno::Any(aModelChange));
}

bool ScAccessibleTableBase::selectRow( sal_Int32 )
{
    return true;
}

bool ScAccessibleTableBase::selectColumn( sal_Int32 )
{
    return true;
}

bool ScAccessibleTableBase::unselectRow( sal_Int32 )
{
        return true;
}

bool ScAccessibleTableBase::unselectColumn( sal_Int32 )
{
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
