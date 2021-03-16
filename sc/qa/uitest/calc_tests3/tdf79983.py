# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 79983 - Calc sort lists aren't case sensitive

class tdf79983(UITestCase):
    def test_tdf79983_sort_list_case_sensitive(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        enter_text_to_cell(gridwin, "A1", "AAA")
        enter_text_to_cell(gridwin, "A2", "BBB")
        enter_text_to_cell(gridwin, "A3", "CCC")
        enter_text_to_cell(gridwin, "A4", "DDD")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))

        #Tools / Options / LibreOffice Calc / Sort Lists
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xCalcEntry = xPages.getChild('3')                 # Calc
        xCalcEntry.executeAction("EXPAND", tuple())
        xCalcSortEntry = xCalcEntry.getChild('5')
        xCalcSortEntry.executeAction("SELECT", tuple())          #Sort List
        #copy new list from selected cells
        xcopy = xDialogOpt.getChild("copy")
        xcopy.executeAction("CLICK", tuple())

        self.assertEqual("AAA\nBBB\nCCC\nDDD", get_state_as_dict(xDialogOpt.getChild("entries"))['Text'])

        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DELETE"}))

        enter_text_to_cell(gridwin, "A1", "AAA")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))
        #fill down
        self.ui_test.execute_dialog_through_command(".uno:FillSeries")
        xDialog = self.xUITest.getTopFocusWindow()
        xautofill = xDialog.getChild("autofill")
        xautofill.executeAction("CLICK", tuple())
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "AAA")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "BBB")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "CCC")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "DDD")
        #UNDO
        self.xUITest.executeCommand(".uno:Undo")
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "AAA")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: