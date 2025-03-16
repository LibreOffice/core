/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include "AccessibleObject.hxx"
#include "PresenterTextView.hxx"

#include <com/sun/star/accessibility/AccessibleScrollType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <cppuhelper/compbase.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace sdext::presenter;

typedef ::cppu::ImplInheritanceHelper <
    AccessibleObject,
    css::accessibility::XAccessibleText
    > PresenterAccessibleParagraphInterfaceBase;

class AccessibleParagraph
    : public PresenterAccessibleParagraphInterfaceBase
{
public:
    AccessibleParagraph(
        const OUString& rsName,
        SharedPresenterTextParagraph pParagraph,
        const sal_Int32 nParagraphIndex);

    //----- XAccessibleContext ------------------------------------------------

    virtual css::uno::Reference<css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet() override;

    //----- XAccessibleText ---------------------------------------------------

    virtual sal_Int32 SAL_CALL getCaretPosition() override;

    virtual sal_Bool SAL_CALL setCaretPosition (sal_Int32 nIndex) override;

    virtual sal_Unicode SAL_CALL getCharacter (sal_Int32 nIndex) override;

    virtual css::uno::Sequence<css::beans::PropertyValue> SAL_CALL
        getCharacterAttributes (
            ::sal_Int32 nIndex,
            const css::uno::Sequence<OUString>& rRequestedAttributes) override;

    virtual css::awt::Rectangle SAL_CALL getCharacterBounds (sal_Int32 nIndex) override;

    virtual sal_Int32 SAL_CALL getCharacterCount() override;

    virtual sal_Int32 SAL_CALL getIndexAtPoint (const css::awt::Point& rPoint) override;

    virtual OUString SAL_CALL getSelectedText() override;

    virtual sal_Int32 SAL_CALL getSelectionStart() override;

    virtual sal_Int32 SAL_CALL getSelectionEnd() override;

    virtual sal_Bool SAL_CALL setSelection (sal_Int32 nStartIndex, sal_Int32 nEndIndex) override;

    virtual OUString SAL_CALL getText() override;

    virtual OUString SAL_CALL getTextRange (
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex) override;

    virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex (
        sal_Int32 nIndex,
        sal_Int16 nTextType) override;

    virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex (
        sal_Int32 nIndex,
        sal_Int16 nTextType) override;

    virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex (
        sal_Int32 nIndex,
        sal_Int16 nTextType) override;

    virtual sal_Bool SAL_CALL copyText (sal_Int32 nStartIndex, sal_Int32 nEndIndex) override;

    virtual sal_Bool SAL_CALL scrollSubstringTo(
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex,
        css::accessibility::AccessibleScrollType aScrollType) override;

protected:
    virtual awt::Point GetRelativeLocation() override;
    virtual awt::Size GetSize() override;
    virtual bool GetWindowState (const sal_Int64 nType) const override;

private:
    SharedPresenterTextParagraph mpParagraph;
    const sal_Int32 mnParagraphIndex;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
