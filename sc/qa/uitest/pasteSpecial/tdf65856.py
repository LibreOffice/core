# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position
from libreoffice.calc.paste_special import reset_default_values

class tdf65856(UITestCase):

    def test_tdf65856_paste_special_shift_right(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf65856.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            #- mark D1:E14; copy
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "D1:E14"}))
            self.xUITest.executeCommand(".uno:Copy")
            #mark cell D1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
            with self.ui_test.execute_dialog_through_command(".uno:PasteSpecial") as xDialog:
                reset_default_values(self, xDialog)

                xmove_right = xDialog.getChild("move_right")
                xmove_right.executeAction("CLICK", tuple())

            #check
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "T1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getString(), "TE1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 0).getString(), "TES1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 0).getString(), "TEST1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 0).getString(), "TEST1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 0).getString(), "TEST1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 0).getString(), "TEST1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 13).getString(), "T14")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 13).getString(), "TE14")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 13).getString(), "TES14")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 13).getString(), "TEST14")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 13).getString(), "TEST14")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 13).getString(), "TEST14")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 13).getString(), "TEST14")

    def test_tdf65856_paste_special_shift_right_2(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf65856_2.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            #- select range C2:D4; copy
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C2:D4"}))
            self.xUITest.executeCommand(".uno:Copy")
            #mark cell B2
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B2"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:PasteSpecial") as xDialog:
                xmove_right = xDialog.getChild("move_right")

                # tdf#69750: Without the fix in place, this test would have failed here
                self.assertEqual("true", get_state_as_dict(xmove_right)["Checked"])

            #check
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getString(), "1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 1).getString(), "1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 1).getString(), "1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 1).getString(), "1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 1).getString(), "1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getString(), "2")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 2).getString(), "2")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 2).getString(), "2")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 2).getString(), "2")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 2).getString(), "2")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getString(), "3")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 3).getString(), "3")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 3).getString(), "3")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 3).getString(), "3")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 3).getString(), "3")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 1).getFormula(), "=D2")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 1).getFormula(), "=D2")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 2).getFormula(), "=E3")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 2).getFormula(), "=E3")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 3).getFormula(), "=F4")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 3).getFormula(), "=F4")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
