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
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_row


from uitest.path import get_srcdir_url
def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sc/qa/uitest/autofilter/data/" + file_name

def is_row_hidden(doc, index):
    row = get_row(doc, index)
    val = row.getPropertyValue("IsVisible")
    return not val

#Bug 122260 - EDITING Autofilters not properly cleared
class tdf122260(UITestCase):
    def check_value_in_AutoFilter(self, gridwin, columnIndex, valueIndex):
        # open filter pop-up window
        self.assertIsNotNone(gridwin)
        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": columnIndex, "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        self.assertIsNotNone(xFloatWindow)

        # get check list
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        self.assertIsNotNone(xCheckListMenu)

        xTreeList = xCheckListMenu.getChild("check_list_box")
        self.assertIsNotNone(xTreeList)

        # on/off required checkbox
        xEntry = xTreeList.getChild(valueIndex)
        self.assertIsNotNone(xEntry)
        xEntry.executeAction("CLICK", tuple())

        # close pop-up window
        xOkBtn = xFloatWindow.getChild("ok")
        self.assertIsNotNone(xOkBtn)
        xOkBtn.executeAction("CLICK", tuple())

    def get_values_count_in_AutoFilter(self, gridwin, columnIndex):
        # open filter pop-up window
        self.assertIsNotNone(gridwin)
        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": columnIndex, "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        self.assertIsNotNone(xFloatWindow)

        # get check list
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        self.assertIsNotNone(xCheckListMenu)

        xTreeList = xCheckListMenu.getChild("check_list_box")
        self.assertIsNotNone(xTreeList)

        valuesCount = len(xTreeList.getChildren())

        # close pop-up window
        xOkBtn = xFloatWindow.getChild("ok")
        self.assertIsNotNone(xOkBtn)
        xOkBtn.executeAction("CLICK", tuple())

        return valuesCount

    def test_tdf122260_autofilter(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf122260.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        self.assertIsNotNone(gridwin)

        # filter out b1
        self.check_value_in_AutoFilter(gridwin, "1", "0")
        # filter out a2 (as a1 is filtered out a2 is the first item)
        self.check_value_in_AutoFilter(gridwin, "0", "0")
        # return back a2 (as a1 is filtered out a2 is the first item)
        self.check_value_in_AutoFilter(gridwin, "0", "0")

        # check rows visibility
        # row-0 is row with headers
        self.assertTrue(is_row_hidden(calc_doc, 1))
        self.assertFalse(is_row_hidden(calc_doc, 2))
        self.assertFalse(is_row_hidden(calc_doc, 3))
        self.assertFalse(is_row_hidden(calc_doc, 4))

        # check if "b1" is accessible in filter of the column-b
        # (so all values of the column B are available)
        self.assertEqual(4, self.get_values_count_in_AutoFilter(gridwin, "1"))

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
