# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

class tdf99386(UITestCase):

    def test_td99386_undo_merged_cell_needs_hard_recalculate(self):

        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf99386.ods"))

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B1"})) #select cells
        self.xUITest.executeCommand(".uno:ToggleMergeCells") # merge cells
        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "This")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
