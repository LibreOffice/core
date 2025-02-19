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

class tdf160252(UITestCase):
    def test_tdf160252(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf160252.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            self.assertEqual("1", get_state_as_dict(gridwin)["CurrentColumn"])
            self.assertEqual("10", get_state_as_dict(gridwin)["CurrentRow"])

            with self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog", close_button="") as xCondFormatMgr:

                xContainer = xCondFormatMgr.getChild("CONTAINER")
                self.assertEqual("2", get_state_as_dict(xContainer)["Children"])

                xEditBtn = xCondFormatMgr.getChild("edit")
                with self.ui_test.execute_dialog_through_action(xEditBtn, "CLICK", event_name = "ModelessDialogVisible", close_button="cancel") as xCondFormatDlg:
                    xEdassign = xCondFormatDlg.getChild("edassign")
                    self.assertEqual("A1:A3", get_state_as_dict(xEdassign)["Text"])

                # we need to get a pointer again as the old window has been deleted
                xCondFormatMgr = self.xUITest.getTopFocusWindow()

                xContainer = xCondFormatMgr.getChild("CONTAINER")
                self.assertEqual("2", get_state_as_dict(xContainer)["Children"])

                xEditBtn = xCondFormatMgr.getChild("edit")
                with self.ui_test.execute_dialog_through_action(xEditBtn, "CLICK", event_name = "ModelessDialogVisible", close_button="cancel") as xCondFormatDlg:
                    xEdassign = xCondFormatDlg.getChild("edassign")
                    # Without the fix in place, this test would have failed with
                    # AssertionError: 'A1:A3' != 'B11'
                    self.assertEqual("A1:A3", get_state_as_dict(xEdassign)["Text"])

                # we need to get a pointer again as the old window has been deleted
                xCondFormatMgr = self.xUITest.getTopFocusWindow()

                # close the conditional format manager
                xOKBtn = xCondFormatMgr.getChild("ok")
                self.ui_test.close_dialog_through_button(xOKBtn)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
