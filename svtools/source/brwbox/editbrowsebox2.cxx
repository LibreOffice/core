/*************************************************************************
 *
 *  $RCSfile: editbrowsebox2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2002-05-31 13:25:17 $
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
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
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
    using namespace drafts::com::sun::star::accessibility;
    using namespace com::sun::star::uno;
    using namespace drafts::com::sun::star::accessibility::AccessibleEventId;

// -----------------------------------------------------------------------------
Reference< XAccessible > EditBrowseBox::CreateAccessibleCell( sal_Int32 nRow, sal_uInt16 nColumnId )
{
    Reference< XAccessible > xRet;
    if ( nRow == GetCurRow() && IsEditing() && nColumnId == GetCurColumnId() )
    {
        //CellController* pController = GetController(nRow, nColumnId);
        if ( aController.Is() )
        {
            Reference< XAccessible > xCont = aController->GetWindow().GetAccessible();
            Reference< XAccessible > xMy = GetAccessible();
            if ( xMy.is() && xCont.is() )
            {
                m_aImpl->m_xActiveCell = new EditBrowseBoxTableCell(xMy->getAccessibleContext()->getAccessibleChild(::svt::BBINDEX_TABLE),
                                                                    *this,
                                                                    VCLUnoHelper::GetInterface(&aController->GetWindow()),
                                                                    nRow,
                                                                    nColumnId,
                                                                    xCont->getAccessibleContext());
                xRet = m_aImpl->m_xActiveCell;
/*              commitTableEvent(ACCESSIBLE_ACTIVE_DESCENDANT_EVENT,
                             com::sun::star::uno::Any(),
                             com::sun::star::uno::makeAny(m_aImpl->m_xActiveCell));
*/          }
        }
    }
    else
        xRet = BrowseBox::CreateAccessibleCell( nRow, nColumnId );
    return xRet;
}
// -----------------------------------------------------------------------------
sal_Int32 EditBrowseBox::GetAccessibleControlCount() const
{
    return GetRowCount() * (ColCount()-1);
}
// -----------------------------------------------------------------------------

Reference< XAccessible > EditBrowseBox::CreateAccessibleControl( sal_Int32 _nIndex )
{
    sal_uInt16 nColCount = ColCount()-1;
    sal_uInt16 nColID   = _nIndex % nColCount;
    sal_Int32 nRow      = _nIndex / nColCount;

    return CreateAccessibleCell(nRow, nColID+1);
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
    }
}
// -----------------------------------------------------------------------------
void EditBrowseBox::GrabTableFocus()
{
    if ( aController.Is() )
        aController->GetWindow().GrabFocus();
}
// -----------------------------------------------------------------------------
} // namespace svt
// -----------------------------------------------------------------------------


