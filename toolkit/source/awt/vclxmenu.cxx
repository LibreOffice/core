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

#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>
#include <vcl/dialoghelper.hxx>
#include <vcl/graph.hxx>
#include <vcl/menu.hxx>
#include <vcl/keycod.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/awt/KeyModifier.hpp>

VCLXMenu::VCLXMenu()
    : maMenuListeners( *this )
    , mnDefaultItem(0)
{
    mpMenu = nullptr;
}

VCLXMenu::VCLXMenu( Menu* pMenu )
    : maMenuListeners( *this )
    , mnDefaultItem(0)
{
    mpMenu = pMenu;
}

VCLXMenu::~VCLXMenu()
{
    maPopupMenuRefs.clear();
    if ( mpMenu )
    {
        SolarMutexGuard g;
        mpMenu->RemoveEventListener( LINK( this, VCLXMenu, MenuEventListener ) );
        mpMenu.disposeAndClear();
    }
}

bool VCLXMenu::IsPopupMenu() const
{
    return (mpMenu && ! mpMenu->IsMenuBar());
}

void VCLXMenu::ImplCreateMenu( bool bPopup )
{
    DBG_ASSERT( !mpMenu, "CreateMenu: Menu exists!" );

    if ( bPopup )
        mpMenu = VclPtr<PopupMenu>::Create();
    else
        mpMenu = VclPtr<MenuBar>::Create();

    mpMenu->AddEventListener( LINK( this, VCLXMenu, MenuEventListener ) );
}

void VCLXMenu::ImplAddListener()
{
    assert(mpMenu);
    mpMenu->AddEventListener( LINK( this, VCLXMenu, MenuEventListener ) );
}

IMPL_LINK( VCLXMenu, MenuEventListener, VclMenuEvent&, rMenuEvent, void )
{
    DBG_ASSERT( rMenuEvent.GetMenu() && mpMenu, "Menu???" );

    if ( rMenuEvent.GetMenu() != mpMenu )  // Also called for the root menu
        return;

    switch ( rMenuEvent.GetId() )
    {
        case VclEventId::MenuSelect:
        {
            if ( maMenuListeners.getLength() )
            {
                css::awt::MenuEvent aEvent;
                aEvent.Source = getXWeak();
                aEvent.MenuId = mpMenu->GetCurItemId();
                maMenuListeners.itemSelected( aEvent );
            }
        }
        break;
        case VclEventId::ObjectDying:
        {
            mpMenu = nullptr;
        }
        break;
        case VclEventId::MenuHighlight:
        {
            if ( maMenuListeners.getLength() )
            {
                css::awt::MenuEvent aEvent;
                aEvent.Source = getXWeak();
                aEvent.MenuId = mpMenu->GetCurItemId();
                maMenuListeners.itemHighlighted( aEvent );
            }
        }
        break;
        case VclEventId::MenuActivate:
        {
            if ( maMenuListeners.getLength() )
            {
                css::awt::MenuEvent aEvent;
                aEvent.Source = getXWeak();
                aEvent.MenuId = mpMenu->GetCurItemId();
                maMenuListeners.itemActivated( aEvent );
            }
        }
        break;
        case VclEventId::MenuDeactivate:
        {
            if ( maMenuListeners.getLength() )
            {
                css::awt::MenuEvent aEvent;
                aEvent.Source = getXWeak();
                aEvent.MenuId = mpMenu->GetCurItemId();
                maMenuListeners.itemDeactivated( aEvent );
            }
        }
        break;

        // ignore accessibility events
        case VclEventId::MenuEnable:
        case VclEventId::MenuInsertItem:
        case VclEventId::MenuRemoveItem:
        case VclEventId::MenuSubmenuActivate:
        case VclEventId::MenuSubmenuDeactivate:
        case VclEventId::MenuSubmenuChanged:
        case VclEventId::MenuDehighlight:
        case VclEventId::MenuDisable:
        case VclEventId::MenuItemRoleChanged:
        case VclEventId::MenuItemTextChanged:
        case VclEventId::MenuItemChecked:
        case VclEventId::MenuItemUnchecked:
        case VclEventId::MenuShow:
        case VclEventId::MenuHide:
        break;

        default:    OSL_FAIL( "MenuEventListener - Unknown event!" );
   }
}


OUString SAL_CALL VCLXMenu::getImplementationName(  )
{
    std::unique_lock aGuard( maMutex );
    const bool bIsPopupMenu = IsPopupMenu();
    aGuard.unlock();

    OUString implName( u"stardiv.Toolkit."_ustr );
    if ( bIsPopupMenu )
        implName += "VCLXPopupMenu";
    else
        implName += "VCLXMenuBar";

    return implName;
}

css::uno::Sequence< OUString > SAL_CALL VCLXMenu::getSupportedServiceNames(  )
{
    std::unique_lock aGuard( maMutex );
    const bool bIsPopupMenu = IsPopupMenu();
    aGuard.unlock();

    if ( bIsPopupMenu )
        return css::uno::Sequence<OUString>{
            u"com.sun.star.awt.PopupMenu"_ustr,
            u"stardiv.vcl.PopupMenu"_ustr};
    else
        return css::uno::Sequence<OUString>{
            u"com.sun.star.awt.MenuBar"_ustr,
            u"stardiv.vcl.MenuBar"_ustr};
}

sal_Bool SAL_CALL VCLXMenu::supportsService(const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Any VCLXMenu::queryInterface(
    const css::uno::Type & rType )
{
    std::unique_lock aGuard( maMutex );
    const bool bIsPopupMenu = IsPopupMenu();
    aGuard.unlock();

    css::uno::Any aRet;

    if ( bIsPopupMenu )
        aRet = ::cppu::queryInterface(  rType,
                                        static_cast< css::awt::XMenu* >(static_cast<css::awt::XMenuBar*>(this)),
                                        static_cast< css::awt::XPopupMenu* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this),
                                        static_cast< css::lang::XServiceInfo* >(this) );
    else
        aRet = ::cppu::queryInterface(  rType,
                                        static_cast< css::awt::XMenu* >(static_cast<css::awt::XMenuBar*>(this)),
                                        static_cast< css::awt::XMenuBar* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this),
                                        static_cast< css::lang::XServiceInfo* >(this) );

    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}


css::uno::Sequence< css::uno::Type > VCLXMenu::getTypes()
{
    std::unique_lock aGuard( maMutex );
    const bool bIsPopupMenu = IsPopupMenu();
    aGuard.unlock();

    if ( bIsPopupMenu )
    {
        static cppu::OTypeCollection collectionPopupMenu(
            cppu::UnoType<css::lang::XTypeProvider>::get(), cppu::UnoType<css::awt::XMenu>::get(),
            cppu::UnoType<css::awt::XPopupMenu>::get(),
            cppu::UnoType<css::lang::XServiceInfo>::get());
        return collectionPopupMenu.getTypes();
    }
    else
    {
        static cppu::OTypeCollection collectionMenuBar(
            cppu::UnoType<css::lang::XTypeProvider>::get(), cppu::UnoType<css::awt::XMenu>::get(),
            cppu::UnoType<css::awt::XMenuBar>::get(),
            cppu::UnoType<css::lang::XServiceInfo>::get());
        return collectionMenuBar.getTypes();
    }
}


css::uno::Sequence< sal_Int8 > VCLXMenu::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void VCLXMenu::addMenuListener(
    const css::uno::Reference< css::awt::XMenuListener >& rxListener )
{
    std::unique_lock aGuard( maMutex );

    maMenuListeners.addInterface( rxListener );
}

void VCLXMenu::removeMenuListener(
    const css::uno::Reference< css::awt::XMenuListener >& rxListener )
{
    std::unique_lock aGuard( maMutex );

    maMenuListeners.removeInterface( rxListener );
}

void VCLXMenu::insertItem(
    sal_Int16 nItemId,
    const OUString& aText,
    sal_Int16 nItemStyle,
    sal_Int16 nPos )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu )
        mpMenu->InsertItem(nItemId, aText, static_cast<MenuItemBits>(nItemStyle), {}, nPos);
}

void VCLXMenu::removeItem(
    sal_Int16 nPos,
    sal_Int16 nCount )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if (!mpMenu)
        return;

    sal_Int32 nItemCount = static_cast<sal_Int32>(mpMenu->GetItemCount());
    if ((nCount > 0) && (nPos >= 0) && (nPos < nItemCount))
    {
        sal_Int16 nP = sal::static_int_cast< sal_Int16 >(
            std::min( static_cast<int>(nPos+nCount), static_cast<int>(nItemCount) ));
        while( nP-nPos > 0 )
            mpMenu->RemoveItem( --nP );
    }
}

sal_Int16 VCLXMenu::getItemCount(  )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    return mpMenu ? mpMenu->GetItemCount() : 0;
}

sal_Int16 VCLXMenu::getItemId(
    sal_Int16 nPos )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    return mpMenu ? mpMenu->GetItemId( nPos ) : 0;
}

sal_Int16 VCLXMenu::getItemPos(
    sal_Int16 nId )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    return mpMenu ? mpMenu->GetItemPos( nId ) : 0;
}

void VCLXMenu::enableItem(
    sal_Int16 nItemId,
    sal_Bool bEnable )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu )
        mpMenu->EnableItem( nItemId, bEnable );
}

sal_Bool VCLXMenu::isItemEnabled(
    sal_Int16 nItemId )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    return mpMenu && mpMenu->IsItemEnabled( nItemId );
}

void VCLXMenu::setItemText(
    sal_Int16 nItemId,
    const OUString& aText )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu )
        mpMenu->SetItemText( nItemId, aText );
}

OUString VCLXMenu::getItemText(
    sal_Int16 nItemId )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    OUString aItemText;
    if ( mpMenu )
        aItemText = mpMenu->GetItemText( nItemId );
    return aItemText;
}

void VCLXMenu::setPopupMenu(
    sal_Int16 nItemId,
    const css::uno::Reference< css::awt::XPopupMenu >& rxPopupMenu )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    VCLXMenu* pVCLMenu = dynamic_cast<VCLXMenu*>( rxPopupMenu.get() );
    DBG_ASSERT( pVCLMenu && pVCLMenu->GetMenu() && pVCLMenu->IsPopupMenu(), "setPopupMenu: Invalid Menu!" );

    if ( mpMenu && pVCLMenu && pVCLMenu->GetMenu() && pVCLMenu->IsPopupMenu() )
    {
        maPopupMenuRefs.push_back( rxPopupMenu );

        mpMenu->SetPopupMenu( nItemId, static_cast<PopupMenu*>( pVCLMenu->GetMenu() ) );
    }
}

css::uno::Reference< css::awt::XPopupMenu > VCLXMenu::getPopupMenu(
    sal_Int16 nItemId )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    css::uno::Reference< css::awt::XPopupMenu >  aRef;
    Menu* pMenu = mpMenu ? mpMenu->GetPopupMenu( nItemId ) : nullptr;
    if ( pMenu )
    {
        for ( size_t n = maPopupMenuRefs.size(); n; )
        {
            css::uno::Reference< css::awt::XPopupMenu >& rRef = maPopupMenuRefs[ --n ];
            Menu* pM = static_cast<VCLXMenu*>(rRef.get())->GetMenu();
            if ( pM == pMenu )
            {
                aRef = rRef;
                break;
            }
        }
        /*
           If the popup menu is not inserted via setPopupMenu then
           maPopupMenuRefs won't have an entry for it, so create an XPopupMenu
           for it now.

           This means that this vcl PopupMenu "pMenu" either existed as a child
           of the vcl Menu "mpMenu" before the VCLXMenu was created for that or
           it was added directly via vcl.
        */
        if( !aRef.is() )
        {
            aRef = new VCLXPopupMenu( static_cast<PopupMenu*>(pMenu) );
            /*
               In any case, the VCLXMenu has ownership of "mpMenu" and will
               destroy it in the VCLXMenu dtor.

               Similarly because VCLXPopupMenu takes ownership of the vcl
               PopupMenu "pMenu", the underlying vcl popup will be destroyed
               when VCLXPopupMenu is, so we should add it now to
               maPopupMenuRefs to ensure its lifecycle is at least bound to
               the VCLXMenu that owns the parent "mpMenu" similarly to
               PopupMenus added via the more conventional setPopupMenu.
            */
            maPopupMenuRefs.push_back( aRef );
        }
    }
    return aRef;
}

// css::awt::XPopupMenu
void VCLXMenu::insertSeparator(
    sal_Int16 nPos )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu )
        mpMenu->InsertSeparator({}, nPos);
}

void VCLXMenu::setDefaultItem(
    sal_Int16 nItemId )
{
    std::unique_lock aGuard( maMutex );

    mnDefaultItem = nItemId;
}

sal_Int16 VCLXMenu::getDefaultItem(  )
{
    std::unique_lock aGuard( maMutex );

    return mnDefaultItem;
}

void VCLXMenu::checkItem(
    sal_Int16 nItemId,
    sal_Bool bCheck )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu )
        mpMenu->CheckItem( nItemId, bCheck );
}

sal_Bool VCLXMenu::isItemChecked(
    sal_Int16 nItemId )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    return mpMenu && mpMenu->IsItemChecked( nItemId );
}

sal_Int16 VCLXMenu::execute(
    const css::uno::Reference< css::awt::XWindowPeer >& rxWindowPeer,
    const css::awt::Rectangle& rPos,
    sal_Int16 nFlags )
{
    SolarMutexGuard aSolarGuard;
    auto pMenu = mpMenu;
    {
        std::unique_lock aGuard( maMutex );
        if ( !mpMenu || !IsPopupMenu() )
            return 0;
    }
    PopupMenu* pPopupMenu = static_cast<PopupMenu*>(pMenu.get());
    MenuFlags nMenuFlags = pPopupMenu->GetMenuFlags();
    // #102790# context menus shall never show disabled entries
    nMenuFlags |= MenuFlags::HideDisabledEntries;
    pPopupMenu->SetMenuFlags(nMenuFlags);
    // cannot call this with mutex locked because it will call back into us
    return pPopupMenu->Execute(
                VCLUnoHelper::GetWindow( rxWindowPeer ),
                VCLRectangle( rPos ),
                static_cast<PopupMenuFlags>(nFlags) | PopupMenuFlags::NoMouseUpClose );
}


void SAL_CALL VCLXMenu::setCommand(
    sal_Int16 nItemId,
    const OUString& aCommand )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu )
        mpMenu->SetItemCommand( nItemId, aCommand );
}

OUString SAL_CALL VCLXMenu::getCommand(
    sal_Int16 nItemId )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    OUString aItemCommand;
    if ( mpMenu )
        aItemCommand = mpMenu->GetItemCommand( nItemId );
    return aItemCommand;
}

void SAL_CALL VCLXMenu::setHelpCommand(
    sal_Int16 nItemId,
    const OUString& aHelp )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu )
        mpMenu->SetHelpCommand( nItemId, aHelp );
}

OUString SAL_CALL VCLXMenu::getHelpCommand(
    sal_Int16 nItemId )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    OUString aHelpCommand;
    if ( mpMenu )
        aHelpCommand = mpMenu->GetHelpCommand( nItemId );
    return aHelpCommand;
}


