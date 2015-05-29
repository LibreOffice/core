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

#include <sfx2/sidebar/UnoDeck.hxx>

#include <sfx2/sidebar/UnoPanels.hxx>

#include <sfx2/sidebar/ResourceManager.hxx>
#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/DeckTitleBar.hxx>
#include <sfx2/sidebar/Deck.hxx>

#include <vcl/svapp.hxx>

using namespace css;
using namespace ::sfx2::sidebar;

SfxUnoDeck::SfxUnoDeck(const uno::Reference<frame::XFrame>& rFrame, const OUString& deckId):
xFrame(rFrame),
mDeckId(deckId),
mTitle(deckId)
{

}
SidebarController* SfxUnoDeck::getSidebarController()
{
    return SidebarController::GetSidebarControllerForFrame(xFrame);
}

OUString SAL_CALL SfxUnoDeck::getId()
{
    SolarMutexGuard aGuard;

    return mDeckId;
}

OUString SAL_CALL  SfxUnoDeck::getTitle()
{
    SolarMutexGuard aGuard;

    SidebarController* pSidebarController = getSidebarController();
    Deck* pDeck = pSidebarController->GetResourceManager()->GetDeckDescriptor(mDeckId)->mpDeck;


    if (!pDeck)
    {
        pSidebarController->CreateDeck(mDeckId);
        pDeck = pSidebarController->GetResourceManager()->GetDeckDescriptor(mDeckId)->mpDeck;
    }

    DeckTitleBar* pTitleBar = pDeck->GetTitleBar();
    return pTitleBar->GetTitle();
}

void SAL_CALL SfxUnoDeck::setTitle( const OUString& newTitle )
{
    SolarMutexGuard aGuard;

    SidebarController* pSidebarController = getSidebarController();
    pSidebarController->CreateDeck(mDeckId);

    Deck* pDeck = pSidebarController->GetResourceManager()->GetDeckDescriptor(mDeckId)->mpDeck;

    DeckTitleBar* pTitleBar = pDeck->GetTitleBar();
    pTitleBar->SetTitle(newTitle);
}

sal_Bool SAL_CALL SfxUnoDeck::isActive()
{
    SolarMutexGuard aGuard;

    SidebarController* pSidebarController = getSidebarController();
    return pSidebarController->IsDeckVisible(mDeckId);
}


void SAL_CALL SfxUnoDeck::activate( const sal_Bool bActivate )
{
    SolarMutexGuard aGuard;

    SidebarController* pSidebarController = getSidebarController();

    if (bActivate)
        pSidebarController->SwitchToDeck(mDeckId);
    else
        pSidebarController->SwitchToDefaultDeck();
}

uno::Reference<ui::XPanels> SAL_CALL SfxUnoDeck::getPanels()
{
    SolarMutexGuard aGuard;

    uno::Reference<ui::XPanels> panels = new SfxUnoPanels(xFrame, mDeckId);
    return panels;
}

sal_Int32 SAL_CALL SfxUnoDeck::getOrderIndex()
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    sal_Int32 index = pSidebarController->GetResourceManager()->GetDeckDescriptor(mDeckId)->mnOrderIndex;
    return index;
}

void SAL_CALL SfxUnoDeck::setOrderIndex( const sal_Int32 newOrderIndex )
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    pSidebarController->GetResourceManager()->SetDeckOrderIndex(mDeckId, newOrderIndex);

    pSidebarController->NotifyResize();
}

void SAL_CALL SfxUnoDeck::moveFirst()
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    sal_Int32 minIndex = pSidebarController->GetResourceManager()->GetDecksMinOrderIndex();
    minIndex -= 1;

    pSidebarController->GetResourceManager()->SetDeckOrderIndex(mDeckId, minIndex);

    pSidebarController->NotifyResize();
}

void SAL_CALL SfxUnoDeck::moveLast()
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    sal_Int32 maxIndex = pSidebarController->GetResourceManager()->GetDecksMaxOrderIndex();
    maxIndex += 1;

    pSidebarController->GetResourceManager()->SetDeckOrderIndex(mDeckId, maxIndex);

    pSidebarController->NotifyResize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */