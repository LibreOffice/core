/*************************************************************************
 *
 *  $RCSfile: brwbox3.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: oj $ $Date: 2002-06-21 14:04:32 $
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

#ifndef _SVX_BRWBOX_HXX
#include "brwbox.hxx"
#endif
#ifndef _SVTOOLS_ACCESSIBLEBROWSEBOX_HXX
#include "AccessibleBrowseBox.hxx"
#endif
#include "AccessibleBrowseBoxHeaderCell.hxx"
#ifndef _SVTOOLS_ACCESSIBLEBROWSEBOXOBJTYPE_HXX
#include "AccessibleBrowseBoxObjType.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SFXDATWIN_HXX
#include "datwin.hxx"
#endif
#ifndef _SVTOOLS_BRWIMPL_HXX
#include "brwimpl.hxx"
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _SVTOOLS_ACCESSIBILEBROWSEBOXTABLECELL_HXX
#include "AccessibleBrowseBoxTableCell.hxx"
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

// Accessibility ==============================================================

using ::rtl::OUString;
using ::svt::AccessibleBrowseBox;
using namespace ::com::sun::star::uno;
using ::drafts::com::sun::star::accessibility::XAccessible;
using namespace ::drafts::com::sun::star::accessibility;

// ============================================================================
namespace svt
{
    using namespace ::com::sun::star::lang;
    using namespace utl;

    Reference< XAccessible > getHeaderCell( BrowseBoxImpl::THeaderCellMap& _raHederCells,
                                            sal_Int32 _nId,
                                            AccessibleBrowseBoxObjType _eType,
                                            const Reference< XAccessible >& _rParent,
                                            BrowseBox& _rBrowseBox)
    {
        Reference< XAccessible > xRet;
        BrowseBoxImpl::THeaderCellMap::iterator aFind = _raHederCells.find( _nId );
        if ( aFind == _raHederCells.end() )
        {
            svt::AccessibleBrowseBoxHeaderCell* pNew = new svt::AccessibleBrowseBoxHeaderCell(_nId,
                                                        _rParent,
                                                        _rBrowseBox,
                                                        NULL,
                                                        _eType);
            pNew->acquire();
            aFind = _raHederCells.insert( BrowseBoxImpl::THeaderCellMap::value_type(_nId,pNew) ).first;
        }
        if ( aFind != _raHederCells.end() )
        {
            sal_Int32 nId = aFind->first;
            xRet = aFind->second;
        }
        return xRet;
    }
}
// ============================================================================

Reference< XAccessible > BrowseBox::CreateAccessible()
{
    Window* pParent = GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "BrowseBox::CreateAccessible - parent not found" );

    Reference< XAccessible > xAccessible = m_pImpl->m_pAccessible;
    if( pParent && !m_pImpl->m_pAccessible)
    {
        Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if( xAccParent.is() )
        {
            m_pImpl->m_pAccessible = new AccessibleBrowseBox( xAccParent, *this );
            xAccessible = m_pImpl->m_pAccessible;
        }
    }
    return xAccessible;
}
// -----------------------------------------------------------------------------

// Children -------------------------------------------------------------------

Reference< XAccessible > BrowseBox::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnId )
{
    // BBINDEX_TABLE must be the table
    OSL_ENSURE(m_pImpl->m_pAccessible,"Invalid call: Accessible is null");
    return new svt::AccessibleBrowseBoxTableCell(m_pImpl->m_pAccessible->getAccessibleChild(::svt::BBINDEX_TABLE),*this,NULL,_nRow,_nColumnId);
}
// -----------------------------------------------------------------------------

Reference< XAccessible > BrowseBox::CreateAccessibleRowHeader( sal_Int32 _nRow )
{
    return svt::getHeaderCell(
            m_pImpl->m_aRowHeaderCellMap,
            _nRow,
            svt::BBTYPE_ROWHEADERCELL,
            m_pImpl->m_pAccessible->getHeaderBar(svt::BBTYPE_ROWHEADERBAR),
            *this);
}
// -----------------------------------------------------------------------------

Reference< XAccessible > BrowseBox::CreateAccessibleColumnHeader( sal_uInt16 _nColumnId )
{
    return svt::getHeaderCell(
            m_pImpl->m_aColHeaderCellMap,
            _nColumnId,
            svt::BBTYPE_COLUMNHEADERCELL,
            m_pImpl->m_pAccessible->getHeaderBar(svt::BBTYPE_COLUMNHEADERBAR),
            *this);
}
// -----------------------------------------------------------------------------

sal_Int32 BrowseBox::GetAccessibleControlCount() const
{
    return 0;
}
// -----------------------------------------------------------------------------

Reference< XAccessible > BrowseBox::CreateAccessibleControl( sal_Int32 nIndex )
{
    return NULL;
}
// -----------------------------------------------------------------------------

// Conversions ----------------------------------------------------------------

sal_Bool BrowseBox::ConvertPointToCellAddress(
        sal_Int32& rnRow, sal_uInt16& rnColumnId, const Point& rPoint )
{
    //! TODO has to be checked
    rnRow = GetRowAtYPosPixel(rPoint.Y());
    rnColumnId = GetColumnAtXPosPixel(rPoint.X());
    return rnRow != BROWSER_INVALIDID && rnColumnId != BROWSER_INVALIDID;
}
// -----------------------------------------------------------------------------

sal_Bool BrowseBox::ConvertPointToRowHeader( sal_Int32& rnRow, const Point& rPoint )
{
    rnRow = GetRowAtYPosPixel(rPoint.Y());
    //  USHORT nColumnId = GetColumnAtXPosPixel(rPoint.X());
    return rnRow != BROWSER_INVALIDID;// && nColumnId == 0;
}
// -----------------------------------------------------------------------------

sal_Bool BrowseBox::ConvertPointToColumnHeader( sal_uInt16& _rnColumnId, const Point& _rPoint )
{
    _rnColumnId = GetColumnAtXPosPixel(_rPoint.X());
    return _rnColumnId != BROWSER_INVALIDID;
}
// -----------------------------------------------------------------------------

sal_Bool BrowseBox::ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint )
{
    //! TODO has to be checked
    sal_Int32 nRow = 0;
    sal_uInt16 nColumn = 0;
    sal_Bool bRet = ConvertPointToCellAddress(nRow,nColumn,_rPoint);
    if ( bRet )
        _rnIndex = nRow * ColCount() + nColumn;

    return bRet;
}
// -----------------------------------------------------------------------------

// Object data and state ------------------------------------------------------

OUString BrowseBox::GetAccessibleName( ::svt::AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition ) const
{
    OUString aRetText;
    switch( eObjType )
    {
        case ::svt::BBTYPE_BROWSEBOX:
            aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "BrowseBox" ) );
            break;
        case ::svt::BBTYPE_TABLE:
            aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "Table" ) );
            break;
        case ::svt::BBTYPE_ROWHEADERBAR:
            aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "RowHeaderBar" ) );
            break;
        case ::svt::BBTYPE_COLUMNHEADERBAR:
            aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "ColumnHeaderBar" ) );
            break;
        case ::svt::BBTYPE_TABLECELL:
            aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "TableCell[ " ) );
#ifdef DEBUG
            aRetText += OUString::valueOf(sal_Int32(GetCurRow()));
            aRetText += OUString( RTL_CONSTASCII_USTRINGPARAM( "," ) );
            aRetText += OUString::valueOf(sal_Int32(GetCurColumnId()));
            aRetText += OUString( RTL_CONSTASCII_USTRINGPARAM( "]" ) );
#endif
            break;
        case ::svt::BBTYPE_ROWHEADERCELL:
            aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "RowHeaderCell" ) );
#ifdef DEBUG
            aRetText += OUString::valueOf(sal_Int32(GetCurRow()));
            aRetText += OUString( RTL_CONSTASCII_USTRINGPARAM( "," ) );
            aRetText += OUString::valueOf(sal_Int32(GetCurColumnId()));
            aRetText += OUString( RTL_CONSTASCII_USTRINGPARAM( "]" ) );
#endif
            break;
        case ::svt::BBTYPE_COLUMNHEADERCELL:
            aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "ColumnHeaderCell" ) );
#ifdef DEBUG
            aRetText += OUString::valueOf(sal_Int32(GetCurRow()));
            aRetText += OUString( RTL_CONSTASCII_USTRINGPARAM( "," ) );
            aRetText += OUString::valueOf(sal_Int32(GetCurColumnId()));
            aRetText += OUString( RTL_CONSTASCII_USTRINGPARAM( "]" ) );
#endif
            break;
        default:
            OSL_ENSURE(0,"BrowseBox::GetAccessibleName: invalid enum!");
    }
    return aRetText;
}
// -----------------------------------------------------------------------------

OUString BrowseBox::GetAccessibleDescription( ::svt::AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition ) const
{
    OUString aRetText;
    switch( eObjType )
    {
        case ::svt::BBTYPE_BROWSEBOX:
            aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "BrowseBox description" ) );
            break;
        case ::svt::BBTYPE_TABLE:
            //  aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "TABLE description" ) );
            break;
        case ::svt::BBTYPE_ROWHEADERBAR:
            //  aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "ROWHEADERBAR description" ) );
            break;
        case ::svt::BBTYPE_COLUMNHEADERBAR:
            //  aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "COLUMNHEADERBAR description" ) );
            break;
        case ::svt::BBTYPE_TABLECELL:
            //  aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "TABLECELL description" ) );
            break;
        case ::svt::BBTYPE_ROWHEADERCELL:
            //  aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "ROWHEADERCELL description" ) );
            break;
        case ::svt::BBTYPE_COLUMNHEADERCELL:
            //  aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "COLUMNHEADERCELL description" ) );
            break;
    }
    return aRetText;
}
// -----------------------------------------------------------------------------

OUString BrowseBox::GetRowDescription( sal_Int32 nRow ) const
{
    return OUString();
}
// -----------------------------------------------------------------------------

void BrowseBox::FillAccessibleStateSet(
        ::utl::AccessibleStateSetHelper& rStateSet,
        ::svt::AccessibleBrowseBoxObjType eObjType ) const
{
    switch( eObjType )
    {
        case ::svt::BBTYPE_BROWSEBOX:
        case ::svt::BBTYPE_TABLE:

            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            if ( HasFocus() )
                rStateSet.AddState( AccessibleStateType::FOCUSED );
            if ( IsActive() )
                rStateSet.AddState( AccessibleStateType::ACTIVE );
            if ( GetUpdateMode() )
                rStateSet.AddState( AccessibleStateType::EDITABLE );
            if ( IsEnabled() )
                rStateSet.AddState( AccessibleStateType::ENABLED );
            if ( IsReallyVisible() )
                rStateSet.AddState( AccessibleStateType::VISIBLE );
            if ( eObjType == ::svt::BBTYPE_TABLE )
                rStateSet.AddState( AccessibleStateType::CHILDREN_TRANSIENT );

            break;
        case ::svt::BBTYPE_ROWHEADERBAR:
            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            rStateSet.AddState( AccessibleStateType::VISIBLE );
            if ( GetSelectRowCount() )
                rStateSet.AddState( AccessibleStateType::FOCUSED );
            rStateSet.AddState( AccessibleStateType::CHILDREN_TRANSIENT );
            break;
        case ::svt::BBTYPE_COLUMNHEADERBAR:
            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            rStateSet.AddState( AccessibleStateType::VISIBLE );
            if ( GetSelectColumnCount() )
                rStateSet.AddState( AccessibleStateType::FOCUSED );
            rStateSet.AddState( AccessibleStateType::CHILDREN_TRANSIENT );
            break;
        case ::svt::BBTYPE_TABLECELL:
            {
                sal_Int32 nCurRow = GetCurRow();
                sal_uInt16 nCurColumn = GetCurColumnId();
                if ( IsFieldVisible(nCurRow,nCurColumn) )
                    rStateSet.AddState( AccessibleStateType::VISIBLE );
                if ( !IsFrozen( nCurColumn ) )
                    rStateSet.AddState( AccessibleStateType::FOCUSABLE );
                rStateSet.AddState( AccessibleStateType::TRANSIENT );
            }
            break;
        case ::svt::BBTYPE_ROWHEADERCELL:
        case ::svt::BBTYPE_COLUMNHEADERCELL:
            rStateSet.AddState( AccessibleStateType::VISIBLE );
            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            rStateSet.AddState( AccessibleStateType::TRANSIENT );
            break;
    }
}
// -----------------------------------------------------------------------------

void BrowseBox::GrabTableFocus()
{
    GrabFocus();
}
// -----------------------------------------------------------------------------
String BrowseBox::GetCellText(long _nRow, USHORT _nColId) const
{
    DBG_ASSERT(0,"This method has to be implemented by the derived classes! BUG!!");
    return String();
}
// -----------------------------------------------------------------------------
void BrowseBox::commitTableEvent(sal_Int16 _nEventId,
            const ::com::sun::star::uno::Any& _rNewValue,
            const ::com::sun::star::uno::Any& _rOldValue)
{
    if ( m_pImpl->m_pAccessible )
        m_pImpl->commitTableEvent(  _nEventId, _rNewValue, _rOldValue);
}
// -----------------------------------------------------------------------------
void BrowseBox::commitBrowseBoxEvent(sal_Int16 _nEventId,
            const ::com::sun::star::uno::Any& _rNewValue,
            const ::com::sun::star::uno::Any& _rOldValue)
{
    if ( m_pImpl->m_pAccessible )
        m_pImpl->m_pAccessible->commitEvent( _nEventId, _rNewValue, _rOldValue);
}
// -----------------------------------------------------------------------------
sal_Bool BrowseBox::isAccessibleCreated() const
{
    return m_pImpl->m_pAccessible != NULL;
}
// -----------------------------------------------------------------------------

