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
import uno

class CheckSidebarRegistry(unittest.TestCase):
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

    def test_sidebar_registry(self):

        # assert(result) after whole processing to list defected nodes at once
        result = True

        #open registry node in Sidebar.xcu
        configProvider = self.createUnoService("com.sun.star.configuration.ConfigurationProvider")

        param = uno.createUnoStruct('com.sun.star.beans.PropertyValue')
        param.Name = "nodepath"


        # Deck names consitency

        param.Value = "org.openoffice.Office.UI.Sidebar/Content/DeckList"

        sidebarDecksSettings = configProvider.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess",
                                                                    (param, ))
        for nodeName in sidebarDecksSettings:

            node = sidebarDecksSettings.getByName(nodeName)

            if (node.Id != nodeName):
                print("\nNon-consistent sidebar.xcu Deck registry names", nodeName, node.Id)
                result = False

        # panel names consitency

        param.Value = "org.openoffice.Office.UI.Sidebar/Content/PanelList"

        sidebarPanelsSettings = configProvider.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess",
                                                                    (param, ))
        for nodeName in sidebarPanelsSettings:

            node = sidebarPanelsSettings.getByName(nodeName)

            if (node.Id != nodeName):
                print("\nNon-consistent sidebar.xcu Panel registry names", nodeName, node.Id)
                result = False

            # is panel bound to an existing Deck ?
            FoundDeckId = False
            for deckNodeName in sidebarDecksSettings:
                deckNode = sidebarDecksSettings.getByName(deckNodeName)
                if (node.DeckId == deckNode.Id):
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

# vim: set noet sw=4 ts=4:
