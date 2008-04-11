/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salmenu.cxx,v $
 * $Revision: 1.8 $
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

#include "saldata.hxx"
#include "salinst.h"
#include "salmenu.h"
#include "salnsmenu.h"
#include "salframe.h"
#include "vcl/svids.hrc"
#include "vcl/cmdevt.hxx"
#include "vcl/window.hxx"
#include "vcl/svapp.hxx"
#include "rtl/ustrbuf.hxx"

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
        String aDialog;
        if( nDialog == SHOWDIALOG_ID_ABOUT )
            aDialog = String( RTL_CONSTASCII_USTRINGPARAM( "ABOUT" ) );
        else if( nDialog == SHOWDIALOG_ID_PREFERENCES )
            aDialog = String( RTL_CONSTASCII_USTRINGPARAM( "PREFERENCES" ) );
        const ApplicationEvent* pAppEvent = new ApplicationEvent( String(),
                                                                  ApplicationAddress(),
                                                                  ByteString( "SHOWDIALOG" ),
                                                                  aDialog );
        AquaSalInstance::aAppEventList.push_back( pAppEvent );
    }
}

-(void)showPreferences: (id) sender
{
    [self showDialog: SHOWDIALOG_ID_PREFERENCES];
}
-(void)showAbout: (id) sender
{
    [self showDialog: SHOWDIALOG_ID_ABOUT];
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
                    String aAbout( ResId( SV_STDTEXT_ABOUT, *pMgr ) );
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
                    String aPref( ResId( SV_STDTEXT_PREFERENCES, *pMgr ) );
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


                    // FIXME: there probably is a more canonical way to set the string
                    // in the application menu

                    // replace instances of "NewApplication" (artifact from out .nib)
                    // by the application name

                    // get the application name
                    ResHookProc pProc = ResMgr::GetReadStringHook();
                    String aAppName( RTL_CONSTASCII_USTRINGPARAM( "%PRODUCTNAME" ) );
                    if( pProc )
                        pProc( aAppName );

                    int nItems = [pAppMenu numberOfItems];
                    for( int i = 4; i < nItems; i++ )
                    {
                        NSMenuItem* pItem = [pAppMenu itemAtIndex: i];
                        if( pItem != nil )
                        {
                            OUString aTitle( GetOUString( [pItem title] ) );
                            sal_Int32 nInd = aTitle.indexOfAsciiL( "NewApplication", 14 );
                            if( nInd != -1 )
                            {
                                rtl::OUStringBuffer aBuf( aTitle.getLength()*2 );
                                aBuf.append( aTitle.copy( 0, nInd ) );
                                aBuf.append( aAppName );
                                if( aTitle.getLength() > nInd + 14 )
                                    aBuf.append( aTitle.copy( nInd+14, aTitle.getLength() - nInd - 14 ) );
                                [pItem setTitle: [CreateNSString( aBuf.makeStringAndClear() ) autorelease]];
                            }
                        }
                    }
                }
            }
        }
    }
}

// =======================================================================

SalMenu* AquaSalInstance::CreateMenu( BOOL bMenuBar )
{
    initAppMenu();

    AquaSalMenu *pAquaSalMenu = new AquaSalMenu( bMenuBar );

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


// =======================================================================


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
        [mpMenu setDelegate: mpMenu];
    }
    else
    {
        mpMenu = [NSApp mainMenu];
    }
    [mpMenu setAutoenablesItems: NO];
}

AquaSalMenu::~AquaSalMenu()
{
    // is this leaking in some cases ? the release often leads to a duplicate release
    // it seems the parent item gets ownership of the menu
    if( mpMenu && ! mbMenuBar )
    {
        // the system may still hold a reference on mpMenu
        // so set the pointer to this AquaSalMenu to NULL
        // to protect from calling a dead object

        // in ! mbMenuBar case our mpMenu is actually a SalNSMenu*
        // so we can safely cast here
        [static_cast<SalNSMenu*>(mpMenu) setSalMenu: NULL];
        [mpMenu release];
    }
}

int AquaSalMenu::getItemIndexByPos( USHORT nPos ) const
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
    DBG_ASSERT( mbMenuBar, "unsetMainMenu on non menubar" );
    if( mbMenuBar )
    {
        pCurrentMenuBar = NULL;
        // remove items from main menu
        for( int nItems = [mpMenu numberOfItems]; nItems > 1; nItems-- )
            [mpMenu removeItemAtIndex: 1];
    }
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
            for( unsigned int i = 0; i < maItems.size(); i++ )
            {
                NSMenuItem* pItem = maItems[i]->mpMenuItem;
                [mpMenu insertItem: pItem atIndex: i+1];
            }
            pCurrentMenuBar = this;
        }
        enableMainMenu( true );
    }
}

void AquaSalMenu::setDefaultMenu()
{
    NSMenu* pMenu = [NSApp mainMenu];

    pCurrentMenuBar = NULL;
    // remove items from main menu
    for( int nItems = [pMenu numberOfItems]; nItems > 1; nItems-- )
        [pMenu removeItemAtIndex: 1];

    // insert default items
    std::vector< NSMenuItem* >& rFallbackMenu( GetSalData()->maFallbackMenu );
    unsigned int nItems = rFallbackMenu.size();
    for( unsigned int i = 0; i < nItems; i++ )
    {
        NSMenuItem* pItem = rFallbackMenu[i];
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

    if( pCurrentMenuBar == NULL )
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

            if( pCurrentMenuBar == NULL )
                setDefaultMenu();

            return;
        }
    }
}

BOOL AquaSalMenu::VisibleMenuBar()
{
    // Enable/disable experimental native menus code?
    //
    // To disable native menus, set the environment variable AQUA_NATIVE_MENUS to FALSE

    static const char *pExperimental = getenv ("AQUA_NATIVE_MENUS");

    if ( pExperimental && !strcasecmp(pExperimental, "FALSE") )
        return FALSE;

    // End of experimental code enable/disable part

    return TRUE;
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
        DBG_ERROR( "invalid item index in insert" );

    if( ! mbMenuBar || pCurrentMenuBar == this )
        [mpMenu insertItem: pAquaSalMenuItem->mpMenuItem atIndex: getItemIndexByPos(nPos)];
}

void AquaSalMenu::RemoveItem( unsigned nPos )
{
    if( nPos == MENU_APPEND )
        maItems.pop_back();
    else if( nPos < maItems.size() )
        maItems.erase( maItems.begin()+nPos );
    else
        DBG_ERROR( "invalid item index in remove" );
    if( ! mbMenuBar || pCurrentMenuBar == this )
        [mpMenu removeItemAtIndex: getItemIndexByPos(nPos)];
}

void AquaSalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
    AquaSalMenuItem *pAquaSalMenuItem = static_cast<AquaSalMenuItem*>(pSalMenuItem);
    AquaSalMenu *subAquaSalMenu = static_cast<AquaSalMenu*>(pSubMenu);

    // FIXME: in svtools - workben - svdem, pSubMenu is NULL!
    if (subAquaSalMenu)
    {
        pAquaSalMenuItem->mpSubMenu = subAquaSalMenu;
        if( subAquaSalMenu->mpParentSalMenu == NULL )
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
}

void AquaSalMenu::CheckItem( unsigned nPos, BOOL bCheck )
{
    if( nPos < maItems.size() )
    {
        NSMenuItem* pItem = maItems[nPos]->mpMenuItem;
        [pItem setState: bCheck ? NSOnState : NSOffState];
    }
}

void AquaSalMenu::EnableItem( unsigned nPos, BOOL bEnable )
{
    if( nPos < maItems.size() )
    {
        NSMenuItem* pItem = maItems[nPos]->mpMenuItem;
        [pItem setEnabled: bEnable ? YES : NO];
    }
}

void AquaSalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage )
{
}

void AquaSalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText )
{
    if (!pSalMenuItem)
        return;

    AquaSalMenuItem *pAquaSalMenuItem = (AquaSalMenuItem *) pSalMenuItem;

    String aText( rText );

    // Delete mnemonics
    aText.EraseAllChars( '~' );

    NSString* pString = CreateNSString( aText );
    [pAquaSalMenuItem->mpMenuItem setTitle: pString];
    // if the menu item has a submenu, change its title as well
    if (pAquaSalMenuItem->mpSubMenu)
        [pAquaSalMenuItem->mpSubMenu->mpMenu setTitle: pString];
    if (pString)
        [pString release];
}

void AquaSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName )
{
    USHORT nModifier;
    sal_Unicode nCommandKey = 0;

    USHORT nKeyCode=rKeyCode.GetCode();
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

    nModifier=rKeyCode.GetAllModifier();

    // should always use the command key
    int nItemModifier = NSCommandKeyMask;

    // ignore KEY_MOD1, Ctrl replaced by Command
    #if 0
    if (nModifier & KEY_MOD1)
        nItemModifier |= NSControlKeyMask; // for the Ctrl key
    #endif

    if (nModifier & KEY_SHIFT)
    {
        nItemModifier |= NSShiftKeyMask;   // actually useful only for function keys
        if( nKeyCode >= KEY_A && nKeyCode <= KEY_Z )
            nCommandKey = nKeyCode - KEY_A + 'A';
    }

    if(nModifier & KEY_MOD2)
        nItemModifier |= NSAlternateKeyMask;

    AquaSalMenuItem *pAquaSalMenuItem = (AquaSalMenuItem *) pSalMenuItem;
    NSString* pString = CreateNSString( rtl::OUString( &nCommandKey, 1 ) );
    [pAquaSalMenuItem->mpMenuItem setKeyEquivalent: pString];
    [pAquaSalMenuItem->mpMenuItem setKeyEquivalentModifierMask: nItemModifier];
    if (pString)
        [pString release];
}

void AquaSalMenu::GetSystemMenuData( SystemMenuData* pData )
{
}

// =======================================================================

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
    String aText( pItemData->aText );

    // Delete mnemonics
    aText.EraseAllChars( '~' );

    if (pItemData->eType == MENUITEM_SEPARATOR)
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
        [mpMenuItem setTitle: pString];
        if (pString)
            [pString release];

        // anything but a separator should set a menu to dispatch to
        DBG_ASSERT( mpVCLMenu, "no menu" );
    }
}

AquaSalMenuItem::~AquaSalMenuItem()
{
    if( mpMenuItem )
        [mpMenuItem release];
}

// -------------------------------------------------------------------

