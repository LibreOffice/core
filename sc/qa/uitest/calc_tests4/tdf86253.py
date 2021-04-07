# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf86253(UITestCase):

    def test_tdf86253(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf86253.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #* Copy A1, then paste special only "formatting" to C1:C17;
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:Copy")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C1:C17"}))
        self.ui_test.execute_dialog_through_command(".uno:PasteSpecial")
        xDialog = self.xUITest.getTopFocusWindow()

        xtext = xDialog.getChild("text")
        xnumbers = xDialog.getChild("numbers")
        xdatetime = xDialog.getChild("datetime")
        xformats = xDialog.getChild("formats")

        xtext.executeAction("CLICK", tuple())
        xnumbers.executeAction("CLICK", tuple())
        xdatetime.executeAction("CLICK", tuple())
        xformats.executeAction("CLICK", tuple())

        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        #--> Cell formatting for C1:C17 is changed. But, if you go to "Format - Conditional Formatting - Manage",
        #you will see that a new formatting condition is created with the range "C1:C6", rather than "C1:C17". This is wrong behavior.
        self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog")

        xCondFormatMgr = self.xUITest.getTopFocusWindow()

        # check that we have exactly 1 conditional format and range is C1:C17
        xList = xCondFormatMgr.getChild("CONTAINER")
        list_state = get_state_as_dict(xList)
        self.assertEqual(list_state['Children'], '1')

        xTreeEntry = xList.getChild('0')
        self.assertEqual(get_state_as_dict(xTreeEntry)["Text"], "A1:A6,C1:C17\tCell value >= 0")

        xCancelBtn = xCondFormatMgr.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
