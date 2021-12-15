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

            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            # check the shape type
            self.assertEqual("com.sun.star.drawing.GroupShape", document.DrawPage.getByIndex(1).ShapeType)

            # select the shape
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            self.ui_test.wait_until_child_is_available('metricfield')

            # go inside the group
            self.xUITest.executeCommand(".uno:EnterGroup")
            xToolkit.processEventsToIdle()

            # select a shape in the group
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
            xToolkit.processEventsToIdle()

            # add a textbox to this subshape
            self.xUITest.executeCommand(".uno:AddTextBox")
            xToolkit.processEventsToIdle()

            # select the next shape in the group
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
            xToolkit.processEventsToIdle()

            # add a textbox to this subshape
            self.xUITest.executeCommand(".uno:AddTextBox")
            xToolkit.processEventsToIdle()

            # leave the groupshape
            self.xUITest.executeCommand(".uno:LeaveGroup")
            xToolkit.processEventsToIdle()

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
            xToolkit.processEventsToIdle()

            # move it down
            for i in range(1, 30):
                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
                time.sleep(0.1)

            # do ungroup
            self.xUITest.executeCommand(".uno:FormatUngroup")
            xToolkit.processEventsToIdle()

            # check if the first shape in the selection is a textbox
            self.assertEqual(True,document.getCurrentSelection().getByIndex(0).TextBox)

            # the socond shape is still a group, so it cannot be a textbox
            self.assertEqual(False,document.getCurrentSelection().getByIndex(1).TextBox)

            # do ungroup
            self.xUITest.executeCommand(".uno:FormatUngroup")
            xToolkit.processEventsToIdle()

            # check if all shapes in the seloection are textbox
            self.assertEqual(True,document.getCurrentSelection().getByIndex(0).TextBox)
            self.assertEqual(True,document.getCurrentSelection().getByIndex(1).TextBox)
            self.assertEqual(True,document.getCurrentSelection().getByIndex(2).TextBox)

            # Without the fix in place, the following problems occurred during this test:
            # - After the grouping old textbox frames detached from their shape before
            # - Moving caused messed layout
            # - After ungroup, the shapes in the embed group lost their textbox

# vim: set shiftwidth=4 softtabstop=4 expandtab:
