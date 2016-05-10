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

#include <sal/config.h>

#include <objc/objc-runtime.h>

#include <comphelper/string.hxx>
#include <rtl/ustrbuf.hxx>

#include <vcl/commandevent.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>

#include "osx/saldata.hxx"
#include "osx/salinst.h"
#include "osx/salmenu.h"
#include "osx/salnsmenu.h"
#include "osx/salframe.h"
#include "osx/a11ywrapper.h"
#include "quartz/utils.h"
#include "svids.hrc"
#include "window.h"

namespace {

void releaseButtonEntry( AquaSalMenu::MenuBarButtonEntry& i_rEntry )
{
    if( i_rEntry.mpNSImage )
    {
        [i_rEntry.mpNSImage release];
        i_rEntry.mpNSImage = nil;
    }
    if( i_rEntry.mpToolTipString )
    {
        [i_rEntry.mpToolTipString release];
        i_rEntry.mpToolTipString = nil;
    }
}

}

const AquaSalMenu* AquaSalMenu::pCurrentMenuBar = nullptr;

@interface MainMenuSelector : NSObject
{
}
-(void)showDialog: (ShowDialogId)nDialog;
-(void)showPreferences: (id)sender;
-(void)showAbout: (id)sender;
@end

@implementation MainMenuSelector
-(void)showDialog: (ShowDialogId)nDialog
{
    if( AquaSalMenu::pCurrentMenuBar )
    {
        const AquaSalFrame* pFrame = AquaSalMenu::pCurrentMenuBar->mpFrame;
        if( pFrame && AquaSalFrame::isAlive( pFrame ) )
        {
            pFrame->CallCallback( SalEvent::ShowDialog, reinterpret_cast<void*>(nDialog) );
        }
    }
    else
    {
        OUString aDialog;
        if( nDialog == ShowDialogId::About )
            aDialog = "ABOUT";
        else if( nDialog == ShowDialogId::Preferences )
            aDialog = "PREFERENCES";
        const ApplicationEvent* pAppEvent = new ApplicationEvent(
            ApplicationEvent::TYPE_SHOWDIALOG, aDialog);
        AquaSalInstance::aAppEventList.push_back( pAppEvent );
    }
}

-(void)showPreferences: (id) sender
{
    (void)sender;
    SolarMutexGuard aGuard;

    [self showDialog: ShowDialogId::Preferences];
}
-(void)showAbout: (id) sender
{
    (void)sender;
    SolarMutexGuard aGuard;

    [self showDialog: ShowDialogId::About];
}
@end

// FIXME: currently this is leaked
static MainMenuSelector* pMainMenuSelector = nil;

