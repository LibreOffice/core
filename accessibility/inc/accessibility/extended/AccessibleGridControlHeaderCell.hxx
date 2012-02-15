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


#ifndef ACCESSIBILITY_EXT_ACCESSIBLEGRIDCONTROLHEADERCELL_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLEGRIDCONTROLHEADERCELL_HXX

#include "accessibility/extended/AccessibleGridControlTableCell.hxx"

namespace accessibility
{
    class AccessibleGridControlHeaderCell : public AccessibleGridControlCell, public ::com::sun::star::accessibility::XAccessible
    {
        sal_Int32   m_nColumnRowId;
    public:
        AccessibleGridControlHeaderCell(sal_Int32 _nColumnRowId,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
                        ::svt::table::IAccessibleTable& _rTable,
                        ::svt::table::AccessibleTableControlObjType  _eObjType);
        /** @return  The count of visible children. */
        virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw ( ::com::sun::star::uno::RuntimeException );

        /** @return  The XAccessible interface of the specified child. */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
            getAccessibleChild( sal_Int32 nChildIndex ) throw ( ::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException );

        /** @return  The index of this object among the parent's children. */
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw ( ::com::sun::star::uno::RuntimeException );

        /** Grabs the focus to the GridControl. */
        virtual void SAL_CALL grabFocus() throw ( ::com::sun::star::uno::RuntimeException );

        // XInterface -------------------------------------------------------------

        /** Queries for a new interface. */
        ::com::sun::star::uno::Any SAL_CALL queryInterface(
                const ::com::sun::star::uno::Type& rType )
            throw ( ::com::sun::star::uno::RuntimeException );

        /** Aquires the object (calls acquire() on base class). */
        virtual void SAL_CALL acquire() throw ();

        /** Releases the object (calls release() on base class). */
        virtual void SAL_CALL release() throw ();
        // XAccessible ------------------------------------------------------------

        /** @return  The XAccessibleContext interface of this object. */
        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL
        getAccessibleContext()
            throw ( ::com::sun::star::uno::RuntimeException );
        //-------------------------------------------------------------------------
        inline sal_Bool isRowBarCell() const
        {
            return getType() == ::svt::table::TCTYPE_ROWHEADERCELL;
        }

        /** @return
                The name of this class.
        */
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw ( ::com::sun::star::uno::RuntimeException );

        /** Creates a new AccessibleStateSetHelper and fills it with states of the
            current object.
            @return
                A filled AccessibleStateSetHelper.
        */
        ::utl::AccessibleStateSetHelper* implCreateStateSetHelper();

    protected:
        virtual Rectangle implGetBoundingBox();

        virtual Rectangle implGetBoundingBoxOnScreen();
    private:
        ::rtl::OUString m_sHeaderName;
    };
}

#endif // ACCESSIBILITY_EXT_ACCESSIBLEGRIDCONTROLHEADERCELL_HXX

