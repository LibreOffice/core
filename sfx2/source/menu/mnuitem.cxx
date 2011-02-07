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

#ifdef SOLARIS
// HACK: prevent conflict between STLPORT and Workshop headers on Solaris 8
#include <ctime>
#endif

#include <string> // HACK: prevent conflict between STLPORT and Workshop includes
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <com/sun/star/util/XURLTransformer.hpp>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <tools/urlobj.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/menuoptions.hxx>
#include <framework/menuconfiguration.hxx>
#ifndef GCC
#endif

#include <sfx2/app.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/msgpool.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "idpool.hxx"
#include "sfxtypes.hxx"
#include "virtmenu.hxx"
#include <sfx2/mnuitem.hxx>
#include <sfx2/tbxctrl.hxx>
#include "arrdecl.hxx"
#include <sfx2/module.hxx>
#include <sfx2/unoctitm.hxx>
#include <sfx2/viewfrm.hxx>
#include "sfx2/imgmgr.hxx"
#include "sfx2/imagemgr.hxx"
#include "sfx2/sfxresid.hxx"
#include "../doc/doc.hrc"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;

//====================================================================

class SfxEnumMenu: public PopupMenu
{
    sal_uInt16          nSlot;
    SfxEnumItem    *pItem;
    SfxBindings*    pBindings;

protected:
    virtual void    Select();

public:
                    SfxEnumMenu( sal_uInt16 nSlot, SfxBindings* pBind, const SfxEnumItem &rItem );
                    ~SfxEnumMenu();
};

//=========================================================================

SfxEnumMenu::SfxEnumMenu( sal_uInt16 nSlotId, SfxBindings* pBind, const SfxEnumItem &rItem ):
    nSlot( nSlotId ),
    pItem( (SfxEnumItem*) rItem.Clone() ),
    pBindings( pBind )
{
    for ( sal_uInt16 nVal = 0; nVal < pItem->GetValueCount(); ++nVal )
        InsertItem( nVal+1, pItem->GetValueTextByPos(nVal) );
    CheckItem( pItem->GetValue() + 1, sal_True );
}

//-------------------------------------------------------------------------

SfxEnumMenu::~SfxEnumMenu()
{
    delete pItem;
}

//-------------------------------------------------------------------------

void SfxEnumMenu::Select()
{
    pItem->SetValue( GetCurItemId()-1 );
    pBindings->GetDispatcher()->Execute( nSlot,
                SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD,
                pItem, 0L, 0L );
}

//--------------------------------------------------------------------

void SfxMenuControl::SetOwnMenu( SfxVirtualMenu* pMenu )
{
    pOwnMenu = pMenu;
    if ( pSubMenu )
        pSubMenu->SetParentMenu( pMenu );
}


//--------------------------------------------------------------------

// binds the instance to the specified id and assignes the title

void SfxMenuControl::Bind(
    SfxVirtualMenu* pOwn,
    sal_uInt16 nSlotId,
    const String& rTitle,
    const String &rHelpText,
    SfxBindings &rBindings )
{
    DBG_MEMTEST();

    aTitle = rTitle;
    aHelpText = rHelpText;
    pOwnMenu = pOwn;
    pSubMenu = 0;
    if ( pOwn )
        SfxControllerItem::Bind(nSlotId, &rBindings);
    else
        SetId( nSlotId );

    DBG( CheckConfigure_Impl(SFX_SLOT_MENUCONFIG) );
}


//--------------------------------------------------------------------

// binds the item to the specified menu and assignes the title

void SfxMenuControl::Bind(
    SfxVirtualMenu* pOwn,
    sal_uInt16 nSlotId,
    SfxVirtualMenu& rMenu,
    const String& rTitle,
    const String &rHelpText,
    SfxBindings &rBindings )
{
    DBG_MEMTEST();
    SetId( nSlotId );
    SetBindings(rBindings);
    pOwnMenu = pOwn;
    pSubMenu = &rMenu;
    aTitle = rTitle;
    aHelpText = rHelpText;
}

//--------------------------------------------------------------------

// ctor for explicit registration

SfxMenuControl::SfxMenuControl( sal_Bool bShowStrings )
:   pOwnMenu(0),
    pSubMenu(0),
    b_ShowStrings(bShowStrings)
{
    DBG_MEMTEST();
}

//--------------------------------------------------------------------

// ctor for array

SfxMenuControl::SfxMenuControl():
    pOwnMenu(0),
    pSubMenu(0),
    b_ShowStrings(sal_False)
{
    DBG_MEMTEST();
}

//--------------------------------------------------------------------

SfxMenuControl::SfxMenuControl(sal_uInt16 nSlotId, SfxBindings& rBindings):
    SfxControllerItem(nSlotId, rBindings),
    pOwnMenu(0),
    pSubMenu(0),
    b_ShowStrings(sal_False)
{
    DBG_MEMTEST();

    // Dieser Ctor soll es erm"oglichen, w"ahrend der Konstruktion schon
    // auf die Bindings zur"uckgreifen zu k"onnen, aber gebunden wird
    // wie immer erst sp"ater. Anwendung z.B. wenn im ctor der abgeleiteten
    // Klasse z.B. ein StatusForwarder erzeugt werden soll.
    UnBind();
}


//--------------------------------------------------------------------

// dtor

SfxMenuControl::~SfxMenuControl()
{
    delete pSubMenu;
}

void SfxMenuControl::RemovePopup()
{
    DELETEZ( pSubMenu );
}

//--------------------------------------------------------------------

// changes the state in the virtual menu

void SfxMenuControl::StateChanged
(
    sal_uInt16              nSID,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    (void)nSID; //unused
    DBG_MEMTEST();
    DBG_ASSERT( nSID == GetId(), "strange SID" );
    DBG_ASSERT( pOwnMenu != 0, "setting state to dangling SfxMenuControl" );

    bool bIsObjMenu =
                GetId() >= SID_OBJECTMENU0 && GetId() < SID_OBJECTMENU_LAST;

    // enabled/disabled-Flag pauschal korrigieren

#ifdef UNIX
    if (nSID == SID_PASTE)
        pOwnMenu->EnableItem( GetId(), sal_True );
    else
#endif
    pOwnMenu->EnableItem( GetId(), bIsObjMenu
                ? 0 != pOwnMenu->GetSVMenu()->GetPopupMenu( GetId() )
                : eState != SFX_ITEM_DISABLED );

    if ( eState != SFX_ITEM_AVAILABLE )
    {
        // checken nur bei nicht-Object-Menus
        if ( !bIsObjMenu )
            pOwnMenu->CheckItem( GetId(), sal_False );

        // SetItemText flackert in MenuBar insbes. unter OS/2 (Bug #20658)
        if ( // !bIsObjMenu && nicht wegen "Format/Datenbank"
             pOwnMenu->GetSVMenu()->GetItemText( GetId() ) != GetTitle() )
        {
             DBG_WARNING("Title of menu item changed - please check if this needs correction!");
            // pOwnMenu->SetItemText( GetId(), GetTitle() );
        }
        return;
    }

    // ggf. das alte Enum-Menu entfernen/loeschen
    //! delete pOwnMenu->GetMenu().ChangePopupMenu( GetId(), 0 );

    bool bCheck = false;
    if ( pState->ISA(SfxBoolItem) )
    {
        // BoolItem fuer checken
        DBG_ASSERT( GetId() < SID_OBJECTMENU0 || GetId() > SID_OBJECTMENU_LAST,
                    "SfxBoolItem not allowed for SID_OBJECTMENUx" );
        bCheck = ((const SfxBoolItem*)pState)->GetValue();
    }
    else if ( pState->ISA(SfxEnumItemInterface) &&
              ((SfxEnumItemInterface *)pState)->HasBoolValue() )
    {
        // EnumItem wie Bool behandeln
        DBG_ASSERT( GetId() < SID_OBJECTMENU0 || GetId() > SID_OBJECTMENU_LAST,
                    "SfxEnumItem not allowed for SID_OBJECTMENUx" );
        bCheck = ((SfxEnumItemInterface *)pState)->GetBoolValue();
    }
    else if ( ( b_ShowStrings || bIsObjMenu ) && pState->ISA(SfxStringItem) )
    {
        // MenuText aus SfxStringItem holen
        String aStr( ((const SfxStringItem*)pState)->GetValue() );
        if ( aStr.CompareToAscii("($1)",4) == COMPARE_EQUAL )
        {
            String aEntry(SfxResId(STR_UPDATEDOC));
            aEntry += ' ';
            aEntry += aStr.Copy(4);
            aStr = aEntry;
        }
        else if ( aStr.CompareToAscii("($2)",4) == COMPARE_EQUAL )
        {
            String aEntry(SfxResId(STR_CLOSEDOC_ANDRETURN));
            aEntry += aStr.Copy(4);
            aStr = aEntry;
        }

        pOwnMenu->SetItemText( GetId(), aStr );
    }

#ifdef enum_item_menu_ok
    else if ( aType == TYPE(SfxEnumItem) )
    {
        DBG_ASSERT( GetId() < SID_OBJECTMENU0 || GetId() > SID_OBJECTMENU_LAST,
                    "SfxEnumItem not allowed for SID_OBJECTMENUx" );
        pOwnMenu->GetMenu().ChangePopupMenu( GetId(), &GetBindings(),
            new SfxEnumMenu( GetId(), *(const SfxEnumItem*)pState ) );
    }
#endif

    pOwnMenu->CheckItem( GetId(), bCheck );
}

