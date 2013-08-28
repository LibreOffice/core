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
#include <unotools/pathoptions.hxx>
#include <svl/stritem.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <osl/file.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <svl/lngmisc.hxx>

#include <sfx2/mnumgr.hxx>

#include <svtools/menuoptions.hxx>

#include "virtmenu.hxx"
#include <sfx2/msg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>
#include "sfxtypes.hxx"
#include <sfx2/bindings.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/msgpool.hxx>
#include <sfx2/sfx.hrc>
#include "menu.hrc"
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objface.hxx>
#include "thessubmenu.hxx"

// static member initialization
PopupMenu * SfxPopupMenuManager::pStaticThesSubMenu = NULL;

using namespace com::sun::star;

void TryToHideDisabledEntries_Impl( Menu* pMenu )
{
    DBG_ASSERT( pMenu, "invalid menu" );
    if( SvtMenuOptions().IsEntryHidingEnabled() == sal_False )
    {
        pMenu->SetMenuFlags( pMenu->GetMenuFlags() | MENU_FLAG_HIDEDISABLEDENTRIES );
    }
}

//--------------------------------------------------------------------

SfxMenuManager::~SfxMenuManager()
{
    pBindings->ENTERREGISTRATIONS();
    delete pMenu;
    pBindings->LEAVEREGISTRATIONS();
}

//--------------------------------------------------------------------

void SfxMenuManager::Construct( SfxVirtualMenu& rMenu )
{
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

            sal_uInt16 nId = SID_VERB_START + nr++;
            DBG_ASSERT(nId <= SID_VERB_END, "Too many verbs!");
            if ( nId > SID_VERB_END )
                break;

            pMenu->InsertItem( nId, aVerbs[n].VerbName );
        }
    }
}


//--------------------------------------------------------------------


static Image lcl_GetImageFromPngUrl( const OUString &rFileUrl )
{
    Image aRes;

    OUString aTmp;
    osl::FileBase::getSystemPathFromFileURL( rFileUrl, aTmp );

    Graphic aGraphic;
    const OUString aFilterName( IMP_PNG  );
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
    OUString aThesLookUpStr;
    SfxStringItem *pStrItem = dynamic_cast< SfxStringItem * >(pItem);
    sal_Int32 nDelimPos = STRING_LEN;
    if (pStrItem)
    {
        aThesLookUpStr = pStrItem->GetValue();
        nDelimPos = aThesLookUpStr.lastIndexOf( '#' );
    }
    delete pItem;
    if ( !aThesLookUpStr.isEmpty() && nDelimPos != -1 )
    {
        // get synonym list for sub menu
        std::vector< OUString > aSynonyms;
        SfxThesSubMenuHelper aHelper;
        OUString aText( aHelper.GetText( aThesLookUpStr, nDelimPos ) );
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
            OUString sThesImplName( aHelper.GetThesImplName( aLocale ) );
            OUString aSynonymsImageUrl( aCfg.GetSynonymsContextImage( sThesImplName ) );
            if (!sThesImplName.isEmpty() && !aSynonymsImageUrl.isEmpty())
                aImage = Image( lcl_GetImageFromPngUrl( aSynonymsImageUrl ) );

            for (sal_uInt16 i = 0; (size_t)i < nNumSynonyms; ++i)
            {
                //! item ids should start with values > 0, since 0 has special meaning
                const sal_uInt16 nId = i + 1;

                OUString aItemText( linguistic::GetThesaurusReplaceText( aSynonyms[i] ) );
                pThesSubMenu->InsertItem( nId, aItemText );
                OUString aCmd(".uno:ThesaurusFromContext?WordReplace:string=" );
                aCmd += aItemText;
                pThesSubMenu->SetItemCommand( nId, aCmd );

                if (!aSynonymsImageUrl.isEmpty())
                    pThesSubMenu->SetItemImage( nId, aImage );
            }
        }
        else // nNumSynonyms == 0
        {
            const OUString aItemText( SfxResId(STR_MENU_NO_SYNONYM_FOUND).toString() );
            pThesSubMenu->InsertItem( 1, aItemText, MIB_NOSELECT );
        }
        pThesSubMenu->InsertSeparator();
        const OUString sThesaurus( SfxResId(STR_MENU_THESAURUS).toString() );
        pThesSubMenu->InsertItem( 100, sThesaurus );
        pThesSubMenu->SetItemCommand( 100, OUString(".uno:ThesaurusDialog") );

        pSVMenu->InsertSeparator();
        const OUString sSynonyms( SfxResId(STR_MENU_SYNONYMS).toString() );
        pSVMenu->InsertItem( SID_THES, sSynonyms );
        pSVMenu->SetPopupMenu( SID_THES, pThesSubMenu );
    }

    return pThesSubMenu;
}

// ------------------------------------------------------------------------

// executes the function for the selected item
IMPL_LINK( SfxMenuManager, Select, Menu *, pSelMenu )
{
    sal_uInt16 nId = (sal_uInt16) pSelMenu->GetCurItemId();
    OUString aCommand = pSelMenu->GetItemCommand( nId );
    if ( aCommand.isEmpty() && pBindings )
    {
        const SfxSlot* pSlot = SfxSlotPool::GetSlotPool( pBindings->GetDispatcher()->GetFrame() ).GetSlot( nId );
        if ( pSlot && pSlot->pUnoName )
        {
            aCommand = ".uno:" + OUString::createFromAscii( pSlot->GetUnoName() );
        }
    }

    if ( !aCommand.isEmpty() )
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
    sal_uInt16 nVal = ( (PopupMenu*) GetMenu()->GetSVMenu() )->Execute( pWindow, rPos );
    delete pStaticThesSubMenu;  pStaticThesSubMenu = NULL;
    return nVal;
}

//-------------------------------------------------------------------------

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
    OUString sDummyMenuName;
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
    OUString sDummyMenuName;
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
