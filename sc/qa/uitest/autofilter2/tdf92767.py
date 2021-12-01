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
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf92767(UITestCase):

    def test_tdf92767(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf92767.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            for i in range(0,25):
                self.assertFalse(is_row_hidden(calc_doc, i))

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xTreeList = xFloatWindow.getChild("check_tree_box")
            xFirstEntry = xTreeList.getChild("0")
            self.assertEqual('2015', get_state_as_dict(xFirstEntry)["Text"])
            self.assertEqual('7', get_state_as_dict(xFirstEntry)["Children"])

            # Deselect all the options but the last one
            for i in range(6):
                xChild = xFirstEntry.getChild(str(i))
                xChild.executeAction("CLICK", tuple())
                self.assertEqual('false', get_state_as_dict(xChild)['IsChecked'])

            xLastChild = xFirstEntry.getChild('6')
            self.assertEqual('true', get_state_as_dict(xLastChild)['IsChecked'])
            self.assertEqual('July', get_state_as_dict(xLastChild)['Text'])

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            for i in range(1,22):
                self.assertTrue(is_row_hidden(calc_doc, i))

            # Without the fix in place, this test would have failed here
            self.assertFalse(is_row_hidden(calc_doc, 23))
            self.assertFalse(is_row_hidden(calc_doc, 24))
            self.assertFalse(is_row_hidden(calc_doc, 25))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
