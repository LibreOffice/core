# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 101165 - Crashing on a filter selection, every time

class tdf101165(UITestCase):
    def test_tdf101165_autofilter(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf101165.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xAll = xFloatWindow.getChild("toggle_all")
        xAll.executeAction("CLICK", tuple())

        self.assertEqual(get_cell_by_position(document, 1, 0, 1).getValue(), 6494)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab: