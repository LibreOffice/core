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



#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX

#include "AccessibleBrowseBox.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <svtools/accessibletableprovider.hxx>

class SvHeaderTabListBox;

// ============================================================================

namespace accessibility {

class AccessibleBrowseBoxTable;

typedef ::cppu::ImplHelper1  <   ::com::sun::star::accessibility::XAccessible
                            >   AccessibleTabListBox_Base;

/** !!! */
class AccessibleTabListBox
                :public AccessibleBrowseBox
                ,public AccessibleTabListBox_Base
                ,public ::svt::IAccessibleTabListBox
{
private:
    SvHeaderTabListBox*         m_pTabListBox;

public:
    /** ctor()
        @param rxParent  XAccessible interface of the parent object.
        @param rBox  The HeaderTabListBox control. */
    AccessibleTabListBox(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
        SvHeaderTabListBox& rBox );

public:
    // XInterface
    DECLARE_XINTERFACE( )
    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )

    // XAccessibleContext -----------------------------------------------------

    /** @return  The count of visible children. */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The XAccessible interface of the specified child. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleChild( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    // XAccessibleContext
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext() throw ( ::com::sun::star::uno::RuntimeException );

    // IAccessibleTabListBox
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getMyself()
    {
        return this;
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getHeaderBar( ::svt::AccessibleBrowseBoxObjType _eObjType )
    {
        return AccessibleBrowseBox::getHeaderBar( _eObjType );
    }

protected:
    /** dtor() */
    virtual ~AccessibleTabListBox();

    /** This method creates and returns an accessible table.
        @return  An AccessibleBrowseBoxTable. */
    virtual AccessibleBrowseBoxTable*   createAccessibleTable();
};

// ============================================================================

} // namespace accessibility

// ============================================================================

#endif // ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX

