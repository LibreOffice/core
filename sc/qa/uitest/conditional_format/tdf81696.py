# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 81696 - CRASH while sorting cells with conditional formatting

class tdf81696(UITestCase):

    def test_tdf81696_sort_cell_conditional_formatting(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf81696.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B3"}))
        #Open sort dialog by DATA - SORT,Just sort it by Column A, ascending. (it's default)
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        xleftright = xDialog.getChild("leftright")
        select_pos(xTabs, "0")

        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "A")
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "B")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getValue(), 1)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab: