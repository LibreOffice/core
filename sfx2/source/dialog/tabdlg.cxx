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


#include <stdlib.h>
#include <algorithm>
#include <string_view>

#include <sfx2/tabdlg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/viewsh.hxx>
#include <unotools/viewoptions.hxx>
#include <utility>
#include <vcl/virdev.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <comphelper/lok.hxx>

#include <sfx2/strings.hrc>
#include <helpids.h>

using namespace ::com::sun::star::uno;

constexpr OUString USERITEM_NAME = u"UserItem"_ustr;


struct TabPageImpl
{
    bool                        mbStandard;
    SfxOkDialogController*      mpSfxDialogController;
    css::uno::Reference< css::frame::XFrame > mxFrame;

    TabPageImpl() : mbStandard(false), mpSfxDialogController(nullptr) {}
};

namespace {

struct Data_Impl
{
    OUString sId;                 // The ID
    CreateTabPage fnCreatePage;   // Pointer to Factory
    GetTabPageRanges fnGetRanges; // Pointer to Ranges-Function
    std::unique_ptr<SfxTabPage> xTabPage;         // The TabPage itself
    bool bRefresh;                // Flag: Page must be re-initialized

    // Constructor
    Data_Impl( const OUString& rId, CreateTabPage fnPage,
               GetTabPageRanges fnRanges ) :

        sId         ( rId ),
        fnCreatePage( fnPage ),
        fnGetRanges ( fnRanges ),
        bRefresh    ( false )
    {
    }
};

}

SfxTabDialogItem::SfxTabDialogItem( const SfxTabDialogItem& rAttr, SfxItemPool* pItemPool )
    : SfxSetItem( rAttr, pItemPool )
{
}

SfxTabDialogItem::SfxTabDialogItem( sal_uInt16 nId, const SfxItemSet& rItemSet )
    : SfxSetItem( nId, rItemSet )
{
}

SfxTabDialogItem* SfxTabDialogItem::Clone(SfxItemPool* pToPool) const
{
    return new SfxTabDialogItem( *this, pToPool );
}

typedef std::vector<Data_Impl*> SfxTabDlgData_Impl;

struct TabDlg_Impl
{
    bool                bHideResetBtn : 1;
    bool                bStarted : 1;
    SfxTabDlgData_Impl  aData;

    explicit TabDlg_Impl(sal_uInt8 nCnt)
        : bHideResetBtn(false)
        , bStarted(false)
    {
        aData.reserve( nCnt );
    }
};

static Data_Impl* Find( const SfxTabDlgData_Impl& rArr, std::u16string_view rId, sal_uInt16* pPos = nullptr)
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
    if (mpImpl)
        mpImpl->mxFrame = xFrame;
}

css::uno::Reference< css::frame::XFrame > SfxTabPage::GetFrame() const
{
    if (mpImpl)
        return mpImpl->mxFrame;
    return css::uno::Reference< css::frame::XFrame >();
}

static bool isLOKMobilePhone()
{
    if (!comphelper::LibreOfficeKit::isActive())
        return false;
    const SfxViewShell* pCurrentShell = SfxViewShell::Current();
    return pCurrentShell && pCurrentShell->isLOKMobilePhone();
}

SfxTabPage::SfxTabPage(weld::Container* pPage, weld::DialogController* pController, const OUString& rUIXMLDescription, const OUString& rID, const SfxItemSet *rAttrSet)
    : BuilderPage(pPage, pController, rUIXMLDescription, rID, isLOKMobilePhone())
    , mpSet(rAttrSet)
    , mbHasExchangeSupport(false)
    , mpImpl(new TabPageImpl)
{
    mpImpl->mpSfxDialogController = dynamic_cast<SfxOkDialogController*>(m_pDialogController);
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
    mpImpl.reset();
    m_xBuilder.reset();
}

bool SfxTabPage::FillItemSet( SfxItemSet* )
{
    return false;
}

/*
Returns the visible strings of a dialog.

Supported items:
- label
- check button
- radio button
- toggle button
- link button
- button
*/
OUString SfxTabPage::GetAllStrings() { return OUString(); }

void SfxTabPage::Reset( const SfxItemSet* )
{
}

bool SfxTabPage::DeferResetToFirstActivation() { return false; }

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
    sal_uInt16 nWh = pPool->GetWhichIDFromSlotID( nSlot, bDeep );
    const SfxPoolItem* pItem = nullptr;
    rSet.GetItemState( nWh, true, &pItem );

    if ( !pItem && nWh != nSlot )
        pItem = &pPool->GetUserOrPoolDefaultItem( nWh );
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

    if (mpImpl->mbStandard && rOldSet.GetParent())
        pItem = GetItem( *rOldSet.GetParent(), nSlot );
    else if ( rSet.GetParent() &&
              SfxItemState::INVALID == rSet.GetItemState( nWh ) )
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

void SfxTabPage::SetDialogController(SfxOkDialogController* pDialog)
{
    mpImpl->mpSfxDialogController = pDialog;
    m_pDialogController = mpImpl->mpSfxDialogController;
}

SfxOkDialogController* SfxTabPage::GetDialogController() const
{
    return mpImpl->mpSfxDialogController;
}

OUString SfxTabPage::GetHelpId() const
{
    if (m_xContainer)
        return m_xContainer->get_help_id();
    return {};
}

weld::Window* SfxTabPage::GetFrameWeld() const
{
    if (m_pDialogController)
        return m_pDialogController->getDialog();
    return nullptr;
}

const SfxItemSet* SfxTabPage::GetDialogExampleSet() const
{
    if (mpImpl->mpSfxDialogController)
        return mpImpl->mpSfxDialogController->GetExampleSet();
    return nullptr;
}

SfxTabDialogController::SfxTabDialogController
(
    weld::Widget* pParent,              // Parent Window
    const OUString& rUIXMLDescription, const OUString& rID, // Dialog .ui path, Dialog Name
    const SfxItemSet* pItemSet,   // Itemset with the data;
                                  // can be NULL, when Pages are onDemand
    bool bEditFmt                 // when yes -> additional Button for standard
)
    : SfxOkDialogController(pParent, rUIXMLDescription, rID)
    , m_xTabCtrl(m_xBuilder->weld_notebook(u"tabcontrol"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xApplyBtn(m_xBuilder->weld_button(u"apply"_ustr))
    , m_xUserBtn(m_xBuilder->weld_button(u"user"_ustr))
    , m_xCancelBtn(m_xBuilder->weld_button(u"cancel"_ustr))
    , m_xResetBtn(m_xBuilder->weld_button(u"reset"_ustr))
    , m_xBaseFmtBtn(m_xBuilder->weld_button(u"standard"_ustr))
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

    // The reset functionality seems to be confusing to many; disable in LOK.
    if (comphelper::LibreOfficeKit::isActive())
        RemoveResetButton();
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
            nRet = RET_CANCEL;
        m_xDialog->response(nRet);
    }
}

IMPL_LINK_NOARG(SfxTabDialogController, CancelHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
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

    pDataObject->xTabPage->Reset(m_pSet.get());
    // Also reset relevant items of ExampleSet and OutSet to initial state
    if (!pDataObject->fnGetRanges)
        return;

    if (!m_xExampleSet)
        m_xExampleSet.reset(new SfxItemSet(*m_pSet));

    const SfxItemPool* pPool = m_pSet->GetPool();
    const WhichRangesContainer& pTmpRanges = (pDataObject->fnGetRanges)();

    for (const auto & rPair : pTmpRanges)
    {
        // Correct Range with multiple values
        sal_uInt16 nTmp = rPair.first, nTmpEnd = rPair.second;
        DBG_ASSERT(nTmp <= nTmpEnd, "Range is sorted the wrong way");

        if (nTmp > nTmpEnd)
        {
            // If really sorted wrongly, then set new
            std::swap(nTmp, nTmpEnd);
        }

        while (nTmp && nTmp <= nTmpEnd)
        {
            // Iterate over the Range and set the Items
            sal_uInt16 nWh = pPool->GetWhichIDFromSlotID(nTmp);
            const SfxPoolItem* pItem;
            if (SfxItemState::SET == m_pSet->GetItemState(nWh, false, &pItem))
            {
                m_xExampleSet->Put(*pItem);
            }
            else
            {
                m_xExampleSet->ClearItem(nWh);
            }
            m_pOutSet->ClearItem(nWh);
            nTmp++;
        }
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
    const WhichRangesContainer& pTmpRanges = (pDataObject->fnGetRanges)();
    SfxItemSet aTmpSet(*m_xExampleSet);

    for (const auto& rPair : pTmpRanges)
    {
        // Correct Range with multiple values
        sal_uInt16 nTmp = rPair.first, nTmpEnd = rPair.second;
        DBG_ASSERT( nTmp <= nTmpEnd, "Range is sorted the wrong way" );

        if ( nTmp > nTmpEnd )
        {
            // If really sorted wrongly, then set new
            std::swap(nTmp, nTmpEnd);
        }

        while ( nTmp && nTmp <= nTmpEnd ) // guard against overflow
        {
            // Iterate over the Range and set the Items
            sal_uInt16 nWh = pPool->GetWhichIDFromSlotID(nTmp);
            m_xExampleSet->ClearItem(nWh);
            aTmpSet.ClearItem(nWh);
            // At the Outset of InvalidateItem,
            // so that the change takes effect
            m_pOutSet->InvalidateItem(nWh);
            nTmp++;
        }
    }
    // Set all Items as new  -> the call the current Page Reset()
    assert(pDataObject->xTabPage && "the Page is gone");
    pDataObject->xTabPage->Reset( &aTmpSet );
    pDataObject->xTabPage->mpImpl->mbStandard = true;
}

IMPL_LINK(SfxTabDialogController, ActivatePageHdl, const OUString&, rPage, void)
{
    ActivatePage(rPage);
}

void SfxTabDialogController::ActivatePage(const OUString& rPage)
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
        SAL_WARN("sfx.dialog", "Tab Page ID '" << rPage << "' not known, this is pretty serious and needs investigation");
        return;
    }

    SfxTabPage* pTabPage = pDataObject->xTabPage.get();
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

IMPL_LINK(SfxTabDialogController, DeactivatePageHdl, const OUString&, rPage, bool)
{
    return DeactivatePage(rPage);
}

bool SfxTabDialogController::DeactivatePage(std::u16string_view aPage)
/*  [Description]

    Handler that is called by StarView before leaving a page.

    [Cross-reference]

    <SfxTabPage::DeactivatePage(SfxItemSet *)>
*/

{
    assert(!m_pImpl->aData.empty() && "no Pages registered");
    Data_Impl* pDataObject = Find(m_pImpl->aData, aPage);
    if (!pDataObject)
    {
        SAL_WARN("sfx.dialog", "Tab Page ID not known, this is pretty serious and needs investigation");
        return false;
    }

    SfxTabPage* pPage = pDataObject->xTabPage.get();
    if (!pPage)
        return true;

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
            elem->bRefresh = ( elem->xTabPage.get() != pPage ); // Do not refresh own Page anymore
        }
    }
    return static_cast<bool>(nRet & DeactivateRC::LeavePage);
}

bool SfxTabDialogController::PrepareLeaveCurrentPage()
{
    const OUString sId = m_xTabCtrl->get_current_page_ident();
    Data_Impl* pDataObject = Find(m_pImpl->aData, sId);
    DBG_ASSERT( pDataObject, "Id not known" );
    SfxTabPage* pPage = pDataObject ? pDataObject->xTabPage.get() : nullptr;

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

const WhichRangesContainer & SfxTabDialogController::GetInputRanges(const SfxItemPool& rPool)

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

    if ( !m_pRanges.empty() )
        return m_pRanges;
    SfxItemSet aUS(const_cast<SfxItemPool&>(rPool));

    for (auto const& elem : m_pImpl->aData)
    {

        if ( elem->fnGetRanges )
        {
            const WhichRangesContainer& pTmpRanges = (elem->fnGetRanges)();

            for (const auto & rPair : pTmpRanges)
            {
                sal_uInt16 nWidFrom = rPool.GetWhichIDFromSlotID(rPair.first);
                sal_uInt16 nWidTo = rPool.GetWhichIDFromSlotID(rPair.second);
                aUS.MergeRange(nWidFrom, nWidTo); // Keep it valid
            }
        }
    }

    m_pRanges = aUS.GetRanges();
    return m_pRanges;
}

SfxTabDialogController::~SfxTabDialogController()
{
    SavePosAndId();

    for (auto & elem : m_pImpl->aData)
    {
        if ( elem->xTabPage )
        {
            // save settings of all pages (user data)
            elem->xTabPage->FillUserData();
            OUString aPageData( elem->xTabPage->GetUserData() );
            if ( !aPageData.isEmpty() )
            {
                // save settings of all pages (user data)
                SvtViewOptions aPageOpt(EViewType::TabPage, elem->xTabPage->GetConfigId());
                aPageOpt.SetUserItem( USERITEM_NAME, Any( aPageData ) );
            }

            elem->xTabPage.reset();
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
        SfxTabPage* pTabPage = elem->xTabPage.get();

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
    const OUString&, // Id of the created page
    SfxTabPage&     // Reference to the created page
)
{
}

void SfxTabDialogController::SavePosAndId()
{
    // save settings (screen position and current page)
    SvtViewOptions aDlgOpt(EViewType::TabDialog, m_xDialog->get_help_id());
    aDlgOpt.SetPageID(m_xTabCtrl->get_current_page_ident());
}

/*
    Adds a page to the dialog. The Name must correspond to an entry in the
    TabControl in the dialog .ui
*/
void SfxTabDialogController::AddTabPage(const OUString &rName /* Page ID */,
                                        CreateTabPage pCreateFunc  /* Pointer to the Factory Method */,
                                        GetTabPageRanges pRangesFunc /* Pointer to the Method for querying Ranges onDemand */)
{
    m_pImpl->aData.push_back(new Data_Impl(rName, pCreateFunc, pRangesFunc));
}

void SfxTabDialogController::AddTabPage(const OUString &rName /* Page ID */,
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

void SfxTabDialogController::AddTabPage(const OUString &rName, /* Page ID */
                                        const OUString& rRiderText,
                                        CreateTabPage pCreateFunc  /* Pointer to the Factory Method */)
{
    assert(!m_xTabCtrl->get_page(rName) && "Double Page-Ids in the Tabpage");
    m_xTabCtrl->append_page(rName, rRiderText);
    AddTabPage(rName, pCreateFunc, nullptr);
}

void SfxTabDialogController::AddTabPage(const OUString &rName, const OUString& rRiderText,
                                        sal_uInt16 nPageCreateId /* Identifier of the Factory Method to create the page */)
{
    assert(!m_xTabCtrl->get_page(rName) && "Double Page-Ids in the Tabpage");
    m_xTabCtrl->append_page(rName, rRiderText);
    AddTabPage(rName, nPageCreateId);
}

/*  [Description]

    Default implementation of the virtual Method.
    This is called when pages create their sets onDemand.
*/
SfxItemSet* SfxTabDialogController::CreateInputItemSet(const OUString&)
{
    SAL_WARN( "sfx.dialog", "CreateInputItemSet not implemented" );
    m_xItemSet = std::make_unique<SfxAllItemSet>(SfxGetpApp()->GetPool());
    return m_xItemSet.get();
}

void SfxTabDialogController::CreatePages()
{
    for (auto pDataObject : m_pImpl->aData)
    {
        if (pDataObject->xTabPage)
           continue;
        weld::Container* pPage = m_xTabCtrl->get_page(pDataObject->sId);
        if (m_pSet)
            pDataObject->xTabPage = (pDataObject->fnCreatePage)(pPage, this, m_pSet.get());
        else
            pDataObject->xTabPage = (pDataObject->fnCreatePage)(pPage, this, CreateInputItemSet(pDataObject->sId));
        pDataObject->xTabPage->SetDialogController(this);
        SvtViewOptions aPageOpt(EViewType::TabPage, pDataObject->xTabPage->GetConfigId());
        OUString sUserData;
        Any aUserItem = aPageOpt.GetUserItem(USERITEM_NAME);
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            sUserData = aTemp;
        pDataObject->xTabPage->SetUserData(sUserData);

        PageCreated(pDataObject->sId, *pDataObject->xTabPage);
        if (pDataObject->xTabPage->DeferResetToFirstActivation())
            pDataObject->bRefresh = true; // Reset will be called in ActivatePageHdl
        else
            pDataObject->xTabPage->Reset(m_pSet.get());
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
        if (!pDataObject->xTabPage)
            continue;
        if (!pDataObject->xTabPage->m_xBuilder)
            continue;
        std::unique_ptr<weld::Widget> pGrid = pDataObject->xTabPage->m_xBuilder->weld_widget(u"maingrid"_ustr);
        if (!pGrid)
            continue;
        aGrids.emplace_back(std::move(pGrid));
    }

    m_xSizeGroup.reset();

    if (aGrids.size() <= 1)
        return;

    m_xSizeGroup = m_xBuilder->create_size_group();
    m_xSizeGroup->set_mode(VclSizeGroupMode::Both);
    for (auto& rGrid : aGrids)
        m_xSizeGroup->add_widget(rGrid.get());
}

void SfxTabDialogController::RemoveTabPage(const OUString& rId)

/*  [Description]

    Delete the TabPage with ID nId
*/

{
    sal_uInt16 nPos = 0;
    m_xTabCtrl->remove_page(rId);
    Data_Impl* pDataObject = Find( m_pImpl->aData, rId, &nPos );

    if ( pDataObject )
    {
        if ( pDataObject->xTabPage )
        {
            pDataObject->xTabPage->FillUserData();
            OUString aPageData( pDataObject->xTabPage->GetUserData() );
            if ( !aPageData.isEmpty() )
            {
                // save settings of this page (user data)
                SvtViewOptions aPageOpt(EViewType::TabPage, pDataObject->xTabPage->GetConfigId());
                aPageOpt.SetUserItem( USERITEM_NAME, Any( aPageData ) );
            }

            pDataObject->xTabPage.reset();
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
        SvtViewOptions aDlgOpt(EViewType::TabDialog, m_xDialog->get_help_id());
        if (aDlgOpt.Exists())
            m_xTabCtrl->set_current_page(aDlgOpt.GetPageID());
    }

    ActivatePage(m_xTabCtrl->get_current_page_ident());

    m_pImpl->bStarted = true;
}

void SfxTabDialogController::SetCurPageId(const OUString& rIdent)
{
    m_sAppPageId = rIdent;
    m_xTabCtrl->set_current_page(m_sAppPageId);
}

/*  [Description]

    The TabPage is activated with the specified Id.
*/
void SfxTabDialogController::ShowPage(const OUString& rIdent)
{
    SetCurPageId(rIdent);
    ActivatePage(rIdent);
}

OUString SfxTabDialogController::GetCurPageId() const
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
}

SfxTabPage* SfxTabDialogController::GetTabPage(std::u16string_view rPageId) const

/*  [Description]

    Return TabPage with the specified Id.
*/

{
    Data_Impl* pDataObject = Find(m_pImpl->aData, rPageId);
    if (pDataObject)
        return pDataObject->xTabPage.get();
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
            if (!pDataObject->xTabPage)
                continue;
            pDataObject->xTabPage->ChangesApplied();
        }
    }
    return bApplied;
}

std::vector<OUString> SfxTabDialogController::getAllPageUIXMLDescriptions() const
{
    int nPages = m_xTabCtrl->get_n_pages();
    std::vector<OUString> aRet;
    aRet.reserve(nPages);
    for (int i = 0; i < nPages; ++i)
        aRet.push_back(m_xTabCtrl->get_page_ident(i));
    return aRet;
}

bool SfxTabDialogController::selectPageByUIXMLDescription(const OUString& rUIXMLDescription)
{
    ShowPage(rUIXMLDescription);
    return m_xTabCtrl->get_current_page_ident() == rUIXMLDescription;
}

BitmapEx SfxTabDialogController::createScreenshot() const
{
    // if we haven't run Start_Impl yet, do so now to create the initial pages
    if (!m_pImpl->bStarted)
    {
        const_cast<SfxTabDialogController*>(this)->Start_Impl();
    }

    VclPtr<VirtualDevice> xDialogSurface(m_xDialog->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OUString SfxTabDialogController::GetScreenshotId() const
{
    const OUString sId = m_xTabCtrl->get_current_page_ident();
    Data_Impl* pDataObject = Find(m_pImpl->aData, sId);
    SfxTabPage* pPage = pDataObject ? pDataObject->xTabPage.get() : nullptr;
    if (pPage)
    {
        OUString sHelpId(pPage->GetHelpId());
        if (!sHelpId.isEmpty())
            return sHelpId;
    }
    return m_xDialog->get_help_id();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
