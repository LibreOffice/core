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

OUString SAL_CALL  SfxUnoPanel::getTitle()
{
    SolarMutexGuard aGuard;

    PanelTitleBar* pTitleBar = mpPanel->GetTitleBar();
    return pTitleBar->GetTitle();
}

void SAL_CALL SfxUnoPanel::setTitle( const OUString& newTitle )
{
    SolarMutexGuard aGuard;

    PanelTitleBar* pTitleBar = mpPanel->GetTitleBar();
    pTitleBar->SetTitle(newTitle);
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
     Panel* aPanel;

     SharedPanelContainer maPanels = mpDeck->GetPanels();
     for ( SharedPanelContainer::iterator iPanel(maPanels.begin()), iEnd(maPanels.end());
        iPanel!=iEnd; ++iPanel)
        {
            aPanel = *iPanel;

            if (! aPanel->HasIdPredicate(mPanelId))
                aPanel->SetExpanded(false);
        }
    }

}
void SAL_CALL SfxUnoPanel::collapse()
{
    SolarMutexGuard aGuard;

    mpPanel->SetExpanded(false);
}

uno::Reference<awt::XWindow> SAL_CALL SfxUnoPanel::getDialog()
{
    SolarMutexGuard aGuard;

    return mpPanel->GetElementWindow();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */