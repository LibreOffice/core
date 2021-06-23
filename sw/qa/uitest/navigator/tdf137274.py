# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

class tdf137274(UITestCase):

    def test_tdf137274(self):

        xMainDoc = self.ui_test.create_doc_in_start_center("writer")

        xMainWindow = self.xUITest.getTopFocusWindow()
        xWriterEdit = xMainWindow.getChild("writer_edit")

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "Test"}))

        # Insert first comment
        xArgs = mkPropertyValues({"Text": "C1"})
        self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

        # wait until the comment is available
        self.ui_test.wait_until_child_is_available('Comment1')

        self.xUITest.executeCommand(".uno:Sidebar")

        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

        # wait until the navigator panel is available
        xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

        xContentTree = xNavigatorPanel.getChild("contenttree")
        xComments = xContentTree.getChild('10')
        self.assertEqual('Comments', get_state_as_dict(xComments)['Text'])

        xComments.executeAction("EXPAND", tuple())

        self.assertEqual(1, len(xComments.getChildren()))
        self.assertEqual('C1', get_state_as_dict(xComments.getChild('0'))['Text'])

        xComments.executeAction("COLLAPSE", tuple())

        xArgs = mkPropertyValues({"Text": "C2"})
        self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

        # wait until the second comment is available
        self.ui_test.wait_until_child_is_available('Comment2')

        xComments.executeAction("EXPAND", tuple())

        # Without the fix in place, this test would have failed with AssertionError: 2 != 0
        self.assertEqual(2, len(xComments.getChildren()))
        self.assertEqual('C1', get_state_as_dict(xComments.getChild('0'))['Text'])

        xComments.executeAction("COLLAPSE", tuple())

        self.xUITest.executeCommand(".uno:Sidebar")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
