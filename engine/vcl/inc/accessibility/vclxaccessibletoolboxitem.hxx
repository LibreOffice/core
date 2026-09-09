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
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/vclptr.hxx>

class VCLXAccessibleToolBoxItem final
    : public cppu::ImplInheritanceHelper<
          comphelper::OAccessibleTextHelper, css::accessibility::XAccessibleAction,
          css::accessibility::XAccessibleValue, css::lang::XServiceInfo>
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

    rtl::Reference<comphelper::OAccessible> m_pChild;

public:
    sal_Int32    getIndexInParent() const                    { return m_nIndexInParent; }
    void         setIndexInParent( sal_Int32 _nNewIndex )    { m_nIndexInParent = _nNewIndex; }

private:
    OUString implGetAccessibleName();

    virtual ~VCLXAccessibleToolBoxItem() override;

    virtual void                   disposing() override;

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
    void SetChild(const rtl::Reference<comphelper::OAccessible>& rpChild);
    const rtl::Reference<comphelper::OAccessible>& GetChild() const { return m_pChild; }
    void                NotifyChildEvent( const css::uno::Reference< css::accessibility::XAccessible >& _xChild, bool _bShow );

    void                ToggleEnableState();

    // XInterface
    cpo::uno::Any queryInterface(cpo::uno::Type const & aType) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& rServiceName ) override;
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

    // XAccessibleText
    virtual OUString getText() override;
    virtual sal_Unicode getCharacter( sal_Int32 nIndex ) override;
    virtual sal_Int32 getCharacterCount() override;
    virtual sal_Int32 getCaretPosition() override;
    virtual bool setCaretPosition( sal_Int32 nIndex ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyValue > getCharacterAttributes( sal_Int32 nIndex, const cpo::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual bool setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual bool copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual OUString getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual bool scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void grabFocus(  ) override;
    virtual sal_Int32 getForeground(  ) override;
    virtual sal_Int32 getBackground(  ) override;

    // XAccessibleExtendedComponent
    virtual OUString getTitledBorderText(  ) override;
    virtual OUString getToolTipText(  ) override;

    // XAccessibleAction
    virtual sal_Int32 getAccessibleActionCount( ) override;
    virtual bool doAccessibleAction ( sal_Int32 nIndex ) override;
    virtual OUString getAccessibleActionDescription ( sal_Int32 nIndex ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > getAccessibleActionKeyBinding( sal_Int32 nIndex ) override;

    // XAccessibleValue
    virtual cpo::uno::Any getCurrentValue(  ) override;
    virtual bool setCurrentValue( const cpo::uno::Any& aNumber ) override;
    virtual cpo::uno::Any getMaximumValue(  ) override;
    virtual cpo::uno::Any getMinimumValue(  ) override;
    virtual cpo::uno::Any getMinimumIncrement(  ) override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
