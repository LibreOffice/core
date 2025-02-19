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

class tdf163897(UITestCase):
    def test_tdf163897(self):

        # Use an existing document
        with self.ui_test.load_file(get_url_for_data_file("tdf105544.ods")):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            with self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog", close_button="") as xCondFormatMgr:

                xContainer = xCondFormatMgr.getChild("CONTAINER")
                self.assertEqual("4", get_state_as_dict(xContainer)['Children'])

                xRemoveBtn = xCondFormatMgr.getChild("remove")

                xRemoveBtn.executeAction("CLICK", tuple())
                self.assertEqual("3", get_state_as_dict(xContainer)['Children'])
                xRemoveBtn.executeAction("CLICK", tuple())
                self.assertEqual("2", get_state_as_dict(xContainer)['Children'])
                xRemoveBtn.executeAction("CLICK", tuple())
                self.assertEqual("1", get_state_as_dict(xContainer)['Children'])
                xRemoveBtn.executeAction("CLICK", tuple())
                self.assertEqual("0", get_state_as_dict(xContainer)['Children'])

                xAddBtn = xCondFormatMgr.getChild("add")
                with self.ui_test.execute_dialog_through_action(xAddBtn, "CLICK", event_name="ModelessDialogVisible", close_button="cancel"):
                    pass

                # we need to get a pointer again as the old window has been removed
                xCondFormatMgr = self.xUITest.getTopFocusWindow()

                xContainer = xCondFormatMgr.getChild("CONTAINER")

                # Without the fix in place, this test would have failed with
                # AssertionError: '0' != '4'
                self.assertEqual("0", get_state_as_dict(xContainer)['Children'])

                # close the conditional format manager
                xOKBtn = xCondFormatMgr.getChild("ok")
                self.ui_test.close_dialog_through_button(xOKBtn)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
