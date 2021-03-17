# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf77509(UITestCase):
    def test_tdf77509_consolidate(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf77509.xls"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #1. Open attachment: Consolidate-test.xls
        #2. Select any empty cell, eg. cell D1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
        #3. Tab: Data > Consolidate
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataConsolidate")
        xDialog = self.xUITest.getTopFocusWindow()
        xfunc = xDialog.getChild("func")
        xeddataarea = xDialog.getChild("eddataarea")
        xadd = xDialog.getChild("add")
        xbyrow = xDialog.getChild("byrow")
        xbycol = xDialog.getChild("bycol")
        xeddestarea = xDialog.getChild("eddestarea")

        select_by_text(xfunc, "Sum")
        #4. Source data ranges: $Sheet1.$A$1:$B$7
        #5. Click 'Add' so that ranges appear in "Consolidation ranges"
        xeddataarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$A$1:$B$7"}))
        xadd.executeAction("CLICK", tuple())
        #6. Click 'Options' > check 'Row labels' > click OK
        xbyrow.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getString(), "A 1")
        self.assertEqual(get_cell_by_position(document, 0, 3, 1).getString(), "AB 1")
        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getString(), "AB 12")
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getString(), "AB 123")
        self.assertEqual(get_cell_by_position(document, 0, 3, 4).getString(), "ABC 1")

        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 4, 4).getValue(), 1)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
