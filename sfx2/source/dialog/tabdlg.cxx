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


#include <limits.h>
#include <stdlib.h>
#include <algorithm>
#include <vcl/builder.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/viewoptions.hxx>

#include "appdata.hxx"
#include "sfxtypes.hxx"
#include <sfx2/tabdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxhelp.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/itemconnect.hxx>

#include "dialog.hrc"
#include "helpid.hrc"

using namespace ::com::sun::star::uno;
using namespace ::rtl;

#define USERITEM_NAME           OUString("UserItem")

TYPEINIT1(SfxTabDialogItem,SfxSetItem);

struct TabPageImpl
{
    bool                        mbStandard;
    sfx::ItemConnectionArray    maItemConn;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;

    TabPageImpl() : mbStandard( false ) {}
};

struct Data_Impl
{
    sal_uInt16 nId;                   // The ID
    CreateTabPage fnCreatePage;   // Pointer to Factory
    GetTabPageRanges fnGetRanges; // Pointer to Ranges-Function
    SfxTabPage* pTabPage;         // The TabPage itself
    bool bOnDemand;              // Flag: ItemSet onDemand
    bool bRefresh;                // Flag: Page must be re-initialized

    // Constructor
    Data_Impl( sal_uInt16 Id, CreateTabPage fnPage,
               GetTabPageRanges fnRanges, bool bDemand ) :

        nId         ( Id ),
        fnCreatePage( fnPage ),
        fnGetRanges ( fnRanges ),
        pTabPage    ( 0 ),
        bOnDemand   ( bDemand ),
        bRefresh    ( false )
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

typedef std::vector<Data_Impl*> SfxTabDlgData_Impl;

struct TabDlg_Impl
{
    bool                bModified       : 1,
                        bModal          : 1,
                        bHideResetBtn   : 1;
    SfxTabDlgData_Impl  aData;

    TabDlg_Impl( sal_uInt8 nCnt ) :

        bModified       ( false ),
        bModal          ( true ),
        bHideResetBtn   ( false )
    {
        aData.reserve( nCnt );
    }
    ~TabDlg_Impl()
    {
    }
};


static Data_Impl* Find( const SfxTabDlgData_Impl& rArr, sal_uInt16 nId, sal_uInt16* pPos = 0)
{
    const sal_uInt16 nCount = rArr.size();

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
    bHasExchangeSupport ( false ),
    pImpl               ( new TabPageImpl )

{
}


SfxTabPage::SfxTabPage(Window *pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet &rAttrSet)
    : TabPage(pParent, rID, rUIXMLDescription)
    , pSet                ( &rAttrSet )
    , bHasExchangeSupport ( false )
    , pImpl               ( new TabPageImpl )
{
}



SfxTabPage::~SfxTabPage()

/*  [Description]

    Destructor
*/

{
    delete pImpl;
}



bool SfxTabPage::FillItemSet( SfxItemSet& rSet )
{
    return pImpl->maItemConn.DoFillItemSet( rSet, GetItemSet() );
}



void SfxTabPage::Reset( const SfxItemSet& rSet )
{
    pImpl->maItemConn.DoApplyFlags( rSet );
    pImpl->maItemConn.DoReset( rSet );
}



void SfxTabPage::ActivatePage( const SfxItemSet& )

/*  [Description]

    Default implementation of the virtual ActivatePage method. This method is
    called when a page of dialogue supports the exchange of data between pages.
    <SfxTabPage::DeactivatePage(SfxItemSet *)>
*/

{
}



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



void SfxTabPage::FillUserData()

/*  [Description]

    Virtual method is called by the base class in the destructor to save
    specific information of the TabPage in the ini-file. When overloading a
    string must be compiled, which is then flushed with the <SetUserData()>.
*/

{
}



bool SfxTabPage::IsReadOnly() const
{
    return false;
}



const SfxPoolItem* SfxTabPage::GetItem( const SfxItemSet& rSet, sal_uInt16 nSlot, bool bDeep )

/*  [Description]

    static Method: hereby are the implementations of the TabPage code
    beeing simplified.
*/

{
    const SfxItemPool* pPool = rSet.GetPool();
    sal_uInt16 nWh = pPool->GetWhich( nSlot, bDeep );
    const SfxPoolItem* pItem = 0;
    rSet.GetItemState( nWh, true, &pItem );

    if ( !pItem && nWh != nSlot )
        pItem = &pPool->GetDefaultItem( nWh );
    return pItem;
}



const SfxPoolItem* SfxTabPage::GetOldItem( const SfxItemSet& rSet,
                                           sal_uInt16 nSlot, bool bDeep )

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

void SfxTabPage::PageCreated( const SfxAllItemSet& /*aSet*/ )
{
    DBG_ASSERT(false, "SfxTabPage::PageCreated should not be called");
}



void SfxTabPage::AddItemConnection( sfx::ItemConnectionBase* pConnection )
{
    pImpl->maItemConn.AddConnection( pConnection );
}

SfxTabDialog* SfxTabPage::GetTabDialog() const
{
    return dynamic_cast<SfxTabDialog*>(GetParentDialog());
}



SfxTabDialog::SfxTabDialog
(
    SfxViewFrame* pViewFrame,     // Frame, to which the Dialog belongs
    Window* pParent,              // Parent Window
    const OString& rID, const OUString& rUIXMLDescription, //Dialog Name, Dialog .ui path
    const SfxItemSet* pItemSet,   // Itemset with the data;
                                  // can be NULL, when Pages are onDemand
    bool bEditFmt                 // when yes -> additional Button for standard
)
    : TabDialog(pParent, rID, rUIXMLDescription)
    , pFrame(pViewFrame)
    , pSet(pItemSet)
    , pOutSet(0)
    , pRanges(0)
    , nAppPageId(USHRT_MAX)
    , bItemsReset(false)
    , bStandardPushed(false)
    , pExampleSet(0)
{
    Init_Impl(bEditFmt, NULL, NULL);
}



SfxTabDialog::SfxTabDialog

/*  [Description]

    Constructor, temporary without Frame
*/

(
    Window* pParent,              // Parent Window
    const OString& rID, const OUString& rUIXMLDescription, //Dialog Name, Dialog .ui path
    const SfxItemSet* pItemSet,   // Itemset with the data;
                                  // can be NULL, when Pages are onDemand
    bool bEditFmt                 // when yes -> additional Button for standard
)
    : TabDialog(pParent, rID, rUIXMLDescription)
    , pFrame(0)
    , pSet(pItemSet)
    , pOutSet(0)
    , pRanges(0)
    , nAppPageId(USHRT_MAX)
    , bItemsReset(false)
    , bStandardPushed(false)
    , pExampleSet(0)
{
    Init_Impl(bEditFmt, NULL, NULL);
    DBG_WARNING( "Please use the Construtor with the ViewFrame" );
}



SfxTabDialog::~SfxTabDialog()
{
    SavePosAndId();

    for ( SfxTabDlgData_Impl::const_iterator it = pImpl->aData.begin(); it != pImpl->aData.end(); ++it )
    {
        Data_Impl* pDataObject = *it;

        if ( pDataObject->pTabPage )
        {
            // save settings of all pages (user data)
            pDataObject->pTabPage->FillUserData();
            OUString aPageData( pDataObject->pTabPage->GetUserData() );
            if ( !aPageData.isEmpty() )
            {
                // save settings of all pages (user data)
                OUString sConfigId = OStringToOUString(pDataObject->pTabPage->GetConfigId(),
                    RTL_TEXTENCODING_UTF8);
                if (sConfigId.isEmpty())
                {
                    SAL_WARN("sfx.config", "Tabpage needs to be converted to .ui format");
                    sConfigId = OUString::number(pDataObject->nId);
                }

                SvtViewOptions aPageOpt(E_TABPAGE, sConfigId);
                aPageOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( aPageData ) ) );
            }

            if ( pDataObject->bOnDemand )
                delete (SfxItemSet*)&pDataObject->pTabPage->GetItemSet();
            delete pDataObject->pTabPage;
        }
        delete pDataObject;
    }

    delete pImpl;
    delete pOutSet;
    delete pExampleSet;
    delete [] pRanges;

    if (m_bOwnsBaseFmtBtn)
        delete m_pBaseFmtBtn;
    if (m_bOwnsResetBtn)
        delete m_pResetBtn;
    if (m_bOwnsHelpBtn)
        delete m_pHelpBtn;
    if (m_bOwnsCancelBtn)
        delete m_pCancelBtn;
    if (m_bOwnsUserBtn)
        delete m_pUserBtn;
    if (m_bOwnsApplyBtn)
        delete m_pApplyBtn;
    if (m_bOwnsOKBtn)
        delete m_pOKBtn;
    if (m_bOwnsActionArea)
        delete m_pActionArea;
    if (m_bOwnsTabCtrl)
        delete m_pTabCtrl;
    if (m_bOwnsVBox)
        delete m_pBox;
}



void SfxTabDialog::Init_Impl(bool bFmtFlag, const OUString* pUserButtonText, const ResId *pResId)

/*  [Description]

    internal initialization of the dialogue
*/

{
    m_pBox = get_content_area();
    m_bOwnsVBox = m_pBox == NULL;
    if (m_bOwnsVBox)
    {
        m_pBox = new VclVBox(this, false, 7);
        m_pBox->set_expand(true);
        set_content_area(m_pBox);
    }

    m_pTabCtrl = m_pUIBuilder ? m_pUIBuilder->get<TabControl>("tabcontrol") : NULL;
    m_bOwnsTabCtrl = m_pTabCtrl == NULL;
    if (m_bOwnsTabCtrl)
    {
        m_pTabCtrl = new TabControl(m_pBox, ResId(ID_TABCONTROL, *pResId->GetResMgr()));
        m_pTabCtrl->set_expand(true);
    }

    pImpl = new TabDlg_Impl(m_pTabCtrl->GetPageCount());

    m_pActionArea = get_action_area();
    m_bOwnsActionArea = m_pActionArea == NULL;
    if (m_bOwnsActionArea)
    {
        m_pActionArea = new VclHButtonBox(m_pBox);
        set_action_area(m_pActionArea);
    }

    m_pOKBtn = m_pUIBuilder ? m_pUIBuilder->get<PushButton>("ok") : NULL;
    m_bOwnsOKBtn = m_pOKBtn == NULL;
    if (m_bOwnsOKBtn)
        m_pOKBtn = new OKButton(m_pActionArea);

    m_pApplyBtn = m_pUIBuilder ? m_pUIBuilder->get<PushButton>("apply") : NULL;
    m_bOwnsApplyBtn = m_pApplyBtn == NULL;
    if (m_bOwnsApplyBtn)
        m_pApplyBtn = NULL;

    m_pUserBtn = m_pUIBuilder ? m_pUIBuilder->get<PushButton>("user") : NULL;
    m_bOwnsUserBtn = m_pUserBtn == NULL;
    if (m_bOwnsUserBtn)
        m_pUserBtn = pUserButtonText ? new PushButton(m_pActionArea) : NULL;

    m_pCancelBtn = m_pUIBuilder ? m_pUIBuilder->get<CancelButton>("cancel") : NULL;
    m_bOwnsCancelBtn = m_pCancelBtn == NULL;
    if (m_bOwnsCancelBtn)
        m_pCancelBtn = new CancelButton(m_pActionArea);

    m_pHelpBtn = m_pUIBuilder ? m_pUIBuilder->get<HelpButton>("help") : NULL;
    m_bOwnsHelpBtn = m_pHelpBtn == NULL;
    if (m_bOwnsHelpBtn)
        m_pHelpBtn = new HelpButton(m_pActionArea);

    m_pResetBtn = m_pUIBuilder ? m_pUIBuilder->get<PushButton>("reset") : NULL;
    m_bOwnsResetBtn = m_pResetBtn == NULL;
    if (m_bOwnsResetBtn)
        m_pResetBtn = new PushButton(m_pActionArea);
    else
        pImpl->bHideResetBtn = !m_pResetBtn->IsVisible();

    m_pBaseFmtBtn = m_pUIBuilder ? m_pUIBuilder->get<PushButton>("standard") : NULL;
    m_bOwnsBaseFmtBtn = m_pBaseFmtBtn == NULL;
    if (m_bOwnsBaseFmtBtn)
        m_pBaseFmtBtn = new PushButton(m_pActionArea);

    m_pOKBtn->SetClickHdl( LINK( this, SfxTabDialog, OkHdl ) );
    m_pCancelBtn->SetClickHdl( LINK( this, SfxTabDialog, CancelHdl ) );
    m_pResetBtn->SetClickHdl( LINK( this, SfxTabDialog, ResetHdl ) );
    m_pResetBtn->SetText( SfxResId( STR_RESET ).toString() );
    m_pTabCtrl->SetActivatePageHdl(
            LINK( this, SfxTabDialog, ActivatePageHdl ) );
    m_pTabCtrl->SetDeactivatePageHdl(
            LINK( this, SfxTabDialog, DeactivatePageHdl ) );
    m_pActionArea->Show();
    m_pBox->Show();
    m_pTabCtrl->Show();
    m_pOKBtn->Show();
    m_pCancelBtn->Show();
    m_pHelpBtn->Show();
    m_pResetBtn->Show();
    m_pResetBtn->SetHelpId( HID_TABDLG_RESET_BTN );

    if ( m_pUserBtn )
    {
        if (pUserButtonText)
            m_pUserBtn->SetText(*pUserButtonText);
        m_pUserBtn->SetClickHdl( LINK( this, SfxTabDialog, UserHdl ) );
        m_pUserBtn->Show();
    }

    if ( bFmtFlag )
    {
        m_pBaseFmtBtn->SetText( SfxResId( STR_STANDARD_SHORTCUT ).toString() );
        m_pBaseFmtBtn->SetClickHdl( LINK( this, SfxTabDialog, BaseFmtHdl ) );
        m_pBaseFmtBtn->SetHelpId( HID_TABDLG_STANDARD_BTN );
        m_pBaseFmtBtn->Show();
    }

    if ( pSet )
    {
        pExampleSet = new SfxItemSet( *pSet );
        pOutSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );
    }
}

void SfxTabDialog::RemoveResetButton()
{
    m_pResetBtn->Hide();
    pImpl->bHideResetBtn = true;
}

void SfxTabDialog::RemoveStandardButton()
{
    m_pBaseFmtBtn->Hide();
}

short SfxTabDialog::Execute()
{
    if ( !m_pTabCtrl->GetPageCount() )
        return RET_CANCEL;
    Start_Impl();
    return TabDialog::Execute();
}



void SfxTabDialog::StartExecuteModal( const Link& rEndDialogHdl )
{
    if ( !m_pTabCtrl->GetPageCount() )
        return;
    Start_Impl();
    TabDialog::StartExecuteModal( rEndDialogHdl );
}



void SfxTabDialog::Start( bool bShow )
{
    pImpl->bModal = false;
    Start_Impl();

    if ( bShow )
        Show();

    if ( IsVisible() && ( !HasChildPathFocus() || HasFocus() ) )
        GrabFocusToFirstControl();
}



void SfxTabDialog::SetApplyHandler(const Link& _rHdl)
{
    DBG_ASSERT( m_pApplyBtn, "SfxTabDialog::GetApplyHandler: no apply button enabled!" );
    if ( m_pApplyBtn )
        m_pApplyBtn->SetClickHdl( _rHdl );
}



