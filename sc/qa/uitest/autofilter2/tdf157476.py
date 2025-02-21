# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import is_row_hidden

class tdf157476(UITestCase):

    def test_tdf157476(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf157476.ods")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()

            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(6, len(xTreeList.getChildren()))

            self.assertEqual("(empty)", get_state_as_dict(xTreeList.getChild('0'))['Text'])
            self.assertEqual("0.32", get_state_as_dict(xTreeList.getChild('1'))['Text'])
            self.assertEqual("0.65", get_state_as_dict(xTreeList.getChild('2'))['Text'])
            self.assertEqual("1.33", get_state_as_dict(xTreeList.getChild('3'))['Text'])
            self.assertEqual("2.00", get_state_as_dict(xTreeList.getChild('4'))['Text'])
            self.assertEqual("3.00", get_state_as_dict(xTreeList.getChild('5'))['Text'])

            xFirstEntry = xTreeList.getChild("0")
            xFirstEntry.executeAction("CLICK", tuple())
            xFirstEntry = xTreeList.getChild("2")
            xFirstEntry.executeAction("CLICK", tuple())

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.assertFalse(is_row_hidden(doc, 0))
            self.assertFalse(is_row_hidden(doc, 1))
            self.assertFalse(is_row_hidden(doc, 2))
            self.assertFalse(is_row_hidden(doc, 3))
            self.assertTrue(is_row_hidden(doc, 4))
            self.assertFalse(is_row_hidden(doc, 5))
            self.assertFalse(is_row_hidden(doc, 6))
            self.assertTrue(is_row_hidden(doc, 7))
            self.assertTrue(is_row_hidden(doc, 8))
            self.assertTrue(is_row_hidden(doc, 9))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
