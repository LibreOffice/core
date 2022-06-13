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
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from libreoffice.calc.paste_special import reset_default_values

class tdf142932(UITestCase):

    def test_tdf142932(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf142932.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            self.xUITest.executeCommand(".uno:SelectRow")

            self.assertEqual("some comment", get_cell_by_position(calc_doc, 0, 4, 0).Annotation.String)

            self.xUITest.executeCommand(".uno:Copy")

            with self.ui_test.execute_dialog_through_command(".uno:Insert") as xDialog:
                xAfter = xDialog.getChild('after')
                xAfter.executeAction("CLICK", tuple())

            self.assertEqual(get_state_as_dict(gridwin)["SelectedTable"], "1")

            with self.ui_test.execute_dialog_through_command(".uno:PasteSpecial") as xDialog:
                reset_default_values(self, xDialog)

                xSkipEmpty = xDialog.getChild("skip_empty")
                xSkipEmpty.executeAction("CLICK", tuple())
                self.assertEqual('true', get_state_as_dict(xSkipEmpty)['Selected'])

                # Without the fix in place, this test would have crashed here

            self.assertEqual("A", get_cell_by_position(calc_doc, 1, 0, 0).getString())
            self.assertEqual("row", get_cell_by_position(calc_doc, 1, 1, 0).getString())
            self.assertEqual("with", get_cell_by_position(calc_doc, 1, 2, 0).getString())
            self.assertEqual("comment", get_cell_by_position(calc_doc, 1, 3, 0).getString())
            self.assertEqual("for", get_cell_by_position(calc_doc, 1, 4, 0).getString())
            self.assertEqual("a", get_cell_by_position(calc_doc, 1, 5, 0).getString())
            self.assertEqual("certain", get_cell_by_position(calc_doc, 1, 6, 0).getString())
            self.assertEqual("cell", get_cell_by_position(calc_doc, 1, 7, 0).getString())

            self.assertEqual("", get_cell_by_position(calc_doc, 1, 4, 0).Annotation.String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
