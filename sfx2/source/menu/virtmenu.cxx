/*************************************************************************
 *
 *  $RCSfile: virtmenu.cxx,v $
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

#include <sot/factory.hxx>
#include <vcl/system.hxx>
#include <svtools/libcall.hxx>
#pragma hdrstop

#include "virtmenu.hxx"
#include "msgpool.hxx"
#include "statcach.hxx"
#include "msg.hxx"
#include "idpool.hxx"
#include "mnuitem.hxx"
#include "mnumgr.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "app.hxx"
#include "accmgr.hxx"
#include "sfxtypes.hxx"
#include "arrdecl.hxx"
#include "sfx.hrc"
#include "ipfrm.hxx"
#include "ipenv.hxx"
#include "appdata.hxx"
#include "picklist.hxx"
#include "viewsh.hxx"

//=========================================================================

DBG_NAME(SfxVirtualMenu);

static long nAutoDeactivateTimeout_Impl = -1;

//=========================================================================

typedef SfxMenuControl* SfxMenuControlPtr;
SV_IMPL_PTRARR(SfxMenuCtrlArr_Impl, SfxMenuControlPtr);

//=========================================================================

/*  Diese Hilfsfunktion pr"uft, ob eine Slot-Id im aktuellen Applikations-
    Status sichtbar ist oder nicht. Dabei bezieht sich der Applikations-Status
    darauf, ob die Applikation OLE-Server ist oder nicht.
*/

BOOL IsItemHidden_Impl( USHORT nItemId, int bOleServer, int bMac )
{
    return ( bMac &&
             ( nItemId == SID_ARRANGEICONS || nItemId == SID_MINIMIZEWINS ) ) ||
           (  bOleServer &&
             ( nItemId == SID_QUITAPP || nItemId == SID_SAVEDOC ||
               nItemId == SID_OPENDOC || nItemId == SID_SAVEASDOC ||
               nItemId == SID_NEWDOC ) ) ||
           ( !bOleServer &&
             ( nItemId == SID_EXITANDRETURN || nItemId == SID_UPDATEDOC ) );
}

//====================================================================

void SfxVirtualMenu::Construct_Impl()
{
    pWindowMenu = NULL;
    pPickMenu = NULL;
    bIsActive = FALSE;
    bControllersUnBound = FALSE;
    pSVMenu->SetHighlightHdl( LINK(this, SfxVirtualMenu, Highlight) );
    pSVMenu->SetActivateHdl( LINK(this, SfxVirtualMenu, Activate) );
    pSVMenu->SetDeactivateHdl( LINK(this, SfxVirtualMenu, Deactivate) );
    pSVMenu->SetSelectHdl( LINK(this, SfxVirtualMenu, Select) );

    // Accels eintragen
    InvalidateKeyCodes();

    if ( !pResMgr && pParent )
        pResMgr = pParent->pResMgr;
}

//--------------------------------------------------------------------

SfxVirtualMenu::SfxVirtualMenu( USHORT nOwnId,
                SfxVirtualMenu* pOwnParent, Menu& rMenu, BOOL bWithHelp,
                SfxBindings &rBindings, BOOL bOLEServer, BOOL bRes ):
    pItems(0),
    pBindings(&rBindings),
    pResMgr(0),
    nLocks(0), pAutoDeactivate(0), bHelpInitialized( bWithHelp )
    , bRemoveDisabledEntries( FALSE )
{
    DBG_MEMTEST();
    DBG_CTOR(SfxVirtualMenu, 0);
    pSVMenu = &rMenu;

#ifdef UNX
    USHORT nPos = pSVMenu->GetItemPos( SID_DESKTOPMODE );
    if ( nPos != MENU_ITEM_NOTFOUND )
        pSVMenu->RemoveItem( nPos );
#endif

    bResCtor = bRes;
    bOLE = bOLEServer;
    nId = nOwnId;
    pParent = pOwnParent;
    nVisibleItems = 0;
    pAppCtrl = 0;
    CreateFromSVMenu();
    Construct_Impl();
    bHelpInitialized = FALSE;
}

//--------------------------------------------------------------------

// creates a virtual menu from a StarView MenuBar or PopupMenu

SfxVirtualMenu::SfxVirtualMenu( Menu *pStarViewMenu, BOOL bWithHelp,
                    SfxBindings &rBindings, BOOL bOLEServer, BOOL bRes ):
    pItems(0),
    pBindings(&rBindings),
    pResMgr(0),
    nLocks(0), pAutoDeactivate(0),  bHelpInitialized( bWithHelp )
    , bRemoveDisabledEntries( FALSE )
{
    DBG_MEMTEST();
    DBG_CTOR(SfxVirtualMenu, 0);

#ifdef UNX
    USHORT nPos = pStarViewMenu->GetItemPos( SID_DESKTOPMODE );
    if ( nPos != MENU_ITEM_NOTFOUND )
        pStarViewMenu->RemoveItem( nPos );
#endif

    pSVMenu = pStarViewMenu;

    bResCtor = bRes;
    bOLE = bOLEServer;
    nId = 0;
    pParent = 0;
    pAppCtrl = 0;
    nVisibleItems = 0;
    CreateFromSVMenu();
    Construct_Impl();
    bHelpInitialized = FALSE;
}

//--------------------------------------------------------------------

/*  Der Destruktor der Klasse SfxVirtualMenu gib die gebundenen Items frei
    und klinkt das zugeh"orige StarView-PopupMenu aus seinem Parent aus.
    Falls es sich um das Pickmenu oder das MDI-Menu handelt, wird es
    dort abgemeldet.
*/

SfxVirtualMenu::~SfxVirtualMenu()
{
    DBG_MEMTEST();
    DBG_DTOR(SfxVirtualMenu, 0);

    if ( bIsActive )
    {
        pBindings->LEAVEREGISTRATIONS(); --nLocks; bIsActive = FALSE;
    }

    // QAP-Hack
    if ( pAutoDeactivate )
    {
        if ( pAutoDeactivate->IsActive() )
            Deactivate(0);
        DELETEX(pAutoDeactivate);
    }

    if (pItems)
    {
#ifdef MPW
        // der MPW-Compiler ruft sonst keine Dtoren!
        for ( USHORT n = 0; n < nCount; ++n )
            (pItems+n)->SfxMenuControl::~SfxMenuControl();
        delete (void*) pItems;
#else
        __DELETE(nCount) pItems;
#endif
    }

    delete pAppCtrl;
    pBindings = 0;

    // Alle Menues, die von SV erzeugt wurden, werden auch dort wieder
    // gel"oscht (also die beim Laden aus der Resource erzeugten).
    // Das Top-Level-Menu wird nie von SV gel"oscht, da die Allocierung
    // im SFX erfolgt
    if ( !bResCtor || !pParent)
    {
        if ( pParent )
        {
            if( pParent->pSVMenu->GetItemPos( nId ) != MENU_ITEM_NOTFOUND )
                pParent->pSVMenu->SetPopupMenu( nId, 0 );
            if ( pParent->pPickMenu == pSVMenu )
                pParent->pPickMenu = 0;
            if ( pParent->pWindowMenu == pSVMenu)
                pParent->pWindowMenu = 0;
        }

        delete pSVMenu;
    }

    DBG_OUTF( ("SfxVirtualMenu %lx destroyed", this) );
    DBG_ASSERT( !nLocks, "destroying active menu" );
}
//--------------------------------------------------------------------

// internal: creates the virtual menu from the pSVMenu

void SfxVirtualMenu::CreateFromSVMenu()
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    // get and store the number of items
    nCount = pSVMenu->GetItemCount();

    // Achtung: nur zu diesem Zeitpunkt ist garantiert, da\s nCount und
    // der ItemCount des SV-Menues "ubereinstimmen; sp"ater kann das SvMenue
    // auch mehr Eintr"age haben (Pickliste!)
    if (nCount)
        pItems = new SfxMenuControl[nCount];

    // remember some values
    SfxApplication *pSfxApp = SFX_APP();
    const int bOleServer = FALSE;
    const int bMac = FALSE;

    // iterate through the items
    pBindings->ENTERREGISTRATIONS(); ++nLocks;
    USHORT nSVPos = 0;
    for ( USHORT nPos = 0; nPos < nCount; ++nPos, ++nSVPos )
    {
        USHORT nId = pSVMenu->GetItemId(nSVPos);
        PopupMenu* pPopup = pSVMenu->GetPopupMenu(nId);
        if ( pPopup )
        {
            DBG_ASSERT( SfxMenuManager::IsPopupFunction(nId) ||
                        !SFX_SLOTPOOL().GetIdPool().IsLocked(nId),
                        "popupmenu with existing func-id" );
            SfxMenuControl *pMnuCtrl =
                SfxMenuControl::CreateControl(nId, *pPopup, *pBindings);

            if ( pMnuCtrl )
            {
                // Das Popup war offensichtlich kein "echtes"; solche werden
                // niemals aus der Resource geladen und m"ussen daher explizit
                // gel"oscht werden
                if ( pSVMenu->GetPopupMenu( nId ) == pPopup )
                    pSVMenu->SetPopupMenu( nId, NULL );
                DELETEZ( pPopup );

                SfxMenuCtrlArr_Impl &rCtrlArr = GetAppCtrl_Impl();
                rCtrlArr.C40_INSERT( SfxMenuControl, pMnuCtrl, rCtrlArr.Count() );
                (pItems+nPos)->Bind( 0, nId, pSVMenu->GetItemText(nId),
                                    pSVMenu->GetHelpText(nId), *pBindings);
                pMnuCtrl->Bind( this, nId, pSVMenu->GetItemText(nId),
                                pSVMenu->GetHelpText(nId), *pBindings);
            }
            else
            {
/*
                if ( nId >= SID_SFX_START && !SfxMenuManager::IsPopupFunction(nId) )
                {
                    // Echte Popups sollen keine SlotIds haben; leider sind
                    // da noch Altlasten mit herumzuschleppen ...
                    String aTitle = pSVMenu->GetItemText( nId );
                    pSVMenu->SetPopupMenu( nId, NULL );
                    USHORT nPos = pSVMenu->GetItemPos( nId );
                    pSVMenu->RemoveItem( nPos );
                    nId = 1;
                    while ( pSVMenu->GetItemPos(nId) != MENU_ITEM_NOTFOUND )
                        nId++;
                    pSVMenu->InsertItem( nId, aTitle, 0, nPos );
                    pSVMenu->SetPopupMenu( nId, pPopup );
                }
*/

                pSVMenu->SetHelpId( nId, 0L );
                pMnuCtrl = pItems+nPos;

                // normalerweise jetzt erst im Activate-Handler
                if ( bOLE )
                {
                    pMnuCtrl->Bind( this, nId,
                        *new SfxVirtualMenu(nId, this, *pPopup, bHelpInitialized, *pBindings, bOLE, bResCtor),
                        pSVMenu->GetItemText(nId), pSVMenu->GetHelpText(nId),
                        *pBindings );
                }
            }

            ++nVisibleItems;
        }
        else
        {
            switch ( pSVMenu->GetItemType(nSVPos) )
            {
                case MENUITEM_STRING:
                {
                    SfxMenuControl *pMnuCtrl;
                    String aCmd( pSVMenu->GetItemCommand( nId ) );
                    if ( aCmd.Len() )
                    {
                        pMnuCtrl = SfxMenuControl::CreateControl( aCmd, nId,
                            *pSVMenu, *pBindings, this );
                        if ( pMnuCtrl )
                        {
                            SfxMenuCtrlArr_Impl &rCtrlArr = GetAppCtrl_Impl();
                            rCtrlArr.C40_INSERT( SfxMenuControl,
                                                pMnuCtrl, rCtrlArr.Count());
                            (pItems+nPos)->Bind( 0, nId, pSVMenu->GetItemText(nId),
                                            pSVMenu->GetHelpText(nId), *pBindings);
                        }
                        else
                        {
                            pMnuCtrl = (pItems+nPos);
                            pMnuCtrl->Bind( this, nId, pSVMenu->GetItemText(nId),
                                    pSVMenu->GetHelpText(nId), *pBindings);
                        }
                    }
                    else
                    {
                        pMnuCtrl = SfxMenuControl::CreateControl(nId,
                            *pSVMenu, *pBindings);

                        if ( pMnuCtrl )
                        {
                            SfxMenuCtrlArr_Impl &rCtrlArr = GetAppCtrl_Impl();
                            rCtrlArr.C40_INSERT( SfxMenuControl,
                                                    pMnuCtrl, rCtrlArr.Count());
                            (pItems+nPos)->Bind( 0, nId, pSVMenu->GetItemText(nId),
                                            pSVMenu->GetHelpText(nId), *pBindings);
                        }
                        else
                            pMnuCtrl = (pItems+nPos);

                        pMnuCtrl->Bind( this, nId, pSVMenu->GetItemText(nId),
                                    pSVMenu->GetHelpText(nId), *pBindings);
                    }

                    if ( !IsItemHidden_Impl(nId, bOleServer, bMac) )
                        ++nVisibleItems;
                    else
                        pSVMenu->RemoveItem( nSVPos-- );
                    break;
                }

                case MENUITEM_IMAGE:
                    //! not implemented
                    break;

                case MENUITEM_SEPARATOR:
                    //! not implemented
                    break;
            }
        }
    }
    pBindings->LEAVEREGISTRATIONS(); --nLocks;
}

