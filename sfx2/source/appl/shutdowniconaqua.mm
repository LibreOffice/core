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
#include <rtl/ustring.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.h>
#include <comphelper/sequenceashashmap.hxx>
#include <osl/mutex.hxx>
#include <sfx2/app.hxx>
#include <sal/macros.h>
#include "app.hrc"
#include "shutdownicon.hxx"

#include <com/sun/star/util/XStringWidth.hpp>

#include <cppuhelper/implbase.hxx>

#include <set>
#include <vector>

#include "premac.h"
#include <objc/objc-runtime.h>
#include <Cocoa/Cocoa.h>
#include "postmac.h"

#define MI_OPEN                    1
#define MI_WRITER                  2
#define MI_CALC                    3
#define MI_IMPRESS                 4
#define MI_DRAW                    5
#define MI_BASE                    6
#define MI_MATH                    7
#define MI_TEMPLATE                8
#define MI_STARTMODULE             9

@interface QSMenuExecute : NSObject
{
}
-(void)executeMenuItem: (NSMenuItem*)pItem;
-(void)dockIconClicked: (NSObject*)pSender;
@end

@implementation QSMenuExecute
-(void)executeMenuItem: (NSMenuItem*)pItem
{
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
    // start module
    ShutdownIcon::OpenURL( STARTMODULE_URL, "_default" );
}

@end

bool ShutdownIcon::IsQuickstarterInstalled()
{
    return true;
}

static NSMenuItem* pDefMenu = nil, *pDockSubMenu = nil;
static QSMenuExecute* pExecute = nil;

static std::set< OUString > aShortcuts;

static NSString* getAutoreleasedString( const rtl::OUString& rStr )
{
    return [[[NSString alloc] initWithCharacters: reinterpret_cast<unichar const *>(rStr.getStr()) length: rStr.getLength()] autorelease];
}

struct RecentMenuEntry
{
    rtl::OUString aURL;
    rtl::OUString aFilter;
    rtl::OUString aTitle;
    rtl::OUString aPassword;
};

class RecentFilesStringLength : public ::cppu::WeakImplHelper< css::util::XStringWidth >
{
    public:
        RecentFilesStringLength() {}
        virtual ~RecentFilesStringLength() {}

        // XStringWidth
        sal_Int32 SAL_CALL queryStringWidth( const ::rtl::OUString& aString )
            throw ( css::uno::RuntimeException ) override
        {
            return aString.getLength();
        }
};

@interface RecentMenuDelegate : NSObject
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
    // clear menu
    int nItems = [menu numberOfItems];
    while( nItems -- )
        [menu removeItemAtIndex: 0];

    // update recent item list
    css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aHistoryList( SvtHistoryOptions().GetList( ePICKLIST ) );

    int nPickListMenuItems = ( aHistoryList.getLength() > 99 ) ? 99 : aHistoryList.getLength();

    m_pRecentFilesItems->clear();
    if( ( nPickListMenuItems > 0 ) )
    {
        for ( int i = 0; i < nPickListMenuItems; i++ )
        {
            css::uno::Sequence< css::beans::PropertyValue >& rPickListEntry = aHistoryList[i];
            RecentMenuEntry aRecentFile;

            for ( int j = 0; j < rPickListEntry.getLength(); j++ )
            {
                css::uno::Any a = rPickListEntry[j].Value;

                if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_URL )
                    a >>= aRecentFile.aURL;
                else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_FILTER )
                    a >>= aRecentFile.aFilter;
                else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_TITLE )
                    a >>= aRecentFile.aTitle;
                else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_PASSWORD )
                    a >>= aRecentFile.aPassword;
            }

            m_pRecentFilesItems->push_back( aRecentFile );
        }
    }

    // insert new recent items
    for ( std::vector<RecentMenuEntry>::size_type i = 0; i < m_pRecentFilesItems->size(); i++ )
    {
        rtl::OUString   aMenuTitle;
        INetURLObject   aURL( (*m_pRecentFilesItems)[i].aURL );

        if ( aURL.GetProtocol() == INetProtocol::File )
        {
            // Do handle file URL differently => convert it to a system
            // path and abbreviate it with a special function:
            ::rtl::OUString aSystemPath( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );
            ::rtl::OUString aCompactedSystemPath;

            oslFileError nError = osl_abbreviateSystemPath( aSystemPath.pData, &aCompactedSystemPath.pData, 46, nullptr );
            if ( !nError )
                aMenuTitle = aCompactedSystemPath;
            else
                aMenuTitle = aSystemPath;
        }
        else
        {
            // Use INetURLObject to abbreviate all other URLs
            css::uno::Reference< css::util::XStringWidth > xStringLength( new RecentFilesStringLength() );
            aMenuTitle = aURL.getAbbreviated( xStringLength, 46, INetURLObject::DECODE_UNAMBIGUOUS );
        }

        NSMenuItem* pNewItem = [[NSMenuItem alloc] initWithTitle: getAutoreleasedString( aMenuTitle )
                                                   action: @selector(executeRecentEntry:)
                                                   keyEquivalent: @""];
        [pNewItem setTag: i];
        [pNewItem setTarget: self];
        [pNewItem setEnabled: YES];
        [menu addItem: pNewItem];
        [pNewItem autorelease];
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

        aArgsList[0].Name = "Referer";
        aArgsList[0].Value = css::uno::makeAny( OUString( "private:user" ) );

        // documents in the picklist will never be opened as templates
        aArgsList[1].Name = "AsTemplate";
        aArgsList[1].Value = css::uno::makeAny( false );

        ::rtl::OUString  aFilter( rRecentFile.aFilter );
        sal_Int32 nPos = aFilter.indexOf( '|' );
        if ( nPos >= 0 )
        {
            rtl::OUString aFilterOptions;

            if ( nPos < ( aFilter.getLength() - 1 ) )
                aFilterOptions = aFilter.copy( nPos+1 );

            aArgsList[2].Name = "FilterOptions";
            aArgsList[2].Value = css::uno::makeAny( aFilterOptions );

            aFilter = aFilter.copy( 0, nPos-1 );
            aArgsList.realloc( ++NUM_OF_PICKLIST_ARGS );
        }

        aArgsList[NUM_OF_PICKLIST_ARGS-1].Name = "FilterName";
        aArgsList[NUM_OF_PICKLIST_ARGS-1].Value = css::uno::makeAny( aFilter );

        ShutdownIcon::OpenURL( rRecentFile.aURL, "_default", aArgsList );
    }
}
@end

