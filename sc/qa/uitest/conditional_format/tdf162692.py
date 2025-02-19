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

class tdf162692(UITestCase):
    def test_tdf162692(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf162692.ods")):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            self.assertEqual("0", get_state_as_dict(gridwin)["SelectedTable"])

            with self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog", close_button="") as xCondFormatMgr:

                xContainer = xCondFormatMgr.getChild("CONTAINER")
                self.assertEqual("2", get_state_as_dict(xContainer)['Children'])

                self.assertEqual("A1:A500\tCell value is unique ", get_state_as_dict(xContainer.getChild("0"))["Text"])
                self.assertEqual("B1:B500\tCell value contains \"я столб без рамки\"", get_state_as_dict(xContainer.getChild("1"))["Text"])

                xEditBtn = xCondFormatMgr.getChild("edit")
                with self.ui_test.execute_dialog_through_action(xEditBtn, "CLICK", event_name="ModelessDialogVisible", close_button="cancel"):
                    pass

                # we need to get a pointer again as the old window has been removed
                xCondFormatMgr = self.xUITest.getTopFocusWindow()

                xContainer = xCondFormatMgr.getChild("CONTAINER")

                # close the conditional format manager
                xOKBtn = xCondFormatMgr.getChild("ok")
                self.ui_test.close_dialog_through_button(xOKBtn)

            with self.ui_test.execute_dialog_through_command(".uno:JumpToTable") as xDialogGoToSheet:
                xtree_view = xDialogGoToSheet.getChild("treeview")
                xtree_view.getChild('1').executeAction("SELECT", tuple())

            self.assertEqual("1", get_state_as_dict(gridwin)["SelectedTable"])

            with self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog") as xCondFormatMgr:
                xContainer = xCondFormatMgr.getChild("CONTAINER")

                # Without the fix in place, this test would have failed with
                # AssertionError: '1' != '2'
                self.assertEqual("1", get_state_as_dict(xContainer)['Children'])

                self.assertEqual("D1:D400\tCell value is unique ", get_state_as_dict(xContainer.getChild("0"))["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
