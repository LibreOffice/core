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

#include "AccessibleTableBase.hxx"
#include "miscuno.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "sc.hrc"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <vcl/svapp.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleTableBase::ScAccessibleTableBase(
        const uno::Reference<XAccessible>& rxParent,
        ScDocument* pDoc,
        const ScRange& rRange)
    :
    ScAccessibleContextBase (rxParent, AccessibleRole::TABLE),
    maRange(rRange),
    mpDoc(pDoc)
{
}

ScAccessibleTableBase::~ScAccessibleTableBase()
{
}

void SAL_CALL ScAccessibleTableBase::disposing()
{
    SolarMutexGuard aGuard;
    mpDoc = NULL;

    ScAccessibleContextBase::disposing();
}

    //=====  XInterface  =====================================================

uno::Any SAL_CALL ScAccessibleTableBase::queryInterface( uno::Type const & rType )
    throw (uno::RuntimeException)
{
    uno::Any aAny (ScAccessibleTableBaseImpl::queryInterface(rType));
    return aAny.hasValue() ? aAny : ScAccessibleContextBase::queryInterface(rType);
}

void SAL_CALL ScAccessibleTableBase::acquire()
    throw ()
{
    ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessibleTableBase::release()
    throw ()
{
    ScAccessibleContextBase::release();
}

    //=====  XAccessibleTable  ================================================

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleRowCount(  )
                    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return maRange.aEnd.Row() - maRange.aStart.Row() + 1;
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleColumnCount(  )
                    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return maRange.aEnd.Col() - maRange.aStart.Col() + 1;
}

::rtl::OUString SAL_CALL ScAccessibleTableBase::getAccessibleRowDescription( sal_Int32 nRow )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    OSL_FAIL("Here should be a implementation to fill the description");

    if ((nRow > (maRange.aEnd.Row() - maRange.aStart.Row())) || (nRow < 0))
        throw lang::IndexOutOfBoundsException();

    //setAccessibleRowDescription(nRow, xAccessible); // to remember the created Description
    return rtl::OUString();
}

::rtl::OUString SAL_CALL ScAccessibleTableBase::getAccessibleColumnDescription( sal_Int32 nColumn )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    OSL_FAIL("Here should be a implementation to fill the description");

    if ((nColumn > (maRange.aEnd.Col() - maRange.aStart.Col())) || (nColumn < 0))
        throw lang::IndexOutOfBoundsException();

    //setAccessibleColumnDescription(nColumn, xAccessible); // to remember the created Description
    return rtl::OUString();
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    if ((nColumn > (maRange.aEnd.Col() - maRange.aStart.Col())) || (nColumn < 0) ||
        (nRow > (maRange.aEnd.Row() - maRange.aStart.Row())) || (nRow < 0))
        throw lang::IndexOutOfBoundsException();

    sal_Int32 nCount(1); // the same cell
    nRow += maRange.aStart.Row();
    nColumn += maRange.aStart.Col();

    if (mpDoc)
    {
        SCROW nEndRow(0);
        SCCOL nEndCol(0);
        if (mpDoc->ExtendMerge(static_cast<SCCOL>(nColumn), static_cast<SCROW>(nRow),
            nEndCol, nEndRow, maRange.aStart.Tab()))
        {
            if (nEndRow > nRow)
                nCount = nEndRow - nRow + 1;
        }
    }

    return nCount;
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    if ((nColumn > (maRange.aEnd.Col() - maRange.aStart.Col())) || (nColumn < 0) ||
        (nRow > (maRange.aEnd.Row() - maRange.aStart.Row())) || (nRow < 0))
        throw lang::IndexOutOfBoundsException();

    sal_Int32 nCount(1); // the same cell
    nRow += maRange.aStart.Row();
    nColumn += maRange.aStart.Col();

    if (mpDoc)
    {
        SCROW nEndRow(0);
        SCCOL nEndCol(0);
        if (mpDoc->ExtendMerge(static_cast<SCCOL>(nColumn), static_cast<SCROW>(nRow),
            nEndCol, nEndRow, maRange.aStart.Tab()))
        {
            if (nEndCol > nColumn)
                nCount = nEndCol - nColumn + 1;
        }
    }

    return nCount;
}

uno::Reference< XAccessibleTable > SAL_CALL ScAccessibleTableBase::getAccessibleRowHeaders(  )
                    throw (uno::RuntimeException)
{
    uno::Reference< XAccessibleTable > xAccessibleTable;
    OSL_FAIL("Here should be a implementation to fill the row headers");

    //CommitChange
    return xAccessibleTable;
}

uno::Reference< XAccessibleTable > SAL_CALL ScAccessibleTableBase::getAccessibleColumnHeaders(  )
                    throw (uno::RuntimeException)
{
    uno::Reference< XAccessibleTable > xAccessibleTable;
    OSL_FAIL("Here should be a implementation to fill the column headers");

    //CommitChange
    return xAccessibleTable;
}

uno::Sequence< sal_Int32 > SAL_CALL ScAccessibleTableBase::getSelectedAccessibleRows(  )
                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented yet");
    uno::Sequence< sal_Int32 > aSequence;
    return aSequence;
}

uno::Sequence< sal_Int32 > SAL_CALL ScAccessibleTableBase::getSelectedAccessibleColumns(  )
                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented yet");
    uno::Sequence< sal_Int32 > aSequence;
    return aSequence;
}

sal_Bool SAL_CALL ScAccessibleTableBase::isAccessibleRowSelected( sal_Int32 /* nRow */ )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    OSL_FAIL("not implemented yet");
    return false;
}

sal_Bool SAL_CALL ScAccessibleTableBase::isAccessibleColumnSelected( sal_Int32 /* nColumn */ )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    OSL_FAIL("not implemented yet");
    return false;
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleTableBase::getAccessibleCellAt( sal_Int32 /* nRow */, sal_Int32 /* nColumn */ )
                    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    OSL_FAIL("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleTableBase::getAccessibleCaption(  )
                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleTableBase::getAccessibleSummary(  )
                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

sal_Bool SAL_CALL ScAccessibleTableBase::isAccessibleSelected( sal_Int32 /* nRow */, sal_Int32 /* nColumn */ )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    OSL_FAIL("not implemented yet");
    return false;
}

// =====  XAccessibleExtendedTable  ========================================

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    if (nRow > (maRange.aEnd.Row() - maRange.aStart.Row()) ||
        nRow < 0 ||
        nColumn > (maRange.aEnd.Col() - maRange.aStart.Col()) ||
        nColumn < 0)
        throw lang::IndexOutOfBoundsException();

    nRow -= maRange.aStart.Row();
    nColumn -= maRange.aStart.Col();
    return (nRow * (maRange.aEnd.Col() + 1)) + nColumn;
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleRow( sal_Int32 nChildIndex )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    if (nChildIndex >= getAccessibleChildCount() || nChildIndex < 0)
        throw lang::IndexOutOfBoundsException();

    return nChildIndex / (maRange.aEnd.Col() - maRange.aStart.Col() + 1);
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleColumn( sal_Int32 nChildIndex )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    if (nChildIndex >= getAccessibleChildCount() || nChildIndex < 0)
        throw lang::IndexOutOfBoundsException();

    return nChildIndex % static_cast<sal_Int32>(maRange.aEnd.Col() - maRange.aStart.Col() + 1);
}

// =====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL
    ScAccessibleTableBase::getAccessibleChildCount(void)
                    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    // FIXME: representing rows & columns this way is a plain and simple madness.
    // this needs a radical re-think.
    sal_Int64 nMax = ((sal_Int64)(maRange.aEnd.Row() - maRange.aStart.Row() + 1) *
                      (sal_Int64)(maRange.aEnd.Col() - maRange.aStart.Col() + 1));
    if (nMax > SAL_MAX_INT32)
        nMax = SAL_MAX_INT32;
    if (nMax < 0)
        return 0;
    return static_cast<sal_Int32>(nMax);
}

uno::Reference< XAccessible > SAL_CALL
    ScAccessibleTableBase::getAccessibleChild(sal_Int32 nIndex)
        throw (uno::RuntimeException,
        lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

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

::rtl::OUString SAL_CALL
    ScAccessibleTableBase::createAccessibleDescription(void)
    throw (uno::RuntimeException)
{
    String sDesc(ScResId(STR_ACC_TABLE_DESCR));
    return rtl::OUString(sDesc);
}

::rtl::OUString SAL_CALL
    ScAccessibleTableBase::createAccessibleName(void)
    throw (uno::RuntimeException)
{
    String sName(ScResId(STR_ACC_TABLE_NAME));
    rtl::OUString sCoreName;
    if (mpDoc && mpDoc->GetName( maRange.aStart.Tab(), sCoreName ))
        sName.SearchAndReplaceAscii("%1", sCoreName);
    return rtl::OUString(sName);
}

uno::Reference<XAccessibleRelationSet> SAL_CALL
    ScAccessibleTableBase::getAccessibleRelationSet(void)
    throw (uno::RuntimeException)
{
    OSL_FAIL("should be implemented in the abrevated class");
    return uno::Reference<XAccessibleRelationSet>();
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleTableBase::getAccessibleStateSet(void)
    throw (uno::RuntimeException)
{
    OSL_FAIL("should be implemented in the abrevated class");
    uno::Reference< XAccessibleStateSet > xAccessibleStateSet;
    return xAccessibleStateSet;
}

    ///=====  XAccessibleSelection  ===========================================

void SAL_CALL
        ScAccessibleTableBase::selectAccessibleChild( sal_Int32 /* nChildIndex */ )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
}

sal_Bool SAL_CALL
        ScAccessibleTableBase::isAccessibleChildSelected( sal_Int32 nChildIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    // I don't need to guard, because the called functions have a guard
    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();
    return isAccessibleSelected(getAccessibleRow(nChildIndex), getAccessibleColumn(nChildIndex));
}

void SAL_CALL
        ScAccessibleTableBase::clearAccessibleSelection(  )
        throw (uno::RuntimeException)
{
}

void SAL_CALL
        ScAccessibleTableBase::selectAllAccessibleChildren(  )
        throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL
        ScAccessibleTableBase::getSelectedAccessibleChildCount(  )
        throw (uno::RuntimeException)
{
    sal_Int32 nResult(0);
    return nResult;
}

uno::Reference<XAccessible > SAL_CALL
        ScAccessibleTableBase::getSelectedAccessibleChild( sal_Int32 /* nSelectedChildIndex */ )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    uno::Reference < XAccessible > xAccessible;
    return xAccessible;
}

void SAL_CALL
        ScAccessibleTableBase::deselectAccessibleChild( sal_Int32 /* nSelectedChildIndex */ )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
}

    //=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL ScAccessibleTableBase::getImplementationName(void)
        throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleTableBase"));
}

    //=====  XTypeProvider  ===================================================

uno::Sequence< uno::Type > SAL_CALL ScAccessibleTableBase::getTypes()
        throw (uno::RuntimeException)
{
    return comphelper::concatSequences(ScAccessibleTableBaseImpl::getTypes(), ScAccessibleContextBase::getTypes());
}

namespace
{
    class theScAccessibleTableBaseImplementationId : public rtl::Static< UnoTunnelIdInit, theScAccessibleTableBaseImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleTableBase::getImplementationId(void)
    throw (uno::RuntimeException)
{
    return theScAccessibleTableBaseImplementationId::get().getSeq();
}

void ScAccessibleTableBase::CommitTableModelChange(sal_Int32 nStartRow, sal_Int32 nStartCol, sal_Int32 nEndRow, sal_Int32 nEndCol, sal_uInt16 nId)
{
    AccessibleTableModelChange aModelChange;
    aModelChange.FirstRow = nStartRow;
    aModelChange.FirstColumn = nStartCol;
    aModelChange.LastRow = nEndRow;
    aModelChange.LastColumn = nEndCol;
    aModelChange.Type = nId;

    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::TABLE_MODEL_CHANGED;
    aEvent.Source = uno::Reference< XAccessibleContext >(this);
    aEvent.NewValue <<= aModelChange;

    CommitChange(aEvent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
