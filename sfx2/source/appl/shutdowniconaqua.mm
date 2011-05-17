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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "unotools/moduleoptions.hxx"
#include "unotools/dynamicmenuoptions.hxx"
#include "unotools/historyoptions.hxx"
#include "tools/urlobj.hxx"
#include "osl/file.h"
#include "comphelper/sequenceashashmap.hxx"
#include "osl/mutex.hxx"
#include "sfx2/app.hxx"
#include <sal/macros.h>
#include "app.hrc"
#define USE_APP_SHORTCUTS
#include "shutdownicon.hxx"

#include "com/sun/star/util/XStringWidth.hpp"

#include "cppuhelper/implbase1.hxx"

#include <set>
#include <vector>

#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;

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

struct RecentMenuEntry
{
    rtl::OUString aURL;
    rtl::OUString aFilter;
    rtl::OUString aTitle;
    rtl::OUString aPassword;
};

class RecentFilesStringLength : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XStringWidth >
{
	public:
		RecentFilesStringLength() {}
		virtual ~RecentFilesStringLength() {}

		// XStringWidth
		sal_Int32 SAL_CALL queryStringWidth( const ::rtl::OUString& aString )
			throw (::com::sun::star::uno::RuntimeException)
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
    Sequence< Sequence< PropertyValue > > aHistoryList( SvtHistoryOptions().GetList( ePICKLIST ) );

    int nPickListMenuItems = ( aHistoryList.getLength() > 99 ) ? 99 : aHistoryList.getLength();
        
    m_pRecentFilesItems->clear();
    if( ( nPickListMenuItems > 0 ) )
    {
        for ( int i = 0; i < nPickListMenuItems; i++ )
        {
            Sequence< PropertyValue >& rPickListEntry = aHistoryList[i];
            RecentMenuEntry aRecentFile;
            
            for ( int j = 0; j < rPickListEntry.getLength(); j++ )
            {
                Any a = rPickListEntry[j].Value;
                
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
    for ( sal_uInt32 i = 0; i < m_pRecentFilesItems->size(); i++ )
    {
        rtl::OUString	aMenuTitle;
        INetURLObject	aURL( (*m_pRecentFilesItems)[i].aURL );
        
        if ( aURL.GetProtocol() == INET_PROT_FILE )
        {
            // Do handle file URL differently => convert it to a system
            // path and abbreviate it with a special function:
            String aFileSystemPath( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );
            
            ::rtl::OUString	aSystemPath( aFileSystemPath );
            ::rtl::OUString	aCompactedSystemPath;
            
            oslFileError nError = osl_abbreviateSystemPath( aSystemPath.pData, &aCompactedSystemPath.pData, 46, NULL );
            if ( !nError )
                aMenuTitle = String( aCompactedSystemPath );
            else
                aMenuTitle = aSystemPath;
        }
        else
        {
            // Use INetURLObject to abbreviate all other URLs
            Reference< XStringWidth > xStringLength( new RecentFilesStringLength() );
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
        Sequence< PropertyValue > aArgsList( NUM_OF_PICKLIST_ARGS );
        
        aArgsList[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Referer" ));
        aArgsList[0].Value = makeAny( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:user" ) ) );

        // documents in the picklist will never be opened as templates
        aArgsList[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AsTemplate" ));
        aArgsList[1].Value = makeAny( (sal_Bool) sal_False );

        ::rtl::OUString  aFilter( rRecentFile.aFilter );
        sal_Int32 nPos = aFilter.indexOf( '|' );
        if ( nPos >= 0 )
        {
	        rtl::OUString aFilterOptions;

	        if ( nPos < ( aFilter.getLength() - 1 ) )
		        aFilterOptions = aFilter.copy( nPos+1 );

	        aArgsList[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterOptions" ));
	        aArgsList[2].Value = makeAny( aFilterOptions );

	        aFilter = aFilter.copy( 0, nPos-1 );
	        aArgsList.realloc( ++NUM_OF_PICKLIST_ARGS );
        }

        aArgsList[NUM_OF_PICKLIST_ARGS-1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
        aArgsList[NUM_OF_PICKLIST_ARGS-1].Value = makeAny( aFilter );

        ShutdownIcon::OpenURL( rRecentFile.aURL, OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ), aArgsList );
    }
}
@end

static RecentMenuDelegate* pRecentDelegate = nil;

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

static void appendRecentMenu( NSMenu* i_pMenu, NSMenu* i_pDockMenu, const String& i_rTitle )
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

    if( i_pDockMenu )
    {
        // create a similar entry in the dock menu
        pItem = [i_pDockMenu addItemWithTitle: getAutoreleasedString( i_rTitle )
                             action: @selector(executeMenuItem:)
                             keyEquivalent: @""
                        ];
        [pItem setEnabled: YES];
        pRecentMenu = [[NSMenu alloc] initWithTitle: getAutoreleasedString( i_rTitle ) ];
        [pRecentMenu setDelegate: pRecentDelegate];
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
                
                rtl::OUString aKeyEquiv( getShortCut( pShutdownIcon->GetUrlDescription( sURL ) ) );
        
                appendMenuItem( pMenu, pDockMenu, pShutdownIcon->GetUrlDescription( sURL ), aMenuItems[i].nMenuTag, aKeyEquiv );
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
                // insert a separator to the dock menu
                [NSApp performSelector:@selector(addDockMenuItem:) withObject: [NSMenuItem separatorItem]];
                // and now add the submenu
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
