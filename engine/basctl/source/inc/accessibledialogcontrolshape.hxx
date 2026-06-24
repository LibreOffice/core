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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/OAccessible.hxx>
#include <cppuhelper/implbase.hxx>
#include <vcl/vclptr.hxx>

namespace vcl { class Window; }

namespace basctl
{

class DlgEdObj;
class DialogWindow;

class AccessibleDialogControlShape final
    : public cppu::ImplInheritanceHelper<comphelper::OAccessible, css::lang::XServiceInfo,
                                         css::beans::XPropertyChangeListener>
{
    friend class AccessibleDialogWindow;

private:
    VclPtr<DialogWindow>    m_pDialogWindow;
    DlgEdObj*               m_pDlgEdObj;
    bool                    m_bFocused;
    bool                    m_bSelected;

    css::awt::Rectangle                                            m_aBounds;
    css::uno::Reference< css::beans::XPropertySet >   m_xControlModel;

    bool                    IsFocused() const;
    bool                    IsSelected() const;

    void                    SetFocused (bool bFocused);
    void                    SetSelected (bool bSelected);

    css::awt::Rectangle     GetBounds() const;
    void                    SetBounds( const css::awt::Rectangle& aBounds );

    vcl::Window*            GetWindow() const;

    OUString                GetModelStringProperty( OUString const & pPropertyName );

    void                    FillAccessibleStateSet( sal_Int64& rStateSet );

    // OAccessible
    virtual css::awt::Rectangle implGetBounds() override;

    // XComponent
    virtual void   disposing() override;

public:
    AccessibleDialogControlShape (DialogWindow*, DlgEdObj*);
    virtual ~AccessibleDialogControlShape() override;

    // XEventListener
    virtual void disposing( const css::lang::EventObject& rSource ) override;

    // XPropertyChangeListener
    virtual void propertyChange( const css::beans::PropertyChangeEvent& rEvent ) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& rServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XAccessibleContext
    virtual sal_Int64 getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleParent(  ) override;
    virtual sal_Int16 getAccessibleRole(  ) override;
    virtual OUString getAccessibleDescription(  ) override;
    virtual OUString getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > getAccessibleRelationSet(  ) override;
    virtual sal_Int64 getAccessibleStateSet(  ) override;
    virtual css::lang::Locale getLocale(  ) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void grabFocus(  ) override;
    virtual sal_Int32 getForeground(  ) override;
    virtual sal_Int32 getBackground(  ) override;

    // XAccessibleExtendedComponent
    virtual OUString getToolTipText(  ) override;
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