static void initAppMenu()
{
    static bool bOnce = true;
    if( bOnce )
    {
        bOnce = false;

        ResMgr* pMgr = ImplGetResMgr();
        if( pMgr )
        {
            // get the main menu
            NSMenu* pMainMenu = [NSApp mainMenu];
            if( pMainMenu != nil )
            {
                // create the action selector
                pMainMenuSelector = [[MainMenuSelector alloc] init];

                // get the proper submenu
                NSMenu* pAppMenu = [[pMainMenu itemAtIndex: 0] submenu];
                if( pAppMenu )
                {
                    // insert about entry
                    OUString aAbout( ResId( SV_STDTEXT_ABOUT, *pMgr ) );
                    NSString* pString = CreateNSString( aAbout );
                    NSMenuItem* pNewItem = [pAppMenu insertItemWithTitle: pString
                                                     action: @selector(showAbout:)
                                                     keyEquivalent: @""
                                                     atIndex: 0];
                    if (pString)
                        [pString release];
                    if( pNewItem )
                    {
                        [pNewItem setTarget: pMainMenuSelector];
                        [pAppMenu insertItem: [NSMenuItem separatorItem] atIndex: 1];
                    }

                    // insert preferences entry
                    OUString aPref( ResId( SV_STDTEXT_PREFERENCES, *pMgr ) );
                    pString = CreateNSString( aPref );
                    pNewItem = [pAppMenu insertItemWithTitle: pString
                                         action: @selector(showPreferences:)
                                         keyEquivalent: @","
                                         atIndex: 2];
                    if (pString)
                        [pString release];
                    if( pNewItem )
                    {
                        [pNewItem setKeyEquivalentModifierMask: NSCommandKeyMask];
                        [pNewItem setTarget: pMainMenuSelector];
                        [pAppMenu insertItem: [NSMenuItem separatorItem] atIndex: 3];
                    }

                    // WARNING: ultra ugly code ahead

                    // rename standard entries
                    // rename "Services"
                    pNewItem = [pAppMenu itemAtIndex: 4];
                    if( pNewItem )
                    {
                        pString = CreateNSString( OUString( ResId( SV_MENU_MAC_SERVICES, *pMgr ) ) );
                        [pNewItem  setTitle: pString];
                        if( pString )
                            [pString release];
                    }

                    // rename "Hide NewApplication"
                    pNewItem = [pAppMenu itemAtIndex: 6];
                    if( pNewItem )
                    {
                        pString = CreateNSString( OUString( ResId( SV_MENU_MAC_HIDEAPP, *pMgr ) ) );
                        [pNewItem  setTitle: pString];
                        if( pString )
                            [pString release];
                    }

                    // rename "Hide Others"
                    pNewItem = [pAppMenu itemAtIndex: 7];
                    if( pNewItem )
                    {
                        pString = CreateNSString( OUString( ResId( SV_MENU_MAC_HIDEALL, *pMgr ) ) );
                        [pNewItem  setTitle: pString];
                        if( pString )
                            [pString release];
                    }

                    // rename "Show all"
                    pNewItem = [pAppMenu itemAtIndex: 8];
                    if( pNewItem )
                    {
                        pString = CreateNSString( OUString( ResId( SV_MENU_MAC_SHOWALL, *pMgr ) ) );
                        [pNewItem  setTitle: pString];
                        if( pString )
                            [pString release];
                    }

                    // rename "Quit NewApplication"
                    pNewItem = [pAppMenu itemAtIndex: 10];
                    if( pNewItem )
                    {
                        pString = CreateNSString( OUString( ResId( SV_MENU_MAC_QUITAPP, *pMgr ) ) );
                        [pNewItem  setTitle: pString];
                        if( pString )
                            [pString release];
                    }
                }
            }
        }
    }
}

SalMenu* AquaSalInstance::CreateMenu( bool bMenuBar, Menu* pVCLMenu )
{
    initAppMenu();

    AquaSalMenu *pAquaSalMenu = new AquaSalMenu( bMenuBar );
    pAquaSalMenu->mpVCLMenu = pVCLMenu;

    return pAquaSalMenu;
}

void AquaSalInstance::DestroyMenu( SalMenu* pSalMenu )
{
    delete pSalMenu;
}

SalMenuItem* AquaSalInstance::CreateMenuItem( const SalItemParams* pItemData )
{
    if( !pItemData )
        return nullptr;

    AquaSalMenuItem *pSalMenuItem = new AquaSalMenuItem( pItemData );

    return pSalMenuItem;
}

void AquaSalInstance::DestroyMenuItem( SalMenuItem* pSalMenuItem )
{
    delete pSalMenuItem;
}

/*
 * AquaSalMenu
 */

AquaSalMenu::AquaSalMenu( bool bMenuBar ) :
    mbMenuBar( bMenuBar ),
    mpMenu( nil ),
    mpVCLMenu( nullptr ),
    mpFrame( nullptr ),
    mpParentSalMenu( nullptr )
{
    if( ! mbMenuBar )
    {
        mpMenu = [[SalNSMenu alloc] initWithMenu: this];
        [mpMenu setDelegate: reinterpret_cast< id<NSMenuDelegate> >(mpMenu)];
    }
    else
    {
        mpMenu = [NSApp mainMenu];
    }
    [mpMenu setAutoenablesItems: NO];
}

AquaSalMenu::~AquaSalMenu()
{
    // actually someone should have done AquaSalFrame::SetMenu( NULL )
    // on our frame, alas it is not so
    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) && mpFrame->mpMenu == this )
        const_cast<AquaSalFrame*>(mpFrame)->mpMenu = nullptr;

    // this should normally be empty already, but be careful...
    for( size_t i = 0; i < maButtons.size(); i++ )
        releaseButtonEntry( maButtons[i] );
    maButtons.clear();

    // is this leaking in some cases ? the release often leads to a duplicate release
    // it seems the parent item gets ownership of the menu
    if( mpMenu )
    {
        if( mbMenuBar )
        {
            if( pCurrentMenuBar == this )
            {
                // if the current menubar gets destroyed, set the default menubar
                setDefaultMenu();
            }
        }
        else
            // the system may still hold a reference on mpMenu
        {
            // so set the pointer to this AquaSalMenu to NULL
            // to protect from calling a dead object

            // in ! mbMenuBar case our mpMenu is actually a SalNSMenu*
            // so we can safely cast here
            [static_cast<SalNSMenu*>(mpMenu) setSalMenu: nullptr];
            /* #i89860# FIXME:
               using [autorelease] here (and in AquaSalMenuItem::~AquaSalMenuItem)
               instead of [release] fixes an occasional crash. That should
               indicate that we release menus / menu items in the wrong order
               somewhere, but I could not find that case.
            */
            [mpMenu autorelease];
        }
    }
}

bool AquaSalMenu::ShowNativePopupMenu(FloatingWindow * pWin, const Rectangle& rRect, FloatWinPopupFlags nFlags)
{
    // do not use native popup menu when AQUA_NATIVE_MENUS is set to false
    if( ! VisibleMenuBar() ) {
        return false;
    }

    // set offsets for positioning
    const float offset = 9.0;

    // get the pointers
    AquaSalFrame * pParentAquaSalFrame = static_cast<AquaSalFrame *>(pWin->ImplGetWindowImpl()->mpRealParent->ImplGetFrame());
    NSWindow* pParentNSWindow = pParentAquaSalFrame->mpNSWindow;
    NSView* pParentNSView = [pParentNSWindow contentView];
    NSView* pPopupNSView = static_cast<AquaSalFrame *>(pWin->ImplGetWindow()->ImplGetFrame())->mpNSView;
    NSRect popupFrame = [pPopupNSView frame];

    // create frame rect
    NSRect displayPopupFrame = NSMakeRect( rRect.Left()+(offset-1), rRect.Top()+(offset+1), popupFrame.size.width, 0 );
    pParentAquaSalFrame->VCLToCocoa(displayPopupFrame, false);

    // do the same strange semantics as vcl popup windows to arrive at a frame geometry
    // in mirrored UI case; best done by actually executing the same code
    sal_uInt16 nArrangeIndex;
    pWin->SetPosPixel( FloatingWindow::ImplCalcPos( pWin, rRect, nFlags, nArrangeIndex ) );
    displayPopupFrame.origin.x = pWin->ImplGetFrame()->maGeometry.nX - pParentAquaSalFrame->maGeometry.nX + offset;
    displayPopupFrame.origin.y = pWin->ImplGetFrame()->maGeometry.nY - pParentAquaSalFrame->maGeometry.nY + offset;
    pParentAquaSalFrame->VCLToCocoa(displayPopupFrame, false);

    // #i111992# if this menu was opened due to a key event, prevent dispatching that yet again
    if( [pParentNSView respondsToSelector: @selector(clearLastEvent)] )
        [pParentNSView performSelector:@selector(clearLastEvent)];

    // open popup menu
    NSPopUpButtonCell * pPopUpButtonCell = [[NSPopUpButtonCell alloc] initTextCell:@"" pullsDown:NO];
    [pPopUpButtonCell setMenu: mpMenu];
    [pPopUpButtonCell selectItem:nil];
    [AquaA11yWrapper setPopupMenuOpen: YES];
    [pPopUpButtonCell performClickWithFrame:displayPopupFrame inView:pParentNSView];
    [pPopUpButtonCell release];
    [AquaA11yWrapper setPopupMenuOpen: NO];

    return true;
}

int AquaSalMenu::getItemIndexByPos( sal_uInt16 nPos ) const
{
    int nIndex = 0;
    if( nPos == MENU_APPEND )
        nIndex = [mpMenu numberOfItems];
    else
        nIndex = sal::static_int_cast<int>( mbMenuBar ? nPos+1 : nPos );
    return nIndex;
}

const AquaSalFrame* AquaSalMenu::getFrame() const
{
    const AquaSalMenu* pMenu = this;
    while( pMenu && ! pMenu->mpFrame )
        pMenu = pMenu->mpParentSalMenu;
    return pMenu ? pMenu->mpFrame : nullptr;
}

void AquaSalMenu::unsetMainMenu()
{
    pCurrentMenuBar = nullptr;

    // remove items from main menu
    NSMenu* pMenu = [NSApp mainMenu];
    for( int nItems = [pMenu numberOfItems]; nItems > 1; nItems-- )
        [pMenu removeItemAtIndex: 1];
}

void AquaSalMenu::setMainMenu()
{
    DBG_ASSERT( mbMenuBar, "setMainMenu on non menubar" );
    if( mbMenuBar )
    {
        if( pCurrentMenuBar != this )
        {
            unsetMainMenu();
            // insert our items
            for( std::vector<AquaSalMenuItem *>::size_type i = 0; i < maItems.size(); i++ )
            {
                NSMenuItem* pItem = maItems[i]->mpMenuItem;
                [mpMenu insertItem: pItem atIndex: i+1];
            }
            pCurrentMenuBar = this;

            // change status item
            statusLayout();
        }
        enableMainMenu( true );
    }
}

void AquaSalMenu::setDefaultMenu()
{
    NSMenu* pMenu = [NSApp mainMenu];

    unsetMainMenu();

    // insert default items
    std::vector< NSMenuItem* >& rFallbackMenu( GetSalData()->maFallbackMenu );
    for( unsigned int i = 0, nAddItems = rFallbackMenu.size(); i < nAddItems; i++ )
    {
        NSMenuItem* pItem = rFallbackMenu[i];
        if( [pItem menu] == nil )
            [pMenu insertItem: pItem atIndex: i+1];
    }
}

void AquaSalMenu::enableMainMenu( bool bEnable )
{
    NSMenu* pMainMenu = [NSApp mainMenu];
    if( pMainMenu )
    {
        // enable/disable items from main menu
        int nItems = [pMainMenu numberOfItems];
        for( int n = 1; n < nItems; n++ )
        {
            NSMenuItem* pItem = [pMainMenu itemAtIndex: n];
            [pItem setEnabled: bEnable ? YES : NO];
        }
    }
}

void AquaSalMenu::addFallbackMenuItem( NSMenuItem* pNewItem )
{
    initAppMenu();

    std::vector< NSMenuItem* >& rFallbackMenu( GetSalData()->maFallbackMenu );

    // prevent duplicate insertion
    int nItems = rFallbackMenu.size();
    for( int i = 0; i < nItems; i++ )
    {
        if( rFallbackMenu[i] == pNewItem )
            return;
    }

    // push the item to the back and retain it
    [pNewItem retain];
    rFallbackMenu.push_back( pNewItem );

    if( pCurrentMenuBar == nullptr )
        setDefaultMenu();
}

void AquaSalMenu::removeFallbackMenuItem( NSMenuItem* pOldItem )
{
    std::vector< NSMenuItem* >& rFallbackMenu( GetSalData()->maFallbackMenu );

    // find item
    unsigned int nItems = rFallbackMenu.size();
    for( unsigned int i = 0; i < nItems; i++ )
    {
        if( rFallbackMenu[i] == pOldItem )
        {
            // remove item and release
            rFallbackMenu.erase( rFallbackMenu.begin() + i );
            [pOldItem release];

            if( pCurrentMenuBar == nullptr )
                setDefaultMenu();

            return;
        }
    }
}

bool AquaSalMenu::VisibleMenuBar()
{
    // Enable/disable experimental native menus code?

    // To disable native menus, set the environment variable AQUA_NATIVE_MENUS to FALSE

    static const char *pExperimental = getenv ("AQUA_NATIVE_MENUS");

    if ( pExperimental && !strcasecmp(pExperimental, "FALSE") )
        return false;

    // End of experimental code enable/disable part

    return true;
}

void AquaSalMenu::SetFrame( const SalFrame *pFrame )
{
    mpFrame = static_cast<const AquaSalFrame*>(pFrame);
}

void AquaSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    AquaSalMenuItem *pAquaSalMenuItem = static_cast<AquaSalMenuItem*>(pSalMenuItem);

    pAquaSalMenuItem->mpParentMenu = this;
    DBG_ASSERT( pAquaSalMenuItem->mpVCLMenu == nullptr        ||
                pAquaSalMenuItem->mpVCLMenu == mpVCLMenu   ||
                mpVCLMenu == nullptr,
                "resetting menu ?" );
    if( pAquaSalMenuItem->mpVCLMenu )
        mpVCLMenu = pAquaSalMenuItem->mpVCLMenu;

    if( nPos == MENU_APPEND || nPos == maItems.size() )
        maItems.push_back( pAquaSalMenuItem );
    else if( nPos < maItems.size() )
        maItems.insert( maItems.begin() + nPos, pAquaSalMenuItem );
    else
    {
        OSL_FAIL( "invalid item index in insert" );
        return;
    }

    if( ! mbMenuBar || pCurrentMenuBar == this )
        [mpMenu insertItem: pAquaSalMenuItem->mpMenuItem atIndex: getItemIndexByPos(nPos)];
}

void AquaSalMenu::RemoveItem( unsigned nPos )
{
    AquaSalMenuItem* pRemoveItem = nullptr;
    if( nPos == MENU_APPEND || nPos == (maItems.size()-1) )
    {
        pRemoveItem = maItems.back();
        maItems.pop_back();
    }
    else if( nPos < maItems.size() )
    {
        pRemoveItem = maItems[ nPos ];
        maItems.erase( maItems.begin()+nPos );
    }
    else
    {
        OSL_FAIL( "invalid item index in remove" );
        return;
    }

    pRemoveItem->mpParentMenu = nullptr;

    if( ! mbMenuBar || pCurrentMenuBar == this )
        [mpMenu removeItemAtIndex: getItemIndexByPos(nPos)];
}

void AquaSalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned /*nPos*/ )
{
    AquaSalMenuItem *pAquaSalMenuItem = static_cast<AquaSalMenuItem*>(pSalMenuItem);
    AquaSalMenu *subAquaSalMenu = static_cast<AquaSalMenu*>(pSubMenu);

    if (subAquaSalMenu)
    {
        pAquaSalMenuItem->mpSubMenu = subAquaSalMenu;
        if( subAquaSalMenu->mpParentSalMenu == nullptr )
        {
            subAquaSalMenu->mpParentSalMenu = this;
            [pAquaSalMenuItem->mpMenuItem setSubmenu: subAquaSalMenu->mpMenu];

            // set title of submenu
            [subAquaSalMenu->mpMenu setTitle: [pAquaSalMenuItem->mpMenuItem title]];
        }
        else if( subAquaSalMenu->mpParentSalMenu != this )
        {
            // cocoa doesn't allow menus to be submenus of multiple
            // menu items, so place a copy in the menu item instead ?
            // let's hope that NSMenu copy does the right thing
            NSMenu* pCopy = [subAquaSalMenu->mpMenu copy];
            [pAquaSalMenuItem->mpMenuItem setSubmenu: pCopy];

            // set title of submenu
            [pCopy setTitle: [pAquaSalMenuItem->mpMenuItem title]];
        }
    }
    else
    {
        if( pAquaSalMenuItem->mpSubMenu )
        {
            if( pAquaSalMenuItem->mpSubMenu->mpParentSalMenu == this )
                pAquaSalMenuItem->mpSubMenu->mpParentSalMenu = nullptr;
        }
        pAquaSalMenuItem->mpSubMenu = nullptr;
        [pAquaSalMenuItem->mpMenuItem setSubmenu: nil];
    }
}

void AquaSalMenu::CheckItem( unsigned nPos, bool bCheck )
{
    if( nPos < maItems.size() )
    {
        NSMenuItem* pItem = maItems[nPos]->mpMenuItem;
        [pItem setState: bCheck ? NSOnState : NSOffState];
    }
}

void AquaSalMenu::EnableItem( unsigned nPos, bool bEnable )
{
    if( nPos < maItems.size() )
    {
        NSMenuItem* pItem = maItems[nPos]->mpMenuItem;
        [pItem setEnabled: bEnable ? YES : NO];
    }
}

void AquaSalMenu::SetItemImage( unsigned /*nPos*/, SalMenuItem* pSMI, const Image& rImage )
{
    AquaSalMenuItem* pSalMenuItem = static_cast<AquaSalMenuItem*>( pSMI );
    if( ! pSalMenuItem || ! pSalMenuItem->mpMenuItem )
        return;

    NSImage* pImage = CreateNSImage( rImage );

    [pSalMenuItem->mpMenuItem setImage: pImage];
    if( pImage )
        [pImage release];
}

void AquaSalMenu::SetItemText( unsigned /*i_nPos*/, SalMenuItem* i_pSalMenuItem, const OUString& i_rText )
{
    if (!i_pSalMenuItem)
        return;

    AquaSalMenuItem *pAquaSalMenuItem = static_cast<AquaSalMenuItem *>(i_pSalMenuItem);

    // Delete mnemonics
    OUString aText( comphelper::string::remove(i_rText, '~') );

    /* #i90015# until there is a correct solution
       strip out any appended (.*) in menubar entries
    */
    if( mbMenuBar )
    {
        sal_Int32 nPos = aText.lastIndexOf( '(' );
        if( nPos != -1 )
        {
            sal_Int32 nPos2 = aText.indexOf( ')' );
            if( nPos2 != -1 )
                aText = aText.replaceAt( nPos, nPos2-nPos+1, "" );
        }
    }

    NSString* pString = CreateNSString( aText );
    if (pString)
    {
        [pAquaSalMenuItem->mpMenuItem setTitle: pString];
        // if the menu item has a submenu, change its title as well
        if (pAquaSalMenuItem->mpSubMenu)
            [pAquaSalMenuItem->mpSubMenu->mpMenu setTitle: pString];
        [pString release];
    }
}

void AquaSalMenu::SetAccelerator( unsigned /*nPos*/, SalMenuItem* pSalMenuItem, const vcl::KeyCode& rKeyCode, const OUString& /*rKeyName*/ )
{
    sal_uInt16 nModifier;
    sal_Unicode nCommandKey = 0;

    sal_uInt16 nKeyCode=rKeyCode.GetCode();
    if( nKeyCode )
    {
        if ((nKeyCode>=KEY_A) && (nKeyCode<=KEY_Z))           // letter A..Z
            nCommandKey = nKeyCode-KEY_A + 'a';
        else if ((nKeyCode>=KEY_0) && (nKeyCode<=KEY_9))      // numbers 0..9
            nCommandKey = nKeyCode-KEY_0 + '0';
        else if ((nKeyCode>=KEY_F1) && (nKeyCode<=KEY_F26))   // function keys F1..F26
            nCommandKey = nKeyCode-KEY_F1 + NSF1FunctionKey;
        else if( nKeyCode == KEY_REPEAT )
            nCommandKey = NSRedoFunctionKey;
        else if( nKeyCode == KEY_SPACE )
            nCommandKey = ' ';
        else
        {
            switch (nKeyCode)
            {
            case KEY_ADD:
                nCommandKey='+';
                break;
            case KEY_SUBTRACT:
                nCommandKey='-';
                break;
            case KEY_MULTIPLY:
                nCommandKey='*';
                break;
            case KEY_DIVIDE:
                nCommandKey='/';
                break;
            case KEY_POINT:
                nCommandKey='.';
                break;
            case KEY_LESS:
                nCommandKey='<';
                break;
            case KEY_GREATER:
                nCommandKey='>';
                break;
            case KEY_EQUAL:
                nCommandKey='=';
                break;
            }
        }
    }
    else // not even a code ? nonsense -> ignore
        return;

    DBG_ASSERT( nCommandKey, "unmapped accelerator key" );

    nModifier=rKeyCode.GetModifier();

    // should always use the command key
    int nItemModifier = 0;

    if (nModifier & KEY_SHIFT)
    {
        nItemModifier |= NSShiftKeyMask;   // actually useful only for function keys
        if( nKeyCode >= KEY_A && nKeyCode <= KEY_Z )
            nCommandKey = nKeyCode - KEY_A + 'A';
    }

    if (nModifier & KEY_MOD1)
        nItemModifier |= NSCommandKeyMask;

    if(nModifier & KEY_MOD2)
        nItemModifier |= NSAlternateKeyMask;

    if(nModifier & KEY_MOD3)
        nItemModifier |= NSControlKeyMask;

    AquaSalMenuItem *pAquaSalMenuItem = static_cast<AquaSalMenuItem *>(pSalMenuItem);
    NSString* pString = CreateNSString( OUString( &nCommandKey, 1 ) );
    [pAquaSalMenuItem->mpMenuItem setKeyEquivalent: pString];
    [pAquaSalMenuItem->mpMenuItem setKeyEquivalentModifierMask: nItemModifier];
    if (pString)
        [pString release];
}

void AquaSalMenu::GetSystemMenuData( SystemMenuData* )
{
}

AquaSalMenu::MenuBarButtonEntry* AquaSalMenu::findButtonItem( sal_uInt16 i_nItemId )
{
    for( size_t i = 0; i < maButtons.size(); ++i )
    {
        if( maButtons[i].maButton.mnId == i_nItemId )
            return &maButtons[i];
    }
    return nullptr;
}

void AquaSalMenu::statusLayout()
{
    if( GetSalData()->mpStatusItem )
    {
        NSView* pNSView = [GetSalData()->mpStatusItem view];
        if( [pNSView isMemberOfClass: [OOStatusItemView class]] ) // well of course it is
            [(OOStatusItemView*)pNSView layout];
        else
            OSL_FAIL( "someone stole our status view" );
    }
}

bool AquaSalMenu::AddMenuBarButton( const SalMenuButtonItem& i_rNewItem )
{
    if( ! mbMenuBar || ! VisibleMenuBar() )
        return false;

    MenuBarButtonEntry* pEntry = findButtonItem( i_rNewItem.mnId );
    if( pEntry )
    {
        releaseButtonEntry( *pEntry );
        pEntry->maButton = i_rNewItem;
        pEntry->mpNSImage = CreateNSImage( i_rNewItem.maImage );
        if( i_rNewItem.maToolTipText.getLength() )
            pEntry->mpToolTipString = CreateNSString( i_rNewItem.maToolTipText );
    }
    else
    {
        maButtons.push_back( MenuBarButtonEntry( i_rNewItem ) );
        maButtons.back().mpNSImage = CreateNSImage( i_rNewItem.maImage );
        maButtons.back().mpToolTipString = CreateNSString( i_rNewItem.maToolTipText );
    }

    // lazy create status item
    SalData::getStatusItem();

    if( pCurrentMenuBar == this )
        statusLayout();

    return true;
}

void AquaSalMenu::RemoveMenuBarButton( sal_uInt16 i_nId )
{
    MenuBarButtonEntry* pEntry = findButtonItem( i_nId );
    if( pEntry )
    {
        releaseButtonEntry( *pEntry );
        // note: vector guarantees that its contents are in a plain array
        maButtons.erase( maButtons.begin() + (pEntry - &maButtons[0]) );
    }

    if( pCurrentMenuBar == this )
        statusLayout();
}

Rectangle AquaSalMenu::GetMenuBarButtonRectPixel( sal_uInt16 i_nItemId, SalFrame* i_pReferenceFrame )
{
    if( ! i_pReferenceFrame || ! AquaSalFrame::isAlive( static_cast<AquaSalFrame*>(i_pReferenceFrame) ) )
        return Rectangle();

    MenuBarButtonEntry* pEntry = findButtonItem( i_nItemId );

    if( ! pEntry )
        return Rectangle();

    NSStatusItem* pItem = SalData::getStatusItem();
    if( ! pItem )
        return Rectangle();

    NSView* pNSView = [pItem view];
    if( ! pNSView )
        return Rectangle();
    NSWindow* pNSWin = [pNSView window];
    if( ! pNSWin )
        return Rectangle();

    NSRect aRect = [pNSWin convertRectToScreen:[pNSWin frame]];

    // make coordinates relative to reference frame
    static_cast<AquaSalFrame*>(i_pReferenceFrame)->CocoaToVCL( aRect.origin );
    aRect.origin.x -= i_pReferenceFrame->maGeometry.nX;
    aRect.origin.y -= i_pReferenceFrame->maGeometry.nY + aRect.size.height;

    return Rectangle( Point(static_cast<long int>(aRect.origin.x),
                static_cast<long int>(aRect.origin.y)
                ),
              Size( static_cast<long int>(aRect.size.width),
                static_cast<long int>(aRect.size.height)
              )
            );
}

/*
 * SalMenuItem
 */

AquaSalMenuItem::AquaSalMenuItem( const SalItemParams* pItemData ) :
    mnId( pItemData->nId ),
    mpVCLMenu( pItemData->pMenu ),
    mpParentMenu( nullptr ),
    mpSubMenu( nullptr ),
    mpMenuItem( nil )
{
    // Delete mnemonics
    OUString aText( comphelper::string::remove(pItemData->aText, '~') );

    if (pItemData->eType == MenuItemType::SEPARATOR)
    {
        mpMenuItem = [NSMenuItem separatorItem];
        // these can go occasionally go in and out of a menu, ensure their lifecycle
        // also for the release in AquaSalMenuItem destructor
        [mpMenuItem retain];
    }
    else
    {
        mpMenuItem = [[SalNSMenuItem alloc] initWithMenuItem: this];
        [mpMenuItem setEnabled: YES];
        NSString* pString = CreateNSString( aText );
        if (pString)
        {
            [mpMenuItem setTitle: pString];
            [pString release];
        }
        // anything but a separator should set a menu to dispatch to
        DBG_ASSERT( mpVCLMenu, "no menu" );
    }
}

AquaSalMenuItem::~AquaSalMenuItem()
{
    /* #i89860# FIXME:
       using [autorelease] here (and in AquaSalMenu:::~AquaSalMenu) instead of
       [release] fixes an occasional crash. That should indicate that we release
       menus / menu items in the wrong order somewhere, but I
       could not find that case.
    */
    if( mpMenuItem )
        [mpMenuItem autorelease];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
