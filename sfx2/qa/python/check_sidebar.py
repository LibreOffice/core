# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest
from org.libreoffice.unotest import UnoInProcess


class CheckSidebar(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown() 

    def test_check_sidebar(self):

        xDoc = self.__class__._uno.openEmptyDoc( url = "private:factory/scalc", bHidden = False, bReadOnly = False)
        xController = xDoc.getCurrentController()

        xSidebar = xController.getSidebar()
        assert(xSidebar)

        xSidebar.setVisible(True)
        self.assertTrue ( xSidebar.isVisible() )

        # TODO: does not work in unit test context 
#        xSidebar.setVisible(False)
#        isVisible = xSidebar.isVisible()
#        assert( not isVisible )
#        xSidebar.setVisible(True)

        xSidebar.showDecks(False)
        xSidebar.showDecks(True)

        xDecks = xSidebar.getDecks()

        first_deck_name = "PropertyDeck"

        deck_element_names = xDecks.getElementNames()
        assert ( first_deck_name in deck_element_names )
        assert ( xDecks.hasByName(first_deck_name) )

        decks_count = len(xDecks)
        self.assertEqual ( 5, decks_count )

        xDeck = xDecks[first_deck_name]
        assert ( xDeck )
        assert ( xDeck.getId() == first_deck_name )

        new_deck_title = "New title"
        xDeck.setTitle(new_deck_title)
        assert ( xDeck.getTitle() == new_deck_title )

        xDeck.moveFirst()
        initial_index = xDeck.getOrderIndex()
        self.assertEqual(100, initial_index)

        xDeck.moveLast()
        assert ( xDeck.getOrderIndex() > initial_index )

        initial_index = xDeck.getOrderIndex()
        xDeck.moveFirst()
        assert ( xDeck.getOrderIndex() < initial_index )

        initial_index = xDeck.getOrderIndex()
        xDeck.moveDown()
        assert ( xDeck.getOrderIndex() > initial_index )

        initial_index = xDeck.getOrderIndex()
        xDeck.moveUp()
        assert ( xDeck.getOrderIndex() < initial_index )

        xPanels = xDeck.getPanels()

        panels_count = len(xPanels)
        self.assertEqual ( panels_count, 5 )

        first_panel_name = self.getFirstPanel(xPanels)

        panel_element_names = xPanels.getElementNames()
        assert ( first_panel_name in panel_element_names )
        assert ( xPanels.hasByName(first_panel_name) )

        xPanel = xPanels[first_panel_name]
        assert ( xPanel )
        assert ( xPanel.getId() == first_panel_name )

        new_title = "New title"
        xPanel.setTitle(new_title)
        assert ( xPanel.getTitle() == new_title )

        initial_index = xPanel.getOrderIndex()
        xPanel.moveLast()
        assert ( xPanel.getOrderIndex() > initial_index )

        initial_index = xPanel.getOrderIndex()
        xPanel.moveFirst()
        assert ( xPanel.getOrderIndex() < initial_index )

        initial_index = xPanel.getOrderIndex()
        xPanel.moveDown()
        assert ( xPanel.getOrderIndex() > initial_index )

        initial_index = xPanel.getOrderIndex()
        xPanel.moveUp()
        assert ( xPanel.getOrderIndex() < initial_index )

        xPanel.collapse()
        assert( not xPanel.isExpanded() )

        last_panel_name = self.getLastPanel(xPanels)

        other_panel = xPanels[last_panel_name]
        other_panel.expand(False)
        assert( other_panel.isExpanded() )

        xPanel.expand(True)
        assert( xPanel.isExpanded() )
        assert( not other_panel.isExpanded() )

    # close the document
        xDoc.dispose()

    def getFirstPanel(self, xPanels):

        panel_name = ""
        cur_index = 10000

        for panel in xPanels:
            if panel.getOrderIndex() < cur_index:
                panel_name = panel.getId()
                cur_index = panel.getOrderIndex()

        return panel_name

    def getLastPanel(self, xPanels):

        panel_name = ""
        cur_index = 0

        for panel in xPanels:
            if panel.getOrderIndex() > cur_index:
                panel_name = panel.getId()
                cur_index = panel.getOrderIndex()

        return panel_name

if __name__ == "__main__":
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
