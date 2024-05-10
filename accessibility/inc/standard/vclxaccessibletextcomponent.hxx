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

#include <com/sun/star/accessibility/AccessibleScrollType.hpp>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <cppuhelper/implbase.hxx>




class VCLXAccessibleTextComponent : public cppu::ImplInheritanceHelper<
                                        VCLXAccessibleComponent,
                                        css::accessibility::XAccessibleText>,
                                    public ::comphelper::OCommonAccessibleText
{
    OUString                                m_sText;

    // accessible name the object had when SetText was called last time
    OUString m_sOldName;

protected:
    void                                    SetText( const OUString& sText );

    // Whether text segments for old/new value in AccessibleEventId::TEXT_CHANGED
    // event should always include the whole old and new text instead of just
    // the characters that changed between the two
    virtual bool                            PreferFullTextInTextChangedEvent() { return false; };

    virtual void                            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

    // OCommonAccessibleText
    virtual OUString                        implGetText() override;
    virtual css::lang::Locale               implGetLocale() override;
    virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;

    // XComponent
    virtual void SAL_CALL                   disposing() override;

public:
    VCLXAccessibleTextComponent( VCLXWindow* pVCLXWindow );

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
};



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
