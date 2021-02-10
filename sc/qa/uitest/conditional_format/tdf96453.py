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

class ConditionalFormatDlgTest(UITestCase):

    def test_tdf96453(self):

        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf96453.ods"))

        sheet = get_sheet_from_doc(calc_doc, 0)
        conditional_format_list = get_conditional_format_from_sheet(sheet)
        self.assertEqual(conditional_format_list.getLength(), 2)

        self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog")

        xCondFormatMgr = self.xUITest.getTopFocusWindow()

        # check that we have exactly two conditional formats in the beginning
        xList = xCondFormatMgr.getChild("CONTAINER")
        list_state = get_state_as_dict(xList)
        self.assertEqual(list_state['Children'], '2')

        # remove one conditional format
        xRemoveBtn = xCondFormatMgr.getChild("remove")
        xRemoveBtn.executeAction("CLICK", tuple())

        # check that the table only shows one
        # but the document still contains two
        list_state = get_state_as_dict(xList)
        self.assertEqual(list_state['Children'], '1')

        self.assertEqual(conditional_format_list.getLength(), 2)

        # add a new conditional format through the add button
        xAddBtn = xCondFormatMgr.getChild("add")
        self.ui_test.execute_dialog_through_action(xAddBtn, "CLICK", event_name = "ModelessDialogVisible")

        xCondFormatDlg = self.xUITest.getTopFocusWindow()
        xCondFormatOkBtn = xCondFormatDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xCondFormatOkBtn)

        # we need to get a pointer again as the old window has been deleted
        xCondFormatMgr = self.xUITest.getTopFocusWindow()

        # check again that we now have 2 and not 3 entries in the list
        # and still only 2 conditional formats in the document
        xList = xCondFormatMgr.getChild("CONTAINER")
        list_state = get_state_as_dict(xList)
        self.assertEqual(list_state['Children'], '2')

        self.assertEqual(conditional_format_list.getLength(), 2)

        # close the conditional format manager
        xCancelBtn = xCondFormatMgr.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
