/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxaccessiblelist.hxx,v $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLELIST_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLELIST_HXX

#include <vector>
#include <functional>
#include "accessibility/standard/vclxaccessiblelistitem.hxx"
#include <accessibility/standard/vclxaccessibleedit.hxx>
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#ifndef _CPPUHELPER_IMPLBASE2_HXX
#include <cppuhelper/implbase2.hxx>
#endif

typedef ::cppu::ImplHelper2<
    ::com::sun::star::accessibility::XAccessible,
    ::com::sun::star::accessibility::XAccessibleSelection
    > VCLXAccessibleList_BASE;

typedef std::vector< ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > >
    ListItems;

namespace accessibility
{
    class IComboListBoxHelper;
}


/** Base class for the list contained in list- and combo boxes.  This class
    does not support selection because lists of combo boxes give no direct
    access to their underlying list implementation.  Look into derived
    classes for selection.
*/
class VCLXAccessibleList
    : public VCLXAccessibleComponent,
      public VCLXAccessibleList_BASE
{
public:
    enum BoxType {COMBOBOX, LISTBOX};

    VCLXAccessibleList (VCLXWindow* pVCLXindow, BoxType aBoxType,
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::accessibility::XAccessible >& _xParent);

    /** The index that is passed to this method is returned on following
        calls to <member>getAccessibleIndexInParent</member>.
    */
    void SetIndexInParent (sal_Int32 nIndex);

    /** Process some of the events and delegate the rest to the base classes.
    */
    virtual void ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent);

    /** Called on reception of selection events this method checks all known
        list items for a possible change in their selection state and
        updates that accordingly.  No accessibility events are send because
        the XAccessibleSelection interface is not supported and the items
        are transient.
        @param sTextOfSelectedItem
            This string contains the text of the the currently selected
            item.  It is used to retrieve the index of that item.
    */
    void UpdateSelection (::rtl::OUString sTextOfSelectedItem);

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext (void)
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount (void)
        throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 i)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleParent(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** The index returned as index in parent is always the one set with the
        <member>SetIndexInParent()</member> method.
    */
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL contains (const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleAt (const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException);


    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);
    // Return list specific services.
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL selectAllAccessibleChildren(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);

protected:
    BoxType     m_aBoxType;
    ::accessibility::IComboListBoxHelper* m_pListBoxHelper;
    ListItems   m_aAccessibleChildren;
    sal_Int32   m_nVisibleLineCount;
    /// Index in parent.  This is settable from the outside.
    sal_Int32   m_nIndexInParent;
    sal_Int32   m_nLastTopEntry;
    USHORT      m_nLastSelectedPos;
    bool        m_bDisableProcessEvent;
    bool        m_bVisible;



    /// The currently selected item.
    ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible>
        m_xSelectedItem;

    virtual ~VCLXAccessibleList (void);

    /** This function is called from the implementation helper during a
        XComponent::dispose call.  Free the list of items and the items themselves.
    */
    virtual void SAL_CALL disposing (void);

    /** This method adds the states
        <const>AccessibleStateType::FOCUSABLE</const> and possibly
        <const>AccessibleStateType::MULTI_SELECTABLE</const> to the state set
        of the base classes.
    */
    virtual void FillAccessibleStateSet (utl::AccessibleStateSetHelper& rStateSet);

    /** Create the specified child and insert it into the list of children.
        Sets the child's states.
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        CreateChild (sal_Int32 i);

    /** Call this method when the item list has been changed, i.e. items
        have been deleted or inserted.
        @param bItemInserted
            Indicate whether items have been inserted (<TRUE/>) or removed
            (<FALSE/>).
        @param nIndex
            Index of the new or removed item.  A value of -1 indicates that
            the whole list has been cleared.
    */
    virtual void HandleChangedItemList (bool bItemInserted, sal_Int32 nIndex);

    // VCLXAccessibleComponent
    virtual ::com::sun::star::awt::Rectangle SAL_CALL implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);

private:
    /** We need to save the accessible parent to return it in <type>getAccessibleParent()</type>,
        because this method of the base class returns the wrong parent.
    */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >  m_xParent;


    /** dispose all items aand clears the container
    */
    void clearItems();

    void adjustEntriesIndexInParent(ListItems::iterator& _aBegin,::std::mem_fun_t<bool,VCLXAccessibleListItem>& _rMemFun);
    void UpdateEntryRange_Impl (void);
protected:
    void UpdateSelection_Impl (USHORT nPos = 0);
    BOOL checkEntrySelected(USHORT _nPos,
                            ::com::sun::star::uno::Any& _rNewValue,
                            ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxNewAcc);
private:
    void notifyVisibleStates(sal_Bool _bSetNew );
};

#endif

