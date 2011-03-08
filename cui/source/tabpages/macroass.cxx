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

#define ITEMID_MACRO 0
#include <svl/macitem.hxx>
#undef ITEMID_MACRO

#include "macroass.hxx"

#include <basic/basmgr.hxx>
#include <dialmgr.hxx>
#include <svx/dialogs.hrc>
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

#include <svtools/svmedit.hxx>
#include "cfgutil.hxx"
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include "macroass.hrc"
#include "cuires.hrc"
#include <sfx2/macrconf.hxx>
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
    ListBox*                        pScriptTypeLB;
    _HeaderTabListBox*              pEventLB;
    SfxConfigGroupListBox_Impl*     pGroupLB;
    FixedText*                      pFT_MacroLBLabel;
    SfxConfigFunctionListBox_Impl*  pMacroLB;

    FixedText*                      pMacroFT;
    String*                         pMacroStr;
    FNGetRangeHdl                   fnGetRange;
    FNGetMacrosOfRangeHdl           fnGetMacroOfRange;

    BOOL                            bReadOnly;
    Timer                           maFillGroupTimer;
    BOOL                            bGotEvents;
};

_SfxMacroTabPage_Impl::_SfxMacroTabPage_Impl( void ) :
    pAssignPB( NULL ),
    pDeletePB( NULL ),
    pStrEvent( NULL ),
    pAssignedMacro( NULL ),
    pScriptTypeLB( NULL ),
    pEventLB( NULL ),
    pGroupLB( NULL ),
    pFT_MacroLBLabel( NULL ),
    pMacroLB( NULL ),
    pMacroFT( NULL ),
    pMacroStr( NULL ),
    fnGetRange( NULL ),
    fnGetMacroOfRange( NULL ),
    bReadOnly( FALSE ),
    bGotEvents( FALSE )
{
}

_SfxMacroTabPage_Impl::~_SfxMacroTabPage_Impl()
{
    delete pAssignPB;
    delete pDeletePB;
    delete pStrEvent;
    delete pAssignedMacro;
    delete pScriptTypeLB;
    delete pEventLB;
    delete pGroupLB;
    delete pMacroLB;
    delete pFT_MacroLBLabel;
    delete pMacroFT;
    delete pMacroStr;
}


SvStringsDtor* _ImpGetRangeHdl( _SfxMacroTabPage*, const String& rLanguage );
SvStringsDtor* _ImpGetMacrosOfRangeHdl( _SfxMacroTabPage*, const String& rLanguage, const String& rRange );

static USHORT aPageRg[] = {
    SID_ATTR_MACROITEM, SID_ATTR_MACROITEM,
    0
};

// Achtung im Code wird dieses Array direkt (0, 1, ...) indiziert
static long nTabs[] =
    {
        2, // Number of Tabs
        0, 90
    };

#define TAB_WIDTH_MIN       10

// IDs for items in HeaderBar of EventLB
#define ITEMID_EVENT        1
#define ITMEID_ASSMACRO     2


#define LB_EVENTS_ITEMPOS   1
#define LB_MACROS_ITEMPOS   2

String ConvertToUIName_Impl( SvxMacro *pMacro, const String& /*rLanguage*/ )
{
    String aName( pMacro->GetMacName() );
    String aEntry;
    if ( ! pMacro->GetLanguage().EqualsAscii("JavaScript") )
    {
        USHORT nCount = aName.GetTokenCount('.');
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

void _SfxMacroTabPage::EnableButtons( const String& rLangName )
{
    // Solange die Eventbox leer ist, nichts tun
    const SvLBoxEntry* pE = mpImpl->pEventLB->GetListBox().FirstSelected();
    if ( pE )
    {
        // Gebundenes Macro holen
        const SvxMacro* pM = aTbl.Get( (USHORT)(ULONG) pE->GetUserData() );
        mpImpl->pDeletePB->Enable( 0 != pM && !mpImpl->bReadOnly );

        // Bei gleichem ScriptType Zuweisung nur, wenn Macro sich
        // ge"andert hat; bei verschiedenem ScriptType, wenn Script nicht leer
        String sEventMacro;
        sEventMacro = ((SvLBoxString*)pE->GetItem( LB_MACROS_ITEMPOS ))->GetText();
        if ( rLangName.EqualsAscii("JavaScript") )
        {
            OSL_FAIL( "_SfxMacroTabPage::EnableButtons(): this is not an up to date usage!" );
        }
        else
        {
            SfxMacroInfo* pInfo = mpImpl->pMacroLB->GetMacroInfo();
            String sSelMacro;
            if ( pInfo )
                sSelMacro = pInfo->GetMacroName();
            if( pM && rLangName != pM->GetLanguage() )
                mpImpl->pAssignPB->Enable( pInfo != 0 && !mpImpl->bReadOnly );
            else
                mpImpl->pAssignPB->Enable( pInfo && !mpImpl->bReadOnly && !sSelMacro.EqualsIgnoreCaseAscii( sEventMacro ) );
        }
    }
}

_SfxMacroTabPage::_SfxMacroTabPage( Window* pParent, const ResId& rResId, const SfxItemSet& rAttrSet )
    : SfxTabPage( pParent, rResId, rAttrSet )

{
    mpImpl = new _SfxMacroTabPage_Impl;
    mpImpl->fnGetRange = &_ImpGetRangeHdl;
    mpImpl->fnGetMacroOfRange = &_ImpGetMacrosOfRangeHdl;
}

_SfxMacroTabPage::~_SfxMacroTabPage()
{
    DELETEZ( mpImpl );
}

void _SfxMacroTabPage::AddEvent( const String & rEventName, USHORT nEventId )
{
    String sTmp( rEventName );
    sTmp += '\t';

    // falls die Tabelle schon gueltig ist
    SvxMacro* pM = aTbl.Get( nEventId );
    if( pM )
    {
        String sNew( ConvertToUIName_Impl( pM, mpImpl->pScriptTypeLB->GetSelectEntry() ) );
        sTmp += sNew;
    }

    SvLBoxEntry* pE = mpImpl->pEventLB->GetListBox().InsertEntry( sTmp );
    pE->SetUserData( reinterpret_cast< void* >( sal::static_int_cast< sal_IntPtr >( nEventId )) );
}

void _SfxMacroTabPage::ScriptChanged( const String& aLangName )
{
    // neue Bereiche und deren Funktionen besorgen
    {
        mpImpl->pGroupLB->SetScriptType( aLangName );
        mpImpl->pGroupLB->Show();
        mpImpl->pMacroLB->Show();
        mpImpl->pMacroFT->SetText( *mpImpl->pMacroStr );
    }

    EnableButtons( aLangName );
}

void _SfxMacroTabPage::SetGetRangeLink( FNGetRangeHdl pFn )
{
    mpImpl->fnGetRange = pFn;
}

FNGetRangeHdl _SfxMacroTabPage::GetGetRangeLink() const
{
    return mpImpl->fnGetRange;
}

void _SfxMacroTabPage::SetGetMacrosOfRangeLink( FNGetMacrosOfRangeHdl pFn )
{
    mpImpl->fnGetMacroOfRange = pFn;
}

FNGetMacrosOfRangeHdl _SfxMacroTabPage::GetGetMacrosOfRangeLink() const
{
    return mpImpl->fnGetMacroOfRange;
}

BOOL _SfxMacroTabPage::FillItemSet( SfxItemSet& rSet )
{
    SvxMacroItem aItem( GetWhich( aPageRg[0] ) );
    ((SvxMacroTableDtor&)aItem.GetMacroTable()) = aTbl;

    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET != GetItemSet().GetItemState( aItem.Which(), TRUE, &pItem )
        || aItem != *(SvxMacroItem*)pItem )
    {
        rSet.Put( aItem );
        return TRUE;
    }
    return FALSE;
}

void _SfxMacroTabPage::PageCreated (SfxAllItemSet aSet)
{
    const SfxPoolItem* pEventsItem;
    if( !mpImpl->bGotEvents && SFX_ITEM_SET == aSet.GetItemState( SID_EVENTCONFIG, TRUE, &pEventsItem ) )
    {
        mpImpl->bGotEvents = TRUE;
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
    if( SFX_ITEM_SET == rSet.GetItemState( GetWhich( aPageRg[0] ), TRUE, &pItem ))
        aTbl = ((SvxMacroItem*)pItem)->GetMacroTable();

    const SfxPoolItem* pEventsItem;
    if( !mpImpl->bGotEvents && SFX_ITEM_SET == rSet.GetItemState( SID_EVENTCONFIG, TRUE, &pEventsItem ) )
    {
        mpImpl->bGotEvents = TRUE;
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

void _SfxMacroTabPage::SetReadOnly( BOOL bSet )
{
    mpImpl->bReadOnly = bSet;
}

BOOL _SfxMacroTabPage::IsReadOnly() const
{
    return mpImpl->bReadOnly;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, SelectEvent_Impl, SvTabListBox*, EMPTYARG )
{
    _SfxMacroTabPage_Impl*  pImpl = pThis->mpImpl;
    SvHeaderTabListBox&     rListBox = pImpl->pEventLB->GetListBox();
    SvLBoxEntry*            pE = rListBox.FirstSelected();
    ULONG                   nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = rListBox.GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return 0;
    }

    USHORT nEventId = (USHORT)(ULONG)pE->GetUserData();
    String aLanguage = pImpl->pScriptTypeLB->GetSelectEntry();

    const SvxMacro* pM = pThis->aTbl.Get( nEventId );
    if( pM )
    {
        if( aLanguage != pM->GetLanguage() )
        {
            pImpl->pScriptTypeLB->SelectEntry( pM->GetLanguage() );
            pThis->ScriptChanged( pM->GetLanguage() );
        }
        else
        {
            DBG_ASSERT( !aLanguage.EqualsAscii("JavaScript"), "_SfxMacroTabPage, SelectEvent_Impl(): outdated use!" );
        }
    }

    pThis->EnableButtons( aLanguage );
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, SelectGroup_Impl, ListBox*, EMPTYARG )
{
    _SfxMacroTabPage_Impl*  pImpl = pThis->mpImpl;
    String                  sSel( pImpl->pGroupLB->GetGroup() );
    String                  aLanguage = pImpl->pScriptTypeLB->GetSelectEntry();
    if( !aLanguage.EqualsAscii( "JavaScript" ) )
    {
        pImpl->pGroupLB->GroupSelected();
        SfxMacroInfo*   pMacro = pImpl->pMacroLB->GetMacroInfo();
        String          aLabelText;
        if( pMacro )
        {
            aLabelText = pImpl->maStaticMacroLBLabel;
            aLabelText += pMacro->GetModuleName();
        }
        else
        {
            // Wenn dort ein Macro drin ist, wurde es selektiert und der
            // AssignButton schon in SelectMacro richtig enabled
            pImpl->pAssignPB->Enable( FALSE );
        }

        pImpl->pFT_MacroLBLabel->SetText( aLabelText );
    }
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, SelectMacro_Impl, ListBox*, EMPTYARG )
{
    _SfxMacroTabPage_Impl*  pImpl = pThis->mpImpl;
    pImpl->pMacroLB->FunctionSelected();
    pThis->EnableButtons( pImpl->pScriptTypeLB->GetSelectEntry() );
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, GetFocus_Impl, Edit*, EMPTYARG )
{
    pThis->EnableButtons( DEFINE_CONST_UNICODE("JavaScript") );
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, AssignDeleteHdl_Impl, PushButton*, pBtn )
{
    _SfxMacroTabPage_Impl*  pImpl = pThis->mpImpl;
    SvHeaderTabListBox& rListBox = pImpl->pEventLB->GetListBox();
    SvLBoxEntry* pE = rListBox.FirstSelected();
    ULONG nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = rListBox.GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return 0;
    }

    const BOOL bAssEnabled = pBtn != pImpl->pDeletePB && pImpl->pAssignPB->IsEnabled();

    // aus der Tabelle entfernen
    USHORT nEvent = (USHORT)(ULONG)pE->GetUserData();
    SvxMacro *pRemoveMacro = pThis->aTbl.Remove( nEvent );
    delete pRemoveMacro;

    String aLanguage = pImpl->pScriptTypeLB->GetSelectEntry();
    String sNew;
    if( bAssEnabled )
    {
        String sGroup;
        String sMacro;
        String aEntryText( sNew );
        DBG_ASSERT( !aLanguage.EqualsAscii("JavaScript"), "_SfxMacroTabPage, AssignDeleteHdl_Impl(): outdated use!" );

        SfxMacroInfo* pMacro = pImpl->pMacroLB->GetMacroInfo();
        sMacro = pMacro->GetQualifiedName();
        sGroup = pImpl->pGroupLB->GetGroup();
        sNew = pMacro->GetMacroName();

        if( sMacro.CompareToAscii( "vnd.sun.star.script:", 20 ) == COMPARE_EQUAL )
        {
            OSL_TRACE("ASSIGN_DELETE: Its a script");
            pThis->aTbl.Insert(
                nEvent, new SvxMacro( sMacro, String::CreateFromAscii("Script") ) );
        }
        else
        {
            OSL_TRACE("ASSIGN_DELETE: Its a basic macro");
            String sBasicName/*(SfxResId(STR_BASICNAME))*/;
            if ( aLanguage == sBasicName )
                pThis->aTbl.Insert( nEvent, new SvxMacro( sMacro, sGroup, STARBASIC ) );
            else
                pThis->aTbl.Insert( nEvent, new SvxMacro( sMacro, aLanguage ) );
        }
    }

    pImpl->pEventLB->SetUpdateMode( FALSE );
    pE->ReplaceItem( new SvLBoxString( pE, 0, sNew ), LB_MACROS_ITEMPOS );
    rListBox.GetModel()->InvalidateEntry( pE );
    rListBox.Select( pE );
    rListBox.MakeVisible( pE );
    rListBox.SetUpdateMode( TRUE );

    pThis->EnableButtons( aLanguage );
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, ChangeScriptHdl_Impl, RadioButton*, EMPTYARG )
{
    pThis->ScriptChanged( pThis->mpImpl->pScriptTypeLB->GetSelectEntry() );
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
        pTabDlg->EnableInput( FALSE );
    }
    pThis->FillMacroList();
    if ( pTabDlg )
    {
        pTabDlg->EnableInput( TRUE );
        pTabDlg->LeaveWait();
    }
    return 0;
}

void _SfxMacroTabPage::InitAndSetHandler()
{
    // Handler installieren
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

    mpImpl->pScriptTypeLB->SetSelectHdl( STATIC_LINK( this, _SfxMacroTabPage, ChangeScriptHdl_Impl ));

    rListBox.SetSelectionMode( SINGLE_SELECTION );
    rListBox.SetTabs( &nTabs[0], MAP_APPFONT );
    Size aSize( nTabs[ 2 ], 0 );
    rHeaderBar.InsertItem( ITEMID_EVENT, *mpImpl->pStrEvent, LogicToPixel( aSize, MapMode( MAP_APPFONT ) ).Width() );
    aSize.Width() = 1764;       // don't know what, so 42^2 is best to use...
    rHeaderBar.InsertItem( ITMEID_ASSMACRO, *mpImpl->pAssignedMacro, LogicToPixel( aSize, MapMode( MAP_APPFONT ) ).Width() );
    rListBox.SetSpaceBetweenEntries( 0 );

    mpImpl->pEventLB->Show();
    mpImpl->pEventLB->ConnectElements();

    mpImpl->pEventLB->Enable( TRUE );
    mpImpl->pGroupLB->Enable( TRUE );
    mpImpl->pMacroLB->Enable( TRUE );

    mpImpl->pScriptTypeLB->SetDropDownLineCount( 3 );
    String sBasicName/*(SfxResId(STR_BASICNAME))*/;
    mpImpl->pScriptTypeLB->InsertEntry( sBasicName );
    mpImpl->pScriptTypeLB->SelectEntry( sBasicName );

    mpImpl->pGroupLB->SetFunctionListBox( mpImpl->pMacroLB );

    mpImpl->maFillGroupTimer.SetTimeoutHdl( STATIC_LINK( this, _SfxMacroTabPage, TimeOut_Impl ) );
    mpImpl->maFillGroupTimer.SetTimeout( 0 );
    mpImpl->maFillGroupTimer.Start();
}

void _SfxMacroTabPage::FillMacroList()
{
    String aLanguage = mpImpl->pScriptTypeLB->GetSelectEntry();
    if( ! aLanguage.EqualsAscii("JavaScript") )
    {
        // 2 Listboxen
        SvStringsDtor* pArr = (*mpImpl->fnGetRange)( this, String(/*SfxResId(STR_BASICNAME)*/) );
        if( pArr )
        {
            mpImpl->pGroupLB->Init(
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory >(),
                GetFrame(),
                ::rtl::OUString() );

            delete pArr;
        }
    }
}

void _SfxMacroTabPage::FillEvents()
{
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();

    String              aLanguage = mpImpl->pScriptTypeLB->GetSelectEntry();
    ULONG               nEntryCnt = rListBox.GetEntryCount();

    // Events aus der Tabelle holen und die EventListBox entsprechen fuellen
    for( ULONG n = 0 ; n < nEntryCnt ; ++n )
    {
        SvLBoxEntry*    pE = rListBox.GetEntry( n );
        if( pE )
        {
            SvLBoxString*   pLItem = ( SvLBoxString* ) pE->GetItem( LB_MACROS_ITEMPOS );
            DBG_ASSERT( pLItem && SV_ITEM_ID_LBOXSTRING == pLItem->IsA(), "_SfxMacroTabPage::FillEvents(): no LBoxString" );

            String          sOld( pLItem->GetText() );
            String          sNew;
            USHORT          nEventId = ( USHORT ) ( ULONG ) pE->GetUserData();
            if( aTbl.IsKeyValid( nEventId ) )
                sNew = ConvertToUIName_Impl( aTbl.Get( nEventId ), aLanguage );

            if( sOld != sNew )
            {
                pE->ReplaceItem( new SvLBoxString( pE, 0, sNew ), LB_MACROS_ITEMPOS );
                rListBox.GetModel()->InvalidateEntry( pE );
            }
        }
    }
}

void _SfxMacroTabPage::SelectEvent( const String & /*rEventName*/, USHORT nEventId )
{
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    ULONG               nEntryCnt = rListBox.GetEntryCount();

    for( ULONG n = 0 ; n < nEntryCnt ; ++n )
    {
        SvLBoxEntry*    pE = rListBox.GetEntry( n );
        if( pE && ( USHORT ) ( ULONG ) pE->GetUserData() == nEventId )
        {
            rListBox.SetCurEntry( pE );
            rListBox.MakeVisible( pE );
            break;
        }
    }
}


SvStringsDtor* _ImpGetRangeHdl( _SfxMacroTabPage* /*pTbPg*/, const String& rLanguage )
{
    SvStringsDtor* pNew = new SvStringsDtor;
    SfxApplication* pSfxApp = SFX_APP();

    if ( !rLanguage.EqualsAscii("JavaScript") )
    {
        pSfxApp->EnterBasicCall();

        // AppBasic einf"ugen
        String* pNewEntry = new String( pSfxApp->GetName() );
        pNew->Insert( pNewEntry, pNew->Count() );

        // Aktuelles Dokument
        SfxObjectShell* pDoc = SfxObjectShell::Current();
        if ( pDoc )
        {
            String aTitle = pDoc->GetTitle();

            // Hack f"ur Aufruf aus der Basic-IDE : das Basic ermitteln, das
            // gerade bearbeitet wird

            String aAppName(DEFINE_CONST_UNICODE("BASIC - "));
            USHORT nLen = aAppName.Len();
            if ( aTitle.CompareIgnoreCaseToAscii( aAppName, nLen ) == COMPARE_EQUAL )
            {
                // Basic-Namensprefix entfernen
                aTitle.Erase( 0, nLen );
                USHORT nIndex=0, nCount=aTitle.GetTokenCount('.');
                if ( nCount > 1 )
                {
                    // Namen der Library entfernen
                    aTitle.GetToken( nCount-2, '.', nIndex );
                    aTitle.Erase( nIndex-1 );
                }

                // Wenn das App-Basic gerade in der Basic-IDE bearbeitet wird, kein
                // Dokument verwenden
                pDoc = SfxObjectShell::GetFirst();
                while( pDoc )
                {
                    if ( aTitle == pDoc->GetTitle() )
                        break;
                    pDoc = SfxObjectShell::GetNext( *pDoc );
                }
            }

            if ( pDoc && pDoc->GetBasicManager() != SFX_APP()->GetBasicManager() &&
                pDoc->GetBasicManager()->GetLibCount() )
            {
                pNewEntry = new String( aTitle );
                pNew->Insert( pNewEntry, pNew->Count() );
            }
        }

        pSfxApp->LeaveBasicCall();
    }

    return pNew;
}

// besorgen der Funktionen eines Bereiches
SvStringsDtor* _ImpGetMacrosOfRangeHdl(
    _SfxMacroTabPage* /*pTbPg*/,
    const String& /*rLanguage*/,
    const String& /*rRange*/ )
{
    SvStringsDtor* pNew = new SvStringsDtor;
    return pNew;
}


SfxMacroTabPage::SfxMacroTabPage( Window* pParent, const ResId& rResId, const Reference< XFrame >& rxDocumentFrame, const SfxItemSet& rSet )
    : _SfxMacroTabPage( pParent, rResId, rSet )
{
    mpImpl->pStrEvent           = new String(                   CUI_RES( STR_EVENT ) );
    mpImpl->pAssignedMacro      = new String(                   CUI_RES( STR_ASSMACRO ) );
    mpImpl->pEventLB            = new _HeaderTabListBox( this,  CUI_RES( LB_EVENT ) );
    mpImpl->pAssignPB           = new PushButton( this,         CUI_RES( PB_ASSIGN ) );
    mpImpl->pDeletePB           = new PushButton( this,         CUI_RES( PB_DELETE ) );
    mpImpl->pScriptTypeLB       = new ListBox(this,             CUI_RES( LB_SCRIPTTYPE ) );
    mpImpl->pScriptTypeLB->Hide();
    mpImpl->pMacroFT            = new FixedText( this,          CUI_RES( FT_MACRO ) );
    mpImpl->pGroupLB            = new SfxConfigGroupListBox_Impl( this,     CUI_RES( LB_GROUP ) );
    mpImpl->pFT_MacroLBLabel    = new FixedText( this,          CUI_RES( FT_LABEL4LB_MACROS ) );
    mpImpl->maStaticMacroLBLabel= mpImpl->pFT_MacroLBLabel->GetText();
    mpImpl->pMacroLB            = new SfxConfigFunctionListBox_Impl( this,  CUI_RES( LB_MACROS ) );
    mpImpl->pMacroStr           = new String(                   CUI_RES( STR_MACROS ) );

    FreeResource();

    SetFrame( rxDocumentFrame );

    InitAndSetHandler();

    ScriptChanged( String( /*SfxResId(STR_BASICNAME)*/ ) );
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
