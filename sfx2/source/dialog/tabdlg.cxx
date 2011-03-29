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

#define _SVSTDARR_sal_uInt16S
#include <svl/svstdarr.hxx>

#include "appdata.hxx"
#include "sfxtypes.hxx"
#include <sfx2/minarray.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>
#include "sfx2/sfxresid.hxx"
#include "sfx2/sfxhelp.hxx"
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
    sal_Bool                        mbStandard;
    sfx::ItemConnectionArray    maItemConn;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;

    TabPageImpl() : mbStandard( sal_False ) {}
};

NAMESPACE_LAYOUT_SFX_TABDIALOG

struct Data_Impl
{
    sal_uInt16 nId;                   // The ID
    CreateTabPage fnCreatePage;   // Pointer to Factory
    GetTabPageRanges fnGetRanges; // Pointer to Ranges-Function
    SfxTabPage* pTabPage;         // The TabPage itself
    sal_Bool bOnDemand;              // Flag: ItemSet onDemand
    sal_Bool bRefresh;                // Flag: Page must be re-initialized

    // Constructor
    Data_Impl( sal_uInt16 Id, CreateTabPage fnPage,
               GetTabPageRanges fnRanges, sal_Bool bDemand ) :

        nId         ( Id ),
        fnCreatePage( fnPage ),
        fnGetRanges ( fnRanges ),
        pTabPage    ( 0 ),
        bOnDemand   ( bDemand ),
        bRefresh    ( sal_False )
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

SfxTabDialogItem::SfxTabDialogItem( sal_uInt16 nId, const SfxItemSet& rItemSet )
    : SfxSetItem( nId, rItemSet )
{
}

SfxPoolItem* SfxTabDialogItem::Clone(SfxItemPool* pToPool) const
{
    return new SfxTabDialogItem( *this, pToPool );
}

SfxPoolItem* SfxTabDialogItem::Create(SvStream& /*rStream*/, sal_uInt16 /*nVersion*/) const
{
    OSL_FAIL( "Use it only in UI!" );
    return NULL;
}

class SfxTabDialogController : public SfxControllerItem
{
    SfxTabDialog*   pDialog;
    const SfxItemSet*     pSet;
public:
                    SfxTabDialogController( sal_uInt16 nSlotId, SfxBindings& rBindings, SfxTabDialog* pDlg )
                        : SfxControllerItem( nSlotId, rBindings )
                        , pDialog( pDlg )
                        , pSet( NULL )
                    {}

                    ~SfxTabDialogController();

    DECL_LINK(      Execute_Impl, void* );
    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
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

void SfxTabDialogController::StateChanged( sal_uInt16 /*nSID*/, SfxItemState /*eState*/, const SfxPoolItem* pState )
{
    const SfxSetItem* pSetItem = PTR_CAST( SfxSetItem, pState );
    if ( pSetItem )
    {
        pSet = pDialog->pSet = pSetItem->GetItemSet().Clone();
        sal_Bool bDialogStarted = sal_False;
        for ( sal_uInt16 n=0; n<pDialog->aTabCtrl.GetPageCount(); n++ )
        {
            sal_uInt16 nPageId = pDialog->aTabCtrl.GetPageId( n );
            SfxTabPage* pTabPage = dynamic_cast<SfxTabPage*> (pDialog->aTabCtrl.GetTabPage( nPageId ));
            if ( pTabPage )
            {
                pTabPage->Reset( pSetItem->GetItemSet() );
                bDialogStarted = sal_True;
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
    sal_Bool                bModified       : 1,
                        bModal          : 1,
                        bInOK           : 1,
                        bHideResetBtn   : 1;
    SfxTabDlgData_Impl* pData;

    PushButton*         pApplyButton;
    SfxTabDialogController* pController;

    TabDlg_Impl( sal_uInt8 nCnt ) :

        bModified       ( sal_False ),
        bModal          ( sal_True ),
        bInOK           ( sal_False ),
        bHideResetBtn   ( sal_False ),
        pData           ( new SfxTabDlgData_Impl( nCnt ) ),
        pApplyButton    ( NULL ),
        pController     ( NULL )
    {}
};

Data_Impl* Find( SfxTabDlgData_Impl& rArr, sal_uInt16 nId, sal_uInt16* pPos = 0 );

Data_Impl* Find( SfxTabDlgData_Impl& rArr, sal_uInt16 nId, sal_uInt16* pPos )
{
    const sal_uInt16 nCount = rArr.Count();

    for ( sal_uInt16 i = 0; i < nCount; ++i )
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

/*  [Description]

    Constructor
*/

    TabPage( pParent, rResId ),

    pSet                ( &rAttrSet ),
    bHasExchangeSupport ( sal_False ),
    pTabDlg             ( NULL ),
    pImpl               ( new TabPageImpl )

{
}
// -----------------------------------------------------------------------
SfxTabPage:: SfxTabPage( Window *pParent, WinBits nStyle, const SfxItemSet &rAttrSet ) :
    TabPage(pParent, nStyle),
    pSet                ( &rAttrSet ),
    bHasExchangeSupport ( sal_False ),
    pTabDlg             ( NULL ),
    pImpl               ( new TabPageImpl )
{
}
// -----------------------------------------------------------------------

SfxTabPage::~SfxTabPage()

/*  [Description]

    Destructor
*/

{
#if !ENABLE_LAYOUT
    delete pImpl;
#endif /* ENABLE_LAYOUT */
}

// -----------------------------------------------------------------------

sal_Bool SfxTabPage::FillItemSet( SfxItemSet& rSet )
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

/*  [Description]

    Default implementation of the virtual ActivatePage method. This method is
    called when a page of dialogue supports the exchange of data between pages.
    <SfxTabPage::DeactivatePage(SfxItemSet *)>
*/

{
}

// -----------------------------------------------------------------------

int SfxTabPage::DeactivatePage( SfxItemSet* )

/*  [Description]

    Default implementation of the virtual DeactivatePage method. This method is
    called by Sfx when leaving a page; the application can, through the return
    value, control whether to leave the page. If the page is displayed through
    bHasExchangeSupport which supports data exchange between pages, then a
    pointer to the exchange set is passed as parameter. This takes on data for
    the exchange, then the set is available as a parameter in
    <SfxTabPage::ActivatePage(const SfxItemSet &)>.

    [Return value]

    LEAVE_PAGE; Allow leaving the page
*/

{
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void SfxTabPage::FillUserData()

/*  [Description]

    Virtual method is called by the base class in the destructor to save
    specific information of the TabPage in the ini-file. When overloading a
    string must be compiled, which is then flushed with the <SetUserData()>.
*/

{
}

// -----------------------------------------------------------------------

sal_Bool SfxTabPage::IsReadOnly() const
{
    return sal_False;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxTabPage::GetItem( const SfxItemSet& rSet, sal_uInt16 nSlot, sal_Bool bDeep )

/*  [Description]

    static Method: hereby are the implementations of the TabPage code
    beeing simplified.
*/

{
    const SfxItemPool* pPool = rSet.GetPool();
    sal_uInt16 nWh = pPool->GetWhich( nSlot, bDeep );
    const SfxPoolItem* pItem = 0;
    rSet.GetItemState( nWh, sal_True, &pItem );

    if ( !pItem && nWh != nSlot )
        pItem = &pPool->GetDefaultItem( nWh );
    return pItem;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxTabPage::GetOldItem( const SfxItemSet& rSet,
                                           sal_uInt16 nSlot, sal_Bool bDeep )

/*  [Description]

    This method returns an attribute for comparison of the old value.
*/

{
    const SfxItemSet& rOldSet = GetItemSet();
    sal_uInt16 nWh = GetWhich( nSlot, bDeep );
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
                                                sal_uInt16 nSlot )

/*  [Description]

    This method returns an attribute for comparison of the old value. This way
    it will be considered whether the dialogue has just been ended with OK.
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
    pImpl       ( new TabDlg_Impl( (sal_uInt8)aTabCtrl.GetPageCount() ) ), \
    pRanges     ( 0 ), \
    nResId      ( rResId.GetId() ), \
    nAppPageId  ( USHRT_MAX ), \
    bItemsReset ( sal_False ),\
    bFmt        ( bEditFmt ),\
    pExampleSet ( 0 )

// -----------------------------------------------------------------------

SfxTabDialog::SfxTabDialog

/*  [Description]

    Constructor
*/

(
    SfxViewFrame* pViewFrame,     // Frame, to which the Dialog belongs
    Window* pParent,              // Parent Window
    const ResId& rResId,            // ResourceId
    const SfxItemSet* pItemSet,   // Itemset with the data;
                                  // can be NULL, when Pages are onDemand
    sal_Bool bEditFmt,                // Flag: templates are processed
                                  // when yes -> additional Button for standard
    const String* pUserButtonText // Text for UserButton;
                                  // if != 0, the UserButton is created
) :
    TabDialog( pParent, rResId ),
    pFrame( pViewFrame ),
    INI_LIST(pItemSet)
{
    Init_Impl( bFmt, pUserButtonText );
}

// -----------------------------------------------------------------------

SfxTabDialog::SfxTabDialog

/*  [Description]

    Constructor, temporary without Frame
*/

(
    Window* pParent,              // Parent Window
    const ResId& rResId,            // ResourceId
    const SfxItemSet* pItemSet,   // Itemset with the data;
                                  // can be NULL, when Pages are onDemand
    sal_Bool bEditFmt,                // Flag: templates are processed
                                  // when yes -> additional Button for standard
    const String* pUserButtonText // Text for UserButton;
                                  // if != 0, the UserButton is created
) :
    TabDialog( pParent, rResId ),
    pFrame( 0 ),
    INI_LIST(pItemSet)
{
    Init_Impl( bFmt, pUserButtonText );
    DBG_WARNING( "Please use the Construtor with the ViewFrame" );
}

SfxTabDialog::SfxTabDialog

/*  [Description]

    Constructor, temporary without Frame
*/

(
    Window* pParent,              // Parent Window
    const ResId& rResId,            // ResourceId
    sal_uInt16 nSetId,
    SfxBindings& rBindings,
    sal_Bool bEditFmt,                // Flag: templates are processed
                                  // when yes -> additional Button for standard
    const String* pUserButtonText // Text for UserButton;
                                  // if != 0, the UserButton is created
) :
    TabDialog( pParent, rResId ),
    pFrame( 0 ),
    INI_LIST(NULL)
{
    rBindings.ENTERREGISTRATIONS();
    pImpl->pController = new SfxTabDialogController( nSetId, rBindings, this );
    rBindings.LEAVEREGISTRATIONS();

    EnableApplyButton( sal_True );
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

    const sal_uInt16 nCount = pImpl->pData->Count();
    for ( sal_uInt16 i = 0; i < nCount; ++i )
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

void SfxTabDialog::Init_Impl( sal_Bool bFmtFlag, const String* pUserButtonText )

/*  [Description]

    internal initialization of the dialogue
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

        // bFmt = temporary Flag passed on in the Constructor(),
        // if bFmt == 2, then also sal_True,
        // additional suppression of the standard button,
        // after the Initializing set to sal_True again
        if ( bFmtFlag != 2 )
            aBaseFmtBtn.Show();
        else
            bFmtFlag = sal_True;
    }

    if ( pSet )
    {
        pExampleSet = new SfxItemSet( *pSet );
        pOutSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );
    }

    aOKBtn.SetAccessibleRelationMemberOf( &aOKBtn );
    aCancelBtn.SetAccessibleRelationMemberOf( &aCancelBtn );
    aHelpBtn.SetAccessibleRelationMemberOf( &aHelpBtn );
    aResetBtn.SetAccessibleRelationMemberOf( &aResetBtn );
}

// -----------------------------------------------------------------------

void SfxTabDialog::RemoveResetButton()
{
    aResetBtn.Hide();
    pImpl->bHideResetBtn = sal_True;
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

void SfxTabDialog::Start( sal_Bool bShow )
{
    aCancelBtn.SetClickHdl( LINK( this, SfxTabDialog, CancelHdl ) );
    pImpl->bModal = sal_False;
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

void SfxTabDialog::EnableApplyButton(sal_Bool bEnable)
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

sal_Bool SfxTabDialog::IsApplyButtonEnabled() const
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
    sal_uInt16 nActPage = aTabCtrl.GetPageId( 0 );

    // load old settings, when exists
    SvtViewOptions aDlgOpt( E_TABDIALOG, String::CreateFromInt32( nResId ) );
    if ( aDlgOpt.Exists() )
    {
#if !ENABLE_LAYOUT_SFX_TABDIALOG
        SetWindowState( ByteString( aDlgOpt.GetWindowState().getStr(), RTL_TEXTENCODING_ASCII_US ) );
#endif /* !ENABLE_LAYOUT_SFX_TABDIALOG */

        // initial TabPage from Program/Help/config
        nActPage = (sal_uInt16)aDlgOpt.GetPageID();

        if ( USHRT_MAX != nAppPageId )
            nActPage = nAppPageId;
        else
        {
            sal_uInt16 nAutoTabPageId = SFX_APP()->Get_Impl()->nAutoTabPageId;
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

void SfxTabDialog::AddTabPage( sal_uInt16 nId, sal_Bool bItemsOnDemand )
{
    AddTabPage( nId, 0, 0, bItemsOnDemand );
}

void SfxTabDialog::AddTabPage( sal_uInt16 nId, const String &rRiderText, sal_Bool bItemsOnDemand, sal_uInt16 nPos )
{
    AddTabPage( nId, rRiderText, 0, 0, bItemsOnDemand, nPos );
}

#ifdef SV_HAS_RIDERBITMAPS

void SfxTabDialog::AddTabPage( sal_uInt16 nId, const Bitmap &rRiderBitmap, sal_Bool bItemsOnDemand, sal_uInt16 nPos )
{
    AddTabPage( nId, rRiderBitmap, 0, 0, bItemsOnDemand, nPos );
}

#endif

// -----------------------------------------------------------------------

void SfxTabDialog::AddTabPage

/*  [Description]

    Adding a page to the dialogue. Must correspond to a entry in the
    TabControl in the resource of the dialogue.
*/

(
    sal_uInt16 nId,                    // Page ID
    CreateTabPage pCreateFunc,     // Pointer to the Factory Method
    GetTabPageRanges pRangesFunc,  // Pointer to the Method for quering
                                   // Ranges onDemand
    sal_Bool bItemsOnDemand            // indicates whether the set of this page is
                                   // requested when created
)
{
    pImpl->pData->Append(
        new Data_Impl( nId, pCreateFunc, pRangesFunc, bItemsOnDemand ) );
}

// -----------------------------------------------------------------------

void SfxTabDialog::AddTabPage

/*  [Description]

    Add a page to the dialog. The Rider text is passed on, the page has no
    counterpart in the TabControl in the resource of the dialogue.
*/

(
    sal_uInt16 nId,
    const String& rRiderText,
    CreateTabPage pCreateFunc,
    GetTabPageRanges pRangesFunc,
    sal_Bool bItemsOnDemand,
    sal_uInt16 nPos
)
{
    DBG_ASSERT( TAB_PAGE_NOTFOUND == aTabCtrl.GetPagePos( nId ),
                "Double Page-Ids in the Tabpage" );
    aTabCtrl.InsertPage( nId, rRiderText, nPos );
    pImpl->pData->Append(
        new Data_Impl( nId, pCreateFunc, pRangesFunc, bItemsOnDemand ) );
}

// -----------------------------------------------------------------------
#ifdef SV_HAS_RIDERBITMAPS

void SfxTabDialog::AddTabPage

/*  [Description]

    Add a page to the dialog. The riders bitmap is passed on, the page has no
    counterpart in the TabControl in the resource of the dialogue.
*/

(
    sal_uInt16 nId,
    const Bitmap &rRiderBitmap,
    CreateTabPage pCreateFunc,
    GetTabPageRanges pRangesFunc,
    sal_Bool bItemsOnDemand,
    sal_uInt16 nPos
)
{
    DBG_ASSERT( TAB_PAGE_NOTFOUND == aTabCtrl.GetPagePos( nId ),
                "Duplicate Page-Ids in the Tabpage" );
    aTabCtrl.InsertPage( nId, rRiderBitmap, nPos );
    pImpl->pData->Append(
        new Data_Impl( nId, pCreateFunc, pRangesFunc, bItemsOnDemand ) );
}
#endif

// -----------------------------------------------------------------------

void SfxTabDialog::RemoveTabPage( sal_uInt16 nId )

/*  [Description]

    Delete the TabPage with ID nId
*/

{
    sal_uInt16 nPos = 0;
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
        DBG_WARNINGFILE( "TabPage-Id not known" );
    }
}

// -----------------------------------------------------------------------

void SfxTabDialog::PageCreated

/*  [Description]

    Default implemetation of the virtual method. This is called immediately
    after creating a page. Here the dialogue can call the TabPage Method
    directly.
*/

(
    sal_uInt16,      // Id of the created page
    SfxTabPage&  // Reference to the created page
)
{
}

// -----------------------------------------------------------------------

SfxItemSet* SfxTabDialog::GetInputSetImpl()

/*  [Description]

    Derived classes may create new storage for the InputSet. This has to be
    released in the Destructor. To do this, this method must be called.
*/

{
    return (SfxItemSet*)pSet;
}

// -----------------------------------------------------------------------

SfxTabPage* SfxTabDialog::GetTabPage( sal_uInt16 nPageId ) const

/*  [Description]

    Return TabPage with the specified Id.
*/

{
    sal_uInt16 nPos = 0;
    Data_Impl* pDataObject = Find( *pImpl->pData, nPageId, &nPos );

    if ( pDataObject )
        return pDataObject->pTabPage;
    return NULL;
}

// -----------------------------------------------------------------------

sal_Bool SfxTabDialog::IsInOK() const
{
    return pImpl->bInOK;
}

// -----------------------------------------------------------------------

short SfxTabDialog::Ok()

/*  [Description]

    Ok handler for the Dialogue.
    The OutputSet is created and for each page this or the special OutputSet
    is set by calling the method <SfxTabPage::FillItemSet(SfxItemSet &)>, to
    insert the entered data by the user into the set.

    [Return value]

    RET_OK:       if at least one page has returned from FillItemSet,
                  otherwise RET_CANCEL.
*/

{
    pImpl->bInOK = sal_True;

    if ( !pOutSet )
    {
        if ( !pExampleSet && pSet )
            pOutSet = pSet->Clone( sal_False );  // without Items
        else if ( pExampleSet )
            pOutSet = new SfxItemSet( *pExampleSet );
    }
    sal_Bool bModified = sal_False;

    const sal_uInt16 nCount = pImpl->pData->Count();

    for ( sal_uInt16 i = 0; i < nCount; ++i )
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
                    bModified |= sal_True;
                    pExampleSet->Put( aTmp );
                    pOutSet->Put( aTmp );
                }
            }
        }
    }

    if ( pImpl->bModified || ( pOutSet && pOutSet->Count() > 0 ) )
        bModified |= sal_True;

    if ( bFmt == 2 )
        bModified |= sal_True;
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

SfxItemSet* SfxTabDialog::CreateInputItemSet( sal_uInt16 )

/*  [Description]

    Default implemetation of the virtual Method.
    This is called when pages create their sets onDenamd.
*/

{
    DBG_WARNINGFILE( "CreateInputItemSet not implemented" );
    return new SfxAllItemSet( SFX_APP()->GetPool() );
}

// -----------------------------------------------------------------------

const SfxItemSet* SfxTabDialog::GetRefreshedSet()

/*  [Description]

    Default implemetation of the virtual Method.
    This is called, when <SfxTabPage::DeactivatePage(SfxItemSet *)>
    returns <SfxTabPage::REFRESH_SET>.
*/

{
    DBG_ERRORFILE( "GetRefreshedSet not implemented" );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, OkHdl, Button *, EMPTYARG )

/*  [Description]

    Handler of the Ok-Buttons
    This calls the current page <SfxTabPage::DeactivatePage(SfxItemSet *)>.
    Returns <SfxTabPage::LEAVE_PAGE>, <SfxTabDialog::Ok()> is called
    anf the Dialog is ended.
*/

{
    pImpl->bInOK = sal_True;

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
    sal_uInt16 const nId = aTabCtrl.GetCurPageId();
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

/*  [Description]

    Handler of the User-Buttons
    This calls the current page <SfxTabPage::DeactivatePage(SfxItemSet *)>.
    returns this <SfxTabPage::LEAVE_PAGE> and  <SfxTabDialog::Ok()> is called.
    Then the Dialog is ended with the Return value <SfxTabDialog::Ok()>
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

/*  [Description]

    Handler behind the reset button.
    The Current Page is new initialized with their initial data, all the
    settings that the user has made on this page are repealed.
*/

{
    const sal_uInt16 nId = aTabCtrl.GetCurPageId();
    Data_Impl* pDataObject = Find( *pImpl->pData, nId );
    DBG_ASSERT( pDataObject, "Id not known" );

    if ( pDataObject->bOnDemand )
    {
        // CSet on AIS has problems here, thus separated
        const SfxItemSet* pItemSet = &pDataObject->pTabPage->GetItemSet();
        pDataObject->pTabPage->Reset( *(SfxItemSet*)pItemSet );
    }
    else
        pDataObject->pTabPage->Reset( *pSet );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, BaseFmtHdl, Button *, EMPTYARG )

/*  [Description]

    Handler behind the Standard-Button.
    This button is available when editing style sheets. All the set attributes
    in the edited stylesheet are deleted.
*/

{
    const sal_uInt16 nId = aTabCtrl.GetCurPageId();
    Data_Impl* pDataObject = Find( *pImpl->pData, nId );
    DBG_ASSERT( pDataObject, "Id not known" );
    bFmt = 2;

    if ( pDataObject->fnGetRanges )
    {
        if ( !pExampleSet )
            pExampleSet = new SfxItemSet( *pSet );

        const SfxItemPool* pPool = pSet->GetPool();
        const sal_uInt16* pTmpRanges = (pDataObject->fnGetRanges)();
        SfxItemSet aTmpSet( *pExampleSet );

        while ( *pTmpRanges )
        {
            const sal_uInt16* pU = pTmpRanges + 1;

            if ( *pTmpRanges == *pU )
            {
                // Range which two identical values -> only set one Item
                sal_uInt16 nWh = pPool->GetWhich( *pTmpRanges );
                pExampleSet->ClearItem( nWh );
                aTmpSet.ClearItem( nWh );
                // At the Outset of InvalidateItem,
                // so that the change takes effect
                pOutSet->InvalidateItem( nWh );
            }
            else
            {
                // Correct Range with multiple values
                sal_uInt16 nTmp = *pTmpRanges, nTmpEnd = *pU;
                DBG_ASSERT( nTmp <= nTmpEnd, "Range is sorted the wrong way" );

                if ( nTmp > nTmpEnd )
                {
                    // If really sorted wrongly, then set new
                    sal_uInt16 nTmp1 = nTmp;
                    nTmp = nTmpEnd;
                    nTmpEnd = nTmp1;
                }

                while ( nTmp <= nTmpEnd )
                {
                    // Iterate over the Range and set the Items
                    sal_uInt16 nWh = pPool->GetWhich( nTmp );
                    pExampleSet->ClearItem( nWh );
                    aTmpSet.ClearItem( nWh );
                    // At the Outset of InvalidateItem,
                    // so that the change takes effect
                    pOutSet->InvalidateItem( nWh );
                    nTmp++;
                }
            }
            // Go to the next pair
            pTmpRanges += 2;
        }
        // Set all Items as new  -> the call the current Page Reset()
        DBG_ASSERT( pDataObject->pTabPage, "the Page is gone" );
        pDataObject->pTabPage->Reset( aTmpSet );
        pDataObject->pTabPage->pImpl->mbStandard = sal_True;
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

/*  [Description]

    Handler that is called by StarView for switching to a different page.
    If the page not exist yet then it is created and the virtual Method
    <SfxTabDialog::PageCreated( sal_uInt16, SfxTabPage &)> is called. If the page
    exist, then the if possible the <SfxTabPage::Reset(const SfxItemSet &)> or
    <SfxTabPage::ActivatePage(const SfxItemSet &)> is called.
*/

{
    sal_uInt16 const nId = pTabCtrl->GetCurPageId();

    DBG_ASSERT( pImpl->pData->Count(), "no Pages registered" );
    SFX_APP();

    // Tab Page schon da?
    SfxTabPage* pTabPage = dynamic_cast<SfxTabPage*> (pTabCtrl->GetTabPage( nId ));
    Data_Impl* pDataObject = Find( *pImpl->pData, nId );
    DBG_ASSERT( pDataObject, "Id not known" );

    // Create TabPage if possible:
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
        // Only set Size on TabControl when < as TabPage
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
    pDataObject->bRefresh = sal_False;

#if ENABLE_LAYOUT_SFX_TABDIALOG
    pTabCtrl->GetPagePos (nId);
#endif /* ENABLE_LAYOUT_SFX_TABDIALOG */

    if ( pExampleSet )
        pTabPage->ActivatePage( *pExampleSet );
    sal_Bool bReadOnly = pTabPage->IsReadOnly();
    ( bReadOnly || pImpl->bHideResetBtn ) ? aResetBtn.Hide() : aResetBtn.Show();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxTabDialog, DeactivatePageHdl, TabControl *, pTabCtrl )

/*  [Description]

    Handler that is called by StarView before leaving a page.

    [Cross-reference]

    <SfxTabPage::DeactivatePage(SfxItemSet *)>
*/

{
    sal_uInt16 nId = pTabCtrl->GetCurPageId();
    SFX_APP();
    SfxTabPage *pPage = dynamic_cast<SfxTabPage*> (pTabCtrl->GetTabPage( nId ));
    DBG_ASSERT( pPage, "no active Page" );
#ifdef DBG_UTIL
    Data_Impl* pDataObject = Find( *pImpl->pData, pTabCtrl->GetCurPageId() );
    DBG_ASSERT( pDataObject, "no Data structur for current page" );
    if ( pPage->HasExchangeSupport() && pDataObject->bOnDemand )
    {
        DBG_WARNING( "Data exchange in ItemsOnDemand is not desired!" );
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
        DBG_ASSERT( pSet, "GetRefreshedSet() returns NULL" );
        // Flag all Pages as to be initialized as new
        const sal_uInt16 nCount = pImpl->pData->Count();

        for ( sal_uInt16 i = 0; i < nCount; ++i )
        {
            Data_Impl* pObj = (*pImpl->pData)[i];

            if ( pObj->pTabPage != pPage ) // Do not refresh own Page anymore
                pObj->bRefresh = sal_True;
            else
                pObj->bRefresh = sal_False;
        }
    }
    if ( nRet & SfxTabPage::LEAVE_PAGE )
        return sal_True;
    else
        return sal_False;
}

// -----------------------------------------------------------------------

const SfxItemSet* SfxTabDialog::GetOutputItemSet

/*  [Description]

    Return the pages that provide their sets onDemand, the OutputItemSet.

    [Cross-reference]

    <SfxTabDialog::AddTabPage(sal_uInt16, CreateTabPage, GetTabPageRanges, sal_Bool)>
    <SfxTabDialog::AddTabPage(sal_uInt16, const String &, CreateTabPage, GetTabPageRanges, sal_Bool, sal_uInt16)>
    <SfxTabDialog::AddTabPage(sal_uInt16, const Bitmap &, CreateTabPage, GetTabPageRanges, sal_Bool, sal_uInt16)>
*/

(
    sal_uInt16 nId  // the Id, under which the page was added at AddTabPage().
                // hinzugefügt wurde.
) const
{
    Data_Impl* pDataObject = Find( *pImpl->pData, nId );
    DBG_ASSERT( pDataObject, "TabPage not found" );

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

/*  [Description]

    Comparison function for qsort
*/

{
    return *(sal_uInt16*)p1 - *(sal_uInt16*)p2;
}

// -----------------------------------------------------------------------

void SfxTabDialog::ShowPage( sal_uInt16 nId )

/*  [Description]

    The TabPage is activated with the specified Id.
*/

{
    aTabCtrl.SetCurPageId( nId );
    ActivatePageHdl( &aTabCtrl );
}

// -----------------------------------------------------------------------

const sal_uInt16* SfxTabDialog::GetInputRanges( const SfxItemPool& rPool )

/*  [Description]

    Makes the set over the range of all pages of the dialogue. Pages have the
    static method for querying their range in AddTabPage, ie deliver their
    sets onDemand.

    [Return value]

    Pointer to a null-terminated array of sal_uInt16. This array belongs to the
    dialog and is deleted when the dialogue is destroy.

    [Cross-reference]

    <SfxTabDialog::AddTabPage(sal_uInt16, CreateTabPage, GetTabPageRanges, sal_Bool)>
    <SfxTabDialog::AddTabPage(sal_uInt16, const String &, CreateTabPage, GetTabPageRanges, sal_Bool, sal_uInt16)>
    <SfxTabDialog::AddTabPage(sal_uInt16, const Bitmap &, CreateTabPage, GetTabPageRanges, sal_Bool, sal_uInt16)>
*/

{
    if ( pSet )
    {
        DBG_ERRORFILE( "Set already exists!" );
        return pSet->GetRanges();
    }

    if ( pRanges )
        return pRanges;
    SvUShorts aUS( 16, 16 );
    sal_uInt16 nCount = pImpl->pData->Count();

    sal_uInt16 i;
    for ( i = 0; i < nCount; ++i )
    {
        Data_Impl* pDataObject = pImpl->pData->GetObject(i);

        if ( pDataObject->fnGetRanges )
        {
            const sal_uInt16* pTmpRanges = (pDataObject->fnGetRanges)();
            const sal_uInt16* pIter = pTmpRanges;

            sal_uInt16 nLen;
            for( nLen = 0; *pIter; ++nLen, ++pIter )
                ;
            aUS.Insert( pTmpRanges, nLen, aUS.Count() );
        }
    }

    //! Remove duplicated Ids?
#ifndef TF_POOLABLE
    if ( rPool.HasMap() )
#endif
    {
        nCount = aUS.Count();

        for ( i = 0; i < nCount; ++i )
            aUS[i] = rPool.GetWhich( aUS[i] );
    }

    // sort
    if ( aUS.Count() > 1 )
        qsort( (void*)aUS.GetData(),
               aUS.Count(), sizeof(sal_uInt16), TabDlgCmpUS_Impl );

    pRanges = new sal_uInt16[aUS.Count() + 1];
    memcpy(pRanges, aUS.GetData(), sizeof(sal_uInt16) * aUS.Count());
    pRanges[aUS.Count()] = 0;
    return pRanges;
}

// -----------------------------------------------------------------------

void SfxTabDialog::SetInputSet( const SfxItemSet* pInSet )

/*  [Description]

    With this method the Input-Set can subsequently be set initally or re-set.
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
            rtl::OString sHelpId;
            while ( !sHelpId.getLength() && pWindow )
            {
                sHelpId = pWindow->GetHelpId();
                pWindow = pWindow->GetParent();
            }

            if ( sHelpId.getLength() )
                SfxHelp::OpenHelpAgent( &pViewFrame->GetFrame(), sHelpId );
        }
    }

    return TabDialog::Notify( rNEvt );
}

END_NAMESPACE_LAYOUT_SFX_TABDIALOG

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
