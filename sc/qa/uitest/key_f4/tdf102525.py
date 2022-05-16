# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position


# Bug 102525 - F4 breaks array formula
class tdf102525(UITestCase):
    def test_tdf102525_F4_key_array_formula(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf102525.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            #select B1:B4
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B1:B4"}))
            #F4
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
            #verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getFormula(), "{=IF($A$1:$A$4>2;1;2)}")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getValue(), 1)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getValue(), 1)
            #Undo
            self.xUITest.executeCommand(".uno:Undo")
            #verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getFormula(), "{=IF(A1:A4>2;1;2)}")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
