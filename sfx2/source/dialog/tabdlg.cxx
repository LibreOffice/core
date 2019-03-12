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

#include <appdata.hxx>
#include <sfxtypes.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxhelp.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/viewsh.hxx>
#include <uitest/sfx_uiobject.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/builder.hxx>
#include <vcl/IDialogRenderable.hxx>
#include <sal/log.hxx>

#include <sfx2/strings.hrc>
#include <helpids.h>

using namespace ::com::sun::star::uno;

#define USERITEM_NAME           "UserItem"


struct TabPageImpl
{
    bool                        mbStandard;
    VclPtr<SfxTabDialog>        mxDialog;
    SfxOkDialogController*      mpDialogController;
    css::uno::Reference< css::frame::XFrame > mxFrame;

    TabPageImpl() : mbStandard(false), mpDialogController(nullptr) {}
};

struct Data_Impl
{
    sal_uInt16 const nId;               // The ID
    OString const sId;                  // The ID
    CreateTabPage fnCreatePage;   // Pointer to Factory
    GetTabPageRanges fnGetRanges; // Pointer to Ranges-Function
    VclPtr<SfxTabPage> pTabPage;         // The TabPage itself
    bool bRefresh;                // Flag: Page must be re-initialized

    // Constructor
    Data_Impl( sal_uInt16 Id, const OString& rId, CreateTabPage fnPage,
               GetTabPageRanges fnRanges ) :

        nId         ( Id ),
        sId         ( rId ),
        fnCreatePage( fnPage ),
        fnGetRanges ( fnRanges ),
        pTabPage    ( nullptr ),
        bRefresh    ( false )
    {
        if ( !fnCreatePage  )
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            fnCreatePage = pFact->GetTabPageCreatorFunc( nId );
            fnGetRanges = pFact->GetTabPageRangesFunc( nId );
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
    return nullptr;
}

typedef std::vector<Data_Impl*> SfxTabDlgData_Impl;

struct TabDlg_Impl
{
    bool                bModal          : 1,
                        bHideResetBtn   : 1;
    SfxTabDlgData_Impl  aData;

    explicit TabDlg_Impl( sal_uInt8 nCnt ) :
        bModal          ( true ),
        bHideResetBtn   ( false )
    {
        aData.reserve( nCnt );
    }
};


static Data_Impl* Find( const SfxTabDlgData_Impl& rArr, sal_uInt16 nId, sal_uInt16* pPos = nullptr)
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
    return nullptr;
}

static Data_Impl* Find( const SfxTabDlgData_Impl& rArr, const OString& rId, sal_uInt16* pPos = nullptr)
{
    const sal_uInt16 nCount = rArr.size();

    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        Data_Impl* pObj = rArr[i];

        if ( pObj->sId == rId )
        {
            if ( pPos )
                *pPos = i;
            return pObj;
        }
    }
    return nullptr;
}

void SfxTabPage::SetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    if (pImpl)
        pImpl->mxFrame = xFrame;
}

css::uno::Reference< css::frame::XFrame > SfxTabPage::GetFrame()
{
    if (pImpl)
        return pImpl->mxFrame;
    return css::uno::Reference< css::frame::XFrame >();
}

SfxTabPage::SfxTabPage(vcl::Window *pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet *rAttrSet)
    : TabPage(pParent, rID, rUIXMLDescription)
    , pSet                ( rAttrSet )
    , bHasExchangeSupport ( false )
    , pImpl               ( new TabPageImpl )
{
}

SfxTabPage::SfxTabPage(TabPageParent pParent, const OUString& rUIXMLDescription, const OString& rID, const SfxItemSet *rAttrSet)
    : TabPage(pParent.pPage ? Application::GetDefDialogParent() : pParent.pParent.get()) //just drag this along hidden in this scenario
    , pSet                ( rAttrSet )
    , bHasExchangeSupport ( false )
    , pImpl               ( new TabPageImpl )
    , m_xBuilder(pParent.pPage ? Application::CreateBuilder(pParent.pPage, rUIXMLDescription)
                               : Application::CreateInterimBuilder(this, rUIXMLDescription))
    , m_xContainer(m_xBuilder->weld_container(rID))
{
    pImpl->mpDialogController = dynamic_cast<SfxOkDialogController*>(pParent.pController);
}

SfxTabPage::~SfxTabPage()
{
    if (m_xContainer)
    {
        std::unique_ptr<weld::Container> xParent(m_xContainer->weld_parent());
        if (xParent)
            xParent->move(m_xContainer.get(), nullptr);
    }
    m_xContainer.reset();
    disposeOnce();
}

void SfxTabPage::dispose()
{
    pImpl.reset();
    m_xBuilder.reset();
    TabPage::dispose();
}

bool SfxTabPage::FillItemSet( SfxItemSet* )
{
    return false;
}

void SfxTabPage::Reset( const SfxItemSet* )
{
}

void SfxTabPage::ActivatePage( const SfxItemSet& )
/*  [Description]

    Default implementation of the virtual ActivatePage method. This method is
    called when a page of dialogue supports the exchange of data between pages.
    <SfxTabPage::DeactivatePage(SfxItemSet *)>
*/
{
}

DeactivateRC SfxTabPage::DeactivatePage( SfxItemSet* )

/*  [Description]

    Default implementation of the virtual DeactivatePage method. This method is
    called by Sfx when leaving a page; the application can, through the return
    value, control whether to leave the page. If the page is displayed through
    bHasExchangeSupport which supports data exchange between pages, then a
    pointer to the exchange set is passed as parameter. This takes on data for
    the exchange, then the set is available as a parameter in
    <SfxTabPage::ActivatePage(const SfxItemSet &)>.

    [Return value]

    DeactivateRC::LeavePage; Allow leaving the page
*/

{
    return DeactivateRC::LeavePage;
}


void SfxTabPage::FillUserData()

/*  [Description]

    Virtual method is called by the base class in the destructor to save
    specific information of the TabPage in the ini-file. When overriding a
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
    being simplified.
*/

{
    const SfxItemPool* pPool = rSet.GetPool();
    sal_uInt16 nWh = pPool->GetWhich( nSlot, bDeep );
    const SfxPoolItem* pItem = nullptr;
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
    const SfxPoolItem* pItem = nullptr;

    if ( pImpl->mbStandard && rOldSet.GetParent() )
        pItem = GetItem( *rOldSet.GetParent(), nSlot );
    else if ( rSet.GetParent() &&
              SfxItemState::DONTCARE == rSet.GetItemState( nWh ) )
        pItem = GetItem( *rSet.GetParent(), nSlot );
    else
        pItem = GetItem( rOldSet, nSlot );
    return pItem;
}

void SfxTabPage::PageCreated( const SfxAllItemSet& /*aSet*/ )
{
    SAL_WARN( "sfx.dialog", "SfxTabPage::PageCreated should not be called");
}

void SfxTabPage::ChangesApplied()
{
}

void SfxTabPage::SetTabDialog(SfxTabDialog* pDialog)
{
    pImpl->mxDialog = pDialog;
}

SfxTabDialog* SfxTabPage::GetTabDialog() const
{
    return pImpl->mxDialog;
}

void SfxTabPage::SetDialogController(SfxOkDialogController* pDialog)
{
    pImpl->mpDialogController = pDialog;
}

SfxOkDialogController* SfxTabPage::GetDialogController() const
{
    return pImpl->mpDialogController;
}

OString SfxTabPage::GetConfigId() const
{
    if (m_xContainer)
        return m_xContainer->get_help_id();
    OString sId(GetHelpId());
    if (sId.isEmpty() && isLayoutEnabled(this))
        sId = GetWindow(GetWindowType::FirstChild)->GetHelpId();
    return sId;
}

weld::Window* SfxTabPage::GetDialogFrameWeld() const
{
    if (pImpl->mpDialogController)
        return pImpl->mpDialogController->getDialog();
    return GetFrameWeld();
}

const SfxItemSet* SfxTabPage::GetDialogExampleSet() const
{
    if (pImpl->mpDialogController)
        return pImpl->mpDialogController->GetExampleSet();
    if (pImpl->mxDialog)
        return pImpl->mxDialog->GetExampleSet();
    return nullptr;
}

SfxTabDialog::SfxTabDialog

/*  [Description]

    Constructor, temporary without Frame
*/

(
    vcl::Window* pParent,              // Parent Window
    const OUString& rID, const OUString& rUIXMLDescription, //Dialog Name, Dialog .ui path
    const SfxItemSet* pItemSet    // Itemset with the data;
                                  // can be NULL, when Pages are onDemand
)
    : TabDialog(pParent, rID, rUIXMLDescription)
    , m_pSet(pItemSet ? new SfxItemSet(*pItemSet) : nullptr)
    , m_nAppPageId(USHRT_MAX)
    , m_bStandardPushed(false)
    , m_pExampleSet(nullptr)
{
    Init_Impl();

    sal_uInt16 nPageCount = m_pTabCtrl->GetPageCount();
    for (sal_uInt16 nPage = 0; nPage < nPageCount; ++nPage)
    {
        sal_uInt16 nPageId = m_pTabCtrl->GetPageId(nPage);
        m_pTabCtrl->SetTabPage(nPageId, nullptr);
    }
}

SfxTabDialog::~SfxTabDialog()
{
    disposeOnce();
}

void SfxTabDialog::dispose()
{
    SavePosAndId();

    for (auto & elem : m_pImpl->aData)
    {
        if ( elem->pTabPage )
        {
            // save settings of all pages (user data)
            elem->pTabPage->FillUserData();
            OUString aPageData( elem->pTabPage->GetUserData() );
            if ( !aPageData.isEmpty() )
            {
                // save settings of all pages (user data)
                OUString sConfigId = OStringToOUString(elem->pTabPage->GetConfigId(),
                    RTL_TEXTENCODING_UTF8);
                if (sConfigId.isEmpty())
                {
                    SAL_WARN("sfx.dialog", "Tabpage needs to be converted to .ui format");
                    sConfigId = OUString::number(elem->nId);
                }

                SvtViewOptions aPageOpt(EViewType::TabPage, sConfigId);
                aPageOpt.SetUserItem( USERITEM_NAME, makeAny( aPageData ) );
            }

            elem->pTabPage.disposeAndClear();
        }
        delete elem;
        elem = nullptr;
    }

    m_pImpl.reset();
    m_pSet.reset();
    m_pOutSet.reset();
    delete m_pExampleSet;
    m_pExampleSet = nullptr;
    m_pRanges.reset();

    if (m_bOwnsBaseFmtBtn)
        m_pBaseFmtBtn.disposeAndClear();
    if (m_bOwnsResetBtn)
        m_pResetBtn.disposeAndClear();
    if (m_bOwnsHelpBtn)
        m_pHelpBtn.disposeAndClear();
    if (m_bOwnsCancelBtn)
        m_pCancelBtn.disposeAndClear();
    if (m_bOwnsOKBtn)
        m_pOKBtn.disposeAndClear();
    m_pBox.clear();
    m_pTabCtrl.clear();
    m_pOKBtn.clear();
    m_pUserBtn.clear();
    m_pCancelBtn.clear();
    m_pHelpBtn.clear();
    m_pResetBtn.clear();
    m_pBaseFmtBtn.clear();
    m_pActionArea.clear();

    TabDialog::dispose();
}

void SfxTabDialog::Init_Impl()
/*  [Description]

    internal initialization of the dialogue
*/
{
    m_pBox = get_content_area();
    assert(m_pBox);
    m_pUIBuilder->get(m_pTabCtrl, "tabcontrol");

    m_pImpl.reset( new TabDlg_Impl(m_pTabCtrl->GetPageCount()) );

    m_pActionArea = get_action_area();
    assert(m_pActionArea);

    m_pOKBtn = m_pUIBuilder->get<PushButton>("ok");
    m_bOwnsOKBtn = m_pOKBtn == nullptr;
    if (m_bOwnsOKBtn)
        m_pOKBtn = VclPtr<OKButton>::Create(m_pActionArea);

    m_pCancelBtn = m_pUIBuilder->get<CancelButton>("cancel");
    m_bOwnsCancelBtn = m_pCancelBtn == nullptr;
    if (m_bOwnsCancelBtn)
        m_pCancelBtn = VclPtr<CancelButton>::Create(m_pActionArea);

    m_pHelpBtn = m_pUIBuilder->get<HelpButton>("help");
    m_bOwnsHelpBtn = m_pHelpBtn == nullptr;
    if (m_bOwnsHelpBtn)
        m_pHelpBtn = VclPtr<HelpButton>::Create(m_pActionArea);

    m_pResetBtn = m_pUIBuilder->get<PushButton>("reset");
    m_bOwnsResetBtn = m_pResetBtn == nullptr;
    if (m_bOwnsResetBtn)
    {
        m_pResetBtn = VclPtr<PushButton>::Create(m_pActionArea.get());
        m_pResetBtn->set_id("reset");
    }
    else
        m_pImpl->bHideResetBtn = !m_pResetBtn->IsVisible();

    m_pBaseFmtBtn = m_pUIBuilder->get<PushButton>("standard");
    m_bOwnsBaseFmtBtn = m_pBaseFmtBtn == nullptr;
    if (m_bOwnsBaseFmtBtn)
    {
        m_pBaseFmtBtn = VclPtr<PushButton>::Create(m_pActionArea.get());
        m_pBaseFmtBtn->set_id("standard");
    }

    m_pOKBtn->SetClickHdl( LINK( this, SfxTabDialog, OkHdl ) );
    m_pCancelBtn->SetClickHdl( LINK( this, SfxTabDialog, CancelHdl ) );
    m_pResetBtn->SetClickHdl( LINK( this, SfxTabDialog, ResetHdl ) );
    m_pResetBtn->SetText( SfxResId( STR_RESET ) );
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
        m_pUserBtn->SetClickHdl( LINK( this, SfxTabDialog, UserHdl ) );
        m_pUserBtn->Show();
    }

    if ( m_pSet )
    {
        m_pExampleSet = new SfxItemSet( *m_pSet );
        m_pOutSet.reset(new SfxItemSet( *m_pSet->GetPool(), m_pSet->GetRanges() ));
    }
}

short SfxTabDialog::Execute()
{
    if ( !m_pTabCtrl->GetPageCount() )
        return RET_CANCEL;
    Start_Impl();

    return TabDialog::Execute();
}

bool SfxTabDialog::StartExecuteAsync( VclAbstractDialog::AsyncContext &rCtx )
{
    if ( !m_pTabCtrl->GetPageCount() )
    {
        rCtx.mxOwner.disposeAndClear();
        return false;
    }
    Start_Impl();
    return TabDialog::StartExecuteAsync( rCtx );
}

void SfxTabDialog::Start()
{
    m_pImpl->bModal = false;
    Start_Impl();

    Show();

    if ( IsVisible() && ( !HasChildPathFocus() || HasFocus() ) )
        GrabFocusToFirstControl();
}


void SfxTabDialog::Start_Impl()
{
    assert(m_pImpl->aData.size() == m_pTabCtrl->GetPageCount()
            && "not all pages registered");
    sal_uInt16 nActPage = m_pTabCtrl->GetPageId( 0 );

    // load old settings, when exists
    SvtViewOptions aDlgOpt(EViewType::TabDialog, OStringToOUString(GetHelpId(),RTL_TEXTENCODING_UTF8));
    if ( aDlgOpt.Exists() )
    {
        SetWindowState(OUStringToOString(aDlgOpt.GetWindowState(), RTL_TEXTENCODING_ASCII_US));

        // initial TabPage from Program/Help/config
        nActPage = m_pTabCtrl->GetPageId(aDlgOpt.GetPageID());

        if ( USHRT_MAX != m_nAppPageId )
            nActPage = m_nAppPageId;

        if ( TAB_PAGE_NOTFOUND == m_pTabCtrl->GetPagePos( nActPage ) )
            nActPage = m_pTabCtrl->GetPageId( 0 );
    }
    else if ( USHRT_MAX != m_nAppPageId && TAB_PAGE_NOTFOUND != m_pTabCtrl->GetPagePos( m_nAppPageId ) )
        nActPage = m_nAppPageId;

    m_pTabCtrl->SetCurPageId( nActPage );
    ActivatePageHdl( m_pTabCtrl );
}

/*
    Adds a page to the dialog. The Name must correspond to a entry in the
    TabControl in the dialog .ui
*/
sal_uInt16 SfxTabDialog::AddTabPage
(
    const OString &rName,          // Page ID
    CreateTabPage pCreateFunc     // Pointer to the Factory Method
)
{
    sal_uInt16 nId = m_pTabCtrl->GetPageId(rName);
    m_pImpl->aData.push_back(new Data_Impl(nId, rName, pCreateFunc, nullptr));
    return nId;
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
    sal_uInt16 nPos
)
{
    DBG_ASSERT( TAB_PAGE_NOTFOUND == m_pTabCtrl->GetPagePos( nId ),
                "Double Page-Ids in the Tabpage" );
    m_pTabCtrl->InsertPage( nId, rRiderText, nPos );
    m_pImpl->aData.push_back( new Data_Impl(nId, "", pCreateFunc, nullptr ) );
}

void SfxTabDialog::RemoveTabPage( sal_uInt16 nId )

/*  [Description]

    Delete the TabPage with ID nId
*/

{
    sal_uInt16 nPos = 0;
    m_pTabCtrl->RemovePage( nId );
    Data_Impl* pDataObject = Find( m_pImpl->aData, nId, &nPos );

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
                    SAL_WARN("sfx.dialog", "Tabpage needs to be converted to .ui format");
                    sConfigId = OUString::number(pDataObject->nId);
                }

                SvtViewOptions aPageOpt(EViewType::TabPage, sConfigId);
                aPageOpt.SetUserItem( USERITEM_NAME, makeAny( aPageData ) );
            }

            pDataObject->pTabPage.disposeAndClear();
        }

        delete pDataObject;
        m_pImpl->aData.erase( m_pImpl->aData.begin() + nPos );
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
    return m_pSet.get();
}


SfxTabPage* SfxTabDialog::GetTabPage( sal_uInt16 nPageId ) const

/*  [Description]

    Return TabPage with the specified Id.
*/

{
    sal_uInt16 nPos = 0;
    Data_Impl* pDataObject = Find( m_pImpl->aData, nPageId, &nPos );

    if ( pDataObject )
        return pDataObject->pTabPage;
    return nullptr;
}

void SfxTabDialog::SavePosAndId()
{
    // save settings (screen position and current page)
    SvtViewOptions aDlgOpt(EViewType::TabDialog, OStringToOUString(GetHelpId(),RTL_TEXTENCODING_UTF8));
    aDlgOpt.SetWindowState(OStringToOUString(GetWindowState(WindowStateMask::Pos),RTL_TEXTENCODING_ASCII_US));
    // to-do replace with name of page when all pages are converted to .ui
    aDlgOpt.SetPageID(m_pTabCtrl->GetPageName(m_pTabCtrl->GetCurPageId()));
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

    if ( !m_pOutSet )
    {
        if ( m_pExampleSet )
            m_pOutSet.reset(new SfxItemSet( *m_pExampleSet ));
        else if ( m_pSet )
            m_pOutSet = m_pSet->Clone( false );  // without Items
    }
    bool bModified = false;

    for (auto const& elem : m_pImpl->aData)
    {
        SfxTabPage* pTabPage = elem->pTabPage;

        if ( pTabPage )
        {
            if ( m_pSet && !pTabPage->HasExchangeSupport() )
            {
                SfxItemSet aTmp( *m_pSet->GetPool(), m_pSet->GetRanges() );

                if ( pTabPage->FillItemSet( &aTmp ) )
                {
                    bModified = true;
                    if (m_pExampleSet)
                        m_pExampleSet->Put( aTmp );
                    m_pOutSet->Put( aTmp );
                }
            }
        }
    }

    if ( m_pOutSet && m_pOutSet->Count() > 0 )
        bModified = true;

    if (m_bStandardPushed)
        bModified = true;
    return bModified ? RET_OK : RET_CANCEL;
}

IMPL_LINK_NOARG(SfxTabDialog, CancelHdl, Button*, void)
{
    EndDialog( RET_USER_CANCEL );
}


SfxItemSet* SfxTabDialog::CreateInputItemSet( sal_uInt16 )

/*  [Description]

    Default implementation of the virtual Method.
    This is called when pages create their sets onDemand.
*/

{
    SAL_WARN( "sfx.dialog", "CreateInputItemSet not implemented" );
    return new SfxAllItemSet( SfxGetpApp()->GetPool() );
}


IMPL_LINK_NOARG(SfxTabDialog, OkHdl, Button*, void)

/*  [Description]

    Handler of the Ok-Buttons
    This calls the current page <SfxTabPage::DeactivatePage(SfxItemSet *)>.
    Returns <DeactivateRC::LeavePage>, <SfxTabDialog::Ok()> is called
    and the Dialog is ended.
*/

{
    if (PrepareLeaveCurrentPage())
    {
        if ( m_pImpl->bModal )
            EndDialog( Ok() );
        else
        {
            Ok();
            Close();
        }
    }
}

bool SfxTabDialog::PrepareLeaveCurrentPage()
{
    sal_uInt16 const nId = m_pTabCtrl->GetCurPageId();
    SfxTabPage* pPage = dynamic_cast<SfxTabPage*> (m_pTabCtrl->GetTabPage( nId ));
    bool bEnd = !pPage;

    if ( pPage )
    {
        DeactivateRC nRet = DeactivateRC::LeavePage;
        if ( m_pSet )
        {
            SfxItemSet aTmp( *m_pSet->GetPool(), m_pSet->GetRanges() );

            if ( pPage->HasExchangeSupport() )
                nRet = pPage->DeactivatePage( &aTmp );
            else
                nRet = pPage->DeactivatePage( nullptr );

            if ( ( DeactivateRC::LeavePage & nRet ) == DeactivateRC::LeavePage
                 && aTmp.Count() )
            {
                m_pExampleSet->Put( aTmp );
                m_pOutSet->Put( aTmp );
            }
        }
        else
            nRet = pPage->DeactivatePage( nullptr );
        bEnd = nRet != DeactivateRC::KeepPage;
    }

    return bEnd;
}


IMPL_LINK_NOARG(SfxTabDialog, UserHdl, Button*, void)

/*  [Description]

    Handler of the User-Buttons
    This calls the current page <SfxTabPage::DeactivatePage(SfxItemSet *)>.
    returns this <DeactivateRC::LeavePage> and  <SfxTabDialog::Ok()> is called.
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
}


IMPL_LINK_NOARG(SfxTabDialog, ResetHdl, Button*, void)

/*  [Description]

    Handler behind the reset button.
    The Current Page is new initialized with their initial data, all the
    settings that the user has made on this page are repealed.
*/

{
    const sal_uInt16 nId = m_pTabCtrl->GetCurPageId();
    Data_Impl* pDataObject = Find( m_pImpl->aData, nId );
    DBG_ASSERT( pDataObject, "Id not known" );

    pDataObject->pTabPage->Reset( m_pSet.get() );
    // Also reset relevant items of ExampleSet and OutSet to initial state
    if (!pDataObject->fnGetRanges)
        return;

    if (!m_pExampleSet)
        m_pExampleSet = new SfxItemSet(*m_pSet);

    const SfxItemPool* pPool = m_pSet->GetPool();
    const sal_uInt16* pTmpRanges = (pDataObject->fnGetRanges)();

    while (*pTmpRanges)
    {
        const sal_uInt16* pU = pTmpRanges + 1;

        // Correct Range with multiple values
        sal_uInt16 nTmp = *pTmpRanges, nTmpEnd = *pU;
        DBG_ASSERT(nTmp <= nTmpEnd, "Range is sorted the wrong way");

        if (nTmp > nTmpEnd)
        {
            // If really sorted wrongly, then set new
            std::swap(nTmp, nTmpEnd);
        }

        while (nTmp && nTmp <= nTmpEnd)
        {
            // Iterate over the Range and set the Items
            sal_uInt16 nWh = pPool->GetWhich(nTmp);
            const SfxPoolItem* pItem;
            if (SfxItemState::SET == m_pSet->GetItemState(nWh, false, &pItem))
            {
                m_pExampleSet->Put(*pItem);
                m_pOutSet->Put(*pItem);
            }
            else
            {
                m_pExampleSet->ClearItem(nWh);
                m_pOutSet->ClearItem(nWh);
            }
            nTmp++;
        }
        // Go to the next pair
        pTmpRanges += 2;
    }
}


IMPL_LINK_NOARG(SfxTabDialog, BaseFmtHdl, Button*, void)

/*  [Description]

    Handler behind the Standard-Button.
    This button is available when editing style sheets. All the set attributes
    in the edited stylesheet are deleted.
*/

{
    m_bStandardPushed = true;

    const sal_uInt16 nId = m_pTabCtrl->GetCurPageId();
    Data_Impl* pDataObject = Find( m_pImpl->aData, nId );
    DBG_ASSERT( pDataObject, "Id not known" );

    if ( !pDataObject->fnGetRanges )
        return;

    if ( !m_pExampleSet )
        m_pExampleSet = new SfxItemSet( *m_pSet );

    const SfxItemPool* pPool = m_pSet->GetPool();
    const sal_uInt16* pTmpRanges = (pDataObject->fnGetRanges)();
    SfxItemSet aTmpSet( *m_pExampleSet );

    while ( *pTmpRanges )
    {
        const sal_uInt16* pU = pTmpRanges + 1;

        // Correct Range with multiple values
        sal_uInt16 nTmp = *pTmpRanges, nTmpEnd = *pU;
        DBG_ASSERT( nTmp <= nTmpEnd, "Range is sorted the wrong way" );

        if ( nTmp > nTmpEnd )
        {
            // If really sorted wrongly, then set new
            std::swap(nTmp, nTmpEnd);
        }

        while ( nTmp && nTmp <= nTmpEnd ) // guard against overflow
        {
            // Iterate over the Range and set the Items
            sal_uInt16 nWh = pPool->GetWhich( nTmp );
            m_pExampleSet->ClearItem( nWh );
            aTmpSet.ClearItem( nWh );
            // At the Outset of InvalidateItem,
            // so that the change takes effect
            m_pOutSet->InvalidateItem( nWh );
            nTmp++;
        }
        // Go to the next pair
        pTmpRanges += 2;
    }
    // Set all Items as new  -> the call the current Page Reset()
    DBG_ASSERT( pDataObject->pTabPage, "the Page is gone" );
    pDataObject->pTabPage->Reset( &aTmpSet );
    pDataObject->pTabPage->pImpl->mbStandard = true;
}


IMPL_LINK( SfxTabDialog, ActivatePageHdl, TabControl *, pTabCtrl, void )

/*  [Description]

    Handler that is called by StarView for switching to a different page.
    If the page not exist yet then it is created and the virtual Method
    <SfxTabDialog::PageCreated( sal_uInt16, SfxTabPage &)> is called. If the page
    exist, then the if possible the <SfxTabPage::Reset(const SfxItemSet &)> or
    <SfxTabPage::ActivatePage(const SfxItemSet &)> is called.
*/

{
    sal_uInt16 nId = pTabCtrl->GetCurPageId();

    DBG_ASSERT( m_pImpl->aData.size(), "no Pages registered" );

    // Tab Page already there?
    VclPtr<SfxTabPage> pTabPage = dynamic_cast<SfxTabPage*> (pTabCtrl->GetTabPage( nId ));
    Data_Impl* pDataObject = Find( m_pImpl->aData, nId );

    // fallback to 1st page when requested one does not exist
    if(!pDataObject && pTabCtrl->GetPageCount())
    {
        pTabCtrl->SetCurPageId(pTabCtrl->GetPageId(0));
        nId = pTabCtrl->GetCurPageId();
        pTabPage = dynamic_cast< SfxTabPage* >(pTabCtrl->GetTabPage(nId));
        pDataObject = Find(m_pImpl->aData, nId);
    }

    if (!pDataObject)
    {
        SAL_WARN("sfx.dialog", "Tab Page ID not known, this is pretty serious and needs investigation");
        return;
    }

    // Create TabPage if possible:
    if ( !pTabPage )
    {
        if ( m_pSet )
            pTabPage = (pDataObject->fnCreatePage)(static_cast<vcl::Window*>(pTabCtrl), m_pSet.get());
        else
            pTabPage = (pDataObject->fnCreatePage)(pTabCtrl, CreateInputItemSet(nId));
        DBG_ASSERT( nullptr == pDataObject->pTabPage, "create TabPage more than once" );
        pDataObject->pTabPage = pTabPage;
        pTabPage->SetTabDialog(this);

        OUString sConfigId = OStringToOUString(pTabPage->GetConfigId(), RTL_TEXTENCODING_UTF8);
        if (sConfigId.isEmpty())
        {
            SAL_WARN("sfx.dialog", "Tabpage needs to be converted to .ui format");
            sConfigId = OUString::number(pDataObject->nId);
        }
        SvtViewOptions aPageOpt(EViewType::TabPage, sConfigId);
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

        pTabPage->Reset( m_pSet.get() );

        pTabCtrl->SetTabPage( nId, pTabPage );
    }
    else if ( pDataObject->bRefresh )
        pTabPage->Reset( m_pSet.get() );
    pDataObject->bRefresh = false;

    if ( m_pExampleSet )
        pTabPage->ActivatePage( *m_pExampleSet );

    if ( pTabPage->IsReadOnly() || m_pImpl->bHideResetBtn )
        m_pResetBtn->Hide();
    else
        m_pResetBtn->Show();
}


IMPL_LINK( SfxTabDialog, DeactivatePageHdl, TabControl *, pTabCtrl, bool )

/*  [Description]

    Handler that is called by StarView before leaving a page.

    [Cross-reference]

    <SfxTabPage::DeactivatePage(SfxItemSet *)>
*/

{
    sal_uInt16 nId = pTabCtrl->GetCurPageId();
    SfxTabPage *pPage = dynamic_cast<SfxTabPage*> (pTabCtrl->GetTabPage( nId ));
    DBG_ASSERT( pPage, "no active Page" );
    if (!pPage)
        return false;
#ifdef DBG_UTIL
    Data_Impl* pDataObject = Find( m_pImpl->aData, pTabCtrl->GetCurPageId() );
    DBG_ASSERT( pDataObject, "no Data structure for current page" );
#endif

    DeactivateRC nRet = DeactivateRC::LeavePage;

    if ( !m_pExampleSet && pPage->HasExchangeSupport() && m_pSet )
        m_pExampleSet = new SfxItemSet( *m_pSet->GetPool(), m_pSet->GetRanges() );

    if ( m_pSet )
    {
        SfxItemSet aTmp( *m_pSet->GetPool(), m_pSet->GetRanges() );

        if ( pPage->HasExchangeSupport() )
            nRet = pPage->DeactivatePage( &aTmp );
        else
            nRet = pPage->DeactivatePage( nullptr );
        if ( ( DeactivateRC::LeavePage & nRet ) == DeactivateRC::LeavePage &&
             aTmp.Count() && m_pExampleSet)
        {
            m_pExampleSet->Put( aTmp );
            m_pOutSet->Put( aTmp );
        }
    }
    else
    {
        if ( pPage->HasExchangeSupport() ) //!!!
        {
            if ( !m_pExampleSet )
            {
                SfxItemPool* pPool = pPage->GetItemSet().GetPool();
                m_pExampleSet =
                    new SfxItemSet( *pPool, GetInputRanges( *pPool ) );
            }
            nRet = pPage->DeactivatePage( m_pExampleSet );
        }
        else
            nRet = pPage->DeactivatePage( nullptr );
    }

    if ( nRet & DeactivateRC::RefreshSet )
    {
        // Flag all Pages as to be initialized as new

        for (auto const& elem : m_pImpl->aData)
        {
            elem->bRefresh = ( elem->pTabPage.get() != pPage ); // Do not refresh own Page anymore
        }
    }
    return static_cast<bool>(nRet & DeactivateRC::LeavePage);
}


void SfxTabDialog::ShowPage( sal_uInt16 nId )

/*  [Description]

    The TabPage is activated with the specified Id.
*/

{
    m_pTabCtrl->SetCurPageId( nId );
    ActivatePageHdl( m_pTabCtrl );
}

OString SfxTabDialog::GetScreenshotId() const
{
    SfxTabPage *pActiveTabPage = GetCurTabPage();
    OString aScreenshotId = GetHelpId();

    if ( pActiveTabPage )
    {
        vcl::Window* pToplevelBox = pActiveTabPage->GetWindow( GetWindowType::FirstChild );

        if ( pToplevelBox )
            aScreenshotId = pToplevelBox->GetHelpId();
    }

    return aScreenshotId;
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
    if ( m_pSet )
    {
        SAL_WARN( "sfx.dialog", "Set already exists!" );
        return m_pSet->GetRanges();
    }

    if ( m_pRanges )
        return m_pRanges.get();
    std::vector<sal_uInt16> aUS;

    for (auto const& elem : m_pImpl->aData)
    {

        if ( elem->fnGetRanges )
        {
            const sal_uInt16* pTmpRanges = (elem->fnGetRanges)();
            const sal_uInt16* pIter = pTmpRanges;

            sal_uInt16 nLen;
            for( nLen = 0; *pIter; ++nLen, ++pIter )
                ;
            aUS.insert( aUS.end(), pTmpRanges, pTmpRanges + nLen );
        }
    }

    //! Remove duplicated Ids?
    {
        for (auto & elem : aUS)
            elem = rPool.GetWhich(elem);
    }

    // sort
    if ( aUS.size() > 1 )
    {
        std::sort( aUS.begin(), aUS.end() );
    }

    m_pRanges.reset(new sal_uInt16[aUS.size() + 1]);
    std::copy( aUS.begin(), aUS.end(), m_pRanges.get() );
    m_pRanges[aUS.size()] = 0;
    return m_pRanges.get();
}

void SfxTabDialog::SetPageName(sal_uInt16 nPageId, const OString& rName) const
{
    m_pTabCtrl->SetPageName(nPageId, rName);
}

void SfxTabDialog::SetInputSet( const SfxItemSet* pInSet )

/*  [Description]

    With this method the Input-Set can subsequently be set initially or re-set.
*/

{
    bool bSet = ( m_pSet != nullptr );
    m_pSet.reset(pInSet ? new SfxItemSet(*pInSet) : nullptr);

    if (!bSet && !m_pExampleSet && !m_pOutSet && m_pSet)
    {
        m_pExampleSet = new SfxItemSet( *m_pSet );
        m_pOutSet.reset(new SfxItemSet( *m_pSet->GetPool(), m_pSet->GetRanges() ));
    }
}

FactoryFunction SfxTabDialog::GetUITestFactory() const
{
    return SfxTabDialogUIObject::create;
}

std::vector<OString> SfxTabDialog::getAllPageUIXMLDescriptions() const
{
    std::vector<OString> aRetval;

    for (auto const& elem : m_pImpl->aData)
    {
        SfxTabPage* pCandidate = GetTabPage(elem->nId);

        if (!pCandidate)
        {
            // force SfxTabPage creation
            const_cast<SfxTabDialog*>(this)->ShowPage(elem->nId);
            pCandidate = GetTabPage(elem->nId);
        }

        if (pCandidate)
        {
            // use UIXMLDescription (without '.ui', with '/')
            aRetval.push_back(pCandidate->getUIFile());
        }
    }

    return aRetval;
}

bool SfxTabDialog::selectPageByUIXMLDescription(const OString& rUIXMLDescription)
{
    for (auto const& elem : m_pImpl->aData)
    {
        SfxTabPage* pCandidate = elem->pTabPage;

        if (!pCandidate)
        {
            // force SfxTabPage creation
            ShowPage(elem->nId);
            pCandidate = GetTabPage(elem->nId);
        }

        if (pCandidate && pCandidate->getUIFile() == rUIXMLDescription)
        {
            ShowPage(elem->nId);
            return true;
        }
    }

    return false;
}

SfxTabDialogController::SfxTabDialogController
(
    weld::Window* pParent,              // Parent Window
    const OUString& rUIXMLDescription, const OString& rID, // Dialog .ui path, Dialog Name
    const SfxItemSet* pItemSet,   // Itemset with the data;
                                  // can be NULL, when Pages are onDemand
    bool bEditFmt                 // when yes -> additional Button for standard
)
    : SfxOkDialogController(pParent, rUIXMLDescription, rID)
    , m_xTabCtrl(m_xBuilder->weld_notebook("tabcontrol"))
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
    , m_xApplyBtn(m_xBuilder->weld_button("apply"))
    , m_xUserBtn(m_xBuilder->weld_button("user"))
    , m_xCancelBtn(m_xBuilder->weld_button("cancel"))
    , m_xResetBtn(m_xBuilder->weld_button("reset"))
    , m_xBaseFmtBtn(m_xBuilder->weld_button("standard"))
    , m_pSet(pItemSet ? new SfxItemSet(*pItemSet) : nullptr)
    , m_bStandardPushed(false)
{
    m_pImpl.reset(new TabDlg_Impl(m_xTabCtrl->get_n_pages()));
    m_pImpl->bHideResetBtn = !m_xResetBtn->get_visible();
    m_xOKBtn->connect_clicked(LINK(this, SfxTabDialogController, OkHdl));
    m_xCancelBtn->connect_clicked(LINK(this, SfxTabDialogController, CancelHdl));
    m_xResetBtn->connect_clicked(LINK(this, SfxTabDialogController, ResetHdl));
    m_xResetBtn->set_label(SfxResId(STR_RESET));
    m_xTabCtrl->connect_enter_page(LINK(this, SfxTabDialogController, ActivatePageHdl));
    m_xTabCtrl->connect_leave_page(LINK(this, SfxTabDialogController, DeactivatePageHdl));
    m_xResetBtn->set_help_id(HID_TABDLG_RESET_BTN);

    if (bEditFmt)
    {
        m_xBaseFmtBtn->set_label(SfxResId(STR_STANDARD_SHORTCUT));
        m_xBaseFmtBtn->connect_clicked(LINK(this, SfxTabDialogController, BaseFmtHdl));
        m_xBaseFmtBtn->set_help_id(HID_TABDLG_STANDARD_BTN);
        m_xBaseFmtBtn->show();
    }

    if (m_xUserBtn)
        m_xUserBtn->connect_clicked(LINK(this, SfxTabDialogController, UserHdl));

    if (m_pSet)
    {
        m_xExampleSet.reset(new SfxItemSet(*m_pSet));
        m_pOutSet.reset(new SfxItemSet(*m_pSet->GetPool(), m_pSet->GetRanges()));
    }
}

IMPL_LINK_NOARG(SfxTabDialogController, OkHdl, weld::Button&, void)

/*  [Description]

    Handler of the Ok-Buttons
    This calls the current page <SfxTabPage::DeactivatePage(SfxItemSet *)>.
    Returns <DeactivateRC::LeavePage>, <SfxTabDialog::Ok()> is called
    and the Dialog is ended.
*/

{
    if (PrepareLeaveCurrentPage())
        m_xDialog->response(Ok());
}

IMPL_LINK_NOARG(SfxTabDialogController, UserHdl, weld::Button&, void)

/*  [Description]

    Handler of the User-Buttons
    This calls the current page <SfxTabPage::DeactivatePage(SfxItemSet *)>.
    returns this <DeactivateRC::LeavePage> and  <SfxTabDialog::Ok()> is called.
    Then the Dialog is ended with the Return value <SfxTabDialog::Ok()>
*/

{
    if (PrepareLeaveCurrentPage())
    {
        short nRet = Ok();
        if (RET_OK == nRet)
            nRet = RET_USER;
        else
            nRet = RET_USER_CANCEL;
        m_xDialog->response(nRet);
    }
}

IMPL_LINK_NOARG(SfxTabDialogController, CancelHdl, weld::Button&, void)
{
    m_xDialog->response(RET_USER_CANCEL);
}

IMPL_LINK_NOARG(SfxTabDialogController, ResetHdl, weld::Button&, void)

/*  [Description]

    Handler behind the reset button.
    The Current Page is new initialized with their initial data, all the
    settings that the user has made on this page are repealed.
*/

{
    Data_Impl* pDataObject = Find(m_pImpl->aData, m_xTabCtrl->get_current_page_ident());
    assert(pDataObject && "Id not known");

    pDataObject->pTabPage->Reset(m_pSet.get());
    // Also reset relevant items of ExampleSet and OutSet to initial state
    if (!pDataObject->fnGetRanges)
        return;

    if (!m_xExampleSet)
        m_xExampleSet.reset(new SfxItemSet(*m_pSet));

    const SfxItemPool* pPool = m_pSet->GetPool();
    const sal_uInt16* pTmpRanges = (pDataObject->fnGetRanges)();

    while (*pTmpRanges)
    {
        const sal_uInt16* pU = pTmpRanges + 1;

        // Correct Range with multiple values
        sal_uInt16 nTmp = *pTmpRanges, nTmpEnd = *pU;
        DBG_ASSERT(nTmp <= nTmpEnd, "Range is sorted the wrong way");

        if (nTmp > nTmpEnd)
        {
            // If really sorted wrongly, then set new
            std::swap(nTmp, nTmpEnd);
        }

        while (nTmp && nTmp <= nTmpEnd)
        {
            // Iterate over the Range and set the Items
            sal_uInt16 nWh = pPool->GetWhich(nTmp);
            const SfxPoolItem* pItem;
            if (SfxItemState::SET == m_pSet->GetItemState(nWh, false, &pItem))
            {
                m_xExampleSet->Put(*pItem);
                m_pOutSet->Put(*pItem);
            }
            else
            {
                m_xExampleSet->ClearItem(nWh);
                m_pOutSet->ClearItem(nWh);
            }
            nTmp++;
        }
        // Go to the next pair
        pTmpRanges += 2;
    }
}

/*  [Description]

    Handler behind the Standard-Button.
    This button is available when editing style sheets. All the set attributes
    in the edited stylesheet are deleted.
*/
IMPL_LINK_NOARG(SfxTabDialogController, BaseFmtHdl, weld::Button&, void)
{
    m_bStandardPushed = true;

    Data_Impl* pDataObject = Find(m_pImpl->aData, m_xTabCtrl->get_current_page_ident());
    assert(pDataObject && "Id not known");

    if (!pDataObject->fnGetRanges)
        return;

    if (!m_xExampleSet)
        m_xExampleSet.reset(new SfxItemSet(*m_pSet));

    const SfxItemPool* pPool = m_pSet->GetPool();
    const sal_uInt16* pTmpRanges = (pDataObject->fnGetRanges)();
    SfxItemSet aTmpSet(*m_xExampleSet);

    while (*pTmpRanges)
    {
        const sal_uInt16* pU = pTmpRanges + 1;

        // Correct Range with multiple values
        sal_uInt16 nTmp = *pTmpRanges, nTmpEnd = *pU;
        DBG_ASSERT( nTmp <= nTmpEnd, "Range is sorted the wrong way" );

        if ( nTmp > nTmpEnd )
        {
            // If really sorted wrongly, then set new
            std::swap(nTmp, nTmpEnd);
        }

        while ( nTmp && nTmp <= nTmpEnd ) // guard against overflow
        {
            // Iterate over the Range and set the Items
            sal_uInt16 nWh = pPool->GetWhich(nTmp);
            m_xExampleSet->ClearItem(nWh);
            aTmpSet.ClearItem(nWh);
            // At the Outset of InvalidateItem,
            // so that the change takes effect
            m_pOutSet->InvalidateItem(nWh);
            nTmp++;
        }
        // Go to the next pair
        pTmpRanges += 2;
    }
    // Set all Items as new  -> the call the current Page Reset()
    assert(pDataObject->pTabPage && "the Page is gone");
    pDataObject->pTabPage->Reset( &aTmpSet );
    pDataObject->pTabPage->pImpl->mbStandard = true;
}

IMPL_LINK(SfxTabDialogController, ActivatePageHdl, const OString&, rPage, void)

/*  [Description]

    Handler that is called by StarView for switching to a different page.
    If possible the <SfxTabPage::Reset(const SfxItemSet &)> or
    <SfxTabPage::ActivatePage(const SfxItemSet &)> is called on the new page
*/

{
    assert(!m_pImpl->aData.empty() && "no Pages registered");
    Data_Impl* pDataObject = Find(m_pImpl->aData, rPage);
    if (!pDataObject)
    {
        SAL_WARN("sfx.dialog", "Tab Page ID not known, this is pretty serious and needs investigation");
        return;
    }

    VclPtr<SfxTabPage> pTabPage = pDataObject->pTabPage;
    if (!pTabPage)
        return;

    if (pDataObject->bRefresh)
        pTabPage->Reset(m_pSet.get());
    pDataObject->bRefresh = false;

    if (m_xExampleSet)
        pTabPage->ActivatePage(*m_xExampleSet);

    if (pTabPage->IsReadOnly() || m_pImpl->bHideResetBtn)
        m_xResetBtn->hide();
    else
        m_xResetBtn->show();
}

IMPL_LINK(SfxTabDialogController, DeactivatePageHdl, const OString&, rPage, bool)

/*  [Description]

    Handler that is called by StarView before leaving a page.

    [Cross-reference]

    <SfxTabPage::DeactivatePage(SfxItemSet *)>
*/

{
    assert(!m_pImpl->aData.empty() && "no Pages registered");
    Data_Impl* pDataObject = Find(m_pImpl->aData, rPage);
    if (!pDataObject)
    {
        SAL_WARN("sfx.dialog", "Tab Page ID not known, this is pretty serious and needs investigation");
        return false;
    }

    VclPtr<SfxTabPage> pPage = pDataObject->pTabPage;
    DBG_ASSERT( pPage, "no active Page" );
    if (!pPage)
        return false;

    DeactivateRC nRet = DeactivateRC::LeavePage;

    if (!m_xExampleSet && pPage->HasExchangeSupport() && m_pSet)
        m_xExampleSet.reset(new SfxItemSet(*m_pSet->GetPool(), m_pSet->GetRanges()));

    if (m_pSet)
    {
        SfxItemSet aTmp( *m_pSet->GetPool(), m_pSet->GetRanges() );

        if (pPage->HasExchangeSupport())
            nRet = pPage->DeactivatePage(&aTmp);
        else
            nRet = pPage->DeactivatePage(nullptr);
        if ( ( DeactivateRC::LeavePage & nRet ) == DeactivateRC::LeavePage &&
             aTmp.Count() && m_xExampleSet)
        {
            m_xExampleSet->Put( aTmp );
            m_pOutSet->Put( aTmp );
        }
    }
    else
    {
        if ( pPage->HasExchangeSupport() ) //!!!
        {
            if (!m_xExampleSet)
            {
                SfxItemPool* pPool = pPage->GetItemSet().GetPool();
                m_xExampleSet.reset(new SfxItemSet(*pPool, GetInputRanges(*pPool)));
            }
            nRet = pPage->DeactivatePage(m_xExampleSet.get());
        }
        else
            nRet = pPage->DeactivatePage( nullptr );
    }

    if ( nRet & DeactivateRC::RefreshSet )
    {
        RefreshInputSet();
        // Flag all Pages as to be initialized as new

        for (auto const& elem : m_pImpl->aData)
        {
            elem->bRefresh = ( elem->pTabPage.get() != pPage ); // Do not refresh own Page anymore
        }
    }
    return static_cast<bool>(nRet & DeactivateRC::LeavePage);
}

bool SfxTabDialogController::PrepareLeaveCurrentPage()
{
    const OString sId = m_xTabCtrl->get_current_page_ident();
    Data_Impl* pDataObject = Find(m_pImpl->aData, sId);
    DBG_ASSERT( pDataObject, "Id not known" );
    VclPtr<SfxTabPage> pPage = pDataObject ? pDataObject->pTabPage : nullptr;

    bool bEnd = !pPage;

    if ( pPage )
    {
        DeactivateRC nRet = DeactivateRC::LeavePage;
        if ( m_pSet )
        {
            SfxItemSet aTmp( *m_pSet->GetPool(), m_pSet->GetRanges() );

            if ( pPage->HasExchangeSupport() )
                nRet = pPage->DeactivatePage( &aTmp );
            else
                nRet = pPage->DeactivatePage( nullptr );

            if ( ( DeactivateRC::LeavePage & nRet ) == DeactivateRC::LeavePage
                 && aTmp.Count() )
            {
                m_xExampleSet->Put( aTmp );
                m_pOutSet->Put( aTmp );
            }
        }
        else
            nRet = pPage->DeactivatePage( nullptr );
        bEnd = nRet != DeactivateRC::KeepPage;
    }

    return bEnd;
}

const sal_uInt16* SfxTabDialogController::GetInputRanges(const SfxItemPool& rPool)

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
    if ( m_pSet )
    {
        SAL_WARN( "sfx.dialog", "Set already exists!" );
        return m_pSet->GetRanges();
    }

    if ( m_pRanges )
        return m_pRanges.get();
    std::vector<sal_uInt16> aUS;

    for (auto const& elem : m_pImpl->aData)
    {

        if ( elem->fnGetRanges )
        {
            const sal_uInt16* pTmpRanges = (elem->fnGetRanges)();
            const sal_uInt16* pIter = pTmpRanges;

            sal_uInt16 nLen;
            for( nLen = 0; *pIter; ++nLen, ++pIter )
                ;
            aUS.insert( aUS.end(), pTmpRanges, pTmpRanges + nLen );
        }
    }

    //! Remove duplicated Ids?
    {
        for (auto & elem : aUS)
            elem = rPool.GetWhich(elem);
    }

    // sort
    if ( aUS.size() > 1 )
    {
        std::sort( aUS.begin(), aUS.end() );
    }

    m_pRanges.reset(new sal_uInt16[aUS.size() + 1]);
    std::copy( aUS.begin(), aUS.end(), m_pRanges.get() );
    m_pRanges[aUS.size()] = 0;
    return m_pRanges.get();
}

SfxTabDialogController::~SfxTabDialogController()
{
    SavePosAndId();

    for (auto & elem : m_pImpl->aData)
    {
        if ( elem->pTabPage )
        {
            // save settings of all pages (user data)
            elem->pTabPage->FillUserData();
            OUString aPageData( elem->pTabPage->GetUserData() );
            if ( !aPageData.isEmpty() )
            {
                // save settings of all pages (user data)
                OUString sConfigId = OStringToOUString(elem->pTabPage->GetConfigId(),
                    RTL_TEXTENCODING_UTF8);
                SvtViewOptions aPageOpt(EViewType::TabPage, sConfigId);
                aPageOpt.SetUserItem( USERITEM_NAME, makeAny( aPageData ) );
            }

            elem->pTabPage.disposeAndClear();
        }
        delete elem;
        elem = nullptr;
    }
}

short SfxTabDialogController::Ok()

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

    if ( !m_pOutSet )
    {
        if ( m_xExampleSet )
            m_pOutSet.reset(new SfxItemSet( *m_xExampleSet ));
        else if ( m_pSet )
            m_pOutSet = m_pSet->Clone( false );  // without Items
    }
    bool bModified = false;

    for (auto const& elem : m_pImpl->aData)
    {
        SfxTabPage* pTabPage = elem->pTabPage;

        if ( pTabPage )
        {
            if ( m_pSet && !pTabPage->HasExchangeSupport() )
            {
                SfxItemSet aTmp( *m_pSet->GetPool(), m_pSet->GetRanges() );

                if ( pTabPage->FillItemSet( &aTmp ) )
                {
                    bModified = true;
                    if (m_xExampleSet)
                        m_xExampleSet->Put( aTmp );
                    m_pOutSet->Put( aTmp );
                }
            }
        }
    }

    if (m_pOutSet && m_pOutSet->Count() > 0)
        bModified = true;

    if (m_bStandardPushed)
        bModified = true;

    return bModified ? RET_OK : RET_CANCEL;
}

void SfxTabDialogController::RefreshInputSet()

/*  [Description]

    Default implementation of the virtual Method.
    This is called, when <SfxTabPage::DeactivatePage(SfxItemSet *)>
    returns <DeactivateRC::RefreshSet>.
*/

{
    SAL_INFO ( "sfx.dialog", "RefreshInputSet not implemented" );
}

void SfxTabDialogController::PageCreated

/*  [Description]

    Default implementation of the virtual method. This is called immediately
    after creating a page. Here the dialogue can call the TabPage Method
    directly.
*/

(
    const OString&, // Id of the created page
    SfxTabPage&     // Reference to the created page
)
{
}

void SfxTabDialogController::SavePosAndId()
{
    // save settings (screen position and current page)
    SvtViewOptions aDlgOpt(EViewType::TabDialog, OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
    aDlgOpt.SetPageID(m_xTabCtrl->get_current_page_ident());
}

/*
    Adds a page to the dialog. The Name must correspond to a entry in the
    TabControl in the dialog .ui
*/
void SfxTabDialogController::AddTabPage(const OString &rName /* Page ID */,
                                        CreateTabPage pCreateFunc  /* Pointer to the Factory Method */,
                                        GetTabPageRanges pRangesFunc /* Pointer to the Method for querying Ranges onDemand */)
{
    m_pImpl->aData.push_back(new Data_Impl(m_pImpl->aData.size(), rName, pCreateFunc, pRangesFunc));
}

void SfxTabDialogController::AddTabPage(const OString &rName /* Page ID */,
                                        sal_uInt16 nPageCreateId /* Identifier of the Factory Method to create the page */)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    CreateTabPage pCreateFunc = pFact->GetTabPageCreatorFunc(nPageCreateId);
    GetTabPageRanges pRangesFunc = pFact->GetTabPageRangesFunc(nPageCreateId);
    AddTabPage(rName, pCreateFunc, pRangesFunc);
}

/*  [Description]

    Add a page to the dialog. The Rider text is passed on, the page has no
    counterpart in the TabControl in the resource of the dialogue.
*/

void SfxTabDialogController::AddTabPage(const OString &rName, /* Page ID */
                                        const OUString& rRiderText,
                                        CreateTabPage pCreateFunc  /* Pointer to the Factory Method */)
{
    assert(!m_xTabCtrl->get_page(rName) && "Double Page-Ids in the Tabpage");
    m_xTabCtrl->append_page(rName, rRiderText);
    AddTabPage(rName, pCreateFunc, nullptr);
}

void SfxTabDialogController::AddTabPage(const OString &rName, const OUString& rRiderText,
                                        sal_uInt16 nPageCreateId /* Identifier of the Factory Method to create the page */)
{
    assert(!m_xTabCtrl->get_page(rName) && "Double Page-Ids in the Tabpage");
    m_xTabCtrl->append_page(rName, rRiderText);
    AddTabPage(rName, nPageCreateId);
}

void SfxTabDialogController::CreatePages()
{
    for (auto pDataObject : m_pImpl->aData)
    {
        if (pDataObject->pTabPage)
           continue;
        weld::Container* pPage = m_xTabCtrl->get_page(pDataObject->sId);
        // TODO eventually pass DialogController as distinct argument instead of bundling into TabPageParent
        pDataObject->pTabPage = (pDataObject->fnCreatePage)(TabPageParent(pPage, this), m_pSet.get());
        pDataObject->pTabPage->SetDialogController(this);

        OUString sConfigId = OStringToOUString(pDataObject->pTabPage->GetConfigId(), RTL_TEXTENCODING_UTF8);
        SvtViewOptions aPageOpt(EViewType::TabPage, sConfigId);
        OUString sUserData;
        Any aUserItem = aPageOpt.GetUserItem(USERITEM_NAME);
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            sUserData = aTemp;
        pDataObject->pTabPage->SetUserData(sUserData);

        PageCreated(pDataObject->sId, *pDataObject->pTabPage);
        pDataObject->pTabPage->Reset(m_pSet.get());
    }
}

void SfxTabDialogController::setPreviewsToSamePlace()
{
    //where tab pages have the same basic layout with a preview on the right,
    //get both of their non-preview areas to request the same size so that the
    //preview appears in the same place in each one so flipping between tabs
    //isn't distracting as it jumps around
    std::vector<std::unique_ptr<weld::Widget>> aGrids;
    for (auto pDataObject : m_pImpl->aData)
    {
        if (!pDataObject->pTabPage)
            continue;
        if (!pDataObject->pTabPage->m_xBuilder)
            continue;
        std::unique_ptr<weld::Widget> pGrid = pDataObject->pTabPage->m_xBuilder->weld_widget("maingrid");
        if (!pGrid)
            continue;
        aGrids.emplace_back(std::move(pGrid));
    }

    m_xSizeGroup.reset();

    if (aGrids.size() <= 1)
        return;

    m_xSizeGroup = m_xBuilder->create_size_group();
    for (auto& rGrid : aGrids)
        m_xSizeGroup->add_widget(rGrid.get());
}

void SfxTabDialogController::RemoveTabPage(const OString& rId)

/*  [Description]

    Delete the TabPage with ID nId
*/

{
    sal_uInt16 nPos = 0;
    m_xTabCtrl->remove_page(rId);
    Data_Impl* pDataObject = Find( m_pImpl->aData, rId, &nPos );

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
                SvtViewOptions aPageOpt(EViewType::TabPage, sConfigId);
                aPageOpt.SetUserItem( USERITEM_NAME, makeAny( aPageData ) );
            }

            pDataObject->pTabPage.disposeAndClear();
        }

        delete pDataObject;
        m_pImpl->aData.erase( m_pImpl->aData.begin() + nPos );
    }
    else
    {
        SAL_INFO( "sfx.dialog", "TabPage-Id not known" );
    }
}

void SfxTabDialogController::Start_Impl()
{
    CreatePages();

    setPreviewsToSamePlace();

    assert(m_pImpl->aData.size() == static_cast<size_t>(m_xTabCtrl->get_n_pages())
            && "not all pages registered");

    // load old settings, when exists, setting SetCurPageId will override the settings,
    // something that the sort dialog in calc depends on
    if (m_sAppPageId.isEmpty())
    {
        SvtViewOptions aDlgOpt(EViewType::TabDialog, OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
        if (aDlgOpt.Exists())
            m_xTabCtrl->set_current_page(aDlgOpt.GetPageID());
    }

    ActivatePageHdl(m_xTabCtrl->get_current_page_ident());
}

void SfxTabDialogController::SetCurPageId(const OString& rIdent)
{
    m_sAppPageId = rIdent;
    m_xTabCtrl->set_current_page(m_sAppPageId);
}

OString SfxTabDialogController::GetCurPageId() const
{
    return m_xTabCtrl->get_current_page_ident();
}

short SfxTabDialogController::run()
{
    Start_Impl();
    return SfxDialogController::run();
}

bool SfxTabDialogController::runAsync(const std::shared_ptr<SfxTabDialogController>& rController,
                                      const std::function<void(sal_Int32)>& rFunc)
{
    rController->Start_Impl();
    return weld::DialogController::runAsync(rController, rFunc);
}

void SfxTabDialogController::SetInputSet( const SfxItemSet* pInSet )

/*  [Description]

    With this method the Input-Set can subsequently be set initially or re-set.
*/

{
    bool bSet = ( m_pSet != nullptr );
    m_pSet.reset(pInSet ? new SfxItemSet(*pInSet) : nullptr);

    if (!bSet && !m_xExampleSet && !m_pOutSet && m_pSet)
    {
        m_xExampleSet.reset(new SfxItemSet(*m_pSet));
        m_pOutSet.reset(new SfxItemSet( *m_pSet->GetPool(), m_pSet->GetRanges() ));
    }
}

SfxItemSet* SfxTabDialogController::GetInputSetImpl()

/*  [Description]

    Derived classes may create new storage for the InputSet. This has to be
    released in the Destructor. To do this, this method must be called.
*/

{
    return m_pSet.get();
}

void SfxTabDialogController::RemoveResetButton()
{
    m_xResetBtn->hide();
    m_pImpl->bHideResetBtn = true;
}

void SfxTabDialogController::RemoveStandardButton()
{
    m_xBaseFmtBtn->hide();
    m_pImpl->bHideResetBtn = true;
}

SfxTabPage* SfxTabDialogController::GetTabPage(const OString& rPageId) const

/*  [Description]

    Return TabPage with the specified Id.
*/

{
    Data_Impl* pDataObject = Find(m_pImpl->aData, rPageId);
    if (pDataObject)
        return pDataObject->pTabPage;
    return nullptr;
}

void SfxTabDialogController::SetApplyHandler(const Link<weld::Button&, void>& _rHdl)
{
    DBG_ASSERT( m_xApplyBtn, "SfxTabDialog::GetApplyHandler: no apply button enabled!" );
    if (m_xApplyBtn)
        m_xApplyBtn->connect_clicked(_rHdl);
}

bool SfxTabDialogController::Apply()
{
    bool bApplied = false;
    if (PrepareLeaveCurrentPage())
    {
        bApplied = (Ok() == RET_OK);
        //let the pages update their saved values
        GetInputSetImpl()->Put(*GetOutputItemSet());
        for (auto pDataObject : m_pImpl->aData)
        {
            if (!pDataObject->pTabPage)
                continue;
            pDataObject->pTabPage->ChangesApplied();
        }
    }
    return bApplied;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
