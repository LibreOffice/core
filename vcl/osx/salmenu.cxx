/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <comphelper/string.hxx>

#include "rtl/ustrbuf.hxx"

#include "vcl/cmdevt.hxx"
#include "vcl/floatwin.hxx"
#include "vcl/window.hxx"
#include "vcl/svapp.hxx"

#include "osx/saldata.hxx"
#include "osx/salinst.h"
#include "osx/salmenu.h"
#include "osx/salnsmenu.h"
#include "osx/salframe.h"
#include "osx/a11ywrapper.h"
#include "quartz/utils.h"

#include "svids.hrc"
#include "window.h"

#include <objc/objc-runtime.h>

const AquaSalMenu* AquaSalMenu::pCurrentMenuBar = NULL;

@interface MainMenuSelector : NSObject
{
}
-(void)showDialog: (int)nDialog;
-(void)showPreferences: (id)sender;
-(void)showAbout: (id)sender;
@end

@implementation MainMenuSelector
-(void)showDialog: (int)nDialog
{
    if( AquaSalMenu::pCurrentMenuBar )
    {
        const AquaSalFrame* pFrame = AquaSalMenu::pCurrentMenuBar->mpFrame;
        if( pFrame && AquaSalFrame::isAlive( pFrame ) )
        {
            pFrame->CallCallback( SALEVENT_SHOWDIALOG, reinterpret_cast<void*>(nDialog) );
        }
    }
    else
    {
        OUString aDialog;
        if( nDialog == SHOWDIALOG_ID_ABOUT )
            aDialog = "ABOUT";
        else if( nDialog == SHOWDIALOG_ID_PREFERENCES )
            aDialog = "PREFERENCES";
        const ApplicationEvent* pAppEvent = new ApplicationEvent(
            ApplicationEvent::TYPE_SHOWDIALOG, aDialog);
        AquaSalInstance::aAppEventList.push_back( pAppEvent );
    }
}

-(void)showPreferences: (id) sender
{
    (void)sender;
    YIELD_GUARD;

    [self showDialog: SHOWDIALOG_ID_PREFERENCES];
}
-(void)showAbout: (id) sender
{
    (void)sender;
    YIELD_GUARD;

    [self showDialog: SHOWDIALOG_ID_ABOUT];
}
@end



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
            
            NSMenu* pMainMenu = [NSApp mainMenu];
            if( pMainMenu != nil )
            {
                
                pMainMenuSelector = [[MainMenuSelector alloc] init];

                
                NSMenu* pAppMenu = [[pMainMenu itemAtIndex: 0] submenu];
                if( pAppMenu )
                {
                    
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

                    

                    
                    
                    pNewItem = [pAppMenu itemAtIndex: 4];
                    if( pNewItem )
                    {
                        pString = CreateNSString( OUString( ResId( SV_MENU_MAC_SERVICES, *pMgr ) ) );
                        [pNewItem  setTitle: pString];
                        if( pString )
                            [pString release];
                    }

                    
                    pNewItem = [pAppMenu itemAtIndex: 6];
                    if( pNewItem )
                    {
                        pString = CreateNSString( OUString( ResId( SV_MENU_MAC_HIDEAPP, *pMgr ) ) );
                        [pNewItem  setTitle: pString];
                        if( pString )
                            [pString release];
                    }

                    
                    pNewItem = [pAppMenu itemAtIndex: 7];
                    if( pNewItem )
                    {
                        pString = CreateNSString( OUString( ResId( SV_MENU_MAC_HIDEALL, *pMgr ) ) );
                        [pNewItem  setTitle: pString];
                        if( pString )
                            [pString release];
                    }

                    
                    pNewItem = [pAppMenu itemAtIndex: 8];
                    if( pNewItem )
                    {
                        pString = CreateNSString( OUString( ResId( SV_MENU_MAC_SHOWALL, *pMgr ) ) );
                        [pNewItem  setTitle: pString];
                        if( pString )
                            [pString release];
                    }

                    
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



SalMenu* AquaSalInstance::CreateMenu( sal_Bool bMenuBar, Menu* pVCLMenu )
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
        return NULL;

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
    mpVCLMenu( NULL ),
    mpFrame( NULL ),
    mpParentSalMenu( NULL )
{
    if( ! mbMenuBar )
    {
        mpMenu = [[SalNSMenu alloc] initWithMenu: this];
        [mpMenu setDelegate: (id<NSMenuDelegate>)mpMenu];
    }
    else
    {
        mpMenu = [NSApp mainMenu];
    }
    [mpMenu setAutoenablesItems: NO];
}

AquaSalMenu::~AquaSalMenu()
{
    
    
    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) && mpFrame->mpMenu == this )
        const_cast<AquaSalFrame*>(mpFrame)->mpMenu = NULL;

    
    for( size_t i = 0; i < maButtons.size(); i++ )
        releaseButtonEntry( maButtons[i] );
    maButtons.clear();

    
    
    if( mpMenu )
    {
        if( mbMenuBar )
        {
            if( pCurrentMenuBar == this )
            {
                
                setDefaultMenu();
            }
        }
        else
            
        {
            
            

            
            
            [static_cast<SalNSMenu*>(mpMenu) setSalMenu: NULL];
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

sal_Int32 removeUnusedItemsRunner(NSMenu * pMenu)
{
    NSArray * elements = [pMenu itemArray];
    NSEnumerator * it = [elements objectEnumerator];
    id elem;
    NSMenuItem * lastDisplayedMenuItem = nil;
    sal_Int32 drawnItems = 0;
    bool firstEnabledItemIsNoSeparator = false;
    while((elem=[it nextObject]) != nil) {
        NSMenuItem * item = static_cast<NSMenuItem *>(elem);
        if( (![item isEnabled] && ![item isSeparatorItem]) || ([item isSeparatorItem] && (lastDisplayedMenuItem != nil && [lastDisplayedMenuItem isSeparatorItem])) ) {
            [[item menu]removeItem:item];
        } else {
            if( ! firstEnabledItemIsNoSeparator && [item isSeparatorItem] ) {
                [[item menu]removeItem:item];
            } else {
                firstEnabledItemIsNoSeparator = true;
                lastDisplayedMenuItem = item;
                drawnItems++;
                if( [item hasSubmenu] ) {
                    removeUnusedItemsRunner( [item submenu] );
                }
            }
        }
    }
    if( lastDisplayedMenuItem != nil && [lastDisplayedMenuItem isSeparatorItem]) {
        [[lastDisplayedMenuItem menu]removeItem:lastDisplayedMenuItem];
    }
    return drawnItems;
}

bool AquaSalMenu::ShowNativePopupMenu(FloatingWindow * pWin, const Rectangle& rRect, sal_uLong nFlags)
{
    
    if( ! VisibleMenuBar() ) {
        return false;
    }

    
    const float offset = 9.0;

    
    AquaSalFrame * pParentAquaSalFrame = (AquaSalFrame *) pWin->ImplGetWindowImpl()->mpRealParent->ImplGetFrame();
    NSWindow* pParentNSWindow = pParentAquaSalFrame->mpNSWindow;
    NSView* pParentNSView = [pParentNSWindow contentView];
    NSView* pPopupNSView = ((AquaSalFrame *) pWin->ImplGetWindow()->ImplGetFrame())->mpNSView;
    NSRect popupFrame = [pPopupNSView frame];

    
    
    NSMenu* pCopyMenu = [mpMenu copy];

    
    removeUnusedItemsRunner( pCopyMenu );

    
    NSRect displayPopupFrame = NSMakeRect( rRect.Left()+(offset-1), rRect.Top()+(offset+1), popupFrame.size.width, 0 );
    pParentAquaSalFrame->VCLToCocoa(displayPopupFrame, false);

    
    
    sal_uInt16 nArrangeIndex;
    pWin->SetPosPixel( pWin->ImplCalcPos( pWin, rRect, nFlags, nArrangeIndex ) );
    displayPopupFrame.origin.x = pWin->ImplGetFrame()->maGeometry.nX - pParentAquaSalFrame->maGeometry.nX + offset;
    displayPopupFrame.origin.y = pWin->ImplGetFrame()->maGeometry.nY - pParentAquaSalFrame->maGeometry.nY + offset;
    pParentAquaSalFrame->VCLToCocoa(displayPopupFrame, false);

    
    if( [pParentNSView respondsToSelector: @selector(clearLastEvent)] )
        [pParentNSView performSelector:@selector(clearLastEvent)];

    
    NSPopUpButtonCell * pPopUpButtonCell = [[NSPopUpButtonCell alloc] initTextCell:@"" pullsDown:NO];
    [pPopUpButtonCell setMenu: pCopyMenu];
    [pPopUpButtonCell selectItem:nil];
    [AquaA11yWrapper setPopupMenuOpen: YES];
    [pPopUpButtonCell performClickWithFrame:displayPopupFrame inView:pParentNSView];
    [pPopUpButtonCell release];
    [AquaA11yWrapper setPopupMenuOpen: NO];

    
    [pCopyMenu release];
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
    return pMenu ? pMenu->mpFrame : NULL;
}

void AquaSalMenu::unsetMainMenu()
{
    pCurrentMenuBar = NULL;

    
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
            
            for( unsigned int i = 0; i < maItems.size(); i++ )
            {
                NSMenuItem* pItem = maItems[i]->mpMenuItem;
                [mpMenu insertItem: pItem atIndex: i+1];
            }
            pCurrentMenuBar = this;

            
            statusLayout();
        }
        enableMainMenu( true );
    }
}

void AquaSalMenu::setDefaultMenu()
{
    NSMenu* pMenu = [NSApp mainMenu];

    unsetMainMenu();

    
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

    
    int nItems = rFallbackMenu.size();
    for( int i = 0; i < nItems; i++ )
    {
        if( rFallbackMenu[i] == pNewItem )
            return;
    }

    
    [pNewItem retain];
    rFallbackMenu.push_back( pNewItem );

    if( pCurrentMenuBar == NULL )
        setDefaultMenu();
}

void AquaSalMenu::removeFallbackMenuItem( NSMenuItem* pOldItem )
{
    std::vector< NSMenuItem* >& rFallbackMenu( GetSalData()->maFallbackMenu );

    
    unsigned int nItems = rFallbackMenu.size();
    for( unsigned int i = 0; i < nItems; i++ )
    {
        if( rFallbackMenu[i] == pOldItem )
        {
            
            rFallbackMenu.erase( rFallbackMenu.begin() + i );
            [pOldItem release];

            if( pCurrentMenuBar == NULL )
                setDefaultMenu();

            return;
        }
    }
}

bool AquaSalMenu::VisibleMenuBar()
{
    
    //
    

    static const char *pExperimental = getenv ("AQUA_NATIVE_MENUS");

    if ( pExperimental && !strcasecmp(pExperimental, "FALSE") )
        return false;

    

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
    DBG_ASSERT( pAquaSalMenuItem->mpVCLMenu == NULL        ||
                pAquaSalMenuItem->mpVCLMenu == mpVCLMenu   ||
                mpVCLMenu == NULL,
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
    AquaSalMenuItem* pRemoveItem = NULL;
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

    pRemoveItem->mpParentMenu = NULL;

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
        if( subAquaSalMenu->mpParentSalMenu == NULL )
        {
            subAquaSalMenu->mpParentSalMenu = this;
            [pAquaSalMenuItem->mpMenuItem setSubmenu: subAquaSalMenu->mpMenu];

            
            [subAquaSalMenu->mpMenu setTitle: [pAquaSalMenuItem->mpMenuItem title]];
        }
        else if( subAquaSalMenu->mpParentSalMenu != this )
        {
            
            
            
            NSMenu* pCopy = [subAquaSalMenu->mpMenu copy];
            [pAquaSalMenuItem->mpMenuItem setSubmenu: pCopy];

            
            [pCopy setTitle: [pAquaSalMenuItem->mpMenuItem title]];
        }
    }
    else
    {
        if( pAquaSalMenuItem->mpSubMenu )
        {
            if( pAquaSalMenuItem->mpSubMenu->mpParentSalMenu == this )
                pAquaSalMenuItem->mpSubMenu->mpParentSalMenu = NULL;
        }
        pAquaSalMenuItem->mpSubMenu = NULL;
        [pAquaSalMenuItem->mpMenuItem setSubmenu: nil];
    }
}

void AquaSalMenu::CheckItem( unsigned nPos, sal_Bool bCheck )
{
    if( nPos < maItems.size() )
    {
        NSMenuItem* pItem = maItems[nPos]->mpMenuItem;
        [pItem setState: bCheck ? NSOnState : NSOffState];
    }
}

void AquaSalMenu::EnableItem( unsigned nPos, sal_Bool bEnable )
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

    AquaSalMenuItem *pAquaSalMenuItem = (AquaSalMenuItem *) i_pSalMenuItem;

    
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
        
        if (pAquaSalMenuItem->mpSubMenu)
            [pAquaSalMenuItem->mpSubMenu->mpMenu setTitle: pString];
        [pString release];
    }
}

