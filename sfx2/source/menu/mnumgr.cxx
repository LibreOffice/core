/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/embed/VerbAttributes.hpp>
#include <com/sun/star/container/XNamed.hpp>

#ifdef SOLARIS
#include <ctime>
#endif

#include <string>
#include <cstdarg>  // std::va_list

#include <vcl/pointr.hxx>

#include <unotools/streamwrap.hxx>
#include <sfx2/objsh.hxx>
#include <framework/menuconfiguration.hxx>
#include <framework/addonmenu.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/lingucfg.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/stritem.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <osl/file.hxx>
#include <vcl/graph.hxx>
#include <svtools/filter.hxx>
#include <svl/lngmisc.hxx>

#include <sfx2/mnumgr.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#include <svtools/menuoptions.hxx>

#include "virtmenu.hxx"
#include <sfx2/msg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/minstack.hxx>
#include <sfx2/app.hxx>
#include "sfxtypes.hxx"
#include <sfx2/bindings.hxx>
#include "mnucfga.hxx"
#include "sfx2/sfxresid.hxx"
#include <sfx2/msgpool.hxx>
#include <sfx2/sfx.hrc>
#include "menu.hrc"
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objface.hxx>
#include "thessubmenu.hxx"


static const sal_uInt16 nCompatVersion = 4;
static const sal_uInt16 nVersion = 5;

// static member initialization
PopupMenu * SfxPopupMenuManager::pStaticThesSubMenu = NULL;

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
    bAddClipboardFuncs = sal_False;
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
        sal_uInt16 nr=0;
        for ( sal_uInt16 n = 0; n < aVerbs.getLength(); ++n )
        {
            // check for ReadOnly verbs
            if ( pDoc->IsReadOnly() && !(aVerbs[n].VerbAttributes & embed::VerbAttributes::MS_VERBATTR_NEVERDIRTIES) )
                continue;

            // check for verbs that shouldn't appear in the menu
            if ( !(aVerbs[n].VerbAttributes & embed::VerbAttributes::MS_VERBATTR_ONCONTAINERMENU) )
                continue;

            // neue Id vergeben
            sal_uInt16 nId = SID_VERB_START + nr++;
            DBG_ASSERT(nId <= SID_VERB_END, "Zuviele Verben!");
            if ( nId > SID_VERB_END )
                break;

            // einf"ugen
            pMenu->InsertItem( nId, aVerbs[n].VerbName );
        }
    }
}


//--------------------------------------------------------------------


static Image lcl_GetImageFromPngUrl( const ::rtl::OUString &rFileUrl )
{
    Image aRes;

    ::rtl::OUString aTmp;
    osl::FileBase::getSystemPathFromFileURL( rFileUrl, aTmp );

    Graphic aGraphic;
    const String aFilterName( RTL_CONSTASCII_USTRINGPARAM( IMP_PNG ) );
    if( GRFILTER_OK == GraphicFilter::LoadGraphic( aTmp, aFilterName, aGraphic ) )
    {
        aRes = Image( aGraphic.GetBitmapEx() );
    }
    return aRes;
}


PopupMenu* InsertThesaurusSubmenu_Impl( SfxBindings* pBindings, Menu* pSVMenu )
{

    // build thesaurus sub menu if look-up string is available

    PopupMenu* pThesSubMenu = 0;
    SfxPoolItem *pItem = 0;
    pBindings->QueryState( SID_THES, pItem );
    String aThesLookUpStr;
    SfxStringItem *pStrItem = dynamic_cast< SfxStringItem * >(pItem);
    xub_StrLen nDelimPos = STRING_LEN;
    if (pStrItem)
    {
        aThesLookUpStr = pStrItem->GetValue();
        nDelimPos = aThesLookUpStr.SearchBackward( '#' );
    }
    if (aThesLookUpStr.Len() > 0 && nDelimPos != STRING_NOTFOUND)
    {
        // get synonym list for sub menu
        std::vector< ::rtl::OUString > aSynonyms;
        SfxThesSubMenuHelper aHelper;
        ::rtl::OUString aText( aHelper.GetText( aThesLookUpStr, nDelimPos ) );
        lang::Locale aLocale;
        aHelper.GetLocale( aLocale, aThesLookUpStr, nDelimPos );
        const bool bHasMoreSynonyms = aHelper.GetMeanings( aSynonyms, aText, aLocale, 7 /*max number of synonyms to retrieve*/ );
        (void) bHasMoreSynonyms;

        pThesSubMenu = new PopupMenu;
        pThesSubMenu->SetMenuFlags(MENU_FLAG_NOAUTOMNEMONICS);
        const size_t nNumSynonyms = aSynonyms.size();
        if (nNumSynonyms > 0)
        {
            SvtLinguConfig aCfg;

            Image aImage;
            String sThesImplName( aHelper.GetThesImplName( aLocale ) );
            ::rtl::OUString aSynonymsImageUrl( aCfg.GetSynonymsContextImage( sThesImplName ) );
            if (sThesImplName.Len() > 0 && aSynonymsImageUrl.getLength() > 0)
                aImage = Image( lcl_GetImageFromPngUrl( aSynonymsImageUrl ) );

            for (sal_uInt16 i = 0; (size_t)i < nNumSynonyms; ++i)
            {
                //! item ids should start with values > 0, since 0 has special meaning
                const sal_uInt16 nId = i + 1;

                String aItemText( linguistic::GetThesaurusReplaceText( aSynonyms[i] ) );
                pThesSubMenu->InsertItem( nId, aItemText );
                ::rtl::OUString aCmd(RTL_CONSTASCII_USTRINGPARAM(".uno:ThesaurusFromContext?WordReplace:string=") );
                aCmd += aItemText;
                pThesSubMenu->SetItemCommand( nId, aCmd );

                if (aSynonymsImageUrl.getLength() > 0)
                    pThesSubMenu->SetItemImage( nId, aImage );
            }
        }
        else // nNumSynonyms == 0
        {
            const String aItemText( SfxResId( STR_MENU_NO_SYNONYM_FOUND ) );
            pThesSubMenu->InsertItem( 1, aItemText, MIB_NOSELECT );
        }
        pThesSubMenu->InsertSeparator();
        const String sThesaurus( SfxResId( STR_MENU_THESAURUS ) );
        pThesSubMenu->InsertItem( 100, sThesaurus );
        pThesSubMenu->SetItemCommand( 100, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:ThesaurusDialog")) );

        pSVMenu->InsertSeparator();
        const String sSynonyms( SfxResId( STR_MENU_SYNONYMS ) );
        pSVMenu->InsertItem( SID_THES, sSynonyms );
        pSVMenu->SetPopupMenu( SID_THES, pThesSubMenu );
    }

    return pThesSubMenu;
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
        ResId aResId(GetType(),*pResMgr);
        aResId.SetRT(RSC_MENU);
        Menu *pSVMenu = new PopupMenu( aResId );

        if ( bAddClipboardFuncs )
        {
            sal_uInt16 n, nCount = pSVMenu->GetItemCount();
            for ( n=0; n<nCount; n++ )
            {
                sal_uInt16 nId = pSVMenu->GetItemId( n );
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
                    sal_uInt16 nId = aPop.GetItemId( n );
                    pSVMenu->InsertItem( nId, aPop.GetItemText( nId ), aPop.GetItemBits( nId ) );
                }
            }
        }

        pVMenu = new SfxVirtualMenu( pSVMenu, sal_False, *pBindings, sal_True, sal_True );
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

    sal_uInt16 nId = (sal_uInt16) pSelMenu->GetCurItemId();
    String aCommand = pSelMenu->GetItemCommand( nId );
    if ( !aCommand.Len() && pBindings )
    {
        const SfxSlot* pSlot = SfxSlotPool::GetSlotPool( pBindings->GetDispatcher()->GetFrame() ).GetSlot( nId );
        if ( pSlot && pSlot->pUnoName )
        {
            aCommand = DEFINE_CONST_UNICODE(".uno:");
            aCommand += String::CreateFromAscii( pSlot->GetUnoName() );
        }
    }

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

    return sal_True;
}

//--------------------------------------------------------------------

void SfxMenuManager::Construct_Impl( Menu* pSVMenu, sal_Bool bWithHelp )
{
    SfxVirtualMenu *pOldVirtMenu=0;
    if ( pMenu )
    {
        // Es wird umkonfiguriert
        pOldVirtMenu = pMenu;
        pBindings->ENTERREGISTRATIONS();
    }

    TryToHideDisabledEntries_Impl( pSVMenu );
    SfxVirtualMenu *pVMenu = new SfxVirtualMenu( pSVMenu, bWithHelp, *pBindings, sal_True );
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

SfxPopupMenuManager::~SfxPopupMenuManager()
{
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::RemoveDisabledEntries()
{
    if ( pSVMenu )
        TryToHideDisabledEntries_Impl( pSVMenu );
}

//--------------------------------------------------------------------

sal_uInt16 SfxPopupMenuManager::Execute( const Point& rPos, Window* pWindow )
{
    DBG_MEMTEST();
    sal_uInt16 nVal = ( (PopupMenu*) GetMenu()->GetSVMenu() )->Execute( pWindow, rPos );
    delete pStaticThesSubMenu;  pStaticThesSubMenu = NULL;
    return nVal;
}

//--------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxPopupMenuManager, SelectHdl, void *, EMPTYARG )
{
    return 1;
}
IMPL_LINK_INLINE_END( SfxPopupMenuManager, SelectHdl, void *, EMPTYARG )


//--------------------------------------------------------------------

sal_uInt16 SfxPopupMenuManager::Execute( const Point& rPoint, Window* pWindow, va_list pArgs, const SfxPoolItem *pArg1 )
{
    DBG_MEMTEST();

    PopupMenu* pPopMenu = ( (PopupMenu*)GetMenu()->GetSVMenu() );
    pPopMenu->SetSelectHdl( LINK( this, SfxPopupMenuManager, SelectHdl ) );
    sal_uInt16 nId = pPopMenu->Execute( pWindow, rPoint );
    pPopMenu->SetSelectHdl( Link() );

    if ( nId )
        GetBindings().GetDispatcher()->_Execute( nId, SFX_CALLMODE_RECORD, pArgs, pArg1 );

    return nId;
}

//--------------------------------------------------------------------

sal_uInt16 SfxPopupMenuManager::Execute( const Point& rPoint, Window* pWindow, const SfxPoolItem *pArg1, ... )
{
    DBG_MEMTEST();

    va_list pArgs;
    va_start(pArgs, pArg1);

    return (Execute( rPoint, pWindow, pArgs, pArg1 ));
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::StartInsert()
{
    ResId aResId(GetType(),*pResMgr);
    aResId.SetRT(RSC_MENU);
    pSVMenu = new PopupMenu( aResId );
    TryToHideDisabledEntries_Impl( pSVMenu );
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::EndInsert()
{
    pBindings->ENTERREGISTRATIONS();
    pMenu = new SfxVirtualMenu( pSVMenu, sal_False, *pBindings, sal_True, sal_True );
    Construct( *pMenu );
    pBindings->LEAVEREGISTRATIONS();
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::InsertSeparator( sal_uInt16 nPos )
{
    pSVMenu->InsertSeparator( nPos );
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::InsertItem( sal_uInt16 nId, const String& rName, MenuItemBits nBits, const rtl::OString& rHelpId, sal_uInt16 nPos )
{
    pSVMenu->InsertItem( nId, rName, nBits,nPos );
    pSVMenu->SetHelpId( nId, rHelpId );
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::RemoveItem( sal_uInt16 nId )
{
    pSVMenu->RemoveItem( nId );
}

//-------------------------------------------------------------------------

void SfxPopupMenuManager::CheckItem( sal_uInt16 nId, sal_Bool bCheck )
{
    pSVMenu->CheckItem( nId, bCheck );
}

void SfxPopupMenuManager::AddClipboardFunctions()
{
    bAddClipboardFuncs = sal_True;
}

SfxMenuManager::SfxMenuManager( Menu* pMenuArg, SfxBindings &rBindings )
:   pMenu(0),
    pOldMenu(0),
    pBindings(&rBindings),
    pResMgr(NULL),
    nType(0)
{
    bAddClipboardFuncs = sal_False;
    SfxVirtualMenu* pVMenu = new SfxVirtualMenu( pMenuArg, sal_False, rBindings, sal_True, sal_True );
    Construct(*pVMenu);
}

SfxPopupMenuManager::SfxPopupMenuManager( PopupMenu* pMenuArg, SfxBindings& rBindings )
    : SfxMenuManager( pMenuArg, rBindings )
    , pSVMenu( pMenuArg )
{
}

SfxPopupMenuManager* SfxPopupMenuManager::Popup( const ResId& rResId, SfxViewFrame* pFrame,const Point& rPoint, Window* pWindow )
{
    PopupMenu *pSVMenu = new PopupMenu( rResId );
    sal_uInt16 n, nCount = pSVMenu->GetItemCount();
    for ( n=0; n<nCount; n++ )
    {
        sal_uInt16 nId = pSVMenu->GetItemId( n );
        if ( nId == SID_COPY || nId == SID_CUT || nId == SID_PASTE )
            break;
    }

    PopupMenu* pThesSubMenu = InsertThesaurusSubmenu_Impl( &pFrame->GetBindings(), pSVMenu );
    // #i107205# (see comment in header file)
    pStaticThesSubMenu = pThesSubMenu;

    if ( n == nCount )
    {
        PopupMenu aPop( SfxResId( MN_CLIPBOARDFUNCS ) );
        nCount = aPop.GetItemCount();
        pSVMenu->InsertSeparator();
        for ( n=0; n<nCount; n++ )
        {
            sal_uInt16 nId = aPop.GetItemId( n );
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
    ::rtl::OUString sDummyMenuName;
    if ( pFrame->GetViewShell()->TryContextMenuInterception( *pSVMenu, sDummyMenuName, pMenu, aEvent ) )
    {
        if ( pMenu )
        {
            delete pSVMenu;
            pSVMenu = (PopupMenu*) pMenu;
        }

        SfxPopupMenuManager* aMgr = new SfxPopupMenuManager( pSVMenu, pFrame->GetBindings());
        aMgr->RemoveDisabledEntries();
        return aMgr;
    }

    return 0;
}


void SfxPopupMenuManager::ExecutePopup( const ResId& rResId, SfxViewFrame* pFrame, const Point& rPoint, Window* pWindow )
{
    PopupMenu *pSVMenu = new PopupMenu( rResId );
    sal_uInt16 n, nCount = pSVMenu->GetItemCount();
    for ( n=0; n<nCount; n++ )
    {
        sal_uInt16 nId = pSVMenu->GetItemId( n );
        if ( nId == SID_COPY || nId == SID_CUT || nId == SID_PASTE )
            break;
    }

    PopupMenu* pThesSubMenu = InsertThesaurusSubmenu_Impl( &pFrame->GetBindings(), pSVMenu );

    if ( n == nCount )
    {
        PopupMenu aPop( SfxResId( MN_CLIPBOARDFUNCS ) );
        nCount = aPop.GetItemCount();
        pSVMenu->InsertSeparator();
        for ( n=0; n<nCount; n++ )
        {
            sal_uInt16 nId = aPop.GetItemId( n );
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
    ::rtl::OUString sDummyMenuName;
    if ( pFrame->GetViewShell()->TryContextMenuInterception( *pSVMenu, sDummyMenuName, pMenu, aEvent ) )
    {
        if ( pMenu )
        {
            delete pSVMenu;
            pSVMenu = (PopupMenu*) pMenu;
        }

        SfxPopupMenuManager aPop( pSVMenu, pFrame->GetBindings() );
        aPop.RemoveDisabledEntries();
        aPop.Execute( rPoint, pWindow );

        // #i112646 avoid crash when context menu is closed.
        // the (manually inserted) sub-menu needs to be destroyed before
        // aPop gets destroyed.
        delete pThesSubMenu;
        pThesSubMenu = 0;
    }

    delete pThesSubMenu;
}

Menu* SfxPopupMenuManager::GetSVMenu()
{
    return (Menu*) GetMenu()->GetSVMenu();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
