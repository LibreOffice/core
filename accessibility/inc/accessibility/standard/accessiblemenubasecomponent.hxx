/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessiblemenubasecomponent.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 14:13:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef ACCESSIBILITY_STANDARD_ACCESSIBLEMENUBASECOMPONENT_HXX
#define ACCESSIBILITY_STANDARD_ACCESSIBLEMENUBASECOMPONENT_HXX

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif

#ifndef COMPHELPER_ACCESSIBLE_COMPONENT_HELPER_HXX
#include <comphelper/accessiblecomponenthelper.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

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