//--------------------------------------------------------------------

SfxMenuControl* SfxMenuControl::CreateImpl( sal_uInt16 /*nId*/, Menu& /*rMenu*/, SfxBindings& /*rBindings*/ )
{
    return new SfxMenuControl( sal_True );
}

//--------------------------------------------------------------------

void SfxMenuControl::RegisterControl( sal_uInt16 nSlotId, SfxModule *pMod )
{
    RegisterMenuControl( pMod, new SfxMenuCtrlFactory(
                SfxMenuControl::CreateImpl, TYPE(SfxStringItem), nSlotId ) );
}

//--------------------------------------------------------------------
void SfxMenuControl::RegisterMenuControl(SfxModule* pMod, SfxMenuCtrlFactory* pFact)
{
    SFX_APP()->RegisterMenuControl_Impl( pMod, pFact );
}

SfxMenuControl* SfxMenuControl::CreateControl( sal_uInt16 nId, Menu &rMenu, SfxBindings &rBindings )
{
    TypeId aSlotType = SFX_SLOTPOOL().GetSlotType(nId);
    if ( aSlotType )
    {
        SfxApplication *pApp = SFX_APP();
        SfxDispatcher *pDisp = rBindings.GetDispatcher_Impl();
        SfxModule *pMod = pDisp ? SfxModule::GetActiveModule( pDisp->GetFrame() ) :0;
        if ( pMod )
        {
            SfxMenuCtrlFactArr_Impl *pFactories = pMod->GetMenuCtrlFactories_Impl();
            if ( pFactories )
            {
                SfxMenuCtrlFactArr_Impl &rFactories = *pFactories;
                for ( sal_uInt16 nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
                    if ( rFactories[nFactory]->nTypeId == aSlotType &&
                         ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                           ( rFactories[nFactory]->nSlotId == nId) ) )
                        return rFactories[nFactory]->pCtor( nId, rMenu, rBindings );
            }
        }

        SfxMenuCtrlFactArr_Impl &rFactories = pApp->GetMenuCtrlFactories_Impl();

        for ( sal_uInt16 nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
            if ( rFactories[nFactory]->nTypeId == aSlotType &&
                 ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                   ( rFactories[nFactory]->nSlotId == nId) ) )
                return rFactories[nFactory]->pCtor( nId, rMenu, rBindings );
    }
    return 0;
}

sal_Bool SfxMenuControl::IsSpecialControl( sal_uInt16 nId, SfxModule* pMod  )
{
    TypeId aSlotType = SFX_SLOTPOOL().GetSlotType( nId );
    if ( aSlotType )
    {
        if ( pMod )
        {
            SfxMenuCtrlFactArr_Impl *pFactories = pMod->GetMenuCtrlFactories_Impl();
            if ( pFactories )
            {
                SfxMenuCtrlFactArr_Impl &rFactories = *pFactories;
                for ( sal_uInt16 nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
                    if ( rFactories[nFactory]->nTypeId == aSlotType &&
                         ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                           ( rFactories[nFactory]->nSlotId == nId) ) )
                        return sal_True;
            }
        }

        SfxMenuCtrlFactArr_Impl &rFactories = SFX_APP()->GetMenuCtrlFactories_Impl();

        for ( sal_uInt16 nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
            if ( rFactories[nFactory]->nTypeId == aSlotType &&
                 ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                   ( rFactories[nFactory]->nSlotId == nId) ) )
                return sal_True;
    }
    return 0;
}

//--------------------------------------------------------------------

PopupMenu* SfxMenuControl::GetPopup () const
{
    if (GetPopupMenu())
        return (PopupMenu*)GetPopupMenu()->GetSVMenu();
    else
        return 0;
}

long Select_Impl( void* pHdl, void* pVoid );

SFX_IMPL_MENU_CONTROL( SfxAppMenuControl_Impl, SfxStringItem );

SfxAppMenuControl_Impl::SfxAppMenuControl_Impl(
    sal_uInt16 nPos, Menu& rMenu, SfxBindings& rBindings )
    : SfxMenuControl( nPos, rBindings ), pMenu(0)
{
    String aText = rMenu.GetItemText( nPos );

    // Determine the current background color setting for menus
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_nSymbolsStyle         = rSettings.GetSymbolsStyle();
    m_bWasHiContrastMode    = rSettings.GetHighContrastMode();
    m_bShowMenuImages       = rSettings.GetUseImagesInMenus();

    Reference<com::sun::star::lang::XMultiServiceFactory> aXMultiServiceFactory(::comphelper::getProcessServiceFactory());
    ::framework::MenuConfiguration aConf( aXMultiServiceFactory );
    Reference<com::sun::star::frame::XFrame> aXFrame( GetBindings().GetDispatcher_Impl()->GetFrame()->GetFrame().GetFrameInterface() );
    pMenu = aConf.CreateBookmarkMenu( aXFrame, GetId() == SID_NEWDOCDIRECT ? BOOKMARK_NEWMENU : BOOKMARK_WIZARDMENU );
    if( pMenu )
    {
        pMenu->SetSelectHdl( Link( &(this->GetBindings()), Select_Impl ) );
        pMenu->SetActivateHdl( LINK(this, SfxAppMenuControl_Impl, Activate) );
        rMenu.SetPopupMenu( nPos, pMenu );
    }
}

SfxAppMenuControl_Impl::~SfxAppMenuControl_Impl()
{
    delete pMenu;
}

