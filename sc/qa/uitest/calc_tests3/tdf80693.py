# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_by_text
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 80693 - EDITING: fill cell with random numbers with min = max
class tdf80693(UITestCase):
    def test_tdf80693_random_numbers_min_max(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))
        self.ui_test.execute_modeless_dialog_through_command(".uno:RandomNumberGeneratorDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xcellrangeedit = xDialog.getChild("cell-range-edit")
        xdistributioncombo = xDialog.getChild("distribution-combo")
        xparameter1spin = xDialog.getChild("parameter1-spin")
        xparameter2spin = xDialog.getChild("parameter2-spin")
        xenableseedcheck = xDialog.getChild("enable-seed-check")
        xseedspin = xDialog.getChild("seed-spin")
        xenableroundingcheck = xDialog.getChild("enable-rounding-check")
        xdecimalplacesspin = xDialog.getChild("decimal-places-spin")

        xcellrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$A$1:$A$2"}))
        select_by_text(xdistributioncombo, "Uniform")

        xparameter1spin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xparameter1spin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xparameter1spin.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))
        xparameter2spin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xparameter2spin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xparameter2spin.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue() , 2)
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(bool(get_cell_by_position(document, 0, 0, 0).getString() ), False)
        self.assertEqual(bool(get_cell_by_position(document, 0, 0, 1).getString() ), False)
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
