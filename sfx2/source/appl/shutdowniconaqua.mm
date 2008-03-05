
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shutdowniconaqua.mm,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:41:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#define MI_TEMPLATE                7


@interface QSMenuExecute : NSObject
{
}
-(void)executeMenuItem: (NSMenuItem*)pItem;
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
    case MI_TEMPLATE:
        ShutdownIcon::FromTemplate();
        break;
    default:
        break;
    }
}
@end

bool ShutdownIcon::IsQuickstarterInstalled()
{
    return true;
}

static NSMenuItem* pDefMenu = nil;
static QSMenuExecute* pExecute = nil;

static std::set< OUString > aShortcuts;

static NSString* getAutoreleasedString( const rtl::OUString& rStr )
{
    return [[[NSString alloc] initWithCharacters: rStr.getStr() length: rStr.getLength()] autorelease];
}

static void appendMenuItem( NSMenu* pMenu, const rtl::OUString& rTitle, int nTag )
{
    if( ! rTitle.getLength() )
        return;
    
    // create shortcut
    rtl::OUString aKeyEquiv;
    for( sal_Int32 nIndex = 0; nIndex < rTitle.getLength(); nIndex++ )
    {
        rtl::OUString aShortcut( rTitle.copy( nIndex, 1 ).toAsciiLowerCase() );
        if( aShortcuts.find( aShortcut ) == aShortcuts.end() )
        {
            aShortcuts.insert( aShortcut );
            aKeyEquiv = aShortcut;
            break;
        }
    }
    
    NSMenuItem* pItem = [[NSMenuItem alloc] initWithTitle: getAutoreleasedString( rTitle )
                                            action: @selector(executeMenuItem:)
                                            keyEquivalent: (aKeyEquiv.getLength() ? getAutoreleasedString( aKeyEquiv ) : @"")
                        ];
    [pItem setTag: nTag];
    [pItem setTarget: pExecute];
    [pItem setEnabled: YES];
    [pMenu addItem: pItem];
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
            NSMenu* pMenu = [[NSMenu alloc] initWithTitle: getAutoreleasedString( pShutdownIcon->GetResString( STR_QUICKSTART_FILE ) )];
            [pMenu setAutoenablesItems: NO];
            
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
                { SvtModuleOptions::E_SDATABASE,  MI_BASE,    BASE_URL }
            };

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
        
                appendMenuItem( pMenu, pShutdownIcon->GetUrlDescription( sURL ), aMenuItems[i].nMenuTag );
            }

            // insert the remaining menu entries
            appendMenuItem( pMenu, pShutdownIcon->GetResString( STR_QUICKSTART_FROMTEMPLATE ), MI_TEMPLATE );
            appendMenuItem( pMenu, pShutdownIcon->GetResString( STR_QUICKSTART_FILEOPEN ), MI_OPEN );
            
            [pDefMenu setSubmenu: pMenu];
            [NSApp performSelector:@selector(addFallbackMenuItem:) withObject: pDefMenu];
        }
        else
            DBG_ERROR( "selector failed on NSApp\n" );
    }
}

void SAL_DLLPUBLIC_EXPORT aqua_shutdown_systray()
{
}

}
