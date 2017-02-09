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

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/rc.h>
#include <tools/stream.hxx>

#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/image.hxx>
#include <vcl/event.hxx>
#include <vcl/help.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/timer.hxx>
#include <vcl/decoview.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/menu.hxx>
#include <vcl/button.hxx>
#include <vcl/gradient.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/controllayout.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/dockingarea.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandinfoprovider.hxx>

#include <salinst.hxx>
#include <svdata.hxx>
#include <svids.hrc>
#include <window.h>
#include <salmenu.hxx>
#include <salframe.hxx>

#include "menubarwindow.hxx"
#include "menufloatingwindow.hxx"
#include "menuitemlist.hxx"

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <vcl/unowrap.hxx>

#include <vcl/unohelp.hxx>
#include <vcl/configsettings.hxx>

#include <vcl/lazydelete.hxx>

#include <sfx2/sfxsids.hrc>

#include <map>
#include <vector>

namespace vcl
{

struct MenuLayoutData : public ControlLayoutData
{
    std::vector< sal_uInt16 >               m_aLineItemIds;
    std::vector< sal_uInt16 >               m_aLineItemPositions;
    std::map< sal_uInt16, Rectangle >       m_aVisibleItemBoundRects;
};

}

using namespace vcl;

#define EXTRAITEMHEIGHT     4
#define SPACE_AROUND_TITLE  4

static bool ImplAccelDisabled()
{
    // display of accelerator strings may be suppressed via configuration
    static int nAccelDisabled = -1;

    if( nAccelDisabled == -1 )
    {
        OUString aStr =
            vcl::SettingsConfigItem::get()->
            getValue( "Menu", "SuppressAccelerators" );
        nAccelDisabled = aStr.equalsIgnoreAsciiCase("true") ? 1 : 0;
    }
    return nAccelDisabled == 1;
}

static void ImplSetMenuItemData( MenuItemData* pData )
{
    // convert data
    if ( !pData->aImage )
        pData->eType = MenuItemType::STRING;
    else if ( pData->aText.isEmpty() )
        pData->eType = MenuItemType::IMAGE;
    else
        pData->eType = MenuItemType::STRINGIMAGE;
}

Menu::Menu()
    : mpFirstDel(nullptr),
      pItemList(new MenuItemList),
      pLogo(nullptr),
      pStartedFrom(nullptr),
      pWindow(nullptr),
      nTitleHeight(0),
      nEventId(nullptr),
      mnHighlightedItemPos(ITEMPOS_INVALID),
      nMenuFlags(MenuFlags::NONE),
      nSelectedId(0),
      nImgOrChkPos(0),
      nTextPos(0),
      bCanceled(false),
      bInCallback(false),
      bKilled(false),
      mpLayoutData(nullptr),
      mpSalMenu(nullptr)
{
}

Menu::~Menu()
{
    disposeOnce();
}

void Menu::dispose()
{
    ImplCallEventListeners( VclEventId::ObjectDying, ITEMPOS_INVALID );

    // at the window free the reference to the accessible component
    // and make sure the MenuFloatingWindow knows about our destruction
    if ( pWindow )
    {
        MenuFloatingWindow* pFloat = static_cast<MenuFloatingWindow*>(pWindow.get());
        if( pFloat->pMenu.get() == this )
            pFloat->pMenu.clear();
        pWindow->SetAccessible( css::uno::Reference< css::accessibility::XAccessible >() );
    }

    // dispose accessible components
    if ( mxAccessible.is() )
    {
        css::uno::Reference< css::lang::XComponent> xComponent( mxAccessible, css::uno::UNO_QUERY );
        if ( xComponent.is() )
            xComponent->dispose();
    }

    if ( nEventId )
        Application::RemoveUserEvent( nEventId );

    // Notify deletion of this menu
    ImplMenuDelData* pDelData = mpFirstDel;
    while ( pDelData )
    {
        pDelData->mpMenu = nullptr;
        pDelData = pDelData->mpNext;
    }

    bKilled = true;

    delete pItemList;
    delete pLogo;
    delete mpLayoutData;

    // Native-support: destroy SalMenu
    ImplSetSalMenu( nullptr );

    pStartedFrom.clear();
    pWindow.clear();
    VclReferenceBase::dispose();
}

void Menu::CreateAutoMnemonics()
{
    MnemonicGenerator aMnemonicGenerator;
    size_t n;
    for ( n = 0; n < pItemList->size(); n++ )
    {
        MenuItemData* pData = pItemList->GetDataFromPos( n );
        if ( ! (pData->nBits & MenuItemBits::NOSELECT ) )
            aMnemonicGenerator.RegisterMnemonic( pData->aText );
    }
    for ( n = 0; n < pItemList->size(); n++ )
    {
        MenuItemData* pData = pItemList->GetDataFromPos( n );
        if ( ! (pData->nBits & MenuItemBits::NOSELECT ) )
            pData->aText = aMnemonicGenerator.CreateMnemonic( pData->aText );
    }
}

void Menu::Activate()
{
    bInCallback = true;

    ImplMenuDelData aDelData( this );

    ImplCallEventListeners( VclEventId::MenuActivate, ITEMPOS_INVALID );

    if( !aDelData.isDeleted() )
    {
        if ( !aActivateHdl.Call( this ) )
        {
            if( !aDelData.isDeleted() )
            {
                Menu* pStartMenu = ImplGetStartMenu();
                if ( pStartMenu && ( pStartMenu != this ) )
                {
                    pStartMenu->bInCallback = true;
                    // MT 11/01: Call EventListener here? I don't know...
                    pStartMenu->aActivateHdl.Call( this );
                    pStartMenu->bInCallback = false;
                }
            }
        }
        bInCallback = false;
    }
}

void Menu::Deactivate()
{
    for ( size_t n = pItemList->size(); n; )
    {
        MenuItemData* pData = pItemList->GetDataFromPos( --n );
        if ( pData->bIsTemporary )
        {
            if ( ImplGetSalMenu() )
                ImplGetSalMenu()->RemoveItem( n );

            pItemList->Remove( n );
        }
    }

    bInCallback = true;

    ImplMenuDelData aDelData( this );

    Menu* pStartMenu = ImplGetStartMenu();
    ImplCallEventListeners( VclEventId::MenuDeactivate, ITEMPOS_INVALID );

    if( !aDelData.isDeleted() )
    {
        if ( !aDeactivateHdl.Call( this ) )
        {
            if( !aDelData.isDeleted() )
            {
                if ( pStartMenu && ( pStartMenu != this ) )
                {
                    pStartMenu->bInCallback = true;
                    pStartMenu->aDeactivateHdl.Call( this );
                    pStartMenu->bInCallback = false;
                }
            }
        }
    }

    if( !aDelData.isDeleted() )
    {
        bInCallback = false;
    }
}

void Menu::ImplSelect()
{
    MenuItemData* pData = GetItemList()->GetData( nSelectedId );
    if ( pData && (pData->nBits & MenuItemBits::AUTOCHECK) )
    {
        bool bChecked = IsItemChecked( nSelectedId );
        if ( pData->nBits & MenuItemBits::RADIOCHECK )
        {
            if ( !bChecked )
                CheckItem( nSelectedId );
        }
        else
            CheckItem( nSelectedId, !bChecked );
    }

    // call select
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mpActivePopupMenu = nullptr;        // if new execute in select()
    nEventId = Application::PostUserEvent( LINK( this, Menu, ImplCallSelect ) );
}

void Menu::Select()
{
    ImplMenuDelData aDelData( this );

    ImplCallEventListeners( VclEventId::MenuSelect, GetItemPos( GetCurItemId() ) );
    if ( !aDelData.isDeleted() && !aSelectHdl.Call( this ) )
    {
        if( !aDelData.isDeleted() )
        {
            Menu* pStartMenu = ImplGetStartMenu();
            if ( pStartMenu && ( pStartMenu != this ) )
            {
                pStartMenu->nSelectedId = nSelectedId;
                pStartMenu->aSelectHdl.Call( this );
            }
        }
    }
}

#if defined(MACOSX)
void Menu::ImplSelectWithStart( Menu* pSMenu )
{
    auto pOldStartedFrom = pStartedFrom;
    pStartedFrom = pSMenu;
    auto pOldStartedStarted = pOldStartedFrom ? pOldStartedFrom->pStartedFrom : VclPtr<Menu>();
    Select();
    if( pOldStartedFrom )
        pOldStartedFrom->pStartedFrom = pOldStartedStarted;
    pStartedFrom = pOldStartedFrom;
}
#endif

void Menu::ImplCallEventListeners( VclEventId nEvent, sal_uInt16 nPos )
{
    ImplMenuDelData aDelData( this );

    VclMenuEvent aEvent( this, nEvent, nPos );

    // This is needed by atk accessibility bridge
    if ( nEvent == VclEventId::MenuHighlight )
    {
        Application::ImplCallEventListeners( aEvent );
    }

    if ( !aDelData.isDeleted() )
    {
        // Copy the list, because this can be destroyed when calling a Link...
        std::list<Link<VclMenuEvent&,void>> aCopy( maEventListeners );
        std::list<Link<VclMenuEvent&,void>>::iterator aIter( aCopy.begin() );
        std::list<Link<VclMenuEvent&,void>>::const_iterator aEnd( aCopy.end() );
        while ( aIter != aEnd )
        {
            Link<VclMenuEvent&,void> &rLink = *aIter;
            if( std::find(maEventListeners.begin(), maEventListeners.end(), rLink) != maEventListeners.end() )
                rLink.Call( aEvent );
            ++aIter;
        }
    }
}

void Menu::AddEventListener( const Link<VclMenuEvent&,void>& rEventListener )
{
    maEventListeners.push_back( rEventListener );
}

void Menu::RemoveEventListener( const Link<VclMenuEvent&,void>& rEventListener )
{
    maEventListeners.remove( rEventListener );
}

MenuItemData* Menu::NbcInsertItem(sal_uInt16 nId, MenuItemBits nBits,
                                  const OUString& rStr, Menu* pMenu,
                                  size_t nPos, const OString &rIdent)
{
    // put Item in MenuItemList
    MenuItemData* pData = pItemList->Insert(nId, MenuItemType::STRING,
                             nBits, rStr, Image(), pMenu, nPos, rIdent);

    // update native menu
    if (ImplGetSalMenu() && pData->pSalMenuItem)
        ImplGetSalMenu()->InsertItem(pData->pSalMenuItem, nPos);

    return pData;
}

void Menu::InsertItem(sal_uInt16 nItemId, const OUString& rStr, MenuItemBits nItemBits,
    const OString &rIdent, sal_uInt16 nPos)
{
    SAL_WARN_IF( !nItemId, "vcl", "Menu::InsertItem(): ItemId == 0" );
    SAL_WARN_IF( GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND, "vcl",
                "Menu::InsertItem(): ItemId already exists" );

    // if Position > ItemCount, append
    if ( nPos >= pItemList->size() )
        nPos = MENU_APPEND;

    // put Item in MenuItemList
    NbcInsertItem(nItemId, nItemBits, rStr, this, nPos, rIdent);

    vcl::Window* pWin = ImplGetWindow();
    delete mpLayoutData;
    mpLayoutData = nullptr;
    if ( pWin )
    {
        ImplCalcSize( pWin );
        if ( pWin->IsVisible() )
            pWin->Invalidate();
    }
    ImplCallEventListeners( VclEventId::MenuInsertItem, nPos );
}

void Menu::InsertItem(sal_uInt16 nItemId, const Image& rImage,
    MenuItemBits nItemBits, const OString &rIdent, sal_uInt16 nPos)
{
    InsertItem(nItemId, OUString(), nItemBits, rIdent, nPos);
    SetItemImage( nItemId, rImage );
}

void Menu::InsertItem(sal_uInt16 nItemId, const OUString& rStr,
    const Image& rImage, MenuItemBits nItemBits,
    const OString &rIdent, sal_uInt16 nPos)
{
    InsertItem(nItemId, rStr, nItemBits, rIdent, nPos);
    SetItemImage( nItemId, rImage );
}

void Menu::InsertItem( const ResId& rResId )
{
    ResMgr* pMgr = rResId.GetResMgr();
    if( ! pMgr )
        return;

    RscMenuItem              nObjMask;

    GetRes( rResId.SetRT( RSC_MENUITEM ) );
    nObjMask    = (RscMenuItem)ReadLongRes();

    bool bSep = false;
    if ( nObjMask & RscMenuItem::Separator )
        bSep = ReadShortRes() != 0;

    sal_uInt16 nItemId = 1;
    if ( nObjMask & RscMenuItem::Id )
        nItemId = sal::static_int_cast<sal_uInt16>(ReadLongRes());

    MenuItemBits nStatus = MenuItemBits::NONE;
    if ( nObjMask & RscMenuItem::Status )
        nStatus = sal::static_int_cast<MenuItemBits>(ReadLongRes());

    OUString aText;
    if ( nObjMask & RscMenuItem::Text )
        aText = ReadStringRes();

    // create item
    if (!bSep)
        InsertItem(nItemId, aText, nStatus);
    else
        InsertSeparator();

    OUString aHelpText;
    if ( nObjMask & RscMenuItem::HelpText )
    {
        aHelpText = ReadStringRes();
        if( !bSep )
            SetHelpText( nItemId, aHelpText );
    }

    if ( nObjMask & RscMenuItem::HelpId )
    {
        OString aHelpId( ReadByteStringRes() );
        if ( !bSep )
            SetHelpId( nItemId, aHelpId );
    }

    if( !bSep )
        SetHelpText( nItemId, aHelpText );

    if ( nObjMask & RscMenuItem::Disable )
    {
        if ( !bSep )
            EnableItem( nItemId, ReadShortRes() == 0 );
    }
    if ( nObjMask & RscMenuItem::Command )
    {
        OUString aCommandStr = ReadStringRes();
        if ( !bSep )
            SetItemCommand( nItemId, aCommandStr );
    }
    if ( nObjMask & RscMenuItem::Menu )
    {
        if ( !bSep )
        {
            MenuItemData* pData = GetItemList()->GetData( nItemId );
            if ( pData )
            {
                VclPtr<PopupMenu> pSubMenu = VclPtr<PopupMenu>::Create( ResId( static_cast<RSHEADER_TYPE*>(GetClassRes()), *pMgr ) );
                pData->pAutoSubMenu = pSubMenu;
                // #111060# keep track of this pointer, may be it will be deleted from outside
                pSubMenu->pRefAutoSubMenu = &pData->pAutoSubMenu;
                SetPopupMenu( nItemId, pSubMenu );
            }
        }
        IncrementRes( GetObjSizeRes( static_cast<RSHEADER_TYPE*>(GetClassRes()) ) );
    }
    delete mpLayoutData;
    mpLayoutData = nullptr;
}

void Menu::InsertItem(const OUString& rCommand, const css::uno::Reference<css::frame::XFrame>& rFrame)
{
    sal_uInt16 nItemId = GetItemCount() + 1;

    if (rFrame.is())
    {
        OUString aLabel(CommandInfoProvider::Instance().GetPopupLabelForCommand(rCommand, rFrame));
        OUString aTooltip(CommandInfoProvider::Instance().GetTooltipForCommand(rCommand, rFrame));
        Image aImage(CommandInfoProvider::Instance().GetImageForCommand(rCommand, rFrame));

        InsertItem(nItemId, aLabel, aImage);
        SetHelpText(nItemId, aTooltip);
    }
    else
        InsertItem(nItemId, OUString());

    SetItemCommand(nItemId, rCommand);
}


void Menu::InsertSeparator(const OString &rIdent, sal_uInt16 nPos)
{
    // do nothing if it's a menu bar
    if (IsMenuBar())
        return;

    // if position > ItemCount, append
    if ( nPos >= pItemList->size() )
        nPos = MENU_APPEND;

    // put separator in item list
    pItemList->InsertSeparator(rIdent, nPos);

    // update native menu
    size_t itemPos = ( nPos != MENU_APPEND ) ? nPos : pItemList->size() - 1;
    MenuItemData *pData = pItemList->GetDataFromPos( itemPos );
    if( ImplGetSalMenu() && pData && pData->pSalMenuItem )
        ImplGetSalMenu()->InsertItem( pData->pSalMenuItem, nPos );

    delete mpLayoutData;
    mpLayoutData = nullptr;

    ImplCallEventListeners( VclEventId::MenuInsertItem, nPos );
}

void Menu::RemoveItem( sal_uInt16 nPos )
{
    bool bRemove = false;

    if ( nPos < GetItemCount() )
    {
        // update native menu
        if( ImplGetSalMenu() )
            ImplGetSalMenu()->RemoveItem( nPos );

        pItemList->Remove( nPos );
        bRemove = true;
    }

    vcl::Window* pWin = ImplGetWindow();
    if ( pWin )
    {
        ImplCalcSize( pWin );
        if ( pWin->IsVisible() )
            pWin->Invalidate();
    }
    delete mpLayoutData;
    mpLayoutData = nullptr;

    if ( bRemove )
        ImplCallEventListeners( VclEventId::MenuRemoveItem, nPos );
}

void ImplCopyItem( Menu* pThis, const Menu& rMenu, sal_uInt16 nPos, sal_uInt16 nNewPos )
{
    MenuItemType eType = rMenu.GetItemType( nPos );

    if ( eType == MenuItemType::DONTKNOW )
        return;

    if ( eType == MenuItemType::SEPARATOR )
        pThis->InsertSeparator( OString(), nNewPos );
    else
    {
        sal_uInt16 nId = rMenu.GetItemId( nPos );

        SAL_WARN_IF( pThis->GetItemPos( nId ) != MENU_ITEM_NOTFOUND, "vcl",
                    "Menu::CopyItem(): ItemId already exists" );

        MenuItemData* pData = rMenu.GetItemList()->GetData( nId );

        if (!pData)
            return;

        if ( eType == MenuItemType::STRINGIMAGE )
            pThis->InsertItem( nId, pData->aText, pData->aImage, pData->nBits, pData->sIdent, nNewPos );
        else if ( eType == MenuItemType::STRING )
            pThis->InsertItem( nId, pData->aText, pData->nBits, pData->sIdent, nNewPos );
        else
            pThis->InsertItem( nId, pData->aImage, pData->nBits, pData->sIdent, nNewPos );

        if ( rMenu.IsItemChecked( nId ) )
            pThis->CheckItem( nId );
        if ( !rMenu.IsItemEnabled( nId ) )
            pThis->EnableItem( nId, false );
        pThis->SetHelpId( nId, pData->aHelpId );
        pThis->SetHelpText( nId, pData->aHelpText );
        pThis->SetAccelKey( nId, pData->aAccelKey );
        pThis->SetItemCommand( nId, pData->aCommandStr );
        pThis->SetHelpCommand( nId, pData->aHelpCommandStr );

        PopupMenu* pSubMenu = rMenu.GetPopupMenu( nId );
        if ( pSubMenu )
        {
            // create auto-copy
            VclPtr<PopupMenu> pNewMenu = VclPtr<PopupMenu>::Create( *pSubMenu );
            pThis->SetPopupMenu( nId, pNewMenu );
        }
    }
}

void Menu::CopyItem( const Menu& rMenu, sal_uInt16 nPos )
{
    ImplCopyItem( this, rMenu, nPos, MENU_APPEND );
}

void Menu::Clear()
{
    for ( sal_uInt16 i = GetItemCount(); i; i-- )
        RemoveItem( 0 );
}

sal_uInt16 Menu::GetItemCount() const
{
    return (sal_uInt16)pItemList->size();
}

sal_uInt16 Menu::ImplGetVisibleItemCount() const
{
    sal_uInt16 nItems = 0;
    for ( size_t n = pItemList->size(); n; )
    {
        if ( ImplIsVisible( --n ) )
            nItems++;
    }
    return nItems;
}

sal_uInt16 Menu::ImplGetFirstVisible() const
{
    for ( size_t n = 0; n < pItemList->size(); n++ )
    {
        if ( ImplIsVisible( n ) )
            return n;
    }
    return ITEMPOS_INVALID;
}

sal_uInt16 Menu::ImplGetPrevVisible( sal_uInt16 nPos ) const
{
    for ( size_t n = nPos; n; )
    {
        if ( n && ImplIsVisible( --n ) )
            return n;
    }
    return ITEMPOS_INVALID;
}

sal_uInt16 Menu::ImplGetNextVisible( sal_uInt16 nPos ) const
{
    for ( size_t n = nPos+1; n < pItemList->size(); n++ )
    {
        if ( ImplIsVisible( n ) )
            return n;
    }
    return ITEMPOS_INVALID;
}

sal_uInt16 Menu::GetItemId(sal_uInt16 nPos) const
{
    MenuItemData* pData = pItemList->GetDataFromPos( nPos );

    if ( pData )
        return pData->nId;
    else
        return 0;
}

sal_uInt16 Menu::GetItemId(const OString &rIdent) const
{
    for (size_t n = 0; n < pItemList->size(); ++n)
    {
        MenuItemData* pData = pItemList->GetDataFromPos(n);
        if (pData && pData->sIdent == rIdent)
            return pData->nId;
    }
    return MENU_ITEM_NOTFOUND;
}

sal_uInt16 Menu::GetItemPos( sal_uInt16 nItemId ) const
{
    size_t          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( pData )
        return (sal_uInt16)nPos;
    else
        return MENU_ITEM_NOTFOUND;
}

MenuItemType Menu::GetItemType( sal_uInt16 nPos ) const
{
    MenuItemData* pData = pItemList->GetDataFromPos( nPos );

    if ( pData )
        return pData->eType;
    else
        return MenuItemType::DONTKNOW;
}

OString Menu::GetCurItemIdent() const
{
    const MenuItemData* pData = pItemList->GetData(nSelectedId);
    return pData ? pData->sIdent : OString();
}

OString Menu::GetItemIdent(sal_uInt16 nId) const
{
    const MenuItemData* pData = pItemList->GetData(nId);
    return pData ? pData->sIdent : OString();
}

void Menu::SetItemBits( sal_uInt16 nItemId, MenuItemBits nBits )
{
    MenuItemData* pData = pItemList->GetData( nItemId );
    if ( pData )
        pData->nBits = nBits;
}

MenuItemBits Menu::GetItemBits( sal_uInt16 nItemId ) const
{
    MenuItemBits nBits = MenuItemBits::NONE;
    MenuItemData* pData = pItemList->GetData( nItemId );
    if ( pData )
        nBits = pData->nBits;
    return nBits;
}

void Menu::SetUserValue(sal_uInt16 nItemId, sal_uLong nValue, MenuUserDataReleaseFunction aFunc)
{
    MenuItemData* pData = pItemList->GetData(nItemId);
    if (pData)
    {
        if (pData->aUserValueReleaseFunc)
            pData->aUserValueReleaseFunc(pData->nUserValue);
        pData->aUserValueReleaseFunc = aFunc;
        pData->nUserValue = nValue;
    }
}

sal_uLong Menu::GetUserValue( sal_uInt16 nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );
    return pData ? pData->nUserValue : 0;
}

void Menu::SetPopupMenu( sal_uInt16 nItemId, PopupMenu* pMenu )
{
    size_t          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    // Item does not exist -> return NULL
    if ( !pData )
        return;

    // same menu, nothing to do
    if ( static_cast<PopupMenu*>(pData->pSubMenu.get()) == pMenu )
        return;

    // remove old menu
    auto oldSubMenu = pData->pSubMenu;

    // data exchange
    pData->pSubMenu = pMenu;

    // #112023# Make sure pStartedFrom does not point to invalid (old) data
    if ( pData->pSubMenu )
        pData->pSubMenu->pStartedFrom = nullptr;

    // set native submenu
    if( ImplGetSalMenu() && pData->pSalMenuItem )
    {
        if( pMenu )
            ImplGetSalMenu()->SetSubMenu( pData->pSalMenuItem, pMenu->ImplGetSalMenu(), nPos );
        else
            ImplGetSalMenu()->SetSubMenu( pData->pSalMenuItem, nullptr, nPos );
    }

    oldSubMenu.disposeAndClear();

    ImplCallEventListeners( VclEventId::MenuSubmenuChanged, nPos );
}

PopupMenu* Menu::GetPopupMenu( sal_uInt16 nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        return static_cast<PopupMenu*>(pData->pSubMenu.get());
    else
        return nullptr;
}

void Menu::SetAccelKey( sal_uInt16 nItemId, const KeyCode& rKeyCode )
{
    size_t          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( !pData )
        return;

    if ( pData->aAccelKey == rKeyCode )
        return;

    pData->aAccelKey = rKeyCode;

    // update native menu
    if( ImplGetSalMenu() && pData->pSalMenuItem )
        ImplGetSalMenu()->SetAccelerator( nPos, pData->pSalMenuItem, rKeyCode, rKeyCode.GetName() );
}

KeyCode Menu::GetAccelKey( sal_uInt16 nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        return pData->aAccelKey;
    else
        return KeyCode();
}

KeyEvent Menu::GetActivationKey( sal_uInt16 nItemId ) const
{
    KeyEvent aRet;
    MenuItemData* pData = pItemList->GetData( nItemId );
    if( pData )
    {
        sal_Int32 nPos = pData->aText.indexOf( '~' );
        if( nPos != -1 && nPos < pData->aText.getLength()-1 )
        {
            sal_uInt16 nCode = 0;
            sal_Unicode cAccel = pData->aText[nPos+1];
            if( cAccel >= 'a' && cAccel <= 'z' )
                nCode = KEY_A + (cAccel-'a');
            else if( cAccel >= 'A' && cAccel <= 'Z' )
                nCode = KEY_A + (cAccel-'A');
            else if( cAccel >= '0' && cAccel <= '9' )
                nCode = KEY_0 + (cAccel-'0');

            aRet = KeyEvent( cAccel, KeyCode( nCode, KEY_MOD2 ) );
        }

    }
    return aRet;
}

void Menu::CheckItem( sal_uInt16 nItemId, bool bCheck )
{
    size_t          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( !pData || pData->bChecked == bCheck )
        return;

    // if radio-check, then uncheck previous
    if ( bCheck && (pData->nBits & MenuItemBits::AUTOCHECK) &&
         (pData->nBits & MenuItemBits::RADIOCHECK) )
    {
        MenuItemData*   pGroupData;
        sal_uInt16          nGroupPos;
        sal_uInt16          nItemCount = GetItemCount();
        bool            bFound = false;

        nGroupPos = nPos;
        while ( nGroupPos )
        {
            pGroupData = pItemList->GetDataFromPos( nGroupPos-1 );
            if ( pGroupData->nBits & MenuItemBits::RADIOCHECK )
            {
                if ( IsItemChecked( pGroupData->nId ) )
                {
                    CheckItem( pGroupData->nId, false );
                    bFound = true;
                    break;
                }
            }
            else
                break;
            nGroupPos--;
        }

        if ( !bFound )
        {
            nGroupPos = nPos+1;
            while ( nGroupPos < nItemCount )
            {
                pGroupData = pItemList->GetDataFromPos( nGroupPos );
                if ( pGroupData->nBits & MenuItemBits::RADIOCHECK )
                {
                    if ( IsItemChecked( pGroupData->nId ) )
                    {
                        CheckItem( pGroupData->nId, false );
                        break;
                    }
                }
                else
                    break;
                nGroupPos++;
            }
        }
    }

    pData->bChecked = bCheck;

    // update native menu
    if( ImplGetSalMenu() )
        ImplGetSalMenu()->CheckItem( nPos, bCheck );

    ImplCallEventListeners( bCheck ? VclEventId::MenuItemChecked : VclEventId::MenuItemUnchecked, nPos );
}

bool Menu::IsItemChecked( sal_uInt16 nItemId ) const
{
    size_t          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( !pData )
        return false;

    return pData->bChecked;
}

void Menu::EnableItem( sal_uInt16 nItemId, bool bEnable )
{
    size_t          nPos;
    MenuItemData*   pItemData = pItemList->GetData( nItemId, nPos );

    if ( pItemData && ( pItemData->bEnabled != bEnable ) )
    {
        pItemData->bEnabled = bEnable;

        vcl::Window* pWin = ImplGetWindow();
        if ( pWin && pWin->IsVisible() )
        {
            SAL_WARN_IF(!IsMenuBar(), "vcl", "Menu::EnableItem - Popup visible!" );
            long nX = 0;
            size_t nCount = pItemList->size();
            for ( size_t n = 0; n < nCount; n++ )
            {
                MenuItemData* pData = pItemList->GetDataFromPos( n );
                if ( n == nPos )
                {
                    pWin->Invalidate( Rectangle( Point( nX, 0 ), Size( pData->aSz.Width(), pData->aSz.Height() ) ) );
                    break;
                }
                nX += pData->aSz.Width();
            }
        }
        // update native menu
        if( ImplGetSalMenu() )
            ImplGetSalMenu()->EnableItem( nPos, bEnable );

        ImplCallEventListeners( bEnable ? VclEventId::MenuEnable : VclEventId::MenuDisable, nPos );
    }
}

bool Menu::IsItemEnabled( sal_uInt16 nItemId ) const
{
    size_t          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( !pData )
        return false;

    return pData->bEnabled;
}

void Menu::ShowItem( sal_uInt16 nItemId, bool bVisible )
{
    size_t          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    SAL_WARN_IF(IsMenuBar(), "vcl", "Menu::ShowItem - ignored for menu bar entries!");
    if (!IsMenuBar()&& pData && (pData->bVisible != bVisible))
    {
        vcl::Window* pWin = ImplGetWindow();
        if ( pWin && pWin->IsVisible() )
        {
            SAL_WARN( "vcl", "Menu::ShowItem - ignored for visible popups!" );
            return;
        }
        pData->bVisible = bVisible;

        // update native menu
        if( ImplGetSalMenu() )
            ImplGetSalMenu()->ShowItem( nPos, bVisible );
    }
}

void Menu::SetItemText( sal_uInt16 nItemId, const OUString& rStr )
{
    size_t          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( !pData )
        return;

    if ( !rStr.equals( pData->aText ) )
    {
        pData->aText = rStr;
        ImplSetMenuItemData( pData );
        // update native menu
        if( ImplGetSalMenu() && pData->pSalMenuItem )
            ImplGetSalMenu()->SetItemText( nPos, pData->pSalMenuItem, rStr );

        vcl::Window* pWin = ImplGetWindow();
        delete mpLayoutData;
        mpLayoutData = nullptr;
        if (pWin && IsMenuBar())
        {
            ImplCalcSize( pWin );
            if ( pWin->IsVisible() )
                pWin->Invalidate();
        }

        ImplCallEventListeners( VclEventId::MenuItemTextChanged, nPos );
    }
}

OUString Menu::GetItemText( sal_uInt16 nItemId ) const
{
    size_t          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( pData )
        return pData->aText;

    return OUString();
}

void Menu::SetItemImage( sal_uInt16 nItemId, const Image& rImage )
{
    size_t          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( !pData )
        return;

    pData->aImage = rImage;
    ImplSetMenuItemData( pData );

    // update native menu
    if( ImplGetSalMenu() && pData->pSalMenuItem )
        ImplGetSalMenu()->SetItemImage( nPos, pData->pSalMenuItem, rImage );
}

Image Menu::GetItemImage( sal_uInt16 nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        return pData->aImage;
    else
        return Image();
}

void Menu::SetItemCommand( sal_uInt16 nItemId, const OUString& rCommand )
{
    size_t        nPos;
    MenuItemData* pData = pItemList->GetData( nItemId, nPos );

    if ( pData )
        pData->aCommandStr = rCommand;
}

OUString Menu::GetItemCommand( sal_uInt16 nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if (pData)
        return pData->aCommandStr;

    return OUString();
}

void Menu::SetHelpCommand( sal_uInt16 nItemId, const OUString& rStr )
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        pData->aHelpCommandStr = rStr;
}

OUString Menu::GetHelpCommand( sal_uInt16 nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        return pData->aHelpCommandStr;

    return OUString();
}

void Menu::SetHelpText( sal_uInt16 nItemId, const OUString& rStr )
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        pData->aHelpText = rStr;
}

OUString Menu::ImplGetHelpText( sal_uInt16 nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData && pData->aHelpText.isEmpty() &&
         (( !pData->aHelpId.isEmpty()  ) || ( !pData->aCommandStr.isEmpty() )))
    {
        Help* pHelp = Application::GetHelp();
        if ( pHelp )
        {
            if (!pData->aCommandStr.isEmpty())
                pData->aHelpText = pHelp->GetHelpText( pData->aCommandStr, nullptr );
            if (pData->aHelpText.isEmpty() && !pData->aHelpId.isEmpty())
                pData->aHelpText = pHelp->GetHelpText( OStringToOUString( pData->aHelpId, RTL_TEXTENCODING_UTF8 ), nullptr );
        }
    }

    return OUString();
}

OUString Menu::GetHelpText( sal_uInt16 nItemId ) const
{
    return ImplGetHelpText( nItemId );
}

void Menu::SetTipHelpText( sal_uInt16 nItemId, const OUString& rStr )
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        pData->aTipHelpText = rStr;
}

OUString Menu::GetTipHelpText( sal_uInt16 nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        return pData->aTipHelpText;

    return OUString();
}

void Menu::SetHelpId( sal_uInt16 nItemId, const OString& rHelpId )
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        pData->aHelpId = rHelpId;
}

OString Menu::GetHelpId( sal_uInt16 nItemId ) const
{
    OString aRet;

    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
    {
        if ( !pData->aHelpId.isEmpty() )
            aRet = pData->aHelpId;
        else
            aRet = OUStringToOString( pData->aCommandStr, RTL_TEXTENCODING_UTF8 );
    }

    return aRet;
}

Menu& Menu::operator=( const Menu& rMenu )
{
    // clean up
    Clear();

    // copy items
    sal_uInt16 nCount = rMenu.GetItemCount();
    for ( sal_uInt16 i = 0; i < nCount; i++ )
        ImplCopyItem( this, rMenu, i, MENU_APPEND );

    aActivateHdl = rMenu.aActivateHdl;
    aDeactivateHdl = rMenu.aDeactivateHdl;
    aSelectHdl = rMenu.aSelectHdl;
    aTitleText = rMenu.aTitleText;
    nTitleHeight = rMenu.nTitleHeight;

    return *this;
}

bool Menu::ImplIsVisible( sal_uInt16 nPos ) const
{
    bool bVisible = true;

    MenuItemData* pData = pItemList->GetDataFromPos( nPos );
    // check general visibility first
    if( pData && !pData->bVisible )
        bVisible = false;

    if ( bVisible && pData && pData->eType == MenuItemType::SEPARATOR )
    {
        if( nPos == 0 ) // no separator should be shown at the very beginning
            bVisible = false;
        else
        {
            // always avoid adjacent separators
            size_t nCount = pItemList->size();
            size_t n;
            MenuItemData* pNextData = nullptr;
            // search next visible item
            for( n = nPos + 1; n < nCount; n++ )
            {
                pNextData = pItemList->GetDataFromPos( n );
                if( pNextData && pNextData->bVisible )
                {
                    if( pNextData->eType == MenuItemType::SEPARATOR || ImplIsVisible(n) )
                        break;
                }
            }
            if( n == nCount ) // no next visible item
                bVisible = false;
            // check for separator
            if( pNextData && pNextData->bVisible && pNextData->eType == MenuItemType::SEPARATOR )
                bVisible = false;

            if( bVisible )
            {
                for( n = nPos; n > 0; n-- )
                {
                    pNextData = pItemList->GetDataFromPos( n-1 );
                    if( pNextData && pNextData->bVisible )
                    {
                        if( pNextData->eType != MenuItemType::SEPARATOR && ImplIsVisible(n-1) )
                            break;
                    }
                }
                if( n == 0 ) // no previous visible item
                    bVisible = false;
            }
        }
    }

    // not allowed for menubar, as I do not know
    // whether a menu-entry will disappear or will appear
    if (bVisible && !IsMenuBar() && (nMenuFlags & MenuFlags::HideDisabledEntries) &&
        !(nMenuFlags & MenuFlags::AlwaysShowDisabledEntries))
    {
        if( !pData ) // e.g. nPos == ITEMPOS_INVALID
            bVisible = false;
        else if ( pData->eType != MenuItemType::SEPARATOR ) // separators handled above
        {
            // tdf#86850 Always display clipboard functions
            if ( pData->nId == SID_CUT || pData->nId == SID_COPY || pData->nId == SID_PASTE ||
                 pData->aCommandStr == ".uno:Cut" || pData->aCommandStr == ".uno:Copy" || pData->aCommandStr == ".uno:Paste" )
                bVisible = true;
            else
                // bVisible = pData->bEnabled && ( !pData->pSubMenu || pData->pSubMenu->HasValidEntries( true ) );
                bVisible = pData->bEnabled; // do not check submenus as they might be filled at Activate().
        }
    }

    return bVisible;
}

bool Menu::IsItemPosVisible( sal_uInt16 nItemPos ) const
{
    return IsMenuVisible() && ImplIsVisible( nItemPos );
}

bool Menu::IsMenuVisible() const
{
    return pWindow && pWindow->IsReallyVisible();
}

bool Menu::ImplIsSelectable( sal_uInt16 nPos ) const
{
    bool bSelectable = true;

    MenuItemData* pData = pItemList->GetDataFromPos( nPos );
    // check general visibility first
    if ( pData && ( pData->nBits & MenuItemBits::NOSELECT ) )
        bSelectable = false;

    return bSelectable;
}

css::uno::Reference<css::accessibility::XAccessible> Menu::GetAccessible()
{
    // Since PopupMenu are sometimes shared by different instances of MenuBar, the mxAccessible member gets
    // overwritten and may contain a disposed object when the initial menubar gets set again. So use the
    // mxAccessible member only for sub menus.
    if ( pStartedFrom )
    {
        for ( sal_uInt16 i = 0, nCount = pStartedFrom->GetItemCount(); i < nCount; ++i )
        {
            sal_uInt16 nItemId = pStartedFrom->GetItemId( i );
            if ( static_cast< Menu* >( pStartedFrom->GetPopupMenu( nItemId ) ) == this )
            {
                css::uno::Reference<css::accessibility::XAccessible> xParent = pStartedFrom->GetAccessible();
                if ( xParent.is() )
                {
                    css::uno::Reference<css::accessibility::XAccessibleContext> xParentContext( xParent->getAccessibleContext() );
                    if (xParentContext.is())
                        return xParentContext->getAccessibleChild( i );
                }
            }
        }
    }
    else if ( !mxAccessible.is() )
    {
        UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
        if ( pWrapper )
            mxAccessible = pWrapper->CreateAccessible(this, IsMenuBar());
    }

    return mxAccessible;
}

void Menu::SetAccessible(const css::uno::Reference<css::accessibility::XAccessible>& rxAccessible )
{
    mxAccessible = rxAccessible;
}

Size Menu::ImplGetNativeCheckAndRadioSize(vcl::RenderContext& rRenderContext, long& rCheckHeight, long& rRadioHeight ) const
{
    long nCheckWidth = 0, nRadioWidth = 0;
    rCheckHeight = rRadioHeight = 0;

    if (!IsMenuBar())
    {
        ImplControlValue aVal;
        Rectangle aNativeBounds;
        Rectangle aNativeContent;

        Rectangle aCtrlRegion(Rectangle(Point(), Size(100, 15)));
        if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::MenuItemCheckMark))
        {
            if (rRenderContext.GetNativeControlRegion(ControlType::MenuPopup, ControlPart::MenuItemCheckMark,
                                              aCtrlRegion, ControlState::ENABLED, aVal, OUString(),
                                              aNativeBounds, aNativeContent))
            {
                rCheckHeight = aNativeBounds.GetHeight();
                nCheckWidth = aNativeContent.GetWidth();
            }
        }
        if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::MenuItemRadioMark))
        {
            if (rRenderContext.GetNativeControlRegion(ControlType::MenuPopup, ControlPart::MenuItemRadioMark,
                                                      aCtrlRegion, ControlState::ENABLED, aVal, OUString(),
                                                      aNativeBounds, aNativeContent))
            {
                rRadioHeight = aNativeBounds.GetHeight();
                nRadioWidth = aNativeContent.GetWidth();
            }
        }
    }
    return Size(std::max(nCheckWidth, nRadioWidth), std::max(rCheckHeight, rRadioHeight));
}

bool Menu::ImplGetNativeSubmenuArrowSize(vcl::RenderContext& rRenderContext, Size& rArrowSize, long& rArrowSpacing)
{
    ImplControlValue aVal;
    Rectangle aNativeBounds;
    Rectangle aNativeContent;
    Rectangle aCtrlRegion(Rectangle(Point(), Size(100, 15)));
    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::SubmenuArrow))
    {
        if (rRenderContext.GetNativeControlRegion(ControlType::MenuPopup, ControlPart::SubmenuArrow,
                                                  aCtrlRegion, ControlState::ENABLED,
                                                  aVal, OUString(), aNativeBounds, aNativeContent))
        {
            Size aSize(aNativeContent.GetWidth(), aNativeContent.GetHeight());
            rArrowSize = aSize;
            rArrowSpacing = aNativeBounds.GetWidth() - aNativeContent.GetWidth();
            return true;
        }
    }
    return false;
}

