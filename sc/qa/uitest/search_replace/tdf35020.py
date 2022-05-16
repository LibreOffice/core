# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 35020 - Find and Replace changes case of sheet name in formulas
class tdf35020(UITestCase):
   def test_tdf39959_find_replace_all_sheets(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf35020.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"X6"}))
                replaceterm = xDialog.getChild("replaceterm")
                replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"A6"})) #replace textbox
                allsheets = xDialog.getChild("allsheets")
                allsheets.executeAction("CLICK", tuple())
                calcsearchin = xDialog.getChild("calcsearchin")
                self.assertEqual("Formulas", get_state_as_dict(calcsearchin)['SelectEntryText'])
                replaceall = xDialog.getChild("replaceall")
                replaceall.executeAction("CLICK", tuple())

            #verify Sheet1.A13 A14 = 2
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 12).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 13).getValue(), 2)
            #Undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 12).getValue(), 1)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 13).getValue(), 1)
# vim: set shiftwidth=4 softtabstop=4 expandtab:
