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

class tdf140661(UITestCase):

    def launch_navigator(self, bIsBeforeUngroup):
        xMainWindow = self.xUITest.getTopFocusWindow()
        xWriterEdit = xMainWindow.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:Sidebar")

        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

        # wait until the navigator panel is available
        xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

        xContentTree = xNavigatorPanel.getChild("contenttree")
        xDrawings = xContentTree.getChild('11')
        self.assertEqual('Drawing objects', get_state_as_dict(xDrawings)['Text'])

        xDrawings.executeAction("EXPAND", tuple())

        if bIsBeforeUngroup:
            self.assertEqual(1, len(xDrawings.getChildren()))
            self.assertEqual('DrawObject1', get_state_as_dict(xDrawings.getChild('0'))['Text'])
        else:
            self.assertEqual(12, len(xDrawings.getChildren()))

            # tdf#134960
            expectedShapeOrder = [1, 2, 8, 9, 7, 10, 11, 3, 12, 4, 5, 6]
            for i in range(12):
                self.assertEqual('Shape' + str(expectedShapeOrder[i]), get_state_as_dict(xDrawings.getChild(str(i)))['Text'])

        xDrawings.executeAction("COLLAPSE", tuple())

        self.xUITest.executeCommand(".uno:Sidebar")

    def test_tdf140661(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf140661.odt")) as writer_doc:

            self.launch_navigator(True)

            # Select the shape and ungroup it
            self.xUITest.executeCommand(".uno:JumpToNextFrame")

            self.ui_test.wait_until_child_is_available('metricfield')

            self.xUITest.executeCommand(".uno:FormatUngroup")

            # Without the fix in place, this test would have failed with
            # AssertionError: 12 != 0
            self.launch_navigator(False)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
