#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos, get_state_as_dict
from uitest.uihelper.common import change_measurement_unit
#Bug 128431 - Synchronize padding in header borders it is not working

class tdf128431(UITestCase):

    def test_tdf128431_pageFormat_sync_padding(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            change_measurement_unit(self, "Centimeter")

            with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "6") #borders

                sync = xDialog.getChild("sync")
                bottomft = xDialog.getChild("bottommf")
                topft = xDialog.getChild("topmf")
                rightft = xDialog.getChild("rightmf")
                leftft = xDialog.getChild("leftmf")

                self.assertEqual(get_state_as_dict(sync)["Selected"], "true")
                bottomft.executeAction("UP", tuple())
                self.assertEqual(get_state_as_dict(bottomft)["Text"], "0.10 cm")
                self.assertEqual(get_state_as_dict(topft)["Text"], "0.10 cm")
                self.assertEqual(get_state_as_dict(rightft)["Text"], "0.10 cm")
                self.assertEqual(get_state_as_dict(leftft)["Text"], "0.10 cm")



# vim: set shiftwidth=4 softtabstop=4 expandtab:
