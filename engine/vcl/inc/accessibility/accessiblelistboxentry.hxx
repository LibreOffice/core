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
    virtual void   disposing() override;

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
    cpo::uno::Any queryInterface(const cpo::uno::Type& rType) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XAccessibleContext
    virtual sal_Int64 getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleParent(  ) override;
    virtual sal_Int64 getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 getAccessibleRole(  ) override;
    virtual OUString getAccessibleDescription(  ) override;
    virtual OUString getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > getAccessibleRelationSet(  ) override;
    virtual sal_Int64 getAccessibleStateSet(  ) override;
    virtual css::lang::Locale getLocale(  ) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void grabFocus(  ) override;
    virtual sal_Int32 getForeground(  ) override;
    virtual sal_Int32 getBackground(  ) override;

    // XAccessibleText
    virtual sal_Int32 getCaretPosition() override;
    virtual bool setCaretPosition( sal_Int32 nIndex ) override;
    virtual sal_Unicode getCharacter( sal_Int32 nIndex ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyValue > getCharacterAttributes( sal_Int32 nIndex, const cpo::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 getCharacterCount() override;
    virtual sal_Int32 getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual OUString getSelectedText() override;
    virtual sal_Int32 getSelectionStart() override;
    virtual sal_Int32 getSelectionEnd() override;
    virtual bool setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual OUString getText() override;
    virtual OUString getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual css::accessibility::TextSegment getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual bool copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual bool scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override;

    // XAccessibleAction
    virtual sal_Int32 getAccessibleActionCount(  ) override;
    virtual bool doAccessibleAction( sal_Int32 nIndex ) override;
    virtual OUString getAccessibleActionDescription( sal_Int32 nIndex ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > getAccessibleActionKeyBinding( sal_Int32 nIndex ) override;

    // XAccessibleSelection
    void selectAccessibleChild( sal_Int64 nChildIndex ) override;
    bool isAccessibleChildSelected( sal_Int64 nChildIndex ) override;
    void clearAccessibleSelection(  ) override;
    void selectAllAccessibleChildren(  ) override;
    sal_Int64 getSelectedAccessibleChildCount(  ) override;
    css::uno::Reference< css::accessibility::XAccessible > getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
    void deselectAccessibleChild( sal_Int64 nSelectedChildIndex ) override;

    // XAccessibleValue
    virtual cpo::uno::Any getCurrentValue(  ) override;
    virtual bool setCurrentValue( const cpo::uno::Any& aNumber ) override;
    virtual cpo::uno::Any getMaximumValue(  ) override;
    virtual cpo::uno::Any getMinimumValue(  ) override;
    virtual cpo::uno::Any getMinimumIncrement(  ) override;

    rtl::Reference<comphelper::OAccessible> implGetParentAccessible() const;
    SvTreeListEntry* GetRealChild(sal_Int32 nIndex);
    sal_Int32 GetRoleType() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
