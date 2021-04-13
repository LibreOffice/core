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
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/implbase.hxx>
#include <svl/SfxBroadcaster.hxx>

#include <editeng/editeng.hxx>
#include <editeng/unoedsrc.hxx>
#include <edit.hxx>
#include <view.hxx>

class SmDocShell;

namespace accessibility { class AccessibleTextHelper; }

// classes and helper-classes used for accessibility in the graphic-window


typedef
cppu::WeakImplHelper
    <
        css::lang::XServiceInfo,
        css::accessibility::XAccessible,
        css::accessibility::XAccessibleComponent,
        css::accessibility::XAccessibleContext,
        css::accessibility::XAccessibleText,
        css::accessibility::XAccessibleEventBroadcaster
    >
SmGraphicAccessibleBaseClass;

class SmGraphicAccessible final :
    public SmGraphicAccessibleBaseClass
{
    OUString                            aAccName;
    /// client id in the AccessibleEventNotifier queue
    sal_uInt32                          nClientId;

    VclPtr<SmGraphicWindow>             pWin;

    SmGraphicAccessible( const SmGraphicAccessible & ) = delete;
    SmGraphicAccessible & operator = ( const SmGraphicAccessible & ) = delete;

    SmDocShell *    GetDoc_Impl();
    OUString        GetAccessibleText_Impl();

public:
    explicit SmGraphicAccessible( SmGraphicWindow *pGraphicWin );
    virtual ~SmGraphicAccessible() override;

    void                ClearWin();     // to be called when view is destroyed
    void                LaunchEvent(
                            const sal_Int16 nAccessibleEventId,
                            const css::uno::Any &rOldVal,
                            const css::uno::Any &rNewVal);

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
    virtual css::awt::Size SAL_CALL getSize(  ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;
    virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition(  ) override;
    virtual sal_Bool SAL_CALL setCaretPosition ( sal_Int32 nIndex ) override;
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

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
