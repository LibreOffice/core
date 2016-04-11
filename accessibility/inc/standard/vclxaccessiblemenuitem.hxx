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

#ifndef INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLEMENUITEM_HXX
#define INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLEMENUITEM_HXX

#include <standard/accessiblemenuitemcomponent.hxx>

#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>

#include <cppuhelper/implbase.hxx>
#include <comphelper/accessibletexthelper.hxx>


//  class VCLXAccessibleMenuItem


typedef ::cppu::ImplHelper<
    css::accessibility::XAccessibleText,
    css::accessibility::XAccessibleAction,
    css::accessibility::XAccessibleValue > VCLXAccessibleMenuItem_BASE;

class VCLXAccessibleMenuItem :  public OAccessibleMenuItemComponent,
                                public ::comphelper::OCommonAccessibleText,
                                public VCLXAccessibleMenuItem_BASE
{
protected:
    virtual bool            IsFocused() override;
    virtual bool            IsSelected() override;
    virtual bool            IsChecked() override;

    virtual bool            IsHighlighted() override;

    virtual void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet ) override;

    // OCommonAccessibleText
    virtual OUString                     implGetText() override;
    virtual css::lang::Locale            implGetLocale() override;
    virtual void                         implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;

public:
    VCLXAccessibleMenuItem( Menu* pParent, sal_uInt16 nItemPos, Menu* pMenu = nullptr );
    virtual ~VCLXAccessibleMenuItem() override;

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getCharacterCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getSelectedText() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getSelectionStart() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getSelectionEnd() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    // XAccessibleValue
    virtual css::uno::Any SAL_CALL getCurrentValue(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setCurrentValue( const css::uno::Any& aNumber ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getMaximumValue(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getMinimumValue(  ) throw (css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLEMENUITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
