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

#define ITEMID_MACRO 0
#include <svl/macitem.hxx>
#undef ITEMID_MACRO

#include "macroass.hxx"

#include <basic/basmgr.hxx>
#include <comphelper/string.hxx>
#include <dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svtools/svmedit.hxx>
#include "cfgutil.hxx"
#include <sfx2/app.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/objsh.hxx>
#include "macroass.hrc"
#include "cuires.hrc"
#include <vcl/fixed.hxx>
#include "headertablistbox.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XFrame;

class _SfxMacroTabPage_Impl
{
public:
    _SfxMacroTabPage_Impl( void );
    ~_SfxMacroTabPage_Impl();

    String                          maStaticMacroLBLabel;
    PushButton*                     pAssignPB;
    PushButton*                     pDeletePB;
    String*                         pStrEvent;
    String*                         pAssignedMacro;
    _HeaderTabListBox*              pEventLB;
    SfxConfigGroupListBox_Impl*     pGroupLB;
    FixedText*                      pFT_MacroLBLabel;
    SfxConfigFunctionListBox_Impl*  pMacroLB;

    FixedText*                      pMacroFT;
    String*                         pMacroStr;

    sal_Bool                            bReadOnly;
    Timer                           maFillGroupTimer;
    sal_Bool                            bGotEvents;
};

_SfxMacroTabPage_Impl::_SfxMacroTabPage_Impl( void ) :
    pAssignPB( NULL ),
    pDeletePB( NULL ),
    pStrEvent( NULL ),
    pAssignedMacro( NULL ),
    pEventLB( NULL ),
    pGroupLB( NULL ),
    pFT_MacroLBLabel( NULL ),
    pMacroLB( NULL ),
    pMacroFT( NULL ),
    pMacroStr( NULL ),
    bReadOnly( sal_False ),
    bGotEvents( sal_False )
{
}

_SfxMacroTabPage_Impl::~_SfxMacroTabPage_Impl()
{
    delete pAssignPB;
    delete pDeletePB;
    delete pStrEvent;
    delete pAssignedMacro;
    delete pEventLB;
    delete pGroupLB;
    delete pMacroLB;
    delete pFT_MacroLBLabel;
    delete pMacroFT;
    delete pMacroStr;
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

String ConvertToUIName_Impl( SvxMacro *pMacro )
{
    String aName( pMacro->GetMacName() );
    String aEntry;
    if ( pMacro->GetLanguage() != "JavaScript" )
    {
        sal_uInt16 nCount = comphelper::string::getTokenCount(aName, '.');
        aEntry = aName.GetToken( nCount-1, '.' );
        if ( nCount > 2 )
        {
            aEntry += '(';
            aEntry += aName.GetToken( 0, '.' );
            aEntry += '.';
            aEntry += aName.GetToken( nCount-2, '.' );
            aEntry += ')';
        }
        return aEntry;
    }
    else
        return aName;
}

void _SfxMacroTabPage::EnableButtons()
{
    // don't do anything as long as the eventbox is empty
    const SvLBoxEntry* pE = mpImpl->pEventLB->GetListBox().FirstSelected();
    if ( pE )
    {
        // get bound macro
        const SvxMacro* pM = aTbl.Get( (sal_uInt16)(sal_uLong) pE->GetUserData() );
        mpImpl->pDeletePB->Enable( 0 != pM && !mpImpl->bReadOnly );

        String sEventMacro;
        sEventMacro = ((SvLBoxString*)pE->GetItem( LB_MACROS_ITEMPOS ))->GetText();

        String sScriptURI = mpImpl->pMacroLB->GetSelectedScriptURI();
        mpImpl->pAssignPB->Enable( !mpImpl->bReadOnly && !sScriptURI.EqualsIgnoreCaseAscii( sEventMacro ) );
    }
    else
        mpImpl->pAssignPB->Enable( sal_False );
}

_SfxMacroTabPage::_SfxMacroTabPage( Window* pParent, const ResId& rResId, const SfxItemSet& rAttrSet )
    : SfxTabPage( pParent, rResId, rAttrSet )

{
    mpImpl = new _SfxMacroTabPage_Impl;
}

_SfxMacroTabPage::~_SfxMacroTabPage()
{
    DELETEZ( mpImpl );
}

void _SfxMacroTabPage::AddEvent( const String & rEventName, sal_uInt16 nEventId )
{
    String sTmp( rEventName );
    sTmp += '\t';

    // if the table is valid already
    SvxMacro* pM = aTbl.Get( nEventId );
    if( pM )
    {
        String sNew( ConvertToUIName_Impl( pM ) );
        sTmp += sNew;
    }

    SvLBoxEntry* pE = mpImpl->pEventLB->GetListBox().InsertEntry( sTmp );
    pE->SetUserData( reinterpret_cast< void* >( sal::static_int_cast< sal_IntPtr >( nEventId )) );
}

void _SfxMacroTabPage::ScriptChanged()
{
    // get new areas and their functions
    {
        mpImpl->pGroupLB->Show();
        mpImpl->pMacroLB->Show();
        mpImpl->pMacroFT->SetText( *mpImpl->pMacroStr );
    }

    EnableButtons();
}

sal_Bool _SfxMacroTabPage::FillItemSet( SfxItemSet& rSet )
{
    SvxMacroItem aItem( GetWhich( aPageRg[0] ) );
    ((SvxMacroTableDtor&)aItem.GetMacroTable()) = aTbl;

    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET != GetItemSet().GetItemState( aItem.Which(), sal_True, &pItem )
        || aItem != *(SvxMacroItem*)pItem )
    {
        rSet.Put( aItem );
        return sal_True;
    }
    return sal_False;
}

void _SfxMacroTabPage::PageCreated (SfxAllItemSet aSet)
{
    const SfxPoolItem* pEventsItem;
    if( !mpImpl->bGotEvents && SFX_ITEM_SET == aSet.GetItemState( SID_EVENTCONFIG, sal_True, &pEventsItem ) )
    {
        mpImpl->bGotEvents = sal_True;
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
    if( SFX_ITEM_SET == rSet.GetItemState( GetWhich( aPageRg[0] ), sal_True, &pItem ))
        aTbl = ((SvxMacroItem*)pItem)->GetMacroTable();

    const SfxPoolItem* pEventsItem;
    if( !mpImpl->bGotEvents && SFX_ITEM_SET == rSet.GetItemState( SID_EVENTCONFIG, sal_True, &pEventsItem ) )
    {
        mpImpl->bGotEvents = sal_True;
        const SfxEventNamesList& rList = ((SfxEventNamesItem*)pEventsItem)->GetEvents();
        for ( size_t nNo = 0, nCnt = rList.size(); nNo < nCnt; ++nNo )
        {
            const SfxEventName *pOwn = rList.at(nNo);
            AddEvent( pOwn->maUIName, pOwn->mnId );
        }
    }

    FillEvents();

    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    SvLBoxEntry* pE = rListBox.GetEntry( 0 );
    if( pE )
        rListBox.SetCurEntry( pE );
}

sal_Bool _SfxMacroTabPage::IsReadOnly() const
{
    return mpImpl->bReadOnly;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, SelectEvent_Impl, SvTabListBox*, EMPTYARG )
{
    _SfxMacroTabPage_Impl*  pImpl = pThis->mpImpl;
    SvHeaderTabListBox&     rListBox = pImpl->pEventLB->GetListBox();
    SvLBoxEntry*            pE = rListBox.FirstSelected();
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
    String                  sSel( pImpl->pGroupLB->GetGroup() );
    pImpl->pGroupLB->GroupSelected();
    const String sScriptURI = pImpl->pMacroLB->GetSelectedScriptURI();
    String          aLabelText;
    if( sScriptURI.Len() > 0 )
        aLabelText = pImpl->maStaticMacroLBLabel;
    pImpl->pFT_MacroLBLabel->SetText( aLabelText );

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
    SvLBoxEntry* pE = rListBox.FirstSelected();
    sal_uLong nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = rListBox.GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return 0;
    }

    const sal_Bool bAssEnabled = pBtn != pImpl->pDeletePB && pImpl->pAssignPB->IsEnabled();

    // remove from the table
    sal_uInt16 nEvent = (sal_uInt16)(sal_uLong)pE->GetUserData();
    pThis->aTbl.Erase( nEvent );

    String sScriptURI;
    if( bAssEnabled )
    {
        sScriptURI = pImpl->pMacroLB->GetSelectedScriptURI();
        if( sScriptURI.CompareToAscii( "vnd.sun.star.script:", 20 ) == COMPARE_EQUAL )
        {
            pThis->aTbl.Insert(
                nEvent, SvxMacro( sScriptURI, rtl::OUString( SVX_MACRO_LANGUAGE_SF ) ) );
        }
        else
        {
            OSL_ENSURE( false, "_SfxMacroTabPage::AssignDeleteHdl_Impl: this branch is *not* dead? (out of interest: tell fs, please!)" );
            pThis->aTbl.Insert(
                nEvent, SvxMacro( sScriptURI, rtl::OUString( SVX_MACRO_LANGUAGE_STARBASIC ) ) );
        }
    }

    pImpl->pEventLB->SetUpdateMode( sal_False );
    pE->ReplaceItem( new SvLBoxString( pE, 0, sScriptURI ), LB_MACROS_ITEMPOS );
    rListBox.GetModel()->InvalidateEntry( pE );
    rListBox.Select( pE );
    rListBox.MakeVisible( pE );
    rListBox.SetUpdateMode( sal_True );

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
        pTabDlg->EnableInput( sal_False );
    }
    pThis->FillMacroList();
    if ( pTabDlg )
    {
        pTabDlg->EnableInput( sal_True );
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
    rHeaderBar.InsertItem( ITEMID_EVENT, *mpImpl->pStrEvent, LogicToPixel( aSize, MapMode( MAP_APPFONT ) ).Width() );
    aSize.Width() = 1764;       // don't know what, so 42^2 is best to use...
    rHeaderBar.InsertItem( ITMEID_ASSMACRO, *mpImpl->pAssignedMacro, LogicToPixel( aSize, MapMode( MAP_APPFONT ) ).Width() );
    rListBox.SetSpaceBetweenEntries( 0 );

    mpImpl->pEventLB->Show();
    mpImpl->pEventLB->ConnectElements();

    mpImpl->pEventLB->Enable( sal_True );
    mpImpl->pGroupLB->Enable( sal_True );
    mpImpl->pMacroLB->Enable( sal_True );

    mpImpl->pGroupLB->SetFunctionListBox( mpImpl->pMacroLB );

    mpImpl->maFillGroupTimer.SetTimeoutHdl( STATIC_LINK( this, _SfxMacroTabPage, TimeOut_Impl ) );
    mpImpl->maFillGroupTimer.SetTimeout( 0 );
    mpImpl->maFillGroupTimer.Start();
}

void _SfxMacroTabPage::FillMacroList()
{
    mpImpl->pGroupLB->Init(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory >(),
        GetFrame(),
        ::rtl::OUString() );
}

void _SfxMacroTabPage::FillEvents()
{
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();

    sal_uLong       nEntryCnt = rListBox.GetEntryCount();

    // get events from the table and fill the EventListBox respectively
    for( sal_uLong n = 0 ; n < nEntryCnt ; ++n )
    {
        SvLBoxEntry*    pE = rListBox.GetEntry( n );
        if( pE )
        {
            SvLBoxString*   pLItem = ( SvLBoxString* ) pE->GetItem( LB_MACROS_ITEMPOS );
            DBG_ASSERT( pLItem && SV_ITEM_ID_LBOXSTRING == pLItem->IsA(), "_SfxMacroTabPage::FillEvents(): no LBoxString" );

            String          sOld( pLItem->GetText() );
            String          sNew;
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

SfxMacroTabPage::SfxMacroTabPage( Window* pParent, const ResId& rResId, const Reference< XFrame >& rxDocumentFrame, const SfxItemSet& rSet )
    : _SfxMacroTabPage( pParent, rResId, rSet )
{
    mpImpl->pStrEvent           = new String(                   CUI_RES( STR_EVENT ) );
    mpImpl->pAssignedMacro      = new String(                   CUI_RES( STR_ASSMACRO ) );
    mpImpl->pEventLB            = new _HeaderTabListBox( this,  CUI_RES( LB_EVENT ) );
    mpImpl->pAssignPB           = new PushButton( this,         CUI_RES( PB_ASSIGN ) );
    mpImpl->pDeletePB           = new PushButton( this,         CUI_RES( PB_DELETE ) );
    mpImpl->pMacroFT            = new FixedText( this,          CUI_RES( FT_MACRO ) );
    mpImpl->pGroupLB            = new SfxConfigGroupListBox_Impl( this,     CUI_RES( LB_GROUP ) );
    mpImpl->pFT_MacroLBLabel    = new FixedText( this,          CUI_RES( FT_LABEL4LB_MACROS ) );
    mpImpl->maStaticMacroLBLabel= mpImpl->pFT_MacroLBLabel->GetText();
    mpImpl->pMacroLB            = new SfxConfigFunctionListBox_Impl( this,  CUI_RES( LB_MACROS ) );
    mpImpl->pMacroStr           = new String(                   CUI_RES( STR_MACROS ) );

    FreeResource();

    SetFrame( rxDocumentFrame );

    InitAndSetHandler();

    ScriptChanged();
}

SfxTabPage* SfxMacroTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SfxMacroTabPage( pParent, CUI_RES( RID_SVXPAGE_EVENTASSIGN ), NULL, rAttrSet );
}

SfxMacroAssignDlg::SfxMacroAssignDlg( Window* pParent, const Reference< XFrame >& rxDocumentFrame, const SfxItemSet& rSet )
    : SfxSingleTabDialog( pParent, rSet, 0 )
{
    SfxTabPage* pPage = SfxMacroTabPage::Create( this, rSet );
    pPage->SetFrame( rxDocumentFrame );
    SetTabPage( pPage );
}

SfxMacroAssignDlg::~SfxMacroAssignDlg()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