namespace
{
    Image lcl_XGraphic2VCLImage(
        const css::uno::Reference< css::graphic::XGraphic >& xGraphic,
        bool bResize )
    {
        Image aImage;
        if ( !xGraphic.is() )
            return aImage;

        aImage = Image( xGraphic );
        const ::Size aCurSize = aImage.GetSizePixel();
        const sal_Int32 nCurWidth = aCurSize.Width();
        const sal_Int32 nCurHeight = aCurSize.Height();
        constexpr sal_Int32 nIdeal( 16 );

        if ( nCurWidth > 0 && nCurHeight > 0 )
        {
            if ( bResize && ( nCurWidth > nIdeal || nCurHeight > nIdeal ) )
            {
                sal_Int32 nIdealWidth  = std::min(nCurWidth, nIdeal);
                sal_Int32 nIdealHeight = std::min(nCurHeight, nIdeal);

                ::Size aNewSize( nIdealWidth, nIdealHeight );

                bool bModified( false );
                BitmapEx aBitmapEx = aImage.GetBitmapEx();
                bModified = aBitmapEx.Scale( aNewSize, BmpScaleFlag::BestQuality );

                if ( bModified )
                    aImage = Image( aBitmapEx );
            }
        }
        return aImage;
    }

    /** Copied from include/svtools/acceleratorexecute.hxx */
    css::awt::KeyEvent lcl_VCLKey2AWTKey(
        const vcl::KeyCode& aVCLKey)
    {
        css::awt::KeyEvent aAWTKey;
        aAWTKey.Modifiers = 0;
        aAWTKey.KeyCode   = static_cast<sal_Int16>(aVCLKey.GetCode());

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

    vcl::KeyCode lcl_AWTKey2VCLKey(const css::awt::KeyEvent& aAWTKey)
    {
        bool bShift = ((aAWTKey.Modifiers & css::awt::KeyModifier::SHIFT) == css::awt::KeyModifier::SHIFT );
        bool bMod1  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD1 ) == css::awt::KeyModifier::MOD1  );
        bool bMod2  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD2 ) == css::awt::KeyModifier::MOD2  );
        bool bMod3  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD3 ) == css::awt::KeyModifier::MOD3  );
        sal_uInt16   nKey   = static_cast<sal_uInt16>(aAWTKey.KeyCode);

        return vcl::KeyCode(nKey, bShift, bMod1, bMod2, bMod3);
    }

}


sal_Bool SAL_CALL VCLXMenu::isPopupMenu(  )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );
    return IsPopupMenu();
}

void SAL_CALL VCLXMenu::clear(  )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );
    if ( mpMenu )
        mpMenu->Clear();
}


css::awt::MenuItemType SAL_CALL VCLXMenu::getItemType(
    ::sal_Int16 nItemPos )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    css::awt::MenuItemType aMenuItemType =
        css::awt::MenuItemType_DONTKNOW;
    if ( mpMenu )
    {
        aMenuItemType = static_cast<css::awt::MenuItemType>(mpMenu->GetItemType( nItemPos ));
    }

    return aMenuItemType;
}

void SAL_CALL VCLXMenu::hideDisabledEntries(
    sal_Bool bHide )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );
    if ( mpMenu )
    {
        if ( bHide )
            mpMenu->SetMenuFlags( mpMenu->GetMenuFlags() | MenuFlags::HideDisabledEntries );
        else
            mpMenu->SetMenuFlags( mpMenu->GetMenuFlags() & ~MenuFlags::HideDisabledEntries );
    }
}


sal_Bool SAL_CALL VCLXMenu::isInExecute(  )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu && IsPopupMenu() )
        return vcl::IsInPopupMenuExecute();
    else
        return false;
}


void SAL_CALL VCLXMenu::endExecute()
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu && IsPopupMenu() )
        static_cast<PopupMenu*>( mpMenu.get() )->EndExecute();
}


void SAL_CALL VCLXMenu::enableAutoMnemonics(
    sal_Bool bEnable )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );
    if ( mpMenu )
    {
        if ( !bEnable )
            mpMenu->SetMenuFlags( mpMenu->GetMenuFlags() | MenuFlags::NoAutoMnemonics );
        else
            mpMenu->SetMenuFlags( mpMenu->GetMenuFlags() & ~MenuFlags::NoAutoMnemonics );
    }
}


void SAL_CALL VCLXMenu::setAcceleratorKeyEvent(
    ::sal_Int16 nItemId,
    const css::awt::KeyEvent& aKeyEvent )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu && IsPopupMenu() && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        vcl::KeyCode aVCLKeyCode = lcl_AWTKey2VCLKey( aKeyEvent );
        mpMenu->SetAccelKey( nItemId, aVCLKeyCode );
    }
}


css::awt::KeyEvent SAL_CALL VCLXMenu::getAcceleratorKeyEvent(
    ::sal_Int16 nItemId )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    css::awt::KeyEvent aKeyEvent;
    if ( mpMenu && IsPopupMenu() && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        vcl::KeyCode nKeyCode = mpMenu->GetAccelKey( nItemId );
        aKeyEvent = lcl_VCLKey2AWTKey( nKeyCode );
    }

    return aKeyEvent;
}


void SAL_CALL VCLXMenu::setHelpText(
    ::sal_Int16 nItemId,
    const OUString& sHelpText )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        mpMenu->SetHelpText( nItemId, sHelpText );
    }
}


OUString SAL_CALL VCLXMenu::getHelpText(
    ::sal_Int16 nItemId )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    OUString sHelpText;
    if ( mpMenu && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        sHelpText = mpMenu->GetHelpText( nItemId );
    }

    return sHelpText;
}


void SAL_CALL VCLXMenu::setTipHelpText(
    ::sal_Int16 nItemId,
    const OUString& sTipHelpText )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        mpMenu->SetTipHelpText( nItemId, sTipHelpText );
    }
}


OUString SAL_CALL VCLXMenu::getTipHelpText(
    ::sal_Int16 nItemId )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    OUString sTipHelpText;
    if ( mpMenu && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        sTipHelpText = mpMenu->GetTipHelpText( nItemId );
    }
    return sTipHelpText;
}


void SAL_CALL VCLXMenu::setItemImage(
    ::sal_Int16 nItemId,
    const css::uno::Reference< css::graphic::XGraphic >& xGraphic,
    sal_Bool bScale )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    if ( mpMenu && IsPopupMenu() && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        Image aImage = lcl_XGraphic2VCLImage( xGraphic, bScale );
        mpMenu->SetItemImage( nItemId, aImage );
    }
}


css::uno::Reference< css::graphic::XGraphic > SAL_CALL
VCLXMenu::getItemImage(
    ::sal_Int16 nItemId )
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard( maMutex );

    css::uno::Reference< css::graphic::XGraphic > rxGraphic;

    if ( mpMenu && IsPopupMenu() && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        Image aImage = mpMenu->GetItemImage( nItemId );
        if ( !!aImage )
            rxGraphic = Graphic(aImage.GetBitmapEx()).GetXGraphic();
    }
    return rxGraphic;
}

void VCLXMenu::setUserValue(sal_uInt16 nItemId, void* nUserValue, MenuUserDataReleaseFunction aFunc)
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard(maMutex);

    mpMenu->SetUserValue(nItemId, nUserValue, aFunc);
}

void* VCLXMenu::getUserValue(sal_uInt16 nItemId)
{
    SolarMutexGuard aSolarGuard;
    std::unique_lock aGuard(maMutex);

    return mpMenu->GetUserValue(nItemId);
}

VCLXMenuBar::VCLXMenuBar()
{
    ImplCreateMenu( false );
}

VCLXMenuBar::VCLXMenuBar( MenuBar* pMenuBar ) : VCLXMenu( static_cast<Menu *>(pMenuBar) )
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_VCLXMenuBar_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new VCLXMenuBar());
}

VCLXPopupMenu::VCLXPopupMenu()
{
    ImplCreateMenu( true );
}

VCLXPopupMenu::VCLXPopupMenu( PopupMenu* pPopMenu ) : VCLXMenu( static_cast<Menu *>(pPopMenu) )
{
    ImplAddListener();
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_VCLXPopupMenu_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new VCLXPopupMenu());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
