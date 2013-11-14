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



#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_CELL_HXX
#define _SVX_ACCESSIBILITY_ACCESSIBLE_CELL_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include <rtl/ref.hxx>

#include <editeng/AccessibleContextBase.hxx>
#include <editeng/AccessibleComponentBase.hxx>
#include <svx/IAccessibleViewForwarderListener.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <svx/AccessibleShapeTreeInfo.hxx>

#include <cppuhelper/implbase1.hxx>

#include "cell.hxx"

#include <boost/noncopyable.hpp>

#include <svx/AccessibleTableShape.hxx>

class SdrObject;


namespace accessibility
{

class AccessibleShapeInfo;
class AccessibleShapeTreeInfo;
class IAccessibleParent;

typedef ::cppu::ImplInheritanceHelper1< AccessibleContextBase, ::com::sun::star::accessibility::XAccessibleExtendedComponent > AccessibleCellBase;

class AccessibleCell : boost::noncopyable, public AccessibleCellBase, public AccessibleComponentBase, public IAccessibleViewForwarderListener
{
public:
    AccessibleCell( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>& rxParent, const sdr::table::CellRef& rCell, sal_Int32 nIndex, const AccessibleShapeTreeInfo& rShapeTreeInfo);
    virtual ~AccessibleCell (void);

    virtual void Init (void);

    virtual bool operator== (const AccessibleCell& rAccessibleCell);

    virtual sal_Bool SetState (sal_Int16 aState);
    virtual sal_Bool ResetState (sal_Int16 aState);
    sal_Bool GetState (sal_Int16 aState);

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(void) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL getAccessibleChild(sal_Int32 nIndex) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL getAccessibleStateSet(void) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(void) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleName (void) throw (::com::sun::star::uno::RuntimeException);
    sdr::table::CellRef getCellRef();
    void UpdateChildren();
    ::rtl::OUString getCellName( sal_Int32 nCol, sal_Int32 nRow );

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint(const ::com::sun::star::awt::Point& aPoint) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(void) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(void) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(void) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(void) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addFocusListener ( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeFocusListener (const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus (void) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground(void) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(void) throw(::com::sun::star::uno::RuntimeException);

    // XAccessibleExtendedComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont (void) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTitledBorderText (void) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getToolTipText (void) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& rxListener) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName (void) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames (void) throw(::com::sun::star::uno::RuntimeException);

    // IAccessibleViewForwarderListener
    virtual void ViewForwarderChanged (ChangeType aChangeType, const IAccessibleViewForwarder* pViewForwarder);

    using cppu::WeakComponentImplHelperBase::addEventListener;
    using cppu::WeakComponentImplHelperBase::removeEventListener;

    // Misc

    /** set the index _nIndex at the accessible cell param  _nIndex The new index in parent.
    */
    inline void setIndexInParent(sal_Int32 _nIndex) { mnIndexInParent = _nIndex; }

    //Get the parent table
    AccessibleTableShape* GetParentTable() { return pAccTable; }
protected:
    /// Bundle of information passed to all shapes in a document tree.
    AccessibleShapeTreeInfo maShapeTreeInfo;

    /// the index in parent.
    sal_Int32 mnIndexInParent;

    /// The accessible text engine.  May be NULL if it can not be created.
    AccessibleTextHelper* mpText;

    sdr::table::CellRef mxCell;

    /// This method is called from the component helper base class while disposing.
    virtual void SAL_CALL disposing (void);

    AccessibleTableShape *pAccTable;

private:
    explicit AccessibleCell(void);  // not implemented
    explicit AccessibleCell(const AccessibleCell&); // not implemented
    AccessibleCell& operator=(const AccessibleCell&); // not implemented
};

} // end of namespace accessibility

#endif
