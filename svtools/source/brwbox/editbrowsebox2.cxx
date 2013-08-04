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

#include <editbrowsebox.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include "editbrowseboximpl.hxx"
#include <comphelper/types.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "svtaccessiblefactory.hxx"

namespace svt
{
    using namespace com::sun::star::accessibility;
    using namespace com::sun::star::uno;
    using namespace ::com::sun::star::accessibility::AccessibleEventId;

// -----------------------------------------------------------------------------
Reference< XAccessible > EditBrowseBox::CreateAccessibleCheckBoxCell(long _nRow, sal_uInt16 _nColumnPos,const TriState& eState)
{
    Reference< XAccessible > xAccessible( GetAccessible() );
    Reference< XAccessibleContext > xAccContext;
    if ( xAccessible.is() )
        xAccContext = xAccessible->getAccessibleContext();

    Reference< XAccessible > xReturn;
    if ( xAccContext.is() )
    {
        xReturn = getAccessibleFactory().createAccessibleCheckBoxCell(
            xAccContext->getAccessibleChild( ::svt::BBINDEX_TABLE ),
            *this,
            NULL,
            _nRow,
            _nColumnPos,
            eState,
            sal_True
        );
    }
    return xReturn;
}
// -----------------------------------------------------------------------------
Reference< XAccessible > EditBrowseBox::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos )
{
    return BrowseBox::CreateAccessibleCell( _nRow, _nColumnPos );
}
// -----------------------------------------------------------------------------
sal_Int32 EditBrowseBox::GetAccessibleControlCount() const
{
    return IsEditing() ? 1 : 0;
}
// -----------------------------------------------------------------------------
void EditBrowseBox::implCreateActiveAccessible( )
{
    DBG_ASSERT( IsEditing(), "EditBrowseBox::implCreateActiveAccessible: not to be called if we're not editing currently!" );
    DBG_ASSERT( !m_aImpl->m_xActiveCell.is(), "EditBrowseBox::implCreateActiveAccessible: not to be called if the old one is still alive!" );

    if ( !m_aImpl->m_xActiveCell.is() && IsEditing() )
    {
         Reference< XAccessible > xCont = aController->GetWindow().GetAccessible();
         Reference< XAccessible > xMy = GetAccessible();
         if ( xMy.is() && xCont.is() )
          {
             m_aImpl->m_xActiveCell = getAccessibleFactory().createEditBrowseBoxTableCellAccess(
                 xMy,                                                       // parent accessible
                 xCont,                                                     // control accessible
                 VCLUnoHelper::GetInterface( &aController->GetWindow() ),   // focus window (for notifications)
                 *this,                                                     // the browse box
                 GetCurRow(),
                 GetColumnPos( GetCurColumnId() )
             );

             commitBrowseBoxEvent( CHILD, makeAny( m_aImpl->m_xActiveCell ), Any() );
          }
    }
}

// -----------------------------------------------------------------------------
Reference< XAccessible > EditBrowseBox::CreateAccessibleControl( sal_Int32
#ifdef DBG_UTIL
_nIndex
#endif
)
{
    DBG_ASSERT( 0 == _nIndex, "EditBrowseBox::CreateAccessibleControl: invalid index!" );

    if ( isAccessibleAlive() )
    {
        if ( !m_aImpl->m_xActiveCell.is() )
            implCreateActiveAccessible();
    }

    return m_aImpl->m_xActiveCell;
}
// -----------------------------------------------------------------------------
Reference<XAccessible > EditBrowseBox::CreateAccessibleRowHeader( sal_Int32 _nRow )
{
    return BrowseBox::CreateAccessibleRowHeader( _nRow );
}
// -----------------------------------------------------------------------------
void EditBrowseBoxImpl::clearActiveCell()
{
    try
    {
        ::comphelper::disposeComponent(m_xActiveCell);
    }
    catch(const Exception&)
    {
        OSL_FAIL( "EditBrowseBoxImpl::clearActiveCell: caught an exception while disposing the AccessibleCell!" );
    }

    m_xActiveCell = NULL;
}
// -----------------------------------------------------------------------------
void EditBrowseBox::GrabTableFocus()
{
    if ( aController.Is() )
        aController->GetWindow().GrabFocus();
}
//------------------------------------------------------------------------------
void EditBrowseBox::DetermineFocus( const sal_uInt16 _nGetFocusFlags )
{
    sal_Bool bFocus = sal_False;
    for (Window* pWindow = Application::GetFocusWindow();
         pWindow && !bFocus;
         pWindow = pWindow->GetParent())
         bFocus = pWindow == this;

    if (bFocus != bHasFocus)
    {
        bHasFocus = bFocus;

        if ( GetBrowserFlags( ) & EBBF_SMART_TAB_TRAVEL )
        {
            if  (   bHasFocus                           // we got the focus
                &&  ( _nGetFocusFlags & GETFOCUS_TAB )  // using the TAB key
                )
            {
                long nRows = GetRowCount();
                sal_uInt16 nCols = ColCount();

                if ( ( nRows > 0 ) && ( nCols > 0 ) )
                {
                    if ( _nGetFocusFlags & GETFOCUS_FORWARD )
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
                    else if ( _nGetFocusFlags & GETFOCUS_BACKWARD )
                    {
                        GoToRowColumnId( nRows - 1, GetColumnId( nCols -1 ) );
                    }
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
Rectangle EditBrowseBox::GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 _nIndex)
{
    Rectangle aRect;
    if ( SeekRow(_nRow) )
    {
        CellController* pController = GetController(
            _nRow, GetColumnId( sal::static_int_cast< sal_uInt16 >(_nColumnPos) ) );
        if ( pController )
            aRect = pController->GetWindow().GetCharacterBounds(_nIndex);
    }
    return aRect;
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
} // namespace svt
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