void SfxTabDialog::Start_Impl()
{
    DBG_ASSERT( pImpl->aData.size() == m_pTabCtrl->GetPageCount(), "not all pages registered" );
    sal_uInt16 nActPage = m_pTabCtrl->GetPageId( 0 );

    // load old settings, when exists
    SvtViewOptions aDlgOpt(E_TABDIALOG, OStringToOUString(GetHelpId(),RTL_TEXTENCODING_UTF8));
    if ( aDlgOpt.Exists() )
    {
        SetWindowState(OUStringToOString(aDlgOpt.GetWindowState().getStr(), RTL_TEXTENCODING_ASCII_US));

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

        if ( TAB_PAGE_NOTFOUND == m_pTabCtrl->GetPagePos( nActPage ) )
            nActPage = m_pTabCtrl->GetPageId( 0 );
    }
    else if ( USHRT_MAX != nAppPageId && TAB_PAGE_NOTFOUND != m_pTabCtrl->GetPagePos( nAppPageId ) )
        nActPage = nAppPageId;

    m_pTabCtrl->SetCurPageId( nActPage );
    ActivatePageHdl( m_pTabCtrl );
}

void SfxTabDialog::AddTabPage( sal_uInt16 nId, const OUString &rRiderText, bool bItemsOnDemand, sal_uInt16 nPos )
{
    AddTabPage( nId, rRiderText, 0, 0, bItemsOnDemand, nPos );
}

/*
    Adds a page to the dialog. The Name must correspond to a entry in the
    TabControl in the dialog .ui
*/
sal_uInt16 SfxTabDialog::AddTabPage
(
    const OString &rName,          // Page ID
    CreateTabPage pCreateFunc,     // Pointer to the Factory Method
    GetTabPageRanges pRangesFunc,  // Pointer to the Method for quering
                                   // Ranges onDemand
    bool bItemsOnDemand            // indicates whether the set of this page is
                                   // requested when created
)
{
    sal_uInt16 nId = m_pTabCtrl->GetPageId(rName);
    pImpl->aData.push_back(
        new Data_Impl( nId, pCreateFunc, pRangesFunc, bItemsOnDemand ) );
    return nId;
}

/*
    Adds a page to the dialog. The Name must correspond to a entry in the
    TabControl in the dialog .ui
 */
sal_uInt16 SfxTabDialog::AddTabPage
(
    const OString &rName,          // Page ID
    sal_uInt16 nPageCreateId       // Identifier of the Factory Method to create the page
)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    assert(pFact);
    CreateTabPage pCreateFunc = pFact->GetTabPageCreatorFunc(nPageCreateId);
    assert(pCreateFunc);
    GetTabPageRanges pRangesFunc = pFact->GetTabPageRangesFunc(nPageCreateId);
    sal_uInt16 nPageId = m_pTabCtrl->GetPageId(rName);
    pImpl->aData.push_back(new Data_Impl(nPageId, pCreateFunc, pRangesFunc, false));
    return nPageId;
}



void SfxTabDialog::AddTabPage

/*  [Description]

    Add a page to the dialog. The Rider text is passed on, the page has no
    counterpart in the TabControl in the resource of the dialogue.
*/

(
    sal_uInt16 nId,
    const OUString& rRiderText,
    CreateTabPage pCreateFunc,
    GetTabPageRanges pRangesFunc,
    bool bItemsOnDemand,
    sal_uInt16 nPos
)
{
    DBG_ASSERT( TAB_PAGE_NOTFOUND == m_pTabCtrl->GetPagePos( nId ),
                "Double Page-Ids in the Tabpage" );
    m_pTabCtrl->InsertPage( nId, rRiderText, nPos );
    pImpl->aData.push_back(
        new Data_Impl( nId, pCreateFunc, pRangesFunc, bItemsOnDemand ) );
}

void SfxTabDialog::RemoveTabPage( sal_uInt16 nId )

/*  [Description]

    Delete the TabPage with ID nId
*/

{
    sal_uInt16 nPos = 0;
    m_pTabCtrl->RemovePage( nId );
    Data_Impl* pDataObject = Find( pImpl->aData, nId, &nPos );

    if ( pDataObject )
    {
        if ( pDataObject->pTabPage )
        {
            pDataObject->pTabPage->FillUserData();
            OUString aPageData( pDataObject->pTabPage->GetUserData() );
            if ( !aPageData.isEmpty() )
            {
                // save settings of this page (user data)
                OUString sConfigId = OStringToOUString(pDataObject->pTabPage->GetConfigId(),
                    RTL_TEXTENCODING_UTF8);
                if (sConfigId.isEmpty())
                {
                    SAL_WARN("sfx.config", "Tabpage needs to be converted to .ui format");
                    sConfigId = OUString::number(pDataObject->nId);
                }

                SvtViewOptions aPageOpt(E_TABPAGE, sConfigId);
                aPageOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( aPageData ) ) );
            }

            if ( pDataObject->bOnDemand )
                delete (SfxItemSet*)&pDataObject->pTabPage->GetItemSet();
            delete pDataObject->pTabPage;
        }

        delete pDataObject;
        pImpl->aData.erase( pImpl->aData.begin() + nPos );
    }
    else
    {
        SAL_INFO( "sfx.dialog", "TabPage-Id not known" );
    }
}

void SfxTabDialog::RemoveTabPage(const OString &rName)
{
    RemoveTabPage(m_pTabCtrl->GetPageId(rName));
}



void SfxTabDialog::PageCreated

/*  [Description]

    Default implementation of the virtual method. This is called immediately
    after creating a page. Here the dialogue can call the TabPage Method
    directly.
*/

(
    sal_uInt16,      // Id of the created page
    SfxTabPage&  // Reference to the created page
)
{
}



SfxItemSet* SfxTabDialog::GetInputSetImpl()

/*  [Description]

    Derived classes may create new storage for the InputSet. This has to be
    released in the Destructor. To do this, this method must be called.
*/

{
    return (SfxItemSet*)pSet;
}



SfxTabPage* SfxTabDialog::GetTabPage( sal_uInt16 nPageId ) const

/*  [Description]

    Return TabPage with the specified Id.
*/

{
    sal_uInt16 nPos = 0;
    Data_Impl* pDataObject = Find( pImpl->aData, nPageId, &nPos );

    if ( pDataObject )
        return pDataObject->pTabPage;
    return NULL;
}

void SfxTabDialog::SavePosAndId()
{
    // save settings (screen position and current page)
    SvtViewOptions aDlgOpt(E_TABDIALOG, OStringToOUString(GetHelpId(),RTL_TEXTENCODING_UTF8));
    aDlgOpt.SetWindowState(OStringToOUString(GetWindowState(WINDOWSTATE_MASK_POS),RTL_TEXTENCODING_ASCII_US));
    // to-do replace with name of page when all pages are converted to .ui
    aDlgOpt.SetPageID( m_pTabCtrl->GetCurPageId() );
}



short SfxTabDialog::Ok()

/*  [Description]

    Ok handler for the Dialogue.

    Dialog's current location and current page are saved for the next time
    the dialog is shown.

    The OutputSet is created and for each page this or the special OutputSet
    is set by calling the method <SfxTabPage::FillItemSet(SfxItemSet &)>, to
    insert the entered data by the user into the set.

    [Return value]

    RET_OK:       if at least one page has returned from FillItemSet,
                  otherwise RET_CANCEL.
*/
{
    SavePosAndId(); //See fdo#38828 "Apply" resetting window position

    if ( !pOutSet )
    {
        if ( !pExampleSet && pSet )
            pOutSet = pSet->Clone( false );  // without Items
        else if ( pExampleSet )
            pOutSet = new SfxItemSet( *pExampleSet );
    }
    bool bModified = false;

    for ( SfxTabDlgData_Impl::const_iterator it = pImpl->aData.begin(); it != pImpl->aData.end(); ++it )
    {
        Data_Impl* pDataObject = *it;
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
                    bModified |= true;
                    if (pExampleSet)
                        pExampleSet->Put( aTmp );
                    pOutSet->Put( aTmp );
                }
            }
        }
    }

    if ( pImpl->bModified || ( pOutSet && pOutSet->Count() > 0 ) )
        bModified |= true;

    if (bStandardPushed)
        bModified |= true;
    return bModified ? RET_OK : RET_CANCEL;
}

IMPL_LINK_NOARG(SfxTabDialog, CancelHdl)
{
    EndDialog( RET_USER_CANCEL );
    return 0;
}



SfxItemSet* SfxTabDialog::CreateInputItemSet( sal_uInt16 )

/*  [Description]

    Default implementation of the virtual Method.
    This is called when pages create their sets onDemand.
*/

{
    SAL_WARN( "sfx.dialog", "CreateInputItemSet not implemented" );
    return new SfxAllItemSet( SFX_APP()->GetPool() );
}



const SfxItemSet* SfxTabDialog::GetRefreshedSet()

/*  [Description]

    Default implementation of the virtual Method.
    This is called, when <SfxTabPage::DeactivatePage(SfxItemSet *)>
    returns <SfxTabPage::REFRESH_SET>.
*/

{
    SAL_INFO ( "sfx.dialog", "GetRefreshedSet not implemented" );
    return 0;
}



IMPL_LINK_NOARG(SfxTabDialog, OkHdl)

/*  [Description]

    Handler of the Ok-Buttons
    This calls the current page <SfxTabPage::DeactivatePage(SfxItemSet *)>.
    Returns <SfxTabPage::LEAVE_PAGE>, <SfxTabDialog::Ok()> is called
    and the Dialog is ended.
*/

{
    if (PrepareLeaveCurrentPage())
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

bool SfxTabDialog::Apply()
{
    bool bApplied = false;
    if (PrepareLeaveCurrentPage())
         bApplied = (Ok() == RET_OK);
    return bApplied;
}



bool SfxTabDialog::PrepareLeaveCurrentPage()
{
    sal_uInt16 const nId = m_pTabCtrl->GetCurPageId();
    SfxTabPage* pPage = dynamic_cast<SfxTabPage*> (m_pTabCtrl->GetTabPage( nId ));
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




IMPL_LINK_NOARG(SfxTabDialog, UserHdl)

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



IMPL_LINK_NOARG(SfxTabDialog, ResetHdl)

/*  [Description]

    Handler behind the reset button.
    The Current Page is new initialized with their initial data, all the
    settings that the user has made on this page are repealed.
*/

{
    const sal_uInt16 nId = m_pTabCtrl->GetCurPageId();
    Data_Impl* pDataObject = Find( pImpl->aData, nId );
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



IMPL_LINK_NOARG(SfxTabDialog, BaseFmtHdl)

/*  [Description]

    Handler behind the Standard-Button.
    This button is available when editing style sheets. All the set attributes
    in the edited stylesheet are deleted.
*/

{
    bStandardPushed = true;

    const sal_uInt16 nId = m_pTabCtrl->GetCurPageId();
    Data_Impl* pDataObject = Find( pImpl->aData, nId );
    DBG_ASSERT( pDataObject, "Id not known" );

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
        pDataObject->pTabPage->pImpl->mbStandard = true;
    }
    return 1;
}



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

    DBG_ASSERT( pImpl->aData.size(), "no Pages registered" );
    SFX_APP();

    // Tab Page schon da?
    SfxTabPage* pTabPage = dynamic_cast<SfxTabPage*> (pTabCtrl->GetTabPage( nId ));
    Data_Impl* pDataObject = Find( pImpl->aData, nId );
    DBG_ASSERT( pDataObject, "Id not known" );

    // Create TabPage if possible:
    if ( !pTabPage )
    {
        const SfxItemSet* pTmpSet = 0;

        if ( pSet )
        {
            if ( bItemsReset && pSet->GetParent() )
                pTmpSet = pSet->GetParent();
            else
                pTmpSet = pSet;
        }

        if ( pTmpSet && !pDataObject->bOnDemand )
            pTabPage = (pDataObject->fnCreatePage)( pTabCtrl, *pTmpSet );
        else
            pTabPage = (pDataObject->fnCreatePage)
                            ( pTabCtrl, *CreateInputItemSet( nId ) );
        DBG_ASSERT( NULL == pDataObject->pTabPage, "create TabPage more than once" );
        pDataObject->pTabPage = pTabPage;

        OUString sConfigId = OStringToOUString(pTabPage->GetConfigId(), RTL_TEXTENCODING_UTF8);
        if (sConfigId.isEmpty())
        {
            SAL_WARN("sfx.config", "Tabpage needs to be converted to .ui format");
            sConfigId = OUString::number(pDataObject->nId);
        }
        SvtViewOptions aPageOpt(E_TABPAGE, sConfigId);
        OUString sUserData;
        Any aUserItem = aPageOpt.GetUserItem( USERITEM_NAME );
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            sUserData = aTemp;
        pTabPage->SetUserData( sUserData );
        Size aSiz = pTabPage->GetSizePixel();

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
    pDataObject->bRefresh = false;

    if ( pExampleSet )
        pTabPage->ActivatePage( *pExampleSet );
    bool bReadOnly = pTabPage->IsReadOnly();
    ( bReadOnly || pImpl->bHideResetBtn ) ? m_pResetBtn->Hide() : m_pResetBtn->Show();
    return 0;
}



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
    if (!pPage)
        return sal_False;
#ifdef DBG_UTIL
    Data_Impl* pDataObject = Find( pImpl->aData, pTabCtrl->GetCurPageId() );
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

        for ( SfxTabDlgData_Impl::const_iterator it = pImpl->aData.begin(); it != pImpl->aData.end(); ++it )
        {
            Data_Impl* pObj = *it;

            if ( pObj->pTabPage != pPage ) // Do not refresh own Page anymore
                pObj->bRefresh = true;
            else
                pObj->bRefresh = false;
        }
    }
    if ( nRet & SfxTabPage::LEAVE_PAGE )
        return sal_True;
    else
        return sal_False;
}



void SfxTabDialog::ShowPage( sal_uInt16 nId )

/*  [Description]

    The TabPage is activated with the specified Id.
*/

{
    m_pTabCtrl->SetCurPageId( nId );
    ActivatePageHdl( m_pTabCtrl );
}



const sal_uInt16* SfxTabDialog::GetInputRanges( const SfxItemPool& rPool )

/*  [Description]

    Makes the set over the range of all pages of the dialogue. Pages have the
    static method for querying their range in AddTabPage, ie deliver their
    sets onDemand.

    [Return value]

    Pointer to a null-terminated array of sal_uInt16. This array belongs to the
    dialog and is deleted when the dialogue is destroy.

    [Cross-reference]

    <SfxTabDialog::AddTabPage(sal_uInt16, CreateTabPage, GetTabPageRanges, bool)>
    <SfxTabDialog::AddTabPage(sal_uInt16, const String &, CreateTabPage, GetTabPageRanges, bool, sal_uInt16)>
    <SfxTabDialog::AddTabPage(sal_uInt16, const Bitmap &, CreateTabPage, GetTabPageRanges, bool, sal_uInt16)>
*/

{
    if ( pSet )
    {
        SAL_WARN( "sfx.dialog", "Set already exists!" );
        return pSet->GetRanges();
    }

    if ( pRanges )
        return pRanges;
    std::vector<sal_uInt16> aUS;

    for ( SfxTabDlgData_Impl::const_iterator it = pImpl->aData.begin(); it != pImpl->aData.end(); ++it )
    {
        Data_Impl* pDataObject = *it;

        if ( pDataObject->fnGetRanges )
        {
            const sal_uInt16* pTmpRanges = (pDataObject->fnGetRanges)();
            const sal_uInt16* pIter = pTmpRanges;

            sal_uInt16 nLen;
            for( nLen = 0; *pIter; ++nLen, ++pIter )
                ;
            aUS.insert( aUS.end(), pTmpRanges, pTmpRanges + nLen );
        }
    }

    //! Remove duplicated Ids?
    {
        sal_uInt16 nCount = aUS.size();
        for ( sal_uInt16 i = 0; i < nCount; ++i )
            aUS[i] = rPool.GetWhich( aUS[i] );
    }

    // sort
    if ( aUS.size() > 1 )
    {
        std::sort( aUS.begin(), aUS.end() );
    }

    pRanges = new sal_uInt16[aUS.size() + 1];
    std::copy( aUS.begin(), aUS.end(), pRanges );
    pRanges[aUS.size()] = 0;
    return pRanges;
}



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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
