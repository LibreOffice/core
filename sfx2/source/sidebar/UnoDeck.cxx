/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
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
mDeckId(deckId)
{

}
SidebarController* SfxUnoDeck::getSidebarController()
{
    return SidebarController::GetSidebarControllerForFrame(xFrame);
}

OUString SAL_CALL SfxUnoDeck::getId()
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    return mDeckId;
}

OUString SAL_CALL  SfxUnoDeck::getTitle()
                                throw(uno::RuntimeException, std::exception)
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
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SidebarController* pSidebarController = getSidebarController();
    pSidebarController->CreateDeck(mDeckId);

    DeckDescriptor* pDeckDescriptor = pSidebarController->GetResourceManager()->GetDeckDescriptor(mDeckId);

    if (pDeckDescriptor)
    {
        Deck* pDeck = pDeckDescriptor->mpDeck;
        DeckTitleBar* pTitleBar = pDeck->GetTitleBar();
        pTitleBar->SetTitle(newTitle);

        pDeckDescriptor->msTitle = newTitle;
        pDeckDescriptor->msHelpText = newTitle;

        pSidebarController->notifyDeckTitle(mDeckId);
    }
}

sal_Bool SAL_CALL SfxUnoDeck::isActive()
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SidebarController* pSidebarController = getSidebarController();
    return pSidebarController->IsDeckVisible(mDeckId);
}


void SAL_CALL SfxUnoDeck::activate( const sal_Bool bActivate )
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SidebarController* pSidebarController = getSidebarController();

    if (bActivate)
        pSidebarController->SwitchToDeck(mDeckId);
    else
        pSidebarController->SwitchToDefaultDeck();

    // update the sidebar
    pSidebarController->NotifyResize();
}

uno::Reference<ui::XPanels> SAL_CALL SfxUnoDeck::getPanels()
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    uno::Reference<ui::XPanels> panels = new SfxUnoPanels(xFrame, mDeckId);
    return panels;
}

sal_Int32 SAL_CALL SfxUnoDeck::getOrderIndex()
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    sal_Int32 index = pSidebarController->GetResourceManager()->GetDeckDescriptor(mDeckId)->mnOrderIndex;
    return index;
}

void SAL_CALL SfxUnoDeck::setOrderIndex( const sal_Int32 newOrderIndex )
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    DeckDescriptor* pDeckDescriptor = pSidebarController->GetResourceManager()->GetDeckDescriptor(mDeckId);

    if (pDeckDescriptor)
    {
        pDeckDescriptor->mnOrderIndex = newOrderIndex;
        // update the sidebar
        pSidebarController->NotifyResize();
    }
}

void SAL_CALL SfxUnoDeck::moveFirst()
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    ResourceManager::DeckContextDescriptorContainer aDecks = pSidebarController->GetMatchingDecks();

    sal_Int32 minIndex = GetMinOrderIndex(aDecks);
    sal_Int32 curOrderIndex = getOrderIndex();

    if (curOrderIndex != minIndex) // is deck already in place ?
    {
        minIndex -= 1;
        DeckDescriptor* pDeckDescriptor = pSidebarController->GetResourceManager()->GetDeckDescriptor(mDeckId);
        if (pDeckDescriptor)
        {
            pDeckDescriptor->mnOrderIndex = minIndex;
            // update the sidebar
            pSidebarController->NotifyResize();
        }
    }
}

void SAL_CALL SfxUnoDeck::moveLast()
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    ResourceManager::DeckContextDescriptorContainer aDecks = pSidebarController->GetMatchingDecks();

    sal_Int32 maxIndex = GetMaxOrderIndex(aDecks);
    sal_Int32 curOrderIndex = getOrderIndex();

    if (curOrderIndex != maxIndex) // is deck already in place ?
    {
        maxIndex += 1;
        DeckDescriptor* pDeckDescriptor = pSidebarController->GetResourceManager()->GetDeckDescriptor(mDeckId);
        if (pDeckDescriptor)
        {
            pDeckDescriptor->mnOrderIndex = maxIndex;
            // update the sidebar
            pSidebarController->NotifyResize();
        }
    }
}

void SAL_CALL SfxUnoDeck::moveUp()
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    // Search for previous deck OrderIndex
    ResourceManager::DeckContextDescriptorContainer aDecks = pSidebarController->GetMatchingDecks();

    sal_Int32 curOrderIndex = getOrderIndex();
    sal_Int32 previousIndex = GetMinOrderIndex(aDecks);

    ResourceManager::DeckContextDescriptorContainer::const_iterator iDeck;
    for (iDeck = aDecks.begin(); iDeck != aDecks.end(); ++iDeck)
    {
        sal_Int32 index = pSidebarController->GetResourceManager()->GetDeckDescriptor(iDeck->msId)->mnOrderIndex;
        if( index < curOrderIndex && index > previousIndex)
            previousIndex = index;
    }

    if (curOrderIndex != previousIndex) // is deck already in place ?
    {
        previousIndex -= 1;
        DeckDescriptor* pDeckDescriptor = pSidebarController->GetResourceManager()->GetDeckDescriptor(mDeckId);
        if (pDeckDescriptor)
        {
            pDeckDescriptor->mnOrderIndex = previousIndex;
            // update the sidebar
            pSidebarController->NotifyResize();
        }
    }
}

void SAL_CALL SfxUnoDeck::moveDown()
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    ResourceManager::DeckContextDescriptorContainer aDecks = pSidebarController->GetMatchingDecks();

    // Search for next deck OrderIndex
    sal_Int32 curOrderIndex = getOrderIndex();
    sal_Int32 nextIndex = GetMaxOrderIndex(aDecks);


    ResourceManager::DeckContextDescriptorContainer::const_iterator iDeck;
    for (iDeck = aDecks.begin(); iDeck != aDecks.end(); ++iDeck)
    {
        sal_Int32 index = pSidebarController->GetResourceManager()->GetDeckDescriptor(iDeck->msId)->mnOrderIndex;
        if( index > curOrderIndex && index < nextIndex)
            nextIndex = index;
    }

    if (curOrderIndex != nextIndex) // is deck already in place ?
    {
        nextIndex += 1;
        DeckDescriptor* pDeckDescriptor = pSidebarController->GetResourceManager()->GetDeckDescriptor(mDeckId);
        if (pDeckDescriptor)
        {
            pDeckDescriptor->mnOrderIndex = nextIndex;
            // update the sidebar
            pSidebarController->NotifyResize();
        }
    }
}


sal_Int32 SfxUnoDeck::GetMinOrderIndex(ResourceManager::DeckContextDescriptorContainer aDecks)
{
    SidebarController* pSidebarController = getSidebarController();

    ResourceManager::DeckContextDescriptorContainer::const_iterator iDeck;

    iDeck = aDecks.begin();
    sal_Int32 minIndex = pSidebarController->GetResourceManager()->GetDeckDescriptor(iDeck->msId)->mnOrderIndex;

    for (iDeck = aDecks.begin(); iDeck != aDecks.end(); ++iDeck)
    {
        sal_Int32 index = pSidebarController->GetResourceManager()->GetDeckDescriptor(iDeck->msId)->mnOrderIndex;
        if(minIndex > index)
            minIndex = index;
    }
    return minIndex;
}

sal_Int32 SfxUnoDeck::GetMaxOrderIndex(ResourceManager::DeckContextDescriptorContainer aDecks)
{
    SidebarController* pSidebarController = getSidebarController();

    ResourceManager::DeckContextDescriptorContainer::const_iterator iDeck;

    iDeck = aDecks.begin();
    sal_Int32 maxIndex = pSidebarController->GetResourceManager()->GetDeckDescriptor(iDeck->msId)->mnOrderIndex;

    for (iDeck = aDecks.begin(); iDeck != aDecks.end(); ++iDeck)
    {
        sal_Int32 index = pSidebarController->GetResourceManager()->GetDeckDescriptor(iDeck->msId)->mnOrderIndex;
        if(maxIndex < index)
            maxIndex = index;
    }
    return maxIndex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
