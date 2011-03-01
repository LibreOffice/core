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

#include <limits.h>
#include <stdlib.h>
#include <vcl/msgbox.hxx>
#include <unotools/viewoptions.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>

#include "appdata.hxx"
#include "sfxtypes.hxx"
#include <sfx2/minarray.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>
#include "sfxresid.hxx"
#include "sfxhelp.hxx"
#include <sfx2/ctrlitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/itemconnect.hxx>

#include "dialog.hrc"
#include "helpid.hrc"

#if ENABLE_LAYOUT_SFX_TABDIALOG
#undef TabPage
#undef SfxTabPage
#define SfxTabPage ::SfxTabPage
#undef SfxTabDialog
#endif /* ENABLE_LAYOUT_SFX_TABDIALOG */

using namespace ::com::sun::star::uno;
using namespace ::rtl;

#define USERITEM_NAME           OUString(RTL_CONSTASCII_USTRINGPARAM("UserItem"))

TYPEINIT1(LAYOUT_NS_SFX_TABDIALOG SfxTabDialogItem,SfxSetItem);

struct TabPageImpl
{
    BOOL                        mbStandard;
    sfx::ItemConnectionArray    maItemConn;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;

    TabPageImpl() : mbStandard( FALSE ) {}
};

NAMESPACE_LAYOUT_SFX_TABDIALOG

struct Data_Impl
{
    USHORT nId;                  // Die ID
    CreateTabPage fnCreatePage;  // Pointer auf die Factory
    GetTabPageRanges fnGetRanges;// Pointer auf die Ranges-Funktion
    SfxTabPage* pTabPage;        // die TabPage selber
    BOOL bOnDemand;              // Flag: ItemSet onDemand
    BOOL bRefresh;               // Flag: Seite mu\s neu initialisiert werden

    // Konstruktor
    Data_Impl( USHORT Id, CreateTabPage fnPage,
               GetTabPageRanges fnRanges, BOOL bDemand ) :

        nId         ( Id ),
        fnCreatePage( fnPage ),
        fnGetRanges ( fnRanges ),
        pTabPage    ( 0 ),
        bOnDemand   ( bDemand ),
        bRefresh    ( FALSE )
    {
        if ( !fnCreatePage  )
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            if ( pFact )
            {
                fnCreatePage = pFact->GetTabPageCreatorFunc( nId );
                fnGetRanges = pFact->GetTabPageRangesFunc( nId );
            }
        }
    }
};

SfxTabDialogItem::SfxTabDialogItem( const SfxTabDialogItem& rAttr, SfxItemPool* pItemPool )
    : SfxSetItem( rAttr, pItemPool )
{
}

SfxTabDialogItem::SfxTabDialogItem( USHORT nId, const SfxItemSet& rItemSet )
    : SfxSetItem( nId, rItemSet )
{
}

SfxPoolItem* SfxTabDialogItem::Clone(SfxItemPool* pToPool) const
{
    return new SfxTabDialogItem( *this, pToPool );
}

SfxPoolItem* SfxTabDialogItem::Create(SvStream& /*rStream*/, USHORT /*nVersion*/) const
{
    OSL_FAIL( "Use it only in UI!" );
    return NULL;
}

class SfxTabDialogController : public SfxControllerItem
{
    SfxTabDialog*   pDialog;
    const SfxItemSet*     pSet;
public:
                    SfxTabDialogController( USHORT nSlotId, SfxBindings& rBindings, SfxTabDialog* pDlg )
                        : SfxControllerItem( nSlotId, rBindings )
                        , pDialog( pDlg )
                        , pSet( NULL )
                    {}

                    ~SfxTabDialogController();

    DECL_LINK(      Execute_Impl, void* );
    virtual void    StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
};

SfxTabDialogController::~SfxTabDialogController()
{
    delete pSet;
}

IMPL_LINK( SfxTabDialogController, Execute_Impl, void*, pVoid )
{
    (void)pVoid; //unused
    if ( pDialog->OK_Impl() && pDialog->Ok() )
    {
        const SfxPoolItem* aItems[2];
        SfxTabDialogItem aItem( GetId(), *pDialog->GetOutputItemSet() );
        aItems[0] = &aItem;
        aItems[1] = NULL;
        GetBindings().Execute( GetId(), aItems );
    }

    return 0;
}

void SfxTabDialogController::StateChanged( USHORT /*nSID*/, SfxItemState /*eState*/, const SfxPoolItem* pState )
{
    const SfxSetItem* pSetItem = PTR_CAST( SfxSetItem, pState );
    if ( pSetItem )
    {
        pSet = pDialog->pSet = pSetItem->GetItemSet().Clone();
        BOOL bDialogStarted = FALSE;
        for ( USHORT n=0; n<pDialog->aTabCtrl.GetPageCount(); n++ )
        {
            USHORT nPageId = pDialog->aTabCtrl.GetPageId( n );
            SfxTabPage* pTabPage = dynamic_cast<SfxTabPage*> (pDialog->aTabCtrl.GetTabPage( nPageId ));
            if ( pTabPage )
            {
                pTabPage->Reset( pSetItem->GetItemSet() );
                bDialogStarted = TRUE;
            }
        }

        if ( bDialogStarted )
            pDialog->Show();
    }
    else
        pDialog->Hide();
}

DECL_PTRARRAY(SfxTabDlgData_Impl, Data_Impl *, 4,4)

struct TabDlg_Impl
{
    BOOL                bModified       : 1,
                        bModal          : 1,
                        bInOK           : 1,
                        bHideResetBtn   : 1;
    SfxTabDlgData_Impl* pData;

    PushButton*         pApplyButton;
    SfxTabDialogController* pController;

    TabDlg_Impl( BYTE nCnt ) :

        bModified       ( FALSE ),
        bModal          ( TRUE ),
        bInOK           ( FALSE ),
        bHideResetBtn   ( FALSE ),
        pData           ( new SfxTabDlgData_Impl( nCnt ) ),
        pApplyButton    ( NULL ),
        pController     ( NULL )
    {}
};

Data_Impl* Find( SfxTabDlgData_Impl& rArr, USHORT nId, USHORT* pPos = 0 );

Data_Impl* Find( SfxTabDlgData_Impl& rArr, USHORT nId, USHORT* pPos )
{
    const USHORT nCount = rArr.Count();

    for ( USHORT i = 0; i < nCount; ++i )
    {
        Data_Impl* pObj = rArr[i];

        if ( pObj->nId == nId )
        {
            if ( pPos )
                *pPos = i;
            return pObj;
        }
    }
    return 0;
}

#if !ENABLE_LAYOUT_SFX_TABDIALOG

void SfxTabPage::SetFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame)
{
    if (pImpl)
        pImpl->mxFrame = xFrame;
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SfxTabPage::GetFrame()
{
    if (pImpl)
        return pImpl->mxFrame;
    return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >();
}

SfxTabPage::SfxTabPage( Window *pParent,
                        const ResId &rResId, const SfxItemSet &rAttrSet ) :

/*  [Beschreibung]

    Konstruktor
*/

    TabPage( pParent, rResId ),

    pSet                ( &rAttrSet ),
    bHasExchangeSupport ( FALSE ),
    pTabDlg             ( NULL ),
    pImpl               ( new TabPageImpl )

{
}
// -----------------------------------------------------------------------
SfxTabPage:: SfxTabPage( Window *pParent, WinBits nStyle, const SfxItemSet &rAttrSet ) :
    TabPage(pParent, nStyle),
    pSet                ( &rAttrSet ),
    bHasExchangeSupport ( FALSE ),
    pTabDlg             ( NULL ),
    pImpl               ( new TabPageImpl )
{
}
// -----------------------------------------------------------------------

SfxTabPage::~SfxTabPage()

/*  [Beschreibung]

    Destruktor
*/

{
#if !ENABLE_LAYOUT
    delete pImpl;
#endif /* ENABLE_LAYOUT */
}

// -----------------------------------------------------------------------

BOOL SfxTabPage::FillItemSet( SfxItemSet& rSet )
{
    return pImpl->maItemConn.DoFillItemSet( rSet, GetItemSet() );
}

// -----------------------------------------------------------------------

void SfxTabPage::Reset( const SfxItemSet& rSet )
{
    pImpl->maItemConn.DoApplyFlags( rSet );
    pImpl->maItemConn.DoReset( rSet );
}

// -----------------------------------------------------------------------

void SfxTabPage::ActivatePage( const SfxItemSet& )

/*  [Beschreibung]

    Defaultimplementierung der virtuellen ActivatePage-Methode
    Diese wird gerufen, wenn eine Seite des Dialogs den Datenaustausch
    zwischen Pages unterst"utzt.

    <SfxTabPage::DeactivatePage(SfxItemSet *)>
*/

{
}

// -----------------------------------------------------------------------

int SfxTabPage::DeactivatePage( SfxItemSet* )

/*  [Beschreibung]

    Defaultimplementierung der virtuellen DeactivatePage-Methode
    Diese wird vor dem Verlassen einer Seite durch den Sfx gerufen;
    die Anwendung kann "uber den Returnwert steuern,
    ob die Seite verlassen werden soll.
    Falls die Seite "uber bHasExchangeSupport
    anzeigt, da\s sie einen Datenaustausch zwischen Seiten
    unterst"utzt, wird ein Pointer auf das Austausch-Set als
    Parameter "ubergeben. Dieser nimmt die Daten f"ur den Austausch
    entgegen; das Set steht anschlie\send als Parameter in
    <SfxTabPage::ActivatePage(const SfxItemSet &)> zur Verf"ugung.

    [R"uckgabewert]

    LEAVE_PAGE; Verlassen der Seite erlauben
*/

{
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void SfxTabPage::FillUserData()

/*  [Beschreibung]

   virtuelle Methode, wird von der Basisklasse im Destruktor gerufen
   um spezielle Informationen der TabPage in der Ini-Datei zu speichern.
   Beim "Uberladen muss ein String zusammengestellt werden, der mit
   <SetUserData()> dann weggeschrieben wird.
*/

{
}

// -----------------------------------------------------------------------

BOOL SfxTabPage::IsReadOnly() const
{
    return FALSE;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxTabPage::GetItem( const SfxItemSet& rSet, USHORT nSlot, sal_Bool bDeep )

/*  [Beschreibung]

    static Methode: hiermit wird der Code der TabPage-Implementierungen
    vereinfacht.

*/

{
    const SfxItemPool* pPool = rSet.GetPool();
    USHORT nWh = pPool->GetWhich( nSlot, bDeep );
    const SfxPoolItem* pItem = 0;
    rSet.GetItemState( nWh, TRUE, &pItem );

    if ( !pItem && nWh != nSlot )
        pItem = &pPool->GetDefaultItem( nWh );
    return pItem;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxTabPage::GetOldItem( const SfxItemSet& rSet,
                                           USHORT nSlot, sal_Bool bDeep )

/*  [Beschreibung]

    Diese Methode gibt f"ur Vergleiche den alten Wert eines
    Attributs zur"uck.
*/

{
    const SfxItemSet& rOldSet = GetItemSet();
    USHORT nWh = GetWhich( nSlot, bDeep );
    const SfxPoolItem* pItem = 0;

    if ( pImpl->mbStandard && rOldSet.GetParent() )
        pItem = GetItem( *rOldSet.GetParent(), nSlot );
    else if ( rSet.GetParent() &&
              SFX_ITEM_DONTCARE == rSet.GetItemState( nWh ) )
        pItem = GetItem( *rSet.GetParent(), nSlot );
    else
        pItem = GetItem( rOldSet, nSlot );
    return pItem;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxTabPage::GetExchangeItem( const SfxItemSet& rSet,
                                                USHORT nSlot )

/*  [Beschreibung]

    Diese Methode gibt f"ur Vergleiche den alten Wert eines
    Attributs zur"uck. Dabei wird ber"ucksichtigt, ob der Dialog
    gerade mit OK beendet wurde.
*/

{
    if ( pTabDlg && !pTabDlg->IsInOK() && pTabDlg->GetExampleSet() )
        return GetItem( *pTabDlg->GetExampleSet(), nSlot );
    else
        return GetOldItem( rSet, nSlot );
}


void SfxTabPage::PageCreated( SfxAllItemSet /*aSet*/ )
{
    DBG_ASSERT(0, "SfxTabPage::PageCreated should not be called");
}

// -----------------------------------------------------------------------

void SfxTabPage::AddItemConnection( sfx::ItemConnectionBase* pConnection )
{
    pImpl->maItemConn.AddConnection( pConnection );
}

#endif /* !ENABLE_LAYOUT_SFX_TABDIALOG */

#if ENABLE_LAYOUT_SFX_TABDIALOG
#undef ResId
#define ResId(id, foo) #id
#undef TabDialog
#define TabDialog(parent, res_id) Dialog (parent, "tab-dialog.xml", "tab-dialog")

#define aOKBtn(this) aOKBtn (this, "BTN_OK")
#undef PushButton
#define PushButton(this) layout::PushButton (this, "BTN_USER")
#define aCancelBtn(this) aCancelBtn (this, "BTN_CANCEL")
#define aHelpBtn(this) aHelpBtn (this, "BTN_HELP")
#define aResetBtn(this) aResetBtn (this, "BTN_RESET")
#define aBaseFmtBtn(this) aBaseFmtBtn (this, "BTN_BASEFMT")
#endif /* ENABLE_LAYOUT_SFX_TABDIALOG */

#define INI_LIST(ItemSetPtr) \
    aTabCtrl    ( this, ResId(ID_TABCONTROL,*rResId.GetResMgr() ) ),\
    aOKBtn      ( this ),\
    pUserBtn    ( pUserButtonText? new PushButton(this): 0 ),\
    aCancelBtn  ( this ),\
    aHelpBtn    ( this ),\
    aResetBtn   ( this ),\
    aBaseFmtBtn ( this ),\
    pSet        ( ItemSetPtr ),\
    pOutSet     ( 0 ),\
    pImpl       ( new TabDlg_Impl( (BYTE)aTabCtrl.GetPageCount() ) ), \
    pRanges     ( 0 ), \
    nResId      ( rResId.GetId() ), \
    nAppPageId  ( USHRT_MAX ), \
    bItemsReset ( FALSE ),\
    bFmt        ( bEditFmt ),\
    pExampleSet ( 0 )

// -----------------------------------------------------------------------

SfxTabDialog::SfxTabDialog

/*  [Beschreibung]

    Konstruktor
*/

(
    SfxViewFrame* pViewFrame,       // Frame, zu dem der Dialog geh"ort
    Window* pParent,                // Parent-Fenster
    const ResId& rResId,            // ResourceId
    const SfxItemSet* pItemSet,     // Itemset mit den Daten;
                                    // kann NULL sein, wenn Pages onDemand
    BOOL bEditFmt,      // Flag: es werden Vorlagen bearbeitet
                        // wenn ja -> zus"atzlicher Button f"ur Standard
    const String* pUserButtonText   // Text fuer BenutzerButton;
                                    // wenn != 0, wird der UserButton erzeugt
) :
    TabDialog( pParent, rResId ),
    pFrame( pViewFrame ),
    INI_LIST(pItemSet)
{
    Init_Impl( bFmt, pUserButtonText );
}

// -----------------------------------------------------------------------

SfxTabDialog::SfxTabDialog

/*  [Beschreibung]

    Konstruktor, tempor"ar ohne Frame
*/

(
    Window* pParent,                // Parent-Fenster
    const ResId& rResId,            // ResourceId
    const SfxItemSet* pItemSet,     // Itemset mit den Daten; kann NULL sein,
                                    // wenn Pages onDemand
    BOOL bEditFmt,      // Flag: es werden Vorlagen bearbeitet
                        // wenn ja -> zus"atzlicher Button f"ur Standard
    const String* pUserButtonText   // Text f"ur BenutzerButton;
                                    // wenn != 0, wird der UserButton erzeugt
) :
    TabDialog( pParent, rResId ),
    pFrame( 0 ),
    INI_LIST(pItemSet)
{
    Init_Impl( bFmt, pUserButtonText );
    DBG_WARNING( "bitte den Ctor mit ViewFrame verwenden" );
}

SfxTabDialog::SfxTabDialog

/*  [Beschreibung]

    Konstruktor, tempor"ar ohne Frame
*/

(
    Window* pParent,                // Parent-Fenster
    const ResId& rResId,            // ResourceId
    USHORT nSetId,
    SfxBindings& rBindings,
    BOOL bEditFmt,      // Flag: es werden Vorlagen bearbeitet
                        // wenn ja -> zus"atzlicher Button f"ur Standard
    const String* pUserButtonText   // Text f"ur BenutzerButton;
                                    // wenn != 0, wird der UserButton erzeugt
) :
    TabDialog( pParent, rResId ),
    pFrame( 0 ),
    INI_LIST(NULL)
{
    rBindings.ENTERREGISTRATIONS();
    pImpl->pController = new SfxTabDialogController( nSetId, rBindings, this );
    rBindings.LEAVEREGISTRATIONS();

    EnableApplyButton( TRUE );
    SetApplyHandler( LINK( pImpl->pController, SfxTabDialogController, Execute_Impl ) );

    rBindings.Invalidate( nSetId );
    rBindings.Update( nSetId );
    DBG_ASSERT( pSet, "No ItemSet!" );

    Init_Impl( bFmt, pUserButtonText );
}

// -----------------------------------------------------------------------

#if ENABLE_LAYOUT_SFX_TABDIALOG
#undef ResId
#undef TabDialog
#undef aOKBtn
#undef PushButton
#undef aCancelBtn
#undef aHelpBtn
#undef aResetBtn
#undef aBaseFmtBtn
#endif /* ENABLE_LAYOUT_SFX_TABDIALOG */

SfxTabDialog::~SfxTabDialog()
{
    // save settings (screen position and current page)
    SvtViewOptions aDlgOpt( E_TABDIALOG, String::CreateFromInt32( nResId ) );
#if !ENABLE_LAYOUT_SFX_TABDIALOG
    aDlgOpt.SetWindowState( OUString::createFromAscii( GetWindowState( WINDOWSTATE_MASK_POS ).GetBuffer() ) );
#endif /* !ENABLE_LAYOUT_SFX_TABDIALOG */
    aDlgOpt.SetPageID( aTabCtrl.GetCurPageId() );

    const USHORT nCount = pImpl->pData->Count();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        Data_Impl* pDataObject = pImpl->pData->GetObject(i);

        if ( pDataObject->pTabPage )
        {
            // save settings of all pages (user data)
            pDataObject->pTabPage->FillUserData();
            String aPageData( pDataObject->pTabPage->GetUserData() );
            if ( aPageData.Len() )
            {
                // save settings of all pages (user data)
                SvtViewOptions aPageOpt( E_TABPAGE, String::CreateFromInt32( pDataObject->nId ) );
                aPageOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( aPageData ) ) );
            }

            if ( pDataObject->bOnDemand )
                delete (SfxItemSet*)&pDataObject->pTabPage->GetItemSet();
            delete pDataObject->pTabPage;
        }
        delete pDataObject;
    }

    delete pImpl->pController;
    delete pImpl->pApplyButton;
    delete pImpl->pData;
    delete pImpl;
    delete pUserBtn;
    delete pOutSet;
    delete pExampleSet;
    delete [] pRanges;
}

// -----------------------------------------------------------------------

void SfxTabDialog::Init_Impl( BOOL bFmtFlag, const String* pUserButtonText )

/*  [Beschreibung]

    interne Initialisierung des Dialogs
*/

{
    aOKBtn.SetClickHdl( LINK( this, SfxTabDialog, OkHdl ) );
    aResetBtn.SetClickHdl( LINK( this, SfxTabDialog, ResetHdl ) );
    aResetBtn.SetText( String( SfxResId( STR_RESET ) ) );
    aTabCtrl.SetActivatePageHdl(
            LINK( this, SfxTabDialog, ActivatePageHdl ) );
    aTabCtrl.SetDeactivatePageHdl(
            LINK( this, SfxTabDialog, DeactivatePageHdl ) );
    aTabCtrl.Show();
    aOKBtn.Show();
    aCancelBtn.Show();
    aHelpBtn.Show();
    aResetBtn.Show();
    aResetBtn.SetHelpId( HID_TABDLG_RESET_BTN );

    if ( pUserBtn )
    {
        pUserBtn->SetText( *pUserButtonText );
        pUserBtn->SetClickHdl( LINK( this, SfxTabDialog, UserHdl ) );
        pUserBtn->Show();
    }

    /* TODO: Check what is up with bFmt/bFmtFlag. Comment below suggests a
             different behavior than implemented!! */
    if ( bFmtFlag )
    {
        String aStd( SfxResId( STR_STANDARD_SHORTCUT ) );
        aBaseFmtBtn.SetText( aStd );
        aBaseFmtBtn.SetClickHdl( LINK( this, SfxTabDialog, BaseFmtHdl ) );
        aBaseFmtBtn.SetHelpId( HID_TABDLG_STANDARD_BTN );

        // bFmt = tempor"ares Flag im Ctor() "ubergeben,
        // wenn bFmt == 2, dann auch TRUE,
        // zus"atzlich Ausblendung vom StandardButton,
        // nach der Initialisierung wieder auf TRUE setzen
        if ( bFmtFlag != 2 )
            aBaseFmtBtn.Show();
        else
            bFmtFlag = TRUE;
    }

    if ( pSet )
    {
        pExampleSet = new SfxItemSet( *pSet );
        pOutSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );
    }
}

// -----------------------------------------------------------------------

void SfxTabDialog::RemoveResetButton()
{
    aResetBtn.Hide();
    pImpl->bHideResetBtn = TRUE;
}

// -----------------------------------------------------------------------

#if ENABLE_LAYOUT_SFX_TABDIALOG
#undef TabDialog
#define TabDialog Dialog
#endif /* ENABLE_LAYOUT_SFX_TABDIALOG */

short SfxTabDialog::Execute()
{
    if ( !aTabCtrl.GetPageCount() )
        return RET_CANCEL;
    Start_Impl();
    return TabDialog::Execute();
}

// -----------------------------------------------------------------------

void SfxTabDialog::StartExecuteModal( const Link& rEndDialogHdl )
{
#if !ENABLE_LAYOUT_SFX_TABDIALOG
    if ( !aTabCtrl.GetPageCount() )
        return;
    Start_Impl();
    TabDialog::StartExecuteModal( rEndDialogHdl );
#else
    rEndDialogHdl.IsSet();
#endif /* !ENABLE_LAYOUT_SFX_TABDIALOG */
}

// -----------------------------------------------------------------------

void SfxTabDialog::Start( BOOL bShow )
{
    aCancelBtn.SetClickHdl( LINK( this, SfxTabDialog, CancelHdl ) );
    pImpl->bModal = FALSE;
    Start_Impl();

    if ( bShow )
        Show();
}

// -----------------------------------------------------------------------

void SfxTabDialog::SetApplyHandler(const Link& _rHdl)
{
    DBG_ASSERT( pImpl->pApplyButton, "SfxTabDialog::GetApplyHandler: no apply button enabled!" );
    if ( pImpl->pApplyButton )
        pImpl->pApplyButton->SetClickHdl( _rHdl );
}

// -----------------------------------------------------------------------

Link SfxTabDialog::GetApplyHandler() const
{
    DBG_ASSERT( pImpl->pApplyButton, "SfxTabDialog::GetApplyHandler: no button enabled!" );
    if ( !pImpl->pApplyButton )
        return Link();

    return pImpl->pApplyButton->GetClickHdl();
}

// -----------------------------------------------------------------------

void SfxTabDialog::EnableApplyButton(BOOL bEnable)
{
    if ( IsApplyButtonEnabled() == bEnable )
        // nothing to do
        return;

    // create or remove the apply button
    if ( bEnable )
    {
        pImpl->pApplyButton = new PushButton( this );
#if !ENABLE_LAYOUT_SFX_TABDIALOG
        // in the z-order, the apply button should be behind the ok button, thus appearing at the right side of it
        pImpl->pApplyButton->SetZOrder(&aOKBtn, WINDOW_ZORDER_BEHIND);
#endif /* ENABLE_LAYOUT_SFX_TABDIALOG */
        pImpl->pApplyButton->SetText( String( SfxResId( STR_APPLY ) ) );
        pImpl->pApplyButton->Show();

        pImpl->pApplyButton->SetHelpId( HID_TABDLG_APPLY_BTN );
    }
    else
    {
        delete pImpl->pApplyButton;
        pImpl->pApplyButton = NULL;
    }

#if !ENABLE_LAYOUT_SFX_TABDIALOG
    // adjust the layout
    if (IsReallyShown())
        AdjustLayout();
#endif /* !ENABLE_LAYOUT_SFX_TABDIALOG */
}

// -----------------------------------------------------------------------

BOOL SfxTabDialog::IsApplyButtonEnabled() const
{
    return ( NULL != pImpl->pApplyButton );
}

// -----------------------------------------------------------------------

const PushButton* SfxTabDialog::GetApplyButton() const
{
    return pImpl->pApplyButton;
}

// -----------------------------------------------------------------------

PushButton* SfxTabDialog::GetApplyButton()
{
    return pImpl->pApplyButton;
}

// -----------------------------------------------------------------------

void SfxTabDialog::Start_Impl()
{
    DBG_ASSERT( pImpl->pData->Count() == aTabCtrl.GetPageCount(), "not all pages registered" );
    USHORT nActPage = aTabCtrl.GetPageId( 0 );

    // load old settings, when exists
    SvtViewOptions aDlgOpt( E_TABDIALOG, String::CreateFromInt32( nResId ) );
    if ( aDlgOpt.Exists() )
    {
#if !ENABLE_LAYOUT_SFX_TABDIALOG
        SetWindowState( ByteString( aDlgOpt.GetWindowState().getStr(), RTL_TEXTENCODING_ASCII_US ) );
#endif /* !ENABLE_LAYOUT_SFX_TABDIALOG */

        // initiale TabPage aus Programm/Hilfe/Konfig
        nActPage = (USHORT)aDlgOpt.GetPageID();

        if ( USHRT_MAX != nAppPageId )
            nActPage = nAppPageId;
        else
        {
            USHORT nAutoTabPageId = SFX_APP()->Get_Impl()->nAutoTabPageId;
            if ( nAutoTabPageId )
                nActPage = nAutoTabPageId;
        }

        if ( TAB_PAGE_NOTFOUND == aTabCtrl.GetPagePos( nActPage ) )
            nActPage = aTabCtrl.GetPageId( 0 );
    }
    else if ( USHRT_MAX != nAppPageId && TAB_PAGE_NOTFOUND != aTabCtrl.GetPagePos( nAppPageId ) )
        nActPage = nAppPageId;

    aTabCtrl.SetCurPageId( nActPage );
    ActivatePageHdl( &aTabCtrl );
}

void SfxTabDialog::AddTabPage( USHORT nId, BOOL bItemsOnDemand )
{
    AddTabPage( nId, 0, 0, bItemsOnDemand );
}

void SfxTabDialog::AddTabPage( USHORT nId, const String &rRiderText, BOOL bItemsOnDemand, USHORT nPos )
{
    AddTabPage( nId, rRiderText, 0, 0, bItemsOnDemand, nPos );
}

#ifdef SV_HAS_RIDERBITMAPS

void SfxTabDialog::AddTabPage( USHORT nId, const Bitmap &rRiderBitmap, BOOL bItemsOnDemand, USHORT nPos )
{
    AddTabPage( nId, rRiderBitmap, 0, 0, bItemsOnDemand, nPos );
}

#endif

// -----------------------------------------------------------------------

void SfxTabDialog::AddTabPage

/*  [Beschreibung]

    Hinzuf"ugen einer Seite zu dem Dialog.
    Mu\s korrespondieren zu einem entsprechende Eintrag im
    TabControl in der Resource des Dialogs.
*/

(
    USHORT nId,                     // ID der Seite
    CreateTabPage pCreateFunc,      // Pointer auf die Factory-Methode
    GetTabPageRanges pRangesFunc,   // Pointer auf die Methode f"ur das
                                    // Erfragen der Ranges onDemand
    BOOL bItemsOnDemand             // gibt an, ob das Set dieser Seite beim
                                    // Erzeugen der Seite erfragt wird
)
{
    pImpl->pData->Append(
        new Data_Impl( nId, pCreateFunc, pRangesFunc, bItemsOnDemand ) );
}

// -----------------------------------------------------------------------

void SfxTabDialog::AddTabPage

/*  [Beschreibung]

    Hinzuf"ugen einer Seite zu dem Dialog.
    Der Ridertext wird "ubergeben, die Seite hat keine Entsprechung im
    TabControl in der Resource des Dialogs.
*/

(
    USHORT nId,
    const String& rRiderText,
    CreateTabPage pCreateFunc,
    GetTabPageRanges pRangesFunc,
    BOOL bItemsOnDemand,
    USHORT nPos
)
{
    DBG_ASSERT( TAB_PAGE_NOTFOUND == aTabCtrl.GetPagePos( nId ),
                "Doppelte Page-Ids in der Tabpage" );
    aTabCtrl.InsertPage( nId, rRiderText, nPos );
    pImpl->pData->Append(
        new Data_Impl( nId, pCreateFunc, pRangesFunc, bItemsOnDemand ) );
}

// -----------------------------------------------------------------------
#ifdef SV_HAS_RIDERBITMAPS

void SfxTabDialog::AddTabPage

/*  [Beschreibung]

    Hinzuf"ugen einer Seite zu dem Dialog.
    Die Riderbitmap wird "ubergeben, die Seite hat keine Entsprechung im
    TabControl in der Resource des Dialogs.
*/

(
    USHORT nId,
    const Bitmap &rRiderBitmap,
    CreateTabPage pCreateFunc,
    GetTabPageRanges pRangesFunc,
    BOOL bItemsOnDemand,
    USHORT nPos
)
{
    DBG_ASSERT( TAB_PAGE_NOTFOUND == aTabCtrl.GetPagePos( nId ),
                "Doppelte Page-Ids in der Tabpage" );
    aTabCtrl.InsertPage( nId, rRiderBitmap, nPos );
    pImpl->pData->Append(
        new Data_Impl( nId, pCreateFunc, pRangesFunc, bItemsOnDemand ) );
}
#endif

// -----------------------------------------------------------------------

void SfxTabDialog::RemoveTabPage( USHORT nId )

/*  [Beschreibung]

    L"oschen der TabPage mit der ID nId
*/

{
    USHORT nPos = 0;
    aTabCtrl.RemovePage( nId );
    Data_Impl* pDataObject = Find( *pImpl->pData, nId, &nPos );

    if ( pDataObject )
    {
        if ( pDataObject->pTabPage )
        {
            pDataObject->pTabPage->FillUserData();
            String aPageData( pDataObject->pTabPage->GetUserData() );
            if ( aPageData.Len() )
            {
                // save settings of this page (user data)
                SvtViewOptions aPageOpt( E_TABPAGE, String::CreateFromInt32( pDataObject->nId ) );
                aPageOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( aPageData ) ) );
            }

            if ( pDataObject->bOnDemand )
                delete (SfxItemSet*)&pDataObject->pTabPage->GetItemSet();
            delete pDataObject->pTabPage;
        }

        delete pDataObject;
        pImpl->pData->Remove( nPos );
    }
    else
    {
        DBG_WARNINGFILE( "TabPage-Id nicht bekannt" );
    }
}

// -----------------------------------------------------------------------

void SfxTabDialog::PageCreated

/*  [Beschreibung]

    Defaultimplemetierung der virtuellen Methode.
    Diese wird unmittelbar nach dem Erzeugen einer Seite gerufen.
    Hier kann der Dialog direkt an der TabPage Methoden rufen.
*/

(
    USHORT,         // Id der erzeugten Seite
    SfxTabPage&     // Referenz auf die erzeugte Seite
)
{
}

// -----------------------------------------------------------------------

SfxItemSet* SfxTabDialog::GetInputSetImpl()

/*  [Beschreibung]

    Abgeleitete Klassen legen ggf. fuer den InputSet neuen Speicher an.
    Dieser mu\s im Destruktor auch wieder freigegeben werden. Dazu mu\s
    diese Methode gerufen werden.
*/

{
    return (SfxItemSet*)pSet;
}

// -----------------------------------------------------------------------

SfxTabPage* SfxTabDialog::GetTabPage( USHORT nPageId ) const

/*  [Beschreibung]

    TabPage mit der "Ubergebenen Id zur"uckgeben.
*/

{
    USHORT nPos = 0;
    Data_Impl* pDataObject = Find( *pImpl->pData, nPageId, &nPos );

    if ( pDataObject )
        return pDataObject->pTabPage;
    return NULL;
}

// -----------------------------------------------------------------------

BOOL SfxTabDialog::IsInOK() const
{
    return pImpl->bInOK;
}

// -----------------------------------------------------------------------

short SfxTabDialog::Ok()

/*  [Beschreibung]

    Ok-Handler des Dialogs
    Das OutputSet wird erstellt und jede Seite wird mit
    dem bzw. ihrem speziellen OutputSet durch Aufruf der Methode
    <SfxTabPage::FillItemSet(SfxItemSet &)> dazu aufgefordert,
    die vom Benuzter eingestellten Daten in das Set zu tun.

    [R"uckgabewert]

    RET_OK: wenn mindestens eine Seite TRUE als Returnwert von
            FillItemSet geliefert hat, sonst RET_CANCEL.
*/

{
    pImpl->bInOK = TRUE;

    if ( !pOutSet )
    {
        if ( !pExampleSet && pSet )
            pOutSet = pSet->Clone( FALSE ); // ohne Items
        else if ( pExampleSet )
            pOutSet = new SfxItemSet( *pExampleSet );
    }
    BOOL bModified = FALSE;

    const USHORT nCount = pImpl->pData->Count();

    for ( USHORT i = 0; i < nCount; ++i )
    {
        Data_Impl* pDataObject = pImpl->pData->GetObject(i);
        SfxTabPage* pTabPage = pDataObject->pTabPage;

        if ( pTabPage )
        {
            if ( pDataObject->bOnDemand )
            {
                SfxItemSet& rSet = (SfxItemSet&)pTabPage->GetItemSet();
                rSet.ClearItem();
                bModified |= pTabPage->FillItemSet( rSet );
            }
            else if ( pSet && !pTabPage->HasExchangeSupport() )
            {
                SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

                if ( pTabPage->FillItemSet( aTmp ) )
                {
                    bModified |= TRUE;
                    pExampleSet->Put( aTmp );
                    pOutSet->Put( aTmp );
                }
            }
        }
    }

    if ( pImpl->bModified || ( pOutSet && pOutSet->Count() > 0 ) )
        bModified |= TRUE;

    if ( bFmt == 2 )
        bModified |= TRUE;
    return bModified ? RET_OK : RET_CANCEL;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, CancelHdl, Button*, pButton )
{
    (void)pButton; //unused
    Close();
    return 0;
}

// -----------------------------------------------------------------------

SfxItemSet* SfxTabDialog::CreateInputItemSet( USHORT )

/*  [Beschreibung]

    Defaultimplemetierung der virtuellen Methode.
    Diese wird gerufen, wenn Pages ihre Sets onDenamd anlegen
*/

{
    DBG_WARNINGFILE( "CreateInputItemSet nicht implementiert" );
    return new SfxAllItemSet( SFX_APP()->GetPool() );
}

// -----------------------------------------------------------------------

const SfxItemSet* SfxTabDialog::GetRefreshedSet()

/*  [Beschreibung]

    Defaultimplemetierung der virtuellen Methode.
    Diese wird gerufen, wenn <SfxTabPage::DeactivatePage(SfxItemSet *)>
    <SfxTabPage::REFRESH_SET> liefert.
*/

{
    DBG_ERRORFILE( "GetRefreshedSet nicht implementiert" );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, OkHdl, Button *, EMPTYARG )

/*  [Beschreibung]

    Handler des Ok-Buttons
    Dieser ruft f"ur die aktuelle Seite
    <SfxTabPage::DeactivatePage(SfxItemSet *)>.
    Liefert diese <SfxTabPage::LEAVE_PAGE>, wird <SfxTabDialog::Ok()> gerufen
    und so der Dialog beendet.
*/

