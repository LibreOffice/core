#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf114724(UITestCase):

    def test_track_headings_outline(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf114724.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:Sidebar")
        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

        xNavigatorPanel = xWriterEdit.getChild("NavigatorPanelParent")
        xNavigatorPanel.executeAction("ROOT", tuple())

        xWriterEdit.executeAction("FOCUS", tuple())

        self.ui_test.wait_until_property_is_updated(xNavigatorPanel, "selectedtext", "HEADING 1")
        self.assertEqual(get_state_as_dict(xNavigatorPanel)["selectedtext"], "HEADING 1")
        self.assertEqual(get_state_as_dict(xNavigatorPanel)["selectioncount"], "1")
        for _ in range(0,3):
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))

        self.ui_test.wait_until_property_is_updated(xNavigatorPanel, "selectedtext", "HEADING 4")
        self.assertEqual(get_state_as_dict(xNavigatorPanel)["selectedtext"], "HEADING 4")
        self.assertEqual(get_state_as_dict(xNavigatorPanel)["selectioncount"], "1")

        for _ in range(0,3):
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))

        self.ui_test.wait_until_property_is_updated(xNavigatorPanel, "selectedtext", "HEADING 1")
        self.assertEqual(get_state_as_dict(xNavigatorPanel)["selectedtext"], "HEADING 1")
        self.assertEqual(get_state_as_dict(xNavigatorPanel)["selectioncount"], "1")

        self.xUITest.executeCommand(".uno:Sidebar")
        self.ui_test.close_doc()