//--------------------------------------------------------------------

// called on activation of the SV-Menu

IMPL_LINK( SfxVirtualMenu, Highlight, Menu *, pMenu )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    // eigenes StarView-Menu
    if ( pMenu == pSVMenu )
    {
        // AutoDeactivate ist jetzt nicht mehr n"otig
        USHORT nId = pMenu->GetCurItemId();
        if ( pAutoDeactivate )
            pAutoDeactivate->Stop();
    }

    return TRUE;
}
//--------------------------------------------------------------------

FASTBOOL SfxVirtualMenu::Bind_Impl( Menu *pMenu )
{
    // Selber suchen, da SV mit 'USHORT nSID = pSVMenu->GetCurItemId();' immer
    // 0 liefert. Das ist so, weil die Event-Weiterleitung lt. TH nichts mit
    // CurItem des Parent-Menus zu tun hat.
    for ( USHORT nPos = 0; nPos < nCount; ++nPos )
    {
        // angesprochenes Sub-Menu gefunden?
        FASTBOOL bFound = FALSE;
        USHORT nSID = pSVMenu->GetItemId(nPos);
        SfxMenuControl &rCtrl = pItems[nPos];
        bFound = pSVMenu->GetPopupMenu(nSID) == pMenu;
        SfxVirtualMenu *pSubMenu = rCtrl.GetPopupMenu();

        if ( bFound )
        {
            // Nur ein gebundener Menu-Controller hat schon seine Id!
            if ( !rCtrl.GetId() )
            {
                DBG_ASSERT( !pSubMenu, "Popup schon vorhanden!");

                // VirtualMenu f"ur Sub-Menu erzeugen
                BOOL bRes = bResCtor;
                SfxVirtualMenu *pSubMenu = new SfxVirtualMenu( nSID, this,
                        *pMenu, FALSE, *pBindings, bOLE, bRes);

                DBG_OUTF( ("Neues VirtualMenu %lx erzeugt", pSubMenu) );

                rCtrl.Bind( this, nSID, *pSubMenu,
                    pSVMenu->GetItemText(nSID), pSVMenu->GetHelpText(nSID),
                    *pBindings );

                // Activate weiterleiten
                pSubMenu->Bind_Impl( pMenu );
                pSubMenu->Activate( pMenu );
            }
        }

        // rekursiv weitersuchen (SV Activate nur am Menu selbst und Top-Menu)
        if ( !bFound && pSubMenu )
            bFound = pSubMenu->Bind_Impl( pMenu );

        // gefunden, dann abbrechen
        if ( bFound )
            return TRUE;
    }

    // nicht in diesem Untermenu gefunden
    return FALSE;
}

void SfxVirtualMenu::BindControllers()
{
    pBindings->ENTERREGISTRATIONS();

    USHORT nPos;
    for ( nPos = 0; nPos < nCount; ++nPos )
    {
        SfxMenuControl& rCtrl = pItems[nPos];
        if ( rCtrl.IsBindable_Impl() && !rCtrl.GetPopupMenu() )
            rCtrl.ReBind();
    }

    SfxMenuCtrlArr_Impl& rCtrlArr = GetAppCtrl_Impl();
    for ( nPos=0; nPos<rCtrlArr.Count(); nPos++ )
    {
        SfxMenuControl* pCtrl = rCtrlArr[nPos];
        USHORT nId = pCtrl->GetId();
        if ( !pSVMenu->GetItemCommand(nId).Len() )
            pCtrl->ReBind();
    }

    pBindings->LEAVEREGISTRATIONS();
    bControllersUnBound = FALSE;
}

