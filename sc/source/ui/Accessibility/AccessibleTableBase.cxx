/*************************************************************************
 *
 *  $RCSfile: AccessibleTableBase.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "AccessibleTableBase.hxx"
#ifndef SC_MISCUNO_HXX
#include "miscuno.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_UNOGUARD_HXX
#include "unoguard.hxx"
#endif
#ifndef SC_SCRESID_HXX
#include "scresid.hxx"
#endif
#ifndef SC_SC_HRC
#include "sc.hrc"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETABLEMODELCHANGE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

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
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
    IsObjectValid();
    return maRange.aEnd.Row() - maRange.aStart.Row() + 1;
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleColumnCount(  )
                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    return maRange.aEnd.Col() - maRange.aStart.Col() + 1;
}

::rtl::OUString SAL_CALL ScAccessibleTableBase::getAccessibleRowDescription( sal_Int32 nRow )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    DBG_ERROR("Here should be a implementation to fill the description");

    if ((nRow > (maRange.aEnd.Row() - maRange.aStart.Row())) || (nRow < 0))
        throw lang::IndexOutOfBoundsException();

    //setAccessibleRowDescription(nRow, xAccessible); // to remember the created Description
    return rtl::OUString();
}

::rtl::OUString SAL_CALL ScAccessibleTableBase::getAccessibleColumnDescription( sal_Int32 nColumn )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    DBG_ERROR("Here should be a implementation to fill the description");

    if ((nColumn > (maRange.aEnd.Col() - maRange.aStart.Col())) || (nColumn < 0))
        throw lang::IndexOutOfBoundsException();

    //setAccessibleColumnDescription(nColumn, xAccessible); // to remember the created Description
    return rtl::OUString();
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    ScUnoGuard aGuard;
    IsObjectValid();

    if ((nColumn > (maRange.aEnd.Col() - maRange.aStart.Col())) || (nColumn < 0) ||
        (nRow > (maRange.aEnd.Row() - maRange.aStart.Row())) || (nRow < 0))
        throw lang::IndexOutOfBoundsException();

    sal_Int32 nCount(1); // the same cell
    nRow += maRange.aStart.Row();
    nColumn += maRange.aStart.Col();

    if (mpDoc)
    {
        sal_uInt16 nEndRow(0);
        sal_uInt16 nEndCol(0);
        if (mpDoc->ExtendMerge(static_cast<sal_uInt16>(nColumn), static_cast<sal_uInt16>(nRow),
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
    ScUnoGuard aGuard;
    IsObjectValid();

    if ((nColumn > (maRange.aEnd.Col() - maRange.aStart.Col())) || (nColumn < 0) ||
        (nRow > (maRange.aEnd.Row() - maRange.aStart.Row())) || (nRow < 0))
        throw lang::IndexOutOfBoundsException();

    sal_Int32 nCount(1); // the same cell
    nRow += maRange.aStart.Row();
    nColumn += maRange.aStart.Col();

    if (mpDoc)
    {
        sal_uInt16 nEndRow(0);
        sal_uInt16 nEndCol(0);
        if (mpDoc->ExtendMerge(static_cast<sal_uInt16>(nColumn), static_cast<sal_uInt16>(nRow),
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
    DBG_ERROR("Here should be a implementation to fill the row headers");

    //CommitChange
    return xAccessibleTable;
}

uno::Reference< XAccessibleTable > SAL_CALL ScAccessibleTableBase::getAccessibleColumnHeaders(  )
                    throw (uno::RuntimeException)
{
    uno::Reference< XAccessibleTable > xAccessibleTable;
    DBG_ERROR("Here should be a implementation to fill the column headers");

    //CommitChange
    return xAccessibleTable;
}

uno::Sequence< sal_Int32 > SAL_CALL ScAccessibleTableBase::getSelectedAccessibleRows(  )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    uno::Sequence< sal_Int32 > aSequence;
    return aSequence;
}

uno::Sequence< sal_Int32 > SAL_CALL ScAccessibleTableBase::getSelectedAccessibleColumns(  )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    uno::Sequence< sal_Int32 > aSequence;
    return aSequence;
}

sal_Bool SAL_CALL ScAccessibleTableBase::isAccessibleRowSelected( sal_Int32 nRow )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    DBG_ERROR("not implemented yet");
    return sal_False;
}

sal_Bool SAL_CALL ScAccessibleTableBase::isAccessibleColumnSelected( sal_Int32 nColumn )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    DBG_ERROR("not implemented yet");
    return sal_False;
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleTableBase::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    DBG_ERROR("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleTableBase::getAccessibleCaption(  )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleTableBase::getAccessibleSummary(  )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

sal_Bool SAL_CALL ScAccessibleTableBase::isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    DBG_ERROR("not implemented yet");
    return sal_False;
}

    //=====  XAccessibleExtendedTable  ========================================

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
    IsObjectValid();

    if (nChildIndex >= getAccessibleChildCount() || nChildIndex < 0)
        throw lang::IndexOutOfBoundsException();

    return nChildIndex / (maRange.aEnd.Col() - maRange.aStart.Col() + 1);
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleColumn( sal_Int32 nChildIndex )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    ScUnoGuard aGuard;
    IsObjectValid();

    if (nChildIndex >= getAccessibleChildCount() || nChildIndex < 0)
        throw lang::IndexOutOfBoundsException();

    return nChildIndex % (maRange.aEnd.Col() - maRange.aStart.Col() + 1);
}

    //=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL
    ScAccessibleTableBase::getAccessibleChildCount(void)
                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    return (maRange.aEnd.Row() - maRange.aStart.Row() + 1) *
            (maRange.aEnd.Col() - maRange.aStart.Col() + 1);
//  return 1;
}

uno::Reference< XAccessible > SAL_CALL
    ScAccessibleTableBase::getAccessibleChild(sal_Int32 nIndex)
        throw (uno::RuntimeException,
        lang::IndexOutOfBoundsException)
{
    ScUnoGuard aGuard;
    IsObjectValid();

    if (nIndex >= getAccessibleChildCount() || nIndex < 0)
        throw lang::IndexOutOfBoundsException();

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
/*  String sCoreName;
    if (mpDoc && mpDoc->GetName( maRange.aStart.Tab(), sCoreName ))
        sDesc.SearchAndReplaceAscii("%1", sCoreName);
    sDesc.SearchAndReplaceAscii("%2", String(ScResId(SCSTR_UNKNOWN)));*/
    return rtl::OUString(sDesc);
}

::rtl::OUString SAL_CALL
    ScAccessibleTableBase::createAccessibleName(void)
    throw (uno::RuntimeException)
{
    String sName(ScResId(STR_ACC_TABLE_NAME));
    String sCoreName;
    if (mpDoc && mpDoc->GetName( maRange.aStart.Tab(), sCoreName ))
        sName.SearchAndReplaceAscii("%1", sCoreName);
    return rtl::OUString(sName);
}

uno::Reference<XAccessibleRelationSet> SAL_CALL
    ScAccessibleTableBase::getAccessibleRelationSet(void)
    throw (uno::RuntimeException)
{
    DBG_ERROR("should be implemented in the abrevated class");
    return uno::Reference<XAccessibleRelationSet>();
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleTableBase::getAccessibleStateSet(void)
    throw (uno::RuntimeException)
{
    DBG_ERROR("should be implemented in the abrevated class");
    uno::Reference< XAccessibleStateSet > xAccessibleStateSet;
    return xAccessibleStateSet;
}

    ///=====  XAccessibleSelection  ===========================================

void SAL_CALL
        ScAccessibleTableBase::selectAccessibleChild( sal_Int32 nChildIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
}

sal_Bool SAL_CALL
        ScAccessibleTableBase::isAccessibleChildSelected( sal_Int32 nChildIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    // I don't need to guard, because the called funtions have a guard
//    ScUnoGuard aGuard;
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
        ScAccessibleTableBase::selectAllAccessible(  )
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
        ScAccessibleTableBase::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    uno::Reference < XAccessible > xAccessible;
    return xAccessible;
}

void SAL_CALL
        ScAccessibleTableBase::deselectSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
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

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleTableBase::getImplementationId(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        aId.realloc (16);
        rtl_createUuid (reinterpret_cast<sal_uInt8 *>(aId.getArray()), 0, sal_True);
    }
    return aId;
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
    aEvent.EventId = AccessibleEventId::ACCESSIBLE_TABLE_MODEL_EVENT;
    aEvent.Source = uno::Reference< XAccessible >(this);
    aEvent.NewValue <<= aModelChange;

    CommitChange(aEvent);
}