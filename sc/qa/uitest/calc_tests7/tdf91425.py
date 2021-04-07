# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position

#Bug 91425 - CRASH - Calc Insert Columns Left

class tdf91425(UITestCase):
   def test_tdf91425_insert_columns_left(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf91425.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #1) Open test file provided in report
        #2) Move mouse pointer over Column A header, the right mouse button click
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #3) Insert columns left
        self.xUITest.executeCommand(".uno:InsertColumnsBefore")
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getString(), "C")
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getString(), "C")
        #redo
        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getString(), "C")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
