# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import is_row_hidden

class tdf138089(UITestCase):

    def test_tdf138089(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf138089.xlsx")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            self.assertFalse(is_row_hidden(calc_doc, 0))
            self.assertTrue(is_row_hidden(calc_doc, 1))
            self.assertTrue(is_row_hidden(calc_doc, 2))
            self.assertTrue(is_row_hidden(calc_doc, 3))
            self.assertFalse(is_row_hidden(calc_doc, 4))
            self.assertFalse(is_row_hidden(calc_doc, 5))
            self.assertFalse(is_row_hidden(calc_doc, 6))

            # Without the fix in place, this test would have crashed here
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                self.assertEqual("2017-12-01", get_state_as_dict(xDialog.getChild("val1"))['Text'])
                self.assertEqual("过帐日期", get_state_as_dict(xDialog.getChild("field1"))["DisplayText"])


            self.assertFalse(is_row_hidden(calc_doc, 0))
            self.assertTrue(is_row_hidden(calc_doc, 1))
            self.assertTrue(is_row_hidden(calc_doc, 2))
            self.assertTrue(is_row_hidden(calc_doc, 3))
            self.assertFalse(is_row_hidden(calc_doc, 4))
            self.assertFalse(is_row_hidden(calc_doc, 5))
            self.assertTrue(is_row_hidden(calc_doc, 6))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
