# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.path import get_srcdir_url

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.uihelper.common import get_state_as_dict

import time

class SlideLayoutChangingTest(UITestCase):

    def test_tdf98980(self):
        self.ui_test.create_doc_in_start_center("impress")

        # close slide layout dialog
        xTemplateDlg = self.xUITest.getTopFocusWindow()
        print(xTemplateDlg)
        xCancelBtn = xTemplateDlg.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        # ensure side bar is visible
        xImpressDoc = self.ui_test.get_component()
        xController = xImpressDoc.getCurrentController()

        xSidebar = xController.getSidebar()
        assert(xSidebar)

        xSidebar.setVisible(True)
        isVisible = xSidebar.isVisible()
        self.assertTrue ( xSidebar.isVisible() )

        # select different layouts and see if it is updated inside sidebar
        layouts= (  ".uno:AssignLayout?WhatLayout:long=20", ".uno:AssignLayout?WhatLayout:long=19",
                    ".uno:AssignLayout?WhatLayout:long=0",  ".uno:AssignLayout?WhatLayout:long=1",
                    ".uno:AssignLayout?WhatLayout:long=32", ".uno:AssignLayout?WhatLayout:long=3",
                    ".uno:AssignLayout?WhatLayout:long=12", ".uno:AssignLayout?WhatLayout:long=15",
                    ".uno:AssignLayout?WhatLayout:long=14", ".uno:AssignLayout?WhatLayout:long=16",
                    ".uno:AssignLayout?WhatLayout:long=18", ".uno:AssignLayout?WhatLayout:long=34",
                    ".uno:AssignLayout?WhatLayout:long=28", ".uno:AssignLayout?WhatLayout:long=27",
                    ".uno:AssignLayout?WhatLayout:long=29", ".uno:AssignLayout?WhatLayout:long=30")

        for i in layouts:
            self.xUITest.executeCommand(i)

            xImpressDoc = self.ui_test.get_component()
            xController = xImpressDoc.getCurrentController()

            xSidebar = xController.getSidebar()
            assert(xSidebar)

            xDecks = xSidebar.getDecks()
            print(xDecks)
            print(xDecks.getElementNames())

            xDeck = xDecks["PropertyDeck"]
            xPanels = xDeck.getPanels()

            xSdLayoutsPanel = xPanels["SdLayoutsPanel"]

            xDialog = xSdLayoutsPanel.getDialog()

            # TODO: check if the layout was really selected

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: