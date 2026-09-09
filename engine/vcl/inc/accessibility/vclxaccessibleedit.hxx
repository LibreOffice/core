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
    virtual OUString getImplementationName() override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XAccessibleContext
    virtual sal_Int64 getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( sal_Int64 i ) override;
    virtual OUString getAccessibleName() override;
    virtual sal_Int16 getAccessibleRole(  ) override;

    // XAccessibleAction
    virtual sal_Int32 getAccessibleActionCount( ) override;
    virtual bool doAccessibleAction ( sal_Int32 nIndex ) override;
    virtual OUString getAccessibleActionDescription ( sal_Int32 nIndex ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > getAccessibleActionKeyBinding( sal_Int32 nIndex ) override;

    // XAccessibleText
    virtual sal_Int32 getCaretPosition(  ) override;
    virtual bool setCaretPosition( sal_Int32 nIndex ) override;
    virtual sal_Unicode getCharacter( sal_Int32 nIndex ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyValue > getCharacterAttributes( sal_Int32 nIndex, const cpo::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 getCharacterCount(  ) override;
    virtual sal_Int32 getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual OUString getSelectedText(  ) override;
    virtual sal_Int32 getSelectionStart(  ) override;
    virtual sal_Int32 getSelectionEnd(  ) override;
    virtual bool setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual OUString getText(  ) override;
    virtual OUString getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual css::accessibility::TextSegment getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual bool copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual bool scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override;

    // XAccessibleEditableText
    virtual bool cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual bool pasteText( sal_Int32 nIndex ) override;
    virtual bool deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual bool insertText( const OUString& sText, sal_Int32 nIndex ) override;
    virtual bool replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const OUString& sReplacement ) override;
    virtual bool setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const cpo::uno::Sequence< css::beans::PropertyValue >& aAttributeSet ) override;
    virtual bool setText( const OUString& sText ) override;

private:
    bool isComboBoxChild();
    bool isEditable();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
