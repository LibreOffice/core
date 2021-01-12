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
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text

#Bug 124818 - CRASH: selecting all and switching spreadsheet

class tdf124818(UITestCase):
   def test_tdf124818_crash_select_all_and_switch_spreadsheet(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf124818.xls"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    #Select all ( Ctrl + A );Go to sheet Graph2;Select all;Crash
    self.xUITest.executeCommand(".uno:SelectAll")
    self.xUITest.executeCommand(".uno:JumpToPrevTable")
    self.xUITest.executeCommand(".uno:SelectAll")

    #verify; no crashes
    self.assertEqual(document.Sheets.getCount(), 3)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
