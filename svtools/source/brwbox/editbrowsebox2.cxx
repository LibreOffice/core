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

#include <svtools/editbrowsebox.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <comphelper/types.hxx>
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/accessibility/AccessibleBrowseBoxCheckBoxCell.hxx>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>

namespace svt
{
    using namespace com::sun::star::accessibility;
    using namespace com::sun::star::uno;
    using namespace ::com::sun::star::accessibility::AccessibleEventId;


Reference< XAccessible > EditBrowseBox::CreateAccessibleCheckBoxCell(sal_Int32 _nRow, sal_uInt16 _nColumnPos,const TriState& eState)
{
    Reference< XAccessible > xAccessible( GetAccessible() );
    if ( !xAccessible.is() )
        return nullptr;

    Reference< XAccessibleContext > xAccContext = xAccessible->getAccessibleContext();
    if ( !xAccContext.is() )
        return nullptr;

    return new AccessibleCheckBoxCell(xAccContext->getAccessibleChild(::vcl::BBINDEX_TABLE),
                                         *this, _nRow, _nColumnPos, eState, true);
}

sal_Int32 EditBrowseBox::GetAccessibleControlCount() const
{
    return IsEditing() ? 1 : 0;
}

void EditBrowseBox::implCreateActiveAccessible( )
{
    DBG_ASSERT( IsEditing(), "EditBrowseBox::implCreateActiveAccessible: not to be called if we're not editing currently!" );
    DBG_ASSERT(!m_pActiveCell.is(), "EditBrowseBox::implCreateActiveAccessible: not to be called "
                                    "if the old one is still alive!");

    if (m_pActiveCell.is() || !IsEditing())
        return;

    ControlBase& rControl = aController->GetWindow();

    // set accessible name based on current cell (which is the one that is being edited)
    const sal_uInt16 nCol = GetColumnPos(GetCurColumnId());
    const sal_Int32 nRow = GetCurRow();
    OUString sAccName
        = SvtResId(STR_ACC_COLUMN_NUM).replaceAll("%COLUMNNUMBER", OUString::number(nCol - 1))
          + ", " + SvtResId(STR_ACC_ROW_NUM).replaceAll("%ROWNUMBER", OUString::number(nRow));
    rControl.SetAccessibleName(sAccName);

    m_pActiveCell = rControl.GetAccessible();

    commitBrowseBoxEvent(CHILD, Any(css::uno::Reference<XAccessible>(m_pActiveCell)), Any());
}

rtl::Reference<comphelper::OAccessible> EditBrowseBox::CreateAccessibleControl(sal_Int32 _nIndex)
{
    DBG_ASSERT( 0 == _nIndex, "EditBrowseBox::CreateAccessibleControl: invalid index!" );

    if ( isAccessibleAlive() )
    {
        if (!m_pActiveCell.is())
            implCreateActiveAccessible();
    }

    return m_pActiveCell;
}

void EditBrowseBox::clearActiveCell() { m_pActiveCell.clear(); }

void EditBrowseBox::GrabTableFocus()
{
    if ( aController.is() )
        aController->GetWindow().GrabFocus();
}

void EditBrowseBox::DetermineFocus( const GetFocusFlags _nGetFocusFlags )
{
    bool bFocus = ControlHasFocus();
    for (vcl::Window* pWindow = Application::GetFocusWindow();
         pWindow && !bFocus;
         pWindow = pWindow->GetParent())
         bFocus = pWindow == this;

    if (bFocus == bHasFocus)
        return;

    bHasFocus = bFocus;

    if ( !(GetBrowserFlags( ) & EditBrowseBoxFlags::SMART_TAB_TRAVEL) )
        return;

    if  (   !(bHasFocus                           // we got the focus
        &&  ( _nGetFocusFlags & GetFocusFlags::Tab ))  // using the TAB key
        )
        return;

    sal_Int32 nRows = GetRowCount();
    sal_uInt16 nCols = ColCount();

    if (( nRows <= 0 ) || ( nCols <= 0 ))
        return;

    if ( _nGetFocusFlags & GetFocusFlags::Forward )
    {
        if ( GetColumnId( 0 ) != HandleColumnId )
        {
            GoToRowColumnId( 0, GetColumnId( 0 ) );
        }
        else
        {   // the first column is the handle column -> not focussable
            if ( nCols > 1 )
                GoToRowColumnId( 0, GetColumnId( 1 ) );
        }
    }
    else if ( _nGetFocusFlags & GetFocusFlags::Backward )
    {
        GoToRowColumnId( nRows - 1, GetColumnId( nCols -1 ) );
    }
}

tools::Rectangle EditBrowseBox::GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 _nIndex)
{
    tools::Rectangle aRect;
    if ( SeekRow(_nRow) )
    {
        CellController* pController = GetController(
            _nRow, GetColumnId( sal::static_int_cast< sal_uInt16 >(_nColumnPos) ) );
        if ( pController )
            aRect = pController->GetWindow().GetCharacterBounds(_nIndex);
    }
    return aRect;
}

sal_Int32 EditBrowseBox::GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint)
{
    sal_Int32 nRet = -1;
    if ( SeekRow(_nRow) )
    {
        CellController* pController = GetController(
            _nRow, GetColumnId( sal::static_int_cast< sal_uInt16 >(_nColumnPos) ) );
        if ( pController )
            nRet = pController->GetWindow().GetIndexForPoint(_rPoint);
    }
    return nRet;
}


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
