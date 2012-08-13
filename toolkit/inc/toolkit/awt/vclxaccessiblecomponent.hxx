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

#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_
#define _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_

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

#include <tools/gen.hxx>    // Size
#include <tools/link.hxx>   // Size

class Window;
class VCLXWindow;
class VclSimpleEvent;
class VclWindowEvent;

namespace utl {
class AccessibleRelationSetHelper;
class AccessibleStateSetHelper;
}


//  ----------------------------------------------------
//  class VCLXAccessibleComponent
//  ----------------------------------------------------

typedef ::comphelper::OAccessibleExtendedComponentHelper    AccessibleExtendedComponentHelper_BASE;

typedef ::cppu::ImplHelper1<
    ::com::sun::star::lang::XServiceInfo > VCLXAccessibleComponent_BASE;

class VCLExternalSolarLock;

class TOOLKIT_DLLPUBLIC VCLXAccessibleComponent
        :public AccessibleExtendedComponentHelper_BASE
        ,public ::comphelper::OAccessibleImplementationAccess
        ,public VCLXAccessibleComponent_BASE
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow> mxWindow;
    VCLXWindow*                         mpVCLXindow;

    VCLExternalSolarLock*           m_pSolarLock;

protected:
     DECL_LINK( WindowEventListener, VclSimpleEvent* );
     DECL_LINK( WindowChildEventListener, VclSimpleEvent* );

    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void    ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void    FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet );
    virtual void    FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GetChildAccessible( const VclWindowEvent& rVclWindowEvent );

public:
    VCLXAccessibleComponent( VCLXWindow* pVCLXindow );
    ~VCLXAccessibleComponent();

    VCLXWindow*    GetVCLXWindow() const { return mpVCLXindow; }
    Window*        GetWindow() const;

    virtual void SAL_CALL disposing();

    // ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE()
    // ::com::sun::star::lang::XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::accessibility::XAccessibleContext
    sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::accessibility::XAccessibleComponent
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::accessibility::XAccessibleExtendedComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTitledBorderText(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getToolTipText(  ) throw (::com::sun::star::uno::RuntimeException);

protected:
    // base class overridables
    ::com::sun::star::awt::Rectangle SAL_CALL implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);

private:
    /** we may be reparented (if external components use OAccessibleImplementationAccess base class),
        so this method here returns the parent in the VCL world, in opposite to the parent
        an external component gave us
    @precond
        the caller must ensure thread safety, i.e. our mutex must be locked
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
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
    Reference< XAccessibleStateSet > getAccessibleStateSet() => overload FillAccessibleStateSet( ... )

---------------------------------------------------------- */


#endif // _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
