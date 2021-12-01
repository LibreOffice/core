# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf114724(UITestCase):

    def test_track_headings_outline(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf114724.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:Sidebar")
            xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("content5")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "0"})) # 'root' button

            xWriterEdit.executeAction("FOCUS", tuple())

            xContentTree = xNavigatorPanel.getChild("contenttree")

            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "HEADING 1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "HEADING 1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectionCount"], "1")
            for _ in range(0,3):
                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))

            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "HEADING 4")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "HEADING 4")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectionCount"], "1")

            for _ in range(0,3):
                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))

            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "HEADING 1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "HEADING 1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectionCount"], "1")

            # tdf#124456 makes the sidebar navigator consistent with the floating navigator's use
            # of the stored root mode setting. This requires the root mode button to be clicked
            # here to change the content navigation view back to all categories view for subsequent
            # tests that expect the navigator tree to not be in root mode.
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "0"})) # 'root' button
            self.xUITest.executeCommand(".uno:Sidebar")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
