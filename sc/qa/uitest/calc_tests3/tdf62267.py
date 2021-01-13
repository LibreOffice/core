# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from uitest.debug import sleep
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 62267 - Conditional formatting lost after paste special of text, numbers and dates.
#If you have a cell with conditional formatting and you use paste special only inserting only text,
#numbers and dates the formatting is lost. Undo do not recover the conditional formatting.

class tdf62267(UITestCase):

    def test_tdf62267(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf62267.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #* Copy A1, then paste special to C1;
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:Copy")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
        self.ui_test.execute_dialog_through_command(".uno:PasteSpecial")
        xDialog = self.xUITest.getTopFocusWindow()
        #it's the default - text, numbers and dates
        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        #--> Cell formatting should stay as before
        self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog")

        xCondFormatMgr = self.xUITest.getTopFocusWindow()

        # check that we have exactly 1 conditional format
        xList = xCondFormatMgr.getChild("CONTAINER")
        list_state = get_state_as_dict(xList)
        self.assertEqual(list_state['Children'], '1')

        xTreeEntry = xList.getChild('0')
        self.assertEqual(get_state_as_dict(xTreeEntry)["Text"], "A1\tCell value = 1")

        xCancelBtn = xCondFormatMgr.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
