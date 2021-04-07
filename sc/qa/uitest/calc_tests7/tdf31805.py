# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 31805 - Deleting a column or row from merged cells

class tdf31805(UITestCase):
    def test_tdf31805_delete_column_merged(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf31805.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #1. open "sample.ods"
        #2. click 'G1'
        #3. Menu 'edit -> Delete Cell - Delete entire Column'
        #expected: Column with cell containing "g" will be deleted,
        #     a merged cell range will remain left row 1

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "G1"}))
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getIsMerged(), True)
        self.xUITest.executeCommand(".uno:DeleteColumns")
        #verify. C5:F11 should be merged
        #isMerged   returns true if this cell is merged with another cell.
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getIsMerged(), True)      #C5
        self.assertEqual(get_cell_by_position(document, 0, 6, 4).getIsMerged(), False)      #G5
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A11"}))
        self.xUITest.executeCommand(".uno:DeleteRows")
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getIsMerged(), True)      #C5
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
        self.xUITest.executeCommand(".uno:DeleteColumns")
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getIsMerged(), True)      #C5
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
