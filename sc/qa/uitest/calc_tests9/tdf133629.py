# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf133629(UITestCase):

    def test_tdf133629(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        self.ui_test.execute_dialog_through_command(".uno:Insert")
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("ok")
        xOKButton.executeAction("CLICK", tuple())

        self.xUITest.executeCommand(".uno:TableSelectAll")

        self.xUITest.executeCommand(".uno:SelectAll")

        # Check selection
        self.assertIsNotNone(getattr(document.CurrentSelection, 'Sheet1.A1:AMJ1048576'))
        self.assertIsNotNone(getattr(document.CurrentSelection, 'Sheet1.A2:AMJ1048576'))

        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "5")

        xPresets = xDialog.getChild("presets")
        xPresets.executeAction("CHOOSE", mkPropertyValues({"POS": "4"}))

        # Change width
        xLineWidth = xDialog.getChild("linewidthmf")
        xLineWidth.executeAction("UP", tuple())
        xLineWidth.executeAction("UP", tuple())
        self.assertEqual("1.25 pt", get_state_as_dict(xLineWidth)['Text'])

        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        # Without the fix in place, this test would have crash here

        xA1Sheet1 = document.Sheets[0].getCellRangeByName("A1")
        self.assertEqual(44, xA1Sheet1.BottomBorder.OuterLineWidth)
        self.assertEqual(44, xA1Sheet1.TopBorder.OuterLineWidth)
        self.assertEqual(44, xA1Sheet1.LeftBorder.OuterLineWidth)
        self.assertEqual(44, xA1Sheet1.RightBorder.OuterLineWidth)

        xA1Sheet2 = document.Sheets[1].getCellRangeByName("A1")
        self.assertEqual(44, xA1Sheet2.BottomBorder.OuterLineWidth)
        self.assertEqual(44, xA1Sheet2.TopBorder.OuterLineWidth)
        self.assertEqual(44, xA1Sheet2.LeftBorder.OuterLineWidth)
        self.assertEqual(44, xA1Sheet2.RightBorder.OuterLineWidth)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
