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


#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXHEADERCELL_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXHEADERCELL_HXX

#include "accessibility/extended/AccessibleBrowseBoxBase.hxx"

namespace accessibility
{
    class AccessibleBrowseBoxHeaderCell : public BrowseBoxAccessibleElement
    {
        sal_Int32   m_nColumnRowId;
    public:
        AccessibleBrowseBoxHeaderCell(sal_Int32 _nColumnRowId,
                                          const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
                                          ::svt::IAccessibleTableProvider&                  _rBrowseBox,
                                          const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
                                          ::svt::AccessibleBrowseBoxObjType  _eObjType);
        /** @return  The count of visible children. */
        virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw ( ::com::sun::star::uno::RuntimeException );

        /** @return  The XAccessible interface of the specified child. */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
            getAccessibleChild( sal_Int32 nChildIndex ) throw ( ::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException );

        /** @return  The index of this object among the parent's children. */
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw ( ::com::sun::star::uno::RuntimeException );

        /** Grabs the focus to the BrowseBox. */
        virtual void SAL_CALL grabFocus() throw ( ::com::sun::star::uno::RuntimeException );

        inline sal_Bool isRowBarCell() const
        {
            return getType() == ::svt::BBTYPE_ROWHEADERCELL;
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
    };
}

#endif // ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXHEADERCELL_HXX

