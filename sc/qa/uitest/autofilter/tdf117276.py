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
from libreoffice.calc.document import get_row
# import org.libreoffice.unotest
# import pathlib
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
#    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()
    return get_srcdir_url() + "/sc/qa/uitest/autofilter/data/" + file_name

def is_row_hidden(doc, index):
    row = get_row(doc, index)
    val = row.getPropertyValue("IsVisible")
    return not val

#Bug 117276 - Autofilter settings being reset in some cases

class tdf117276(UITestCase):
    def test_tdf117276_autofilter(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf117276.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        # 1. open attached file
        # 2. open filter of column B (Fabrikat) and deselect (Citroen, Fiat, Ford, Opel, Peugeot, Renault, Tesla)
        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")

        xCitroenEntry = xTreeList.getChild("2")
        xCitroenEntry.executeAction("CLICK", tuple())   #Citroen
        xFiatEntry = xTreeList.getChild("3")
        xFiatEntry.executeAction("CLICK", tuple())   #Fiat
        xFordEntry = xTreeList.getChild("4")
        xFordEntry.executeAction("CLICK", tuple())   #Ford
        xOpelEntry = xTreeList.getChild("6")
        xOpelEntry.executeAction("CLICK", tuple())   #Opel
        xPeugeotEntry = xTreeList.getChild("7")
        xPeugeotEntry.executeAction("CLICK", tuple())   #Peugeot
        xRenaultEntry = xTreeList.getChild("9")
        xRenaultEntry.executeAction("CLICK", tuple())   #Renault
        xTeslaEntry = xTreeList.getChild("10")
        xTeslaEntry.executeAction("CLICK", tuple())   #Tesla

        xOkBtn = xFloatWindow.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.assertFalse(is_row_hidden(calc_doc, 0))
        self.assertFalse(is_row_hidden(calc_doc, 1))
        self.assertTrue(is_row_hidden(calc_doc, 3))

        # 3. open filter of column I (Wert) and deselect 8000 (Values 7000 and 9000 are not shown)
        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "8", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")

        xCitroenEntry = xTreeList.getChild("0")
        xCitroenEntry.executeAction("CLICK", tuple())

        xOkBtn = xFloatWindow.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.assertFalse(is_row_hidden(calc_doc, 0))
        self.assertFalse(is_row_hidden(calc_doc, 1))
        self.assertTrue(is_row_hidden(calc_doc, 9))

        # 4. open filter of column B and select Tesla
        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")
        xTeslaEntry = xTreeList.getChild("4")
        xTeslaEntry.executeAction("CLICK", tuple())   #Tesla

        xOkBtn = xFloatWindow.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())
        self.assertFalse(is_row_hidden(calc_doc, 0))
        self.assertFalse(is_row_hidden(calc_doc, 1))
        self.assertFalse(is_row_hidden(calc_doc, 21))

        # 5. open filter of column I and select 7000 --> 8000 because:new strategy of the filter is implemented
        #(which strings to show and which to hide, when multiple filters are in used).
        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "8", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")

        x8000Entry = xTreeList.getChild("1") # check "8000"
        x8000Entry.executeAction("CLICK", tuple())

        xOkBtn = xFloatWindow.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.assertFalse(is_row_hidden(calc_doc, 0))
        self.assertFalse(is_row_hidden(calc_doc, 1))
        self.assertFalse(is_row_hidden(calc_doc, 7))

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
