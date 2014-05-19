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

#include "macroass.hxx"

#include <basic/basmgr.hxx>
#include <comphelper/string.hxx>
#include <dialmgr.hxx>
#include <svl/macitem.hxx>
#include <svx/dialogs.hrc>
#include <svtools/svmedit.hxx>
#include "cfgutil.hxx"
#include <sfx2/app.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/objsh.hxx>
#include "cuires.hrc"
#include <vcl/fixed.hxx>
#include "headertablistbox.hxx"
#include "svtools/svlbitm.hxx"
#include "svtools/treelistentry.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XFrame;

class _SfxMacroTabPage_Impl
{
public:
    _SfxMacroTabPage_Impl();

    OUString                        maStaticMacroLBLabel;
    PushButton*                     pAssignPB;
    PushButton*                     pDeletePB;
    OUString                        sStrEvent;
    OUString                        sAssignedMacro;
    MacroEventListBox*              pEventLB;
    VclFrame*                       pGroupFrame;
    SfxConfigGroupListBox*          pGroupLB;
    VclFrame*                       pMacroFrame;
    SfxConfigFunctionListBox*       pMacroLB;

    bool                            bReadOnly;
    Timer                           maFillGroupTimer;
    bool                            bGotEvents;
    bool m_bDummyActivated; ///< has this tab page already been activated
};

_SfxMacroTabPage_Impl::_SfxMacroTabPage_Impl()
    : pAssignPB(NULL)
    , pDeletePB(NULL)
    , pEventLB(NULL)
    , pGroupFrame(NULL)
    , pGroupLB(NULL)
    , pMacroFrame(NULL)
    , pMacroLB(NULL)
    , bReadOnly(false)
    , bGotEvents(false)
    , m_bDummyActivated(false)
{
}

static sal_uInt16 aPageRg[] = {
    SID_ATTR_MACROITEM, SID_ATTR_MACROITEM,
    0
};

// attention, this array is indexed directly (0, 1, ...) in the code
static long nTabs[] =
    {
        2, // Number of Tabs
        0, 90
    };

// IDs for items in HeaderBar of EventLB
#define ITEMID_EVENT        1
#define ITMEID_ASSMACRO     2


#define LB_MACROS_ITEMPOS   2

OUString ConvertToUIName_Impl( SvxMacro *pMacro )
{
    OUString aName( pMacro->GetMacName() );
    OUString aEntry;
    if ( pMacro->GetLanguage() != "JavaScript" )
    {
        sal_uInt16 nCount = comphelper::string::getTokenCount(aName, '.');
        aEntry = aName.getToken( nCount-1, '.' );
        if ( nCount > 2 )
        {
            aEntry += "(";
            aEntry += aName.getToken( 0, '.' );
            aEntry += ".";
            aEntry += aName.getToken( nCount-2, '.' );
            aEntry += ")";
        }
        return aEntry;
    }
    else
        return aName;
}

void _SfxMacroTabPage::EnableButtons()
{
    // don't do anything as long as the eventbox is empty
    const SvTreeListEntry* pE = mpImpl->pEventLB->GetListBox().FirstSelected();
    if ( pE )
    {
        // get bound macro
        const SvxMacro* pM = aTbl.Get( (sal_uInt16)(sal_uLong) pE->GetUserData() );
        mpImpl->pDeletePB->Enable( 0 != pM && !mpImpl->bReadOnly );

        OUString sEventMacro;
        sEventMacro = ((SvLBoxString*)pE->GetItem( LB_MACROS_ITEMPOS ))->GetText();

        OUString sScriptURI = mpImpl->pMacroLB->GetSelectedScriptURI();
        mpImpl->pAssignPB->Enable( !mpImpl->bReadOnly && !sScriptURI.equalsIgnoreAsciiCase( sEventMacro ) );
    }
    else
        mpImpl->pAssignPB->Enable( false );
}

_SfxMacroTabPage::_SfxMacroTabPage(Window* pParent, const SfxItemSet& rAttrSet)
    : SfxTabPage(pParent, "EventAssignPage", "cui/ui/eventassignpage.ui", rAttrSet)
{
    mpImpl = new _SfxMacroTabPage_Impl;
}

_SfxMacroTabPage::~_SfxMacroTabPage()
{
    DELETEZ( mpImpl );
}

void _SfxMacroTabPage::AddEvent( const OUString & rEventName, sal_uInt16 nEventId )
{
    OUString sTmp( rEventName );
    sTmp += "\t";

    // if the table is valid already
    SvxMacro* pM = aTbl.Get( nEventId );
    if( pM )
    {
        OUString sNew( ConvertToUIName_Impl( pM ) );
        sTmp += sNew;
    }

    SvTreeListEntry* pE = mpImpl->pEventLB->GetListBox().InsertEntry( sTmp );
    pE->SetUserData( reinterpret_cast< void* >( sal::static_int_cast< sal_IntPtr >( nEventId )) );
}

void _SfxMacroTabPage::ScriptChanged()
{
    // get new areas and their functions
    mpImpl->pGroupFrame->Show();
    mpImpl->pMacroFrame->Show();

    EnableButtons();
}

bool _SfxMacroTabPage::FillItemSet( SfxItemSet& rSet )
{
    SvxMacroItem aItem( GetWhich( aPageRg[0] ) );
    ((SvxMacroTableDtor&)aItem.GetMacroTable()) = aTbl;

    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET != GetItemSet().GetItemState( aItem.Which(), true, &pItem )
        || aItem != *(SvxMacroItem*)pItem )
    {
        rSet.Put( aItem );
        return true;
    }
    return false;
}

void _SfxMacroTabPage::LaunchFillGroup()
{
    if (!mpImpl->maFillGroupTimer.GetTimeoutHdl().IsSet())
    {
        mpImpl->maFillGroupTimer.SetTimeoutHdl( STATIC_LINK( this, _SfxMacroTabPage, TimeOut_Impl ) );
        mpImpl->maFillGroupTimer.SetTimeout( 0 );
        mpImpl->maFillGroupTimer.Start();
    }
}

void _SfxMacroTabPage::ActivatePage( const SfxItemSet& )
{
    // fdo#57553 lazily init script providers, because it is annoying if done
    // on dialog open (SfxTabDialog::Start_Impl activates all tab pages once!)
    if (!mpImpl->m_bDummyActivated)
    {
        mpImpl->m_bDummyActivated = true;
        return;
    }
    LaunchFillGroup();
}

void _SfxMacroTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxPoolItem* pEventsItem;
    if( !mpImpl->bGotEvents && SFX_ITEM_SET == aSet.GetItemState( SID_EVENTCONFIG, true, &pEventsItem ) )
    {
        mpImpl->bGotEvents = true;
        const SfxEventNamesList& rList = ((SfxEventNamesItem*)pEventsItem)->GetEvents();
        for ( size_t nNo = 0, nCnt = rList.size(); nNo < nCnt; ++nNo )
        {
            const SfxEventName *pOwn = rList.at(nNo);
            AddEvent( pOwn->maUIName, pOwn->mnId );
        }
    }
}

void _SfxMacroTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( GetWhich( aPageRg[0] ), true, &pItem ))
        aTbl = ((SvxMacroItem*)pItem)->GetMacroTable();

    const SfxPoolItem* pEventsItem;
    if( !mpImpl->bGotEvents && SFX_ITEM_SET == rSet.GetItemState( SID_EVENTCONFIG, true, &pEventsItem ) )
    {
        mpImpl->bGotEvents = true;
        const SfxEventNamesList& rList = ((SfxEventNamesItem*)pEventsItem)->GetEvents();
        for ( size_t nNo = 0, nCnt = rList.size(); nNo < nCnt; ++nNo )
        {
            const SfxEventName *pOwn = rList.at(nNo);
            AddEvent( pOwn->maUIName, pOwn->mnId );
        }
    }

    FillEvents();

    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    SvTreeListEntry* pE = rListBox.GetEntry( 0 );
    if( pE )
        rListBox.SetCurEntry( pE );
}

bool _SfxMacroTabPage::IsReadOnly() const
{
    return mpImpl->bReadOnly;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, SelectEvent_Impl, SvTabListBox*, EMPTYARG )
{
    _SfxMacroTabPage_Impl*  pImpl = pThis->mpImpl;
    SvHeaderTabListBox&     rListBox = pImpl->pEventLB->GetListBox();
    SvTreeListEntry*            pE = rListBox.FirstSelected();
    sal_uLong                   nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = rListBox.GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return 0;
    }

    pThis->ScriptChanged();
    pThis->EnableButtons();
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, SelectGroup_Impl, ListBox*, EMPTYARG )
{
    _SfxMacroTabPage_Impl*  pImpl = pThis->mpImpl;
    pImpl->pGroupLB->GroupSelected();
    const OUString sScriptURI = pImpl->pMacroLB->GetSelectedScriptURI();
    OUString       aLabelText;
    if( !sScriptURI.isEmpty() )
        aLabelText = pImpl->maStaticMacroLBLabel;
    pImpl->pMacroFrame->set_label( aLabelText );

    pThis->EnableButtons();
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, SelectMacro_Impl, ListBox*, EMPTYARG )
{
    _SfxMacroTabPage_Impl*  pImpl = pThis->mpImpl;
    pImpl->pMacroLB->FunctionSelected();
    pThis->EnableButtons();
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, AssignDeleteHdl_Impl, PushButton*, pBtn )
{
    _SfxMacroTabPage_Impl*  pImpl = pThis->mpImpl;
    SvHeaderTabListBox& rListBox = pImpl->pEventLB->GetListBox();
    SvTreeListEntry* pE = rListBox.FirstSelected();
    sal_uLong nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = rListBox.GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return 0;
    }

    const bool bAssEnabled = pBtn != pImpl->pDeletePB && pImpl->pAssignPB->IsEnabled();

    // remove from the table
    sal_uInt16 nEvent = (sal_uInt16)(sal_uLong)pE->GetUserData();
    pThis->aTbl.Erase( nEvent );

    OUString sScriptURI;
    if( bAssEnabled )
    {
        sScriptURI = pImpl->pMacroLB->GetSelectedScriptURI();
        if( sScriptURI.startsWith( "vnd.sun.star.script:" ) )
        {
            pThis->aTbl.Insert(
                nEvent, SvxMacro( sScriptURI, OUString( SVX_MACRO_LANGUAGE_SF ) ) );
        }
        else
        {
            OSL_ENSURE( false, "_SfxMacroTabPage::AssignDeleteHdl_Impl: this branch is *not* dead? (out of interest: tell fs, please!)" );
            pThis->aTbl.Insert(
                nEvent, SvxMacro( sScriptURI, OUString( SVX_MACRO_LANGUAGE_STARBASIC ) ) );
        }
    }

    pImpl->pEventLB->SetUpdateMode( false );
    pE->ReplaceItem( new SvLBoxString( pE, 0, sScriptURI ), LB_MACROS_ITEMPOS );
    rListBox.GetModel()->InvalidateEntry( pE );
    rListBox.Select( pE );
    rListBox.MakeVisible( pE );
    rListBox.SetUpdateMode( true );

    pThis->EnableButtons();
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, TimeOut_Impl, Timer*, EMPTYARG )
{
    // FillMacroList() can take a long time -> show wait cursor and disable input
    SfxTabDialog* pTabDlg = pThis->GetTabDialog();
    // perhaps the tabpage is part of a SingleTabDialog then pTabDlg == NULL
    if ( pTabDlg )
    {
        pTabDlg->EnterWait();
        pTabDlg->EnableInput( false );
    }
    pThis->FillMacroList();
    if ( pTabDlg )
    {
        pTabDlg->EnableInput( true );
        pTabDlg->LeaveWait();
    }
    return 0;
}

void _SfxMacroTabPage::InitAndSetHandler()
{
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    HeaderBar&          rHeaderBar = mpImpl->pEventLB->GetHeaderBar();
    Link                aLnk(STATIC_LINK(this, _SfxMacroTabPage, AssignDeleteHdl_Impl ));
    mpImpl->pMacroLB->SetDoubleClickHdl( aLnk );
    mpImpl->pDeletePB->SetClickHdl( aLnk );
    mpImpl->pAssignPB->SetClickHdl( aLnk );
    rListBox.SetDoubleClickHdl( aLnk );

    rListBox.SetSelectHdl( STATIC_LINK( this, _SfxMacroTabPage, SelectEvent_Impl ));
    mpImpl->pGroupLB->SetSelectHdl( STATIC_LINK( this, _SfxMacroTabPage, SelectGroup_Impl ));
    mpImpl->pMacroLB->SetSelectHdl( STATIC_LINK( this, _SfxMacroTabPage, SelectMacro_Impl ));

    rListBox.SetSelectionMode( SINGLE_SELECTION );
    rListBox.SetTabs( &nTabs[0], MAP_APPFONT );
    Size aSize( nTabs[ 2 ], 0 );
    rHeaderBar.InsertItem( ITEMID_EVENT, mpImpl->sStrEvent, LogicToPixel( aSize, MapMode( MAP_APPFONT ) ).Width() );
    aSize.Width() = 1764;       // don't know what, so 42^2 is best to use...
    rHeaderBar.InsertItem( ITMEID_ASSMACRO, mpImpl->sAssignedMacro, LogicToPixel( aSize, MapMode( MAP_APPFONT ) ).Width() );
    rListBox.SetSpaceBetweenEntries( 0 );

    mpImpl->pEventLB->Show();
    mpImpl->pEventLB->ConnectElements();

    mpImpl->pEventLB->Enable( true );
    mpImpl->pGroupLB->Enable( true );
    mpImpl->pMacroLB->Enable( true );

    mpImpl->pGroupLB->SetFunctionListBox( mpImpl->pMacroLB );

}

