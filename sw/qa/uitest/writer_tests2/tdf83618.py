# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos, change_measurement_unit
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf83618(UITestCase):

    def test_tdf83618(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")

        change_measurement_unit(self, "Point")

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")

        xLineDist = xDialog.getChild("comboLB_LINEDIST")
        self.assertEqual("Single", get_state_as_dict(xLineDist)['SelectEntryText'])

        select_pos(xLineDist, "7")

        self.assertEqual("Fixed", get_state_as_dict(xLineDist)['SelectEntryText'])

        xLineDistValue = xDialog.getChild("spinED_LINEDISTMETRIC")
        xLineDistValue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xLineDistValue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xLineDistValue.executeAction("TYPE", mkPropertyValues({"TEXT":"20"}))

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        document = self.ui_test.get_component()
        paragraph = document.Text.createEnumeration().nextElement()
        self.assertEqual(706, paragraph.ParaLineSpacing.Height)

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:Sidebar")
        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "TextPropertyPanel"}))

        xLineSpacing = xWriterEdit.getChild('linespacing')

        xLineSpacing.executeAction("CLICK", mkPropertyValues({"POS": "1"}))

        xFloatWindow = self.xUITest.getFloatWindow()

        xLineDistSidebar = xFloatWindow.getChild("line_dist")
        xLineDistMetric = xFloatWindow.getChild("metric_box")

        self.assertEqual("Fixed", get_state_as_dict(xLineDistSidebar)['SelectEntryText'])

        # Without the fix in place, this test would have failed with
        # AssertionError: '20.0 pt' != '11.3 pt'
        self.assertEqual("20.0 pt", get_state_as_dict(xLineDistMetric)['Text'])

        self.xUITest.executeCommand(".uno:Sidebar")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

