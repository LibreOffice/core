# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import is_row_hidden

class tdf141547(UITestCase):

    def test_tdf141547(self):

        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf141547.xlsx"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

        self.assertFalse(is_row_hidden(calc_doc, 0))
        for i in range(1, 7):
            self.assertTrue(is_row_hidden(calc_doc, i))

        # Without the fix in place, this test would have crashed here
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        self.assertEqual("0", get_state_as_dict(xDialog.getChild("val1"))['Text'])
        self.assertEqual("过帐日期", get_state_as_dict(xDialog.getChild("field1"))["DisplayText"])
        self.assertEqual("Empty", get_state_as_dict(xDialog.getChild("val2"))['Text'])
        self.assertEqual("过帐日期", get_state_as_dict(xDialog.getChild("field2"))["DisplayText"])

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.assertFalse(is_row_hidden(calc_doc, 0))
        for i in range(1, 7):
            self.assertTrue(is_row_hidden(calc_doc, i))

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
