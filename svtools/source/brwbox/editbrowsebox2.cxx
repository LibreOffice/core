/*************************************************************************
 *
 *  $RCSfile: editbrowsebox2.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 15:47:14 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef _SVTOOLS_EDITBROWSEBOX_HXX_
#include "editbrowsebox.hxx"
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _SVTOOLS_ACCESSIBILEEDITBROWSEBOXTABLECELL_HXX
#include "editbrowseboxcell.hxx"
#endif
#ifndef SVTOOLS_EDITBROWSEBOX_IMPL_HXX
#include "editbrowseboximpl.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
namespace svt
{
    using namespace com::sun::star::accessibility;
    using namespace com::sun::star::uno;
    using namespace com::sun::star::accessibility::AccessibleEventId;

// -----------------------------------------------------------------------------
Reference< XAccessible > EditBrowseBox::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnId )
{
    return BrowseBox::CreateAccessibleCell( _nRow, _nColumnId );
}
// -----------------------------------------------------------------------------
sal_Int32 EditBrowseBox::GetAccessibleControlCount() const
{
    return (IsEditing() && bHasFocus) ? 1 : 0;
}
// -----------------------------------------------------------------------------

Reference< XAccessible > EditBrowseBox::CreateAccessibleControl( sal_Int32 _nIndex )
{
    if ( isAccessibleCreated() )
    {
        if ( !m_aImpl->m_xActiveCell.is() && IsEditing() && bHasFocus )
        {
            Reference< XAccessible > xCont = aController->GetWindow().GetAccessible();
            Reference< XAccessible > xMy = GetAccessible();
            if ( xMy.is() && xCont.is() )
            {
                if ( m_aImpl->m_xActiveCell.is() )
                {
                    commitBrowseBoxEvent(CHILD,Any(),makeAny(m_aImpl->m_xActiveCell));
                    m_aImpl->disposeCell();
                }

                m_aImpl->m_pFocusCell  = new EditBrowseBoxTableCell(xMy->getAccessibleContext()->getAccessibleChild(::svt::BBTYPE_BROWSEBOX),
                                                                    *this,
                                                                    VCLUnoHelper::GetInterface(&aController->GetWindow()),
                                                                    GetCurRow(),
                                                                    GetCurColumnId(),
                                                                    xCont->getAccessibleContext());
                m_aImpl->m_xActiveCell = m_aImpl->m_pFocusCell;

                commitBrowseBoxEvent(CHILD,makeAny(m_aImpl->m_xActiveCell),Any());
            }
        }
    }

    return m_aImpl->m_xActiveCell;//CreateAccessibleCell(nRow, nColID+1);
}
// -----------------------------------------------------------------------------
Reference<XAccessible > EditBrowseBox::CreateAccessibleRowHeader( sal_Int32 _nRow )
{
    return BrowseBox::CreateAccessibleRowHeader( _nRow );
}
// -----------------------------------------------------------------------------
void EditBrowseBoxImpl::disposeCell()
{
    try
    {
        ::comphelper::disposeComponent(m_xActiveCell);
    }
    catch(const Exception&)
    {
        OSL_ENSURE( sal_False, "EditBrowseBoxImpl::disposeCell: caught an exception while disposing the AccessibleCell!" );
    }
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
        if ( bHasFocus )
            CreateAccessibleControl(0);
        else
        {
            commitBrowseBoxEvent(CHILD,Any(),makeAny(m_aImpl->m_xActiveCell));
            m_aImpl->disposeCell();

            m_aImpl->m_pFocusCell  = NULL;
            m_aImpl->m_xActiveCell = NULL;
        }

        if ( GetBrowserFlags( ) & EBBF_SMART_TAB_TRAVEL )
        {
            if  (   bHasFocus                           // we got the focus
                &&  ( _nGetFocusFlags & GETFOCUS_TAB )  // using the TAB key
                )
            {
                long nRowCount = GetRowCount();
                USHORT nColCount = ColCount();

                if ( ( nRowCount > 0 ) && ( nColCount > 0 ) )
                {
                    if ( _nGetFocusFlags & GETFOCUS_FORWARD )
                    {
                        if ( GetColumnId( 0 ) != 0 )
                        {
                            GoToRowColumnId( 0, GetColumnId( 0 ) );
                        }
                        else
                        {   // the first column is the handle column -> not focussable
                            if ( nColCount > 1 )
                                GoToRowColumnId( 0, GetColumnId( 1 ) );
                        }
                    }
                    else if ( _nGetFocusFlags & GETFOCUS_BACKWARD )
                    {
                        GoToRowColumnId( nRowCount - 1, GetColumnId( nColCount -1 ) );
                    }
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
Rectangle EditBrowseBox::GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnId,sal_Int32 _nIndex)
{
    Rectangle aRect;
    if ( SeekRow(_nRow) )
    {
        CellController* pController = GetController(_nRow,_nColumnId);
        if ( pController )
            aRect = pController->GetWindow().GetCharacterBounds(_nIndex);
    }
    return aRect;
}
// -----------------------------------------------------------------------------
sal_Int32 EditBrowseBox::GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnId,const Point& _rPoint)
{
    sal_Int32 nRet = -1;
    if ( SeekRow(_nRow) )
    {
        CellController* pController = GetController(_nRow,_nColumnId);
        if ( pController )
            nRet = pController->GetWindow().GetIndexForPoint(_rPoint);
    }
    return nRet;
}


// -----------------------------------------------------------------------------
} // namespace svt
// -----------------------------------------------------------------------------


