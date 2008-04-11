/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessiblemenubasecomponent.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef ACCESSIBILITY_STANDARD_ACCESSIBLEMENUBASECOMPONENT_HXX
#define ACCESSIBILITY_STANDARD_ACCESSIBLEMENUBASECOMPONENT_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <comphelper/accessiblecomponenthelper.hxx>
#ifndef _CPPUHELPER_IMPLBASE2_HXX
#include <cppuhelper/implbase2.hxx>
#endif
#include <tools/link.hxx>

#include <vector>

class Menu;
class VclSimpleEvent;
class VclMenuEvent;
class VCLExternalSolarLock;

namespace utl {
class AccessibleStateSetHelper;
}

//  ----------------------------------------------------
//  class OAccessibleMenuBaseComponent
//  ----------------------------------------------------

typedef ::comphelper::OAccessibleExtendedComponentHelper    AccessibleExtendedComponentHelper_BASE;

typedef ::cppu::ImplHelper2<
    ::com::sun::star::accessibility::XAccessible,
    ::com::sun::star::lang::XServiceInfo > OAccessibleMenuBaseComponent_BASE;

class OAccessibleMenuBaseComponent : public AccessibleExtendedComponentHelper_BASE,
                                     public OAccessibleMenuBaseComponent_BASE
{
    friend class OAccessibleMenuItemComponent;
    friend class VCLXAccessibleMenuItem;
    friend class VCLXAccessibleMenu;

private:
    VCLExternalSolarLock*   m_pExternalLock;

protected:
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > > AccessibleChildren;

    AccessibleChildren      m_aAccessibleChildren;
    Menu*                   m_pMenu;

    sal_Bool                m_bEnabled;
    sal_Bool                m_bFocused;
    sal_Bool                m_bVisible;
    sal_Bool                m_bSelected;
    sal_Bool                m_bChecked;

    Menu*                   GetMenu() { return m_pMenu; }

    virtual sal_Bool        IsEnabled();
    virtual sal_Bool        IsFocused();
    virtual sal_Bool        IsVisible();
    virtual sal_Bool        IsSelected();
    virtual sal_Bool        IsChecked();

    void                    SetEnabled( sal_Bool bEnabled );
    void                    SetFocused( sal_Bool bFocused );
    void                    SetVisible( sal_Bool bVisible );
    void                    SetSelected( sal_Bool bSelected );
    void                    SetChecked( sal_Bool bChecked );

    void                    UpdateEnabled( sal_Int32 i, sal_Bool bEnabled );
    void                    UpdateFocused( sal_Int32 i, sal_Bool bFocused );
    void                    UpdateVisible();
    void                    UpdateSelected( sal_Int32 i, sal_Bool bSelected );
    void                    UpdateChecked( sal_Int32 i, sal_Bool bChecked );
    void                    UpdateAccessibleName( sal_Int32 i );
    void                    UpdateItemText( sal_Int32 i );

    sal_Int32               GetChildCount();

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GetChild( sal_Int32 i );
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GetChildAt( const ::com::sun::star::awt::Point& rPoint );

    void                    InsertChild( sal_Int32 i );
    void                    RemoveChild( sal_Int32 i );

    virtual sal_Bool        IsHighlighted();
    sal_Bool                IsChildHighlighted();

    void                    SelectChild( sal_Int32 i );
    void                    DeSelectAll();
    sal_Bool                IsChildSelected( sal_Int32 i );

    virtual void            Select();
    virtual void            DeSelect();
    virtual void            Click();
    virtual sal_Bool        IsPopupMenuOpen();

    DECL_LINK( MenuEventListener, VclSimpleEvent* );

    virtual void            ProcessMenuEvent( const VclMenuEvent& rVclMenuEvent );

    virtual void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet ) = 0;

    // XComponent
    virtual void SAL_CALL   disposing();

public:
    OAccessibleMenuBaseComponent( Menu* pMenu );
    virtual ~OAccessibleMenuBaseComponent();

    void                    SetStates();

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
};

#endif // ACCESSIBILITY_STANDARD_ACCESSIBLEMENUBASECOMPONENT_HXX

