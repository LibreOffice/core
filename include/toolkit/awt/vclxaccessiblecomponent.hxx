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
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/accimplaccess.hxx>
#include <comphelper/accessiblecomponenthelper.hxx>

#include <tools/link.hxx>
#include <vcl/vclptr.hxx>

namespace vcl { class Window; }
class VCLXWindow;
class VclSimpleEvent;
class VclWindowEvent;

namespace utl {
class AccessibleRelationSetHelper;
class AccessibleStateSetHelper;
}



//  class VCLXAccessibleComponent


typedef ::comphelper::OAccessibleExtendedComponentHelper    AccessibleExtendedComponentHelper_BASE;

typedef ::cppu::ImplHelper1<
    css::lang::XServiceInfo > VCLXAccessibleComponent_BASE;

class VCLExternalSolarLock;

class TOOLKIT_DLLPUBLIC VCLXAccessibleComponent
        :public AccessibleExtendedComponentHelper_BASE
        ,public ::comphelper::OAccessibleImplementationAccess
        ,public VCLXAccessibleComponent_BASE
{
private:
    css::uno::Reference< css::awt::XWindow> mxWindow;
    VCLXWindow*                         mpVCLXindow;

    VCLExternalSolarLock*           m_pSolarLock;

protected:
     DECL_LINK_TYPED( WindowEventListener, VclWindowEvent&, void );
     DECL_LINK_TYPED( WindowChildEventListener, VclWindowEvent&, void );

    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void    ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void    FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet );
    virtual void    FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    virtual css::uno::Reference< css::accessibility::XAccessible > GetChildAccessible( const VclWindowEvent& rVclWindowEvent );

public:
    VCLXAccessibleComponent( VCLXWindow* pVCLXindow );
    virtual ~VCLXAccessibleComponent();

    VCLXWindow*    GetVCLXWindow() const { return mpVCLXindow; }
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
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // css::accessibility::XAccessibleContext
    sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    css::lang::Locale SAL_CALL getLocale(  ) throw (css::accessibility::IllegalAccessibleComponentStateException, css::uno::RuntimeException, std::exception) override;

    // css::accessibility::XAccessibleComponent
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    css::awt::Point SAL_CALL getLocationOnScreen(  ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL grabFocus(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

    // css::accessibility::XAccessibleExtendedComponent
    virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTitledBorderText(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getToolTipText(  ) throw (css::uno::RuntimeException, std::exception) override;

protected:
    // base class overridables
    css::awt::Rectangle implGetBounds(  ) throw (css::uno::RuntimeException) override;

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
