# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

class tdf140754(UITestCase):

    def test_tdf140754(self):

        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf140754.ods"))

        #Make sure 'multi-threaded calculation' is enabled
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xCalcEntry = xPages.getChild('3')
        xCalcEntry.executeAction("EXPAND", tuple())
        xCalcCalculateEntry = xCalcEntry.getChild('3')
        xCalcCalculateEntry.executeAction("SELECT", tuple())

        self.assertEqual('true', get_state_as_dict(xDialogOpt.getChild('threadingenabled'))["Selected"])

        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        document = self.ui_test.get_component()

        self.assertEqual(0, get_cell_by_position(document, 0, 0, 30).getValue())
        self.assertEqual(0, get_cell_by_position(document, 0, 0, 82).getValue())
        self.assertEqual(1, get_cell_by_position(document, 0, 0, 238).getValue())
        self.assertEqual(28, get_cell_by_position(document, 0, 0, 265).getValue())
        self.assertEqual(28, get_cell_by_position(document, 0, 0, 1370).getValue())

        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "2", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xAll = xFloatWindow.getChild("toggle_all")
        xAll.executeAction("CLICK", tuple())

        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xList = xCheckListMenu.getChild("check_list_box")
        self.assertEqual(25, len(xList.getChildren()))

        # Without the fix in place, this test would have crashed here
        xOkBtn = xFloatWindow.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.assertEqual(0, get_cell_by_position(document, 0, 0, 30).getValue())
        self.assertEqual(1, get_cell_by_position(document, 0, 0, 82).getValue())
        self.assertEqual(39, get_cell_by_position(document, 0, 0, 238).getValue())
        self.assertEqual(66, get_cell_by_position(document, 0, 0, 265).getValue())
        self.assertEqual(282, get_cell_by_position(document, 0, 0, 1370).getValue())

        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "6", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xAll = xFloatWindow.getChild("toggle_all")
        xAll.executeAction("CLICK", tuple())

        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xList = xCheckListMenu.getChild("check_list_box")
        self.assertEqual(10, len(xList.getChildren()))

        xOkBtn = xFloatWindow.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.assertEqual(1, get_cell_by_position(document, 0, 0, 30).getValue())
        self.assertEqual(11, get_cell_by_position(document, 0, 0, 82).getValue())
        self.assertEqual(69, get_cell_by_position(document, 0, 0, 238).getValue())
        self.assertEqual(96, get_cell_by_position(document, 0, 0, 265).getValue())
        self.assertEqual(411, get_cell_by_position(document, 0, 0, 1370).getValue())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
