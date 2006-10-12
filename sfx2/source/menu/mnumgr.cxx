/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mnumgr.cxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 15:57:54 $
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

#ifndef _COM_SUN_STAR_EMBED_VERBDESCRIPTOR_HPP_
#include <com/sun/star/embed/VerbDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_VERBATTRIBUTES_HPP_
#include <com/sun/star/embed/VerbAttributes.hpp>
#endif

#ifdef SOLARIS
// HACK: prevent conflict between STLPORT and Workshop headers on Solaris 8
#include <ctime>
#endif

#include <string>   // HACK: prevent conflict between STLPORT and Workshop headers
#include <cstdarg>  // std::va_list

#ifndef _POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif
#ifndef GCC
#endif

#include <unotools/streamwrap.hxx>
#include <objsh.hxx>
#include <framework/menuconfiguration.hxx>
#include <framework/addonmenu.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/urlobj.hxx>
#include <svtools/pathoptions.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include "mnumgr.hxx"

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#include <svtools/menuoptions.hxx>

#include "virtmenu.hxx"
#include "msg.hxx"
#include "dispatch.hxx"
#include "minstack.hxx"
#include "app.hxx"
#include "sfxtypes.hxx"
#include "bindings.hxx"
#include "mnucfga.hxx"
#include "sfxresid.hxx"
#include "macrconf.hxx"
#include "msgpool.hxx"
#include "sfx.hrc"
#include "menu.hrc"
#include "viewfrm.hxx"
#include "viewsh.hxx"
#include "objface.hxx"

static const USHORT nCompatVersion = 4;
static const USHORT nVersion = 5;

using namespace com::sun::star;

//=========================================================================

DECL_PTRSTACK(SfxMenuCfgItemArrStack, SfxMenuCfgItemArr*, 4, 4 );

//-------------------------------------------------------------------------

void TryToHideDisabledEntries_Impl( Menu* pMenu )
{
    DBG_ASSERT( pMenu, "invalid menu" );
    if( SvtMenuOptions().IsEntryHidingEnabled() == sal_False )
    {
        pMenu->SetMenuFlags( pMenu->GetMenuFlags() | MENU_FLAG_HIDEDISABLEDENTRIES );
    }
}

//-------------------------------------------------------------------------

SfxMenuManager::SfxMenuManager( const ResId& rResId, SfxBindings &rBindings )
:   pMenu(0),
    pOldMenu(0),
    pBindings(&rBindings),
    pResMgr(rResId.GetResMgr()),
    nType( rResId.GetId() )
{
    bAddClipboardFuncs = FALSE;
    DBG_MEMTEST();
}

//--------------------------------------------------------------------

SfxMenuManager::~SfxMenuManager()
{
    DBG_MEMTEST();
    pBindings->ENTERREGISTRATIONS();
    delete pMenu;
    pBindings->LEAVEREGISTRATIONS();
}

//--------------------------------------------------------------------

void SfxMenuManager::Construct( SfxVirtualMenu& rMenu )
{
    DBG_MEMTEST();
    pMenu = &rMenu;

    // set the handlers
    Menu *pSvMenu = pMenu->GetSVMenu();
    pSvMenu->SetSelectHdl( LINK(this, SfxMenuManager, Select) );
    TryToHideDisabledEntries_Impl( pSvMenu );
}

//-------------------------------------------------------------------------
void InsertVerbs_Impl( SfxBindings* pBindings, const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& aVerbs, Menu* pMenu )
{
    SfxViewShell *pView = pBindings->GetDispatcher()->GetFrame()->GetViewShell();
    if ( pView && aVerbs.getLength() )
    {
        SfxObjectShell* pDoc = pView->GetObjectShell();
        pMenu->InsertSeparator();
        USHORT nr=0;
        for ( USHORT n = 0; n < aVerbs.getLength(); ++n )
        {
            // check for ReadOnly verbs
            if ( pDoc->IsReadOnly() && !(aVerbs[n].VerbAttributes & embed::VerbAttributes::MS_VERBATTR_NEVERDIRTIES) )
                continue;

            // check for verbs that shouldn't appear in the menu
            if ( !(aVerbs[n].VerbAttributes & embed::VerbAttributes::MS_VERBATTR_ONCONTAINERMENU) )
                continue;

            // neue Id vergeben
            USHORT nId = SID_VERB_START + nr++;
            DBG_ASSERT(nId <= SID_VERB_END, "Zuviele Verben!");
            if ( nId > SID_VERB_END )
                break;

            // einf"ugen
            pMenu->InsertItem( nId, aVerbs[n].VerbName );
            pMenu->SetHelpId( nId, (ULONG) nId );
        }
    }
}

//--------------------------------------------------------------------

void SfxMenuManager::UseDefault()
{
    DBG_MEMTEST();

    SFX_APP();
    SfxVirtualMenu *pOldVirtMenu=0;
    if (pMenu)
    {
        pOldVirtMenu = pMenu;
        pBindings->ENTERREGISTRATIONS();
    }

    SfxVirtualMenu *pVMenu = 0;
    {
        ResId aResId(GetType());
        aResId.SetRT(RSC_MENU);
        aResId.SetResMgr(pResMgr);
        Menu *pSVMenu = new PopupMenu( aResId );
        //SfxMenuManager::EraseItemCmds( pSVMenu ); // Remove .uno cmds to be compatible with 6.0/src641

        if ( bAddClipboardFuncs )
        {
            USHORT n, nCount = pSVMenu->GetItemCount();
            for ( n=0; n<nCount; n++ )
            {
                USHORT nId = pSVMenu->GetItemId( n );
                if ( nId == SID_COPY || nId == SID_CUT || nId == SID_PASTE )
                    break;
            }

            if ( n == nCount )
            {
                PopupMenu aPop( SfxResId( MN_CLIPBOARDFUNCS ) );
                nCount = aPop.GetItemCount();
                pSVMenu->InsertSeparator();
                for ( n=0; n<nCount; n++ )
                {
                    USHORT nId = aPop.GetItemId( n );
                    pSVMenu->InsertItem( nId, aPop.GetItemText( nId ), aPop.GetItemBits( nId ) );
                }
            }
        }

        pVMenu = new SfxVirtualMenu( pSVMenu, FALSE, *pBindings, TRUE, TRUE );
    }

    Construct(*pVMenu);
    if (pOldVirtMenu)
    {
        delete pOldVirtMenu;
        pBindings->LEAVEREGISTRATIONS();
    }
}

// ------------------------------------------------------------------------

// executes the function for the selected item
IMPL_LINK( SfxMenuManager, Select, Menu *, pSelMenu )
{
    DBG_MEMTEST();

    USHORT nId = (USHORT) pSelMenu->GetCurItemId();
    String aCommand = pSelMenu->GetItemCommand( nId );
    if ( aCommand.Len() )
    {
        pBindings->ExecuteCommand_Impl( aCommand );
    }
    else if ( pBindings->IsBound(nId) )
        // normal function
        pBindings->Execute( nId );
    else
        // special menu function
        pBindings->GetDispatcher_Impl()->Execute( nId );

    return TRUE;
}

//--------------------------------------------------------------------

void SfxMenuManager::Construct_Impl( Menu* pSVMenu, BOOL bWithHelp )
{
    SfxVirtualMenu *pOldVirtMenu=0;
    if ( pMenu )
    {
        // Es wird umkonfiguriert
        pOldVirtMenu = pMenu;
        pBindings->ENTERREGISTRATIONS();
    }

    TryToHideDisabledEntries_Impl( pSVMenu );
    SfxVirtualMenu *pVMenu = new SfxVirtualMenu( pSVMenu, bWithHelp, *pBindings, TRUE );
    Construct(*pVMenu);

    if ( pOldVirtMenu )
    {
        delete pOldVirtMenu;
        pBindings->LEAVEREGISTRATIONS();
    }
}

//--------------------------------------------------------------------

// don't insert Popups into ConfigManager, they are not configurable at the moment !
SfxPopupMenuManager::SfxPopupMenuManager(const ResId& rResId, SfxBindings &rBindings )
    : SfxMenuManager( rResId, rBindings )
    , pSVMenu( NULL )
{
    DBG_MEMTEST();
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::RemoveDisabledEntries()
{
    if ( pSVMenu )
        TryToHideDisabledEntries_Impl( pSVMenu );
}

//--------------------------------------------------------------------

USHORT SfxPopupMenuManager::Execute( const Point& rPos, Window* pWindow )
{
    DBG_MEMTEST();
    return ( (PopupMenu*) GetMenu()->GetSVMenu() )->Execute( pWindow, rPos );
}

//--------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxPopupMenuManager, SelectHdl, void *, EMPTYARG )
{
    return 1;
}
IMPL_LINK_INLINE_END( SfxPopupMenuManager, SelectHdl, void *, EMPTYARG )


//--------------------------------------------------------------------

USHORT SfxPopupMenuManager::Execute( const Point& rPoint, Window* pWindow, va_list pArgs, const SfxPoolItem *pArg1 )
{
    DBG_MEMTEST();

    PopupMenu* pPopMenu = ( (PopupMenu*)GetMenu()->GetSVMenu() );
    pPopMenu->SetSelectHdl( LINK( this, SfxPopupMenuManager, SelectHdl ) );
    USHORT nId = pPopMenu->Execute( pWindow, rPoint );
    pPopMenu->SetSelectHdl( Link() );

    if ( nId )
        GetBindings().GetDispatcher()->_Execute( nId, SFX_CALLMODE_RECORD, pArgs, pArg1 );

    return nId;
}

//--------------------------------------------------------------------

USHORT SfxPopupMenuManager::Execute( const Point& rPoint, Window* pWindow, const SfxPoolItem *pArg1, ... )
{
    DBG_MEMTEST();

    va_list pArgs;
    va_start(pArgs, pArg1);

    return (Execute( rPoint, pWindow, pArgs, pArg1 ));
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::StartInsert()
{
    ResId aResId(GetType());
    aResId.SetRT(RSC_MENU);
    aResId.SetResMgr(pResMgr);
    pSVMenu = new PopupMenu( aResId );
    TryToHideDisabledEntries_Impl( pSVMenu );
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::EndInsert()
{
    pBindings->ENTERREGISTRATIONS();
    pMenu = new SfxVirtualMenu( pSVMenu, FALSE, *pBindings, TRUE, TRUE );
    Construct( *pMenu );
    pBindings->LEAVEREGISTRATIONS();
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::InsertSeparator( USHORT nPos )
{
    pSVMenu->InsertSeparator( nPos );
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::InsertItem( USHORT nId, const String& rName, MenuItemBits nBits, USHORT nPos )
{
    pSVMenu->InsertItem( nId, rName, nBits,nPos );
    pSVMenu->SetHelpId( nId, (ULONG) nId );
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::RemoveItem( USHORT nId )
{
    pSVMenu->RemoveItem( nId );
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::CheckItem( USHORT nId, BOOL bCheck )
{
    pSVMenu->CheckItem( nId, bCheck );
}

void SfxPopupMenuManager::AddClipboardFunctions()
{
    bAddClipboardFuncs = TRUE;
}

SfxMenuManager::SfxMenuManager( Menu* pMenuArg, SfxBindings &rBindings )
:   pMenu(0),
    pOldMenu(0),
    pBindings(&rBindings),
    pResMgr(NULL),
    nType(0)
{
    bAddClipboardFuncs = FALSE;
    SfxVirtualMenu* pVMenu = new SfxVirtualMenu( pMenuArg, FALSE, rBindings, TRUE, TRUE );
    Construct(*pVMenu);
}

SfxPopupMenuManager::SfxPopupMenuManager( PopupMenu* pMenuArg, SfxBindings& rBindings )
    : SfxMenuManager( pMenuArg, rBindings )
    , pSVMenu( pMenuArg )
{
}

void SfxPopupMenuManager::ExecutePopup( const ResId& rResId, SfxViewFrame* pFrame, const Point& rPoint, Window* pWindow )
{
    PopupMenu *pSVMenu = new PopupMenu( rResId );
    USHORT n, nCount = pSVMenu->GetItemCount();
    for ( n=0; n<nCount; n++ )
    {
        USHORT nId = pSVMenu->GetItemId( n );
        if ( nId == SID_COPY || nId == SID_CUT || nId == SID_PASTE )
            break;
    }

    if ( n == nCount )
    {
        PopupMenu aPop( SfxResId( MN_CLIPBOARDFUNCS ) );
        nCount = aPop.GetItemCount();
        pSVMenu->InsertSeparator();
        for ( n=0; n<nCount; n++ )
        {
            USHORT nId = aPop.GetItemId( n );
            pSVMenu->InsertItem( nId, aPop.GetItemText( nId ), aPop.GetItemBits( nId ) );
            pSVMenu->SetHelpId( nId, aPop.GetHelpId( nId ));
        }
    }

    InsertVerbs_Impl( &pFrame->GetBindings(), pFrame->GetViewShell()->GetVerbs(), pSVMenu );
    Menu* pMenu = NULL;
    ::com::sun::star::ui::ContextMenuExecuteEvent aEvent;
    aEvent.SourceWindow = VCLUnoHelper::GetInterface( pWindow );
    aEvent.ExecutePosition.X = rPoint.X();
    aEvent.ExecutePosition.Y = rPoint.Y();
    if ( pFrame->GetViewShell()->TryContextMenuInterception( *pSVMenu, pMenu, aEvent ) )
    {
        if ( pMenu )
        {
            delete pSVMenu;
            pSVMenu = (PopupMenu*) pMenu;
        }

        SfxPopupMenuManager aPop( pSVMenu, pFrame->GetBindings() );
        aPop.RemoveDisabledEntries();
        aPop.Execute( rPoint, pWindow );
    }
}
