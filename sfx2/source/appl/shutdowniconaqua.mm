/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: shutdowniconaqua.mm,v $
 * $Revision: 1.5 $
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
#include "precompiled_sfx2.hxx"

#include "svtools/moduleoptions.hxx"
#include "svtools/dynamicmenuoptions.hxx"
#include <comphelper/sequenceashashmap.hxx>
#include "vos/mutex.hxx"
#include "sfx2/app.hxx"
#include "app.hrc"
#define USE_APP_SHORTCUTS
#include "shutdownicon.hxx"

#include <set>

#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

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
        ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( WRITER_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
        break;
    case MI_CALC:
        ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( CALC_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
        break;
    case MI_IMPRESS:
        ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( IMPRESS_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
        break;
    case MI_DRAW:
        ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( DRAW_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
        break;
    case MI_BASE:
        ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( BASE_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
        break;
    case MI_MATH:
        ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( MATH_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
        break;
    case MI_TEMPLATE:
        ShutdownIcon::FromTemplate();
        break;
    case MI_STARTMODULE:
        ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( STARTMODULE_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
        break;
    default:
        break;
    }
}

-(void)dockIconClicked: (NSObject*)pSender
{
    // start start module
    ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( STARTMODULE_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
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
    return [[[NSString alloc] initWithCharacters: rStr.getStr() length: rStr.getLength()] autorelease];
}

static rtl::OUString getShortCut( const rtl::OUString i_rTitle )
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


extern "C"
{

void aqua_init_systray()
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );

    ShutdownIcon *pShutdownIcon = ShutdownIcon::getInstance();
    if( ! pShutdownIcon )
        return;

	// disable shutdown
	pShutdownIcon->SetVeto( true );
	pShutdownIcon->addTerminateListener();
    
    if( ! pDefMenu )
    {
        if( [NSApp respondsToSelector: @selector(addFallbackMenuItem:)] )
        {
            aShortcuts.clear();
            
            pExecute = [[QSMenuExecute alloc] init];
            pDefMenu = [[NSMenuItem alloc] initWithTitle: getAutoreleasedString( pShutdownIcon->GetResString( STR_QUICKSTART_FILE ) ) action: NULL keyEquivalent: @""];
            pDockSubMenu = [[NSMenuItem alloc] initWithTitle: getAutoreleasedString( pShutdownIcon->GetResString( STR_QUICKSTART_FILE ) ) action: NULL keyEquivalent: @""];
            NSMenu* pMenu = [[NSMenu alloc] initWithTitle: getAutoreleasedString( pShutdownIcon->GetResString( STR_QUICKSTART_FILE ) )];
            [pMenu setAutoenablesItems: NO];
            NSMenu* pDockMenu = [[NSMenu alloc] initWithTitle: getAutoreleasedString( pShutdownIcon->GetResString( STR_QUICKSTART_FILE ) )];
            [pDockMenu setAutoenablesItems: NO];
            
            // collect the URLs of the entries in the File/New menu
            SvtModuleOptions	aModuleOptions;
            std::set< rtl::OUString > aFileNewAppsAvailable;
            SvtDynamicMenuOptions aOpt;
            Sequence < Sequence < PropertyValue > > aNewMenu = aOpt.GetMenu( E_NEWMENU );
            const rtl::OUString sURLKey( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
        
            const Sequence< PropertyValue >* pNewMenu = aNewMenu.getConstArray();
            const Sequence< PropertyValue >* pNewMenuEnd = aNewMenu.getConstArray() + aNewMenu.getLength();
            for ( ; pNewMenu != pNewMenuEnd; ++pNewMenu )
            {
                comphelper::SequenceAsHashMap aEntryItems( *pNewMenu );
                rtl::OUString sURL( aEntryItems.getUnpackedValueOrDefault( sURLKey, rtl::OUString() ) );
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
                { SvtModuleOptions::E_SWRITER,    MI_WRITER,  WRITER_URL },
                { SvtModuleOptions::E_SCALC,      MI_CALC,    CALC_URL },
                { SvtModuleOptions::E_SIMPRESS,   MI_IMPRESS, IMPRESS_WIZARD_URL },
                { SvtModuleOptions::E_SDRAW,      MI_DRAW,    DRAW_URL },
                { SvtModuleOptions::E_SDATABASE,  MI_BASE,    BASE_URL },
                { SvtModuleOptions::E_SMATH,      MI_MATH,    MATH_URL }
            };

            // insert entry for startcenter
            if( aModuleOptions.IsModuleInstalled( SvtModuleOptions::E_SSTARTMODULE ) )
            {
                appendMenuItem( pMenu, nil, pShutdownIcon->GetResString( STR_QUICKSTART_STARTCENTER ), MI_STARTMODULE, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "n" ) ) );
                if( [NSApp respondsToSelector: @selector(setDockIconClickHandler:)] )
                    [NSApp performSelector:@selector(setDockIconClickHandler:) withObject: pExecute];
                else
                    DBG_ERROR( "setDockIconClickHandler selector failed on NSApp\n" );

            }
            
            // insert the menu entries for launching the applications
            for ( size_t i = 0; i < sizeof( aMenuItems ) / sizeof( aMenuItems[0] ); ++i )
            {
                if ( !aModuleOptions.IsModuleInstalled( aMenuItems[i].eModuleIdentifier ) )
                    // the complete application is not even installed
                    continue;
        
                rtl::OUString sURL( ::rtl::OUString::createFromAscii( aMenuItems[i].pAsciiURLDescription ) );
        
                if ( aFileNewAppsAvailable.find( sURL ) == aFileNewAppsAvailable.end() )
                    // the application is installed, but the entry has been configured to *not* appear in the File/New
                    // menu => also let not appear it in the quickstarter
                    continue;
                
                rtl::OUString aKeyEquiv( getShortCut( pShutdownIcon->GetUrlDescription( sURL ) ) );
        
                appendMenuItem( pMenu, pDockMenu, pShutdownIcon->GetUrlDescription( sURL ), aMenuItems[i].nMenuTag, aKeyEquiv );
            }

            // insert the remaining menu entries
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
                // insert a separator to the dock menu
                [NSApp performSelector:@selector(addDockMenuItem:) withObject: [NSMenuItem separatorItem]];
                // and now add the submenu
                [NSApp performSelector:@selector(addDockMenuItem:) withObject: pDockSubMenu];
            }
            else
                DBG_ERROR( "addDockMenuItem selector failed on NSApp\n" );
        }
        else
            DBG_ERROR( "addFallbackMenuItem selector failed on NSApp\n" );
    }
}

void SAL_DLLPUBLIC_EXPORT aqua_shutdown_systray()
{
}

}
