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
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase4.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/vclptr.hxx>


typedef ::comphelper::OAccessibleTextHelper AccessibleTextHelper_BASE;
typedef ::cppu::ImplHelper4 < css::accessibility::XAccessible,
                              css::accessibility::XAccessibleAction,
                              css::accessibility::XAccessibleValue,
                              css::lang::XServiceInfo > VCLXAccessibleToolBoxItem_BASE;

class VCLXAccessibleToolBoxItem final : public AccessibleTextHelper_BASE,
                                        public VCLXAccessibleToolBoxItem_BASE
{
private:
    OUString                m_sOldName;
    VclPtr<ToolBox>         m_pToolBox;
    sal_Int32               m_nIndexInParent;
    sal_Int16               m_nRole;
    ToolBoxItemId           m_nItemId;
    bool                    m_bHasFocus;
    bool                    m_bIsChecked;
    bool                    m_bIndeterminate;

    css::uno::Reference< css::accessibility::XAccessible >    m_xChild;
    OUString GetText();

public:
    sal_Int32    getIndexInParent() const                    { return m_nIndexInParent; }
    void         setIndexInParent( sal_Int32 _nNewIndex )    { m_nIndexInParent = _nNewIndex; }

private:
    virtual ~VCLXAccessibleToolBoxItem() override;

    virtual void SAL_CALL                   disposing() override;

    /// implements the calculation of the bounding rectangle
    virtual css::awt::Rectangle implGetBounds(  ) override;

    // OCommonAccessibleText
    virtual OUString                                    implGetText() override;
    virtual css::lang::Locale                           implGetLocale() override;
    virtual void                                        implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;

public:
    VCLXAccessibleToolBoxItem( ToolBox* _pToolBox, sal_Int32 _nPos );

    void                SetFocus( bool _bFocus );
    bool         HasFocus() const { return m_bHasFocus; }
    void                SetChecked( bool _bCheck );
    void                SetIndeterminate( bool _bIndeterminate );
    void         ReleaseToolBox() { m_pToolBox = nullptr; }
    void                NameChanged();
    void                SetChild( const css::uno::Reference< css::accessibility::XAccessible >& _xChild );
    const css::uno::Reference< css::accessibility::XAccessible >&
                        GetChild() const { return m_xChild; }
    void                NotifyChildEvent( const css::uno::Reference< css::accessibility::XAccessible >& _xChild, bool _bShow );

    void                ToggleEnableState();

    // XInterface
    DECLARE_XINTERFACE( )
    DECLARE_XTYPEPROVIDER( )

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

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

    // XAccessibleText
    virtual OUString SAL_CALL getText() override;
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) override;
    virtual sal_Int32 SAL_CALL getCharacterCount() override;
    virtual sal_Int32 SAL_CALL getCaretPosition() override;
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleExtendedComponent
    virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont(  ) override;
    virtual OUString SAL_CALL getTitledBorderText(  ) override;
    virtual OUString SAL_CALL getToolTipText(  ) override;

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) override;
    virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) override;
    virtual OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) override;

    // XAccessibleValue
    virtual css::uno::Any SAL_CALL getCurrentValue(  ) override;
    virtual sal_Bool SAL_CALL setCurrentValue( const css::uno::Any& aNumber ) override;
    virtual css::uno::Any SAL_CALL getMaximumValue(  ) override;
    virtual css::uno::Any SAL_CALL getMinimumValue(  ) override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
