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

#include <deque>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <comphelper/accessibletexthelper.hxx>
#include <vcl/toolkit/treelistentry.hxx>
#include <tools/gen.hxx>
#include <unotools/weakref.hxx>

// forward ---------------------------------------------------------------

class AccessibleListBox;
class SvTreeListEntry;

// class AccessibleListBoxEntry ------------------------------------------
typedef cppu::ImplInheritanceHelper<comphelper::OAccessible, css::accessibility::XAccessibleAction,
                                    css::accessibility::XAccessibleSelection,
                                    css::accessibility::XAccessibleText,
                                    css::accessibility::XAccessibleValue, css::lang::XServiceInfo>
    AccessibleListBoxEntry_BASE;

/** the class AccessibleListBoxEntry represents the class for an accessible object of a listbox entry */
class AccessibleListBoxEntry final : public AccessibleListBoxEntry_BASE
                                    ,public ::comphelper::OCommonAccessibleText
{
friend class AccessibleListBox;

private:
    VclPtr<SvTreeListBox>               m_pTreeListBox;
    /** The treelistbox control */
    std::deque< sal_Int32 >           m_aEntryPath;
    SvTreeListEntry*                    m_pSvLBoxEntry; // Needed for a11y focused item...

    unotools::WeakReference<AccessibleListBox> m_wListBox;

    tools::Rectangle               GetBoundingBox_Impl() const;
    bool                IsShowing_Impl() const;

    /// @throws css::lang::IndexOutOfBoundsException
    void CheckActionIndex(sal_Int32 nIndex);

    /** this function is called upon disposing the component
    */
    virtual void SAL_CALL   disposing() override;

    DECL_LINK( WindowEventListener, VclWindowEvent&, void );

    // OCommonAccessibleText
    virtual OUString                        implGetText() override;
    virtual css::lang::Locale               implGetLocale() override;
    virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;

public:
    /** Ctor()
        @param  _rListBox
            the view control
        @param  rEntry
            the entry
        @param rListBox
            the a11y object for _rListBox
    */
    AccessibleListBoxEntry( SvTreeListBox& _rListBox,
                            SvTreeListEntry& rEntry,
                            AccessibleListBox & rListBox);

    SvTreeListEntry* GetSvLBoxEntry() const { return m_pSvLBoxEntry; }

protected:
    virtual css::awt::Rectangle implGetBounds() override;

private:
    css::uno::Any SAL_CALL queryInterface(const css::uno::Type& rType) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int64 SAL_CALL getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual sal_Int64 SAL_CALL getAccessibleStateSet(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition() override;
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) override;
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 SAL_CALL getCharacterCount() override;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual OUString SAL_CALL getSelectedText() override;
    virtual sal_Int32 SAL_CALL getSelectionStart() override;
    virtual sal_Int32 SAL_CALL getSelectionEnd() override;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual OUString SAL_CALL getText() override;
    virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override;

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount(  ) override;
    virtual sal_Bool SAL_CALL doAccessibleAction( sal_Int32 nIndex ) override;
    virtual OUString SAL_CALL getAccessibleActionDescription( sal_Int32 nIndex ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) override;

    // XAccessibleSelection
    void SAL_CALL selectAccessibleChild( sal_Int64 nChildIndex ) override;
    sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int64 nChildIndex ) override;
    void SAL_CALL clearAccessibleSelection(  ) override;
    void SAL_CALL selectAllAccessibleChildren(  ) override;
    sal_Int64 SAL_CALL getSelectedAccessibleChildCount(  ) override;
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
    void SAL_CALL deselectAccessibleChild( sal_Int64 nSelectedChildIndex ) override;

    // XAccessibleValue
    virtual css::uno::Any SAL_CALL getCurrentValue(  ) override;
    virtual sal_Bool SAL_CALL setCurrentValue( const css::uno::Any& aNumber ) override;
    virtual css::uno::Any SAL_CALL getMaximumValue(  ) override;
    virtual css::uno::Any SAL_CALL getMinimumValue(  ) override;
    virtual css::uno::Any SAL_CALL getMinimumIncrement(  ) override;

    rtl::Reference<comphelper::OAccessible> implGetParentAccessible() const;
    SvTreeListEntry* GetRealChild(sal_Int32 nIndex);
    sal_Int32 GetRoleType() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
