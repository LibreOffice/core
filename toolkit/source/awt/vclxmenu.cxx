/*************************************************************************
 *
 *  $RCSfile: vclxmenu.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:09 $
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


#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>

#include <vcl/menu.hxx>

//  ----------------------------------------------------
//  class VCLXMenu
//  ----------------------------------------------------
VCLXMenu::VCLXMenu() : maMenuListeners( *this )
{
    mpMenu = NULL;
}

VCLXMenu::~VCLXMenu()
{
    for ( sal_uInt32 n = maPopupMenueRefs.Count(); n; )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > * pRef = maPopupMenueRefs.GetObject( --n );
        delete pRef;
    }
    delete mpMenu;
}

void VCLXMenu::ImplCreateMenu( sal_Bool bPopup )
{
    DBG_ASSERT( !mpMenu, "CreateMenu: Menu exists!" );

    mbPopup = bPopup;
    if ( mbPopup )
        mpMenu = new PopupMenu;
    else
        mpMenu = new MenuBar;

    mpMenu->SetSelectHdl( LINK( this, VCLXMenu, SelectHdl ) );
    mpMenu->SetHighlightHdl( LINK( this, VCLXMenu, HighlightHdl ) );
    mpMenu->SetActivateHdl( LINK( this, VCLXMenu, ActivateHdl ) );
    mpMenu->SetDeactivateHdl( LINK( this, VCLXMenu, DeactivateHdl ) );
}

IMPL_LINK( VCLXMenu, SelectHdl, Menu*, EMPTYARG )
{
    if ( maMenuListeners.getLength() )
    {
        ::com::sun::star::awt::MenuEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)this;
        aEvent.MenuId = mpMenu->GetCurItemId();
        maMenuListeners.select( aEvent );
    }
    return 1;
}

IMPL_LINK( VCLXMenu, HighlightHdl, Menu*, EMPTYARG )
{
    if ( maMenuListeners.getLength() )
    {
        ::com::sun::star::awt::MenuEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)this;
        aEvent.MenuId = mpMenu->GetCurItemId();
        maMenuListeners.highlight( aEvent );
    }
    return 1;
}

IMPL_LINK( VCLXMenu, ActivateHdl, Menu*, EMPTYARG )
{
    if ( maMenuListeners.getLength() )
    {
        ::com::sun::star::awt::MenuEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)this;
        aEvent.MenuId = mpMenu->GetCurItemId();
        maMenuListeners.activate( aEvent );
    }
    return 1;
}

IMPL_LINK( VCLXMenu, DeactivateHdl, Menu*, EMPTYARG )
{
    if ( maMenuListeners.getLength() )
    {
        ::com::sun::star::awt::MenuEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)this;
        aEvent.MenuId = mpMenu->GetCurItemId();
        maMenuListeners.deactivate( aEvent );
    }
    return 1;
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXMenu::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XMenu*, (::com::sun::star::awt::XMenuBar*) this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XMenuBar*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XPopupMenu*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XUnoTunnel*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXMenu )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXMenu )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenu>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu>* ) NULL )
IMPL_XTYPEPROVIDER_END


void VCLXMenu::addMenuListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maMenuListeners.addInterface( rxListener );
}

void VCLXMenu::removeMenuListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maMenuListeners.removeInterface( rxListener );
}

void VCLXMenu::insertItem( sal_Int16 nItemId, const ::rtl::OUString& aText, sal_Int16 nItemStyle, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mpMenu->InsertItem( nItemId, aText, (MenuItemBits)nItemStyle, nPos );
}

void VCLXMenu::removeItem( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( nCount )
    {
        sal_uInt16 nP = (sal_uInt16) Min( sal_uInt32(nPos+nCount), (sal_uInt32)mpMenu->GetItemCount() );
        for ( nP; nP; )
            mpMenu->RemoveItem( --nP );
    }
}

sal_Int16 VCLXMenu::getItemCount(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu->GetItemCount();
}

sal_Int16 VCLXMenu::getItemId( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu->GetItemId( nPos );
}

sal_Int16 VCLXMenu::getItemPos( sal_Int16 nId ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu->GetItemPos( nId );
}

void VCLXMenu::enableItem( sal_Int16 nItemId, sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mpMenu->EnableItem( nItemId, bEnable );
}

sal_Bool VCLXMenu::isItemEnabled( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu->IsItemEnabled( nItemId );
}

void VCLXMenu::setItemText( sal_Int16 nItemId, const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mpMenu->SetItemText( nItemId, aText );
}

::rtl::OUString VCLXMenu::getItemText( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu->GetItemText( nItemId );
}

void VCLXMenu::setPopupMenu( sal_Int16 nItemId, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >& rxPopupMenu ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    VCLXMenu* pVCLMenu = VCLXMenu::GetImplementation( rxPopupMenu );
    DBG_ASSERT( pVCLMenu && pVCLMenu->GetMenu() && pVCLMenu->IsPopupMenu(), "setPopupMenu: Invalid Menu!" );

    if ( pVCLMenu && pVCLMenu->GetMenu() && pVCLMenu->IsPopupMenu() )
    {
        // Selbst eine Ref halten!
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > * pNewRef = new ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > ;
        *pNewRef = rxPopupMenu;
        maPopupMenueRefs.Insert( pNewRef, LIST_APPEND );

        mpMenu->SetPopupMenu( nItemId, (PopupMenu*) pVCLMenu->GetMenu() );
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > VCLXMenu::getPopupMenu( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >  aRef;
    Menu* pMenu = mpMenu->GetPopupMenu( nItemId );
    if ( pMenu )
    {
        for ( sal_uInt32 n = maPopupMenueRefs.Count(); n; )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > * pRef = maPopupMenueRefs.GetObject( --n );
            Menu* pM = ((VCLXMenu*)pRef->get())->GetMenu();
            if ( pM == pMenu )
            {
                aRef = *pRef;
                break;
            }
        }
    }
    return aRef;
}


// ::com::sun::star::awt::XPopupMenu
void VCLXMenu::insertSeparator( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mpMenu->InsertSeparator( nPos );
}

void VCLXMenu::setDefaultItem( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mpMenu->SetDefaultItem( nItemId );
}

sal_Int16 VCLXMenu::getDefaultItem(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu->GetDefaultItem();
}

void VCLXMenu::checkItem( sal_Int16 nItemId, sal_Bool bCheck ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mpMenu->CheckItem( nItemId, bCheck );
}

sal_Bool VCLXMenu::isItemChecked( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu->IsItemChecked( nItemId );
}

sal_Int16 VCLXMenu::execute( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rxWindowPeer, const ::com::sun::star::awt::Rectangle& rArea, sal_Int16 nFlags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int16 nRet = 0;
    if ( IsPopupMenu() )
        nRet = ((PopupMenu*)mpMenu)->Execute( VCLUnoHelper::GetWindow( rxWindowPeer ), VCLRectangle(rArea), nFlags );
    return nRet;
}



