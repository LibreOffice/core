/*************************************************************************
 *
 *  $RCSfile: mnumgr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <string>   // HACK: prevent conflict between STLPORT and Workshop headers
#include <cstdarg>  // std::va_list

#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _PSEUDO_HXX //autogen
#include <so3/pseudo.hxx>
#endif
#ifndef _POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif
#pragma hdrstop

#include "mnumgr.hxx"

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

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

// SFX_INIMANAGER()
#include "inimgr.hxx"

#include "sfx.hrc"
#include "menu.hrc"

static const USHORT nCompatVersion = 4;
static const USHORT nVersion = 5;

//=========================================================================

DECL_PTRSTACK(SfxMenuCfgItemArrStack, SfxMenuCfgItemArr*, 4, 4 );

//-------------------------------------------------------------------------

void TryToHideDisabledEntries_Impl( Menu* pMenu )
{
    DBG_ASSERT( pMenu, "invalid menu" );
    if ( !SFX_INIMANAGER()->IsDontHideDisabledEntries() )
        pMenu->SetMenuFlags( pMenu->GetMenuFlags() | MENU_FLAG_HIDEDISABLEDENTRIES );
}

//-------------------------------------------------------------------------

class SfxMenuIter_Impl

/*  [Beschreibung]

    Mit solchen Instanzen wird beim Auslesen des Men"us f"ur die
    Konfiguration ein Stack gebildet, um die Iterator-Position zu
    bestimmen.

    Die Instanzen l"oschen sich selbst, wenn der Stack abgebaut wird.
*/

{
    SfxMenuIter_Impl*   _pPrev;     // der vorherige auf dem Stack
    Menu*               _pMenu;     // das Men"u "uber das gerade iteriert wird
    Menu*               _pPopup;    // Popup an dieser Pos falls noch zu durchl.
    USHORT              _nPos;      // die Position des Iterators in 'pMenu'
    USHORT              _nId;       // Id an Position '_nPos'
    USHORT              _nLevel;    // Schachteltiefe

                        SfxMenuIter_Impl( Menu *pMenu,
                                          SfxMenuIter_Impl *pPrev = 0 );

                        ~SfxMenuIter_Impl()
                        {}

public:
    static SfxMenuIter_Impl* Create( Menu *pMenu )
                        { return ( pMenu && pMenu->GetItemCount() )
                                    ? new SfxMenuIter_Impl( pMenu )
                                    : 0; }

    USHORT              GetLevel() const
                        { return _nLevel; }

    Menu*               GetMenu() const
                        { return _pMenu; }

    USHORT              GetItemId() const
                        { return _nId; }

    MenuItemType        GetItemType() const
                        { return _pMenu->GetItemType( _nPos ); }

    Menu*               GetPopupMenu() const
                        { return _pPopup; }

    String              GetItemText() const
                        { return _pMenu->GetItemText( _nId ); }

    String              GetItemHelpText() const
                        { return _pMenu->GetHelpText( _nId ); }

    BOOL                IsSeparator() const
                        { return MENUITEM_SEPARATOR == GetItemType(); }

    SfxMenuIter_Impl*   NextItem();

    void                RemovePopup()
                        { _pPopup = NULL; }
};

//=========================================================================

SfxMenuIter_Impl::SfxMenuIter_Impl
(
    Menu*               pMenu,
    SfxMenuIter_Impl*   pPrev
)
:   _pPrev( pPrev ),
    _pMenu( pMenu ),
    _nPos( 0 ),
    _nId( pMenu->GetItemId(0) ),
    _nLevel( pPrev ? ( pPrev->_nLevel + 1 ) : 0 )
{
    _pPopup = pMenu->GetPopupMenu( _nId );
}

//--------------------------------------------------------------------

SfxMenuIter_Impl* SfxMenuIter_Impl::NextItem()
{
    // noch nicht durchlaufenes Popup vorhanden
    if ( _pPopup )
    {
        // dann bevor es weitergeht, das Popup durchlaufen
        Menu *pSub = _pPopup;
        _pPopup = 0; // als durchlaufen markieren
        if ( pSub->GetItemCount() )
            return new SfxMenuIter_Impl( pSub, this );
    }

    // zur n"achsten Position in diesem Men"u
    if ( ++_nPos >=_pMenu->GetItemCount() )
    {
        // Men"u zuende => eine Stack-Etage hoch
        SfxMenuIter_Impl *pPrev = _pPrev;
        delete this;
        return pPrev ? pPrev->NextItem() : 0;
    }

    // bleibt in diesem Menu
    _nId = _pMenu->GetItemId(_nPos);
    _pPopup = _pMenu->GetPopupMenu(_nId);

    // nicht alles wird angezeigt
    if ( _nId >= SID_PICK1 && _nId <= SID_PICK9 )
        return NextItem();

    // nicht alle Popups werden durchlaufen
    if ( _nId == SID_OBJECT ||
         ( _nId >= SID_OBJECTMENU0 && _nId <= SID_OBJECTMENU_LAST ) )
        _pPopup = 0;

    // diesen Eintrag nehmen
    return this;
}

//=========================================================================

void SfxMenuManager::Construct( SfxVirtualMenu& rMenu )
{
    DBG_MEMTEST();
    pMenu = &rMenu;

    // set the handlers
    Menu *pSvMenu = pMenu->GetSVMenu();
    pSvMenu->SetSelectHdl( LINK(this, SfxMenuManager, Select) );
    TryToHideDisabledEntries_Impl( pSvMenu );
}

//--------------------------------------------------------------------

SfxMenuManager::SfxMenuManager( const ResId& rResId, SfxBindings &rBindings, BOOL bBar )
:   SfxConfigItem(rResId.GetId()),
    bMenuBar(bBar),
    pMenu(0),
    pOldMenu(0),
    pResMgr(rResId.GetResMgr()),
    pBindings(&rBindings)
{
    bOLE = FALSE;
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

BOOL SfxMenuManager::Initialize(BOOL bOLEServer)
{
    BOOL bRet;
    if (!bOLEServer)
    {
        bRet = SfxConfigItem::Initialize();
    }
    else
    {
        bOLE = TRUE;
        bRet = SfxConfigItem::Initialize();
    }

    return bRet;
}

//--------------------------------------------------------------------

// executes the function for the selected item

IMPL_LINK( SfxMenuManager, Select, Menu *, pMenu )
{
    DBG_MEMTEST();

    USHORT nId = (USHORT) pMenu->GetCurItemId();
    if ( pBindings->IsBound(nId) )
        // normal function
        pBindings->Execute( nId );
    else
        // special menu function
        pBindings->GetDispatcher_Impl()->Execute( nId );

    return TRUE;
}

//--------------------------------------------------------------------

// resets the item iterator, FALSE if none

BOOL SfxMenuManager::FirstItem()
{
    pIterator = SfxMenuIter_Impl::Create( pMenu->GetSVMenu() );
    return 0 != pIterator;
}

//--------------------------------------------------------------------

// skips to the next item, FALSE if no more

BOOL SfxMenuManager::NextItem()
{
    DBG_ASSERT( pIterator, "invalid iterator state" );

    pIterator = pIterator->NextItem();
    return 0 != pIterator;
}

//--------------------------------------------------------------------

// TRUE if current item is a binding

BOOL SfxMenuManager::IsBinding() const
{
    DBG_ASSERT( pIterator, "invalid iterator state" );

    USHORT nId = pIterator->GetItemId();
    if ( pIterator->GetPopupMenu() && nId > SID_SFX_START )
        if ( !SfxMenuControl::IsSpecialControl( nId, *pBindings ) )
            // "Unechtes" Binding: Popup mit SlotId
            return FALSE;
    return nId >= SID_SFX_START;
}

//--------------------------------------------------------------------

// TRUE if current item is a separator

BOOL SfxMenuManager::IsSeparator() const
{
    DBG_ASSERT( pIterator, "invalid iterator state" );

    return pIterator->IsSeparator();
}

//--------------------------------------------------------------------

// TRUE if current item is a popup; may be also a binding

BOOL SfxMenuManager::IsPopup() const
{
    DBG_ASSERT( pIterator, "invalid iterator state" );

    return 0 != pIterator->GetPopupMenu();
}

//--------------------------------------------------------------------

// returns sub-menu-level

USHORT SfxMenuManager::GetLevel() const
{
    DBG_ASSERT( pIterator, "invalid iterator state" );

    return pIterator->GetLevel();
}

//--------------------------------------------------------------------

// id of binding if IsBinding()

USHORT SfxMenuManager::GetItemId() const
{
    DBG_ASSERT( pIterator, "invalid iterator state" );

#ifdef DBG_UTIL
    ByteString aStr( U2S(pIterator->GetItemText()) );
#endif

    USHORT nId = pIterator->GetItemId();

#ifdef DBG_UTIL
    const SfxSlot *pSlot = nId ? SFX_SLOTPOOL().GetSlot( nId ) : NULL;
    if ( pSlot && !pSlot->IsMode( SFX_SLOT_MENUCONFIG ) && nId != SID_MDIWINDOWLIST )
    {
        ByteString aStr( "Slot ");
        aStr += ByteString::CreateFromInt32( nId );
        aStr += ByteString( " : MenuConfig fehlt!" );
        DBG_ERROR( aStr.GetBuffer() );
    }
#endif

    if ( pIterator->GetPopupMenu() )
    {
#if defined( DBG_UTIL ) && !defined( WIN )
        aStr += ByteString( " : Popups muessen Ids != 0 haben!" );
        DBG_ASSERT( nId, aStr.GetBuffer() );
#endif
        return nId;
    }
    else
    {
#if defined( DBG_UTIL ) && !defined( WIN )
        aStr += ByteString( " : Menue-Entries muessen Ids > SID_SFX_START haben!" );
        DBG_ASSERT( nId > SID_SFX_START || pIterator->IsSeparator(), aStr.GetBuffer() );
#endif
        return nId > SID_SFX_START ? nId : SID_NONE;
    }
}

//--------------------------------------------------------------------

// title of popup if IsPopup()

String SfxMenuManager::GetTitle() const
{
    DBG_ASSERT( pIterator, "invalid iterator state" );

    return pIterator->GetItemText();
}

//--------------------------------------------------------------------

String SfxMenuManager::GetHelpText() const
{
    DBG_ASSERT( pIterator, "invalid iterator state" );

    return pIterator->GetItemHelpText();
}

//--------------------------------------------------------------------

// call this before reconfiguring

void SfxMenuManager::Clear()
{
    pBindings->ENTERREGISTRATIONS();
    pOldMenu = pMenu;

    // create the root config and a stack for the current sub-configs
    pCfg = new SfxMenuCfgItemArr;
    pCfgStack = new SfxMenuCfgItemArrStack;
    pCfgStack->Push( pCfg );
}

//--------------------------------------------------------------------

// call this after reconfiguring

void SfxMenuManager::Reconfigure()
{
    SfxVirtualMenu *pVMenu;
    Menu *pSVMenu;
    if (bMenuBar )
    {
        pSVMenu = new MenuBar;
    }
    else
        pSVMenu = new PopupMenu;

    TryToHideDisabledEntries_Impl( pSVMenu );
    ConstructSvMenu( pSVMenu, *pCfg );

    pVMenu = new SfxVirtualMenu( pSVMenu, FALSE, *pBindings, FALSE );
    Construct(*pVMenu);
    if ( bMenuBar && pOldMenu )
    {
        SfxMenuBarManager *pBar = (SfxMenuBarManager*) this;
        MenuBar* pOldBar = (MenuBar*) pOldMenu->GetSVMenu();
        MenuBar* pSvBar = (MenuBar*) GetMenu()->GetSVMenu();
        if ( pBar->GetWindow()->GetMenuBar() == pOldBar )
            //! jetzt im VirtMenu: InvalidateKeyCodes();
            pBar->GetWindow()->SetMenuBar( pSvBar );
    }

    delete pOldMenu;
    pOldMenu = 0;
    pBindings->LEAVEREGISTRATIONS();
    SetDefault( FALSE );
}

//--------------------------------------------------------------------

// Baut aus einem ConfigItemArray rekursiv das StarView-Menu auf

void SfxMenuManager::ConstructSvMenu( Menu* pSuper, SfxMenuCfgItemArr& rCfg)
{
    USHORT nCount = rCfg.Count();
    for ( USHORT n = 0; n < rCfg.Count(); ++n )
    {
        SfxMenuCfgItem *pmIt=rCfg[n];
        if ( rCfg[n]->pPopup )
        {
            pSuper->InsertItem( rCfg[n]->nId, rCfg[n]->aTitle );
            pSuper->SetHelpId( rCfg[n]->nId, (ULONG) rCfg[n]->nId );
            PopupMenu *pPopupMenu;
            pPopupMenu = new PopupMenu;
            pSuper->SetPopupMenu( rCfg[n]->nId, pPopupMenu );
            ConstructSvMenu( pPopupMenu, *(rCfg[n]->pPopup) );
        }
        else if ( rCfg[n]->nId )
        {
            pSuper->InsertItem( rCfg[n]->nId, rCfg[n]->aTitle );
            pSuper->SetHelpId( rCfg[n]->nId, (ULONG) rCfg[n]->nId );
        }
        else
        {
            pSuper->InsertSeparator();
        }
    }
}

//--------------------------------------------------------------------

// Baut aus einem ConfigStream rekursiv das StarView-Menu auf

void SfxMenuManager::ConstructSvMenu( Menu* pSuper, SvStream& rStream,
        BOOL bWithHelp, BOOL bCompat )
{
    SvUShorts aShorts;

    USHORT nCount = 0xFFFF;
    if ( bCompat )
    {
        // Alte Eintr"age wg. Kompatibilit"at einlesen
        USHORT nCtrls,nId;
        rStream >> nCtrls;
        for (USHORT i=0; i<nCtrls; i++)
            rStream >> nId;
        rStream >> nCount;
    }

    char cTag;
    for ( USHORT n=0; n<nCount; n++ )
    {
        rStream >> cTag;
        String aTitle;
        USHORT nId;

        switch ( cTag )
        {
            case 'S':
                pSuper->InsertSeparator();
                break;

            case 'I':
            {
                rStream >> nId;
                rStream.ReadByteString( aTitle, RTL_TEXTENCODING_UTF8 );

                String aHelpText;
                if ( bWithHelp )
                    rStream.ReadByteString( aHelpText, RTL_TEXTENCODING_UTF8 );

                if (nId >= SID_MACRO_START && nId <= SID_MACRO_END)
                {
                    SfxMacroInfo aInfo;
                    rStream >> aInfo;
                    nId = aInfo.GetSlotId();
                }

                pSuper->InsertItem( nId, aTitle );
                pSuper->SetHelpId( nId, (ULONG) nId );
                if ( bWithHelp )
                    pSuper->SetHelpText( nId, aHelpText );
                break;
            }

            case 'P':
            {
                rStream >> nId;
                rStream.ReadByteString( aTitle, RTL_TEXTENCODING_UTF8 );

                if ( nId >= SID_MACRO_START && nId <= SID_MACRO_END ||
                   nId < SID_SFX_START )
                {
                    // Bug im Cfg-Dialog: Popups mit Macro-SlotIds
                    USHORT i;
                    for ( i=0; i<aShorts.Count(); i++ )
                    {
                        // Erste Id nehmen, deren Wert gr"o\ser ist als die Position
                        if ( aShorts[i] > i+1 )
                            break;
                    }

                    nId = i+1;
                    aShorts.Insert( nId, i );
                }

                String aHelpText;
                if ( bWithHelp )
                    rStream.ReadByteString( aHelpText, RTL_TEXTENCODING_UTF8 );

                pSuper->InsertItem( nId, aTitle );
                pSuper->SetHelpId( nId, (ULONG) nId );
                if ( bWithHelp )
                    pSuper->SetHelpText( nId, aHelpText );

                PopupMenu *pPopupMenu;
                pPopupMenu = new PopupMenu;
                pSuper->SetPopupMenu( nId, pPopupMenu );
                ConstructSvMenu( pPopupMenu, rStream, bWithHelp, bCompat );
                break;
            }

            case 'E':
            {
                n = nCount-1;               // Ende
                break;
            }

            default:
            {
                // Wenn ein Fehler aufgetreten ist, sollte man besser aufh"oren
                DBG_ERROR( "invalid item type in menu-config" );
                n = nCount-1;               // Ende
                break;
            }
        }
    }
}

//--------------------------------------------------------------------

// append a binding by function-id

void SfxMenuManager::AppendItem(const String &rText,
                                const String &rHelpText, USHORT nId )
{
    SfxMenuCfgItem* pItem = new SfxMenuCfgItem;
    pItem->nId = nId;
    if (nId >= SID_MACRO_START && nId <= SID_MACRO_END)
        SFX_APP()->GetMacroConfig()->RegisterSlotId(nId);
    pItem->aTitle = rText;
    pItem->aHelpText = rHelpText;
    pItem->pPopup = 0;
    pCfgStack->Top()->Append( pItem );
}

//--------------------------------------------------------------------

// append a separator

void SfxMenuManager::AppendSeparator()
{
    SfxMenuCfgItem* pItem = new SfxMenuCfgItem;
    pItem->nId = 0;
    pItem->pPopup = 0;
    pCfgStack->Top()->Append( pItem );
}

//--------------------------------------------------------------------

// append a popup (sub)-menu

void SfxMenuManager::EnterPopup(const String& rTitle,
                                const String& rHelpText, USHORT nId )
{
    SfxMenuCfgItem* pItem = new SfxMenuCfgItem;
    pItem->nId = nId;
    pItem->aTitle = rTitle;
    pItem->aHelpText = rHelpText;
    pItem->pPopup = new SfxMenuCfgItemArr;
    pCfgStack->Top()->Append( pItem );
    pCfgStack->Push( pItem->pPopup );
}

//--------------------------------------------------------------------

// end a sub-menu

void SfxMenuManager::LeavePopup()
{
    SfxMenuCfgItemArr* pArr = pCfgStack->Top();
    SvUShorts aShorts;

    USHORT n;
    for ( n=0; n<pArr->Count(); n++ )
    {
        USHORT nId = (*pArr)[n]->nId;
        if ( nId < SID_SFX_START )
        {
            // Alle vewendeten Popup-Ids < 5000 merken
            USHORT i;
            for ( i=0; i<aShorts.Count(); i++ )
            {
                if ( aShorts[i] > nId )
                    break;
            }

            aShorts.Insert( nId, i );
        }
    }

    for ( n=0; n<pArr->Count(); n++ )
    {
        SfxMenuCfgItem* pItem = (*pArr)[n];
        if ( pItem->pPopup && pItem->nId >= SID_MACRO_START && pItem->nId <= SID_MACRO_END )
        {
            // Weil in dem Fucking-ConfigDialog Entries und Popups bunt
            // vermischt sind, kann auch ein Popup eine Macro-Id haben, die
            // mu\s nat"urlich weg
            SFX_APP()->GetMacroConfig()->ReleaseSlotId( pItem->nId );

            // Nun noch eine erlaubte Id suchen
            USHORT nCount = aShorts.Count();
            USHORT i;
            for ( i=0; i<nCount; i++ )
            {
                if ( aShorts[i] > i+1 )
                // Erste Id nehmen, deren Wert gr"o\ser ist als die Position
                    break;
            }

            pItem->nId = i+1;
            aShorts.Insert( i+1, i );
        }
    }

    pCfgStack->Pop();
}

//--------------------------------------------------------------------

// reload all KeyCodes

void SfxMenuManager::InvalidateKeyCodes()
{
    if ( pMenu )
        pMenu->InvalidateKeyCodes();
}

//====================================================================

// creates a menu-manager and loads it from resource RID_DEFAULTMENU or RID_DEFAULTPLUGINMENU

SfxMenuBarManager::SfxMenuBarManager( SfxBindings& rBindings, BOOL bPlugin ) :

    SfxMenuManager( bPlugin ? RID_DEFAULTPLUGINMENU : RID_DEFAULTMENU, rBindings, TRUE ),

    pWindow( rBindings.GetSystemWindow() )

{
    DBG_MEMTEST();

    // initialize ObjectMenus
    for ( int n = 0; n < 4; ++n )
    {
        aObjMenus[n].nId = 0;
        aObjMenus[n].pPMMgr = NULL;
        aObjMenus[n].pResMgr = NULL;
    }
}

//--------------------------------------------------------------------

// creates a menu-manager and loads it from a resource

SfxMenuBarManager::SfxMenuBarManager( const ResId& rResId, SfxBindings &rBindings ) :

    SfxMenuManager( rResId, rBindings, TRUE ),

    pWindow( rBindings.GetSystemWindow() )

{
    DBG_MEMTEST();

    // initialize ObjectMenus
    for ( int n = 0; n < 4; ++n )
    {
        aObjMenus[n].nId = 0;
        aObjMenus[n].pPMMgr = NULL;
        aObjMenus[n].pResMgr = NULL;
    }
}

//--------------------------------------------------------------------

SfxMenuBarManager::~SfxMenuBarManager()
{
    DBG_MEMTEST();

    DBG_ASSERT(!aObjMenus[0].pPMMgr || !aObjMenus[1].pPMMgr ||
               !aObjMenus[2].pPMMgr || !aObjMenus[3].pPMMgr,
               "da sind noch ObjectMenus" );

    ResetObjectMenus();
    UpdateObjectMenus();

    MenuBar* pSvBar = (MenuBar*) GetMenu()->GetSVMenu();
    if ( pWindow && pWindow->GetMenuBar() == pSvBar )
        pWindow->SetMenuBar( 0 );
}

//====================================================================

// creates a menu-manager and loads it from a stream

SfxPopupMenuManager::SfxPopupMenuManager(const ResId& rResId, SfxBindings &rBindings )
    : SfxMenuManager( rResId, rBindings, FALSE )
    , pSVMenu( NULL )
{
    DBG_MEMTEST();
    SetInternal(TRUE);
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

IMPL_LINK_INLINE_START( SfxPopupMenuManager, SelectHdl, void *, pvoid )
{
    return 1;
}
IMPL_LINK_INLINE_END( SfxPopupMenuManager, SelectHdl, void *, pvoid )


//--------------------------------------------------------------------

USHORT SfxPopupMenuManager::Execute( Window* pWindow, va_list pArgs, const SfxPoolItem *pArg1 )
{
    DBG_MEMTEST();

    PopupMenu* pMenu = ( (PopupMenu*)GetMenu()->GetSVMenu() );
    pMenu->SetSelectHdl( LINK( this, SfxPopupMenuManager, SelectHdl ) );
    USHORT nId = pMenu->Execute( pWindow, pWindow->GetPointerPosPixel() );
    pMenu->SetSelectHdl( Link() );

    if ( nId )
        GetBindings().GetDispatcher()->_Execute( nId, SFX_CALLMODE_RECORD, pArgs, pArg1 );

    return nId;
}

//--------------------------------------------------------------------

USHORT SfxPopupMenuManager::Execute( Window* pWindow, const SfxPoolItem *pArg1, ... )
{
    DBG_MEMTEST();

    va_list pArgs;
    va_start(pArgs, pArg1);

    return (Execute( pWindow, pArgs, pArg1 ));
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


//--------------------------------------------------------------------

// stores the complete menu-configuration

BOOL SfxMenuManager::Store( SvStream& rStream )
{
    DBG_MEMTEST();
    return StreamMenu( pMenu->GetSVMenu(), rStream, FALSE );
/*
    LanguageType eLangType = SFX_APP()->GetAppInternational().GetLanguage();
    if (eLangType == LANGUAGE_SYSTEM)
    {
        eLangType = System::GetLanguage();
    }

    rStream << nVersion
            << (USHORT) eLangType;

    FirstItem();
    USHORT nLevel = GetLevel();
    do
    {
        while ( GetLevel() < nLevel )
        {
            // Ein Popup wurde verlassen
            // Ende-Markierung setzen
            rStream << 'E';
            nLevel--;
        }

        nLevel = GetLevel();

        if ( IsSeparator() )
        {
            rStream << 'S';
        }
        else
        {
            USHORT nId = GetItemId();
            String aTitle = GetTitle();
            if( !IsPopup() )
            {
                rStream << 'I';
                rStream << nId << aTitle;
                if (nId >= SID_MACRO_START && nId <= SID_MACRO_END)
                    // MacroInfo speichern
                    rStream << *(SFX_APP()->GetMacroConfig()->GetMacroInfo(nId));
            }
            else
            {
                rStream << 'P';
                rStream << nId << aTitle;
                if ( !pIterator->GetPopupMenu()->GetItemCount() )
                    rStream << 'E';
            }
        }
    }
    while ( NextItem() );

    // Letzte Ende-Markierung setzen
    rStream << 'E';

    // Ende-Markierung f"ur MenuBar setzen
    rStream << 'E';
*/
}

//-------------------------------------------------------------------------

void SfxMenuManager::SetForceCtrlCreateMode( BOOL bCreate )
{
    bOLE = TRUE;
}

//--------------------------------------------------------------------

int SfxMenuManager::Load_Impl( SvStream& rStream, BOOL bForPlugComm )
{
    DBG_MEMTEST();

    // Config-Version und Sprache der Menuetexte
    USHORT nFileVersion, nLanguage;
    rStream >> nFileVersion;
    if(nFileVersion < nCompatVersion)
        return SfxConfigItem::WARNING_VERSION;

    rStream >> nLanguage;
    LanguageType eLangType = Application::GetAppInternational().GetLanguage();
    if (eLangType == LANGUAGE_SYSTEM)
    {
        eLangType = System::GetLanguage();
    }

    if ( eLangType != (LanguageType) nLanguage && !bForPlugComm )
    {
        // Wenn die Text f"ur eine andere Sprache sind, Default-Config
        // verwenden. Fehlermeldung ??
        UseDefault();
        return SfxConfigItem::ERR_OK;
    }

    SfxVirtualMenu *pOldVirtMenu=0;
    if (pMenu)
    {
        // Es wird umkonfiguriert
        pOldVirtMenu = pMenu;
        pBindings->ENTERREGISTRATIONS();
    }

    BOOL bCompat = FALSE;
    if ( nFileVersion == nCompatVersion )
        bCompat = TRUE;
    BOOL bWithHelp = FALSE;
    if ( bCompat || bForPlugComm )
        bWithHelp = TRUE;

    Menu *pSVMenu;
    if (bMenuBar )
    {
        pSVMenu = new MenuBar;
        TryToHideDisabledEntries_Impl( pSVMenu );
    }
    else
        pSVMenu = new PopupMenu;

    ConstructSvMenu( pSVMenu, rStream, bWithHelp, bCompat );

    SfxVirtualMenu *pVMenu = new SfxVirtualMenu( pSVMenu, bWithHelp, *pBindings, bOLE );
    Construct(*pVMenu);

    if ( bMenuBar && pOldVirtMenu )
    {
        SfxMenuBarManager *pBar = (SfxMenuBarManager*) this;
        MenuBar* pOldBar = (MenuBar*) pOldVirtMenu->GetSVMenu();
        MenuBar* pSvBar = (MenuBar*) GetMenu()->GetSVMenu();
        if ( pBar->GetWindow()->GetMenuBar() == pOldBar )
            //! jetzt im VirtMenu: InvalidateKeyCodes();
            pBar->GetWindow()->SetMenuBar( pSvBar );
    }

    if (pOldVirtMenu)
    {
        delete pOldVirtMenu;
        pBindings->LEAVEREGISTRATIONS();
    }

    return SfxConfigItem::ERR_OK;
}

//-------------------------------------------------------------------------

int SfxMenuManager::Load( SvStream& rStream )
{
    return Load_Impl( rStream, FALSE );
}

//--------------------------------------------------------------------

void SfxMenuManager::UseDefault()
{
    DBG_MEMTEST();

    SfxApplication *pSfxApp = SFX_APP();
    SfxVirtualMenu *pOldVirtMenu=0;
    if (pMenu)
    {
        pOldVirtMenu = pMenu;
        pBindings->ENTERREGISTRATIONS();
    }

    SfxVirtualMenu *pVMenu = 0;
    if ( bMenuBar )
    {
        ResId aId( GetType(), pResMgr );
        aId.SetRT(RSC_MENU);
        if ( Resource::GetResManager()->IsAvailable( aId ) )
        {
            MenuBar *pSvMenu = new MenuBar( ResId(GetType(), pResMgr) );
            TryToHideDisabledEntries_Impl( pSvMenu );
            pVMenu = new SfxVirtualMenu( pSvMenu, FALSE, *pBindings, bOLE, TRUE );
        }
        else
        {
            MenuBar *pSvMenu = new MenuBar;
            pVMenu = new SfxVirtualMenu( pSvMenu, FALSE, *pBindings, bOLE, TRUE );
        }
    }
    else
    {
        ResId aResId(GetType());
        aResId.SetRT(RSC_MENU);
        aResId.SetResMgr(pResMgr);
        Menu *pSVMenu = new PopupMenu( aResId );

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

    if ( bMenuBar && pOldVirtMenu )
    {
        SfxMenuBarManager *pBar = (SfxMenuBarManager*) this;
        MenuBar* pOldBar = (MenuBar*) pOldVirtMenu->GetSVMenu();
        MenuBar* pSvBar = (MenuBar*) GetMenu()->GetSVMenu();
        if ( pBar->GetWindow()->GetMenuBar() == pOldBar )
            //! jetzt im VirtMenu: InvalidateKeyCodes();
            pBar->GetWindow()->SetMenuBar( pSvBar );
    }

    if (pOldVirtMenu)
    {
        delete pOldVirtMenu;
        pBindings->LEAVEREGISTRATIONS();
    }
}

// ------------------------------------------------------------------------

String SfxMenuManager::GetName() const
{
    return String(SfxResId(STR_MENU_CFGITEM));
}


// ------------------------------------------------------------------------

BOOL SfxMenuManager::IsPopupFunction( USHORT nId )
{
    return nId == SID_PICKLIST ||
           nId == SID_MDIWINDOWLIST ||
           nId == SID_HELPMENU ||
           nId == SID_HELPMENU;
}

//------------------------------------------------------------------------

void SfxMenuBarManager::ResetObjectMenus()
{
    for ( int n = 0; n <= 3; ++n )
        aObjMenus[n].nId = 0;
}

//------------------------------------------------------------------------

void SfxMenuBarManager::SetObjectMenu( USHORT nPos, const ResId& rResId )
{
    nPos -= SID_OBJECTMENU0;
    DBG_ASSERT( nPos < 4, "only 4 object Menu positions defined" );

    aObjMenus[nPos].nId = rResId.GetId();
    aObjMenus[nPos].pResMgr = rResId.GetResMgr();
}


//------------------------------------------------------------------------

MenuBar* SfxMenuBarManager::GetMenuBar() const
{
    return (MenuBar*) GetMenu()->GetSVMenu();
}

//------------------------------------------------------------------------

PopupMenu* SfxMenuBarManager::GetObjectMenu( USHORT nPos, USHORT &rConfigId )
{
    nPos -= SID_OBJECTMENU0;
    DBG_ASSERT( nPos < 4, "only 4 object Menu positions defined" );

    rConfigId = aObjMenus[nPos].nId;
    return aObjMenus[nPos].pPMMgr
            ? (PopupMenu*) aObjMenus[nPos].pPMMgr->GetMenu()->GetSVMenu()
            : 0;
}

//------------------------------------------------------------------------

void SfxMenuBarManager::UpdateObjectMenus()
{
    Menu *pMenu = GetMenu()->GetSVMenu();
    for ( int n = 0; n < 4; ++n )
    {
        SfxPopupMenuManager *pOldPopup = aObjMenus[n].pPMMgr;
        if ( aObjMenus[n].nId )
        {
            // Nur ObjectMenues erzeugen, wenn im Menue auch vorhanden!
            if ( pMenu->GetItemPos( n + SID_OBJECTMENU0 ) != MENU_ITEM_NOTFOUND )
            {
                if ( !pOldPopup ||
                    ( pOldPopup && pOldPopup->GetType() != aObjMenus[n].nId ) )
                {
                    ResId aResId( aObjMenus[n].nId, aObjMenus[n].pResMgr );
                    aObjMenus[n].pPMMgr =
                        new SfxPopupMenuManager( aResId , GetBindings() );
                    aObjMenus[n].pPMMgr->Initialize();
                }
                else
                    pOldPopup = NULL;

                pMenu->SetPopupMenu( n + SID_OBJECTMENU0,
                    (PopupMenu*) aObjMenus[n].pPMMgr->GetMenu()->GetSVMenu() );
                pMenu->EnableItem( n + SID_OBJECTMENU0, TRUE );
            }
        }
        else if ( pOldPopup )
        {
            if ( pMenu->GetItemPos( n + SID_OBJECTMENU0 ) != MENU_ITEM_NOTFOUND )
            {
                pMenu->EnableItem( n + SID_OBJECTMENU0, FALSE );
                pMenu->SetPopupMenu( n + SID_OBJECTMENU0, 0 );
            }
        }

        if ( pOldPopup )
        {
            if ( pOldPopup == aObjMenus[n].pPMMgr )
                aObjMenus[n].pPMMgr = NULL;
            pOldPopup->StoreConfig();
            delete pOldPopup;
        }
    }
}

//------------------------------------------------------------------------

void SfxMenuBarManager::ReconfigureObjectMenus()
{
    for ( int n = 0; n < 4; ++n )
    {
        if ( aObjMenus[n].nId )
        {
            Menu *pMenu = GetMenu()->GetSVMenu();
            if ( pMenu->GetItemPos( n + SID_OBJECTMENU0 ) != MENU_ITEM_NOTFOUND )
            {
                if ( !aObjMenus[n].pPMMgr )
                {
                    ResId aResId(aObjMenus[n].nId, aObjMenus[n].pResMgr);
                    aObjMenus[n].pPMMgr =
                        new SfxPopupMenuManager( aResId , GetBindings() );
                    aObjMenus[n].pPMMgr->Initialize();
                }

                pMenu->SetPopupMenu( n + SID_OBJECTMENU0,
                    (PopupMenu*) aObjMenus[n].pPMMgr->GetMenu()->GetSVMenu() );
                pMenu->EnableItem( n + SID_OBJECTMENU0, TRUE );
            }
        }
    }
}

//-------------------------------------------------------------------------

void SfxMenuManager::InsertVerbs(const SvVerbList *pList)

/*  Man k"onnte hier auch einen Separator oder eine bestimmte Menu-Id
    als Kennung zu Einf"ugen benutzen, dann mu\s man aber das Items-Array
    verschieben etc.
*/

{
    // hinten anh"angen
    if ( !pList->Count() )
        return;

    Menu *pMenu = GetMenu()->GetSVMenu();
    pMenu->InsertSeparator();

    USHORT nr=0;
    for ( USHORT n = 0; n < pList->Count(); ++n )
    {
        // nicht alle Verbs landen im Men"u
        const SvVerb& rVerb = (*pList)[n];
        if ( !rVerb.IsOnMenu() )
            continue;

        // neue Id vergeben
        USHORT nId = SID_VERB_START + nr++;
        DBG_ASSERT(nId <= SID_VERB_END, "Zuviele Verben!");
        if ( nId > SID_VERB_END )
            break;

        // einf"ugen
        pMenu->InsertItem( nId, rVerb.GetName() );
        pMenu->SetHelpId( nId, (ULONG) nId );
    }
}

//-------------------------------------------------------------------------

BOOL SfxMenuManager::StreamMenu( Menu *pMenu, SvStream& rStream, BOOL bWithHelp )
{
    LanguageType eLangType = Application::GetAppInternational().GetLanguage();
    if (eLangType == LANGUAGE_SYSTEM)
    {
        eLangType = System::GetLanguage();
    }

    rStream << nVersion
            << (USHORT) eLangType;

    if ( !FirstItem() )
        return TRUE;

    SfxMenuIter_Impl *pFirstIter = pIterator;
    USHORT nFirstLevel = pFirstIter->GetLevel();
    USHORT nLevel = nFirstLevel;
    do
    {
        while ( pIterator->GetLevel() < nLevel )
        {
            // Ein Popup wurde verlassen
            // Ende-Markierung setzen
            rStream << 'E';
            nLevel--;
        }

        nLevel = pIterator->GetLevel();

        if ( pIterator->IsSeparator() )
        {
            rStream << 'S';
        }
        else
        {
            USHORT nId = pIterator->GetItemId();
            String aTitle = pIterator->GetItemText();
            if( IsBinding() )
            {
                rStream << 'I';
                rStream << nId;
                rStream.WriteByteString(aTitle, RTL_TEXTENCODING_UTF8 );
                if ( bWithHelp )
                    rStream.WriteByteString(pIterator->GetItemHelpText(), RTL_TEXTENCODING_UTF8 );
                if (nId >= SID_MACRO_START && nId <= SID_MACRO_END)
                    // MacroInfo speichern
                    rStream << *(SFX_APP()->GetMacroConfig()->GetMacroInfo(nId));
                if ( pIterator->GetPopupMenu() )
                    // Unechtes Popup "uberspringen
                    pIterator->RemovePopup();
            }
            else if ( pIterator->GetPopupMenu() )
            {
                rStream << 'P';
                rStream << nId;
                rStream.WriteByteString(aTitle, RTL_TEXTENCODING_UTF8 );
                if ( bWithHelp )
                    rStream.WriteByteString(pIterator->GetItemHelpText(), RTL_TEXTENCODING_UTF8 );
                if ( !pIterator->GetPopupMenu()->GetItemCount() )
                    rStream << 'E';
            }
            else
                DBG_ERROR( "Invalid menu configuration!" );
        }
    }
    while ( NextItem() );

    while ( nLevel > nFirstLevel )
    {
        // Ein Popup wurde verlassen
        // Ende-Markierung setzen
        rStream << 'E';
        nLevel--;
    }

    // Ende-Markierung f"ur MenuBar setzen
    rStream << 'E';
    return TRUE;
}

void SfxPopupMenuManager::AddClipboardFunctions()
{
    if ( bMenuBar )
    {
        DBG_ERROR( "Not for menubars!" );
        return;
    }

    bAddClipboardFuncs = TRUE;
}