{
    pImpl->bInOK = TRUE;

    if ( OK_Impl() )
    {
        if ( pImpl->bModal )
            EndDialog( Ok() );
        else
        {
            Ok();
            Close();
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

bool SfxTabDialog::PrepareLeaveCurrentPage()
{
    USHORT const nId = aTabCtrl.GetCurPageId();
    SfxTabPage* pPage = dynamic_cast<SfxTabPage*> (aTabCtrl.GetTabPage( nId ));
    bool bEnd = !pPage;

    if ( pPage )
    {
        int nRet = SfxTabPage::LEAVE_PAGE;
        if ( pSet )
        {
            SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

            if ( pPage->HasExchangeSupport() )
                nRet = pPage->DeactivatePage( &aTmp );
            else
                nRet = pPage->DeactivatePage( NULL );

            if ( ( SfxTabPage::LEAVE_PAGE & nRet ) == SfxTabPage::LEAVE_PAGE
                 && aTmp.Count() )
            {
                pExampleSet->Put( aTmp );
                pOutSet->Put( aTmp );
            }
        }
        else
            nRet = pPage->DeactivatePage( NULL );
        bEnd = nRet;
    }

    return bEnd;
}


// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, UserHdl, Button *, EMPTYARG )

/*  [Beschreibung]

    Handler des User-Buttons
    Dieser ruft f"ur die aktuelle Seite
    <SfxTabPage::DeactivatePage(SfxItemSet *)>.
    Liefert diese <SfxTabPage::LEAVE_PAGE>, wird <SfxTabDialog::Ok()> gerufen.
    Mit dem Return-Wert von <SfxTabDialog::Ok()> wird dann der Dialog beendet.
*/

{
    if ( PrepareLeaveCurrentPage () )
    {
        short nRet = Ok();

        if ( RET_OK == nRet )
            nRet = RET_USER;
        else
            nRet = RET_USER_CANCEL;
        EndDialog( nRet );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, ResetHdl, Button *, EMPTYARG )

/*  [Beschreibung]

    Handler hinter dem Zur"ucksetzen-Button.
    Die aktuelle Page wird mit ihren initialen Daten
    neu initialisiert; alle Einstellungen, die der Benutzer
    auf dieser Seite get"atigt hat, werden aufgehoben.
*/

{
    const USHORT nId = aTabCtrl.GetCurPageId();
    Data_Impl* pDataObject = Find( *pImpl->pData, nId );
    DBG_ASSERT( pDataObject, "Id nicht bekannt" );

    if ( pDataObject->bOnDemand )
    {
        // CSet auf AIS hat hier Probleme, daher getrennt
        const SfxItemSet* pItemSet = &pDataObject->pTabPage->GetItemSet();
        pDataObject->pTabPage->Reset( *(SfxItemSet*)pItemSet );
    }
    else
        pDataObject->pTabPage->Reset( *pSet );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, BaseFmtHdl, Button *, EMPTYARG )

/*  [Beschreibung]

    Handler hinter dem Standard-Button.
    Dieser Button steht beim Bearbeiten von StyleSheets zur Verf"ugung.
    Alle in dem bearbeiteten StyleSheet eingestellten Attribute
    werden gel"oscht.
*/

{
    const USHORT nId = aTabCtrl.GetCurPageId();
    Data_Impl* pDataObject = Find( *pImpl->pData, nId );
    DBG_ASSERT( pDataObject, "Id nicht bekannt" );
    bFmt = 2;

    if ( pDataObject->fnGetRanges )
    {
        if ( !pExampleSet )
            pExampleSet = new SfxItemSet( *pSet );

        const SfxItemPool* pPool = pSet->GetPool();
        const USHORT* pTmpRanges = (pDataObject->fnGetRanges)();
        SfxItemSet aTmpSet( *pExampleSet );

        while ( *pTmpRanges )
        {
            const USHORT* pU = pTmpRanges + 1;

            if ( *pTmpRanges == *pU )
            {
                // Range mit zwei gleichen Werten -> nur ein Item setzen
                USHORT nWh = pPool->GetWhich( *pTmpRanges );
                pExampleSet->ClearItem( nWh );
                aTmpSet.ClearItem( nWh );
                // am OutSet mit InvalidateItem,
                // damit die "Anderung wirksam wird
                pOutSet->InvalidateItem( nWh );
            }
            else
            {
                // richtiger Range mit mehreren Werten
                USHORT nTmp = *pTmpRanges, nTmpEnd = *pU;
                DBG_ASSERT( nTmp <= nTmpEnd, "Range ist falsch sortiert" );

                if ( nTmp > nTmpEnd )
                {
                    // wenn wirklich falsch sortiert, dann neu setzen
                    USHORT nTmp1 = nTmp;
                    nTmp = nTmpEnd;
                    nTmpEnd = nTmp1;
                }

                while ( nTmp <= nTmpEnd )
                {
                    // "uber den Range iterieren, und die Items setzen
                    USHORT nWh = pPool->GetWhich( nTmp );
                    pExampleSet->ClearItem( nWh );
                    aTmpSet.ClearItem( nWh );
                    // am OutSet mit InvalidateItem,
                    // damit die "Anderung wirksam wird
                    pOutSet->InvalidateItem( nWh );
                    nTmp++;
                }
            }
            // zum n"achsten Paar gehen
            pTmpRanges += 2;
        }
        // alle Items neu gesetzt -> dann an der aktuellen Page Reset() rufen
        DBG_ASSERT( pDataObject->pTabPage, "die Page ist weg" );
        pDataObject->pTabPage->Reset( aTmpSet );
        pDataObject->pTabPage->pImpl->mbStandard = TRUE;
    }
    return 1;
}

// -----------------------------------------------------------------------

#if ENABLE_LAYOUT_SFX_TABDIALOG
#define tabControlWindow pTabCtrl->GetWindow ()
#else /* !ENABLE_LAYOUT_SFX_TABDIALOG */
#define tabControlWindow pTabCtrl
#endif /* !ENABLE_LAYOUT_SFX_TABDIALOG */

IMPL_LINK( SfxTabDialog, ActivatePageHdl, TabControl *, pTabCtrl )

/*  [Beschreibung]

    Handler, der vor dem Umschalten auf eine andere Seite
    durch Starview gerufen wird.
    Existiert die Seite noch nicht, so wird sie erzeugt und
    die virtuelle Methode <SfxTabDialog::PageCreated( USHORT, SfxTabPage &)>
    gerufen. Existiert die Seite bereits, so wird ggf.
    <SfxTabPage::Reset(const SfxItemSet &)> oder
    <SfxTabPage::ActivatePage(const SfxItemSet &)> gerufen.
*/

{
    USHORT const nId = pTabCtrl->GetCurPageId();

    DBG_ASSERT( pImpl->pData->Count(), "keine Pages angemeldet" );
    SFX_APP();

    // Tab Page schon da?
    SfxTabPage* pTabPage = dynamic_cast<SfxTabPage*> (pTabCtrl->GetTabPage( nId ));
    Data_Impl* pDataObject = Find( *pImpl->pData, nId );
    DBG_ASSERT( pDataObject, "Id nicht bekannt" );

    // ggf. TabPage erzeugen:
    if ( !pTabPage )
    {
#if ENABLE_LAYOUT_SFX_TABDIALOG
        if (dynamic_cast<layout SfxTabPage*> (pTabPage))
            layout::TabPage::global_parent = pTabCtrl->GetWindow ();
#endif
        const SfxItemSet* pTmpSet = 0;

        if ( pSet )
        {
            if ( bItemsReset && pSet->GetParent() )
                pTmpSet = pSet->GetParent();
            else
                pTmpSet = pSet;
        }

        if ( pTmpSet && !pDataObject->bOnDemand )
            pTabPage = (pDataObject->fnCreatePage)( tabControlWindow, *pTmpSet );
        else
            pTabPage = (pDataObject->fnCreatePage)
                            ( tabControlWindow, *CreateInputItemSet( nId ) );
        DBG_ASSERT( NULL == pDataObject->pTabPage, "create TabPage more than once" );
        pDataObject->pTabPage = pTabPage;

#if !ENABLE_LAYOUT_SFX_TABDIALOG
        pDataObject->pTabPage->SetTabDialog( this );
#endif /* ENABLE_LAYOUT_SFX_TABDIALOG */
        SvtViewOptions aPageOpt( E_TABPAGE, String::CreateFromInt32( pDataObject->nId ) );
        String sUserData;
        Any aUserItem = aPageOpt.GetUserItem( USERITEM_NAME );
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            sUserData = String( aTemp );
        pTabPage->SetUserData( sUserData );
        Size aSiz = pTabPage->GetSizePixel();

#if ENABLE_LAYOUT
        Size optimalSize = pTabPage->GetOptimalSize (WINDOWSIZE_MINIMUM);
#if ENABLE_LAYOUT_SFX_TABDIALOG
        if (dynamic_cast<layout SfxTabPage*> (pTabPage))
        {
            if (optimalSize.Height () && optimalSize.Width ())
            {
                optimalSize.Width () = optimalSize.Width ();
                optimalSize.Height () = optimalSize.Height () + 40;
            }
        }
#endif /* ENABLE_LAYOUT_SFX_TABDIALOG */
        if (optimalSize.Height () > 0 && optimalSize.Width () > 0 )
            aSiz = optimalSize;
#endif /* ENABLE_LAYOUT */

        Size aCtrlSiz = pTabCtrl->GetTabPageSizePixel();
        // Gr"o/se am TabControl nur dann setzen, wenn < als TabPage
        if ( aCtrlSiz.Width() < aSiz.Width() ||
             aCtrlSiz.Height() < aSiz.Height() )
        {
            pTabCtrl->SetTabPageSizePixel( aSiz );
        }

        PageCreated( nId, *pTabPage );

        if ( pDataObject->bOnDemand )
            pTabPage->Reset( (SfxItemSet &)pTabPage->GetItemSet() );
        else
            pTabPage->Reset( *pSet );

        pTabCtrl->SetTabPage( nId, pTabPage );
    }
    else if ( pDataObject->bRefresh )
        pTabPage->Reset( *pSet );
    pDataObject->bRefresh = FALSE;

#if ENABLE_LAYOUT_SFX_TABDIALOG
    pTabCtrl->GetPagePos (nId);
#endif /* ENABLE_LAYOUT_SFX_TABDIALOG */

    if ( pExampleSet )
        pTabPage->ActivatePage( *pExampleSet );
    BOOL bReadOnly = pTabPage->IsReadOnly();
    ( bReadOnly || pImpl->bHideResetBtn ) ? aResetBtn.Hide() : aResetBtn.Show();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, DeactivatePageHdl, TabControl *, pTabCtrl )

/*  [Beschreibung]

    Handler, der vor dem Verlassen einer Seite durch Starview gerufen wird.

    [Querverweise]

    <SfxTabPage::DeactivatePage(SfxItemSet *)>
*/

{
    USHORT nId = pTabCtrl->GetCurPageId();
    SFX_APP();
    SfxTabPage *pPage = dynamic_cast<SfxTabPage*> (pTabCtrl->GetTabPage( nId ));
    DBG_ASSERT( pPage, "keine aktive Page" );
#ifdef DBG_UTIL
    Data_Impl* pDataObject = Find( *pImpl->pData, pTabCtrl->GetCurPageId() );
    DBG_ASSERT( pDataObject, "keine Datenstruktur zur aktuellen Seite" );
    if ( pPage->HasExchangeSupport() && pDataObject->bOnDemand )
    {
        DBG_WARNING( "Datenaustausch bei ItemsOnDemand ist nicht gewuenscht!" );
    }
#endif

    int nRet = SfxTabPage::LEAVE_PAGE;

    if ( !pExampleSet && pPage->HasExchangeSupport() && pSet )
        pExampleSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );

    if ( pSet )
    {
        SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

        if ( pPage->HasExchangeSupport() )
            nRet = pPage->DeactivatePage( &aTmp );
        else
            nRet = pPage->DeactivatePage( NULL );

        if ( ( SfxTabPage::LEAVE_PAGE & nRet ) == SfxTabPage::LEAVE_PAGE &&
             aTmp.Count() )
        {
            pExampleSet->Put( aTmp );
            pOutSet->Put( aTmp );
        }
    }
    else
    {
        if ( pPage->HasExchangeSupport() ) //!!!
        {
            if ( !pExampleSet )
            {
                SfxItemPool* pPool = pPage->GetItemSet().GetPool();
                pExampleSet =
                    new SfxItemSet( *pPool, GetInputRanges( *pPool ) );
            }
            nRet = pPage->DeactivatePage( pExampleSet );
        }
        else
            nRet = pPage->DeactivatePage( NULL );
    }

    if ( nRet & SfxTabPage::REFRESH_SET )
    {
        pSet = GetRefreshedSet();
        DBG_ASSERT( pSet, "GetRefreshedSet() liefert NULL" );
        // alle Pages als neu zu initialsieren flaggen
        const USHORT nCount = pImpl->pData->Count();

        for ( USHORT i = 0; i < nCount; ++i )
        {
            Data_Impl* pObj = (*pImpl->pData)[i];

            if ( pObj->pTabPage != pPage ) // eigene Page nicht mehr refreshen
                pObj->bRefresh = TRUE;
            else
                pObj->bRefresh = FALSE;
        }
    }
    if ( nRet & SfxTabPage::LEAVE_PAGE )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

const SfxItemSet* SfxTabDialog::GetOutputItemSet

/*  [Beschreibung]

    Liefert die Pages, die ihre Sets onDemand liefern, das OutputItemSet.

    [Querverweise]

    <SfxTabDialog::AddTabPage(USHORT, CreateTabPage, GetTabPageRanges, BOOL)>
    <SfxTabDialog::AddTabPage(USHORT, const String &, CreateTabPage, GetTabPageRanges, BOOL, USHORT)>
    <SfxTabDialog::AddTabPage(USHORT, const Bitmap &, CreateTabPage, GetTabPageRanges, BOOL, USHORT)>
*/

(
    USHORT nId  // die Id, unter der die Seite bei AddTabPage()
                // hinzugef"ugt wurde.
) const
{
    Data_Impl* pDataObject = Find( *pImpl->pData, nId );
    DBG_ASSERT( pDataObject, "TabPage nicht gefunden" );

    if ( pDataObject )
    {
        if ( !pDataObject->pTabPage )
            return NULL;

        if ( pDataObject->bOnDemand )
            return &pDataObject->pTabPage->GetItemSet();
        return pOutSet;
    }
    return NULL;
}

// -----------------------------------------------------------------------

int SfxTabDialog::FillOutputItemSet()
{
    int nRet = SfxTabPage::LEAVE_PAGE;
    if ( OK_Impl() )
        Ok();
    else
        nRet = SfxTabPage::KEEP_PAGE;
    return nRet;
}

// -----------------------------------------------------------------------

#ifdef WNT
int __cdecl TabDlgCmpUS_Impl( const void* p1, const void* p2 )
#else
#if defined(OS2) && defined(ICC)
int _Optlink TabDlgCmpUS_Impl( const void* p1, const void* p2 )
#else
extern "C" int TabDlgCmpUS_Impl( const void* p1, const void* p2 )
#endif
#endif

/*  [Beschreibung]

    Vergleichsfunktion f"ur qsort
*/

{
    return *(USHORT*)p1 - *(USHORT*)p2;
}

// -----------------------------------------------------------------------

void SfxTabDialog::ShowPage( USHORT nId )

/*  [Beschreibung]

    Es wird die TabPage mit der "ubergebenen Id aktiviert.
*/

{
    aTabCtrl.SetCurPageId( nId );
    ActivatePageHdl( &aTabCtrl );
}

// -----------------------------------------------------------------------

const USHORT* SfxTabDialog::GetInputRanges( const SfxItemPool& rPool )

/*  [Beschreibung]

    Bildet das Set "uber die Ranges aller Seiten des Dialogs.
    Die Pages m"ussen die statische Methode f"ur das Erfragen ihrer
    Ranges bei AddTabPage angegeben haben, liefern also ihre Sets onDemand.

    [Querverweise]

    <SfxTabDialog::AddTabPage(USHORT, CreateTabPage, GetTabPageRanges, BOOL)>
    <SfxTabDialog::AddTabPage(USHORT, const String &, CreateTabPage, GetTabPageRanges, BOOL, USHORT)>
    <SfxTabDialog::AddTabPage(USHORT, const Bitmap &, CreateTabPage, GetTabPageRanges, BOOL, USHORT)>

    [R"uckgabewert]

    Pointer auf nullterminiertes Array von USHORTs
    Dieses Array geh"ort dem Dialog und wird beim
    Zerst"oren des Dialogs gel"oscht.
*/

{
    if ( pSet )
    {
        DBG_ERRORFILE( "Set bereits vorhanden!" );
        return pSet->GetRanges();
    }

    if ( pRanges )
        return pRanges;
    SvUShorts aUS( 16, 16 );
    USHORT nCount = pImpl->pData->Count();

    USHORT i;
    for ( i = 0; i < nCount; ++i )
    {
        Data_Impl* pDataObject = pImpl->pData->GetObject(i);

        if ( pDataObject->fnGetRanges )
        {
            const USHORT* pTmpRanges = (pDataObject->fnGetRanges)();
            const USHORT* pIter = pTmpRanges;

            USHORT nLen;
            for( nLen = 0; *pIter; ++nLen, ++pIter )
                ;
            aUS.Insert( pTmpRanges, nLen, aUS.Count() );
        }
    }

    //! Doppelte Ids entfernen?
#ifndef TF_POOLABLE
    if ( rPool.HasMap() )
#endif
    {
        nCount = aUS.Count();

        for ( i = 0; i < nCount; ++i )
            aUS[i] = rPool.GetWhich( aUS[i] );
    }

    // sortieren
    if ( aUS.Count() > 1 )
        qsort( (void*)aUS.GetData(),
               aUS.Count(), sizeof(USHORT), TabDlgCmpUS_Impl );

    pRanges = new USHORT[aUS.Count() + 1];
    memcpy(pRanges, aUS.GetData(), sizeof(USHORT) * aUS.Count());
    pRanges[aUS.Count()] = 0;
    return pRanges;
}

// -----------------------------------------------------------------------

void SfxTabDialog::SetInputSet( const SfxItemSet* pInSet )

/*  [Beschreibung]

    Mit dieser Methode kann nachtr"aglich der Input-Set initial oder
    neu gesetzt werden.
*/

{
    bool bSet = ( pSet != NULL );

    pSet = pInSet;

    if ( !bSet && !pExampleSet && !pOutSet )
    {
        pExampleSet = new SfxItemSet( *pSet );
        pOutSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );
    }
}

long SfxTabDialog::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        SfxViewFrame* pViewFrame = GetViewFrame() ? GetViewFrame() : SfxViewFrame::Current();
        if ( pViewFrame )
        {
            Window* pWindow = rNEvt.GetWindow();
            ULONG nHelpId  = 0;
            while ( !nHelpId && pWindow )
            {
                nHelpId = pWindow->GetHelpId();
                pWindow = pWindow->GetParent();
            }

            if ( nHelpId )
                SfxHelp::OpenHelpAgent( &pViewFrame->GetFrame(), nHelpId );
        }
    }

    return TabDialog::Notify( rNEvt );
}

END_NAMESPACE_LAYOUT_SFX_TABDIALOG

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
