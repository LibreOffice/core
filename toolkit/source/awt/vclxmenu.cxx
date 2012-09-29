/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uuid.h>
#include <osl/mutex.hxx>

#include <vcl/menu.hxx>
#include <vcl/keycod.hxx>
#include <vcl/image.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/awt/KeyModifier.hpp>


#ifdef DBG_UTIL
    #define THROW_MENUITEM_NOT_FOUND( Func, nItemId ) \
        if ( MENU_ITEM_NOTFOUND == mpMenu->GetItemPos( nItemId ) ) \
            throw  ::com::sun::star::container::NoSuchElementException( \
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( Func ) ) \
                += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": There is no menu item with " ) ) \
                += ::rtl::OUString::valueOf( sal_Int32( nItemId ) ) \
                += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " as identifier" ) ), \
                *this \
            );
    #define THROW_MENUPOS_NOT_FOUND( Func, nPos ) \
        if ( MENU_ITEM_NOTFOUND == sal_uInt16( nPos ) ) \
            throw  ::com::sun::star::container::NoSuchElementException( \
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( Func ) ) \
                += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": There is no menu item at position " ) ) \
                += ::rtl::OUString::valueOf( sal_Int32( nPos ) ), \
                *this \
            );
#else
    #define THROW_MENUITEM_NOT_FOUND( Func, nItemId ) \
        if ( MENU_ITEM_NOTFOUND == mpMenu->GetItemPos( nItemId ) ) \
            throw  ::com::sun::star::container::NoSuchElementException();
    #define THROW_MENUPOS_NOT_FOUND( Func, nPos ) \
        if ( MENU_ITEM_NOTFOUND == sal_uInt16( nPos ) ) \
            throw  ::com::sun::star::container::NoSuchElementException();
#endif


//  ----------------------------------------------------
//  class VCLXMenu
//  ----------------------------------------------------

DBG_NAME(VCLXMenu)

VCLXMenu::VCLXMenu() : maMenuListeners( *this )
{
    DBG_CTOR( VCLXMenu, 0 );
    mpMenu = NULL;
}

VCLXMenu::VCLXMenu( Menu* pMenu ) : maMenuListeners( *this )
{
    DBG_CTOR( VCLXMenu, 0 );
    mpMenu = pMenu;
}

VCLXMenu::~VCLXMenu()
{
    DBG_DTOR( VCLXMenu, 0 );
    for ( size_t n = maPopupMenueRefs.size(); n; ) {
        delete maPopupMenueRefs[ --n ];
    }
    if ( mpMenu )
    {
        mpMenu->RemoveEventListener( LINK( this, VCLXMenu, MenuEventListener ) );
        delete mpMenu;
    }
}

sal_Bool VCLXMenu::IsPopupMenu() const
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

                default:    OSL_FAIL( "MenuEventListener - Unknown event!" );
           }
       }
    }
    return 0;
}


//=============================================================================
//=============================================================================
//=============================================================================


// ::com::sun::star::lang::XServiceInfo
::rtl::OUString SAL_CALL VCLXMenu::getImplementationName(  )
throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::ResettableGuard < ::osl::Mutex > aGuard( GetMutex() );
    const sal_Bool bIsPopupMenu = IsPopupMenu();
    aGuard.clear();

    ::rtl::OUString implName( RTL_CONSTASCII_USTRINGPARAM( "stardiv.Toolkit." ) );
    if ( bIsPopupMenu )
        implName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VCLXPopupMenu" ) );
    else
        implName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VCLXMenuBar" ) );

    return implName;
}


::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL VCLXMenu::getSupportedServiceNames(  )
throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::ResettableGuard < ::osl::Mutex > aGuard( GetMutex() );
    const sal_Bool bIsPopupMenu = IsPopupMenu();
    aGuard.clear();

    ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames( 1 );
    if ( bIsPopupMenu )
        aNames[ 0 ] = ::rtl::OUString::createFromAscii( szServiceName2_PopupMenu );
    else
        aNames[ 0 ] = ::rtl::OUString::createFromAscii( szServiceName2_MenuBar );

    return aNames;
}


