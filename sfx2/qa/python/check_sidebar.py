# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest
import unohelper
import os
from org.libreoffice.unotest import UnoInProcess

from com.sun.star.ui import XSidebarProvider
from com.sun.star.ui import XDecks
from com.sun.star.ui import XDeck
from com.sun.star.ui import XPanels
from com.sun.star.ui import XPanel

class CheckSidebar(unittest.TestCase):
    _uno = None
    _xDoc = None

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xDoc = cls._uno.openEmptyDoc( url = "private:factory/scalc", bHidden = False, bReadOnly = False)

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown() 

    def test_check_sidebar(self):

        xDoc = self.__class__._xDoc
        xController = xDoc.getCurrentController()

        xSidebar = xController.getSidebar()
        assert(xSidebar)

        xSidebar.setVisible(True)
        isVisible = xSidebar.isVisible()
        self.assertTrue ( xSidebar.isVisible() )

        # TODO: does not work in unit test context 
#        xSidebar.setVisible(False)
#        isVisible = xSidebar.isVisible()
#        assert( not isVisible )
#        xSidebar.setVisible(True)

        xSidebar.showDecks(False)
        xSidebar.showDecks(True)

        xDecks = xSidebar.getDecks()

        firstDeckName = "PropertyDeck";

        deckElementNames = xDecks.getElementNames()
        assert ( firstDeckName in deckElementNames )
        assert ( xDecks.hasByName(firstDeckName) )

        decksCount = xDecks.getCount()
        self.assertEqual ( 5, decksCount )

        xDeck = xDecks.getByName(firstDeckName)
        assert ( xDeck )
        assert ( xDeck.getId() == firstDeckName )

        newDeckTitle = "New title"
        xDeck.setTitle(newDeckTitle)
        assert ( xDeck.getTitle() == newDeckTitle )

        xDeck.moveFirst()
        initialIndex = xDeck.getOrderIndex()
        self.assertEqual(100, initialIndex)

        xDeck.moveLast()
        assert ( xDeck.getOrderIndex() > initialIndex )

        initialIndex = xDeck.getOrderIndex()
        xDeck.moveFirst()
        assert ( xDeck.getOrderIndex() < initialIndex )

        initialIndex = xDeck.getOrderIndex()
        xDeck.moveDown()
        assert ( xDeck.getOrderIndex() > initialIndex )

        initialIndex = xDeck.getOrderIndex()
        xDeck.moveUp()
        assert ( xDeck.getOrderIndex() < initialIndex )

        xPanels = xDeck.getPanels()

        panelsCount = xPanels.getCount()
        self.assertEqual ( panelsCount, 5 )

        firstPanelName = self.getFirstPanel(xPanels)

        panelElementNames = xPanels.getElementNames()
        assert ( firstPanelName in panelElementNames )
        assert ( xPanels.hasByName(firstPanelName) )

        xPanel = xPanels.getByName(firstPanelName)
        assert ( xPanel )
        assert ( xPanel.getId() == firstPanelName )

        newTitle = "New title"
        xPanel.setTitle(newTitle)
        assert ( xPanel.getTitle() == newTitle )

        initialIndex = xPanel.getOrderIndex()
        xPanel.moveLast()
        assert ( xPanel.getOrderIndex() > initialIndex )

        initialIndex = xPanel.getOrderIndex()
        xPanel.moveFirst()
        assert ( xPanel.getOrderIndex() < initialIndex )

        initialIndex = xPanel.getOrderIndex()
        xPanel.moveDown()
        assert ( xPanel.getOrderIndex() > initialIndex )

        initialIndex = xPanel.getOrderIndex()
        xPanel.moveUp()
        assert ( xPanel.getOrderIndex() < initialIndex )

        xPanel.collapse()
        assert( not xPanel.isExpanded() )

        lastPanelName = self.getLastPanel(xPanels)

        otherPanel = xPanels.getByName(lastPanelName)
        otherPanel.expand(False)
        assert( otherPanel.isExpanded() )

        xPanel.expand(True)
        assert( xPanel.isExpanded() )
        assert( not otherPanel.isExpanded() )

    # close the document
        xDoc.dispose()

    def getFirstPanel(self, xPanels):

        panelName = ""
        curIndex = 10000

        for panel in xPanels:
            if panel.getOrderIndex() < curIndex:
                panelName = panel.getId()
                curIndex = panel.getOrderIndex()

        return panelName

    def getLastPanel(self, xPanels):

        panelName = ""
        curIndex = 0

        for panel in xPanels:
            if panel.getOrderIndex() > curIndex:
                panelName = panel.getId()
                curIndex = panel.getOrderIndex()

        return panelName

if __name__ == "__main__":
    unittest.main()

# vim: set noet sw=4 ts=4:
