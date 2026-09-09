/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <accessibility/vclxaccessiblebox.hxx>

#include <memory>
#include <vector>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <cppuhelper/implbase.hxx>
#include <vcl/accessibility/vclxaccessiblecomponent.hxx>

class IComboListBoxHelper;
class VCLXAccessibleListItem;

/** Base class for the list contained in list- and combo boxes.  This class
    does not support selection because lists of combo boxes give no direct
    access to their underlying list implementation.  Look into derived
    classes for selection.
*/
class VCLXAccessibleList final
    : public cppu::ImplInheritanceHelper<VCLXAccessibleComponent,
                                         css::accessibility::XAccessibleSelection>
{
public:
    enum BoxType {COMBOBOX, LISTBOX};

    VCLXAccessibleList(vcl::Window* pWindow, BoxType aBoxType,
                       const rtl::Reference<VCLXAccessibleBox>& _xParent);

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

    // XAccessibleContext
    virtual sal_Int64 getAccessibleChildCount() override final;
    css::uno::Reference< css::accessibility::XAccessible>
        getAccessibleChild (sal_Int64 i) override;
    virtual css::uno::Reference< css::accessibility::XAccessible >
        getAccessibleParent(  ) override;

    /** The index returned as index in parent is always the one set with the
        SetIndexInParent() method.
    */
    virtual sal_Int64 getAccessibleIndexInParent() override;
    virtual sal_Int16 getAccessibleRole() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    // Return list specific services.
    virtual cpo::uno::Sequence< OUString >
        getSupportedServiceNames() override;

    // XAccessibleSelection
    virtual void selectAccessibleChild( sal_Int64 nChildIndex ) override;
    virtual bool isAccessibleChildSelected( sal_Int64 nChildIndex ) override;
    virtual void clearAccessibleSelection(  ) override;
    virtual void selectAllAccessibleChildren(  ) override;
    virtual sal_Int64 getSelectedAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
    virtual void deselectAccessibleChild( sal_Int64 nSelectedChildIndex ) override;

    virtual css::awt::Point getLocationOnScreen(  ) override;
    bool    IsInDropDown() const;
    void        HandleDropOpen();
    void ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent, bool b_IsDropDownList);
    void UpdateSelection_Acc (std::u16string_view sTextOfSelectedItem, bool b_IsDropDownList);
    void UpdateSelection_Impl_Acc (bool b_IsDropDownList);

    static void NotifyListItem(cpo::uno::Any const & val);
    IComboListBoxHelper* getListBoxHelper() { return m_pListBoxHelper.get(); }
private:
    BoxType     m_aBoxType;
    std::unique_ptr<IComboListBoxHelper> m_pListBoxHelper;
    std::vector<rtl::Reference<VCLXAccessibleListItem>> m_aAccessibleChildren;
    sal_Int32   m_nVisibleLineCount;
    /// Index in parent.  This is settable from the outside.
    sal_Int32   m_nIndexInParent;
    sal_Int32   m_nLastTopEntry;
    sal_Int32   m_nLastSelectedPos;
    bool        m_bDisableProcessEvent;
    bool        m_bVisible;
    sal_Int32   m_nCurSelectedPos;


    virtual ~VCLXAccessibleList() override = default;

    sal_Int64 implGetAccessibleChildCount();

    /** This function is called from the implementation helper during a
        XComponent::dispose call.  Free the list of items and the items themselves.
    */
    virtual void disposing() override;

    void disposeChildren();

    /** This method adds the states
        AccessibleStateType::FOCUSABLE and possibly
        AccessibleStateType::MULTI_SELECTABLE to the state set
        of the base classes.
    */
    virtual void FillAccessibleStateSet (sal_Int64& rStateSet) override;

    /** Create the specified child and insert it into the list of children.
        Sets the child's states.
    */
    rtl::Reference<VCLXAccessibleListItem> CreateChild (sal_Int32 i);

    /** Call this method when the item list has been changed, i.e. items
        have been deleted or inserted.
    */
    void HandleChangedItemList();

    // VCLXAccessibleComponent
    virtual css::awt::Rectangle implGetBounds(  ) override;

    /** We need to save the accessible parent to return it in getAccessibleParent(),
        because this method of the base class returns the wrong parent.
    */
    rtl::Reference<VCLXAccessibleBox> m_xParent;

    void UpdateEntryRange_Impl();
    void UpdateSelection_Impl(sal_Int32 nPos = 0);
    bool checkEntrySelected(sal_Int32 _nPos,
                            cpo::uno::Any& _rNewValue,
                            rtl::Reference< VCLXAccessibleListItem >& _rxNewAcc);
    void notifyVisibleStates(bool _bSetNew );
    void UpdateVisibleLineCount();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
