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

class tdf159428(UITestCase):

    def test_tdf159428(self):
        global selectionChangedResult
        with self.ui_test.load_file(get_url_for_data_file('tdf159428.odt')):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:Sidebar")
            xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

            xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')
            xContentTree = xNavigatorPanel.getChild("contenttree")

            # select fist bookmark - Heading "H1" should be tracked
            self.xUITest.executeCommand(".uno:GoToNextPara")
            self.xUITest.executeCommand(".uno:GoToNextPara")
            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "111", "END_POS": "119"}))
            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "H1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "H1")

            # select second bookmark - Heading "H1 mit Lesezeichen" should be tracked
            self.xUITest.executeCommand(".uno:GoToNextPara")
            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "0", "END_POS": "18"}))
            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "H1 mit Lesezeichen")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "H1 mit Lesezeichen")

            # select third bookmark - no heading tracked
            self.xUITest.executeCommand(".uno:GoToPrevPara")
            self.xUITest.executeCommand(".uno:GoToPrevPara")
            self.xUITest.executeCommand(".uno:GoToPrevPara")
            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "141", "END_POS": "146"}))
            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "Lesezeichen 3")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "Lesezeichen 3")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
