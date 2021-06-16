# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

class Tdf142763(UITestCase):

    def test_tdf142763(self):
        doc = self.ui_test.load_file(get_url_for_data_file("tdf142763.ods"))

        self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog")
        xPageFormatDialog = self.xUITest.getTopFocusWindow()

        xTabControl = xPageFormatDialog.getChild("tabcontrol")
        select_pos(xTabControl, "4")

        xHeaderTab = self.xUITest.getTopFocusWindow()
        xHeaderOnBtn = xHeaderTab.getChild("checkHeaderOn")
        xHeaderOnBtn.executeAction("CLICK", tuple())
        xFirstHeaderSameBtn = xHeaderTab.getChild("checkSameFP")
        xFirstHeaderSameBtn.executeAction("CLICK", tuple())
        xEditButton = xHeaderTab.getChild("buttonEdit")
        xEditButton.executeAction("CLICK", tuple())

        xHeaderDialog = self.xUITest.getTopFocusWindow()
        xLeftTextWindow = xHeaderDialog.getChild("textviewWND_LEFT")
        xLeftTextWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "first "}))
        xCenterTextWindow = xHeaderDialog.getChild("textviewWND_CENTER")
        xCenterTextWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "page "}))
        xRightTextWindow = xHeaderDialog.getChild("textviewWND_RIGHT")
        xRightTextWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "header "}))

        self.assertEqual("first (1)", get_state_as_dict(xLeftTextWindow)["Text"])
        self.assertEqual("page (2)", get_state_as_dict(xCenterTextWindow)["Text"])
        self.assertEqual("header (3)", get_state_as_dict(xRightTextWindow)["Text"])

        xOK = xHeaderDialog.getChild('ok')
        with self.ui_test.wait_until_component_loaded():
            self.ui_test.close_dialog_through_button(xOK)

        #select_pos(xTabControl, "5")
        #
        #xFooterTab = self.xUITest.getTopFocusWindow()
        #xFooterOnBtn = xFooterTab.getChild("checkFooterOn")
        #xFooterOnBtn.executeAction("CLICK", tuple())
        #xEditButton = xFooterTab.getChild("buttonEdit") # this also opens the Header content dialog for some reason
        #xEditButton.executeAction("CLICK", tuple())
        #
        #xFooterDialog = self.xUITest.getTopFocusWindow()
        #xLeftTextWindow = xFooterDialog.getChild("textviewWND_LEFT")
        #xLeftTextWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "first "}))
        #xCenterTextWindow = xFooterDialog.getChild("textviewWND_CENTER")
        #xCenterTextWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "page "}))
        #xRightTextWindow = xFooterDialog.getChild("textviewWND_RIGHT")
        #xRightTextWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "footer "}))
        #
        #self.assertEqual("first (I)", get_state_as_dict(xLeftTextWindow)["Text"])
        #self.assertEqual("page (II)", get_state_as_dict(xCenterTextWindow)["Text"])
        #self.assertEqual("footer (III)", get_state_as_dict(xRightTextWindow)["Text"])
        #
        #xOK = xFooterDialog.getChild('ok')
        #with self.ui_test.wait_until_component_loaded():
        #    self.ui_test.close_dialog_through_button(xOK)

        xOK = xPageFormatDialog.getChild('ok')
        with self.ui_test.wait_until_component_loaded():
            self.ui_test.close_dialog_through_button(xOK)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