void SfxVirtualMenu::UnbindControllers()
{
    pBindings->ENTERREGISTRATIONS();

    USHORT nPos;
    for ( nPos = 0; nPos < nCount; ++nPos )
    {
        SfxMenuControl &rCtrl = pItems[nPos];
        if ( rCtrl.IsBound() )
            rCtrl.UnBind();
    }

    SfxMenuCtrlArr_Impl& rCtrlArr = GetAppCtrl_Impl();
    for ( nPos=0; nPos<rCtrlArr.Count(); nPos++ )
    {
        SfxMenuControl* pCtrl = rCtrlArr[nPos];
        if ( pCtrl->IsBound() )
            // UnoController sind nicht gebunden!
            pCtrl->UnBind();
    }

    pBindings->LEAVEREGISTRATIONS();
    bControllersUnBound = TRUE;
}


//--------------------------------------------------------------------

// called on activation of the SV-Menu

#define ITEMID_MDIWINDOW    SHRT_MAX    // all values greater than maximum value for slotids

IMPL_LINK( SfxVirtualMenu, Activate, Menu *, pMenu )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);
    DBG_OUTF( ("SfxVirtualMenu %lx activated %lx, own %lx", this, pMenu, pSVMenu));

    // MI: wozu war der noch gut?
    // MBA: scheint ein alter QAP-Hack gewesen zu sein ( in rev.1.41 eingecheckt ! )
//  if ( Application::IsInModalMode() )
//      return TRUE; // abw"urgen

    if ( pMenu )
    {
        sal_Bool bDontHide = (sal_Bool)(sal_uInt16)
            SFX_INIMANAGER()->Get( SFX_KEY_DONTHIDE_DISABLEDENTRIES ).ToInt32();
        sal_uInt16 nFlag = pMenu->GetMenuFlags();
        if ( bDontHide )
            nFlag &= ~MENU_FLAG_HIDEDISABLEDENTRIES;
        else
            nFlag |= MENU_FLAG_HIDEDISABLEDENTRIES;
        pMenu->SetMenuFlags( nFlag );
    }

    // eigenes StarView-Menu
    if ( pMenu == pSVMenu )
    {
        // doppelt-Activate verhindern
        if ( bIsActive )
            return TRUE;

        // ggf. Pick-Menu erzeugen
        if ( pParent && pSVMenu == pParent->pPickMenu )
        {
            SfxPickList_Impl::Get()->SetPickMenu( pParent->pPickMenu );
        }
        else
            pPickMenu = pSVMenu->GetPopupMenu(SID_PICKLIST);

        if ( pParent && pSVMenu == pParent->pWindowMenu )
        {
            PopupMenu* pWindowMenu = pParent->pWindowMenu;
            sal_uInt16 nPos = pWindowMenu->GetItemPos( ITEMID_MDIWINDOW );
            for ( sal_uInt16 n=nPos; n<pWindowMenu->GetItemCount(); )
                pWindowMenu->RemoveItem( n );
            if ( pWindowMenu->GetItemType( pWindowMenu->GetItemCount()-1 ) == MENUITEM_SEPARATOR )
                pWindowMenu->RemoveItem( pWindowMenu->GetItemCount()-1 );

            SfxViewFrame *pView = pBindings->GetDispatcher()->GetFrame();
            while ( pView->GetParentViewFrame_Impl() )
                pView = pView->GetParentViewFrame_Impl();
            SfxFrame *pActive = pView->GetFrame();

            SfxFrameArr_Impl& rArr = *SFX_APP()->Get_Impl()->pTopFrames;
            if ( rArr.Count() > 0 &&
                 pWindowMenu->GetItemType( pWindowMenu->GetItemCount()-1 ) != MENUITEM_SEPARATOR )
                pWindowMenu->InsertSeparator();
            sal_uInt16 nNo;
            sal_uInt16 nAllowedMenuSize = USHRT_MAX - ITEMID_MDIWINDOW;
            for ( nNo = 0, nPos = 0; ( nPos < nAllowedMenuSize ) && ( nNo < rArr.Count() ); ++nNo )
            {
                SfxFrame *pFrame = rArr[nNo];
                if ( pFrame->GetCurrentViewFrame() && pFrame->GetCurrentViewFrame()->IsVisible() )
                {
                    pWindowMenu->InsertItem( ITEMID_MDIWINDOW + nNo,
                                             pFrame->GetWindow().GetText(), MIB_RADIOCHECK );
                    if ( pFrame == pActive )
                        pWindowMenu->CheckItem( ITEMID_MDIWINDOW + nNo, sal_True );
                    nPos++;
                }
            }
        }
        else
            pWindowMenu = pSVMenu->GetPopupMenu(SID_MDIWINDOWLIST);

        // f"ur konstistenten Status sorgen
        if ( bControllersUnBound )
            BindControllers();

        pBindings->GetDispatcher_Impl()->Flush();
        for ( USHORT nPos = 0; nPos < nCount; ++nPos )
        {
            USHORT nId = (pItems+nPos)->GetId();
            if ( nId && nId < ITEMID_MDIWINDOW )
                pBindings->Update(nId);
        }

        if ( bRemoveDisabledEntries )
            RemoveDisabledEntries();

        // HelpText on-demand
        if ( !bHelpInitialized )
        {
            bHelpInitialized = TRUE;
            SfxSlotPool &rSlotPool = SFX_SLOTPOOL();
            for ( USHORT nPos = 0; nPos < pMenu->GetItemCount(); ++nPos )
            {
                USHORT nId = pMenu->GetItemId(nPos);
                pMenu->SetHelpText( nId, rSlotPool.GetSlotHelpText_Impl(nId) );
            }
        }

        // bis zum Deactivate die Statusupdates unterdr"ucken
        pBindings->ENTERREGISTRATIONS(); ++nLocks; bIsActive = TRUE;

        if ( pAutoDeactivate ) // QAP-Hack
            pAutoDeactivate->Start();

        // erledigt
        return TRUE;
    }
    else
    {
        // VirtualMenu fuer SubMenu finden und ggf. an VirtualMenu binden
        BOOL bRet = Bind_Impl( pMenu );
#ifdef DBG_UTIL
        if ( !bRet)
            DBG_WARNING( "W1: Virtual menu konnte nicht erzeugt werden!" );
#endif
        return bRet;
    }

    return FALSE; // dead code for WTC
}

//--------------------------------------------------------------------

IMPL_LINK( SfxVirtualMenu, Deactivate, Menu *, pMenu )
{
    DBG_MEMTEST();
    DBG_OUTF( ("SfxVirtualMenu %lx deactivated %lx, own %lx", this, pMenu, pSVMenu) );
    if ( bIsActive && ( 0 == pMenu || pMenu == pSVMenu ) )
    {
        if ( pAutoDeactivate )
            pAutoDeactivate->Stop();

        // Bis auf die Menubar k"onnen alle Controller unbinded werden, wenn
        // das Menue deaktiviert ( = zugeklappt ) wird
        if ( pParent )
            UnbindControllers();
        pBindings->LEAVEREGISTRATIONS(); --nLocks; bIsActive = FALSE;
    }
    return TRUE;
}
//--------------------------------------------------------------------

// called on activation of the SV-Menu

IMPL_LINK( SfxVirtualMenu, Select, Menu *, pMenu )
{
    USHORT nId = (USHORT) pMenu->GetCurItemId();
    DBG_OUTF( ("SfxVirtualMenu %lx selected %u from %lx", this, nId, pMenu) );

    if ( pSVMenu->GetItemCommand( nId ).Len() )
    {
        SfxMenuCtrlArr_Impl& rCtrlArr = GetAppCtrl_Impl();
        for ( USHORT nPos=0; nPos<rCtrlArr.Count(); nPos++ )
        {
            SfxMenuControl* pCtrl = rCtrlArr[nPos];
            if ( pCtrl->GetId() == nId )
            {
                SfxUnoMenuControl *pUnoCtrl = (SfxUnoMenuControl*) pCtrl;
                pUnoCtrl->Select();
                return TRUE;
            }
        }
    }

    if ( nId >= ITEMID_MDIWINDOW )
    {
        SfxFrameArr_Impl& rArr = *SFX_APP()->Get_Impl()->pTopFrames;
        sal_uInt16 nWindowId = nId - ITEMID_MDIWINDOW;
        if ( nWindowId < rArr.Count() )
        {
            SfxFrame *pFrame = rArr[ nWindowId ];
            SfxViewFrame *pView = pFrame->GetCurrentViewFrame();
            pView->MakeActive_Impl( TRUE );
            return sal_True;
        }
    }

    pBindings->Execute( nId );

    return TRUE;
}
//--------------------------------------------------------------------

// return the position of the specified item

USHORT SfxVirtualMenu::GetItemPos( USHORT nItemId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    for ( USHORT nPos = 0; nPos < nCount; ++nPos )
        if ( (pItems+nPos)->GetId() == nItemId )
            return nPos;
    return MENU_ITEM_NOTFOUND;
}
//--------------------------------------------------------------------

// returns the associated StarView-menu

Menu* SfxVirtualMenu::GetSVMenu() const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    return pSVMenu;
}

//--------------------------------------------------------------------

// returns the popup-menu assigned to the item or 0 if none

SfxVirtualMenu* SfxVirtualMenu::GetPopupMenu( USHORT nId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    USHORT nPos = GetItemPos(nId);
    if ( nPos != MENU_ITEM_NOTFOUND )
        return (pItems+nPos)->GetPopupMenu();
    return 0;
}
//--------------------------------------------------------------------

// returns the text of the item as currently shown in the menu

String SfxVirtualMenu::GetItemText( USHORT nId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    USHORT nPos = GetItemPos(nId);
    if ( nPos != MENU_ITEM_NOTFOUND )
        return (pItems+nPos)->GetTitle();
    return String();
}
//--------------------------------------------------------------------

// returns the text of the item as currently shown in the menu

String SfxVirtualMenu::GetItemHelpText( USHORT nId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    USHORT nPos = GetItemPos(nId);
    if ( nPos != MENU_ITEM_NOTFOUND )
        return (pItems+nPos)->GetHelpText();
    return String();
}

//--------------------------------------------------------------------

void SfxVirtualMenu::InvalidateKeyCodes()
{
    DBG_ASSERT( pSVMenu, "invalidating key of incomplete menu" );

    SfxApplication* pSfxApp = SFX_APP();
    SfxViewFrame *pViewFrame = pSfxApp->GetViewFrame();
    SfxAcceleratorManager* pAccMgr = pViewFrame->GetViewShell()->GetAccMgr_Impl();
    if ( !pAccMgr )
        return;

    SfxAcceleratorManager* pAppAccel = pSfxApp->GetAppAccel_Impl();
    for ( USHORT nPos = 0; nPos < pSVMenu->GetItemCount(); ++nPos )
    {
        USHORT nId = pSVMenu->GetItemId(nPos);
        SfxVirtualMenu *pPopup = GetPopupMenu(nId);
        if ( pPopup )
            pPopup->InvalidateKeyCodes();
        else if ( nId )
        {
            KeyCode aCode = pAccMgr->GetKeyCode( nId );
            if ( !aCode.GetCode() && pAccMgr != pAppAccel )
                aCode = pAppAccel->GetKeyCode( nId );
            pSVMenu->SetAccelKey( nId, aCode );
        }
    }
}

//--------------------------------------------------------------------

// set the checkmark of the specified item

void SfxVirtualMenu::CheckItem( USHORT nItemId, BOOL bCheck )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);
    DBG_ASSERT( this != 0, "");
    DBG_ASSERT( pSVMenu != 0, "" );
    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        pSVMenu->CheckItem( nItemId, bCheck );
}
//--------------------------------------------------------------------

// set the enabled-state of the specified item

void SfxVirtualMenu::EnableItem( USHORT nItemId, BOOL bEnable )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);
    DBG_ASSERT( this != 0, "");
    DBG_ASSERT( pSVMenu != 0, "" );

    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        pSVMenu->EnableItem( nItemId, bEnable );
}
//--------------------------------------------------------------------

// set the text of the specified item

void SfxVirtualMenu::SetItemText( USHORT nItemId, const String& rText )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);
    DBG_ASSERT( this != 0, "");
    DBG_ASSERT( pSVMenu != 0, "" );
    if (pSVMenu->GetItemPos( nItemId ) != MENU_ITEM_NOTFOUND )
        pSVMenu->SetItemText( nItemId, rText );
}

//--------------------------------------------------------------------

//

void SfxVirtualMenu::SetPopupMenu( USHORT nId, PopupMenu *pMenu )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    if (pSVMenu->GetItemPos( nId ) != MENU_ITEM_NOTFOUND )
        GetSVMenu()->SetPopupMenu( nId, pMenu );
    for ( USHORT n = 0; n < nCount; ++n )
    {
        SfxVirtualMenu *pSubMenu = (pItems+n)->GetPopupMenu();
        if ( pSubMenu )
            pSubMenu->SetPopupMenu( nId, pMenu );
    }
}

//--------------------------------------------------------------------

// Erzwingt die Initialisierung, die sonst nur im Activate kommt

