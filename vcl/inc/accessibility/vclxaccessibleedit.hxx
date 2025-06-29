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

#include <accessibility/vclxaccessibletextcomponent.hxx>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>

#include <cppuhelper/implbase.hxx>
#include <vcl/toolkit/edit.hxx>

class VCLXAccessibleEdit
    : public cppu::ImplInheritanceHelper<VCLXAccessibleTextComponent,
                                         css::accessibility::XAccessibleAction,
                                         css::accessibility::XAccessibleEditableText>
{
    friend class VCLXAccessibleBox;

private:
    sal_Int32   m_nCaretPosition;

protected:
    virtual ~VCLXAccessibleEdit() override = default;

    virtual void                ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual void                FillAccessibleStateSet( sal_Int64& rStateSet ) override;
    sal_Int16                   implGetAccessibleRole();

    // OCommonAccessibleText
    virtual OUString            implGetText() override;
    virtual void                implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;

    // VCLXAccessibleTextComponent
    virtual bool                PreferFullTextInTextChangedEvent() override;

public:
    VCLXAccessibleEdit(Edit* pEdit);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
    virtual OUString SAL_CALL getAccessibleName() override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) override;
    virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) override;
    virtual OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) override;

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition(  ) override;
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) override;
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 SAL_CALL getCharacterCount(  ) override;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual OUString SAL_CALL getSelectedText(  ) override;
    virtual sal_Int32 SAL_CALL getSelectionStart(  ) override;
    virtual sal_Int32 SAL_CALL getSelectionEnd(  ) override;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual OUString SAL_CALL getText(  ) override;
    virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override;

    // XAccessibleEditableText
    virtual sal_Bool SAL_CALL cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL pasteText( sal_Int32 nIndex ) override;
    virtual sal_Bool SAL_CALL deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL insertText( const OUString& sText, sal_Int32 nIndex ) override;
    virtual sal_Bool SAL_CALL replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const OUString& sReplacement ) override;
    virtual sal_Bool SAL_CALL setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const css::uno::Sequence< css::beans::PropertyValue >& aAttributeSet ) override;
    virtual sal_Bool SAL_CALL setText( const OUString& sText ) override;

private:
    bool isComboBoxChild();
    bool isEditable();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
