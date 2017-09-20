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
        layouts = ( "20", "19",
                    "0",  "1",
                    "32", "3",
                    "12", "15",
                    "14", "16",
                    "18", "34",
                    "28", "27",
                    "29", "30")

        for selectedLayoutId in layouts:
            self.xUITest.executeCommand(".uno:AssignLayout?WhatLayout:long=" + selectedLayoutId)

            # get controler
            xImpressDoc     = self.ui_test.get_component()
            xController     = xImpressDoc.getCurrentController()

            # get "Slide layout" panel
            xSidebar        = xController.getSidebar()
            xDecks          = xSidebar.getDecks()
            xDeck           = xDecks["PropertyDeck"]
            xPanels         = xDeck.getPanels()
            xSdLayoutsPanel = xPanels["SdLayoutsPanel"]

            xDialog = xSdLayoutsPanel.getDialog()

#            # TODO: check if the layout was really selected
#            xComboBox = xDialog.getControl("SlideLayout")
#            visibleLayoutId = xComboBox.getItemData(xComboBox.getSelectedItem())
#            
#            assert(str(visibleLayoutId) == selectedLayoutId);

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: