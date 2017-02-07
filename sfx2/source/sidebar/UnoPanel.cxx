/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sfx2/sidebar/UnoPanel.hxx>

#include <sfx2/sidebar/ResourceManager.hxx>
#include <sfx2/sidebar/SidebarController.hxx>

#include <sfx2/sidebar/PanelTitleBar.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/Deck.hxx>

#include <vcl/svapp.hxx>

using namespace css;
using namespace ::sfx2::sidebar;

SfxUnoPanel::SfxUnoPanel(const uno::Reference<frame::XFrame>& rFrame, const OUString& panelId, const OUString& deckId):
xFrame(rFrame),
mPanelId(panelId),
mDeckId(deckId),
mpDeck(),
mpPanel()
{
    SidebarController* pSidebarController = getSidebarController();

    pSidebarController->CreateDeck(mDeckId); // creates deck object is not already
    mpDeck = pSidebarController->GetResourceManager()->GetDeckDescriptor(mDeckId)->mpDeck;
    mpPanel = mpDeck->GetPanel(mPanelId);
}

SidebarController* SfxUnoPanel::getSidebarController()
{
    return SidebarController::GetSidebarControllerForFrame(xFrame);
}

OUString SAL_CALL SfxUnoPanel::getId()
{
    SolarMutexGuard aGuard;

    return mPanelId;
}

OUString SAL_CALL SfxUnoPanel::getTitle()
{
    SolarMutexGuard aGuard;

    VclPtr<PanelTitleBar> pTitleBar = mpPanel->GetTitleBar();
    if (pTitleBar)
        return pTitleBar->GetTitle();
    else
        return OUString();
}

void SAL_CALL SfxUnoPanel::setTitle( const OUString& newTitle )
{
    SolarMutexGuard aGuard;

    SidebarController* pSidebarController = getSidebarController();
    std::shared_ptr<PanelDescriptor> xPanelDescriptor = pSidebarController->GetResourceManager()->GetPanelDescriptor(mPanelId);

    if (xPanelDescriptor)
    {
        xPanelDescriptor->msTitle = newTitle;
        VclPtr<PanelTitleBar> pTitleBar = mpPanel->GetTitleBar();
        if (pTitleBar)
            pTitleBar->SetTitle(newTitle);
    }
}

sal_Bool SAL_CALL SfxUnoPanel::isExpanded()
{
    SolarMutexGuard aGuard;

    return mpPanel->IsExpanded();
}


void SAL_CALL SfxUnoPanel::expand( const sal_Bool bCollapseOther )
{

    SolarMutexGuard aGuard;

    mpPanel->SetExpanded(true);

    if (bCollapseOther)
    {
        SharedPanelContainer aPanels = mpDeck->GetPanels();
        for ( SharedPanelContainer::iterator iPanel(aPanels.begin()), iEnd(aPanels.end());
              iPanel!=iEnd; ++iPanel)
        {
            VclPtr<Panel> const & aPanel = *iPanel;

            if (! aPanel->HasIdPredicate(mPanelId))
                aPanel->SetExpanded(false);
        }
    }

    SidebarController* pSidebarController = getSidebarController();
    pSidebarController->NotifyResize();

}

void SAL_CALL SfxUnoPanel::collapse()
{
    SolarMutexGuard aGuard;

    mpPanel->SetExpanded(false);
    SidebarController* pSidebarController = getSidebarController();
    pSidebarController->NotifyResize();
}

uno::Reference<awt::XWindow> SAL_CALL SfxUnoPanel::getDialog()
{
    SolarMutexGuard aGuard;

    return mpPanel->GetElementWindow();
}


sal_Int32 SAL_CALL SfxUnoPanel::getOrderIndex()
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    sal_Int32 index = pSidebarController->GetResourceManager()->GetPanelDescriptor(mPanelId)->mnOrderIndex;
    return index;
}

void SAL_CALL SfxUnoPanel::setOrderIndex( const sal_Int32 newOrderIndex )
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    std::shared_ptr<PanelDescriptor> xPanelDescriptor = pSidebarController->GetResourceManager()->GetPanelDescriptor(mPanelId);

    if (xPanelDescriptor)
    {
        xPanelDescriptor->mnOrderIndex = newOrderIndex;
        // update the sidebar
        pSidebarController->NotifyResize();
    }
}

void SAL_CALL SfxUnoPanel::moveFirst()
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    ResourceManager::PanelContextDescriptorContainer aPanels = pSidebarController->GetMatchingPanels(mDeckId);

    sal_Int32 curOrderIndex = getOrderIndex();
    sal_Int32 minIndex = GetMinOrderIndex(aPanels);

    if (curOrderIndex != minIndex) // is current panel already in place ?
    {
        minIndex -= 1;
        std::shared_ptr<PanelDescriptor> xPanelDescriptor = pSidebarController->GetResourceManager()->GetPanelDescriptor(mPanelId);
        if (xPanelDescriptor)
        {
            xPanelDescriptor->mnOrderIndex = minIndex;
            // update the sidebar
            pSidebarController->NotifyResize();
        }
    }
}

