# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict

class tdf154543(UITestCase):

   def test_tdf154543_reset_snap_to_grid_parent(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            # Open the paragraph style dialog and unselect the snap to grid checkbox
            with self.ui_test.execute_dialog_through_command(".uno:EditStyle") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")
                xSnapCheckbox = xTabs.getChild("checkCB_SNAP")
                xSnapCheckbox.executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xSnapCheckbox)["Selected"], "false")

            # Open the paragraph style dialog and reset dialog to parent settings
            with self.ui_test.execute_dialog_through_command(".uno:EditStyle") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")
                xSnapCheckbox = xTabs.getChild("checkCB_SNAP")
                xStandardButton = xDialog.getChild("standard")
                xStandardButton.executeAction("CLICK", tuple())
                # Without the fix in place, this test would have failed with
                # AssertionError: 'false' != 'true'
                self.assertEqual(get_state_as_dict(xSnapCheckbox)["Selected"], "true")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