::sal_Bool SAL_CALL VCLXMenu::supportsService( const ::rtl::OUString& rServiceName )
throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames( getSupportedServiceNames() );

    if ( aServiceNames[ 0 ] == rServiceName )
        return sal_True;

    return sal_False;
}


// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXMenu::queryInterface( const ::com::sun::star::uno::Type & rType )
throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::ResettableGuard < ::osl::Mutex > aGuard( GetMutex() );
    const sal_Bool bIsPopupMenu = IsPopupMenu();
    aGuard.clear();

    ::com::sun::star::uno::Any aRet;

    if ( bIsPopupMenu )
        aRet = ::cppu::queryInterface(  rType,
                                        (static_cast< ::com::sun::star::awt::XMenu* >((::com::sun::star::awt::XMenuBar*) this) ),
                                        (static_cast< ::com::sun::star::awt::XPopupMenu* >(this)),
                                        (static_cast< ::com::sun::star::awt::XPopupMenuExtended* >(this)),
                                        (static_cast< ::com::sun::star::awt::XMenuExtended* >((::com::sun::star::awt::XPopupMenuExtended*) this) ),
                                        (static_cast< ::com::sun::star::awt::XMenuExtended2* >((::com::sun::star::awt::XPopupMenuExtended*) this) ),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)),
                                        (static_cast< ::com::sun::star::lang::XServiceInfo* >(this)),
                                        (static_cast< ::com::sun::star::lang::XUnoTunnel* >(this)) );
    else
        aRet = ::cppu::queryInterface(  rType,
                                        (static_cast< ::com::sun::star::awt::XMenu* >((::com::sun::star::awt::XMenuBar*) this) ),
                                        (static_cast< ::com::sun::star::awt::XMenuBar* >(this)),
                                        (static_cast< ::com::sun::star::awt::XMenuBarExtended* >(this)),
                                        (static_cast< ::com::sun::star::awt::XMenuExtended* >((::com::sun::star::awt::XMenuBarExtended*) this) ),
                                        (static_cast< ::com::sun::star::awt::XMenuExtended2* >((::com::sun::star::awt::XMenuBarExtended*) this) ),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)),
                                        (static_cast< ::com::sun::star::lang::XServiceInfo* >(this)),
                                        (static_cast< ::com::sun::star::lang::XUnoTunnel* >(this)) );

    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXMenu )

// ::com::sun::star::lang::XTypeProvider
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > VCLXMenu::getTypes()
throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::ResettableGuard < ::osl::Mutex > aGuard( GetMutex() );
    const sal_Bool bIsPopupMenu = IsPopupMenu();
    aGuard.clear();

    static ::cppu::OTypeCollection* pCollectionMenuBar = NULL;
    static ::cppu::OTypeCollection* pCollectionPopupMenu = NULL;

    if ( bIsPopupMenu )
    {
        if( !pCollectionPopupMenu )
        {
            ::osl::Guard< ::osl::Mutex > aGlobalGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pCollectionPopupMenu )
            {
                static ::cppu::OTypeCollection collectionPopupMenu(
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenu>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenuExtended>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuExtended>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuExtended2>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo>* ) NULL ) );
                pCollectionPopupMenu = &collectionPopupMenu;
            }
        }

        return (*pCollectionPopupMenu).getTypes();
    }
    else
    {
        if( !pCollectionMenuBar )
        {
            ::osl::Guard< ::osl::Mutex > aGlobalGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pCollectionMenuBar )
            {
                static ::cppu::OTypeCollection collectionMenuBar(
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenu>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBarExtended>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuExtended>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuExtended2>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo>* ) NULL ) );
                pCollectionMenuBar = &collectionMenuBar;
            }
        }
        return (*pCollectionMenuBar).getTypes();
    }
}


::com::sun::star::uno::Sequence< sal_Int8 > VCLXMenu::getImplementationId()
throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::ResettableGuard < ::osl::Mutex > aGuard( GetMutex() );
    const sal_Bool bIsPopupMenu = IsPopupMenu();
    aGuard.clear();

    static ::cppu::OImplementationId* pIdMenuBar = NULL;
    static ::cppu::OImplementationId* pIdPopupMenu = NULL;

    if ( bIsPopupMenu )
    {
        if( !pIdPopupMenu )
        {
            ::osl::Guard< ::osl::Mutex > aGlobalGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pIdPopupMenu )
            {
                static ::cppu::OImplementationId idPopupMenu( sal_False );
                pIdPopupMenu = &idPopupMenu;
            }
        }

        return (*pIdPopupMenu).getImplementationId();
    }
    else
    {
        if( !pIdMenuBar )
        {
            ::osl::Guard< ::osl::Mutex > aGlobalGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pIdMenuBar )
            {
                static ::cppu::OImplementationId idMenuBar( sal_False );
                pIdMenuBar = &idMenuBar;
            }
        }

        return (*pIdMenuBar).getImplementationId();
    }
}


//=============================================================================
//=============================================================================
//=============================================================================


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
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->InsertItem( nItemId, aText, (MenuItemBits)nItemStyle, nPos );
}

void VCLXMenu::removeItem( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
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
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->GetItemCount() : 0;
}

sal_Int16 VCLXMenu::getItemId( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->GetItemId( nPos ) : 0;
}

sal_Int16 VCLXMenu::getItemPos( sal_Int16 nId ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->GetItemPos( nId ) : 0;
}

void VCLXMenu::enableItem( sal_Int16 nItemId, sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->EnableItem( nItemId, bEnable );
}

sal_Bool VCLXMenu::isItemEnabled( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->IsItemEnabled( nItemId ) : sal_False;
}

void VCLXMenu::setItemText( sal_Int16 nItemId, const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->SetItemText( nItemId, aText );
}

::rtl::OUString VCLXMenu::getItemText( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString aItemText;
    if ( mpMenu )
        aItemText = mpMenu->GetItemText( nItemId );
    return aItemText;
}

void VCLXMenu::setPopupMenu( sal_Int16 nItemId, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >& rxPopupMenu ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    VCLXMenu* pVCLMenu = VCLXMenu::GetImplementation( rxPopupMenu );
    DBG_ASSERT( pVCLMenu && pVCLMenu->GetMenu() && pVCLMenu->IsPopupMenu(), "setPopupMenu: Invalid Menu!" );

    if ( mpMenu && pVCLMenu && pVCLMenu->GetMenu() && pVCLMenu->IsPopupMenu() )
    {
        // Selbst eine Ref halten!
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > * pNewRef = new ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > ;
        *pNewRef = rxPopupMenu;
        maPopupMenueRefs.push_back( pNewRef );

        mpMenu->SetPopupMenu( nItemId, (PopupMenu*) pVCLMenu->GetMenu() );
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > VCLXMenu::getPopupMenu( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >  aRef;
    Menu* pMenu = mpMenu ? mpMenu->GetPopupMenu( nItemId ) : NULL;
    if ( pMenu )
    {
        for ( size_t n = maPopupMenueRefs.size(); n; )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > * pRef = maPopupMenueRefs[ --n ];
            Menu* pM = ((VCLXMenu*)pRef->get())->GetMenu();
            if ( pM == pMenu )
            {
                aRef = *pRef;
                break;
            }
        }
        // it seems the popup menu is not insert into maPopupMenueRefs
        // if the popup men is not created by stardiv.Toolkit.VCLXPopupMenu
        if( !aRef.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > * pNewRef = new ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > ;
            *pNewRef = new VCLXPopupMenu( (PopupMenu*)pMenu );
            aRef = *pNewRef;
        }
    }
    return aRef;
}

// ::com::sun::star::awt::XPopupMenu
void VCLXMenu::insertSeparator( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->InsertSeparator( nPos );
}

void VCLXMenu::setDefaultItem( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->SetDefaultItem( nItemId );
}

sal_Int16 VCLXMenu::getDefaultItem(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->GetDefaultItem() : 0;
}

void VCLXMenu::checkItem( sal_Int16 nItemId, sal_Bool bCheck ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->CheckItem( nItemId, bCheck );
}

sal_Bool VCLXMenu::isItemChecked( sal_Int16 nItemId ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->IsItemChecked( nItemId ) : sal_False;
}

sal_Int16 VCLXMenu::execute( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rxWindowPeer, const ::com::sun::star::awt::Rectangle& rArea, sal_Int16 nFlags ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int16 nRet = 0;
    if ( mpMenu && IsPopupMenu() )
        nRet = ((PopupMenu*)mpMenu)->Execute( VCLUnoHelper::GetWindow( rxWindowPeer ), VCLRectangle(rArea), nFlags | POPUPMENU_NOMOUSEUPCLOSE );
    return nRet;
}


void SAL_CALL VCLXMenu::setCommand( sal_Int16 nItemId, const ::rtl::OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->SetItemCommand( nItemId, aCommand );
}

::rtl::OUString SAL_CALL VCLXMenu::getCommand( sal_Int16 nItemId ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString aItemCommand;
    if ( mpMenu )
        aItemCommand = mpMenu->GetItemCommand( nItemId );
    return aItemCommand;
}

void SAL_CALL VCLXMenu::setHelpCommand( sal_Int16 nItemId, const ::rtl::OUString& aHelp ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->SetHelpCommand( nItemId, aHelp );
}

::rtl::OUString SAL_CALL VCLXMenu::getHelpCommand( sal_Int16 nItemId ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString aHelpCommand;
    if ( mpMenu )
        aHelpCommand = mpMenu->GetHelpCommand( nItemId );
    return aHelpCommand;
}


// ============================================================================
// ============================================================================
// ============================================================================


// BEGIN ANONYMOUS NAMESPACE
namespace
{
    namespace css = ::com::sun::star;

    Image lcl_XGraphic2VCLImage(
                                const css::uno::Reference< css::graphic::XGraphic >& xGraphic,
                                sal_Bool bResize )
    {
        Image aImage;
        if ( !xGraphic.is() )
            return aImage;

        aImage = Image( xGraphic );
        const ::Size aCurSize = aImage.GetSizePixel();
        const sal_Int32 nCurWidth = aCurSize.Width();
        const sal_Int32 nCurHeight = aCurSize.Height();
        const sal_Int32 nIdeal( 16 );

        if ( nCurWidth > 0 && nCurHeight > 0 )
        {
            if ( bResize && ( nCurWidth > nIdeal || nCurHeight > nIdeal ) )
            {
                sal_Int32 nIdealWidth  = nCurWidth  > nIdeal ? nIdeal : nCurWidth;
                sal_Int32 nIdealHeight = nCurHeight > nIdeal ? nIdeal : nCurHeight;

                ::Size aNewSize( nIdealWidth, nIdealHeight );

                sal_Bool bModified( sal_False );
                BitmapEx aBitmapEx = aImage.GetBitmapEx();
                bModified = aBitmapEx.Scale( aNewSize, BMP_SCALE_BEST );

                if ( bModified )
                    aImage = Image( aBitmapEx );
            }
        }
        return aImage;
    }

    /**
        As svtools builds after toolkit, we can not include/use
        svtools/inc/acceleratorexecute.hxx
        So I just copy here svt::AcceleratorExecute::st_AWTKey2VCLKey
        and svt::AcceleratorExecute::st_VCLKey2AWTKey
    */
    css::awt::KeyEvent lcl_VCLKey2AWTKey(const KeyCode& aVCLKey)
    {
        css::awt::KeyEvent aAWTKey;
        aAWTKey.Modifiers = 0;
        aAWTKey.KeyCode   = (sal_Int16)aVCLKey.GetCode();

        if (aVCLKey.IsShift())
            aAWTKey.Modifiers |= css::awt::KeyModifier::SHIFT;
        if (aVCLKey.IsMod1())
            aAWTKey.Modifiers |= css::awt::KeyModifier::MOD1;
        if (aVCLKey.IsMod2())
            aAWTKey.Modifiers |= css::awt::KeyModifier::MOD2;
        if (aVCLKey.IsMod3())
            aAWTKey.Modifiers |= css::awt::KeyModifier::MOD3;

        return aAWTKey;
    }

    KeyCode lcl_AWTKey2VCLKey(const css::awt::KeyEvent& aAWTKey)
    {
        sal_Bool bShift = ((aAWTKey.Modifiers & css::awt::KeyModifier::SHIFT) == css::awt::KeyModifier::SHIFT );
        sal_Bool bMod1  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD1 ) == css::awt::KeyModifier::MOD1  );
        sal_Bool bMod2  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD2 ) == css::awt::KeyModifier::MOD2  );
        sal_Bool bMod3  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD3 ) == css::awt::KeyModifier::MOD3  );
        sal_uInt16   nKey   = (sal_uInt16)aAWTKey.KeyCode;

        return KeyCode(nKey, bShift, bMod1, bMod2, bMod3);
    }

} // END ANONYMOUS NAMESPACE


// ============================================================================
// ============================================================================
// ============================================================================


// XMenuExtended2 Methods

::sal_Bool SAL_CALL VCLXMenu::isPopupMenu(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return IsPopupMenu();
}

void SAL_CALL VCLXMenu::clear(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    if ( mpMenu )
        mpMenu->Clear();
}


::com::sun::star::awt::MenuItemType SAL_CALL VCLXMenu::getItemType( ::sal_Int16 nItemPos )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::awt::MenuItemType aMenuItemType =
        ::com::sun::star::awt::MenuItemType_DONTKNOW;
    if ( mpMenu )
    {
        THROW_MENUPOS_NOT_FOUND( "VCLXMenu::getItemType()", nItemPos )
        aMenuItemType = ( (::com::sun::star::awt::MenuItemType) mpMenu->GetItemType( nItemPos ) );
    }

    return aMenuItemType;
}

void SAL_CALL VCLXMenu::hideDisabledEntries( ::sal_Bool bHide )
throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    if ( mpMenu )
    {
        if ( bHide )
            mpMenu->SetMenuFlags( mpMenu->GetMenuFlags() | MENU_FLAG_HIDEDISABLEDENTRIES );
        else
            mpMenu->SetMenuFlags( mpMenu->GetMenuFlags() & ~MENU_FLAG_HIDEDISABLEDENTRIES );
    }
}


// ============================================================================
// ============================================================================
// ============================================================================


// XPopupMenuExtended Methods

::sal_Bool SAL_CALL VCLXMenu::isInExecute(  )
throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && IsPopupMenu() )
        return ( (PopupMenu*) mpMenu )->IsInExecute();
    else
        return sal_False;
}


void SAL_CALL VCLXMenu::endExecute()
throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && IsPopupMenu() )
        ( (PopupMenu*) mpMenu )->EndExecute();
}


void SAL_CALL VCLXMenu::setLogo( const ::com::sun::star::awt::MenuLogo& aMenuLogo )
throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
    {
        if ( aMenuLogo.Graphic.is() )
        {
            Image aImage = lcl_XGraphic2VCLImage( aMenuLogo.Graphic, sal_False );
            MenuLogo aVCLMenuLogo;

            aVCLMenuLogo.aBitmap        = aImage.GetBitmapEx();
            aVCLMenuLogo.aStartColor    = Color( (sal_uInt32)(aMenuLogo.StartColor) );
            aVCLMenuLogo.aEndColor      = Color( (sal_uInt32)(aMenuLogo.EndColor) );

            mpMenu->SetLogo( aVCLMenuLogo );
        }
        else
            mpMenu->SetLogo();
    }
}


::com::sun::star::awt::MenuLogo SAL_CALL VCLXMenu::getLogo(  )
throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::awt::MenuLogo aAWTMenuLogo;
    if ( mpMenu )
    {
        if ( mpMenu->HasLogo() )
        {
            MenuLogo aVCLMenuLogo      = mpMenu->GetLogo();
            aAWTMenuLogo.Graphic       = Image(aVCLMenuLogo.aBitmap).GetXGraphic();
            aAWTMenuLogo.StartColor    = aVCLMenuLogo.aStartColor.GetColor();
            aAWTMenuLogo.EndColor      = aVCLMenuLogo.aEndColor.GetColor();
        }
    }
    return aAWTMenuLogo;
}


void SAL_CALL VCLXMenu::enableAutoMnemonics( ::sal_Bool bEnable )
throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    if ( mpMenu )
    {
        if ( !bEnable )
            mpMenu->SetMenuFlags( mpMenu->GetMenuFlags() | MENU_FLAG_NOAUTOMNEMONICS );
        else
            mpMenu->SetMenuFlags( mpMenu->GetMenuFlags() & ~MENU_FLAG_NOAUTOMNEMONICS );
    }
}


void SAL_CALL VCLXMenu::setAcceleratorKeyEvent( ::sal_Int16 nItemId,
                                                const ::com::sun::star::awt::KeyEvent& aKeyEvent )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && IsPopupMenu() )
    {
        THROW_MENUITEM_NOT_FOUND( "VCLXMenu::setAcceleratorKeyEvent()", nItemId )
        KeyCode aVCLKeyCode = lcl_AWTKey2VCLKey( aKeyEvent );
        mpMenu->SetAccelKey( nItemId, aVCLKeyCode );
    }
}


::com::sun::star::awt::KeyEvent SAL_CALL VCLXMenu::getAcceleratorKeyEvent( ::sal_Int16 nItemId )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::awt::KeyEvent aKeyEvent;
    if ( mpMenu && IsPopupMenu() )
    {
        THROW_MENUITEM_NOT_FOUND( "VCLXMenu::getAcceleratorKeyEvent()", nItemId )
        KeyCode nKeyCode = mpMenu->GetAccelKey( nItemId );
        aKeyEvent = lcl_VCLKey2AWTKey( nKeyCode );
    }

    return aKeyEvent;
}


void SAL_CALL VCLXMenu::setHelpText( ::sal_Int16 nItemId, const ::rtl::OUString& sHelpText )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && IsPopupMenu() )
    {
        THROW_MENUITEM_NOT_FOUND( "VCLXMenu::setHelpText()", nItemId )
        mpMenu->SetHelpText( nItemId, sHelpText );
    }
}


::rtl::OUString SAL_CALL VCLXMenu::getHelpText( ::sal_Int16 nItemId )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    rtl::OUString sHelpText;
    if ( mpMenu && IsPopupMenu() )
    {
        THROW_MENUITEM_NOT_FOUND( "VCLXMenu::getHelpText()", nItemId )
        sHelpText = mpMenu->GetHelpText( nItemId );
    }

    return sHelpText;
}


void SAL_CALL VCLXMenu::setTipHelpText( ::sal_Int16 nItemId, const ::rtl::OUString& sTipHelpText )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && IsPopupMenu() )
    {
        THROW_MENUITEM_NOT_FOUND( "VCLXMenu::setTipHelpText()", nItemId )
        mpMenu->SetTipHelpText( nItemId, sTipHelpText );
    }
}


::rtl::OUString SAL_CALL VCLXMenu::getTipHelpText( ::sal_Int16 nItemId )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    rtl::OUString sTipHelpText;
    if ( mpMenu && IsPopupMenu() )
    {
        THROW_MENUITEM_NOT_FOUND( "VCLXMenu::getTipHelpText()", nItemId )
        sTipHelpText = mpMenu->GetTipHelpText( nItemId );
    }
    return sTipHelpText;
}


void SAL_CALL VCLXMenu::setItemImage(
                                            ::sal_Int16 nItemId,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& xGraphic, ::sal_Bool bScale )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && IsPopupMenu() )
    {
        THROW_MENUITEM_NOT_FOUND( "VCLXMenu::setItemImage()", nItemId )
        Image aImage = lcl_XGraphic2VCLImage( xGraphic, bScale );
        mpMenu->SetItemImage( nItemId, aImage );
    }
}


::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL VCLXMenu::getItemImage( ::sal_Int16 nItemId )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > rxGraphic;

    if ( mpMenu && IsPopupMenu() )
    {
        THROW_MENUITEM_NOT_FOUND( "VCLXMenu::getItemImage()", nItemId )
        Image aImage = mpMenu->GetItemImage( nItemId );
        if ( !!aImage )
            rxGraphic = aImage.GetXGraphic();
    }
    return rxGraphic;
}


void SAL_CALL VCLXMenu::setItemImageAngle( ::sal_Int16 nItemId, ::sal_Int32 nAngle )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && IsPopupMenu() )
    {
        THROW_MENUITEM_NOT_FOUND( "VCLXMenu::setItemImageAngle()", nItemId )
        mpMenu->SetItemImageAngle( nItemId, nAngle );
    }
}


::sal_Int32 SAL_CALL VCLXMenu::getItemImageAngle( ::sal_Int16 nItemId )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::sal_Int32 nItemImageAngle( 0 );
    if ( mpMenu && IsPopupMenu() )
    {
        THROW_MENUITEM_NOT_FOUND( "VCLXMenu::getItemImageAngle()", nItemId )
        nItemImageAngle = mpMenu->GetItemImageAngle( nItemId );
    }
    return nItemImageAngle;
}


void SAL_CALL VCLXMenu::setItemImageMirrorMode( ::sal_Int16 nItemId, ::sal_Bool bMirror )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && IsPopupMenu() )
    {
        THROW_MENUITEM_NOT_FOUND( "VCLXMenu::setItemImageMirrorMode()", nItemId )
        mpMenu->SetItemImageMirrorMode( nItemId, bMirror );
    }
}


::sal_Bool SAL_CALL VCLXMenu::isItemImageInMirrorMode( ::sal_Int16 nItemId )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Bool bMirrorMode( sal_False );
    if ( mpMenu && IsPopupMenu() )
    {
        THROW_MENUITEM_NOT_FOUND( "VCLXMenu::isItemImageInMirrorMode()", nItemId )
        bMirrorMode = mpMenu->GetItemImageMirrorMode( nItemId );
    }
    return bMirrorMode;
}


//  ----------------------------------------------------
//  class VCLXMenuBar
//  ----------------------------------------------------

DBG_NAME(VCLXMenuBar);

VCLXMenuBar::VCLXMenuBar()
{
    DBG_CTOR( VCLXMenuBar, 0 );
    ImplCreateMenu( sal_False );
}

VCLXMenuBar::VCLXMenuBar( MenuBar* pMenuBar ) : VCLXMenu( (Menu *)pMenuBar )
{
    DBG_CTOR( VCLXMenuBar, 0 );
}

//  ----------------------------------------------------
//  class VCLXPopupMenu
//  ----------------------------------------------------

DBG_NAME(VCLXPopupMenu);

VCLXPopupMenu::VCLXPopupMenu()
{
    DBG_CTOR( VCLXPopupMenu, 0 );
    ImplCreateMenu( sal_True );
}

VCLXPopupMenu::VCLXPopupMenu( PopupMenu* pPopMenu ) : VCLXMenu( (Menu *)pPopMenu )
{
    DBG_CTOR( VCLXPopupMenu, 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