void _SfxMacroTabPage::FillMacroList()
{
    mpImpl->pGroupLB->Init(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext >(),
        GetFrame(),
        OUString(), false);
}

void _SfxMacroTabPage::FillEvents()
{
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();

    sal_uLong       nEntryCnt = rListBox.GetEntryCount();

    // get events from the table and fill the EventListBox respectively
    for( sal_uLong n = 0 ; n < nEntryCnt ; ++n )
    {
        SvTreeListEntry*    pE = rListBox.GetEntry( n );
        if( pE )
        {
            SvLBoxString*   pLItem = ( SvLBoxString* ) pE->GetItem( LB_MACROS_ITEMPOS );
            DBG_ASSERT( pLItem && SV_ITEM_ID_LBOXSTRING == pLItem->GetType(), "_SfxMacroTabPage::FillEvents(): no LBoxString" );

            OUString          sOld( pLItem->GetText() );
            OUString          sNew;
            sal_uInt16          nEventId = ( sal_uInt16 ) ( sal_uLong ) pE->GetUserData();
            if( aTbl.IsKeyValid( nEventId ) )
                sNew = ConvertToUIName_Impl( aTbl.Get( nEventId ) );

            if( sOld != sNew )
            {
                pE->ReplaceItem( new SvLBoxString( pE, 0, sNew ), LB_MACROS_ITEMPOS );
                rListBox.GetModel()->InvalidateEntry( pE );
            }
        }
    }
}

SfxMacroTabPage::SfxMacroTabPage(Window* pParent, const Reference< XFrame >& rxDocumentFrame, const SfxItemSet& rSet )
    : _SfxMacroTabPage( pParent, rSet )
{
    mpImpl->sStrEvent = get<FixedText>("eventft")->GetText();
    mpImpl->sAssignedMacro = get<FixedText>("assignft")->GetText();
    get(mpImpl->pEventLB , "assignments");
    get(mpImpl->pAssignPB, "assign");
    get(mpImpl->pDeletePB, "delete");
    get(mpImpl->pGroupFrame, "groupframe");
    get(mpImpl->pGroupLB, "libraries");
    get(mpImpl->pMacroFrame, "macroframe");
    mpImpl->maStaticMacroLBLabel = mpImpl->pMacroFrame->get_label();
    get(mpImpl->pMacroLB, "macros");

    SetFrame( rxDocumentFrame );

    InitAndSetHandler();

    ScriptChanged();
}

namespace
{
    SfxMacroTabPage* CreateSfxMacroTabPage( Window* pParent, const SfxItemSet& rAttrSet )
    {
        return new SfxMacroTabPage( pParent, NULL, rAttrSet );
    }
}

SfxTabPage* SfxMacroTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return CreateSfxMacroTabPage(pParent, rAttrSet);
}

SfxMacroAssignDlg::SfxMacroAssignDlg(Window* pParent,
    const Reference< XFrame >& rxDocumentFrame, const SfxItemSet& rSet)
    : SfxSingleTabDialog(pParent, rSet, "EventAssignDialog",
        "cui/ui/eventassigndialog.ui")
{
    SfxMacroTabPage* pPage = CreateSfxMacroTabPage(get_content_area(), rSet);
    pPage->SetFrame( rxDocumentFrame );
    SetTabPage( pPage );
    pPage->LaunchFillGroup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
