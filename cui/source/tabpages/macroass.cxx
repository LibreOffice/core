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
#include <vcl/idle.hxx>
#include "cfgutil.hxx"
#include <sfx2/app.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/objsh.hxx>
#include "cuires.hrc"
#include <vcl/fixed.hxx>
#include "headertablistbox.hxx"
#include "svtools/svlbitm.hxx"
#include "svtools/treelistentry.hxx"
#include <o3tl/make_unique.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XFrame;

class SfxMacroTabPage_Impl
{
public:
    SfxMacroTabPage_Impl();

    OUString                               maStaticMacroLBLabel;
    VclPtr<PushButton>                     pAssignPB;
    VclPtr<PushButton>                     pDeletePB;
    OUString                               sStrEvent;
    OUString                               sAssignedMacro;
    VclPtr<MacroEventListBox>              pEventLB;
    VclPtr<VclFrame>                       pGroupFrame;
    VclPtr<SfxConfigGroupListBox>          pGroupLB;
    VclPtr<VclFrame>                       pMacroFrame;
    VclPtr<SfxConfigFunctionListBox>       pMacroLB;

    bool                            bReadOnly;
    Idle                            maFillGroupIdle;
    bool                            bGotEvents;
    bool m_bDummyActivated; ///< has this tab page already been activated
};

SfxMacroTabPage_Impl::SfxMacroTabPage_Impl()
    : pAssignPB(nullptr)
    , pDeletePB(nullptr)
    , pEventLB(nullptr)
    , pGroupFrame(nullptr)
    , pGroupLB(nullptr)
    , pMacroFrame(nullptr)
    , pMacroLB(nullptr)
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
        const sal_Int32 nCount = comphelper::string::getTokenCount(aName, '.');
        aEntry = aName.getToken( nCount-1, '.' );
        if ( nCount > 2 )
        {
            aEntry += "(" + aName.getToken( 0, '.' ) + "." + aName.getToken( nCount-2, '.' ) + ")";
        }
        return aEntry;
    }
    else
        return aName;
}

void SfxMacroTabPage::EnableButtons()
{
    // don't do anything as long as the eventbox is empty
    const SvTreeListEntry* pE = mpImpl->pEventLB->GetListBox().FirstSelected();
    if ( pE )
    {
        // get bound macro
        const SvxMacro* pM = aTbl.Get( (sal_uInt16)reinterpret_cast<sal_uLong>(pE->GetUserData()) );
        mpImpl->pDeletePB->Enable( nullptr != pM && !mpImpl->bReadOnly );

        OUString sEventMacro = static_cast<const SvLBoxString&>(pE->GetItem( LB_MACROS_ITEMPOS )).GetText();

        OUString sScriptURI = mpImpl->pMacroLB->GetSelectedScriptURI();
        mpImpl->pAssignPB->Enable( !mpImpl->bReadOnly && !sScriptURI.equalsIgnoreAsciiCase( sEventMacro ) );
    }
    else
        mpImpl->pAssignPB->Enable( false );
}

SfxMacroTabPage::SfxMacroTabPage(vcl::Window* pParent, const Reference< XFrame >& rxDocumentFrame, const SfxItemSet& rAttrSet )
    : SfxTabPage(pParent, "EventAssignPage", "cui/ui/eventassignpage.ui", &rAttrSet)
{
    mpImpl = new SfxMacroTabPage_Impl;
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


SfxMacroTabPage::~SfxMacroTabPage()
{
    disposeOnce();
}

void SfxMacroTabPage::dispose()
{
    DELETEZ( mpImpl );
    SfxTabPage::dispose();
}

void SfxMacroTabPage::AddEvent( const OUString & rEventName, sal_uInt16 nEventId )
{
    OUString sTmp = rEventName  + "\t";

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

void SfxMacroTabPage::ScriptChanged()
{
    // get new areas and their functions
    mpImpl->pGroupFrame->Show();
    mpImpl->pMacroFrame->Show();

    EnableButtons();
}

bool SfxMacroTabPage::FillItemSet( SfxItemSet* rSet )
{
    SvxMacroItem aItem( GetWhich( aPageRg[0] ) );
    ((SvxMacroTableDtor&)aItem.GetMacroTable()) = aTbl;

    const SfxPoolItem* pItem;
    if( SfxItemState::SET != GetItemSet().GetItemState( aItem.Which(), true, &pItem )
        || aItem != *static_cast<const SvxMacroItem*>(pItem) )
    {
        rSet->Put( aItem );
        return true;
    }
    return false;
}

void SfxMacroTabPage::LaunchFillGroup()
{
    if (!mpImpl->maFillGroupIdle.GetIdleHdl().IsSet())
    {
        mpImpl->maFillGroupIdle.SetIdleHdl( LINK( this, SfxMacroTabPage, TimeOut_Impl ) );
        mpImpl->maFillGroupIdle.SetPriority( SchedulerPriority::HIGHEST );
        mpImpl->maFillGroupIdle.Start();
    }
}

void SfxMacroTabPage::ActivatePage( const SfxItemSet& )
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

void SfxMacroTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxPoolItem* pEventsItem;
    if( !mpImpl->bGotEvents && SfxItemState::SET == aSet.GetItemState( SID_EVENTCONFIG, true, &pEventsItem ) )
    {
        mpImpl->bGotEvents = true;
        const SfxEventNamesList& rList = static_cast<const SfxEventNamesItem*>(pEventsItem)->GetEvents();
        for ( size_t nNo = 0, nCnt = rList.size(); nNo < nCnt; ++nNo )
        {
            const SfxEventName *pOwn = rList.at(nNo);
            AddEvent( pOwn->maUIName, pOwn->mnId );
        }
    }
}

void SfxMacroTabPage::Reset( const SfxItemSet* rSet )
{
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rSet->GetItemState( GetWhich( aPageRg[0] ), true, &pItem ))
        aTbl = static_cast<const SvxMacroItem*>(pItem)->GetMacroTable();

    const SfxPoolItem* pEventsItem;
    if( !mpImpl->bGotEvents && SfxItemState::SET == rSet->GetItemState( SID_EVENTCONFIG, true, &pEventsItem ) )
    {
        mpImpl->bGotEvents = true;
        const SfxEventNamesList& rList = static_cast<const SfxEventNamesItem*>(pEventsItem)->GetEvents();
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

bool SfxMacroTabPage::IsReadOnly() const
{
    return mpImpl->bReadOnly;
}

IMPL_LINK_NOARG_TYPED( SfxMacroTabPage, SelectEvent_Impl, SvTreeListBox*, void)
{
    SvHeaderTabListBox&     rListBox = mpImpl->pEventLB->GetListBox();
    SvTreeListEntry*            pE = rListBox.FirstSelected();
    sal_uLong                   nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = rListBox.GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return;
    }

    ScriptChanged();
    EnableButtons();
}

IMPL_LINK_NOARG_TYPED( SfxMacroTabPage, SelectGroup_Impl, SvTreeListBox*, void)
{
    mpImpl->pGroupLB->GroupSelected();
    const OUString sScriptURI = mpImpl->pMacroLB->GetSelectedScriptURI();
    OUString       aLabelText;
    if( !sScriptURI.isEmpty() )
        aLabelText = mpImpl->maStaticMacroLBLabel;
    mpImpl->pMacroFrame->set_label( aLabelText );

    EnableButtons();
}

IMPL_LINK_NOARG_TYPED( SfxMacroTabPage, SelectMacro_Impl, SvTreeListBox*, void)
{
    EnableButtons();
}

IMPL_LINK_TYPED( SfxMacroTabPage, AssignDeleteClickHdl_Impl, Button*, pBtn, void )
{
    AssignDeleteHdl(pBtn);
}

IMPL_LINK_TYPED( SfxMacroTabPage, AssignDeleteHdl_Impl, SvTreeListBox*, pBtn, bool )
{
    return AssignDeleteHdl(pBtn);
}

bool SfxMacroTabPage::AssignDeleteHdl(Control* pBtn)
{
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    SvTreeListEntry* pE = rListBox.FirstSelected();
    sal_uLong nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = rListBox.GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return false;
    }

    const bool bAssEnabled = pBtn != mpImpl->pDeletePB && mpImpl->pAssignPB->IsEnabled();

    // remove from the table
    sal_uInt16 nEvent = (sal_uInt16)reinterpret_cast<sal_uLong>(pE->GetUserData());
    aTbl.Erase( nEvent );

    OUString sScriptURI;
    if( bAssEnabled )
    {
        sScriptURI = mpImpl->pMacroLB->GetSelectedScriptURI();
        if( sScriptURI.startsWith( "vnd.sun.star.script:" ) )
        {
            aTbl.Insert(
                nEvent, SvxMacro( sScriptURI, OUString( SVX_MACRO_LANGUAGE_SF ) ) );
        }
        else
        {
            OSL_ENSURE( false, "SfxMacroTabPage::AssignDeleteHdl_Impl: this branch is *not* dead? (out of interest: tell fs, please!)" );
            aTbl.Insert(
                nEvent, SvxMacro( sScriptURI, OUString( SVX_MACRO_LANGUAGE_STARBASIC ) ) );
        }
    }

    mpImpl->pEventLB->SetUpdateMode( false );
    pE->ReplaceItem(o3tl::make_unique<SvLBoxString>(sScriptURI), LB_MACROS_ITEMPOS);
    rListBox.GetModel()->InvalidateEntry( pE );
    rListBox.Select( pE );
    rListBox.MakeVisible( pE );
    rListBox.SetUpdateMode( true );

    EnableButtons();
    return false;
}

