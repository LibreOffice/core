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

#include <sfx2/sidebar/UnoDecks.hxx>
#include <sfx2/sidebar/UnoDeck.hxx>

#include <sfx2/sidebar/ResourceManager.hxx>
#include <sfx2/sidebar/SidebarController.hxx>

#include <vcl/svapp.hxx>

using namespace css;
using namespace ::sfx2::sidebar;

SfxUnoDecks::SfxUnoDecks(const uno::Reference<frame::XFrame>& rFrame):
xFrame(rFrame)
{
}

SidebarController* SfxUnoDecks::getSidebarController()
{
    return SidebarController::GetSidebarControllerForFrame(xFrame);
}

// XNameAccess

uno::Any SAL_CALL SfxUnoDecks::getByName( const OUString& aName )
                                throw(container::NoSuchElementException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    uno::Any aRet;

    if (hasByName(aName))
    {
        uno::Reference<ui::XDeck> xDeck = new SfxUnoDeck(xFrame, aName);
        aRet <<= xDeck;
    }
    else
        throw container::NoSuchElementException();

    return aRet;
}


uno::Sequence< OUString > SAL_CALL SfxUnoDecks::getElementNames()
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SidebarController* pSidebarController = getSidebarController();

    ResourceManager::DeckContextDescriptorContainer aDecks;
    css::uno::Sequence< OUString > deckList(aDecks.size());

    if (pSidebarController)
    {
        pSidebarController->GetResourceManager()->GetMatchingDecks (
            aDecks,
            pSidebarController->GetCurrentContext(),
            pSidebarController->IsDocumentReadOnly(),
            xFrame);

        deckList.realloc(aDecks.size());

        long n = 0;

        for (ResourceManager::DeckContextDescriptorContainer::const_iterator
            iDeck(aDecks.begin()), iEnd(aDecks.end());
            iDeck!=iEnd; ++iDeck)
            {
                deckList[n] = iDeck->msId;
                n++;
            }
    }

    return deckList;

}

sal_Bool SAL_CALL SfxUnoDecks::hasByName( const OUString& aName )
                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SidebarController* pSidebarController = getSidebarController();

    sal_Bool bFound = false;

    if (pSidebarController)
    {
        ResourceManager::DeckContextDescriptorContainer aDecks;

        pSidebarController->GetResourceManager()->GetMatchingDecks (
            aDecks,
            pSidebarController->GetCurrentContext(),
            pSidebarController->IsDocumentReadOnly(),
            xFrame);

        for (ResourceManager::DeckContextDescriptorContainer::const_iterator
            iDeck(aDecks.begin()), iEnd(aDecks.end());
            iDeck!=iEnd && !bFound; ++iDeck)
            {
                if (iDeck->msId == aName)
                    bFound = true;
            }
    }

    return bFound;

}

// XIndexAccess

sal_Int32 SAL_CALL SfxUnoDecks::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > decks = getElementNames();
    return decks.getLength();
}

uno::Any SAL_CALL SfxUnoDecks::getByIndex( sal_Int32 Index )
                                throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

        return uno::Any();

}

// XElementAccess
uno::Type SAL_CALL SfxUnoDecks::getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    return uno::Type();
}

sal_Bool SAL_CALL SfxUnoDecks::hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > decks = getElementNames();
    return decks.hasElements();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */