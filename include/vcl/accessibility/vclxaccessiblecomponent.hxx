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

#include <vcl/dllapi.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/accessiblecomponenthelper.hxx>


#include <tools/link.hxx>
#include <vcl/vclptr.hxx>

namespace com::sun::star::accessibility { class XAccessible; }

namespace vcl { class Window; }
class VclWindowEvent;

namespace utl {
class AccessibleRelationSetHelper;
}



class VCL_DLLPUBLIC VCLXAccessibleComponent
        :public cppu::ImplInheritanceHelper<
             comphelper::OAccessibleComponentHelper, css::lang::XServiceInfo>
{
private:
    VclPtr<vcl::Window> m_xWindow;

    DECL_DLLPRIVATE_LINK( WindowEventListener, VclWindowEvent&, void );
    DECL_DLLPRIVATE_LINK( WindowChildEventListener, VclWindowEvent&, void );
    void            DisconnectEvents();

protected:
    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void    ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void    FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet );
    virtual void    FillAccessibleStateSet( sal_Int64& rStateSet );

    virtual css::uno::Reference< css::accessibility::XAccessible > GetChildAccessible( const VclWindowEvent& rVclWindowEvent );

public:
    VCLXAccessibleComponent(vcl::Window* pWindow);
    virtual ~VCLXAccessibleComponent() override;

    vcl::Window* GetWindow() const;
    template< class derived_type > derived_type* GetAs() const {
        return static_cast< derived_type * >( GetWindow() ); }
    template< class derived_type > derived_type* GetAsDynamic() const {
        return dynamic_cast< derived_type * >( GetWindow() ); }

    virtual void SAL_CALL disposing() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // css::accessibility::XAccessibleContext
    sal_Int64 SAL_CALL getAccessibleChildCount(  ) override;
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    OUString SAL_CALL getAccessibleDescription(  ) override;
    OUString SAL_CALL getAccessibleName(  ) override;
    OUString SAL_CALL getAccessibleId(  ) override;
    css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    sal_Int64 SAL_CALL getAccessibleStateSet(  ) override;
    css::lang::Locale SAL_CALL getLocale(  ) override;

    // css::accessibility::XAccessibleComponent
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
    void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // css::accessibility::XAccessibleExtendedComponent
    virtual OUString SAL_CALL getTitledBorderText(  ) override;
    virtual OUString SAL_CALL getToolTipText(  ) override;

protected:
    // base class overridables
    css::awt::Rectangle implGetBounds(  ) override;
};

/* ----------------------------------------------------------
  Accessibility only for the Window hierarchy!
  Maybe derived classes must overwrite these Accessibility interfaces:

    // XAccessibleContext:
    sal_Int16 getAccessibleRole() => VCL Window::GetAccessibleRole()
    OUString getAccessibleDescription() => VCL Window::GetAccessibleDescription
    OUString getAccessibleName() => VCL Window::GetAccessibleText() => Most windows return Window::GetText()
    OUString getAccessibleId() => VCL Window::get_id()
    Reference< XAccessibleRelationSet > getAccessibleRelationSet()
    sal_Int64 getAccessibleStateSet() => override FillAccessibleStateSet( ... )

---------------------------------------------------------- */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
