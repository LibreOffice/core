/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessiblebrowseboxcell.hxx,v $
 * $Revision: 1.3 $
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

#ifndef ACCESSIBILITY_ACCESSIBLE_BROWSE_BOX_CELL_HXX
#define ACCESSIBILITY_ACCESSIBLE_BROWSE_BOX_CELL_HXX

#include "accessibility/extended/AccessibleBrowseBoxBase.hxx"
#include <svtools/AccessibleBrowseBoxObjType.hxx>

// .................................................................................
namespace accessibility
{
// .................................................................................

    // =============================================================================
    // = AccessibleBrowseBoxCell
    // =============================================================================
    /** common accessibility-functionality for browse box elements which occupy a cell
    */
    class AccessibleBrowseBoxCell : public AccessibleBrowseBoxBase
    {
    private:
        sal_Int32               m_nRowPos;      // the row number of the table cell
        sal_uInt16              m_nColPos;      // the column id of the table cell

    protected:
        // attribute access
        inline sal_Int32    getRowPos( ) const { return m_nRowPos; }
        inline sal_Int32    getColumnPos( ) const { return m_nColPos; }

    protected:
        // AccessibleBrowseBoxBase overridables
        virtual Rectangle implGetBoundingBox();
        virtual Rectangle implGetBoundingBoxOnScreen();

        // XAccessibleComponent
        virtual void SAL_CALL grabFocus() throw ( ::com::sun::star::uno::RuntimeException );

    protected:
        AccessibleBrowseBoxCell(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
            ::svt::IAccessibleTableProvider& _rBrowseBox,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
            sal_Int32 _nRowPos,
            sal_uInt16 _nColPos,
            ::svt::AccessibleBrowseBoxObjType _eType = ::svt::BBTYPE_TABLECELL
        );

        virtual ~AccessibleBrowseBoxCell();

    private:
        AccessibleBrowseBoxCell();                                                  // never implemented
        AccessibleBrowseBoxCell( const AccessibleBrowseBoxCell& );              // never implemented
        AccessibleBrowseBoxCell& operator=( const AccessibleBrowseBoxCell& );   // never implemented
    };

// .................................................................................
}   // namespace accessibility
// .................................................................................


#endif // ACCESSIBILITY_ACCESSIBLE_BROWSE_BOX_CELL_HXX