void AquaSalMenu::SetAccelerator( unsigned /*nPos*/, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const OUString& /*rKeyName*/ )
{
    sal_uInt16 nModifier;
    sal_Unicode nCommandKey = 0;

    sal_uInt16 nKeyCode=rKeyCode.GetCode();
    if( nKeyCode )
    {
        if ((nKeyCode>=KEY_A) && (nKeyCode<=KEY_Z))           
            nCommandKey = nKeyCode-KEY_A + 'a';
        else if ((nKeyCode>=KEY_0) && (nKeyCode<=KEY_9))      
            nCommandKey = nKeyCode-KEY_0 + '0';
        else if ((nKeyCode>=KEY_F1) && (nKeyCode<=KEY_F26))   
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
    else 
        return;

    DBG_ASSERT( nCommandKey, "unmapped accelerator key" );

    nModifier=rKeyCode.GetAllModifier();

    
    int nItemModifier = 0;

    if (nModifier & KEY_SHIFT)
    {
        nItemModifier |= NSShiftKeyMask;   
        if( nKeyCode >= KEY_A && nKeyCode <= KEY_Z )
            nCommandKey = nKeyCode - KEY_A + 'A';
    }

    if (nModifier & KEY_MOD1)
        nItemModifier |= NSCommandKeyMask;

    if(nModifier & KEY_MOD2)
        nItemModifier |= NSAlternateKeyMask;

    if(nModifier & KEY_MOD3)
        nItemModifier |= NSControlKeyMask;

    AquaSalMenuItem *pAquaSalMenuItem = (AquaSalMenuItem *) pSalMenuItem;
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
    return NULL;
}

void AquaSalMenu::statusLayout()
{
    if( GetSalData()->mpStatusItem )
    {
        NSView* pNSView = [GetSalData()->mpStatusItem view];
        if( [pNSView isMemberOfClass: [OOStatusItemView class]] ) 
            [(OOStatusItemView*)pNSView layout];
        else
            OSL_FAIL( "someone stole our status view" );
    }
}

void AquaSalMenu::releaseButtonEntry( MenuBarButtonEntry& i_rEntry )
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

    NSRect aRect = [pNSWin frame];
    aRect.origin = [pNSWin convertBaseToScreen: NSMakePoint( 0, 0 )];

    
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
    mpParentMenu( NULL ),
    mpSubMenu( NULL ),
    mpMenuItem( nil )
{
    
    OUString aText( comphelper::string::remove(pItemData->aText, '~') );

    if (pItemData->eType == MENUITEM_SEPARATOR)
    {
        mpMenuItem = [NSMenuItem separatorItem];
        
        
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
