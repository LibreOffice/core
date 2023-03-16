# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell

from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position


class keyF4(UITestCase):
    def test_f4(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #enter data
            enter_text_to_cell(gridwin, "A1", "1")
            enter_text_to_cell(gridwin, "A2", "2")
            enter_text_to_cell(gridwin, "A3", "3")
            enter_text_to_cell(gridwin, "B1", "=A1")
            enter_text_to_cell(gridwin, "B2", "=A2")
            enter_text_to_cell(gridwin, "B3", "=A3")
            #select B1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
            #F4
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
            #verify
            self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 1)
            self.assertEqual(get_cell_by_position(document, 0, 1, 0).getFormula(), "=$A$1")
            #F4
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
            #verify
            self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 1)
            self.assertEqual(get_cell_by_position(document, 0, 1, 0).getFormula(), "=A$1")
            #F4
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
            #verify
            self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 1)
            self.assertEqual(get_cell_by_position(document, 0, 1, 0).getFormula(), "=$A1")

            #non continuous select
            #enter data
            enter_text_to_cell(gridwin, "C1", "=A1")
            enter_text_to_cell(gridwin, "C2", "=A2")
            enter_text_to_cell(gridwin, "C3", "=A3")

            #select C1 and C3
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C3", "EXTEND":"1"}))
            #F4
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
            #verify
            self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 1)
            self.assertEqual(get_cell_by_position(document, 0, 2, 0).getFormula(), "=$A$1")
            self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(document, 0, 2, 2).getFormula(), "=$A$3")
            #F4
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
            #verify
            self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 1)
            self.assertEqual(get_cell_by_position(document, 0, 2, 0).getFormula(), "=A$1")
            self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(document, 0, 2, 2).getFormula(), "=A$3")
            #F4
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
            #verify
            self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 1)
            self.assertEqual(get_cell_by_position(document, 0, 2, 0).getFormula(), "=$A1")
            self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(document, 0, 2, 2).getFormula(), "=$A3")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
