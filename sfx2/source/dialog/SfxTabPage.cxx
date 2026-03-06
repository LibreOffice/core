/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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


#include <comphelper/lok.hxx>
#include <sfx2/SfxTabPage.hxx>
#include <sfx2/viewsh.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/Dialog.hxx>

void SfxTabPage::SetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    mxFrame = xFrame;
}

css::uno::Reference< css::frame::XFrame > SfxTabPage::GetFrame() const
{
    return mxFrame;
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
    , mbCancel(false)
    , mbStandard(false)
    , mpSfxDialogController(nullptr)
{
    mpSfxDialogController = dynamic_cast<SfxOkDialogController*>(m_pDialogController);
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

    if (mbStandard && rOldSet.GetParent())
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
    mpSfxDialogController = pDialog;
    m_pDialogController = mpSfxDialogController;
}

SfxOkDialogController* SfxTabPage::GetDialogController() const
{
    return mpSfxDialogController;
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
    if (mpSfxDialogController)
        return mpSfxDialogController->GetExampleSet();
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
