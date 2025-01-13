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


#include <unotools/moduleoptions.hxx>
#include <unotools/dynamicmenuoptions.hxx>
#include <unotools/historyoptions.hxx>
#include <officecfg/Office/Common.hxx>
#include <rtl/ustring.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.h>
#include <osl/diagnose.h>
#include <comphelper/sequenceashashmap.hxx>
#include <sfx2/app.hxx>
#include <sal/macros.h>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/image.hxx>
#include <svtools/imagemgr.hxx>
#include <shutdownicon.hxx>

#include <com/sun/star/util/XStringWidth.hpp>

#include <cppuhelper/implbase.hxx>

#include <set>
#include <vector>

#include <premac.h>
#include <objc/objc-runtime.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

#define MI_OPEN                    1
#define MI_WRITER                  2
#define MI_CALC                    3
#define MI_IMPRESS                 4
#define MI_DRAW                    5
#define MI_BASE                    6
#define MI_MATH                    7
#define MI_TEMPLATE                8
#define MI_STARTMODULE             9

#define UNO_TOGGLECURRENTMODULE_COMMAND ".uno:ToggleCurrentModule"

@interface QSMenuExecute : NSObject
{
}
-(void)executeMenuItem: (NSMenuItem*)pItem;
-(void)dockIconClicked: (NSObject*)pSender;
@end

@implementation QSMenuExecute
-(void)executeMenuItem: (NSMenuItem*)pItem
{
    SolarMutexGuard aGuard;

    switch( [pItem tag] )
    {
    case MI_OPEN:
        ShutdownIcon::FileOpen();
        break;
    case MI_WRITER:
        ShutdownIcon::OpenURL( WRITER_URL, "_default" );
        break;
    case MI_CALC:
        ShutdownIcon::OpenURL( CALC_URL, "_default" );
        break;
    case MI_IMPRESS:
        ShutdownIcon::OpenURL( IMPRESS_URL, "_default" );
        break;
    case MI_DRAW:
        ShutdownIcon::OpenURL( DRAW_URL, "_default" );
        break;
    case MI_BASE:
        ShutdownIcon::OpenURL( BASE_URL, "_default" );
        break;
    case MI_MATH:
        ShutdownIcon::OpenURL( MATH_URL, "_default" );
        break;
    case MI_TEMPLATE:
        ShutdownIcon::FromTemplate();
        break;
    case MI_STARTMODULE:
        ShutdownIcon::OpenURL( STARTMODULE_URL, "_default" );
        break;
    default:
        break;
    }
}

-(void)dockIconClicked: (NSObject*)pSender
{
    (void)pSender;

    SolarMutexGuard aGuard;

    // start module
    ShutdownIcon::OpenURL( STARTMODULE_URL, "_default" );
}

@end

bool ShutdownIcon::IsQuickstarterInstalled()
{
    return true;
}

static NSArray<NSMenuItem*>* pPreferredMenus = nil;
static NSMenuItem* pDefMenu = nil, *pDockSubMenu = nil;
static QSMenuExecute* pExecute = nil;

static std::set< OUString > aShortcuts;

static NSString* getAutoreleasedString( const OUString& rStr )
{
    return [[[NSString alloc] initWithCharacters: reinterpret_cast<unichar const *>(rStr.getStr()) length: rStr.getLength()] autorelease];
}

namespace {

struct RecentMenuEntry
{
    OUString aURL;
    OUString aFilter;
    OUString aTitle;
    OUString aPassword;
};

}

@interface QSCommandMenuItem : NSMenuItem
{
    OUString m_aCommand;
}
-(void)menuItemTriggered: (id)aSender;
-(void)setCommand: (OUString)aCommand;
@end

@implementation QSCommandMenuItem

-(void)menuItemTriggered: (id)aSender
{
    if ( m_aCommand.isEmpty() )
        return;

    SolarMutexGuard aGuard;

    if ( m_aCommand == "vnd.org.libreoffice.recentdocs:ClearRecentFileList" )
    {
        // Clearing the recent file list requires an extra step
        SvtHistoryOptions::Clear( EHistoryType::PickList, false );
    }
    else if ( m_aCommand == ".uno:Open" )
    {
        ShutdownIcon::FileOpen();
        return;
    }
    else if ( m_aCommand == ".uno:ConfigureDialog" )
    {
        // Selecting some menu items will cause a crash if there are
        // no visibile windows
        ShutdownIcon::OpenURL( STARTMODULE_URL, "_default" );
    }
    else if ( m_aCommand == UNO_TOGGLECURRENTMODULE_COMMAND )
    {
        bool bIsExclusive = officecfg::Office::Common::History::ShowCurrentModuleOnly::get();
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::History::ShowCurrentModuleOnly::set(!bIsExclusive, batch);
        batch->commit();
        [self setState: bIsExclusive ? NSControlStateValueOff : NSControlStateValueOn];
        return;
    }

    // "private:" commands are used for menu items in the File > New menu
    if ( m_aCommand.startsWith( "private:" ) || m_aCommand == STARTMODULE_URL )
        ShutdownIcon::OpenURL( m_aCommand, "_default" );
    else
        ShutdownIcon::FromCommand( m_aCommand );
}

-(void)setCommand: (OUString)aCommand
{
    m_aCommand = aCommand;
}

@end

@interface RecentMenuDelegate : NSObject <NSMenuDelegate>
{
    std::vector< RecentMenuEntry >* m_pRecentFilesItems;
}
-(id)init;
-(void)dealloc;
-(void)menuNeedsUpdate:(NSMenu *)menu;
-(void)executeRecentEntry: (NSMenuItem*)item;
@end

@implementation RecentMenuDelegate
-(id)init
{
    if( (self = [super init]) )
    {
        m_pRecentFilesItems = new std::vector< RecentMenuEntry >();
    }
    return self;
}

-(void)dealloc
{
    delete m_pRecentFilesItems;
    [super dealloc];
}

-(void)menuNeedsUpdate:(NSMenu *)menu
{
    SolarMutexGuard aGuard;

    // clear menu
    int nItems = [menu numberOfItems];
    while( nItems -- )
        [menu removeItemAtIndex: 0];

    // update recent item list
    std::vector< SvtHistoryOptions::HistoryItem > aHistoryList( SvtHistoryOptions::GetList( EHistoryType::PickList ) );

    int nPickListMenuItems = ( aHistoryList.size() > 99 ) ? 99 : aHistoryList.size();

    m_pRecentFilesItems->clear();
    if( nPickListMenuItems > 0 )
    {
        for ( int i = 0; i < nPickListMenuItems; i++ )
        {
            const SvtHistoryOptions::HistoryItem & rPickListEntry = aHistoryList[i];
            RecentMenuEntry aRecentFile;
            aRecentFile.aURL = rPickListEntry.sURL;
            aRecentFile.aFilter = rPickListEntry.sFilter;
            aRecentFile.aTitle = rPickListEntry.sTitle;
            m_pRecentFilesItems->push_back( aRecentFile );
        }
    }

    // insert new recent items
    for ( std::vector<RecentMenuEntry>::size_type i = 0; i < m_pRecentFilesItems->size(); i++ )
    {
        OUStringBuffer aMenuShortCut;
        if ( i <= 9 )
        {
            if ( i == 9 )
                aMenuShortCut.append( "1~0. " );
            else
            {
                aMenuShortCut.append( "~N. " );
                aMenuShortCut[ 1 ] = sal_Unicode( i + '1' );
            }
        }
        else
        {
            aMenuShortCut.append( OUString::number(sal_Int32( i + 1 ) ) + ". " );
        }

        OUString   aMenuTitle;
        INetURLObject   aURL( (*m_pRecentFilesItems)[i].aURL );
        NSImage *pImage = nil;

        if ( aURL.GetProtocol() == INetProtocol::File )
        {
            // Do handle file URL differently: don't show the protocol,
            // just the file name
            aMenuTitle = aURL.GetLastName(INetURLObject::DecodeMechanism::WithCharset);

            if ( [NSApp respondsToSelector: @selector(createNSImage:)] )
            {
                BitmapEx aThumbnail(SvFileInformationManager::GetFileImageId(aURL));
                Size aBmpSize = aThumbnail.GetSizePixel();
                if ( aBmpSize.Width() > 0 && aBmpSize.Height() > 0 )
                {
                    Image aImage( aThumbnail );
                    NSValue *pImageValue = [NSValue valueWithPointer: &aImage];
                    pImage = [NSApp performSelector: @selector(createNSImage:) withObject: pImageValue];
                }
            }
        }
        else
        {
            // In all other URLs show the protocol name before the file name
            aMenuTitle   = INetURLObject::GetSchemeName(aURL.GetProtocol()) + ": " + aURL.getName();
        }

        aMenuShortCut.append( aMenuTitle );
        aMenuTitle = MnemonicGenerator::EraseAllMnemonicChars( aMenuShortCut.makeStringAndClear() );
        if ( aMenuTitle.isEmpty() )
            continue;

        if ( aMenuTitle.endsWith( "...", &aMenuTitle ) )
            aMenuTitle += u"\u2026";

        NSMenuItem* pNewItem = [[NSMenuItem alloc] initWithTitle: getAutoreleasedString( aMenuTitle )
                                                   action: @selector(executeRecentEntry:)
                                                   keyEquivalent: @""];
        [pNewItem setTag: i];
        [pNewItem setTarget: self];
        [pNewItem setEnabled: YES];
        if ( pImage )
        {
            [pNewItem setImage: pImage];
            [pImage release];
        }
        [menu addItem: pNewItem];
        [pNewItem autorelease];
    }

    if ( [menu numberOfItems] )
    {
        TranslateId aId( "STR_CLEAR_RECENT_FILES", "Clear List" );
        OUString aClearList = Translate::get( aId, Translate::Create("fwk") );
        if ( !aClearList.isEmpty() )
        {
            [menu addItem: [NSMenuItem separatorItem]];

            QSCommandMenuItem* pNewItem = [[QSCommandMenuItem alloc] initWithTitle: getAutoreleasedString( aClearList ) action: @selector(menuItemTriggered:) keyEquivalent: @""];
            [pNewItem setCommand: "vnd.org.libreoffice.recentdocs:ClearRecentFileList"];
            [pNewItem setTarget: pNewItem];
            [pNewItem setEnabled: YES];
            [menu addItem: pNewItem];
            [pNewItem autorelease];

            aId = TranslateId( "STR_TOGGLECURRENTMODULE", "Current Module Only" );
            OUString aToggleCurrentMode = Translate::get( aId, Translate::Create("fwk") );
            if ( !aToggleCurrentMode.isEmpty() )
            {
                pNewItem = [[QSCommandMenuItem alloc] initWithTitle: getAutoreleasedString( aToggleCurrentMode ) action: @selector(menuItemTriggered:) keyEquivalent: @""];
                [pNewItem setCommand: UNO_TOGGLECURRENTMODULE_COMMAND];
                [pNewItem setTarget: pNewItem];
                [pNewItem setState: officecfg::Office::Common::History::ShowCurrentModuleOnly::get() ? NSControlStateValueOn : NSControlStateValueOff];
                [pNewItem setEnabled: YES];
                [menu addItem: pNewItem];
                [pNewItem autorelease];
            }
        }
    }
    else
    {
        TranslateId aId( "STR_NODOCUMENT", "No Documents" );
        OUString aNoDocuments = Translate::get( aId, Translate::Create("fwk") );
        if ( !aNoDocuments.isEmpty() )
        {
            NSMenuItem* pNewItem = [[NSMenuItem alloc] initWithTitle: getAutoreleasedString( aNoDocuments ) action: nil keyEquivalent: @""];
            [pNewItem setEnabled: YES];
            [menu addItem: pNewItem];
            [pNewItem autorelease];
        }
    }
}

-(void)executeRecentEntry: (NSMenuItem*)item
{
    sal_Int32 nIndex = [item tag];
    if( ( nIndex >= 0 ) && ( nIndex < static_cast<sal_Int32>( m_pRecentFilesItems->size() ) ) )
    {
        const RecentMenuEntry& rRecentFile = (*m_pRecentFilesItems)[ nIndex ];
        int NUM_OF_PICKLIST_ARGS = 3;
        css::uno::Sequence< css::beans::PropertyValue > aArgsList( NUM_OF_PICKLIST_ARGS );
        css::beans::PropertyValue* pArgsList = aArgsList.getArray();

        pArgsList[0].Name = "Referer";
        pArgsList[0].Value <<= OUString( "private:user" );

        // documents in the picklist will never be opened as templates
        pArgsList[1].Name = "AsTemplate";
        pArgsList[1].Value <<= false;

        OUString  aFilter( rRecentFile.aFilter );
        sal_Int32 nPos = aFilter.indexOf( '|' );
        if ( nPos >= 0 )
        {
            OUString aFilterOptions;

            if ( nPos < ( aFilter.getLength() - 1 ) )
                aFilterOptions = aFilter.copy( nPos+1 );

            pArgsList[2].Name = "FilterOptions";
            pArgsList[2].Value <<= aFilterOptions;

            aFilter = aFilter.copy( 0, nPos-1 );
            aArgsList.realloc( ++NUM_OF_PICKLIST_ARGS );
            pArgsList = aArgsList.getArray();
        }

        pArgsList[NUM_OF_PICKLIST_ARGS-1].Name = "FilterName";
        pArgsList[NUM_OF_PICKLIST_ARGS-1].Value <<= aFilter;

        ShutdownIcon::OpenURL( rRecentFile.aURL, "_default", aArgsList );
    }
}
@end

