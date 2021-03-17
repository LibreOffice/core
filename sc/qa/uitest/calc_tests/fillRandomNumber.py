# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#randomnumbergenerator.ui
class fillRandomNumber(UITestCase):
    def test_fill_random_number(self):
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
        select_by_text(xdistributioncombo, "Uniform Integer")

        xparameter1spin.executeAction("UP", tuple())
        xparameter2spin.executeAction("UP", tuple())
        xenableseedcheck.executeAction("CLICK", tuple())
        xseedspin.executeAction("UP", tuple())
        xenableroundingcheck.executeAction("CLICK", tuple())
        xdecimalplacesspin.executeAction("UP", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(bool(get_cell_by_position(document, 0, 0, 0).getString() ), True)
        self.assertEqual(bool(get_cell_by_position(document, 0, 0, 1).getString() ), True)
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(bool(get_cell_by_position(document, 0, 0, 0).getString() ), False)
        self.assertEqual(bool(get_cell_by_position(document, 0, 0, 1).getString() ), False)
        #close dialog without doing anything
        self.ui_test.execute_modeless_dialog_through_command(".uno:RandomNumberGeneratorDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)
        self.assertEqual(bool(get_cell_by_position(document, 0, 0, 0).getString() ), False)
        self.assertEqual(bool(get_cell_by_position(document, 0, 0, 1).getString() ), False)
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
