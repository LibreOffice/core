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

#ifndef INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLESTATUSBARITEM_HXX
#define INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLESTATUSBARITEM_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/accessibletexthelper.hxx>
#include <cppuhelper/implbase2.hxx>
#include <vcl/vclptr.hxx>

class StatusBar;

namespace utl {
class AccessibleStateSetHelper;
}


//  class VCLXAccessibleStatusBarItem


typedef ::comphelper::OAccessibleTextHelper AccessibleTextHelper_BASE;

typedef ::cppu::ImplHelper2<
    css::accessibility::XAccessible,
    css::lang::XServiceInfo > VCLXAccessibleStatusBarItem_BASE;

class VCLXAccessibleStatusBarItem : public AccessibleTextHelper_BASE,
                                    public VCLXAccessibleStatusBarItem_BASE
{
    friend class VCLXAccessibleStatusBar;

private:
    VclPtr<StatusBar>       m_pStatusBar;
    sal_uInt16              m_nItemId;
    OUString                m_sItemName;
    OUString                m_sItemText;
    bool                    m_bShowing;

protected:
    bool                    IsShowing();
    void                    SetShowing( bool bShowing );
    void                    SetItemName( const OUString& sItemName );
    OUString                GetItemName();
    void                    SetItemText( const OUString& sItemText );
    OUString                GetItemText();
    sal_uInt16              GetItemId() const { return m_nItemId; }

    void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    // OCommonAccessibleComponent
    virtual css::awt::Rectangle implGetBounds(  ) override;

    // OCommonAccessibleText
    virtual OUString                        implGetText() override;
    virtual css::lang::Locale  implGetLocale() override;
    virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;

    // XComponent
    virtual void SAL_CALL   disposing() override;

public:
    VCLXAccessibleStatusBarItem( StatusBar* pStatusBar, sal_uInt16 nItemId );
    virtual ~VCLXAccessibleStatusBarItem() override;

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

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
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleExtendedComponent
    virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont(  ) override;
    virtual OUString SAL_CALL getTitledBorderText(  ) override;
    virtual OUString SAL_CALL getToolTipText(  ) override;

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition() override;
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
};

#endif // INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLESTATUSBARITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
