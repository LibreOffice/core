/*************************************************************************
 *
 *  $RCSfile: mnumgr.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 11:28:46 $
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

#ifdef SOLARIS
// HACK: prevent conflict between STLPORT and Workshop headers on Solaris 8
#include <ctime>
#endif

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

#include <unotools/streamwrap.hxx>
#include <objsh.hxx>
#include <framework/menuconfiguration.hxx>
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

class SfxMenuIter_Impl

/*  [Beschreibung]

    Mit solchen Instanzen wird beim Auslesen des Men"us f"ur die
    Konfiguration ein Stack gebildet, um die Iterator-Position zu
    bestimmen.

    Die Instanzen l"oschen sich selbst, wenn der Stack abgebaut wird.
*/

{
    String              _aCommand;
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

    String              GetCommand() const
                        { return _aCommand; }

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

    BOOL                IsBinding( SfxModule* pMod ) const;

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
    _aCommand = pMenu->GetItemCommand( _nId );
    if ( _aCommand.CompareToAscii("slot:", 5) == 0 )
        _aCommand.Erase();

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
    _aCommand = _pMenu->GetItemCommand( _nId );
    if ( _aCommand.CompareToAscii("slot:", 5) == 0 )
        _aCommand.Erase();
    _pPopup = _pMenu->GetPopupMenu(_nId);

    // nicht alles wird angezeigt
    if ( _nId >= START_ITEMID_PICKLIST && _nId <= END_ITEMID_PICKLIST )
        return NextItem();

    if ( _nId >= START_ITEMID_WINDOWLIST && _nId <= END_ITEMID_WINDOWLIST )
        return NextItem();

    if ( _nId == SID_ADDONS )
        return NextItem();

    // nicht alle Popups werden durchlaufen
    if ( _nId == SID_OBJECT ||
         ( _nId >= SID_OBJECTMENU0 && _nId <= SID_OBJECTMENU_LAST ) )
        _pPopup = 0;

    // diesen Eintrag nehmen
    return this;
}

BOOL SfxMenuIter_Impl::IsBinding( SfxModule* pMod ) const
{
    if ( GetPopupMenu() && _nId > SID_SFX_START )
        if ( !SfxMenuControl::IsSpecialControl( _nId, pMod ) )
            // "Unechtes" Binding: Popup mit SlotId
            return FALSE;
    return _nId >= SID_SFX_START || ( GetCommand().Len() != 0);
}

//--------------------------------------------------------------------

SfxMenuManager::SfxMenuManager( const ResId& rResId, SfxBindings &rBindings, SfxConfigManager* pMgr, BOOL bBar )
:   SfxConfigItem( rResId.GetId(), pMgr ),
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

BOOL SfxMenuManager::Store( SvStream& rStream )
{
    DBG_MEMTEST();
    SfxDispatcher* pDisp = pBindings->GetDispatcher_Impl();
    SfxModule *pMod = pDisp ? SFX_APP()->GetActiveModule( pDisp->GetFrame() ) :0;
    return StoreMenu( rStream, pMenu->GetSVMenu(), pMod );
}

//-------------------------------------------------------------------------
void InsertVerbs_Impl( const SvVerbList* pList, Menu* pMenu )
{
    if ( pList )
    {
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
}

void SfxMenuManager::InsertVerbs(const SvVerbList *pList)

/*  Man k"onnte hier auch einen Separator oder eine bestimmte Menu-Id
    als Kennung zu Einf"ugen benutzen, dann mu\s man aber das Items-Array
    verschieben etc.
*/

{
    // hinten anh"angen
    if ( !pList || !pList->Count() )
        return;

    Menu *pMenu = GetMenu()->GetSVMenu();
    InsertVerbs_Impl( pList, pMenu );
}

//-------------------------------------------------------------------------

BOOL SfxMenuManager::StoreMenu( SvStream& rStream, Menu* pMenu, SfxModule* pMod )
{
    LanguageType eLangType = Application::GetSettings().GetUILanguage();
    rStream << nVersion
            << (USHORT) eLangType;

    SfxMenuIter_Impl *pIterator = SfxMenuIter_Impl::Create( pMenu );
    if ( !pIterator )
        return TRUE;

    rtl_TextEncoding nEnc = osl_getThreadTextEncoding();
    SfxMacroConfig* pMC = SfxMacroConfig::GetOrCreate();
    SvUShorts aMacroSlots;

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
            if( pIterator->IsBinding( pMod ) )
            {
                String aCmd( pIterator->GetCommand() );
                BOOL bIsMacroSlot = SfxMacroConfig::IsMacroSlot(nId);

                if ( aCmd.CompareToAscii("macro:", 6) == 0 && !bIsMacroSlot )
                {
                    SfxMacroInfo aInfo( aCmd );
                    pMC->GetSlotId( &aInfo );
                    nId = aInfo.GetSlotId();
                    aMacroSlots.Insert( nId, aMacroSlots.Count() );
                }

                if ( bIsMacroSlot )
                {
                    SfxMacroInfo* pInfo = pMC->GetMacroInfo(nId);
                    if ( pInfo )
                    {
                        rStream << 'I';
                        rStream << nId;
                        rStream.WriteByteString(aTitle, nEnc );
                        rStream << *(pInfo);
                    }
                    else
                    {
                        // trashed config entry, avoid disaster
                        rStream << 'S';
                    }
                }
                else
                {
                    rStream << 'I';
                    rStream << nId;
                    rStream.WriteByteString(aTitle, nEnc );

                    if ( pIterator->GetPopupMenu() )
                        // Unechtes Popup "uberspringen
                        pIterator->RemovePopup();
                }
            }
            else if ( pIterator->GetPopupMenu() )
            {
                rStream << 'P';
                rStream << nId;
                rStream.WriteByteString(aTitle, nEnc );
                if ( !pIterator->GetPopupMenu()->GetItemCount() )
                    rStream << 'E';
            }
            else
                DBG_ERROR( "Invalid menu configuration!" );
        }
    }
    while ( ( pIterator = pIterator->NextItem() ) != 0 );

    while ( nLevel > nFirstLevel )
    {
        // Ein Popup wurde verlassen
        // Ende-Markierung setzen
        rStream << 'E';
        nLevel--;
    }

    // Ende-Markierung f"ur MenuBar setzen
    rStream << 'E';

    for ( USHORT n=0; n<aMacroSlots.Count(); n++ )
        pMC->ReleaseSlotId( aMacroSlots[n] );

    return TRUE;
}

//-------------------------------------------------------------------------

void SfxMenuManager::SetForceCtrlCreateMode( BOOL bCreate )
{
    bOLE = bCreate;
}

//-------------------------------------------------------------------------

Menu* SfxMenuManager::LoadMenu( SvStream& rStream )
{
    // Config-Version und Sprache der Menuetexte
    USHORT nFileVersion, nLanguage;
    rStream >> nFileVersion;
    if(nFileVersion < nCompatVersion)
        return NULL;

    rStream >> nLanguage;
    LanguageType eLangType = Application::GetSettings().GetUILanguage();
    if ( eLangType != (LanguageType) nLanguage )
        return NULL;

    BOOL bCompat = FALSE;
    if ( nFileVersion == nCompatVersion )
        bCompat = TRUE;
    BOOL bWithHelp = FALSE;
//    if ( bCompat )
//        bWithHelp = TRUE;

    MenuBar* pSVMenu = new MenuBar;
    ConstructSvMenu( pSVMenu, rStream, bWithHelp, bCompat );
    return pSVMenu;
}

int SfxMenuManager::Load( SvStream& rStream )
{
    DBG_MEMTEST();

    if ( !bMenuBar )
    {
        UseDefault();
    }
    else
    {
        Menu* pSVMenu = LoadMenu( rStream );
        if ( pSVMenu )
        {
            Construct_Impl( pSVMenu, FALSE );
            SetDefault( FALSE );
        }
        else
            UseDefault();
    }

    return SfxConfigItem::ERR_OK;
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
            SfxMenuManager::EraseItemCmds( pSvMenu ); // Remove .uno cmds to be compatible with 6.0/src641
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
        SfxMenuManager::EraseItemCmds( pSVMenu ); // Remove .uno cmds to be compatible with 6.0/src641

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
            pBar->GetWindow()->SetMenuBar( pSvBar );
    }

    if (pOldVirtMenu)
    {
        delete pOldVirtMenu;
        pBindings->LEAVEREGISTRATIONS();
    }

    SetDefault( TRUE );
}

// ------------------------------------------------------------------------

String SfxMenuManager::GetStreamName() const
{
    return SfxConfigItem::GetStreamName( GetType() );
}

//--------------------------------------------------------------------

// Baut aus einem ConfigItemArray rekursiv das StarView-Menu auf

void SfxMenuManager::ConstructSvMenu( Menu* pSuper, SfxMenuCfgItemArr& rCfg)
{
    USHORT nCount = rCfg.Count();
    for ( USHORT n = 0; n < rCfg.Count(); ++n )
    {
        SfxMenuCfgItem *pmIt=rCfg[n];
        USHORT nId = rCfg[n]->nId;
        if ( rCfg[n]->pPopup )
        {
            pSuper->InsertItem( nId, rCfg[n]->aTitle );
            pSuper->SetHelpId( nId, (ULONG) nId );
            PopupMenu *pPopupMenu;
            pPopupMenu = new PopupMenu;
            pSuper->SetPopupMenu( nId, pPopupMenu );
            ConstructSvMenu( pPopupMenu, *(rCfg[n]->pPopup) );
        }
        else if ( nId )
        {
            pSuper->InsertItem( nId, rCfg[n]->aTitle );
            if ( SfxMacroConfig::IsMacroSlot( nId ) )
            {
                SfxMacroInfo* pInfo = SFX_APP()->GetMacroConfig()->GetMacroInfo(nId);
                if ( pInfo )
                {
                    SFX_APP()->GetMacroConfig()->RegisterSlotId( nId );
                    pSuper->SetItemCommand( nId, pInfo->GetURL() );
                }
            }
            else if ( rCfg[n]->aCommand.Len() )
            {
                pSuper->SetItemCommand( nId, rCfg[n]->aCommand );
                pSuper->SetHelpId( nId, (ULONG) nId ); // Set help id even if we have a command
            }
            else
            {
                pSuper->SetHelpId( nId, (ULONG) nId );
            }
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

    rtl_TextEncoding nEnc = osl_getThreadTextEncoding();

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
                rStream.ReadByteString( aTitle, nEnc );

                String aHelpText;
                if ( bWithHelp )
                    rStream.ReadByteString( aHelpText, nEnc );

                if ( SfxMacroConfig::IsMacroSlot(nId) )
                {
                    SfxMacroInfo aInfo;
                    rStream >> aInfo;
//                    nId = aInfo.GetSlotId();
                }

                pSuper->InsertItem( nId, aTitle );
                pSuper->SetHelpId( nId, (ULONG) nId );
                if ( bWithHelp )
                    pSuper->SetHelpText( nId, aHelpText );

                if ( nId >= SID_OBJECTMENU0 && nId <= SID_OBJECTMENU_LAST )
                {
                    // our new menu configuration must have an empty popupmenu, iut doesn't allow for
                    // menu entries without popups in the menubar root
                    PopupMenu *pPopupMenu;
                    pPopupMenu = new PopupMenu;
                    pSuper->SetPopupMenu( nId, pPopupMenu );
                }

                break;
            }

            case 'P':
            {
                rStream >> nId;
                rStream.ReadByteString( aTitle, nEnc );

                if ( SfxMacroConfig::IsMacroSlot(nId) || nId < SID_SFX_START )
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
                    rStream.ReadByteString( aHelpText, nEnc );

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
                                const String &rHelpText, USHORT nId, const String &rCommand )
{
    SfxMenuCfgItem* pItem = new SfxMenuCfgItem;
    pItem->nId = nId;
    pItem->aTitle = rText;
    pItem->aHelpText = rHelpText;
    pItem->aCommand = rCommand;
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
        if ( pItem->pPopup && SfxMacroConfig::IsMacroSlot(pItem->nId) )
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

// executes the function for the selected item
extern Select_Impl( void*, void* ) ;

IMPL_LINK( SfxMenuManager, Select, Menu *, pMenu )
{
    DBG_MEMTEST();

    USHORT nId = (USHORT) pMenu->GetCurItemId();
    String aCommand = pMenu->GetItemCommand( nId );
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
    SfxDispatcher* pDisp = pBindings->GetDispatcher_Impl();
    SfxModule *pMod = pDisp ? SFX_APP()->GetActiveModule( pDisp->GetFrame() ) :0;
    return pIterator->IsBinding( pMod );
}

String SfxMenuManager::GetCommand() const
{
    DBG_ASSERT( pIterator, "invalid iterator state" );
    return pIterator->GetCommand();
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
    else if ( pIterator->GetCommand().Len() )
    {
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
    if ( bMenuBar )
        pSVMenu = new MenuBar;
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
        if ( pBar->GetWindow() && pBar->GetWindow()->GetMenuBar() == pOldBar )
            pBar->GetWindow()->SetMenuBar( pSvBar );
    }

    delete pOldMenu;
    pOldMenu = 0;
    pBindings->LEAVEREGISTRATIONS();
    SetDefault( FALSE );
}

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
    SfxVirtualMenu *pVMenu = new SfxVirtualMenu( pSVMenu, bWithHelp, *pBindings, bOLE );
    Construct(*pVMenu);

    if ( bMenuBar && pOldVirtMenu )
    {
        SfxMenuBarManager *pBar = (SfxMenuBarManager*) this;
        MenuBar* pOldBar = (MenuBar*) pOldVirtMenu->GetSVMenu();
        MenuBar* pSvBar = (MenuBar*) GetMenu()->GetSVMenu();
        if ( pBar->GetWindow()->GetMenuBar() == pOldBar )
            pBar->GetWindow()->SetMenuBar( pSvBar );
    }

    if ( pOldVirtMenu )
    {
        delete pOldVirtMenu;
        pBindings->LEAVEREGISTRATIONS();
    }
}

//--------------------------------------------------------------------

// reload all KeyCodes

void SfxMenuManager::InvalidateKeyCodes()
{
    if ( pMenu )
        pMenu->InvalidateKeyCodes();
}

// ------------------------------------------------------------------------

BOOL SfxMenuManager::IsPopupFunction( USHORT nId )
{
    return nId == SID_PICKLIST ||
           nId == SID_MDIWINDOWLIST ||
           nId == SID_HELPMENU ||
           nId == SID_ADDONLIST;
}

//====================================================================

// creates a menu-manager and loads it from resource RID_DEFAULTMENU or RID_DEFAULTPLUGINMENU
/*
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
*/
//--------------------------------------------------------------------

// creates a menu-manager and loads it from a resource

SfxMenuBarManager::SfxMenuBarManager( const ResId& rResId, SfxBindings &rBindings, SfxConfigManager* pMgr, BOOL bOLE )
    : SfxMenuManager( rResId, rBindings, pMgr, TRUE )
    , pWindow( rBindings.GetSystemWindow() )
    , bDowning( FALSE )
{
    DBG_MEMTEST();

    // initialize ObjectMenus
    for ( int n = 0; n < 4; ++n )
    {
        aObjMenus[n].nId = 0;
        aObjMenus[n].pPMMgr = NULL;
        aObjMenus[n].pResMgr = NULL;
    }

    // #107258# full menu bar hierarchy is needed, if accessibility is enabled
    BOOL bAccessibilityEnabled = Application::GetSettings().GetMiscSettings().GetEnableATToolSupport();
    SetForceCtrlCreateMode( bOLE || bAccessibilityEnabled );

    Initialize();
}

SfxMenuBarManager::SfxMenuBarManager( const SfxMenuBarManager& rOther, SfxConfigManager* pMgr )
    : SfxMenuManager( ResId( rOther.GetType(), rOther.GetResMgr() ), (SfxBindings&) rOther.GetBindings(), pMgr, TRUE )
    , pWindow( NULL )
    , bDowning( FALSE )
{
    DBG_MEMTEST();

    // initialize ObjectMenus
    for ( int n = 0; n < 4; ++n )
    {
        aObjMenus[n].nId =     rOther.aObjMenus[n].nId;
        aObjMenus[n].pPMMgr =  0;
        aObjMenus[n].pResMgr = rOther.aObjMenus[n].pResMgr;
    }

    Initialize();
}

SfxMenuBarManager::~SfxMenuBarManager()
{
    DBG_MEMTEST();

    DBG_ASSERT(!aObjMenus[0].pPMMgr || !aObjMenus[1].pPMMgr ||
               !aObjMenus[2].pPMMgr || !aObjMenus[3].pPMMgr,
               "da sind noch ObjectMenus" );

    bDowning = TRUE;
    ResetObjectMenus();
    UpdateObjectMenus();

    MenuBar* pSvBar = (MenuBar*) GetMenu()->GetSVMenu();
    if ( pWindow && pWindow->GetMenuBar() == pSvBar )
        pWindow->SetMenuBar( 0 );
}


/*
SvStream* SfxMenuBarManager::GetDefaultStream( StreamMode nMode )
{
    String aUserConfig = SvtPathOptions().GetUserConfigPath();
    INetURLObject aObj( aUserConfig );
    aObj.insertName( GetStreamName() );
    return ::utl::UcbStreamHelper::CreateStream( aObj.GetMainURL( INetURLObject::NO_DECODE ), nMode );
}

BOOL SfxMenuBarManager::Load( SvStream& rStream, BOOL bOLEServer )
{
    SetForceCtrlCreateMode( bOLEServer );
    Menu *pSVMenu = LoadMenuBar( rStream );
    if ( pSVMenu )
        Construct_Impl( pSVMenu, FALSE );
    return ( pSVMenu != NULL );
}
*/

// To be compatible to 6.0/src641 we have to erase .uno commands we got
// from resource file. Otherwise these commands get saved to our XML configurations
// files and 6.0/src641 is not able to map these to slot ids again!!!
void SfxMenuManager::EraseItemCmds( Menu* pMenu )
{
    USHORT nCount = pMenu->GetItemCount();
    for ( USHORT nSVPos = 0; nSVPos < nCount; nSVPos++ )
    {
        USHORT nId = pMenu->GetItemId( nSVPos );
        PopupMenu* pPopupMenu = pMenu->GetPopupMenu( nId );
        if ( pPopupMenu )
            EraseItemCmds( pPopupMenu );
        else if ( nId > 0 )
        {
            // Remove .uno commands to be compatible with 6.0/src641
            String aCommand = pMenu->GetItemCommand( nId );
            if ( aCommand.CompareToAscii(".uno:", 5 ) == COMPARE_EQUAL )
                pMenu->SetItemCommand( nId, String() );
        }
    }
}

// Restore the correct macro ID so that menu items with an associated accelerator
// are correctly identified. This ensures that the accelerator info is displayed on
// the popup.
void SfxMenuManager::RestoreMacroIDs( Menu* pMenu )
{
    USHORT nCount = pMenu->GetItemCount();
    for ( USHORT nSVPos = 0; nSVPos < nCount; nSVPos++ )
    {
        USHORT nId = pMenu->GetItemId( nSVPos );
        PopupMenu* pPopupMenu = pMenu->GetPopupMenu( nId );
        if ( pPopupMenu )
            RestoreMacroIDs( pPopupMenu );
        else if ( nId < SID_SFX_START )
        {
            // Restore a valid ID for macro commands!
            String aCommand = pMenu->GetItemCommand( nId );
            if ( aCommand.CompareToAscii("macro:", 6 ) == COMPARE_EQUAL )
            {
                String  aTitle      = pMenu->GetItemText( nId );
                String  aHelpText   = pMenu->GetHelpText( nId );
                ULONG   nHelpId     = pMenu->GetHelpId( nId );
                MenuItemBits nBits  = pMenu->GetItemBits( nId );

                SfxMacroInfo aInfo( aCommand );
                SFX_APP()->GetMacroConfig()->GetSlotId( &aInfo );
                nId = aInfo.GetSlotId();

                pMenu->RemoveItem( nSVPos );
                pMenu->InsertItem( nId, aTitle, nBits, nSVPos );
                pMenu->SetHelpText( nId, aHelpText );
                pMenu->SetHelpId( nId, nHelpId );
            }
        }
    }
}

MenuBar* SfxMenuBarManager::LoadMenuBar( SvStream& rStream )
{
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > xInputStream =
        new ::utl::OInputStreamWrapper( rStream );
    MenuBar *pSVMenu = NULL;
    try
    {
        ::com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory> aXMultiServiceFactory(::comphelper::getProcessServiceFactory());
        ::framework::MenuConfiguration aConfig( aXMultiServiceFactory );
        pSVMenu = aConfig.CreateMenuBarFromConfiguration( xInputStream );
        if ( pSVMenu )
            SfxMenuManager::RestoreMacroIDs( pSVMenu );
    }
    catch ( ::com::sun::star::lang::WrappedTargetException&  )
    {
    }

    return pSVMenu;
}

BOOL SfxMenuBarManager::StoreMenuBar( SvStream& rStream, MenuBar* pMenuBar )
{
    BOOL bRet = TRUE;
    ::utl::OOutputStreamWrapper* pHelper = new ::utl::OOutputStreamWrapper( rStream );
    com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > xOut( pHelper );
    try
    {
        ::com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory> aXMultiServiceFactory(::comphelper::getProcessServiceFactory());
        framework::MenuConfiguration aCfg( aXMultiServiceFactory );
        aCfg.StoreMenuBar( pMenuBar, xOut );
    }
    catch ( ::com::sun::star::lang::WrappedTargetException&  )
    {
        bRet = FALSE;
    }

    return bRet;
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

        // check if this object menue is in the menubar at all
        USHORT nId = n + SID_OBJECTMENU0;
        BOOL bInMenu = ( pMenu->GetItemPos( nId ) != MENU_ITEM_NOTFOUND );
        PopupMenu* pOldSVPopup = pMenu->GetPopupMenu( nId );

        if ( aObjMenus[n].nId )
        {
            if ( bInMenu )
            {
                if ( !pOldPopup && pOldSVPopup )
                {
                    // remove dummy popup
                    pMenu->SetPopupMenu( nId, 0 );
                    delete pOldSVPopup;
                }

                if ( !pOldPopup || ( pOldPopup && pOldPopup->GetType() != aObjMenus[n].nId ) )
                {
                    ResId aResId( aObjMenus[n].nId, aObjMenus[n].pResMgr );
                    aObjMenus[n].pPMMgr =
                        new SfxPopupMenuManager( aResId , GetBindings() );
                    aObjMenus[n].pPMMgr->Initialize();
                }
                else
                    // popups remains the same, prevent deletion
                    pOldPopup = NULL;

                pMenu->SetPopupMenu( nId, (PopupMenu*) aObjMenus[n].pPMMgr->GetMenu()->GetSVMenu() );
                pMenu->EnableItem( nId, TRUE );
            }
        }
        else if ( pOldPopup )
        {
            if ( pMenu->GetItemPos( nId ) != MENU_ITEM_NOTFOUND )
            {
//                pMenu->EnableItem( nId, FALSE );
                pMenu->SetPopupMenu( nId, 0 );
            }
        }

        if ( pOldPopup )
        {
            // no new popup menu created, so reset pointer before deleting menu
            if ( pOldPopup == aObjMenus[n].pPMMgr )
                aObjMenus[n].pPMMgr = NULL;
            pOldPopup->StoreConfig();
            delete pOldPopup;
        }

        if ( bDowning )
        {
            // delete possible empty popup menues
            PopupMenu *pPop = pMenu->GetPopupMenu( nId );
            if ( pPop )
            {
                pMenu->SetPopupMenu( nId, 0 );
                delete pPop;
            }
        }
        else if ( bInMenu )
        {
            // insert an empty popup window
            if ( !aObjMenus[n].pPMMgr && !pMenu->GetPopupMenu( nId ) )
                pMenu->SetPopupMenu( nId, new PopupMenu );
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
//====================================================================

// don't insert Popups into ConfigManager, they are not configurable at the moment !
SfxPopupMenuManager::SfxPopupMenuManager(const ResId& rResId, SfxBindings &rBindings )
    : SfxMenuManager( rResId, rBindings, NULL/*rBindings.GetConfigManager( rResId.GetId() )*/, FALSE )
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

IMPL_LINK_INLINE_START( SfxPopupMenuManager, SelectHdl, void *, pvoid )
{
    return 1;
}
IMPL_LINK_INLINE_END( SfxPopupMenuManager, SelectHdl, void *, pvoid )


//--------------------------------------------------------------------

USHORT SfxPopupMenuManager::Execute( const Point& rPoint, Window* pWindow, va_list pArgs, const SfxPoolItem *pArg1 )
{
    DBG_MEMTEST();

    PopupMenu* pMenu = ( (PopupMenu*)GetMenu()->GetSVMenu() );
    pMenu->SetSelectHdl( LINK( this, SfxPopupMenuManager, SelectHdl ) );
    USHORT nId = pMenu->Execute( pWindow, rPoint );
    pMenu->SetSelectHdl( Link() );

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
    if ( bMenuBar )
    {
        DBG_ERROR( "Not for menubars!" );
        return;
    }

    bAddClipboardFuncs = TRUE;
}

int SfxMenuManager::Load( SotStorage& rStorage )
{
    SotStorageStreamRef xStream = rStorage.OpenSotStream( SfxMenuManager::GetStreamName(), STREAM_STD_READ );
    if ( xStream->GetError() )
        return SfxConfigItem::ERR_READ;
    else
        return Load( *xStream );
}

BOOL SfxMenuManager::Store( SotStorage& rStorage )
{
    SotStorageStreamRef xStream = rStorage.OpenSotStream( SfxMenuManager::GetStreamName(), STREAM_STD_READWRITE|STREAM_TRUNC );
    if ( xStream->GetError() )
        return FALSE;
    else
        return Store( *xStream );
}

int SfxMenuBarManager::Load( SotStorage& rStorage )
{
    SotStorageStreamRef xStream = rStorage.OpenSotStream( SfxMenuManager::GetStreamName(), STREAM_STD_READ );
    if ( xStream->GetError() )
        return SfxConfigItem::ERR_READ;
    else
    {
        Menu *pSVMenu = LoadMenuBar( *xStream );
        if ( pSVMenu )
        {
            Construct_Impl( pSVMenu, FALSE );
            SetDefault( FALSE );
            return ERR_OK;
        }
        else
            UseDefault();
        return ERR_READ;
    }
}

BOOL SfxMenuBarManager::Store( SotStorage& rStorage )
{
    SotStorageStreamRef xStream = rStorage.OpenSotStream( SfxMenuManager::GetStreamName(), STREAM_STD_READWRITE|STREAM_TRUNC );
    if ( xStream->GetError() )
        return FALSE;
    else
        return StoreMenuBar( *xStream, (MenuBar*) GetMenu()->GetSVMenu() );
}

SfxMenuManager::SfxMenuManager( Menu* pMenu, SfxBindings &rBindings )
:   SfxConfigItem( 0, NULL ),
    bMenuBar(FALSE),
    pMenu(0),
    pOldMenu(0),
    pResMgr(NULL),
    pBindings(&rBindings)
{
    bOLE = FALSE;
    bAddClipboardFuncs = FALSE;
    SfxVirtualMenu* pVMenu = new SfxVirtualMenu( pMenu, FALSE, rBindings, TRUE, TRUE );
    Construct(*pVMenu);
}

SfxPopupMenuManager::SfxPopupMenuManager( PopupMenu* pMenu, SfxBindings& rBindings )
    : SfxMenuManager( pMenu, rBindings )
    , pSVMenu( pMenu )
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
        }
    }

    InsertVerbs_Impl( pFrame->GetViewShell()->GetVerbs(), pSVMenu );
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

BOOL SfxMenuBarManager::ReInitialize()
{
    BOOL bRet = SfxConfigItem::ReInitialize();
    if ( bRet )
        UpdateObjectMenus();
    return bRet;
}