IMPL_LINK_TYPED( SfxMacroTabPage, TimeOut_Impl, Idle*,, void )
{
    // FillMacroList() can take a long time -> show wait cursor and disable input
    SfxTabDialog* pTabDlg = GetTabDialog();
    // perhaps the tabpage is part of a SingleTabDialog then pTabDlg == NULL
    if ( pTabDlg )
    {
        pTabDlg->EnterWait();
        pTabDlg->EnableInput( false );
    }
    FillMacroList();
    if ( pTabDlg )
    {
        pTabDlg->EnableInput();
        pTabDlg->LeaveWait();
    }
}

void SfxMacroTabPage::InitAndSetHandler()
{
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    HeaderBar&          rHeaderBar = mpImpl->pEventLB->GetHeaderBar();
    Link<SvTreeListBox*,bool> aLnk(LINK(this, SfxMacroTabPage, AssignDeleteHdl_Impl ));
    mpImpl->pMacroLB->SetDoubleClickHdl( aLnk );
    mpImpl->pDeletePB->SetClickHdl( LINK(this, SfxMacroTabPage, AssignDeleteClickHdl_Impl ) );
    mpImpl->pAssignPB->SetClickHdl( LINK(this, SfxMacroTabPage, AssignDeleteClickHdl_Impl ) );
    rListBox.SetDoubleClickHdl( aLnk );

    rListBox.SetSelectHdl( LINK( this, SfxMacroTabPage, SelectEvent_Impl ));
    mpImpl->pGroupLB->SetSelectHdl( LINK( this, SfxMacroTabPage, SelectGroup_Impl ));
    mpImpl->pMacroLB->SetSelectHdl( LINK( this, SfxMacroTabPage, SelectMacro_Impl ));

    rListBox.SetSelectionMode( SelectionMode::Single );
    rListBox.SetTabs( &nTabs[0] );
    Size aSize( nTabs[ 2 ], 0 );
    rHeaderBar.InsertItem( ITEMID_EVENT, mpImpl->sStrEvent, LogicToPixel( aSize, MapMode( MAP_APPFONT ) ).Width() );
    aSize.Width() = 1764;       // don't know what, so 42^2 is best to use...
    rHeaderBar.InsertItem( ITMEID_ASSMACRO, mpImpl->sAssignedMacro, LogicToPixel( aSize, MapMode( MAP_APPFONT ) ).Width() );
    rListBox.SetSpaceBetweenEntries( 0 );

    mpImpl->pEventLB->Show();
    mpImpl->pEventLB->ConnectElements();

    mpImpl->pEventLB->Enable();
    mpImpl->pGroupLB->Enable();
    mpImpl->pMacroLB->Enable();

    mpImpl->pGroupLB->SetFunctionListBox( mpImpl->pMacroLB );

}

void SfxMacroTabPage::FillMacroList()
{
    mpImpl->pGroupLB->Init(
        css::uno::Reference<
            css::uno::XComponentContext >(),
        GetFrame(),
        OUString(), false);
}

void SfxMacroTabPage::FillEvents()
{
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();

    sal_uLong       nEntryCnt = rListBox.GetEntryCount();

    // get events from the table and fill the EventListBox respectively
    for( sal_uLong n = 0 ; n < nEntryCnt ; ++n )
    {
        SvTreeListEntry*    pE = rListBox.GetEntry( n );
        if( pE )
        {
            SvLBoxString&     rLItem = static_cast<SvLBoxString&>( pE->GetItem( LB_MACROS_ITEMPOS ) );
            DBG_ASSERT( SV_ITEM_ID_LBOXSTRING == rLItem.GetType(), "SfxMacroTabPage::FillEvents(): no LBoxString" );

            OUString          sOld( rLItem.GetText() );
            OUString          sNew;
            sal_uInt16        nEventId = ( sal_uInt16 ) reinterpret_cast<sal_uLong>( pE->GetUserData() );
            if( aTbl.IsKeyValid( nEventId ) )
                sNew = ConvertToUIName_Impl( aTbl.Get( nEventId ) );

            if( sOld != sNew )
            {
                pE->ReplaceItem(o3tl::make_unique<SvLBoxString>(sNew), LB_MACROS_ITEMPOS);
                rListBox.GetModel()->InvalidateEntry( pE );
            }
        }
    }
}

namespace
{
    VclPtr<SfxMacroTabPage> CreateSfxMacroTabPage( vcl::Window* pParent, const SfxItemSet& rAttrSet )
    {
        return VclPtr<SfxMacroTabPage>::Create( pParent, nullptr, rAttrSet );
    }
}

VclPtr<SfxTabPage> SfxMacroTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return CreateSfxMacroTabPage(pParent, *rAttrSet);
}

SfxMacroAssignDlg::SfxMacroAssignDlg(vcl::Window* pParent,
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