static RecentMenuDelegate* pRecentDelegate = nil;

static rtl::OUString getShortCut( const rtl::OUString& i_rTitle )
{
    // create shortcut
    rtl::OUString aKeyEquiv;
    for( sal_Int32 nIndex = 0; nIndex < i_rTitle.getLength(); nIndex++ )
    {
        rtl::OUString aShortcut( i_rTitle.copy( nIndex, 1 ).toAsciiLowerCase() );
        if( aShortcuts.find( aShortcut ) == aShortcuts.end() )
        {
            aShortcuts.insert( aShortcut );
            aKeyEquiv = aShortcut;
            break;
        }
    }

    return aKeyEquiv;   
}

static void appendMenuItem( NSMenu* i_pMenu, NSMenu* i_pDockMenu, const rtl::OUString& i_rTitle, int i_nTag, const rtl::OUString& i_rKeyEquiv )
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
    }
}

static void appendRecentMenu( NSMenu* i_pMenu, NSMenu* i_pDockMenu, const OUString& i_rTitle )
{
    if( ! pRecentDelegate )
        pRecentDelegate = [[RecentMenuDelegate alloc] init];

    NSMenuItem* pItem = [i_pMenu addItemWithTitle: getAutoreleasedString( i_rTitle )
                                                   action: @selector(executeMenuItem:)
                                                   keyEquivalent: @""
                        ];
    [pItem setEnabled: YES];
    NSMenu* pRecentMenu = [[NSMenu alloc] initWithTitle: getAutoreleasedString( i_rTitle ) ];

    // When compiling against 10.6 SDK, we get the warning:
    // class 'RecentMenuDelegate' does not implement the 'NSMenuDelegate' protocol

    // No idea if that is a bogus warning, or if the way this is
    // implemented just is so weird that the compiler gets
    // confused. Anyway, to avoid warnings, instead of this:
    // [pRecentMenu setDelegate: pRecentDelegate];
    // do this:
    objc_msgSend(pRecentMenu, @selector(setDelegate:), pRecentDelegate);

    [pRecentMenu setAutoenablesItems: NO];
    [pItem setSubmenu: pRecentMenu];

    if( i_pDockMenu )
    {
        // create a similar entry in the dock menu
        pItem = [i_pDockMenu addItemWithTitle: getAutoreleasedString( i_rTitle )
                             action: @selector(executeMenuItem:)
                             keyEquivalent: @""
                        ];
        [pItem setEnabled: YES];
        pRecentMenu = [[NSMenu alloc] initWithTitle: getAutoreleasedString( i_rTitle ) ];

        // See above
        // [pRecentMenu setDelegate: pRecentDelegate];
        objc_msgSend(pRecentMenu, @selector(setDelegate:), pRecentDelegate);

        [pRecentMenu setAutoenablesItems: NO];
        [pItem setSubmenu: pRecentMenu];
    }
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
            pDefMenu = [[NSMenuItem alloc] initWithTitle: getAutoreleasedString( pShutdownIcon->GetResString( STR_QUICKSTART_FILE ) ) action: nullptr keyEquivalent: @""];
            pDockSubMenu = [[NSMenuItem alloc] initWithTitle: getAutoreleasedString( pShutdownIcon->GetResString( STR_QUICKSTART_FILE ) ) action: nullptr keyEquivalent: @""];
            NSMenu* pMenu = [[NSMenu alloc] initWithTitle: getAutoreleasedString( pShutdownIcon->GetResString( STR_QUICKSTART_FILE ) )];
            [pMenu setAutoenablesItems: NO];
            NSMenu* pDockMenu = [[NSMenu alloc] initWithTitle: getAutoreleasedString( pShutdownIcon->GetResString( STR_QUICKSTART_FILE ) )];
            [pDockMenu setAutoenablesItems: NO];

            // collect the URLs of the entries in the File/New menu
            SvtModuleOptions    aModuleOptions;
            std::set< rtl::OUString > aFileNewAppsAvailable;
            SvtDynamicMenuOptions aOpt;
            css::uno::Sequence < css::uno::Sequence < css::beans::PropertyValue > > aNewMenu = aOpt.GetMenu( E_NEWMENU );

            const css::uno::Sequence< css::beans::PropertyValue >* pNewMenu = aNewMenu.getConstArray();
            const css::uno::Sequence< css::beans::PropertyValue >* pNewMenuEnd = aNewMenu.getConstArray() + aNewMenu.getLength();
            for ( ; pNewMenu != pNewMenuEnd; ++pNewMenu )
            {
                comphelper::SequenceAsHashMap aEntryItems( *pNewMenu );
                rtl::OUString sURL( aEntryItems.getUnpackedValueOrDefault( "URL", rtl::OUString() ) );
                if ( sURL.getLength() )
                    aFileNewAppsAvailable.insert( sURL );
            }

            // describe the menu entries for launching the applications
            struct MenuEntryDescriptor
            {
                SvtModuleOptions::EModule   eModuleIdentifier;
                int                         nMenuTag;
                const char*                 pAsciiURLDescription;
            }   aMenuItems[] =
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
                appendMenuItem( pMenu, nil, pShutdownIcon->GetResString( STR_QUICKSTART_STARTCENTER ), MI_STARTMODULE, "n" );
                if( [NSApp respondsToSelector: @selector(setDockIconClickHandler:)] )
                    [NSApp performSelector:@selector(setDockIconClickHandler:) withObject: pExecute];
                else
                    OSL_FAIL( "setDockIconClickHandler selector failed on NSApp\n" );

            }

            // insert the menu entries for launching the applications
            for ( size_t i = 0; i < SAL_N_ELEMENTS( aMenuItems ); ++i )
            {
                if ( !aModuleOptions.IsModuleInstalled( aMenuItems[i].eModuleIdentifier ) )
                    // the complete application is not even installed
                    continue;

                rtl::OUString sURL( ::rtl::OUString::createFromAscii( aMenuItems[i].pAsciiURLDescription ) );

                if ( aFileNewAppsAvailable.find( sURL ) == aFileNewAppsAvailable.end() )
                    // the application is installed, but the entry has been configured to *not* appear in the File/New
                    // menu => also let not appear it in the quickstarter
                    continue;

                rtl::OUString aKeyEquiv( getShortCut( ShutdownIcon::GetUrlDescription( sURL ) ) );

                appendMenuItem( pMenu, pDockMenu, ShutdownIcon::GetUrlDescription( sURL ), aMenuItems[i].nMenuTag, aKeyEquiv );
            }

            // insert the remaining menu entries

            // add recent menu
            appendRecentMenu( pMenu, pDockMenu, pShutdownIcon->GetResString( STR_QUICKSTART_RECENTDOC ) );

            rtl::OUString aTitle( pShutdownIcon->GetResString( STR_QUICKSTART_FROMTEMPLATE ) );
            rtl::OUString aKeyEquiv( getShortCut( aTitle ) );
            appendMenuItem( pMenu, pDockMenu, aTitle, MI_TEMPLATE, aKeyEquiv );
            aTitle = pShutdownIcon->GetResString( STR_QUICKSTART_FILEOPEN );
            aKeyEquiv = getShortCut( aTitle );
            appendMenuItem( pMenu, pDockMenu, aTitle, MI_OPEN, aKeyEquiv );

            [pDefMenu setSubmenu: pMenu];
            [NSApp performSelector:@selector(addFallbackMenuItem:) withObject: pDefMenu];

            if( [NSApp respondsToSelector: @selector(addDockMenuItem:)] )
            {
                [pDockSubMenu setSubmenu: pDockMenu];
                // add the submenu
                [NSApp performSelector:@selector(addDockMenuItem:) withObject: pDockSubMenu];
            }
            else
                OSL_FAIL( "addDockMenuItem selector failed on NSApp\n" );
        }
        else
            OSL_FAIL( "addFallbackMenuItem selector failed on NSApp\n" );
    }
}

void SAL_DLLPUBLIC_EXPORT aqua_shutdown_systray()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