IMPL_LINK( SfxAppMenuControl_Impl, Activate, Menu *, pActMenu )
{
    if ( pActMenu )
    {
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        sal_uIntPtr nSymbolsStyle = rSettings.GetSymbolsStyle();
        sal_Bool bIsHiContrastMode = rSettings.GetHighContrastMode();
        sal_Bool bShowMenuImages = rSettings.GetUseImagesInMenus();

        if (( nSymbolsStyle != m_nSymbolsStyle ) ||
            ( bIsHiContrastMode != m_bWasHiContrastMode ) ||
            ( bShowMenuImages != m_bShowMenuImages ))
        {
            m_nSymbolsStyle         = nSymbolsStyle;
            m_bWasHiContrastMode    = bIsHiContrastMode;
            m_bShowMenuImages       = bShowMenuImages;

            sal_uInt16 nCount = pActMenu->GetItemCount();
            for ( sal_uInt16 nSVPos = 0; nSVPos < nCount; nSVPos++ )
            {
                sal_uInt16 nItemId = pActMenu->GetItemId( nSVPos );
                if ( pActMenu->GetItemType( nSVPos ) != MENUITEM_SEPARATOR )
                {
                    if ( bShowMenuImages )
                    {
                        sal_Bool        bImageSet = sal_False;
                        ::rtl::OUString aImageId;
                        ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                            (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( nItemId );

                        if ( pMenuAttributes )
                            aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                        if ( aImageId.getLength() > 0 )
                        {
                            Reference< ::com::sun::star::frame::XFrame > xFrame;
                            Image aImage = GetImage( xFrame, aImageId, sal_False, bIsHiContrastMode );
                            if ( !!aImage )
                            {
                                bImageSet = sal_True;
                                pActMenu->SetItemImage( nItemId, aImage );
                            }
                        }

                        String aCmd( pActMenu->GetItemCommand( nItemId ) );
                        if ( !bImageSet && aCmd.Len() )
                        {
                            Image aImage = SvFileInformationManager::GetImage(
                                INetURLObject(aCmd), sal_False, bIsHiContrastMode );
                            if ( !!aImage )
                                pActMenu->SetItemImage( nItemId, aImage );
                        }
                    }
                    else
                        pActMenu->SetItemImage( nItemId, Image() );
                }
            }
        }

        return sal_True;
    }

    return sal_False;
}

SfxUnoMenuControl* SfxMenuControl::CreateControl( const String& rCmd,
        sal_uInt16 nId, Menu& rMenu, SfxBindings &rBindings, SfxVirtualMenu* pVirt )
{
    return new SfxUnoMenuControl( rCmd, nId, rMenu, rBindings, pVirt );
}

SfxUnoMenuControl* SfxMenuControl::CreateControl( const String& rCmd,
        sal_uInt16 nId, Menu& rMenu, const String& sItemText, const String& sHelpText,
        SfxBindings& rBindings, SfxVirtualMenu* pVirt)
{
    return new SfxUnoMenuControl( rCmd, nId, rMenu, sItemText, sHelpText, rBindings, pVirt);
}

SfxUnoMenuControl::SfxUnoMenuControl( const String& rCmd, sal_uInt16 nSlotId,
    Menu& rMenu, SfxBindings& rBindings, SfxVirtualMenu* pVirt )
    : SfxMenuControl( nSlotId, rBindings )
{
    Bind( pVirt, nSlotId, rMenu.GetItemText(nSlotId),
                        rMenu.GetHelpText(nSlotId), rBindings);
    UnBind();
    pUnoCtrl = new SfxUnoControllerItem( this, rBindings, rCmd );
    pUnoCtrl->acquire();
    pUnoCtrl->GetNewDispatch();
}

SfxUnoMenuControl::SfxUnoMenuControl(
    const String& rCmd, sal_uInt16 nSlotId, Menu& /*rMenu*/,
    const String& rItemText, const String& rHelpText,
    SfxBindings& rBindings, SfxVirtualMenu* pVirt)
    : SfxMenuControl( nSlotId, rBindings )
{
    Bind( pVirt, nSlotId, rItemText, rHelpText, rBindings);
    UnBind();
    pUnoCtrl = new SfxUnoControllerItem( this, rBindings, rCmd );
    pUnoCtrl->acquire();
    pUnoCtrl->GetNewDispatch();
}

SfxUnoMenuControl::~SfxUnoMenuControl()
{
    pUnoCtrl->UnBind();
    pUnoCtrl->release();
}

void SfxUnoMenuControl::Select()
{
    pUnoCtrl->Execute();
}
