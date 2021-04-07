# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

class tdf65856(UITestCase):

    def test_tdf65856_paste_special_shift_right(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf65856.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #- mark D1:E14; copy
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "D1:E14"}))
        self.xUITest.executeCommand(".uno:Copy")
        #mark cell D1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
        self.ui_test.execute_dialog_through_command(".uno:PasteSpecial")
        xDialog = self.xUITest.getTopFocusWindow()

        xmove_right = xDialog.getChild("move_right")
        xmove_right.executeAction("CLICK", tuple())

        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        #check
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "T1")
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "TE1")
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getString(), "TES1")
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getString(), "TEST1")
        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getString(), "TEST1")
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "TEST1")
        self.assertEqual(get_cell_by_position(document, 0, 6, 0).getString(), "TEST1")
        self.assertEqual(get_cell_by_position(document, 0, 0, 13).getString(), "T14")
        self.assertEqual(get_cell_by_position(document, 0, 1, 13).getString(), "TE14")
        self.assertEqual(get_cell_by_position(document, 0, 2, 13).getString(), "TES14")
        self.assertEqual(get_cell_by_position(document, 0, 3, 13).getString(), "TEST14")
        self.assertEqual(get_cell_by_position(document, 0, 4, 13).getString(), "TEST14")
        self.assertEqual(get_cell_by_position(document, 0, 5, 13).getString(), "TEST14")
        self.assertEqual(get_cell_by_position(document, 0, 6, 13).getString(), "TEST14")
        self.ui_test.close_doc()

    def test_tdf65856_paste_special_shift_right_2(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf65856_2.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #- select range C2:D4; copy
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C2:D4"}))
        self.xUITest.executeCommand(".uno:Copy")
        #mark cell B2
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B2"}))
        self.ui_test.execute_dialog_through_command(".uno:PasteSpecial")
        xDialog = self.xUITest.getTopFocusWindow()

        xmove_right = xDialog.getChild("move_right")
        xmove_right.executeAction("CLICK", tuple())

        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        #check
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "1")
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getString(), "1")
        self.assertEqual(get_cell_by_position(document, 0, 3, 1).getString(), "1")
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getString(), "1")
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getString(), "1")
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getString(), "2")
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getString(), "2")
        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getString(), "2")
        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getString(), "2")
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getString(), "2")
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getString(), "3")
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getString(), "3")
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getString(), "3")
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getString(), "3")
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getString(), "3")
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getFormula(), "=D2")
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getFormula(), "=D2")
        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getFormula(), "=E3")
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getFormula(), "=E3")
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getFormula(), "=F4")
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getFormula(), "=F4")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

