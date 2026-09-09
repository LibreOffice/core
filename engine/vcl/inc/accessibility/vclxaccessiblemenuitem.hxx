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

#include <accessibility/accessiblemenuitemcomponent.hxx>

#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>

#include <cppuhelper/implbase.hxx>
#include <comphelper/accessibletexthelper.hxx>




class VCLXAccessibleMenuItem :  public cppu::ImplInheritanceHelper<
                                    OAccessibleMenuItemComponent,
                                    css::accessibility::XAccessibleText,
                                    css::accessibility::XAccessibleAction,
                                    css::accessibility::XAccessibleValue>,
                                public ::comphelper::OCommonAccessibleText
{
protected:
    virtual bool            IsFocused() override;
    virtual bool            IsSelected() override;
    bool                    IsCheckable();
    virtual bool            IsChecked() override;

    virtual bool            IsHighlighted() override;

    virtual void            FillAccessibleStateSet( sal_Int64& rStateSet ) override;

    // OCommonAccessibleText
    virtual OUString                     implGetText() final override;
    virtual css::lang::Locale            implGetLocale() override;
    virtual void                         implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override final;

public:
    VCLXAccessibleMenuItem( Menu* pParent, sal_uInt16 nItemPos, Menu* pMenu = nullptr );

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XAccessibleContext
    virtual sal_Int16 getAccessibleRole(  ) override;

    // XAccessibleText
    virtual sal_Int32 getCaretPosition() override;
    virtual bool setCaretPosition( sal_Int32 nIndex ) override;
    virtual sal_Unicode getCharacter( sal_Int32 nIndex ) final override;
    virtual cpo::uno::Sequence< css::beans::PropertyValue > getCharacterAttributes( sal_Int32 nIndex, const cpo::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 getCharacterCount() final override;
    virtual sal_Int32 getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual OUString getSelectedText() override final;
    virtual sal_Int32 getSelectionStart() override final;
    virtual sal_Int32 getSelectionEnd() override final;
    virtual bool setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual OUString getText() final override;
    virtual OUString getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual css::accessibility::TextSegment getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual bool copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual bool scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override;

    // XAccessibleAction
    virtual sal_Int32 getAccessibleActionCount( ) final override;
    virtual bool doAccessibleAction ( sal_Int32 nIndex ) override;
    virtual OUString getAccessibleActionDescription ( sal_Int32 nIndex ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > getAccessibleActionKeyBinding( sal_Int32 nIndex ) override;

    // XAccessibleValue
    virtual cpo::uno::Any getCurrentValue(  ) override;
    virtual bool setCurrentValue( const cpo::uno::Any& aNumber ) override;
    virtual cpo::uno::Any getMaximumValue(  ) override;
    virtual cpo::uno::Any getMinimumValue(  ) override;
    virtual cpo::uno::Any getMinimumIncrement( ) override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
