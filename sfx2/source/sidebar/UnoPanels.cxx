/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sidebar/UnoPanels.hxx>

#include <sfx2/sidebar/ResourceManager.hxx>
#include <sfx2/sidebar/SidebarController.hxx>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/ui/XPanel.hpp>
#include <sidebar/UnoPanel.hxx>

#include <vcl/svapp.hxx>

#include <algorithm>

using namespace css;
using namespace ::sfx2::sidebar;

SfxUnoPanels::SfxUnoPanels(const uno::Reference<frame::XFrame>& rFrame, const OUString& deckId):
xFrame(rFrame),
mDeckId(deckId)
{
}

SidebarController* SfxUnoPanels::getSidebarController()
{
    return SidebarController::GetSidebarControllerForFrame(xFrame);
}

OUString SAL_CALL SfxUnoPanels::getDeckId()
{
    SolarMutexGuard aGuard;

    return mDeckId;
}

// XNameAccess

uno::Any SAL_CALL SfxUnoPanels::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    if (!hasByName(aName))
        throw container::NoSuchElementException();

    uno::Reference<ui::XPanel> xPanel = new SfxUnoPanel(xFrame, aName, mDeckId);
    return uno::Any(xPanel);
}


uno::Sequence< OUString > SAL_CALL SfxUnoPanels::getElementNames()
{

    SolarMutexGuard aGuard;

    SidebarController* pSidebarController = getSidebarController();

    ResourceManager::PanelContextDescriptorContainer aPanels;
    uno::Sequence< OUString > panelList(aPanels.size());

    if (pSidebarController)
    {
        pSidebarController->GetResourceManager()->GetMatchingPanels(aPanels,
                                                      pSidebarController->GetCurrentContext(),
                                                      mDeckId,
                                                      xFrame->getController());

        panelList.realloc(aPanels.size());
        std::transform(aPanels.begin(), aPanels.end(), panelList.getArray(),
                       [](const auto& rPanel) { return rPanel.msId; });
    }

    return panelList;

}

sal_Bool SAL_CALL SfxUnoPanels::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    SidebarController* pSidebarController = getSidebarController();

    if (pSidebarController)
    {
        ResourceManager::PanelContextDescriptorContainer aPanels;

        pSidebarController->GetResourceManager()->GetMatchingPanels(aPanels,
                                                      pSidebarController->GetCurrentContext(),
                                                      mDeckId,
                                                      xFrame->getController());

        bool bIsDocumentReadOnly = pSidebarController->IsDocumentReadOnly();

        return std::any_of(aPanels.begin(), aPanels.end(),
            [&bIsDocumentReadOnly, &aName](const ResourceManager::PanelContextDescriptor& rPanel) {
                return (!bIsDocumentReadOnly || rPanel.mbShowForReadOnlyDocuments) // Determine if the panel can be displayed.
                    && (rPanel.msId == aName);
            });
    }

    // nothing found
    return false;

}

// XIndexAccess

sal_Int32 SAL_CALL SfxUnoPanels::getCount()
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > panels = getElementNames();
    return panels.getLength();
}

uno::Any SAL_CALL SfxUnoPanels::getByIndex( sal_Int32 Index )
{
    SolarMutexGuard aGuard;

    uno::Any aRet;

    uno::Sequence< OUString > panels = getElementNames();

    if (Index > panels.getLength()-1 || Index < 0)
        throw lang::IndexOutOfBoundsException();

    uno::Reference<ui::XPanel> xPanel = new SfxUnoPanel(xFrame, panels[Index], mDeckId);
    aRet <<= xPanel;
    return aRet;

}

// XElementAccess
uno::Type SAL_CALL SfxUnoPanels::getElementType()
{
    SolarMutexGuard aGuard;

    return uno::Type();
}

sal_Bool SAL_CALL SfxUnoPanels::hasElements()
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > panels = getElementNames();
    return panels.hasElements();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
