# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
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
import uno

class CheckSidebarRegistry(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xDoc = cls._uno.openEmptyDoc( url = "private:factory/scalc", bHidden = False, bReadOnly = False)

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_sidebar_registry(self):

        # assert(result) after whole processing to list defective nodes at once
        result = True

        #open registry node in Sidebar.xcu
        config_provider = self.createUnoService("com.sun.star.configuration.ConfigurationProvider")

        param = uno.createUnoStruct('com.sun.star.beans.PropertyValue')
        param.Name = "nodepath"


        # Deck names consistency

        param.Value = "org.openoffice.Office.UI.Sidebar/Content/DeckList"

        sidebar_decks_settings = config_provider.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess",
                                                                    (param, ))
        for nodeName in sidebar_decks_settings:

            node = sidebar_decks_settings[nodeName]

            if (node.Id != nodeName):
                print("\nNon-consistent sidebar.xcu Deck registry names", nodeName, node.Id)
                result = False

        # panel names consistency

        param.Value = "org.openoffice.Office.UI.Sidebar/Content/PanelList"

        sidebar_panels_settings = config_provider.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess",
                                                                    (param, ))
        for nodeName in sidebar_panels_settings:

            node = sidebar_panels_settings[nodeName]

            if (node.Id != nodeName):
                print("\nNon-consistent sidebar.xcu Panel registry names", nodeName, node.Id)
                result = False

            # is panel bound to an existing Deck ?
            FoundDeckId = False
            for deckNodeName in sidebar_decks_settings:
                deck_node = sidebar_decks_settings[deckNodeName]
                if (node.DeckId == deck_node.Id):
                    FoundDeckId = True
            if not FoundDeckId:
                print("\nNon existing DeckId for the panel ",node.Id)
                result = False

        # trigger the overall result. details of each error have already be printed
        assert(result)


    def createUnoService(self, serviceName):

        sm = uno.getComponentContext().ServiceManager
        return sm.createInstanceWithContext(serviceName, uno.getComponentContext())

if __name__ == "__main__":
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
