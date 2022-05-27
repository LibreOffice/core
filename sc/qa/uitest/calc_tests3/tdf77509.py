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
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf77509(UITestCase):
    def test_tdf77509_consolidate(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf77509.xls")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #1. Open attachment: Consolidate-test.xls
            #2. Select any empty cell, eg. cell D1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
            #3. Tab: Data > Consolidate
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataConsolidate") as xDialog:

                xfunc = xDialog.getChild("func")
                xeddataarea = xDialog.getChild("eddataarea")
                xadd = xDialog.getChild("add")
                xbyrow = xDialog.getChild("byrow")

                select_by_text(xfunc, "Sum")
                #4. Source data ranges: $Sheet1.$A$1:$B$7
                #5. Click 'Add' so that ranges appear in "Consolidation ranges"
                xeddataarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$A$1:$B$7"}))
                xadd.executeAction("CLICK", tuple())

                xConsAreas = xDialog.getChild("consareas")
                self.assertEqual(1, len(xConsAreas.getChildren()))
                self.assertEqual("$Sheet1.$A$1:$B$7", get_state_as_dict(xConsAreas.getChild("0"))['Text'])

                #6. Click 'Options' > check 'Row labels' > click OK
                xbyrow.executeAction("CLICK", tuple())

            #verify
            self.assertEqual("A 1", get_cell_by_position(calc_doc, 0, 3, 0).getString())
            self.assertEqual("AB 1", get_cell_by_position(calc_doc, 0, 3, 1).getString())
            self.assertEqual("AB 12", get_cell_by_position(calc_doc, 0, 3, 2).getString())
            self.assertEqual("AB 123", get_cell_by_position(calc_doc, 0, 3, 3).getString())
            self.assertEqual("ABC 1", get_cell_by_position(calc_doc, 0, 3, 4).getString())

            self.assertEqual(1, get_cell_by_position(calc_doc, 0, 4, 0).getValue())
            self.assertEqual(2, get_cell_by_position(calc_doc, 0, 4, 1).getValue())
            self.assertEqual(2, get_cell_by_position(calc_doc, 0, 4, 2).getValue())
            self.assertEqual(1, get_cell_by_position(calc_doc, 0, 4, 3).getValue())
            self.assertEqual(1, get_cell_by_position(calc_doc, 0, 4, 4).getValue())

            self.xUITest.executeCommand(".uno:Undo")

            self.assertEqual("", get_cell_by_position(calc_doc, 0, 3, 0).getString())
            self.assertEqual("", get_cell_by_position(calc_doc, 0, 3, 1).getString())
            self.assertEqual("", get_cell_by_position(calc_doc, 0, 3, 2).getString())
            self.assertEqual("", get_cell_by_position(calc_doc, 0, 3, 3).getString())
            self.assertEqual("", get_cell_by_position(calc_doc, 0, 3, 4).getString())

            self.assertEqual(0, get_cell_by_position(calc_doc, 0, 4, 0).getValue())
            self.assertEqual(0, get_cell_by_position(calc_doc, 0, 4, 1).getValue())
            self.assertEqual(0, get_cell_by_position(calc_doc, 0, 4, 2).getValue())
            self.assertEqual(0, get_cell_by_position(calc_doc, 0, 4, 3).getValue())
            self.assertEqual(0, get_cell_by_position(calc_doc, 0, 4, 4).getValue())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
