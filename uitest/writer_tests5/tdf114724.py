#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import time
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/uitest/writer_tests/data/" + file_name

class tdf114724(UITestCase):

    def test_track_headings_outline(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf114724.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:Sidebar")
        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

        xNavigatorPanel = xWriterEdit.getChild("NavigatorPanelParent")
        xNavigatorPanel.executeAction("ROOT", tuple())

        xContentTree = xNavigatorPanel.getChild('contenttree')
        #Check the content has changed
        self.assertEqual(len(xContentTree.getChildren()), 1)

        xWriterEdit.executeAction("FOCUS", tuple())
        time.sleep(2)
        self.assertEqual(get_state_as_dict(xNavigatorPanel)["selectioncount"], "1")
        for _ in range(0,3):
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            time.sleep(2)
            self.assertEqual(get_state_as_dict(xNavigatorPanel)["selectioncount"], "1")
        for _ in range(0,3):
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            time.sleep(2)
            self.assertEqual(get_state_as_dict(xNavigatorPanel)["selectioncount"], "1")

        self.xUITest.executeCommand(".uno:Sidebar")
        self.ui_test.close_doc()
