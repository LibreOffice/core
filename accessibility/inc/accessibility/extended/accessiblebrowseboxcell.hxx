/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessiblebrowseboxcell.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:19:15 $
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

#ifndef ACCESSIBILITY_ACCESSIBLE_BROWSE_BOX_CELL_HXX
#define ACCESSIBILITY_ACCESSIBLE_BROWSE_BOX_CELL_HXX

#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXBASE_HXX
#include "accessibility/extended/AccessibleBrowseBoxBase.hxx"
#endif

#ifndef _SVTOOLS_ACCESSIBLEBROWSEBOXOBJTYPE_HXX
#include <svtools/AccessibleBrowseBoxObjType.hxx>
#endif

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
