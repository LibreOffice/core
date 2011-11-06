/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
