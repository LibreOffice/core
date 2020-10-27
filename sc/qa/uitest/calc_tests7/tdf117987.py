#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

class tdf117987(UITestCase):

    def test_highlight_cell_after_moving_cursor(self):
        self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        colorProperty = mkPropertyValues({"BackgroundColor": 16776960})
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommandWithParameters(".uno:BackgroundColor", colorProperty)
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        self.xUITest.executeCommandWithParameters(".uno:BackgroundColor", colorProperty)

        document = self.ui_test.get_component()
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).CellBackColor, 16776960)
        self.ui_test.close_doc()
