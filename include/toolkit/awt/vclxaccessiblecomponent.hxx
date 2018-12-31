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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX

#include <toolkit/dllapi.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/accimplaccess.hxx>
#include <comphelper/accessiblecomponenthelper.hxx>


#include <tools/link.hxx>
#include <vcl/vclptr.hxx>

namespace com { namespace sun { namespace star { namespace accessibility { class XAccessible; } } } }

namespace vcl { class Window; }
class VCLXWindow;
class VclWindowEvent;

namespace utl {
class AccessibleRelationSetHelper;
class AccessibleStateSetHelper;
}


//  class VCLXAccessibleComponent

typedef ::cppu::ImplHelper1<
    css::lang::XServiceInfo > VCLXAccessibleComponent_BASE;

class TOOLKIT_DLLPUBLIC VCLXAccessibleComponent
        :public comphelper::OAccessibleExtendedComponentHelper
        ,public ::comphelper::OAccessibleImplementationAccess
        ,public VCLXAccessibleComponent_BASE
{
private:
    rtl::Reference<VCLXWindow>      m_xVCLXWindow;
    VclPtr<vcl::Window>             m_xEventSource;

    DECL_LINK( WindowEventListener, VclWindowEvent&, void );
    DECL_LINK( WindowChildEventListener, VclWindowEvent&, void );
    void            DisconnectEvents();

protected:
    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void    ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void    FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet );
    virtual void    FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    virtual css::uno::Reference< css::accessibility::XAccessible > GetChildAccessible( const VclWindowEvent& rVclWindowEvent );

public:
    VCLXAccessibleComponent( VCLXWindow* pVCLXWindow );
    virtual ~VCLXAccessibleComponent() override;

    VCLXWindow*    GetVCLXWindow() const;
    VclPtr<vcl::Window> GetWindow() const;
    template< class derived_type > VclPtr< derived_type > GetAs() const {
        return VclPtr< derived_type >( static_cast< derived_type * >( GetWindow().get() ) ); }
    template< class derived_type > VclPtr< derived_type > GetAsDynamic() const {
        return VclPtr< derived_type >( dynamic_cast< derived_type * >( GetWindow().get() ) ); }

    virtual void SAL_CALL disposing() override;

    // css::uno::XInterface
    DECLARE_XINTERFACE()
    // css::lang::XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // css::accessibility::XAccessibleContext
    sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
    sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    OUString SAL_CALL getAccessibleDescription(  ) override;
    OUString SAL_CALL getAccessibleName(  ) override;
    css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;
    css::lang::Locale SAL_CALL getLocale(  ) override;

    // css::accessibility::XAccessibleComponent
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
    void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // css::accessibility::XAccessibleExtendedComponent
    virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont(  ) override;
    virtual OUString SAL_CALL getTitledBorderText(  ) override;
    virtual OUString SAL_CALL getToolTipText(  ) override;

protected:
    // base class overridables
    css::awt::Rectangle implGetBounds(  ) override;

private:
    /** we may be reparented (if external components use OAccessibleImplementationAccess base class),
        so this method here returns the parent in the VCL world, in opposite to the parent
        an external component gave us
    @precond
        the caller must ensure thread safety, i.e. our mutex must be locked
    */
    css::uno::Reference< css::accessibility::XAccessible >
            getVclParent() const;
};

/* ----------------------------------------------------------
  Accessibility only for the Window hierarchy!
  Maybe derived classes must overwrite these Accessibility interfaces:

    // XAccessibleContext:
    sal_Int16 getAccessibleRole() => VCL Window::GetAccessibleRole()
    OUString getAccessibleDescription() => VCL Window::GetAccessibleDescription
    OUString getAccessibleName() => VCL Window::GetAccessibleText() => Most windows return Window::GetText()
    Reference< XAccessibleRelationSet > getAccessibleRelationSet()
    Reference< XAccessibleStateSet > getAccessibleStateSet() => override FillAccessibleStateSet( ... )

---------------------------------------------------------- */


#endif // INCLUDED_TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
