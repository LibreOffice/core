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

class Tdf144296(UITestCase):
    def test_tdf144296_automatic_cell_direction(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")
            xCell = get_cell_by_position(document, 0, 0, 0)

            # Before input, the cell's direction should be inherited
            self.assertEqual(4, xCell.getPropertyValue("WritingMode"))

            # Type a single RTL character into the cell and dismiss
            xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            xGridWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "א"}))
            xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

            # After typing, the cell should now be automatically set right-to-left
            self.assertEqual(1, xCell.getPropertyValue("WritingMode"))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
