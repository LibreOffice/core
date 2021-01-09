/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <memory>
#include <vector>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <cppuhelper/implbase2.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>

typedef ::cppu::ImplHelper2<
    css::accessibility::XAccessible,
    css::accessibility::XAccessibleSelection
    > VCLXAccessibleList_BASE;

typedef std::vector< css::uno::WeakReference< css::accessibility::XAccessible > >
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
class VCLXAccessibleList final
    : public VCLXAccessibleComponent,
      public VCLXAccessibleList_BASE
{
public:
    enum BoxType {COMBOBOX, LISTBOX};

    VCLXAccessibleList (VCLXWindow* pVCLXindow, BoxType aBoxType,
                        const css::uno::Reference<
                            css::accessibility::XAccessible >& _xParent);

    /** The index that is passed to this method is returned on following
        calls to getAccessibleIndexInParent.
    */
    void SetIndexInParent (sal_Int32 nIndex);

    /** Process some of the events and delegate the rest to the base classes.
    */
    virtual void ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent) override;
    virtual void    FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet ) override;

    /** Called on reception of selection events this method checks all known
        list items for a possible change in their selection state and
        updates that accordingly.  No accessibility events are sent because
        the XAccessibleSelection interface is not supported and the items
        are transient.
        @param sTextOfSelectedItem
            This string contains the text of the currently selected
            item.  It is used to retrieve the index of that item.
    */
    void UpdateSelection (std::u16string_view sTextOfSelectedItem);

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override final;
    css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 i) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleParent(  ) override;

    /** The index returned as index in parent is always the one set with the
        SetIndexInParent() method.
    */
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() override;
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    // Return list specific services.
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) override;
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) override;
    virtual void SAL_CALL clearAccessibleSelection(  ) override;
    virtual void SAL_CALL selectAllAccessibleChildren(  ) override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) override;
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) override;

    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
    bool    IsInDropDown() const;
    void        HandleDropOpen();
    void ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent, bool b_IsDropDownList);
    void UpdateSelection_Acc (std::u16string_view sTextOfSelectedItem, bool b_IsDropDownList);
    void UpdateSelection_Impl_Acc (bool b_IsDropDownList);

    static void NotifyListItem(css::uno::Any const & val);
    ::accessibility::IComboListBoxHelper* getListBoxHelper() { return m_pListBoxHelper.get(); }
private:
    BoxType     m_aBoxType;
    std::unique_ptr<::accessibility::IComboListBoxHelper> m_pListBoxHelper;
    ListItems   m_aAccessibleChildren;
    sal_Int32   m_nVisibleLineCount;
    /// Index in parent.  This is settable from the outside.
    sal_Int32   m_nIndexInParent;
    sal_Int32   m_nLastTopEntry;
    sal_Int32   m_nLastSelectedPos;
    bool        m_bDisableProcessEvent;
    bool        m_bVisible;
    sal_Int32   m_nCurSelectedPos;


    virtual ~VCLXAccessibleList() override = default;

    sal_Int32 implGetAccessibleChildCount();

    /** This function is called from the implementation helper during a
        XComponent::dispose call.  Free the list of items and the items themselves.
    */
    virtual void SAL_CALL disposing() override;

    /** This method adds the states
        AccessibleStateType::FOCUSABLE and possibly
        AccessibleStateType::MULTI_SELECTABLE to the state set
        of the base classes.
    */
    virtual void FillAccessibleStateSet (utl::AccessibleStateSetHelper& rStateSet) override;

    /** Create the specified child and insert it into the list of children.
        Sets the child's states.
    */
    css::uno::Reference< css::accessibility::XAccessible >
        CreateChild (sal_Int32 i);

    /** Call this method when the item list has been changed, i.e. items
        have been deleted or inserted.
    */
    void HandleChangedItemList();

    // VCLXAccessibleComponent
    virtual css::awt::Rectangle implGetBounds(  ) override;

    /** We need to save the accessible parent to return it in getAccessibleParent(),
        because this method of the base class returns the wrong parent.
    */
    css::uno::Reference< css::accessibility::XAccessible >  m_xParent;

    void UpdateEntryRange_Impl();
    void UpdateSelection_Impl(sal_Int32 nPos = 0);
    bool checkEntrySelected(sal_Int32 _nPos,
                            css::uno::Any& _rNewValue,
                            css::uno::Reference< css::accessibility::XAccessible >& _rxNewAcc);
    void notifyVisibleStates(bool _bSetNew );
    void UpdateVisibleLineCount();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
