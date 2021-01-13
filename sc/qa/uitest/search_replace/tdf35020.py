# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text

#Bug 35020 - Find and Replace changes case of sheet name in formulas

class tdf35020(UITestCase):
   def test_tdf39959_find_replace_all_sheets(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf35020.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        searchterm = xDialog.getChild("searchterm")
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"X6"}))
        replaceterm = xDialog.getChild("replaceterm")
        replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"A6"})) #replace textbox
        allsheets = xDialog.getChild("allsheets")
        allsheets.executeAction("CLICK", tuple())
        calcsearchin = xDialog.getChild("calcsearchin")
        props = {"TEXT": "Formulas"}
        actionProps = mkPropertyValues(props)
        calcsearchin.executeAction("SELECT", actionProps)
        replaceall = xDialog.getChild("replaceall")
        replaceall.executeAction("CLICK", tuple())
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        #verify Sheet1.A13 A14 = 2
        self.assertEqual(get_cell_by_position(document, 0, 0, 12).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 13).getValue(), 2)
        #Undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 0, 12).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 0, 13).getValue(), 1)
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
