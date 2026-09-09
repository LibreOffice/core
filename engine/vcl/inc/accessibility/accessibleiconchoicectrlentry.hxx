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

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <tools/gen.hxx>
#include <vcl/vclptr.hxx>

// forward

class SvtIconChoiceCtrl;

typedef ::cppu::ImplInheritanceHelper<comphelper::OAccessible, css::accessibility::XAccessibleText,
                                      css::accessibility::XAccessibleAction,
                                      css::lang::XServiceInfo, css::lang::XEventListener>
    AccessibleIconChoiceCtrlEntry_BASE;

/** the class AccessibleListBoxEntry represents the class for an accessible object of a listbox entry */
class AccessibleIconChoiceCtrlEntry final : public AccessibleIconChoiceCtrlEntry_BASE,
                                            public ::comphelper::OCommonAccessibleText
{
    /** The treelistbox control */
    VclPtr<SvtIconChoiceCtrl>           m_pIconCtrl;
    sal_Int32                           m_nIndex;

    css::uno::Reference< css::accessibility::XAccessible > m_xParent;

    tools::Rectangle               GetBoundingBox_Impl() const;
    bool                IsAlive_Impl() const;
    bool                IsShowing_Impl() const;

    /// @throws css::lang::DisposedException
    /// @throws cpo::uno::RuntimeException
    AbsoluteScreenPixelRectangle   GetBoundingBoxOnScreen();
    /// @throws css::lang::DisposedException
    void                    EnsureIsAlive() const;

    virtual ~AccessibleIconChoiceCtrlEntry() override;
    /** this function is called upon disposing the component
    */
    virtual void                   disposing() override;

    // OAccessible
    virtual css::awt::Rectangle implGetBounds() override;

    // OCommonAccessibleText
    virtual OUString                        implGetText() override;
    virtual css::lang::Locale               implGetLocale() override;
    virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override final;

public:
    /** Ctor()
        @param  _rIconCtrl
            the icon control
        @param  _nPos
            the entry number
        @param  _xParent
            is our parent accessible object
    */
    AccessibleIconChoiceCtrlEntry( SvtIconChoiceCtrl& _rIconCtrl,
                                   sal_Int32 _nPos,
                                   const css::uno::Reference< css::accessibility::XAccessible >& _xParent );

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XEventListener
    virtual void disposing( const css::lang::EventObject& Source ) override;

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
    virtual css::awt::Point getLocationOnScreen(  ) override;
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
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