static RecentMenuDelegate* pRecentDelegate = nil;

static OUString getShortCut( const OUString& i_rTitle )
{
    // create shortcut
    OUString aKeyEquiv;
    for( sal_Int32 nIndex = 0; nIndex < i_rTitle.getLength(); nIndex++ )
    {
        OUString aShortcut( i_rTitle.copy( nIndex, 1 ).toAsciiLowerCase() );
        if( aShortcuts.find( aShortcut ) == aShortcuts.end() )
        {
            aShortcuts.insert( aShortcut );
            aKeyEquiv = aShortcut;
            break;
        }
    }

    return aKeyEquiv;   
}

static void appendMenuItem( NSMenu* i_pMenu, NSMenu* i_pDockMenu, const OUString& i_rTitle, int i_nTag, const OUString& i_rKeyEquiv )
{
    if( ! i_rTitle.getLength() )
        return;

    NSMenuItem* pItem = [[NSMenuItem alloc] initWithTitle: getAutoreleasedString( i_rTitle )
                                            action: @selector(executeMenuItem:)
                                            keyEquivalent: (i_rKeyEquiv.getLength() ? getAutoreleasedString( i_rKeyEquiv ) : @"")
                        ];
    [pItem setTag: i_nTag];
    [pItem setTarget: pExecute];
    [pItem setEnabled: YES];
    [i_pMenu addItem: pItem];
    [pItem autorelease];

    if( i_pDockMenu )
    {
        // create a similar entry in the dock menu
        pItem = [[NSMenuItem alloc] initWithTitle: getAutoreleasedString( i_rTitle )
                                    action: @selector(executeMenuItem:)
                                    keyEquivalent: @""
                            ];
        [pItem setTag: i_nTag];
        [pItem setTarget: pExecute];
        [pItem setEnabled: YES];
        [i_pDockMenu addItem: pItem];
        [pItem autorelease];
    }
}

static void appendRecentMenu( NSMenu* i_pMenu, const OUString& i_rTitle )
{
    if( ! pRecentDelegate )
        pRecentDelegate = [[RecentMenuDelegate alloc] init];

    NSMenuItem* pItem = [i_pMenu addItemWithTitle: getAutoreleasedString( i_rTitle )
                                                   action: @selector(executeMenuItem:)
                                                   keyEquivalent: @""
                        ];
    [pItem setEnabled: YES];
    NSMenu* pRecentMenu = [[NSMenu alloc] initWithTitle: getAutoreleasedString( i_rTitle ) ];

    [pRecentMenu setDelegate: pRecentDelegate];

    [pRecentMenu setAutoenablesItems: NO];
    [pItem setSubmenu: pRecentMenu];
}

void setKeyEquivalent( const vcl::KeyCode &rKeyCode, NSMenuItem *pNSMenuItem )
{
    if ( !pNSMenuItem )
        return;

    sal_uInt16 nKeyCode = rKeyCode.GetCode();
    if ( !nKeyCode )
        return;

    sal_Unicode nCommandKey = 0;
    if ((nKeyCode>=KEY_A) && (nKeyCode<=KEY_Z))           // letter A..Z
        nCommandKey = nKeyCode - KEY_A + 'a';
    else if ((nKeyCode>=KEY_0) && (nKeyCode<=KEY_9))      // numbers 0..9
        nCommandKey = nKeyCode - KEY_0 + '0';
    else if ((nKeyCode>=KEY_F1) && (nKeyCode<=KEY_F26))   // function keys F1..F26
        nCommandKey = nKeyCode - KEY_F1 + NSF1FunctionKey;

    if ( !nCommandKey )
        return;

    sal_uInt16 nModifier = rKeyCode.GetModifier();
    int nItemModifier = 0;

    if ( nModifier & KEY_SHIFT )
    {
        nItemModifier |= NSEventModifierFlagShift;   // actually useful only for function keys
        if ( nKeyCode >= KEY_A && nKeyCode <= KEY_Z )
            nCommandKey = nKeyCode - KEY_A + 'A';
    }

    if ( nModifier & KEY_MOD1 )
        nItemModifier |= NSEventModifierFlagCommand;

    if ( nModifier & KEY_MOD2 )
        nItemModifier |= NSEventModifierFlagOption;

    if ( nModifier & KEY_MOD3 )
        nItemModifier |= NSEventModifierFlagControl;

    OUString aCommandKey( &nCommandKey, 1 );
    NSString *pCommandKey = [NSString stringWithCharacters: reinterpret_cast< unichar const* >(aCommandKey.getStr()) length: aCommandKey.getLength()];
    [pNSMenuItem setKeyEquivalent: pCommandKey];
    [pNSMenuItem setKeyEquivalentModifierMask: nItemModifier];
}

static NSMenu *getNSMenuForVCLMenu( Menu *pMenu )
{
    NSMenu *pRet = nil;

    if ( !pMenu )
        return pRet;

    pMenu->Activate();

    sal_uInt16 nItemCount = pMenu->GetItemCount();
    if ( nItemCount )
    {
        pRet = [[[NSMenu alloc] initWithTitle: @""] autorelease];
        [pRet setAutoenablesItems: NO];
        for ( sal_uInt16 i = 0; i < nItemCount; i++ )
        {
            sal_uInt16 nId = pMenu->GetItemId( i );
            if ( nId && pMenu->IsItemEnabled( nId ) )
            {
                OUString aText = MnemonicGenerator::EraseAllMnemonicChars( pMenu->GetItemText( nId ) );
                if ( aText.isEmpty() )
                    continue;

                if ( aText.endsWith( "...", &aText ) )
                    aText += u"\u2026";

                // Use a custom menu in place of the Start Center's recent
                // documents menu so that the list can be dynamically updated
                OUString aCommand = pMenu->GetItemCommand( nId );
                if ( aCommand == ".uno:RecentFileList" )
                {
                    appendRecentMenu( pRet, aText );
                    continue;
                }

                NSString *pText = getAutoreleasedString( aText );
                // TODO: use the QSMenuExecute class to connect the command
                // string to one of the existing handler functions
                QSCommandMenuItem *pNSMenuItem = [[QSCommandMenuItem alloc] initWithTitle: pText action: @selector(menuItemTriggered:) keyEquivalent: @""];
                NSMenu *pNSSubmenu = getNSMenuForVCLMenu( pMenu->GetPopupMenu( nId ) );
                if ( pNSSubmenu && [pNSSubmenu numberOfItems] )
                {
                    [pNSSubmenu setTitle: pText];
                    [pNSMenuItem setSubmenu: pNSSubmenu];

                    if ( aCommand == ".uno:AddDirect" )
                    {
                        SvtModuleOptions aModuleOptions;
                        if ( aModuleOptions.IsModuleInstalled( SvtModuleOptions::EModule::STARTMODULE ) )
                        {
                            QSCommandMenuItem *pStartModuleMenuItem = [[QSCommandMenuItem alloc] initWithTitle: getAutoreleasedString( SfxResId( STR_QUICKSTART_STARTCENTER ) ) action: @selector(menuItemTriggered:) keyEquivalent: @"n"];
                            [pStartModuleMenuItem setTarget: pStartModuleMenuItem];
                            [pStartModuleMenuItem setCommand: STARTMODULE_URL];
                            [pNSSubmenu insertItem: pStartModuleMenuItem atIndex: 0];
                            [pStartModuleMenuItem autorelease];
                        }
                    }
                }
                else if ( !aCommand.isEmpty() )
                {
                    [pNSMenuItem setTarget: pNSMenuItem];
                    [pNSMenuItem setCommand: aCommand];

                    // Use the default menu's special "open new file" shortcuts
                    if ( aCommand == WRITER_URL )
                        [pNSMenuItem setKeyEquivalent: @"t"];
                    else if ( aCommand == CALC_URL )
                        [pNSMenuItem setKeyEquivalent: @"s"];
                    else if ( aCommand == IMPRESS_WIZARD_URL )
                        [pNSMenuItem setKeyEquivalent: @"p"];
                    else if ( aCommand == DRAW_URL )
                        [pNSMenuItem setKeyEquivalent: @"d"];
                    else if ( aCommand == MATH_URL )
                        [pNSMenuItem setKeyEquivalent: @"f"];
                    else if ( aCommand == BASE_URL )
                        [pNSMenuItem setKeyEquivalent: @"a"];
                    else
                        setKeyEquivalent( pMenu->GetAccelKey( nId ), pNSMenuItem );
                }

                [pRet addItem: pNSMenuItem];
                [pNSMenuItem autorelease];
            }
            else if ( pMenu->GetItemType( i ) == MenuItemType::SEPARATOR )
            {
                [pRet addItem: [NSMenuItem separatorItem]];
            }
        }
    }

    pMenu->Deactivate();

    return pRet;
}

static void clearDefaultMenuBar()
{
    if( ![NSApp respondsToSelector: @selector(removeFallbackMenuItem:)] )
        return;

    // Remove previous default menu
    if ( pDefMenu )
        [NSApp performSelector:@selector(removeFallbackMenuItem:) withObject: pDefMenu];

    // Remove previous preferred menu
    if ( pPreferredMenus && [pPreferredMenus count] )
    {
        for ( NSMenuItem *pNSMenuItem in pPreferredMenus )
            [NSApp performSelector:@selector(removeFallbackMenuItem:) withObject: pNSMenuItem];
    }
}

static void resetMenuBar()
{
    if( ![NSApp respondsToSelector: @selector(addFallbackMenuItem:)] )
        return;

    clearDefaultMenuBar();

    if ( pPreferredMenus && [pPreferredMenus count] )
    {
        for ( NSMenuItem *pNSMenuItem in pPreferredMenus )
            [NSApp performSelector:@selector(addFallbackMenuItem:) withObject: pNSMenuItem];
    }
    else if ( pDefMenu )
    {
        [NSApp performSelector:@selector(addFallbackMenuItem:) withObject: pDefMenu];
    }
}

void ShutdownIcon::SetDefaultMenuBar( MenuBar *pMenuBar )
{
    if ( !pMenuBar )
        return;

    SolarMutexGuard aGuard;

    clearDefaultMenuBar();
    if ( pPreferredMenus )
    {
        [pPreferredMenus release];
        pPreferredMenus = nil;
    }

    NSMenu *pNSMenu = getNSMenuForVCLMenu( pMenuBar );
    if ( pNSMenu && [pNSMenu numberOfItems] )
    {
        pPreferredMenus = [NSMutableArray arrayWithArray: [pNSMenu itemArray]];
        [pNSMenu removeAllItems];
        [pPreferredMenus retain];
    }

    resetMenuBar();
}


extern "C"
{

void aqua_init_systray()
{
    SolarMutexGuard aGuard;

    ShutdownIcon *pShutdownIcon = ShutdownIcon::getInstance();
    if( ! pShutdownIcon )
        return;

    // disable shutdown
    pShutdownIcon->SetVeto( true );
    ShutdownIcon::addTerminateListener();

    if( ! pDefMenu )
    {
        if( [NSApp respondsToSelector: @selector(addFallbackMenuItem:)] )
        {
            aShortcuts.clear();

            pExecute = [[QSMenuExecute alloc] init];
            pDefMenu = [[NSMenuItem alloc] initWithTitle: getAutoreleasedString( SfxResId(STR_QUICKSTART_FILE) ) action: nullptr keyEquivalent: @""];
            pDockSubMenu = [[NSMenuItem alloc] initWithTitle: getAutoreleasedString( SfxResId(STR_QUICKSTART_FILE) ) action: nullptr keyEquivalent: @""];
            NSMenu* pMenu = [[NSMenu alloc] initWithTitle: getAutoreleasedString( SfxResId(STR_QUICKSTART_FILE) )];
            [pMenu setAutoenablesItems: NO];
            NSMenu* pDockMenu = [[NSMenu alloc] initWithTitle: getAutoreleasedString( SfxResId(STR_QUICKSTART_FILE) )];
            [pDockMenu setAutoenablesItems: NO];

            // collect the URLs of the entries in the File/New menu
            SvtModuleOptions    aModuleOptions;
            std::set< OUString > aFileNewAppsAvailable;
            std::vector < SvtDynMenuEntry > const aNewMenu = SvtDynamicMenuOptions::GetMenu( EDynamicMenuType::NewMenu );

            for ( SvtDynMenuEntry const & newMenuProp : aNewMenu )
            {
                if ( !newMenuProp.sURL.isEmpty() )
                    aFileNewAppsAvailable.insert( newMenuProp.sURL );
            }

            // describe the menu entries for launching the applications
            struct MenuEntryDescriptor
            {
                SvtModuleOptions::EModule   eModuleIdentifier;
                int                         nMenuTag;
                OUString                    sURLDescription;
            } static constexpr aMenuItems[] =
            {
                { SvtModuleOptions::EModule::WRITER,    MI_WRITER,  WRITER_URL },
                { SvtModuleOptions::EModule::CALC,      MI_CALC,    CALC_URL },
                { SvtModuleOptions::EModule::IMPRESS,   MI_IMPRESS, IMPRESS_WIZARD_URL },
                { SvtModuleOptions::EModule::DRAW,      MI_DRAW,    DRAW_URL },
                { SvtModuleOptions::EModule::DATABASE,  MI_BASE,    BASE_URL },
                { SvtModuleOptions::EModule::MATH,      MI_MATH,    MATH_URL }
            };

            // insert entry for startcenter
            if( aModuleOptions.IsModuleInstalled( SvtModuleOptions::EModule::STARTMODULE ) )
            {
                appendMenuItem( pMenu, nil, SfxResId(STR_QUICKSTART_STARTCENTER), MI_STARTMODULE, "n" );
                if( [NSApp respondsToSelector: @selector(setDockIconClickHandler:)] )
                    [NSApp performSelector:@selector(setDockIconClickHandler:) withObject: pExecute];
                else
                    OSL_FAIL( "setDockIconClickHandler selector failed on NSApp" );

            }

            // insert the menu entries for launching the applications
            for ( size_t i = 0; i < SAL_N_ELEMENTS( aMenuItems ); ++i )
            {
                if ( !aModuleOptions.IsModuleInstalled( aMenuItems[i].eModuleIdentifier ) )
                    // the complete application is not even installed
                    continue;

                const OUString& sURL( aMenuItems[i].sURLDescription );

                if ( aFileNewAppsAvailable.find( sURL ) == aFileNewAppsAvailable.end() )
                    // the application is installed, but the entry has been configured to *not* appear in the File/New
                    // menu => also let not appear it in the quickstarter
                    continue;

                OUString aKeyEquiv( getShortCut( ShutdownIcon::GetUrlDescription( sURL ) ) );

                appendMenuItem( pMenu, pDockMenu, ShutdownIcon::GetUrlDescription( sURL ), aMenuItems[i].nMenuTag, aKeyEquiv );
            }

            // insert the remaining menu entries

            // add recent menu
            appendRecentMenu( pMenu, SfxResId(STR_QUICKSTART_RECENTDOC) );

            OUString aTitle( SfxResId(STR_QUICKSTART_FROMTEMPLATE) );
            OUString aKeyEquiv( getShortCut( aTitle ) );
            appendMenuItem( pMenu, pDockMenu, aTitle, MI_TEMPLATE, aKeyEquiv );
            aTitle = SfxResId(STR_QUICKSTART_FILEOPEN);
            aKeyEquiv = getShortCut( aTitle );
            appendMenuItem( pMenu, pDockMenu, aTitle, MI_OPEN, aKeyEquiv );

            [pDefMenu setSubmenu: pMenu];
            resetMenuBar();

            if( [NSApp respondsToSelector: @selector(addDockMenuItem:)] )
            {
                [pDockSubMenu setSubmenu: pDockMenu];
                // add the submenu
                [NSApp performSelector:@selector(addDockMenuItem:) withObject: pDockSubMenu];
            }
            else
                OSL_FAIL( "addDockMenuItem selector failed on NSApp" );

            [pMenu autorelease];
            [pDockMenu autorelease];
        }
        else
            OSL_FAIL( "addFallbackMenuItem selector failed on NSApp" );
    }
}

void SAL_DLLPUBLIC_EXPORT aqua_shutdown_systray()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
