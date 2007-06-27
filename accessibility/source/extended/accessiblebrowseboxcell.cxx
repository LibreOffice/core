/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessiblebrowseboxcell.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:32:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"

#ifndef ACCESSIBILITY_ACCESSIBLE_BROWSE_BOX_CELL_HXX
#include "accessibility/extended/accessiblebrowseboxcell.hxx"
#endif
#ifndef _SVTOOLS_ACCESSIBLETABLEPROVIDER_HXX
#include <svtools/accessibletableprovider.hxx>
#endif

// .................................................................................
namespace accessibility
{
// .................................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::accessibility;
    using namespace ::svt;

    // =============================================================================
    // = AccessibleBrowseBoxCell
    // =============================================================================
    DBG_NAME( svt_AccessibleBrowseBoxCell )
    // -----------------------------------------------------------------------------
    AccessibleBrowseBoxCell::AccessibleBrowseBoxCell(
            const Reference< XAccessible >& _rxParent, IAccessibleTableProvider& _rBrowseBox,
            const Reference< XWindow >& _xFocusWindow,
            sal_Int32 _nRowPos, sal_uInt16 _nColPos, AccessibleBrowseBoxObjType _eType )
        :AccessibleBrowseBoxBase( _rxParent, _rBrowseBox, _xFocusWindow, _eType )
        ,m_nRowPos( _nRowPos )
        ,m_nColPos( _nColPos )
    {
        DBG_CTOR( svt_AccessibleBrowseBoxCell, NULL );
        // set accessible name here, because for that we need the position of the cell
        // and so the base class isn't capable of doing this
        sal_Int32 nPos = _nRowPos * _rBrowseBox.GetColumnCount() + _nColPos;
        ::rtl::OUString aAccName = _rBrowseBox.GetAccessibleObjectName( BBTYPE_TABLECELL, nPos );
        implSetName( aAccName );
    }

    // -----------------------------------------------------------------------------
    AccessibleBrowseBoxCell::~AccessibleBrowseBoxCell()
    {
        DBG_DTOR( svt_AccessibleBrowseBoxCell, NULL );
    }

    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleBrowseBoxCell::grabFocus() throw ( RuntimeException )
    {
        SolarMethodGuard aGuard( *this );
        mpBrowseBox->GoToCell( m_nRowPos, m_nColPos );
    }
    // -----------------------------------------------------------------------------
    ::Rectangle AccessibleBrowseBoxCell::implGetBoundingBox()
    {
        return mpBrowseBox->GetFieldRectPixelAbs( m_nRowPos, m_nColPos, FALSE, FALSE );
    }

    // -----------------------------------------------------------------------------
    ::Rectangle AccessibleBrowseBoxCell::implGetBoundingBoxOnScreen()
    {
        return mpBrowseBox->GetFieldRectPixelAbs( m_nRowPos, m_nColPos, FALSE );
    }

// .................................................................................
}   // namespace accessibility
// .................................................................................


