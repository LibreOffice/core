/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"
#include "accessibility/extended/accessiblebrowseboxcell.hxx"
#include <svtools/accessibletableprovider.hxx>

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
        return mpBrowseBox->GetFieldRectPixelAbs( m_nRowPos, m_nColPos, sal_False, sal_False );
    }

    // -----------------------------------------------------------------------------
    ::Rectangle AccessibleBrowseBoxCell::implGetBoundingBoxOnScreen()
    {
        return mpBrowseBox->GetFieldRectPixelAbs( m_nRowPos, m_nColPos, sal_False );
    }

// .................................................................................
}   // namespace accessibility
// .................................................................................


