# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_by_text
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import is_row_hidden
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf82008(UITestCase):

    def test_tdf82008_drop_standard_filter_criterion(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            #     A    B
            # 1   a    b     <- column headers
            # 2   1          <- matched by the criterion to be dropped
            # 3        2     <- matched by the criterion to be kept
            enter_text_to_cell(gridwin, "A1", "a")
            enter_text_to_cell(gridwin, "B1", "b")
            enter_text_to_cell(gridwin, "A2", "1")
            enter_text_to_cell(gridwin, "B3", "2")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B3"}))

            # Open the standard data filter and apply both criteria
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xField1 = xDialog.getChild("field1")
                xVal1 = xDialog.getChild("val1")
                xConnect2 = xDialog.getChild("connect2")
                xField2 = xDialog.getChild("field2")
                xVal2 = xDialog.getChild("val2")

                # Add criteria to the standard data filter (a = 1 OR b = 2)
                select_by_text(xField1, "a")
                xVal1.executeAction("TYPE", mkPropertyValues({"TEXT": "1"}))
                select_by_text(xConnect2, "OR")
                select_by_text(xField2, "b")
                xVal2.executeAction("TYPE", mkPropertyValues({"TEXT": "2"}))

                # Drop the first criterion only
                select_by_text(xField1, "- none -")

                # Without the fix in place, this test would have failed with
                # AssertionError: 'b' != '- none -'
                self.assertEqual("b", get_state_as_dict(xField1)["SelectEntryText"])
                self.assertEqual("2", get_state_as_dict(xVal1)["Text"])
                self.assertEqual("- none -", get_state_as_dict(xField2)["SelectEntryText"])

            # The remaining criterion (b = 2) is applied after closing the standard data filter dialog
            self.assertTrue(is_row_hidden(document, 1))
            self.assertFalse(is_row_hidden(document, 2))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
