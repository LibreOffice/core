/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/ui/xsidebarprovider.hxx>

#include <rtl/ustring.hxx>
#include "cppunit/extensions/HelperMacros.h"

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XController2.hpp>

#include <com/sun/star/ui/XSidebarProvider.hpp>
#include <com/sun/star/ui/XDecks.hpp>
#include <com/sun/star/ui/XDeck.hpp>
#include <com/sun/star/ui/XPanels.hpp>
#include <com/sun/star/ui/XPanel.hpp>

using namespace css;
using namespace css::uno;

namespace apitest {


void XSidebarProvider::testSidebar()
{

   // get document
   uno::Reference< sheet::XSpreadsheetDocument > xDoc(init(), UNO_QUERY_THROW);
   uno::Reference< frame::XModel > xModel(xDoc, UNO_QUERY_THROW);

   // get controller
   uno::Reference< frame::XController > xController = xModel->getCurrentController();
   uno::Reference< frame::XController2 > xController2(xController, UNO_QUERY_THROW);

   // sidebar visibility
    uno::Reference< ui::XSidebarProvider > xSidebar = xController2->getSidebar();
    CPPUNIT_ASSERT_MESSAGE("Sidebar not found", xSidebar.is());

   // show sidebar
    xSidebar->setVisible(true);
    CPPUNIT_ASSERT_MESSAGE("Sidebar should be visible", xSidebar->isVisible());

/* TODO
    xSidebar->setVisible(false);
    CPPUNIT_ASSERT_MESSAGE("Sidebar should not be visible", !xSidebar->isVisible());

    xSidebar->setVisible(true);
*/

    // expand the Decks area
    xSidebar->showDecks(false);
    xSidebar->showDecks(true);

    // get the Decks
    uno::Reference< ui::XDecks > xDecks = xSidebar->getDecks();

    uno::Reference< container::XIndexAccess > xDecksIndex (xDecks, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Decks Count" , 4, xDecksIndex->getCount());

    uno::Reference< container::XNameAccess > xDecksNames (xDecks, UNO_QUERY_THROW);

    OUString firstDeckName = "PropertyDeck";

    bool bDeckFound = xDecksNames->hasByName(firstDeckName);
    CPPUNIT_ASSERT_MESSAGE("PropertyDeck not found", bDeckFound);

    uno::Sequence< OUString > elementNames = xDecksNames->getElementNames();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Decks ElementNames count", 4, elementNames.getLength());

    uno::Reference< ui::XDeck > xDeck (xDecksNames->getByName(firstDeckName), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("PropertyDeck not available", xDeck.is());

    xDeck->activate(true);
    xDeck->activate(false);
    CPPUNIT_ASSERT_MESSAGE("Sidebar disappeared", xSidebar.is());

    xDeck->activate(true);

    OUString deckId = xDeck->getId();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong deck ID", firstDeckName, deckId);

    OUString newDeckTitle = "New title";
    xDeck->setTitle(newDeckTitle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong deck title", newDeckTitle, xDeck->getTitle());

    xDeck->moveFirst();
    long initialIndex = xDeck->getOrderIndex();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong deck initial index", 100l, initialIndex);

    xDeck->moveLast();
    CPPUNIT_ASSERT_MESSAGE("Deck not moved last", xDeck->getOrderIndex() > initialIndex);

    initialIndex = xDeck->getOrderIndex();
    xDeck->moveFirst();
    CPPUNIT_ASSERT_MESSAGE("Deck not moved first", xDeck->getOrderIndex() < initialIndex);

    initialIndex = xDeck->getOrderIndex();
    xDeck->moveDown();
    CPPUNIT_ASSERT_MESSAGE("Deck not moved down", xDeck->getOrderIndex() > initialIndex);

    initialIndex = xDeck->getOrderIndex();
    xDeck->moveUp();
    CPPUNIT_ASSERT_MESSAGE("Deck not moved up", xDeck->getOrderIndex() < initialIndex);

    // get panels

    uno::Reference< ui::XPanels > xPanels = xDeck->getPanels();

    uno::Reference< container::XNameAccess > xPanelsNames (xPanels, UNO_QUERY_THROW);

// !!!!!!! fails here

    uno::Sequence< OUString > panelElementNames = xPanelsNames->getElementNames();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Panels ElementNames count", 4, panelElementNames.getLength());

    OUString firstPanelName = "TextPropertyPanel";

    bool bPanelFound = xPanelsNames->hasByName(firstPanelName);
    CPPUNIT_ASSERT_MESSAGE("Panel not found", bPanelFound);

    uno::Reference< container::XIndexAccess > xPanelsIndex (xPanels, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Panels Count" , 4, xPanelsIndex->getCount());


    uno::Reference< ui::XPanel > xPanel (xPanelsNames->getByName(firstPanelName), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Panel not available", xPanel.is());

    OUString panelId = xPanel->getId();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong panel ID", firstPanelName, panelId);

    OUString newPanelTitle = "New title";
    xPanel->setTitle(newPanelTitle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong panel title", newPanelTitle, xPanel->getTitle());

    xPanel->moveFirst();
    initialIndex = xPanel->getOrderIndex();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Panel initial index", 100l, initialIndex);

    xPanel->moveLast();
    CPPUNIT_ASSERT_MESSAGE("Panel not moved last", xPanel->getOrderIndex() > initialIndex);

    initialIndex = xPanel->getOrderIndex();
    xPanel->moveFirst();
    CPPUNIT_ASSERT_MESSAGE("Panel not moved first", xPanel->getOrderIndex() < initialIndex);

    initialIndex = xPanel->getOrderIndex();
    xPanel->moveDown();
    CPPUNIT_ASSERT_MESSAGE("Panel not moved down", xPanel->getOrderIndex() > initialIndex);

    initialIndex = xPanel->getOrderIndex();
    xPanel->moveUp();
    CPPUNIT_ASSERT_MESSAGE("Panel not moved up", xPanel->getOrderIndex() < initialIndex);


/*    xDeck->activate(true);
    xDeck->activate(false);
    CPPUNIT_ASSERT_MESSAGE("Sidebar disappeared", xSidebar.is());
    xDeck->activate(true);

            panel.collapse()

        panel.expand(true) ' expand and collapse other
        isExpanded

*/


/*
 *      controller =  thisComponent.currentcontroller
        frameIni =  thisComponent.currentcontroller.frame
        sidebar =  thisComponent.currentcontroller.getSidebar()

        sidebar.setVisible(true)

        frame = sidebar.getFrame()

        decks = sidebar.getdecks()

        c = decks.count

        h = decks.hasElements()

        e = decks.getElementNames()

        a =  decks.hasByName("MyDeck")

        deck = decks.getByName("MyDeck")
        deck.activate(true)

        t = deck.getTitle()
        deck.setTitle("new deck title")

        deck.moveFirst()
        deck.moveLast()
        deck.moveUp()
        deck.moveDown()

        index = deck.getOrderIndex()

        panels = deck.getPanels()
        ep = panels.getElementnames()

        ap = panels.hasByName("aPanelName")

        panel = panels.getByName("aPanelName")
        panel.setTitle("new panel title")

        panel.collapse()

        panel.expand(true) ' expand and collapse other

        index = panel.getOrderIndex()

        panel.moveLast()
        panel.moveFirst()
        panel.moveDown()
        panel.moveUp()
*/


}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
