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
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <objc/objc-runtime.h>

#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>

#include <vcl/commandevent.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>

#include <osx/saldata.hxx>
#include <osx/salinst.h>
#include <osx/salmenu.h>
#include <osx/salnsmenu.h>
#include <osx/salframe.h>
#include <osx/a11ywrapper.h>
#include <quartz/utils.h>
#include <strings.hrc>
#include <window.h>

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
            ApplicationEvent::Type::ShowDialog, aDialog);
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
    static bool bInitialized = false;
    if (bInitialized)
        return;
    bInitialized = true;

    NSMenu* pAppMenu = nil;
    NSMenuItem* pNewItem = nil;

    NSMenu* pMainMenu = [[[NSMenu alloc] initWithTitle: @"Main Menu"] autorelease];
    pNewItem = [pMainMenu addItemWithTitle: @"Application"
        action: nil
        keyEquivalent: @""];
    pAppMenu = [[[NSMenu alloc] initWithTitle: @"Application"] autorelease];
    [pNewItem setSubmenu: pAppMenu];
    [NSApp setMainMenu: pMainMenu];

    pMainMenuSelector = [[MainMenuSelector alloc] init];

    // about
    NSString* pString = CreateNSString(VclResId(SV_STDTEXT_ABOUT));
    pNewItem = [pAppMenu addItemWithTitle: pString
        action: @selector(showAbout:)
        keyEquivalent: @""];
    [pString release];
    [pNewItem setTarget: pMainMenuSelector];

    [pAppMenu addItem:[NSMenuItem separatorItem]];

    // preferences
    pString = CreateNSString(VclResId(SV_STDTEXT_PREFERENCES));
    pNewItem = [pAppMenu addItemWithTitle: pString
        action: @selector(showPreferences:)
        keyEquivalent: @","];
    [pString release];
SAL_WNODEPRECATED_DECLARATIONS_PUSH
    // 'NSCommandKeyMask' is deprecated: first deprecated in macOS 10.12
    [pNewItem setKeyEquivalentModifierMask: NSCommandKeyMask];
SAL_WNODEPRECATED_DECLARATIONS_POP
    [pNewItem setTarget: pMainMenuSelector];

    [pAppMenu addItem:[NSMenuItem separatorItem]];

    // Services item and menu
    pString = CreateNSString(VclResId(SV_MENU_MAC_SERVICES));
    pNewItem = [pAppMenu addItemWithTitle: pString
        action: nil
        keyEquivalent: @""];
    NSMenu *servicesMenu = [[[NSMenu alloc] initWithTitle:@"Services"] autorelease];
    [pNewItem setSubmenu: servicesMenu];
    [NSApp setServicesMenu: servicesMenu];

    [pAppMenu addItem:[NSMenuItem separatorItem]];

    // Hide Application
    pString = CreateNSString(VclResId(SV_MENU_MAC_HIDEAPP));
    [pAppMenu addItemWithTitle: pString
        action:@selector(hide:)
        keyEquivalent:@"h"];
    [pString release];

    // Hide Others
    pString = CreateNSString(VclResId(SV_MENU_MAC_HIDEALL));
    [pAppMenu addItemWithTitle: pString
        action:@selector(hideOtherApplications:)
        keyEquivalent:@"h"];
    [pString release];
SAL_WNODEPRECATED_DECLARATIONS_PUSH
    // 'NSCommandKeyMask' is deprecated: first deprecated in macOS 10.12
    [pNewItem setKeyEquivalentModifierMask: NSCommandKeyMask | NSAlternateKeyMask];
SAL_WNODEPRECATED_DECLARATIONS_POP

    // Show All
    pString = CreateNSString(VclResId(SV_MENU_MAC_SHOWALL));
    [pAppMenu addItemWithTitle: pString
        action:@selector(unhideAllApplications:)
        keyEquivalent:@""];
    [pString release];

    [pAppMenu addItem:[NSMenuItem separatorItem]];

    // Quit
    pString = CreateNSString(VclResId(SV_MENU_MAC_QUITAPP));
    [pAppMenu addItemWithTitle: pString
        action:@selector(terminate:)
        keyEquivalent:@"q"];
    [pString release];
}

std::unique_ptr<SalMenu> AquaSalInstance::CreateMenu( bool bMenuBar, Menu* pVCLMenu )
{
    initAppMenu();

    AquaSalMenu *pAquaSalMenu = new AquaSalMenu( bMenuBar );
    pAquaSalMenu->mpVCLMenu = pVCLMenu;

    return std::unique_ptr<SalMenu>(pAquaSalMenu);
}

std::unique_ptr<SalMenuItem> AquaSalInstance::CreateMenuItem( const SalItemParams & rItemData )
{
    AquaSalMenuItem *pSalMenuItem = new AquaSalMenuItem( &rItemData );

    return std::unique_ptr<SalMenuItem>(pSalMenuItem);
}

/*
 * AquaSalMenu
 */

AquaSalMenu::AquaSalMenu( bool bMenuBar ) :
    mbMenuBar( bMenuBar ),
    mpMenu( nil ),
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

bool AquaSalMenu::ShowNativePopupMenu(FloatingWindow * pWin, const tools::Rectangle& rRect, FloatWinPopupFlags nFlags)
{
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
    SAL_WARN_IF( !mbMenuBar, "vcl", "setMainMenu on non menubar" );
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
        [pItem setState: bCheck ? NSControlStateValueOn : NSControlStateValueOff];
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
    OUString aText = i_rText.replaceAll("~", "");

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

    if (aText.endsWith("...", &aText))
        aText += u"\u2026";

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
            case KEY_SEMICOLON:
                nCommandKey=';';
                break;
            case KEY_BACKSPACE:
                nCommandKey=u'\x232b';
                break;
            case KEY_PAGEUP:
                nCommandKey=u'\x21de';
                break;
            case KEY_PAGEDOWN:
                nCommandKey=u'\x21df';
                break;
            case KEY_UP:
                nCommandKey=u'\x21e1';
                break;
            case KEY_DOWN:
                nCommandKey=u'\x21e3';
                break;
            case KEY_RETURN:
                nCommandKey=u'\x21a9';
                break;
            case KEY_BRACKETLEFT:
                nCommandKey='[';
                break;
            case KEY_BRACKETRIGHT:
                nCommandKey=']';
                break;
            }
        }
    }
    else // not even a code ? nonsense -> ignore
        return;

    SAL_WARN_IF( !nCommandKey, "vcl", "unmapped accelerator key" );

    nModifier=rKeyCode.GetModifier();

    // should always use the command key
    int nItemModifier = 0;

SAL_WNODEPRECATED_DECLARATIONS_PUSH
        // 'NSAlternateKeyMask' is deprecated: first deprecated in macOS 10.12
        // 'NSCommandKeyMask' is deprecated: first deprecated in macOS 10.12
        // 'NSControlKeyMask' is deprecated: first deprecated in macOS 10.12
        // 'NSShiftKeyMask' is deprecated: first deprecated in macOS 10.12
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
SAL_WNODEPRECATED_DECLARATIONS_POP

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
SAL_WNODEPRECATED_DECLARATIONS_PUSH
            // "'view' is deprecated: first deprecated in macOS 10.14 - Use the standard button
            // property instead"
        NSView* pNSView = [GetSalData()->mpStatusItem view];
SAL_WNODEPRECATED_DECLARATIONS_POP
        if( [pNSView isMemberOfClass: [OOStatusItemView class]] ) // well of course it is
            [static_cast<OOStatusItemView*>(pNSView) layout];
        else
            OSL_FAIL( "someone stole our status view" );
    }
}

bool AquaSalMenu::AddMenuBarButton( const SalMenuButtonItem& i_rNewItem )
{
    if( ! mbMenuBar  )
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

tools::Rectangle AquaSalMenu::GetMenuBarButtonRectPixel( sal_uInt16 i_nItemId, SalFrame* i_pReferenceFrame )
{
    if( ! i_pReferenceFrame || ! AquaSalFrame::isAlive( static_cast<AquaSalFrame*>(i_pReferenceFrame) ) )
        return tools::Rectangle();

    MenuBarButtonEntry* pEntry = findButtonItem( i_nItemId );

    if( ! pEntry )
        return tools::Rectangle();

    NSStatusItem* pItem = SalData::getStatusItem();
    if( ! pItem )
        return tools::Rectangle();

SAL_WNODEPRECATED_DECLARATIONS_PUSH
        // "'view' is deprecated: first deprecated in macOS 10.14 - Use the standard button property
        // instead"
    NSView* pNSView = [pItem view];
SAL_WNODEPRECATED_DECLARATIONS_POP
    if( ! pNSView )
        return tools::Rectangle();
    NSWindow* pNSWin = [pNSView window];
    if( ! pNSWin )
        return tools::Rectangle();

    NSRect aRect = [pNSWin convertRectToScreen:[pNSWin frame]];

    // make coordinates relative to reference frame
    static_cast<AquaSalFrame*>(i_pReferenceFrame)->CocoaToVCL( aRect.origin );
    aRect.origin.x -= i_pReferenceFrame->maGeometry.nX;
    aRect.origin.y -= i_pReferenceFrame->maGeometry.nY + aRect.size.height;

    return tools::Rectangle( Point(static_cast<long int>(aRect.origin.x),
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

        // peel mnemonics because on mac there are no such things for menu items
        NSString* pString = CreateNSString( pItemData->aText.replaceAll( "~", "" ) );
        if (pString)
        {
            [mpMenuItem setTitle: pString];
            [pString release];
        }
        // anything but a separator should set a menu to dispatch to
        SAL_WARN_IF( !mpVCLMenu, "vcl", "no menu" );
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
