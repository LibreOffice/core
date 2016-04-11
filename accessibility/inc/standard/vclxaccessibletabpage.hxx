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

#ifndef INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLETABPAGE_HXX
#define INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLETABPAGE_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/accessibletexthelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <vcl/vclptr.hxx>


class TabControl;
class VCLExternalSolarLock;

namespace utl {
class AccessibleStateSetHelper;
}


//  class VCLXAccessibleTabPage


typedef ::comphelper::OAccessibleTextHelper AccessibleTextHelper_BASE;

typedef ::cppu::ImplHelper<
    css::accessibility::XAccessible,
    css::lang::XServiceInfo > VCLXAccessibleTabPage_BASE;

class VCLXAccessibleTabPage :   public AccessibleTextHelper_BASE,
                                public VCLXAccessibleTabPage_BASE
{
    friend class VCLXAccessibleTabControl;

private:
    VCLExternalSolarLock*   m_pExternalLock;
    VclPtr<TabControl>      m_pTabControl;
    sal_uInt16              m_nPageId;
    bool                    m_bFocused;
    bool                    m_bSelected;
    OUString                m_sPageText;

protected:
    bool                    IsFocused();
    bool                    IsSelected();

    void                    SetFocused( bool bFocused );
    void                    SetSelected( bool bSelected );
    void                    SetPageText( const OUString& sPageText );
    OUString                GetPageText();

    void                    Update( bool bNew );

    sal_uInt16              GetPageId() const { return m_nPageId; }

    void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    // OCommonAccessibleComponent
    virtual css::awt::Rectangle implGetBounds(  ) throw (css::uno::RuntimeException) override;

    // OCommonAccessibleText
    virtual OUString                        implGetText() override;
    virtual css::lang::Locale  implGetLocale() override;
    virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;

    // XComponent
    virtual void SAL_CALL   disposing() override;

public:
    VCLXAccessibleTabPage( TabControl* pTabControl, sal_uInt16 nPageId );
    virtual ~VCLXAccessibleTabPage() override;

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) throw (css::accessibility::IllegalAccessibleComponentStateException, css::uno::RuntimeException, std::exception) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleExtendedComponent
    virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTitledBorderText(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getToolTipText(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLETABPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