void SAL_CALL SfxUnoPanel::moveLast()
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    ResourceManager::PanelContextDescriptorContainer aPanels = pSidebarController->GetMatchingPanels(mDeckId);

    sal_Int32 curOrderIndex = getOrderIndex();
    sal_Int32 maxIndex = GetMaxOrderIndex(aPanels);

    if (curOrderIndex != maxIndex) // is current panel already in place ?
    {
        maxIndex += 1;
        std::shared_ptr<PanelDescriptor> xPanelDescriptor = pSidebarController->GetResourceManager()->GetPanelDescriptor(mPanelId);
        if (xPanelDescriptor)
        {
            xPanelDescriptor->mnOrderIndex = maxIndex;
            // update the sidebar
            pSidebarController->NotifyResize();
        }
    }
}

void SAL_CALL SfxUnoPanel::moveUp()
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    // Search for previous panel OrderIndex
    ResourceManager::PanelContextDescriptorContainer aPanels = pSidebarController->GetMatchingPanels(mDeckId);

    sal_Int32 curOrderIndex = getOrderIndex();
    sal_Int32 previousIndex = GetMinOrderIndex(aPanels);

    ResourceManager::PanelContextDescriptorContainer::const_iterator iPanel;
    for (iPanel = aPanels.begin(); iPanel != aPanels.end(); ++iPanel)
    {
        sal_Int32 index = pSidebarController->GetResourceManager()->GetPanelDescriptor(iPanel->msId)->mnOrderIndex;
        if( index < curOrderIndex && index > previousIndex)
            previousIndex = index;
    }

    if (curOrderIndex != previousIndex) // is current panel already in place ?
    {
        previousIndex -= 1;
        std::shared_ptr<PanelDescriptor> xPanelDescriptor = pSidebarController->GetResourceManager()->GetPanelDescriptor(mPanelId);
        if (xPanelDescriptor)
        {
            xPanelDescriptor->mnOrderIndex = previousIndex;
            // update the sidebar
            pSidebarController->NotifyResize();
        }
    }
}

void SAL_CALL SfxUnoPanel::moveDown()
{
    SolarMutexGuard aGuard;
    SidebarController* pSidebarController = getSidebarController();

    ResourceManager::PanelContextDescriptorContainer aPanels = pSidebarController->GetMatchingPanels(mDeckId);

    // Search for next panel OrderIndex
    sal_Int32 curOrderIndex = getOrderIndex();
    sal_Int32 nextIndex = GetMaxOrderIndex(aPanels);

    ResourceManager::PanelContextDescriptorContainer::const_iterator iPanel;
    for (iPanel = aPanels.begin(); iPanel != aPanels.end(); ++iPanel)
    {
        sal_Int32 index = pSidebarController->GetResourceManager()->GetPanelDescriptor(iPanel->msId)->mnOrderIndex;
        if( index > curOrderIndex && index < nextIndex)
            nextIndex = index;
    }

    if (curOrderIndex != nextIndex) // is current panel already in place ?
    {
        nextIndex += 1;
        std::shared_ptr<PanelDescriptor> xPanelDescriptor = pSidebarController->GetResourceManager()->GetPanelDescriptor(mPanelId);
        if (xPanelDescriptor)
        {
            xPanelDescriptor->mnOrderIndex = nextIndex;
            // update the sidebar
            pSidebarController->NotifyResize();
        }
    }
}

sal_Int32 SfxUnoPanel::GetMinOrderIndex(ResourceManager::PanelContextDescriptorContainer aPanels)
{
    SidebarController* pSidebarController = getSidebarController();

    ResourceManager::PanelContextDescriptorContainer::iterator iPanel;

    iPanel = aPanels.begin();
    sal_Int32 minIndex = pSidebarController->GetResourceManager()->GetPanelDescriptor(iPanel->msId)->mnOrderIndex;

    for (iPanel = aPanels.begin(); iPanel != aPanels.end(); ++iPanel)
    {
        sal_Int32 index = pSidebarController->GetResourceManager()->GetPanelDescriptor(iPanel->msId)->mnOrderIndex;
        if(minIndex > index)
            minIndex = index;
    }
    return minIndex;
}

sal_Int32 SfxUnoPanel::GetMaxOrderIndex(ResourceManager::PanelContextDescriptorContainer aPanels)
{
    SidebarController* pSidebarController = getSidebarController();

    ResourceManager::PanelContextDescriptorContainer::iterator iPanel;

    iPanel = aPanels.begin();
    sal_Int32 maxIndex = pSidebarController->GetResourceManager()->GetPanelDescriptor(iPanel->msId)->mnOrderIndex;

    for (iPanel = aPanels.begin(); iPanel != aPanels.end(); ++iPanel)
    {
        sal_Int32 index = pSidebarController->GetResourceManager()->GetPanelDescriptor(iPanel->msId)->mnOrderIndex;
        if(maxIndex < index)
            maxIndex = index;
    }
    return maxIndex;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
