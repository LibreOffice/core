# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from libreoffice.calc.document import get_column
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.common import get_url_for_data_file

class tdf81351(UITestCase):

    def test_tdf81351(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf81351.ods"))
        xTopWindow = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        gridwin = xTopWindow.getChild("grid_window")

        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), ".uno:Paste")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:F5"}))

        self.xUITest.executeCommand(".uno:SortAscending")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B7"}))
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), ".uno:Bold")

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), ".uno:Paste")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B7"}))

    def test_tdf81351_comment4(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf81351.ods"))
        xTopWindow = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()

        gridwin = xTopWindow.getChild("grid_window")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:F5"}))


        self.xUITest.executeCommand(".uno:SortAscending")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), ".uno:Bold")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B7"}))

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), ".uno:Paste")

        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), ".uno:Bold")

    def test_sort_descending(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf81351.ods"))
        xTopWindow = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()

        gridwin = xTopWindow.getChild("grid_window")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:F5"}))

        self.xUITest.executeCommand(".uno:SortDescending")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), ".uno:Undo")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B7"}))

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), ".uno:Paste")

        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), ".uno:Undo")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