void SfxVirtualMenu::InitPopup( USHORT nPos, BOOL bOLE )
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxVirtualMenu, 0);

    USHORT nSID = pSVMenu->GetItemId(nPos);
    PopupMenu *pMenu = pSVMenu->GetPopupMenu( nSID );

    DBG_ASSERT( pMenu, "Hier gibt es kein Popup!");

    SfxMenuControl &rCtrl = pItems[nPos];
    if ( !rCtrl.GetId() )
    {
        // VirtualMenu f"ur Sub-Menu erzeugen
        BOOL bRes = bResCtor;
        SfxVirtualMenu *pSubMenu =
            new SfxVirtualMenu(nSID, this, *pMenu, FALSE, *pBindings, bOLE, bRes);

        DBG_OUTF( ("Neues VirtualMenu %lx erzeugt", pSubMenu) );

        rCtrl.Bind( this, nSID, *pSubMenu,
            pSVMenu->GetItemText(nSID), pSVMenu->GetHelpText(nSID),
            *pBindings );
    }
}

void SfxVirtualMenu::InitializeHelp()
{
    SfxSlotPool &rSlotPool = SFX_SLOTPOOL();
    for ( USHORT nPos = 0; nPos<pSVMenu->GetItemCount(); ++nPos )
    {
        USHORT nId = pSVMenu->GetItemId(nPos);
        if ( !bHelpInitialized )
            pSVMenu->SetHelpText( nId, rSlotPool.GetSlotHelpText_Impl( nId ) );
        SfxMenuControl &rCtrl = pItems[nPos];
        if ( nId && !rCtrl.GetId() )
        {
            InitPopup( nPos, TRUE );
        }

        SfxVirtualMenu *pSubMenu = rCtrl.GetPopupMenu();
        if ( pSubMenu )
            pSubMenu->InitializeHelp();
    }

    bHelpInitialized = TRUE;
}

typedef ULONG (__LOADONCALLAPI *HelpIdFunc) ( const String& );

void SfxVirtualMenu::SetHelpIds( ResMgr *pRes )
{
    pResMgr = pRes;
/*  SvLibrary aLib( SVLIBRARY( "ss" ) );
    if ( aLib.ModulFound() )
    {
        HelpIdFunc pFunc = (HelpIdFunc) aLib.GetFunction( DEFINE_CONST_UNICODE("GetHelpId") );
        if ( pFunc )
        {
            for ( USHORT nPos=0; nPos<pSVMenu->GetItemCount(); nPos++ )
            {
                USHORT nId = pSVMenu->GetItemId( nPos );
                SfxVirtualMenu *pPopup = GetPopupMenu(nId);
                if ( pPopup )
                    pPopup->SetHelpIds( pResMgr );
                else
                {
                    USHORT nHelpId = (*pFunc)( pSVMenu->GetItemCommand( nId ) );
                    pSVMenu->SetHelpId( nId, nHelpId );
                }
            }
        }
    }*/
}

void SfxVirtualMenu::SetRemoveDisabledEntries()
{
    bRemoveDisabledEntries = TRUE;
}

void SfxVirtualMenu::RemoveDisabledEntries()
{
    pBindings->ENTERREGISTRATIONS();

    // Zuerst die Spezial-Controller
    SfxMenuCtrlArr_Impl& rCtrlArr = GetAppCtrl_Impl();
    USHORT nPos;
    for ( nPos=0; nPos<rCtrlArr.Count(); nPos++ )
    {
        SfxMenuControl* pCtrl = rCtrlArr[nPos];
        USHORT nId = pCtrl->GetId();
        if ( !pSVMenu->IsItemEnabled( nId ) && pCtrl->IsBound() )
            pCtrl->UnBind();
    }

    for ( nPos = 0; nPos<nCount; ++nPos )
    {
        SfxMenuControl &rCtrl = pItems[nPos];
        USHORT nId = rCtrl.GetId();
        if ( nId && !pSVMenu->IsItemEnabled( nId ) && rCtrl.IsBound() )
            rCtrl.UnBind();

        SfxVirtualMenu *pPopup = rCtrl.GetPopupMenu();
        if ( pPopup )
        {
            pPopup->RemoveDisabledEntries();
            if ( !pPopup->pSVMenu->GetItemCount() )
            {
                rCtrl.RemovePopup();
                pSVMenu->RemoveItem( pSVMenu->GetItemPos( nId ) );
            }
        }
    }

    pSVMenu->RemoveDisabledEntries( FALSE, FALSE );
    pBindings->LEAVEREGISTRATIONS();
}


