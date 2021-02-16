# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import change_measurement_unit

class tdf139511(UITestCase):

   def test_tdf139511(self):
        self.ui_test.create_doc_in_start_center("impress")
        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        change_measurement_unit(self, "Centimeter")

        self.xUITest.executeCommand(".uno:InsertTable?Columns:short=4&Rows:short=4")

        document = self.ui_test.get_component()
        self.assertEqual(8004, document.DrawPages[0].getByIndex(2).BoundRect.Height)
        self.assertEqual(14104, document.DrawPages[0].getByIndex(2).BoundRect.Width)

        self.ui_test.execute_dialog_through_command(".uno:TransformDialog")

        xDialog = self.xUITest.getTopFocusWindow()

        xWidth = xDialog.getChild('MTR_FLD_WIDTH')
        xHeight = xDialog.getChild('MTR_FLD_HEIGHT')

        xWidth.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
        xWidth.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
        xWidth.executeAction("TYPE", mkPropertyValues({"TEXT": "10"}))

        xHeight.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
        xHeight.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
        xHeight.executeAction("TYPE", mkPropertyValues({"TEXT": "5"}))

        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        document = self.ui_test.get_component()

        # Without the fix in place, this test would have failed with
        # AssertionError: 5005 != 8004
        self.assertEqual(5005, document.DrawPages[0].getByIndex(2).BoundRect.Height)
        self.assertEqual(10005, document.DrawPages[0].getByIndex(2).BoundRect.Width)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
