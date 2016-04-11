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
#ifndef INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLETOOLBOXITEM_HXX
#define INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLETOOLBOXITEM_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <vcl/vclptr.hxx>

class ToolBox;

typedef ::comphelper::OAccessibleTextHelper AccessibleTextHelper_BASE;
typedef ::cppu::ImplHelper  < css::accessibility::XAccessible,
                              css::accessibility::XAccessibleAction,
                              css::accessibility::XAccessibleValue,
                              css::lang::XServiceInfo > VCLXAccessibleToolBoxItem_BASE;

class VCLExternalSolarLock;

class VCLXAccessibleToolBoxItem : public AccessibleTextHelper_BASE,
                                  public VCLXAccessibleToolBoxItem_BASE
{
private:
    OUString                m_sOldName;
    VclPtr<ToolBox>         m_pToolBox;
    VCLExternalSolarLock*   m_pExternalLock;
    sal_Int32               m_nIndexInParent;
    sal_Int16               m_nRole;
    sal_uInt16              m_nItemId;
    bool                    m_bHasFocus;
    bool                    m_bIsChecked;
    bool                    m_bIndeterminate;

    css::uno::Reference< css::accessibility::XAccessible >    m_xChild;

public:
    inline sal_Int32    getIndexInParent() const                    { return m_nIndexInParent; }
    inline void         setIndexInParent( sal_Int32 _nNewIndex )    { m_nIndexInParent = _nNewIndex; }

protected:
    virtual ~VCLXAccessibleToolBoxItem() override;

    virtual void SAL_CALL                   disposing() override;

    /// implements the calculation of the bounding rectangle
    virtual css::awt::Rectangle implGetBounds(  ) throw (css::uno::RuntimeException) override;

    // OCommonAccessibleText
    virtual OUString                                    implGetText() override;
    virtual css::lang::Locale                           implGetLocale() override;
    virtual void                                        implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex ) override;

    OUString GetText();

public:
    VCLXAccessibleToolBoxItem( ToolBox* _pToolBox, sal_Int32 _nPos );

    void                SetFocus( bool _bFocus );
    inline bool         HasFocus() const { return m_bHasFocus; }
    void                SetChecked( bool _bCheck );
    void                SetIndeterminate( bool _bIndeterminate );
    inline void         ReleaseToolBox() { m_pToolBox = nullptr; }
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

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleExtendedComponent
    virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTitledBorderText(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getToolTipText(  ) throw (css::uno::RuntimeException, std::exception) override;

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

#endif // INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLETOOLBOXITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