void Menu::ImplAddDel( ImplMenuDelData& rDel )
{
    SAL_WARN_IF( rDel.mpMenu, "vcl", "Menu::ImplAddDel(): cannot add ImplMenuDelData twice !" );
    if( !rDel.mpMenu )
    {
        rDel.mpMenu = this;
        rDel.mpNext = mpFirstDel;
        mpFirstDel = &rDel;
    }
}

void Menu::ImplRemoveDel( ImplMenuDelData& rDel )
{
    rDel.mpMenu = nullptr;
    if ( mpFirstDel == &rDel )
    {
        mpFirstDel = rDel.mpNext;
    }
    else
    {
        ImplMenuDelData* pData = mpFirstDel;
        while ( pData && (pData->mpNext != &rDel) )
            pData = pData->mpNext;

        SAL_WARN_IF( !pData, "vcl", "Menu::ImplRemoveDel(): ImplMenuDelData not registered !" );
        if( pData )
            pData->mpNext = rDel.mpNext;
    }
}

Size Menu::ImplCalcSize( vcl::Window* pWin )
{
    // | Check/Radio/Image| Text| Accel/Popup|

    // for symbols: nFontHeight x nFontHeight
    long nFontHeight = pWin->GetTextHeight();
    long nExtra = nFontHeight/4;

    long nMinMenuItemHeight = nFontHeight;
    long nCheckHeight = 0, nRadioHeight = 0;
    Size aMaxSize = ImplGetNativeCheckAndRadioSize(*pWin, nCheckHeight, nRadioHeight); // FIXME
    if( aMaxSize.Height() > nMinMenuItemHeight )
        nMinMenuItemHeight = aMaxSize.Height();

    Size aMaxImgSz;

    const StyleSettings& rSettings = pWin->GetSettings().GetStyleSettings();
    if ( rSettings.GetUseImagesInMenus() )
    {
        if ( 16 > nMinMenuItemHeight )
            nMinMenuItemHeight = 16;
        for ( size_t i = pItemList->size(); i; )
        {
            MenuItemData* pData = pItemList->GetDataFromPos( --i );
            if ( ImplIsVisible( i )
               && (  ( pData->eType == MenuItemType::IMAGE )
                  || ( pData->eType == MenuItemType::STRINGIMAGE )
                  )
               )
            {
                Size aImgSz = pData->aImage.GetSizePixel();
                if ( aImgSz.Height() > aMaxImgSz.Height() )
                    aMaxImgSz.Height() = aImgSz.Height();
                if ( aImgSz.Height() > nMinMenuItemHeight )
                    nMinMenuItemHeight = aImgSz.Height();
                break;
            }
        }
    }

    Size aSz;
    long nCheckWidth = 0;
    long nMaxWidth = 0;

    for ( size_t n = pItemList->size(); n; )
    {
        MenuItemData* pData = pItemList->GetDataFromPos( --n );

        pData->aSz.Height() = 0;
        pData->aSz.Width() = 0;

        if ( ImplIsVisible( n ) )
        {
            long nWidth = 0;

            // Separator
            if (!IsMenuBar()&& (pData->eType == MenuItemType::SEPARATOR))
            {
                //Useless: SAL_WARN_IF( IsMenuBar(), "vcl", "Separator in MenuBar ?! " );
                pData->aSz.Height() = 4;
            }

            // Image:
            if (!IsMenuBar() && ((pData->eType == MenuItemType::IMAGE) || (pData->eType == MenuItemType::STRINGIMAGE)))
            {
                Size aImgSz = pData->aImage.GetSizePixel();

                aImgSz.Height() += 4; // add a border for native marks
                aImgSz.Width() += 4; // add a border for native marks
                if ( aImgSz.Width() > aMaxImgSz.Width() )
                    aMaxImgSz.Width() = aImgSz.Width();
                if ( aImgSz.Height() > aMaxImgSz.Height() )
                    aMaxImgSz.Height() = aImgSz.Height();
                if ( aImgSz.Height() > pData->aSz.Height() )
                    pData->aSz.Height() = aImgSz.Height();
            }

            // Check Buttons:
            if (!IsMenuBar() && pData->HasCheck())
            {
                nCheckWidth = aMaxSize.Width();
                // checks / images take the same place
                if( ! ( ( pData->eType == MenuItemType::IMAGE ) || ( pData->eType == MenuItemType::STRINGIMAGE ) ) )
                    nWidth += nCheckWidth + nExtra * 2;
            }

            // Text:
            if ( (pData->eType == MenuItemType::STRING) || (pData->eType == MenuItemType::STRINGIMAGE) )
            {
                long nTextWidth = pWin->GetCtrlTextWidth( pData->aText );
                long nTextHeight = pWin->GetTextHeight();

                if (IsMenuBar())
                {
                    if ( nTextHeight > pData->aSz.Height() )
                        pData->aSz.Height() = nTextHeight;

                    pData->aSz.Width() = nTextWidth + 4*nExtra;
                    aSz.Width() += pData->aSz.Width();
                }
                else
                    pData->aSz.Height() = std::max( std::max( nTextHeight, pData->aSz.Height() ), nMinMenuItemHeight );

                nWidth += nTextWidth;
            }

            // Accel
            if (!IsMenuBar()&& pData->aAccelKey.GetCode() && !ImplAccelDisabled())
            {
                OUString aName = pData->aAccelKey.GetName();
                long nAccWidth = pWin->GetTextWidth( aName );
                nAccWidth += nExtra;
                nWidth += nAccWidth;
            }

            // SubMenu?
            if (!IsMenuBar() && pData->pSubMenu)
            {
                    if ( nFontHeight > nWidth )
                        nWidth += nFontHeight;

                pData->aSz.Height() = std::max( std::max( nFontHeight, pData->aSz.Height() ), nMinMenuItemHeight );
            }

            pData->aSz.Height() += EXTRAITEMHEIGHT; // little bit more distance

            if (!IsMenuBar())
                aSz.Height() += (long)pData->aSz.Height();

            if ( nWidth > nMaxWidth )
                nMaxWidth = nWidth;

        }
    }

    // Additional space for title
    nTitleHeight = 0;
    if (!IsMenuBar() && aTitleText.getLength() > 0) {
        // Set expected font
        pWin->Push(PushFlags::FONT);
        vcl::Font aFont = pWin->GetFont();
        aFont.SetWeight(WEIGHT_BOLD);
        pWin->SetFont(aFont);

        // Compute text bounding box
        Rectangle aTextBoundRect;
        pWin->GetTextBoundRect(aTextBoundRect, aTitleText);

        // Vertically, one height of char + extra space for decoration
        nTitleHeight =  aTextBoundRect.GetSize().Height() + 4 * SPACE_AROUND_TITLE ;
        aSz.Height() += nTitleHeight;

        long nWidth = aTextBoundRect.GetSize().Width() + 4 * SPACE_AROUND_TITLE;
        pWin->Pop();
        if ( nWidth > nMaxWidth )
            nMaxWidth = nWidth;
    }

    if (!IsMenuBar())
    {
        // popup menus should not be wider than half the screen
        // except on rather small screens
        // TODO: move GetScreenNumber from SystemWindow to Window ?
        // currently we rely on internal privileges
        unsigned int nDisplayScreen = pWin->ImplGetWindowImpl()->mpFrame->maGeometry.nDisplayScreenNumber;
        Rectangle aDispRect( Application::GetScreenPosSizePixel( nDisplayScreen ) );
        long nScreenWidth = aDispRect.GetWidth() >= 800 ? aDispRect.GetWidth() : 800;
        if( nMaxWidth > nScreenWidth/2 )
            nMaxWidth = nScreenWidth/2;

        sal_uInt16 gfxExtra = (sal_uInt16) std::max( nExtra, 7L ); // #107710# increase space between checkmarks/images/text
        nImgOrChkPos = (sal_uInt16)nExtra;
        long nImgOrChkWidth = 0;
        if( aMaxSize.Height() > 0 ) // NWF case
            nImgOrChkWidth = aMaxSize.Height() + nExtra;
        else // non NWF case
            nImgOrChkWidth = nFontHeight/2 + gfxExtra;
        nImgOrChkWidth = std::max( nImgOrChkWidth, aMaxImgSz.Width() + gfxExtra );
        nTextPos = (sal_uInt16)(nImgOrChkPos + nImgOrChkWidth);
        nTextPos = nTextPos + gfxExtra;

        aSz.Width() = nTextPos + nMaxWidth + nExtra;
        aSz.Width() += 4*nExtra;   // a _little_ more ...

        aSz.Width() += 2*ImplGetSVData()->maNWFData.mnMenuFormatBorderX;
        aSz.Height() += 2*ImplGetSVData()->maNWFData.mnMenuFormatBorderY;
    }
    else
    {
        nTextPos = (sal_uInt16)(2*nExtra);
        aSz.Height() = nFontHeight+6;

        // get menubar height from native methods if supported
        if( pWindow->IsNativeControlSupported( ControlType::Menubar, ControlPart::Entire ) )
        {
            ImplControlValue aVal;
            Rectangle aNativeBounds;
            Rectangle aNativeContent;
            Point tmp( 0, 0 );
            Rectangle aCtrlRegion( tmp, Size( 100, 15 ) );
            if( pWindow->GetNativeControlRegion( ControlType::Menubar,
                                                 ControlPart::Entire,
                                                 aCtrlRegion,
                                                 ControlState::ENABLED,
                                                 aVal,
                                                 OUString(),
                                                 aNativeBounds,
                                                 aNativeContent )
            )
            {
                int nNativeHeight = aNativeBounds.GetHeight();
                if( nNativeHeight > aSz.Height() )
                    aSz.Height() = nNativeHeight;
            }
        }

        // account for the size of the close button, which actually is a toolbox
        // due to NWF this is variable
        long nCloseButtonHeight = static_cast<MenuBarWindow*>(pWindow.get())->MinCloseButtonSize().Height();
        if (aSz.Height() < nCloseButtonHeight)
            aSz.Height() = nCloseButtonHeight;
    }

    if ( pLogo )
        aSz.Width() += pLogo->aBitmap.GetSizePixel().Width();

    return aSz;
}

static void ImplPaintCheckBackground(vcl::RenderContext& rRenderContext, vcl::Window& rWindow, const Rectangle& i_rRect, bool i_bHighlight)
{
    bool bNativeOk = false;
    if (rRenderContext.IsNativeControlSupported(ControlType::Toolbar, ControlPart::Button))
    {
        ImplControlValue    aControlValue;
        Rectangle           aCtrlRegion( i_rRect );
        ControlState        nState = ControlState::PRESSED | ControlState::ENABLED;

        aControlValue.setTristateVal(ButtonValue::On);

        bNativeOk = rRenderContext.DrawNativeControl(ControlType::Toolbar, ControlPart::Button,
                                                     aCtrlRegion, nState, aControlValue,
                                                     OUString());
    }

    if (!bNativeOk)
    {
        const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();
        Color aColor( i_bHighlight ? rSettings.GetMenuHighlightTextColor() : rSettings.GetHighlightColor() );
        RenderTools::DrawSelectionBackground(rRenderContext, rWindow, i_rRect, 0, i_bHighlight, true, false, nullptr, 2, &aColor);
    }
}

static OUString getShortenedString( const OUString& i_rLong, vcl::RenderContext& rRenderContext, long i_nMaxWidth )
{
    sal_Int32 nPos = -1;
    OUString aNonMnem(OutputDevice::GetNonMnemonicString(i_rLong, nPos));
    aNonMnem = rRenderContext.GetEllipsisString( aNonMnem, i_nMaxWidth, DrawTextFlags::CenterEllipsis);
    // re-insert mnemonic
    if (nPos != -1)
    {
        if (nPos < aNonMnem.getLength() && i_rLong[nPos+1] == aNonMnem[nPos])
        {
            OUStringBuffer aBuf( i_rLong.getLength() );
            aBuf.append( aNonMnem.copy( 0, nPos) );
            aBuf.append( '~' );
            aBuf.append( aNonMnem.copy(nPos) );
            aNonMnem = aBuf.makeStringAndClear();
        }
    }
    return aNonMnem;
}

void Menu::ImplPaintMenuTitle(vcl::RenderContext& rRenderContext, const Rectangle& rRect ) const
{
    // Save previous graphical settings, set new one
    rRenderContext.Push(PushFlags::FONT | PushFlags::FILLCOLOR);
    Wallpaper aOldBackground = rRenderContext.GetBackground();

    Color aBackgroundColor = rRenderContext.GetSettings().GetStyleSettings().GetMenuBarColor();
    rRenderContext.SetBackground(Wallpaper(aBackgroundColor));
    rRenderContext.SetFillColor(aBackgroundColor);
    vcl::Font aFont = rRenderContext.GetFont();
    aFont.SetWeight(WEIGHT_BOLD);
    rRenderContext.SetFont(aFont);

    // Draw background rectangle
    Rectangle aBgRect(rRect);
    int nOuterSpaceX = ImplGetSVData()->maNWFData.mnMenuFormatBorderX;
    aBgRect.setX(aBgRect.getX() + SPACE_AROUND_TITLE);
    aBgRect.setWidth(aBgRect.getWidth() - 2 * SPACE_AROUND_TITLE - 2 * nOuterSpaceX);
    aBgRect.setY(aBgRect.getY() + SPACE_AROUND_TITLE);
    aBgRect.setHeight(nTitleHeight - 2 * SPACE_AROUND_TITLE);
    rRenderContext.DrawRect(aBgRect);

    // Draw the text centered
    Point aTextTopLeft(aBgRect.TopLeft());
    Rectangle aTextBoundRect;
    rRenderContext.GetTextBoundRect( aTextBoundRect, aTitleText );
    aTextTopLeft.X() += (aBgRect.getWidth() - aTextBoundRect.GetSize().Width()) / 2;
    aTextTopLeft.Y() += (aBgRect.GetHeight() - aTextBoundRect.GetSize().Height()) / 2
                        - aTextBoundRect.TopLeft().Y();
    rRenderContext.DrawText(aTextTopLeft, aTitleText, 0, aTitleText.getLength());

    // Restore
    rRenderContext.Pop();
    rRenderContext.SetBackground(aOldBackground);
}

void Menu::ImplPaint(vcl::RenderContext& rRenderContext,
                     sal_uInt16 nBorder, long nStartY, MenuItemData* pThisItemOnly,
                     bool bHighlighted, bool bLayout, bool bRollover) const
{
    // for symbols: nFontHeight x nFontHeight
    long nFontHeight = rRenderContext.GetTextHeight();
    long nExtra = nFontHeight / 4;

    long nCheckHeight = 0, nRadioHeight = 0;
    ImplGetNativeCheckAndRadioSize(rRenderContext, nCheckHeight, nRadioHeight);

    DecorationView aDecoView(&rRenderContext);
    const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();

    Point aTopLeft, aTmpPos;

    if (pLogo)
        aTopLeft.X() = pLogo->aBitmap.GetSizePixel().Width();

    int nOuterSpaceX = 0;
    if (!IsMenuBar())
    {
        nOuterSpaceX = ImplGetSVData()->maNWFData.mnMenuFormatBorderX;
        aTopLeft.X() += nOuterSpaceX;
        aTopLeft.Y() += ImplGetSVData()->maNWFData.mnMenuFormatBorderY;
    }

    // for the computations, use size of the underlying window, not of RenderContext
    Size aOutSz = pWindow->GetOutputSizePixel();

    size_t nCount = pItemList->size();
    if (bLayout)
        mpLayoutData->m_aVisibleItemBoundRects.clear();

    // Paint title
    if (!pThisItemOnly && !IsMenuBar() && nTitleHeight > 0)
        ImplPaintMenuTitle(rRenderContext, Rectangle(aTopLeft, aOutSz));

    for (size_t n = 0; n < nCount; n++)
    {
        MenuItemData* pData = pItemList->GetDataFromPos( n );
        if (ImplIsVisible(n) && (!pThisItemOnly || (pData == pThisItemOnly)))
        {
            if (pThisItemOnly)
            {
                if (IsMenuBar())
                {
                    if (!ImplGetSVData()->maNWFData.mbRolloverMenubar)
                    {
                        if (bRollover)
                            rRenderContext.SetTextColor(rSettings.GetMenuBarRolloverTextColor());
                        else if (bHighlighted)
                            rRenderContext.SetTextColor(rSettings.GetMenuBarHighlightTextColor());
                    }
                    else
                    {
                        if (bHighlighted)
                            rRenderContext.SetTextColor(rSettings.GetMenuBarHighlightTextColor());
                        else if (bRollover)
                            rRenderContext.SetTextColor(rSettings.GetMenuBarRolloverTextColor());
                    }
                    if (!bRollover && !bHighlighted)
                        rRenderContext.SetTextColor(rSettings.GetMenuBarTextColor());
                }
                else if (bHighlighted)
                    rRenderContext.SetTextColor(rSettings.GetMenuHighlightTextColor());
            }

            Point aPos(aTopLeft);
            aPos.Y() += nBorder;
            aPos.Y() += nStartY;

            if (aPos.Y() >= 0)
            {
                long nTextOffsetY = ((pData->aSz.Height() - nFontHeight) / 2);
                if (IsMenuBar())
                    nTextOffsetY += (aOutSz.Height()-pData->aSz.Height()) / 2;
                DrawTextFlags   nTextStyle   = DrawTextFlags::NONE;
                DrawSymbolFlags nSymbolStyle = DrawSymbolFlags::NONE;
                DrawImageFlags  nImageStyle  = DrawImageFlags::NONE;

                // submenus without items are not disabled when no items are
                // contained. The application itself should check for this!
                // Otherwise it could happen entries are disabled due to
                // asynchronous loading
                if (!pData->bEnabled || !pWindow->IsEnabled())
                {
                    nTextStyle   |= DrawTextFlags::Disable;
                    nSymbolStyle |= DrawSymbolFlags::Disable;
                    nImageStyle  |= DrawImageFlags::Disable;
                }

                // Separator
                if (!bLayout && !IsMenuBar() && (pData->eType == MenuItemType::SEPARATOR))
                {
                    bool bNativeOk = false;
                    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::Separator))
                    {
                        ControlState nState = ControlState::NONE;
                        if (pData->bEnabled && pWindow->IsEnabled())
                            nState |= ControlState::ENABLED;
                        if (bHighlighted)
                            nState |= ControlState::SELECTED;
                        int nSepPad = ImplGetSVData()->maNWFData.mnMenuSeparatorBorderX;
                        Point aMpos (aPos);
                        aMpos.X() += nSepPad;
                        Size aSz(pData->aSz);
                        aSz.Width() = aOutSz.Width() - 2*nOuterSpaceX - 2 * nSepPad;
                        Rectangle aItemRect(aMpos, aSz);
                        MenupopupValue aVal(nTextPos - GUTTERBORDER, aItemRect);
                        bNativeOk = rRenderContext.DrawNativeControl(ControlType::MenuPopup, ControlPart::Separator,
                                                                     aItemRect, nState, aVal, OUString());
                    }
                    if (!bNativeOk)
                    {
                        aTmpPos.Y() = aPos.Y() + ((pData->aSz.Height() - 2) / 2);
                        aTmpPos.X() = aPos.X() + 2 + nOuterSpaceX;
                        rRenderContext.SetLineColor(rSettings.GetShadowColor());
                        rRenderContext.DrawLine(aTmpPos, Point(aOutSz.Width() - 3 - 2 * nOuterSpaceX, aTmpPos.Y()));
                        aTmpPos.Y()++;
                        rRenderContext.SetLineColor(rSettings.GetLightColor());
                        rRenderContext.DrawLine(aTmpPos, Point(aOutSz.Width() - 3 - 2 * nOuterSpaceX, aTmpPos.Y()));
                        rRenderContext.SetLineColor();
                    }
                }

                Rectangle aOuterCheckRect(Point(aPos.X()+nImgOrChkPos, aPos.Y()),
                                          Size(pData->aSz.Height(), pData->aSz.Height()));
                aOuterCheckRect.Left() += 1;
                aOuterCheckRect.Right() -= 1;
                aOuterCheckRect.Top() += 1;
                aOuterCheckRect.Bottom() -= 1;

                // CheckMark
                if (!bLayout && !IsMenuBar() && pData->HasCheck())
                {
                    // draw selection transparent marker if checked
                    // onto that either a checkmark or the item image
                    // will be painted
                    // however do not do this if native checks will be painted since
                    // the selection color too often does not fit the theme's check and/or radio

                    if( !((pData->eType == MenuItemType::IMAGE) || (pData->eType == MenuItemType::STRINGIMAGE)))
                    {
                        if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup,
                                                                    (pData->nBits & MenuItemBits::RADIOCHECK)
                                                                        ? ControlPart::MenuItemCheckMark
                                                                        : ControlPart::MenuItemRadioMark))
                        {
                            ControlPart nPart = ((pData->nBits & MenuItemBits::RADIOCHECK)
                                                 ? ControlPart::MenuItemRadioMark
                                                 : ControlPart::MenuItemCheckMark);

                            ControlState nState = ControlState::NONE;

                            if (pData->bChecked)
                                nState |= ControlState::PRESSED;

                            if (pData->bEnabled && pWindow->IsEnabled())
                                nState |= ControlState::ENABLED;

                            if (bHighlighted)
                                nState |= ControlState::SELECTED;

                            long nCtrlHeight = (pData->nBits & MenuItemBits::RADIOCHECK) ? nCheckHeight : nRadioHeight;
                            aTmpPos.X() = aOuterCheckRect.Left() + (aOuterCheckRect.GetWidth() - nCtrlHeight) / 2;
                            aTmpPos.Y() = aOuterCheckRect.Top() + (aOuterCheckRect.GetHeight() - nCtrlHeight) / 2;

                            Rectangle aCheckRect(aTmpPos, Size(nCtrlHeight, nCtrlHeight));
                            Size aSz(pData->aSz);
                            aSz.Width() = aOutSz.Width() - 2 * nOuterSpaceX;
                            Rectangle aItemRect(aPos, aSz);
                            MenupopupValue aVal(nTextPos - GUTTERBORDER, aItemRect);
                            rRenderContext.DrawNativeControl(ControlType::MenuPopup, nPart, aCheckRect,
                                                             nState, aVal, OUString());
                        }
                        else if (pData->bChecked) // by default do nothing for unchecked items
                        {
                            ImplPaintCheckBackground(rRenderContext, *pWindow, aOuterCheckRect, pThisItemOnly && bHighlighted);

                            SymbolType eSymbol;
                            Size aSymbolSize;
                            if (pData->nBits & MenuItemBits::RADIOCHECK)
                            {
                                eSymbol = SymbolType::RADIOCHECKMARK;
                                aSymbolSize = Size(nFontHeight / 2, nFontHeight / 2);
                            }
                            else
                            {
                                eSymbol = SymbolType::CHECKMARK;
                                aSymbolSize = Size((nFontHeight * 25) / 40, nFontHeight / 2);
                            }
                            aTmpPos.X() = aOuterCheckRect.Left() + (aOuterCheckRect.GetWidth() - aSymbolSize.Width()) / 2;
                            aTmpPos.Y() = aOuterCheckRect.Top() + (aOuterCheckRect.GetHeight() - aSymbolSize.Height()) / 2;
                            Rectangle aRect(aTmpPos, aSymbolSize);
                            aDecoView.DrawSymbol(aRect, eSymbol, rRenderContext.GetTextColor(), nSymbolStyle);
                        }
                    }
                }

                // Image:
                if (!bLayout && !IsMenuBar() && ((pData->eType == MenuItemType::IMAGE) || (pData->eType == MenuItemType::STRINGIMAGE)))
                {
                    // Don't render an image for a check thing
                    if (pData->bChecked)
                        ImplPaintCheckBackground(rRenderContext, *pWindow, aOuterCheckRect, pThisItemOnly && bHighlighted);

                    Image aImage = pData->aImage;

                    aTmpPos = aOuterCheckRect.TopLeft();
                    aTmpPos.X() += (aOuterCheckRect.GetWidth() - aImage.GetSizePixel().Width()) / 2;
                    aTmpPos.Y() += (aOuterCheckRect.GetHeight() - aImage.GetSizePixel().Height()) / 2;
                    rRenderContext.DrawImage(aTmpPos, aImage, nImageStyle);
                }

                // Text:
                if ((pData->eType == MenuItemType::STRING ) || (pData->eType == MenuItemType::STRINGIMAGE))
                {
                    aTmpPos.X() = aPos.X() + nTextPos;
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += nTextOffsetY;
                    DrawTextFlags nStyle = nTextStyle | DrawTextFlags::Mnemonic;

                    const Menu *pMenu = this;
                    while (!pMenu->IsMenuBar() && pMenu->pStartedFrom)
                        pMenu = pMenu->pStartedFrom;
                    if (pMenu->IsMenuBar() && (static_cast<MenuBarWindow*>(pMenu->pWindow.get()))->GetMBWHideAccel())
                        nStyle |= DrawTextFlags::HideMnemonic;

                    if (pData->bIsTemporary)
                        nStyle |= DrawTextFlags::Disable;
                    MetricVector* pVector = bLayout ? &mpLayoutData->m_aUnicodeBoundRects : nullptr;
                    OUString* pDisplayText = bLayout ? &mpLayoutData->m_aDisplayText : nullptr;
                    if (bLayout)
                    {
                        mpLayoutData->m_aLineIndices.push_back(mpLayoutData->m_aDisplayText.getLength());
                        mpLayoutData->m_aLineItemIds.push_back(pData->nId);
                        mpLayoutData->m_aLineItemPositions.push_back(n);
                    }
                    // #i47946# with NWF painted menus the background is transparent
                    // since DrawCtrlText can depend on the background (e.g. for
                    // DrawTextFlags::Disable), temporarily set a background which
                    // hopefully matches the NWF background since it is read
                    // from the system style settings
                    bool bSetTmpBackground = !rRenderContext.IsBackground()
                                           && rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::Entire);
                    if (bSetTmpBackground)
                    {
                        Color aBg = IsMenuBar() ? rRenderContext.GetSettings().GetStyleSettings().GetMenuBarColor()
                                                : rRenderContext.GetSettings().GetStyleSettings().GetMenuColor();
                        rRenderContext.SetBackground(Wallpaper(aBg));
                    }
                    // how much space is there for the text ?
                    long nMaxItemTextWidth = aOutSz.Width() - aTmpPos.X() - nExtra - nOuterSpaceX;
                    if (!IsMenuBar() && pData->aAccelKey.GetCode() && !ImplAccelDisabled())
                    {
                        OUString aAccText = pData->aAccelKey.GetName();
                        nMaxItemTextWidth -= rRenderContext.GetTextWidth(aAccText) + 3 * nExtra;
                    }
                    if (!IsMenuBar() && pData->pSubMenu)
                    {
                        nMaxItemTextWidth -= nFontHeight - nExtra;
                    }
                    OUString aItemText(getShortenedString(pData->aText, rRenderContext, nMaxItemTextWidth));
                    rRenderContext.DrawCtrlText(aTmpPos, aItemText, 0, aItemText.getLength(), nStyle, pVector, pDisplayText);
                    if (bSetTmpBackground)
                        rRenderContext.SetBackground();
                }

                // Accel
                if (!bLayout && !IsMenuBar() && pData->aAccelKey.GetCode() && !ImplAccelDisabled())
                {
                    OUString aAccText = pData->aAccelKey.GetName();
                    aTmpPos.X() = aOutSz.Width() - rRenderContext.GetTextWidth(aAccText);
                    aTmpPos.X() -= 4 * nExtra;

                    aTmpPos.X() -= nOuterSpaceX;
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += nTextOffsetY;
                    rRenderContext.DrawCtrlText(aTmpPos, aAccText, 0, aAccText.getLength(), nTextStyle);
                }

                // SubMenu?
                if (!bLayout && !IsMenuBar() && pData->pSubMenu)
                {
                    bool bNativeOk = false;
                    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::SubmenuArrow))
                    {
                        ControlState nState = ControlState::NONE;
                        Size aTmpSz(0, 0);
                        long aSpacing = 0;

                        if (!ImplGetNativeSubmenuArrowSize(rRenderContext, aTmpSz, aSpacing))
                        {
                            aTmpSz = Size(nFontHeight, nFontHeight);
                            aSpacing = nOuterSpaceX;
                        }

                        if (pData->bEnabled && pWindow->IsEnabled())
                            nState |= ControlState::ENABLED;
                        if (bHighlighted)
                            nState |= ControlState::SELECTED;

                        aTmpPos.X() = aOutSz.Width() - aTmpSz.Width() - aSpacing - nOuterSpaceX;
                        aTmpPos.Y() = aPos.Y() + ( pData->aSz.Height() - aTmpSz.Height() ) / 2;
                        aTmpPos.Y() += nExtra / 2;

                        Rectangle aItemRect(aTmpPos, aTmpSz);
                        MenupopupValue aVal(nTextPos - GUTTERBORDER, aItemRect);
                        bNativeOk = rRenderContext.DrawNativeControl(ControlType::MenuPopup, ControlPart::SubmenuArrow,
                                                                     aItemRect, nState, aVal, OUString());
                    }
                    if (!bNativeOk)
                    {
                        aTmpPos.X() = aOutSz.Width() - nFontHeight + nExtra - nOuterSpaceX;
                        aTmpPos.Y() = aPos.Y();
                        aTmpPos.Y() += nExtra/2;
                        aTmpPos.Y() += (pData->aSz.Height() / 2) - (nFontHeight / 4);
                        if (pData->nBits & MenuItemBits::POPUPSELECT)
                        {
                            rRenderContext.SetTextColor(rSettings.GetMenuTextColor());
                            Point aTmpPos2(aPos);
                            aTmpPos2.X() = aOutSz.Width() - nFontHeight - nFontHeight/4;
                            aDecoView.DrawFrame(Rectangle(aTmpPos2, Size(nFontHeight + nFontHeight / 4,
                                                                         pData->aSz.Height())),
                                                DrawFrameStyle::Group);
                        }
                        aDecoView.DrawSymbol(Rectangle(aTmpPos, Size(nFontHeight / 2, nFontHeight / 2)),
                                             SymbolType::SPIN_RIGHT, rRenderContext.GetTextColor(), nSymbolStyle);
                    }
                }

                if (pThisItemOnly && bHighlighted)
                {
                    // This restores the normal menu or menu bar text
                    // color for when it is no longer highlighted.
                    if (IsMenuBar())
                        rRenderContext.SetTextColor(rSettings.GetMenuBarTextColor());
                    else
                        rRenderContext.SetTextColor(rSettings.GetMenuTextColor());
                }
            }
            if( bLayout )
            {
                if (!IsMenuBar())
                    mpLayoutData->m_aVisibleItemBoundRects[ n ] = Rectangle(aTopLeft, Size(aOutSz.Width(), pData->aSz.Height()));
                else
                    mpLayoutData->m_aVisibleItemBoundRects[ n ] = Rectangle(aTopLeft, pData->aSz);
            }
        }

        if (!IsMenuBar())
            aTopLeft.Y() += pData->aSz.Height();
        else
            aTopLeft.X() += pData->aSz.Width();
    }

    if (!bLayout && !pThisItemOnly && pLogo)
    {
        Size aLogoSz = pLogo->aBitmap.GetSizePixel();

        Rectangle aRect(Point(), Point(aLogoSz.Width() - 1, aOutSz.Height()));
        if (rRenderContext.GetColorCount() >= 256)
        {
            Gradient aGrad(GradientStyle::Linear, pLogo->aStartColor, pLogo->aEndColor);
            aGrad.SetAngle(1800);
            aGrad.SetBorder(15);
            rRenderContext.DrawGradient(aRect, aGrad);
        }
        else
        {
            rRenderContext.SetFillColor(pLogo->aStartColor);
            rRenderContext.DrawRect(aRect);
        }

        Point aLogoPos(0, aOutSz.Height() - aLogoSz.Height());
        pLogo->aBitmap.Draw(&rRenderContext, aLogoPos);
    }
}

Menu* Menu::ImplGetStartMenu()
{
    Menu* pStart = this;
    while ( pStart && pStart->pStartedFrom && ( pStart->pStartedFrom != pStart ) )
        pStart = pStart->pStartedFrom;
    return pStart;
}

void Menu::ImplCallHighlight(sal_uInt16 nItem)
{
    ImplMenuDelData aDelData( this );

    nSelectedId = 0;
    MenuItemData* pData = pItemList->GetDataFromPos(nItem);
    if ( pData )
        nSelectedId = pData->nId;
    ImplCallEventListeners( VclEventId::MenuHighlight, GetItemPos( GetCurItemId() ) );

    if( !aDelData.isDeleted() )
        nSelectedId = 0;
}

IMPL_LINK_NOARG(Menu, ImplCallSelect, void*, void)
{
    nEventId = nullptr;
    Select();
}

Menu* Menu::ImplFindSelectMenu()
{
    Menu* pSelMenu = nEventId ? this : nullptr;

    for ( size_t n = GetItemList()->size(); n && !pSelMenu; )
    {
        MenuItemData* pData = GetItemList()->GetDataFromPos( --n );

        if ( pData->pSubMenu )
            pSelMenu = pData->pSubMenu->ImplFindSelectMenu();
    }

    return pSelMenu;
}

Menu* Menu::ImplFindMenu( sal_uInt16 nItemId )
{
    Menu* pSelMenu = nullptr;

    for ( size_t n = GetItemList()->size(); n && !pSelMenu; )
    {
        MenuItemData* pData = GetItemList()->GetDataFromPos( --n );

        if( pData->nId == nItemId )
            pSelMenu = this;
        else if ( pData->pSubMenu )
            pSelMenu = pData->pSubMenu->ImplFindMenu( nItemId );
    }

    return pSelMenu;
}

void Menu::RemoveDisabledEntries( bool bCheckPopups, bool bRemoveEmptyPopups )
{
    for ( sal_uInt16 n = 0; n < GetItemCount(); n++ )
    {
        bool bRemove = false;
        MenuItemData* pItem = pItemList->GetDataFromPos( n );
        if ( pItem->eType == MenuItemType::SEPARATOR )
        {
            if ( !n || ( GetItemType( n-1 ) == MenuItemType::SEPARATOR ) )
                bRemove = true;
        }
        else
            bRemove = !pItem->bEnabled;

        if ( bCheckPopups && pItem->pSubMenu )
        {
            pItem->pSubMenu->RemoveDisabledEntries();
            if ( bRemoveEmptyPopups && !pItem->pSubMenu->GetItemCount() )
                bRemove = true;
        }

        if ( bRemove )
            RemoveItem( n-- );
    }

    if ( GetItemCount() )
    {
        sal_uInt16 nLast = GetItemCount() - 1;
        MenuItemData* pItem = pItemList->GetDataFromPos( nLast );
        if ( pItem->eType == MenuItemType::SEPARATOR )
            RemoveItem( nLast );
    }
    delete mpLayoutData;
    mpLayoutData = nullptr;
}

bool Menu::HasValidEntries()
{
    bool bValidEntries = false;
    sal_uInt16 nCount = GetItemCount();
    for ( sal_uInt16 n = 0; !bValidEntries && ( n < nCount ); n++ )
    {
        MenuItemData* pItem = pItemList->GetDataFromPos( n );
        if ( pItem->bEnabled && ( pItem->eType != MenuItemType::SEPARATOR ) )
        {
            if ( pItem->pSubMenu )
                bValidEntries = pItem->pSubMenu->HasValidEntries();
            else
                bValidEntries = true;
        }
    }
    return bValidEntries;
}

void Menu::UpdateNativeMenu()
{
    if ( ImplGetSalMenu() )
        ImplGetSalMenu()->Update();
}

void Menu::MenuBarKeyInput(const KeyEvent&)
{
}

void Menu::ImplKillLayoutData() const
{
    delete mpLayoutData;
    mpLayoutData = nullptr;
}

void Menu::ImplFillLayoutData() const
{
    if (pWindow && pWindow->IsReallyVisible())
    {
        mpLayoutData = new MenuLayoutData();
        if (IsMenuBar())
        {
            ImplPaint(*pWindow, 0, 0, nullptr, false, true); // FIXME
        }
        else
        {
            MenuFloatingWindow* pFloat = static_cast<MenuFloatingWindow*>(pWindow.get());
            ImplPaint(*pWindow, pFloat->nScrollerHeight, pFloat->ImplGetStartY(), nullptr, false, true); //FIXME
        }
    }
}

Rectangle Menu::GetCharacterBounds( sal_uInt16 nItemID, long nIndex ) const
{
    long nItemIndex = -1;
    if( ! mpLayoutData )
        ImplFillLayoutData();
    if( mpLayoutData )
    {
        for( size_t i = 0; i < mpLayoutData->m_aLineItemIds.size(); i++ )
        {
            if( mpLayoutData->m_aLineItemIds[i] == nItemID )
            {
                nItemIndex = mpLayoutData->m_aLineIndices[i];
                break;
            }
        }
    }
    return (mpLayoutData && nItemIndex != -1) ? mpLayoutData->GetCharacterBounds( nItemIndex+nIndex ) : Rectangle();
}

long Menu::GetIndexForPoint( const Point& rPoint, sal_uInt16& rItemID ) const
{
    long nIndex = -1;
    rItemID = 0;
    if( ! mpLayoutData )
        ImplFillLayoutData();
    if( mpLayoutData )
    {
        nIndex = mpLayoutData->GetIndexForPoint( rPoint );
        for( size_t i = 0; i < mpLayoutData->m_aLineIndices.size(); i++ )
        {
            if( mpLayoutData->m_aLineIndices[i] <= nIndex &&
                (i == mpLayoutData->m_aLineIndices.size()-1 || mpLayoutData->m_aLineIndices[i+1] > nIndex) )
            {
                // make index relative to item
                nIndex -= mpLayoutData->m_aLineIndices[i];
                rItemID = mpLayoutData->m_aLineItemIds[i];
                break;
            }
        }
    }
    return nIndex;
}

Rectangle Menu::GetBoundingRectangle( sal_uInt16 nPos ) const
{
    Rectangle aRet;

    if (!mpLayoutData )
        ImplFillLayoutData();
    if (mpLayoutData)
    {
        std::map< sal_uInt16, Rectangle >::const_iterator it = mpLayoutData->m_aVisibleItemBoundRects.find( nPos );
        if( it != mpLayoutData->m_aVisibleItemBoundRects.end() )
            aRet = it->second;
    }
    return aRet;
}

OUString Menu::GetAccessibleName( sal_uInt16 nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        return pData->aAccessibleName;

    return OUString();
}

void Menu::ImplSetSalMenu( SalMenu *pSalMenu )
{
    if( mpSalMenu )
        ImplGetSVData()->mpDefInst->DestroyMenu( mpSalMenu );
    mpSalMenu = pSalMenu;
}

bool Menu::GetSystemMenuData( SystemMenuData* pData ) const
{
    Menu* pMenu = const_cast<Menu*>(this);
    if( pData && pMenu->ImplGetSalMenu() )
    {
        pMenu->ImplGetSalMenu()->GetSystemMenuData( pData );
        return true;
    }
    else
        return false;
}

bool Menu::IsHighlighted( sal_uInt16 nItemPos ) const
{
    bool bRet = false;

    if( pWindow )
    {
        if (IsMenuBar())
            bRet = ( nItemPos == static_cast< MenuBarWindow * > (pWindow.get())->GetHighlightedItem() );
        else
            bRet = ( nItemPos == static_cast< MenuFloatingWindow * > (pWindow.get())->GetHighlightedItem() );
    }

    return bRet;
}

void Menu::HighlightItem( sal_uInt16 nItemPos )
{
    if ( pWindow )
    {
        if (IsMenuBar())
        {
            MenuBarWindow* pMenuWin = static_cast< MenuBarWindow* >( pWindow.get() );
            pMenuWin->SetAutoPopup( false );
            pMenuWin->ChangeHighlightItem( nItemPos, false );
        }
        else
        {
            static_cast< MenuFloatingWindow* >( pWindow.get() )->ChangeHighlightItem( nItemPos, false );
        }
    }
}

MenuBarWindow* MenuBar::getMenuBarWindow()
{
    // so far just a dynamic_cast, hopefully to be turned into something saner
    // at some stage
    MenuBarWindow *pWin = dynamic_cast<MenuBarWindow*>(pWindow.get());
    //either there is no window (fdo#87663) or it is a MenuBarWindow
    assert(!pWindow || pWin);
    return pWin;
}

