# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug 67346 - EDITING: Undo broken when pasting text that has been copied from the input line

class tdf67346(UITestCase):

    def test_tdf67346_undo_paste_text_input_line(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        # type 'Apple' in A1
        enter_text_to_cell(gridwin, "A1", "Apple")
        # input line: copy the text from there
        xInputWin = xCalcDoc.getChild("sc_input_window")
        xInputWin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        self.xUITest.executeCommand(".uno:Copy")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
        # Ctrl-V
        self.xUITest.executeCommand(".uno:Paste")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Apple")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "Apple")
        self.assertEqual(get_state_as_dict(xInputWin)["Text"], "Apple")
        #Ctrl-Z
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Apple")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "")
        self.assertEqual(get_state_as_dict(xInputWin)["Text"], "")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
