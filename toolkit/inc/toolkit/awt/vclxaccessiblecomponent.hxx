/*************************************************************************
 *
 *  $RCSfile: vclxaccessiblecomponent.hxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:02:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_
#define _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleContext.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEXTENDEDCOMPONENT_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef COMPHELPER_ACCIMPLACCESS_HXX
#include <comphelper/accimplaccess.hxx>
#endif
#ifndef COMPHELPER_ACCESSIBLE_COMPONENT_HELPER_HXX
#include <comphelper/accessiblecomponenthelper.hxx>
#endif

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
class VCLXAccessibleComponent
        :public AccessibleExtendedComponentHelper_BASE
        ,public ::comphelper::OAccessibleImplementationAccess
        ,public VCLXAccessibleComponent_BASE
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow> mxWindow;
    VCLXWindow*                         mpVCLXindow;

    ULONG                           nDummy1;
    ULONG                           nDummy2;
    void*                           pDummy1;
    VCLExternalSolarLock*           m_pSolarLock;

protected:
     DECL_LINK( WindowEventListener, VclSimpleEvent* );
     DECL_LINK( WindowChildEventListener, VclSimpleEvent* );

    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void    ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void    FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet );
    virtual void    FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > GetChildAccessible( const VclWindowEvent& rVclWindowEvent );

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

    // ::drafts::com::sun::star::accessibility::XAccessibleContext
    sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

    // ::drafts::com::sun::star::accessibility::XAccessibleComponent
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAt( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::drafts::com::sun::star::accessibility::XAccessibleExtendedComponent
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
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >
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