MenuBar::MenuBar()
    : Menu(),
      mbCloseBtnVisible(false),
      mbFloatBtnVisible(false),
      mbHideBtnVisible(false),
      mbDisplayable(true)
{
    mpSalMenu = ImplGetSVData()->mpDefInst->CreateMenu(true, this);
}

MenuBar::MenuBar( const MenuBar& rMenu )
    : Menu(),
      mbCloseBtnVisible(false),
      mbFloatBtnVisible(false),
      mbHideBtnVisible(false),
      mbDisplayable(true)
{
    mpSalMenu = ImplGetSVData()->mpDefInst->CreateMenu(true, this);
    *this = rMenu;
}

MenuBar::~MenuBar()
{
    disposeOnce();
}

void MenuBar::dispose()
{
    ImplDestroy( this, true );
    Menu::dispose();
}

void MenuBar::ClosePopup(Menu *pMenu)
{
    MenuBarWindow* pMenuWin = getMenuBarWindow();
    if (!pMenuWin)
        return;
    pMenuWin->PopupClosed(pMenu);
}

void MenuBar::MenuBarKeyInput(const KeyEvent& rEvent)
{
    pWindow->KeyInput(rEvent);
}

void MenuBar::ShowCloseButton(bool bShow)
{
    ShowButtons( bShow, mbFloatBtnVisible, mbHideBtnVisible );
}

void MenuBar::ShowButtons( bool bClose, bool bFloat, bool bHide )
{
    if ((bClose != mbCloseBtnVisible) ||
        (bFloat != mbFloatBtnVisible) ||
        (bHide  != mbHideBtnVisible))
    {
        mbCloseBtnVisible = bClose;
        mbFloatBtnVisible = bFloat;
        mbHideBtnVisible = bHide;
        MenuBarWindow* pMenuWin = getMenuBarWindow();
        if (pMenuWin)
            pMenuWin->ShowButtons(bClose, bFloat, bHide);
    }
}

void MenuBar::SetDisplayable( bool bDisplayable )
{
    if( bDisplayable != mbDisplayable )
    {
        if ( ImplGetSalMenu() )
            ImplGetSalMenu()->ShowMenuBar( bDisplayable );

        mbDisplayable = bDisplayable;
        MenuBarWindow* pMenuWin = getMenuBarWindow();
        if (pMenuWin)
            pMenuWin->LayoutChanged();
    }
}

VclPtr<vcl::Window> MenuBar::ImplCreate(vcl::Window* pParent, vcl::Window* pWindow, MenuBar* pMenu)
{
    VclPtr<MenuBarWindow> pMenuBarWindow = dynamic_cast<MenuBarWindow*>(pWindow);
    if (!pMenuBarWindow)
    {
        pWindow = pMenuBarWindow = VclPtr<MenuBarWindow>::Create( pParent );
    }

    pMenu->pStartedFrom = nullptr;
    pMenu->pWindow = pWindow;
    pMenuBarWindow->SetMenu(pMenu);
    long nHeight = pWindow ? pMenu->ImplCalcSize(pWindow).Height() : 0;

    // depending on the native implementation or the displayable flag
    // the menubar windows is suppressed (ie, height=0)
    if (!pMenu->IsDisplayable() || (pMenu->ImplGetSalMenu() && pMenu->ImplGetSalMenu()->VisibleMenuBar()))
    {
        nHeight = 0;
    }

    pMenuBarWindow->SetHeight(nHeight);
    return pWindow;
}

void MenuBar::ImplDestroy( MenuBar* pMenu, bool bDelete )
{
    vcl::Window *pWindow = pMenu->ImplGetWindow();
    if (pWindow && bDelete)
    {
        MenuBarWindow* pMenuWin = pMenu->getMenuBarWindow();
        if (pMenuWin)
            pMenuWin->KillActivePopup();
        pWindow->disposeOnce();
    }
    pMenu->pWindow = nullptr;
}

bool MenuBar::ImplHandleKeyEvent( const KeyEvent& rKEvent )
{
    // No keyboard processing when our menubar is invisible
    if (!IsDisplayable())
        return false;

    // No keyboard processing when system handles the menu.
    SalMenu *pNativeMenu = ImplGetSalMenu();
    if (pNativeMenu && pNativeMenu->VisibleMenuBar())
    {
        // Except when the event is the F6 cycle pane event and we can put our
        // focus into it (i.e. the gtk3 menubar case but not the mac/unity case
        // where it's not part of the application window)
        if (!TaskPaneList::IsCycleKey(rKEvent.GetKeyCode()))
            return false;
        if (!pNativeMenu->CanGetFocus())
            return false;
    }

    bool bDone = false;
    // check for enabled, if this method is called from another window...
    vcl::Window* pWin = ImplGetWindow();
    if (pWin && pWin->IsEnabled() && pWin->IsInputEnabled()  && !pWin->IsInModalMode())
    {
        MenuBarWindow* pMenuWin = getMenuBarWindow();
        bDone = pMenuWin && pMenuWin->HandleKeyEvent(rKEvent, false/*bFromMenu*/);
    }
    return bDone;
}

bool MenuBar::ImplHandleCmdEvent( const CommandEvent& rCEvent )
{
    // No keyboard processing when system handles the menu or our menubar is invisible
    if( !IsDisplayable() ||
        ( ImplGetSalMenu() && ImplGetSalMenu()->VisibleMenuBar() ) )
        return false;

    // check for enabled, if this method is called from another window...
    MenuBarWindow* pWin = static_cast<MenuBarWindow*>(ImplGetWindow());
    if ( pWin && pWin->IsEnabled() && pWin->IsInputEnabled()  && ! pWin->IsInModalMode() )
    {
        if (rCEvent.GetCommand() == CommandEventId::ModKeyChange && ImplGetSVData()->maNWFData.mbAutoAccel)
        {
            const CommandModKeyData* pCData = rCEvent.GetModKeyData ();
            if (pWin->nHighlightedItem == ITEMPOS_INVALID)
            {
                if (pCData && pCData->IsMod2())
                    pWin->SetMBWHideAccel(false);
                else
                    pWin->SetMBWHideAccel(true);
                pWin->Invalidate(InvalidateFlags::Update);
            }
            return true;
        }
    }
    return false;
}

void MenuBar::SelectItem(sal_uInt16 nId)
{
    if (pWindow)
    {
        pWindow->GrabFocus();
        nId = GetItemPos( nId );

        MenuBarWindow* pMenuWin = getMenuBarWindow();
        if (pMenuWin)
        {
            // #99705# popup the selected menu
            pMenuWin->SetAutoPopup( true );
            if (ITEMPOS_INVALID != pMenuWin->GetHighlightedItem())
            {
                pMenuWin->KillActivePopup();
                pMenuWin->ChangeHighlightItem( ITEMPOS_INVALID, false );
            }
            if (nId != ITEMPOS_INVALID)
                pMenuWin->ChangeHighlightItem( nId, false );
        }
    }
}

// handler for native menu selection and command events
bool Menu::HandleMenuActivateEvent( Menu *pMenu ) const
{
    if( pMenu )
    {
        ImplMenuDelData aDelData( this );

        pMenu->pStartedFrom = const_cast<Menu*>(this);
        pMenu->bInCallback = true;
        pMenu->Activate();

        if( !aDelData.isDeleted() )
            pMenu->bInCallback = false;
    }
    return true;
}

bool Menu::HandleMenuDeActivateEvent( Menu *pMenu ) const
{
    if( pMenu )
    {
        ImplMenuDelData aDelData( this );

        pMenu->pStartedFrom = const_cast<Menu*>(this);
        pMenu->bInCallback = true;
        pMenu->Deactivate();
        if( !aDelData.isDeleted() )
            pMenu->bInCallback = false;
    }
    return true;
}

bool MenuBar::HandleMenuHighlightEvent( Menu *pMenu, sal_uInt16 nHighlightEventId ) const
{
    if( !pMenu )
        pMenu = const_cast<MenuBar*>(this)->ImplFindMenu(nHighlightEventId);
    if( pMenu )
    {
        ImplMenuDelData aDelData( pMenu );

        if( mnHighlightedItemPos != ITEMPOS_INVALID )
            pMenu->ImplCallEventListeners( VclEventId::MenuDehighlight, mnHighlightedItemPos );

        if( !aDelData.isDeleted() )
        {
            pMenu->mnHighlightedItemPos = pMenu->GetItemPos( nHighlightEventId );
            pMenu->nSelectedId = nHighlightEventId;
            pMenu->pStartedFrom = const_cast<MenuBar*>(this);
            pMenu->ImplCallHighlight( pMenu->mnHighlightedItemPos );
        }
        return true;
    }
    else
        return false;
}

bool Menu::HandleMenuCommandEvent( Menu *pMenu, sal_uInt16 nCommandEventId ) const
{
    if( !pMenu )
        pMenu = const_cast<Menu*>(this)->ImplFindMenu(nCommandEventId);
    if( pMenu )
    {
        pMenu->nSelectedId = nCommandEventId;
        pMenu->pStartedFrom = const_cast<Menu*>(this);
        pMenu->ImplSelect();
        return true;
    }
    else
        return false;
}

sal_uInt16 MenuBar::AddMenuBarButton( const Image& i_rImage, const Link<MenuBar::MenuBarButtonCallbackArg&,bool>& i_rLink, const OUString& i_rToolTip )
{
    MenuBarWindow* pMenuWin = getMenuBarWindow();
    return pMenuWin ? pMenuWin->AddMenuBarButton(i_rImage, i_rLink, i_rToolTip) : 0;
}

void MenuBar::SetMenuBarButtonHighlightHdl( sal_uInt16 nId, const Link<MenuBar::MenuBarButtonCallbackArg&,bool>& rLink )
{
    MenuBarWindow* pMenuWin = getMenuBarWindow();
    if (!pMenuWin)
        return;
    pMenuWin->SetMenuBarButtonHighlightHdl(nId, rLink);
}

void MenuBar::RemoveMenuBarButton( sal_uInt16 nId )
{
    MenuBarWindow* pMenuWin = getMenuBarWindow();
    if (!pMenuWin)
        return;
    pMenuWin->RemoveMenuBarButton(nId);
}

Rectangle MenuBar::GetMenuBarButtonRectPixel( sal_uInt16 nId )
{
    MenuBarWindow* pMenuWin = getMenuBarWindow();
    return pMenuWin ? pMenuWin->GetMenuBarButtonRectPixel(nId) : Rectangle();
}

bool MenuBar::HandleMenuButtonEvent( Menu *, sal_uInt16 i_nButtonId )
{
    MenuBarWindow* pMenuWin = getMenuBarWindow();
    return pMenuWin && pMenuWin->HandleMenuButtonEvent(i_nButtonId);
}

// bool PopupMenu::bAnyPopupInExecute = false;

MenuFloatingWindow * PopupMenu::ImplGetFloatingWindow() const {
    return static_cast<MenuFloatingWindow *>(Menu::ImplGetWindow());
}

PopupMenu::PopupMenu()
    : pRefAutoSubMenu(nullptr)
{
    mpSalMenu = ImplGetSVData()->mpDefInst->CreateMenu(false, this);
}

PopupMenu::PopupMenu( const ResId& rResId )
    : pRefAutoSubMenu(nullptr)
{
    mpSalMenu = ImplGetSVData()->mpDefInst->CreateMenu(false, this);

    ResMgr* pMgr = rResId.GetResMgr();
    if( ! pMgr )
        return;

    rResId.SetRT( RSC_MENU );
    GetRes( rResId );

    RscMenu nObjMask = (RscMenu)ReadLongRes();

    if( nObjMask & RscMenu::Items )
    {
        sal_uLong nObjFollows = ReadLongRes();
        // insert menu items
        for( sal_uLong i = 0; i < nObjFollows; i++ )
        {
            InsertItem( ResId( static_cast<RSHEADER_TYPE*>(GetClassRes()), *pMgr ) );
            IncrementRes( GetObjSizeRes( static_cast<RSHEADER_TYPE*>(GetClassRes()) ) );
        }
    }

    if( nObjMask & RscMenu::Text )
    {
        aTitleText = ReadStringRes();
    }
}

PopupMenu::PopupMenu( const PopupMenu& rMenu )
    : Menu(),
      pRefAutoSubMenu(nullptr)
{
    mpSalMenu = ImplGetSVData()->mpDefInst->CreateMenu(false, this);
    *this = rMenu;
}

PopupMenu::~PopupMenu()
{
    disposeOnce();
}

void PopupMenu::dispose()
{
    if( pRefAutoSubMenu && *pRefAutoSubMenu == this )
        *pRefAutoSubMenu = nullptr;    // #111060# avoid second delete in ~MenuItemData
    Menu::dispose();
}

void PopupMenu::ClosePopup(Menu* pMenu)
{
    MenuFloatingWindow* p = dynamic_cast<MenuFloatingWindow*>(ImplGetWindow());
    PopupMenu *pPopup = dynamic_cast<PopupMenu*>(pMenu);
    if (p && pMenu)
        p->KillActivePopup(pPopup);
}

bool PopupMenu::IsInExecute()
{
    return GetActivePopupMenu() != nullptr;
}

PopupMenu* PopupMenu::GetActivePopupMenu()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->maAppData.mpActivePopupMenu;
}

void PopupMenu::EndExecute()
{
    if ( ImplGetWindow() )
        ImplGetFloatingWindow()->EndExecute( 0 );
}

void PopupMenu::SelectItem(sal_uInt16 nId)
{
    if ( ImplGetWindow() )
    {
        if( nId != ITEMPOS_INVALID )
        {
            size_t nPos = 0;
            MenuItemData* pData = GetItemList()->GetData( nId, nPos );
            if (pData && pData->pSubMenu)
                ImplGetFloatingWindow()->ChangeHighlightItem( nPos, true );
            else
                ImplGetFloatingWindow()->EndExecute( nId );
        }
        else
        {
            MenuFloatingWindow* pFloat = ImplGetFloatingWindow();
            pFloat->GrabFocus();

            for( size_t nPos = 0; nPos < GetItemList()->size(); nPos++ )
            {
                MenuItemData* pData = GetItemList()->GetDataFromPos( nPos );
                if( pData->pSubMenu )
                {
                    pFloat->KillActivePopup();
                }
            }
            pFloat->ChangeHighlightItem( ITEMPOS_INVALID, false );
        }
    }
}

void PopupMenu::SetSelectedEntry( sal_uInt16 nId )
{
    nSelectedId = nId;
}

sal_uInt16 PopupMenu::Execute( vcl::Window* pExecWindow, const Point& rPopupPos )
{
    return Execute( pExecWindow, Rectangle( rPopupPos, rPopupPos ), PopupMenuFlags::ExecuteDown );
}

sal_uInt16 PopupMenu::Execute( vcl::Window* pExecWindow, const Rectangle& rRect, PopupMenuFlags nFlags )
{
    ENSURE_OR_RETURN( pExecWindow, "PopupMenu::Execute: need a non-NULL window!", 0 );

    FloatWinPopupFlags nPopupModeFlags = FloatWinPopupFlags::NONE;
    if ( nFlags & PopupMenuFlags::ExecuteDown )
        nPopupModeFlags = FloatWinPopupFlags::Down;
    else if ( nFlags & PopupMenuFlags::ExecuteUp )
        nPopupModeFlags = FloatWinPopupFlags::Up;
    else if ( nFlags & PopupMenuFlags::ExecuteLeft )
        nPopupModeFlags = FloatWinPopupFlags::Left;
    else if ( nFlags & PopupMenuFlags::ExecuteRight )
        nPopupModeFlags = FloatWinPopupFlags::Right;
    else
        nPopupModeFlags = FloatWinPopupFlags::Down;

    if (nFlags & PopupMenuFlags::NoMouseUpClose )                      // allow popup menus to stay open on mouse button up
        nPopupModeFlags |= FloatWinPopupFlags::NoMouseUpClose;    // useful if the menu was opened on mousebutton down (eg toolbox configuration)

    if (nFlags & PopupMenuFlags::NoHorzPlacement)
        nPopupModeFlags |= FloatWinPopupFlags::NoHorzPlacement;

    return ImplExecute( pExecWindow, rRect, nPopupModeFlags, nullptr, false );
}

void PopupMenu::ImplFlushPendingSelect()
{
    // is there still Select?
    Menu* pSelect = ImplFindSelectMenu();
    if (pSelect)
    {
        // Select should be called prior to leaving execute in a popup menu!
        Application::RemoveUserEvent( pSelect->nEventId );
        pSelect->nEventId = nullptr;
        pSelect->Select();
    }
}

sal_uInt16 PopupMenu::ImplExecute( const VclPtr<vcl::Window>& pW, const Rectangle& rRect, FloatWinPopupFlags nPopupModeFlags, Menu* pSFrom, bool bPreSelectFirst )
{
    if ( !pSFrom && ( PopupMenu::IsInExecute() || !GetItemCount() ) )
        return 0;

    // set the flag to hide or show accelerators in the menu depending on whether the menu was launched by mouse or keyboard shortcut
    if( pSFrom && pSFrom->IsMenuBar())
        ((static_cast<MenuBarWindow*>(pSFrom->pWindow.get())))->SetMBWHideAccel(!(static_cast<MenuBarWindow*>(pSFrom->pWindow.get())->GetMBWMenuKey()));

    delete mpLayoutData;
    mpLayoutData = nullptr;

    ImplSVData* pSVData = ImplGetSVData();

    pStartedFrom = pSFrom;
    nSelectedId = 0;
    bCanceled = false;

    VclPtr<vcl::Window> xFocusId;
    bool bRealExecute = false;
    if ( !pStartedFrom )
    {
        pSVData->maWinData.mbNoDeactivate = true;
        xFocusId = Window::SaveFocus();
        bRealExecute = true;
    }
    else
    {
        // assure that only one menu is open at a time
        if (pStartedFrom->IsMenuBar() && pSVData->maWinData.mpFirstFloat)
            pSVData->maWinData.mpFirstFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll );
    }

    SAL_WARN_IF( ImplGetWindow(), "vcl", "Win?!" );
    Rectangle aRect( rRect );
    aRect.SetPos( pW->OutputToScreenPixel( aRect.TopLeft() ) );

    WinBits nStyle = WB_BORDER;
    if (bRealExecute)
        nPopupModeFlags |= FloatWinPopupFlags::NewLevel;
    nPopupModeFlags |= FloatWinPopupFlags::NoKeyClose | FloatWinPopupFlags::AllMouseButtonClose;

    bInCallback = true; // set it here, if Activate overridden
    Activate();
    bInCallback = false;

    if ( pW->IsDisposed() )
        return 0;   // Error

    if ( bCanceled || bKilled )
        return 0;

    if ( !GetItemCount() )
        return 0;

    // The flag MenuFlags::HideDisabledEntries is inherited.
    if ( pSFrom )
    {
        if ( pSFrom->nMenuFlags & MenuFlags::HideDisabledEntries )
            nMenuFlags |= MenuFlags::HideDisabledEntries;
        else
            nMenuFlags &= ~MenuFlags::HideDisabledEntries;
    }
    else
        // #102790# context menus shall never show disabled entries
        nMenuFlags |= MenuFlags::HideDisabledEntries;

    sal_uInt16 nVisibleEntries = ImplGetVisibleItemCount();
    if ( !nVisibleEntries )
    {
        ResMgr* pResMgr = ImplGetResMgr();
        if( pResMgr )
        {
            OUString aTmpEntryText( ResId( SV_RESID_STRING_NOSELECTIONPOSSIBLE, *pResMgr ) );

            MenuItemData* pData = NbcInsertItem(0xFFFF, MenuItemBits::NONE, aTmpEntryText, nullptr, 0xFFFF, OString());
            size_t nPos = 0;
            pData = pItemList->GetData( pData->nId, nPos );
            assert(pData);
            if (pData)
            {
                pData->bIsTemporary = true;
            }
            ImplCallEventListeners(VclEventId::MenuSubmenuChanged, nPos);
        }
    }
    else if (!(nMenuFlags & MenuFlags::NoAutoMnemonics))
    {
        CreateAutoMnemonics();
    }

    VclPtrInstance<MenuFloatingWindow> pWin( this, pW, nStyle | WB_SYSTEMWINDOW );
    if( pSVData->maNWFData.mbFlatMenu )
        pWin->SetBorderStyle( WindowBorderStyle::NOBORDER );
    else
        pWin->SetBorderStyle( pWin->GetBorderStyle() | WindowBorderStyle::MENU );
    pWindow = pWin;

    Size aSz = ImplCalcSize( pWin );

    Rectangle aDesktopRect(pWin->GetDesktopRectPixel());
    if( Application::GetScreenCount() > 1 && Application::IsUnifiedDisplay() )
    {
        vcl::Window* pDeskW = pWindow->GetWindow( GetWindowType::RealParent );
        if( ! pDeskW )
            pDeskW = pWindow;
        Point aDesktopTL( pDeskW->OutputToAbsoluteScreenPixel( aRect.TopLeft() ) );
        aDesktopRect = Application::GetScreenPosSizePixel(
            Application::GetBestScreen( Rectangle( aDesktopTL, aRect.GetSize() ) ));
    }

    long nMaxHeight = aDesktopRect.GetHeight();

    //rhbz#1021915. If a menu won't fit in the desired location the default
    //mode is to place it somewhere it will fit.  e.g. above, left, right. For
    //some cases, e.g. menubars, it's desirable to limit the options to
    //above/below and force the menu to scroll if it won't fit
    if (nPopupModeFlags & FloatWinPopupFlags::NoHorzPlacement)
    {
        vcl::Window* pRef = pWin;
        if ( pRef->GetParent() )
            pRef = pRef->GetParent();

        Rectangle devRect(  pRef->OutputToAbsoluteScreenPixel( aRect.TopLeft() ),
                            pRef->OutputToAbsoluteScreenPixel( aRect.BottomRight() ) );

        long nHeightAbove = devRect.Top() - aDesktopRect.Top();
        long nHeightBelow = aDesktopRect.Bottom() - devRect.Bottom();
        nMaxHeight = std::min(nMaxHeight, std::max(nHeightAbove, nHeightBelow));
    }

    if (pStartedFrom && pStartedFrom->IsMenuBar())
        nMaxHeight -= pW->GetSizePixel().Height();
    sal_Int32 nLeft, nTop, nRight, nBottom;
    pWindow->GetBorder( nLeft, nTop, nRight, nBottom );
    nMaxHeight -= nTop+nBottom;
    if ( aSz.Height() > nMaxHeight )
    {
        pWin->EnableScrollMenu( true );
        sal_uInt16 nStart = ImplGetFirstVisible();
        sal_uInt16 nEntries = ImplCalcVisEntries( nMaxHeight, nStart );
        aSz.Height() = ImplCalcHeight( nEntries );
    }

    pWin->SetFocusId( xFocusId );
    pWin->SetOutputSizePixel( aSz );
    if ( GetItemCount() )
    {
        SalMenu* pMenu = ImplGetSalMenu();
        if( pMenu && bRealExecute && pMenu->ShowNativePopupMenu( pWin, aRect, nPopupModeFlags | FloatWinPopupFlags::GrabFocus ) )
        {
            ImplFlushPendingSelect();
            pWin->StopExecute();
            pWin->doShutdown();
            pWindow->doLazyDelete();
            pWindow = nullptr;
            return nSelectedId;
        }
        else
        {
            pWin->StartPopupMode( aRect, nPopupModeFlags | FloatWinPopupFlags::GrabFocus );
        }
        if( pSFrom )
        {
            sal_uInt16 aPos;
            if (pSFrom->IsMenuBar())
                aPos = static_cast<MenuBarWindow *>(pSFrom->pWindow.get())->GetHighlightedItem();
            else
                aPos = static_cast<MenuFloatingWindow *>(pSFrom->pWindow.get())->GetHighlightedItem();

            pWin->SetPosInParent( aPos );  // store position to be sent in SUBMENUDEACTIVATE
            pSFrom->ImplCallEventListeners( VclEventId::MenuSubmenuActivate, aPos );
        }
    }
    if ( bPreSelectFirst )
    {
        size_t nCount = pItemList->size();
        for ( size_t n = 0; n < nCount; n++ )
        {
            MenuItemData* pData = pItemList->GetDataFromPos( n );
            if (  (  pData->bEnabled
                  || !Application::GetSettings().GetStyleSettings().GetSkipDisabledInMenus()
                  )
               && ( pData->eType != MenuItemType::SEPARATOR )
               && ImplIsVisible( n )
               && ImplIsSelectable( n )
               )
            {
                pWin->ChangeHighlightItem( n, false );
                break;
            }
        }
    }
    if ( bRealExecute )
    {
        pWin->Execute();
        if (pWin->IsDisposed())
            return 0;

        xFocusId = pWin->GetFocusId();
        assert(xFocusId == nullptr && "Focus should already be restored by MenuFloatingWindow::End");
        pWin->ImplEndPopupMode(FloatWinPopupEndFlags::NONE, xFocusId);

        if ( nSelectedId )  // then clean up .. ( otherwise done by TH )
        {
            PopupMenu* pSub = pWin->GetActivePopup();
            while ( pSub )
            {
                pSub->ImplGetFloatingWindow()->EndPopupMode();
                pSub = pSub->ImplGetFloatingWindow()->GetActivePopup();
            }
        }
        pWin->doShutdown();
        pWindow->doLazyDelete();
        pWindow = nullptr;

        ImplFlushPendingSelect();
    }

    return bRealExecute ? nSelectedId : 0;
}

sal_uInt16 PopupMenu::ImplCalcVisEntries( long nMaxHeight, sal_uInt16 nStartEntry, sal_uInt16* pLastVisible ) const
{
    nMaxHeight -= 2 * ImplGetFloatingWindow()->GetScrollerHeight();

    long nHeight = 0;
    size_t nEntries = pItemList->size();
    sal_uInt16 nVisEntries = 0;

    if ( pLastVisible )
        *pLastVisible = 0;

    for ( size_t n = nStartEntry; n < nEntries; n++ )
    {
        if ( ImplIsVisible( n ) )
        {
            MenuItemData* pData = pItemList->GetDataFromPos( n );
            nHeight += pData->aSz.Height();
            if ( nHeight > nMaxHeight )
                break;

            if ( pLastVisible )
                *pLastVisible = n;
            nVisEntries++;
        }
    }
    return nVisEntries;
}

long PopupMenu::ImplCalcHeight( sal_uInt16 nEntries ) const
{
    long nHeight = 0;

    sal_uInt16 nFound = 0;
    for ( size_t n = 0; ( nFound < nEntries ) && ( n < pItemList->size() ); n++ )
    {
        if ( ImplIsVisible( (sal_uInt16) n ) )
        {
            MenuItemData* pData = pItemList->GetDataFromPos( n );
            nHeight += pData->aSz.Height();
            nFound++;
        }
    }

    nHeight += 2*ImplGetFloatingWindow()->GetScrollerHeight();

    return nHeight;
}

ImplMenuDelData::ImplMenuDelData( const Menu* pMenu )
: mpNext( nullptr )
, mpMenu( nullptr )
{
    if( pMenu )
        const_cast< Menu* >( pMenu )->ImplAddDel( *this );
}

ImplMenuDelData::~ImplMenuDelData()
{
    if( mpMenu )
        const_cast< Menu* >( mpMenu.get() )->ImplRemoveDel( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
