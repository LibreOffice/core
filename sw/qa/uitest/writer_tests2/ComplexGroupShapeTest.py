# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
import time

class ComplexGroupShapeTest(UITestCase):
    def test_ComplexGroupShape(self):
        with self.ui_test.load_file(get_url_for_data_file("ComplexGroupShapeTest.odt")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            document = self.ui_test.get_component()

            # check the shape type
            self.assertEqual("com.sun.star.drawing.GroupShape", document.DrawPage.getByIndex(1).ShapeType)

            # select the shape
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            self.ui_test.wait_until_child_is_available('metricfield')

            # go inside the group
            self.xUITest.executeCommand(".uno:EnterGroup")

            # select a shape in the group
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

            # add a textbox to this subshape
            self.xUITest.executeCommand(".uno:AddTextBox")

            # select the next shape in the group
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

            # add a textbox to this subshape
            self.xUITest.executeCommand(".uno:AddTextBox")

            # leave the groupshape
            self.xUITest.executeCommand(".uno:LeaveGroup")

            # select the other shape
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            self.ui_test.wait_until_child_is_available('metricfield')

            # get the current selection
            ShapeCollection = document.getCurrentSelection()

            # extend the selection with the grouped shape
            ShapeCollection.add(document.DrawPage.getByIndex(0))
            ShapeCollection.add(document.DrawPage.getByIndex(1))

            # select these shapes
            document.getCurrentController().select(ShapeCollection)

            # do ungroup
            self.xUITest.executeCommand(".uno:FormatGroup")

            # deselect
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"ESC"}))
            time.sleep(0.1)

            # select the group
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            self.ui_test.wait_until_child_is_available('metricfield')

            # move it down
            for i in range(1, 30):
                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
                time.sleep(0.1)

            # select again
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            self.ui_test.wait_until_child_is_available('metricfield')

            # do ungroup
            self.xUITest.executeCommand(".uno:FormatUngroup")

            # deselect everything
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"ESC"}))
            time.sleep(0.1)

            # select the first ex-group member shape
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            self.ui_test.wait_until_child_is_available('metricfield')
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

            # check if it is a textbox
            self.assertEqual(True,document.getCurrentSelection().getByIndex(0).TextBox)

            # go to the other one
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

            # this is still a group, so it cannot be a textbox
            self.assertEqual(False,document.getCurrentSelection().getByIndex(0).TextBox)

            # do ungroup
            self.xUITest.executeCommand(".uno:FormatUngroup")

            # deselect
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"ESC"}))
            time.sleep(0.1)

            # select one shape of the last group
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            self.ui_test.wait_until_child_is_available('metricfield')
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

            # check if it is a textbox
            self.assertEqual(True,document.getCurrentSelection().getByIndex(0).TextBox)

            # Without the fix in place, the following problems occurred during this test:
            # - After the grouping old textbox frames detached from their shape before
            # - Moving caused messed layout
            # - After ungroup, the shapes in the embed group lost their textbox

# vim: set shiftwidth=4 softtabstop=4 expandtab:
