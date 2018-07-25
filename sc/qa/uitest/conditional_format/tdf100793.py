# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import os
from uitest.uihelper.common import get_state_as_dict
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from libreoffice.uno.propertyvalue import mkPropertyValues

#import org.libreoffice.unotest
#import pathlib
#def get_url_for_data_file(file_name):
#    return get_srcdir_url() + "/uitest/calc_tests/data/" + file_name

from uitest.path import get_srcdir_url
def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sc/qa/uitest/calc_tests/data/" + file_name

#Bug 100793 - FORMATTING - conditional formatting gets corrupted upon copy/paste/insert
class tdf100793(UITestCase):
    def test_tdf100793(self):

        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf100793.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        sheet = get_sheet_from_doc(calc_doc, 2)
        conditional_format_list = get_conditional_format_from_sheet(sheet)
        self.assertEqual(conditional_format_list.getLength(), 1)
        #2) Go to sheet "plan"
        gridwin.executeAction("SELECT", mkPropertyValues({"TABLE": "2"}))
        #3) Open the conditional formatting management dialog-> The indicated range should be G18:K29,F18,F20:F29
        self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog")

        xCondFormatMgr = self.xUITest.getTopFocusWindow()

        # check that we have exactly one conditional formats in the beginning
        xList = xCondFormatMgr.getChild("CONTAINER")
        list_state = get_state_as_dict(xList)
        self.assertEqual(list_state['Children'], '1')

        #3) Click Edit & try to change to intended one, F18:K33
        xEditBtn = xCondFormatMgr.getChild("edit")
        self.ui_test.execute_dialog_through_action(xEditBtn, "CLICK", event_name = "ModelessDialogVisible")
        xCondFormatDlg = self.xUITest.getTopFocusWindow()

        #modify textbox
        xedassign = xCondFormatDlg.getChild("edassign")
        #go at the beginning
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "END"}))
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "HOME"}))
        # 11x right
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        xedassign.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        #now I'm at F18 - write :K33
        xedassign.executeAction("TYPE", mkPropertyValues({"TEXT":":K33"}))

        xCondFormatOkBtn = xCondFormatDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xCondFormatOkBtn)

        # we need to get a pointer again as the old window has been deleted
        xCondFormatMgr = self.xUITest.getTopFocusWindow()

        # check again that we still have 1 entry in the list
        # and still only 1 conditional format in the document
        xList = xCondFormatMgr.getChild("CONTAINER")
        list_state = get_state_as_dict(xList)
        self.assertEqual(list_state['Children'], '1')

        self.assertEqual(conditional_format_list.getLength(), 1)

        # close the conditional format manager
        xOKBtn = xCondFormatMgr.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #verify - reopen, check range
        self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog")
        xCondFormatMgr = self.xUITest.getTopFocusWindow()
        xEditBtn = xCondFormatMgr.getChild("edit")
        self.ui_test.execute_dialog_through_action(xEditBtn, "CLICK", event_name = "ModelessDialogVisible")
        xCondFormatDlg = self.xUITest.getTopFocusWindow()
        xedassign = xCondFormatDlg.getChild("edassign")
        self.assertEqual(get_state_as_dict(xedassign)["Text"], "G18:K29,F18:K33,F20:F29")
        xCondFormatOkBtn = xCondFormatDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xCondFormatOkBtn)
        xCondFormatMgr = self.xUITest.getTopFocusWindow()
        xCancelBtn = xCondFormatMgr.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
