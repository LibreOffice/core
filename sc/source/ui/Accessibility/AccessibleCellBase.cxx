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

#include "AccessibleCellBase.hxx"
#include "attrib.hxx"
#include "scitems.hxx"
#include "miscuno.hxx"
#include "document.hxx"
#include "docfunc.hxx"
#include "docsh.hxx"
#include "formulacell.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "unonames.hxx"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <editeng/brushitem.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>

#include <float.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleCellBase::ScAccessibleCellBase(
        const uno::Reference<XAccessible>& rxParent,
        ScDocument* pDoc,
        const ScAddress& rCellAddress,
        sal_Int32 nIndex)
    :
    ScAccessibleContextBase(rxParent, AccessibleRole::TABLE_CELL),
    maCellAddress(rCellAddress),
    mpDoc(pDoc),
    mnIndex(nIndex)
{
}

ScAccessibleCellBase::~ScAccessibleCellBase()
{
}

    //=====  XAccessibleComponent  ============================================

sal_Bool SAL_CALL ScAccessibleCellBase::isVisible(  )
        throw (uno::RuntimeException)
{
     SolarMutexGuard aGuard;
    IsObjectValid();
    // test whether the cell is hidden (column/row - hidden/filtered)
    sal_Bool bVisible(sal_True);
    if (mpDoc)
    {
        bool bColHidden = mpDoc->ColHidden(maCellAddress.Col(), maCellAddress.Tab());
        bool bRowHidden = mpDoc->RowHidden(maCellAddress.Row(), maCellAddress.Tab());
        bool bColFiltered = mpDoc->ColFiltered(maCellAddress.Col(), maCellAddress.Tab());
        bool bRowFiltered = mpDoc->RowFiltered(maCellAddress.Row(), maCellAddress.Tab());

        if (bColHidden || bColFiltered || bRowHidden || bRowFiltered)
            bVisible = false;
    }
    return bVisible;
}

sal_Int32 SAL_CALL ScAccessibleCellBase::getForeground()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);
    if (mpDoc)
    {
        SfxObjectShell* pObjSh = mpDoc->GetDocumentShell();
        if ( pObjSh )
        {
            uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( pObjSh->GetModel(), uno::UNO_QUERY );
            if ( xSpreadDoc.is() )
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
                uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                if ( xIndex.is() )
                {
                    uno::Any aTable = xIndex->getByIndex(maCellAddress.Tab());
                    uno::Reference<sheet::XSpreadsheet> xTable;
                    if (aTable>>=xTable)
                    {
                        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(maCellAddress.Col(), maCellAddress.Row());
                        if (xCell.is())
                        {
                            uno::Reference<beans::XPropertySet> xCellProps(xCell, uno::UNO_QUERY);
                            if (xCellProps.is())
                            {
                                uno::Any aAny = xCellProps->getPropertyValue(OUString(SC_UNONAME_CCOLOR));
                                aAny >>= nColor;
                            }
                        }
                    }
                }
            }
        }
    }
    return nColor;
}

sal_Int32 SAL_CALL ScAccessibleCellBase::getBackground()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);

    if (mpDoc)
    {
        SfxObjectShell* pObjSh = mpDoc->GetDocumentShell();
        if ( pObjSh )
        {
            uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( pObjSh->GetModel(), uno::UNO_QUERY );
            if ( xSpreadDoc.is() )
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
                uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                if ( xIndex.is() )
                {
                    uno::Any aTable = xIndex->getByIndex(maCellAddress.Tab());
                    uno::Reference<sheet::XSpreadsheet> xTable;
                    if (aTable>>=xTable)
                    {
                        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(maCellAddress.Col(), maCellAddress.Row());
                        if (xCell.is())
                        {
                            uno::Reference<beans::XPropertySet> xCellProps(xCell, uno::UNO_QUERY);
                            if (xCellProps.is())
                            {
                                uno::Any aAny = xCellProps->getPropertyValue(OUString(SC_UNONAME_CELLBACK));
                                aAny >>= nColor;
                            }
                        }
                    }
                }
            }
        }
    }

    return nColor;
}

    //=====  XInterface  =====================================================

uno::Any SAL_CALL ScAccessibleCellBase::queryInterface( uno::Type const & rType )
    throw (uno::RuntimeException)
{
    uno::Any aAny (ScAccessibleCellBaseImpl::queryInterface(rType));
    return aAny.hasValue() ? aAny : ScAccessibleContextBase::queryInterface(rType);
}

void SAL_CALL ScAccessibleCellBase::acquire()
    throw ()
{
    ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessibleCellBase::release()
    throw ()
{
    ScAccessibleContextBase::release();
}

    //=====  XAccessibleContext  ==============================================

sal_Int32
    ScAccessibleCellBase::getAccessibleIndexInParent(void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return mnIndex;
}

OUString SAL_CALL
    ScAccessibleCellBase::createAccessibleDescription(void)
    throw (uno::RuntimeException)
{
    OUString sDescription = String(ScResId(STR_ACC_CELL_DESCR));

    return sDescription;
}

OUString SAL_CALL
    ScAccessibleCellBase::createAccessibleName(void)
    throw (uno::RuntimeException)
{
    String sName( ScResId(STR_ACC_CELL_NAME) );
    // Document not needed, because only the cell address, but not the tablename is needed
    // always us OOO notation
    OUString sAddress(maCellAddress.Format(SCA_VALID, NULL));
    sName.SearchAndReplaceAscii("%1", sAddress);
    /*  #i65103# ZoomText merges cell address and contents, e.g. if value 2 is
        contained in cell A1, ZT reads "cell A twelve" instead of "cell A1 - 2".
        Simple solution: Append a space character to the cell address. */
    sName.Append( ' ' );
    return OUString(sName);
}

    //=====  XAccessibleValue  ================================================

uno::Any SAL_CALL
    ScAccessibleCellBase::getCurrentValue(  )
    throw (uno::RuntimeException)
{
     SolarMutexGuard aGuard;
    IsObjectValid();
    uno::Any aAny;
    if (mpDoc)
        aAny <<= mpDoc->GetValue(maCellAddress);

    return aAny;
}

sal_Bool SAL_CALL
    ScAccessibleCellBase::setCurrentValue( const uno::Any& aNumber )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    double fValue = 0;
    bool bResult = false;
    if((aNumber >>= fValue) && mpDoc && mpDoc->GetDocumentShell())
    {
        uno::Reference<XAccessibleStateSet> xParentStates;
        if (getAccessibleParent().is())
        {
            uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
            xParentStates = xParentContext->getAccessibleStateSet();
        }
        if (IsEditable(xParentStates))
        {
            ScDocShell* pDocShell = (ScDocShell*) mpDoc->GetDocumentShell();
            bResult = pDocShell->GetDocFunc().SetValueCell(maCellAddress, fValue, false);
        }
    }
    return bResult;
}

uno::Any SAL_CALL
    ScAccessibleCellBase::getMaximumValue(  )
    throw (uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= DBL_MAX;

    return aAny;
}

uno::Any SAL_CALL
    ScAccessibleCellBase::getMinimumValue(  )
    throw (uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= -DBL_MAX;

    return aAny;
}

    //=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessibleCellBase::getImplementationName(void)
        throw (uno::RuntimeException)
{
    return OUString("ScAccessibleCellBase");
}

    //=====  XTypeProvider  ===================================================

uno::Sequence< uno::Type > SAL_CALL ScAccessibleCellBase::getTypes()
        throw (uno::RuntimeException)
{
    return comphelper::concatSequences(ScAccessibleCellBaseImpl::getTypes(), ScAccessibleContextBase::getTypes());
}

namespace
{
    class theScAccessibleCellBaseImplementationId : public rtl::Static< UnoTunnelIdInit, theScAccessibleCellBaseImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleCellBase::getImplementationId(void)
    throw (uno::RuntimeException)
{
    return theScAccessibleCellBaseImplementationId::get().getSeq();
}

sal_Bool ScAccessibleCellBase::IsEditable(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    sal_Bool bEditable(false);
    if (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::EDITABLE))
        bEditable = sal_True;
    return bEditable;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
