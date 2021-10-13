# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf144940(UITestCase):

    def test_tdf144940_header(self):

        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "4")
                xFirstHeaderSameBtn = xDialog.getChild("checkSameFP")
                xFirstHeaderSameBtn.executeAction("CLICK", tuple())

                # FIXME: apply changes
                xOK = xDialog.getChild("ok")
                self.ui_test.close_dialog_through_button(xOK)

            self.xUITest.executeCommand(".uno:Reload")

            with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog") as xDialog2:
                xTabs = xDialog2.getChild("tabcontrol")
                select_pos(xTabs, "4")
                xHeaderOnBtn = xDialog2.getChild("checkHeaderOn")
                self.assertEqual(get_state_as_dict(xHeaderOnBtn)["Selected"], "true")
                xFirstHeaderSameBtn = xDialog2.getChild("checkSameFP")
                self.assertEqual(get_state_as_dict(xFirstHeaderSameBtn)["Selected"], "true")

    def test_tdf144940_footer(self):

        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")
                xFirstFooterSameBtn = xDialog.getChild("checkSameFP")
                xFirstFooterSameBtn.executeAction("CLICK", tuple())

                # FIXME: apply changes
                xOK = xDialog.getChild("ok")
                self.ui_test.close_dialog_through_button(xOK)

            self.xUITest.executeCommand(".uno:Reload")

            with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog") as xDialog2:
                xTabs = xDialog2.getChild("tabcontrol")
                select_pos(xTabs, "5")
                xFooterOnBtn = xDialog2.getChild("checkFooterOn")
                self.assertEqual(get_state_as_dict(xFooterOnBtn)["Selected"], "true")
                xFirstFooterSameBtn = xDialog2.getChild("checkSameFP")
                self.assertEqual(get_state_as_dict(xFirstHeaderSameBtn)["Selected"], "true")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
