/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxmenu.cxx,v $
 * $Revision: 1.13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"


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

VCLXMenu::VCLXMenu( Menu* pMenu ) : maMenuListeners( *this )
{
    mpMenu = pMenu;
}

VCLXMenu::~VCLXMenu()
{
    for ( sal_uInt32 n = maPopupMenueRefs.Count(); n; )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > * pRef = maPopupMenueRefs.GetObject( --n );
        delete pRef;
    }
    if ( mpMenu )
    {
        mpMenu->RemoveEventListener( LINK( this, VCLXMenu, MenuEventListener ) );
        delete mpMenu;
    }
}

BOOL VCLXMenu::IsPopupMenu() const
{
    return (mpMenu && ! mpMenu->IsMenuBar());
}

void VCLXMenu::ImplCreateMenu( sal_Bool bPopup )
{
    DBG_ASSERT( !mpMenu, "CreateMenu: Menu exists!" );

    if ( bPopup )
        mpMenu = new PopupMenu;
    else
        mpMenu = new MenuBar;

    mpMenu->AddEventListener( LINK( this, VCLXMenu, MenuEventListener ) );
}

IMPL_LINK( VCLXMenu, MenuEventListener, VclSimpleEvent*, pEvent )
{
    DBG_ASSERT( pEvent && pEvent->ISA( VclMenuEvent ), "Unknown Event!" );
    if ( pEvent && pEvent->ISA( VclMenuEvent ) )
    {
        DBG_ASSERT( ((VclMenuEvent*)pEvent)->GetMenu() && mpMenu, "Menu???" );

        VclMenuEvent* pMenuEvent = (VclMenuEvent*)pEvent;
        if ( pMenuEvent->GetMenu() == mpMenu )  // Also called for the root menu
        {
            switch ( pMenuEvent->GetId() )
            {
                case VCLEVENT_MENU_SELECT:
                {
                    if ( maMenuListeners.getLength() )
                    {
                        ::com::sun::star::awt::MenuEvent aEvent;
                        aEvent.Source = (::cppu::OWeakObject*)this;
                        aEvent.MenuId = mpMenu->GetCurItemId();
                        maMenuListeners.select( aEvent );
                    }
                }
                break;
                case VCLEVENT_OBJECT_DYING:
                {
                    mpMenu = NULL;
                }
                break;
                case VCLEVENT_MENU_HIGHLIGHT:
                {
                    if ( maMenuListeners.getLength() )
                    {
                        ::com::sun::star::awt::MenuEvent aEvent;
                        aEvent.Source = (::cppu::OWeakObject*)this;
                        aEvent.MenuId = mpMenu->GetCurItemId();
                        maMenuListeners.highlight( aEvent );
                    }
                }
                break;
                case VCLEVENT_MENU_ACTIVATE:
                {
                    if ( maMenuListeners.getLength() )
                    {
                        ::com::sun::star::awt::MenuEvent aEvent;
                        aEvent.Source = (::cppu::OWeakObject*)this;
                        aEvent.MenuId = mpMenu->GetCurItemId();
                        maMenuListeners.activate( aEvent );
                    }
                }
                break;
                case VCLEVENT_MENU_DEACTIVATE:
                {
                    if ( maMenuListeners.getLength() )
                    {
                        ::com::sun::star::awt::MenuEvent aEvent;
                        aEvent.Source = (::cppu::OWeakObject*)this;
                        aEvent.MenuId = mpMenu->GetCurItemId();
                        maMenuListeners.deactivate( aEvent );
                    }
                }
                break;

                // ignore accessibility events
                case VCLEVENT_MENU_ENABLE:
                case VCLEVENT_MENU_INSERTITEM:
                case VCLEVENT_MENU_REMOVEITEM:
                case VCLEVENT_MENU_SUBMENUACTIVATE:
                case VCLEVENT_MENU_SUBMENUDEACTIVATE:
                case VCLEVENT_MENU_SUBMENUCHANGED:
                case VCLEVENT_MENU_DEHIGHLIGHT:
                case VCLEVENT_MENU_DISABLE:
                case VCLEVENT_MENU_ITEMTEXTCHANGED:
                case VCLEVENT_MENU_ITEMCHECKED:
                case VCLEVENT_MENU_ITEMUNCHECKED:
                case VCLEVENT_MENU_SHOW:
                case VCLEVENT_MENU_HIDE:
                break;

                default:    DBG_ERROR( "MenuEventListener - Unknown event!" );
           }
       }
    }
    return 0;
}


// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXMenu::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XMenu*, (::com::sun::star::awt::XMenuBar*) this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XMenuBar*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XPopupMenu*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XMenuExtended*, this ),
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
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuExtended>* ) NULL )
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

    if ( mpMenu )
        mpMenu->InsertItem( nItemId, aText, (MenuItemBits)nItemStyle, nPos );
}

void VCLXMenu::removeItem( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nItemCount = (sal_Int32)mpMenu->GetItemCount();
    if ( mpMenu && ( nCount > 0 ) && ( nPos >= 0 ) && ( nPos < nItemCount ) && ( nItemCount > 0 ))
    {
        sal_Int16 nP = sal::static_int_cast< sal_Int16 >(
            Min( (int)(nPos+nCount), (int)nItemCount ));
        while( nP-nPos > 0 )
            mpMenu->RemoveItem( --nP );
    }
}

sal_Int16 VCLXMenu::getItemCount(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->GetItemCount() : 0;
}

sal_Int16 VCLXMenu::getItemId( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->GetItemId( nPos ) : 0;
}

sal_Int16 VCLXMenu::getItemPos( sal_Int16 nId ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->GetItemPos( nId ) : 0;
}

void VCLXMenu::enableItem( sal_Int16 nItemId, sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->EnableItem( nItemId, bEnable );
}

sal_Bool VCLXMenu::isItemEnabled( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->IsItemEnabled( nItemId ) : sal_False;
}

void VCLXMenu::setItemText( sal_Int16 nItemId, const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->SetItemText( nItemId, aText );
}

::rtl::OUString VCLXMenu::getItemText( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString aItemText;
    if ( mpMenu )
        aItemText = mpMenu->GetItemText( nItemId );
    return aItemText;
}

void VCLXMenu::setPopupMenu( sal_Int16 nItemId, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >& rxPopupMenu ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    VCLXMenu* pVCLMenu = VCLXMenu::GetImplementation( rxPopupMenu );
    DBG_ASSERT( pVCLMenu && pVCLMenu->GetMenu() && pVCLMenu->IsPopupMenu(), "setPopupMenu: Invalid Menu!" );

    if ( mpMenu && pVCLMenu && pVCLMenu->GetMenu() && pVCLMenu->IsPopupMenu() )
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
    Menu* pMenu = mpMenu ? mpMenu->GetPopupMenu( nItemId ) : NULL;
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

    if ( mpMenu )
        mpMenu->InsertSeparator( nPos );
}

void VCLXMenu::setDefaultItem( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->SetDefaultItem( nItemId );
}

sal_Int16 VCLXMenu::getDefaultItem(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->GetDefaultItem() : 0;
}

void VCLXMenu::checkItem( sal_Int16 nItemId, sal_Bool bCheck ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->CheckItem( nItemId, bCheck );
}

sal_Bool VCLXMenu::isItemChecked( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->IsItemChecked( nItemId ) : sal_False;
}

sal_Int16 VCLXMenu::execute( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rxWindowPeer, const ::com::sun::star::awt::Rectangle& rArea, sal_Int16 nFlags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int16 nRet = 0;
    if ( mpMenu && IsPopupMenu() )
        nRet = ((PopupMenu*)mpMenu)->Execute( VCLUnoHelper::GetWindow( rxWindowPeer ), VCLRectangle(rArea), nFlags );
    return nRet;
}


void SAL_CALL VCLXMenu::setCommand( sal_Int16 nItemId, const ::rtl::OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->SetItemCommand( nItemId, aCommand );
}

::rtl::OUString SAL_CALL VCLXMenu::getCommand( sal_Int16 nItemId ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString aItemCommand;
    if ( mpMenu )
        aItemCommand = mpMenu->GetItemCommand( nItemId );
    return aItemCommand;
}

void SAL_CALL VCLXMenu::setHelpCommand( sal_Int16 nItemId, const ::rtl::OUString& aHelp ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->SetHelpCommand( nItemId, aHelp );
}

::rtl::OUString SAL_CALL VCLXMenu::getHelpCommand( sal_Int16 nItemId ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString aHelpCommand;
    if ( mpMenu )
        aHelpCommand = mpMenu->GetHelpCommand( nItemId );
    return aHelpCommand;
}

//  ----------------------------------------------------
//  class VCLXMenuBar
//  ----------------------------------------------------

VCLXMenuBar::VCLXMenuBar()
{
    ImplCreateMenu( FALSE );
}

VCLXMenuBar::VCLXMenuBar( MenuBar* pMenuBar ) : VCLXMenu( (Menu *)pMenuBar )
{
}

//  ----------------------------------------------------
//  class VCLXPopupMenu
//  ----------------------------------------------------

VCLXPopupMenu::VCLXPopupMenu()
{
    ImplCreateMenu( TRUE );
}
