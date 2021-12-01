# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import is_row_hidden
from uitest.uihelper.common import get_url_for_data_file

#Bug 130770 - Autofilter not updated on data change (automatically or manually)

class tdf130770(UITestCase):
    def test_tdf130770_autofilter(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf130770.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            # 1. open attached file and check initial state
            self.assertFalse(is_row_hidden(calc_doc, 0))
            self.assertFalse(is_row_hidden(calc_doc, 1))
            self.assertTrue(is_row_hidden(calc_doc, 2))
            self.assertFalse(is_row_hidden(calc_doc, 3))
            self.assertFalse(is_row_hidden(calc_doc, 4))

            # 2. open filter of column A  and cancel
            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()

            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

            self.assertFalse(is_row_hidden(calc_doc, 0))
            self.assertFalse(is_row_hidden(calc_doc, 1))
            self.assertTrue(is_row_hidden(calc_doc, 2))
            self.assertFalse(is_row_hidden(calc_doc, 3))
            self.assertFalse(is_row_hidden(calc_doc, 4))

            # 3. open filter of column A  and just click OK
            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.assertFalse(is_row_hidden(calc_doc, 0))
            self.assertFalse(is_row_hidden(calc_doc, 1))
            self.assertTrue(is_row_hidden(calc_doc, 2))
            self.assertFalse(is_row_hidden(calc_doc, 3))
            self.assertTrue(is_row_hidden(calc_doc, 4)) # filtered out

# vim: set shiftwidth=4 softtabstop=4 expandtab:
