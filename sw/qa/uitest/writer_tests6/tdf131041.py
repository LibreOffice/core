#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos, get_state_as_dict

class tdf131041(UITestCase):

    def test_run(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "4") #header

                xHeaderOn = xDialog.getChild("checkHeaderOn")
                xMoreBtn = xDialog.getChild("buttonMore")

                self.assertEqual(get_state_as_dict(xHeaderOn)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xMoreBtn)["Enabled"], "false")

                xHeaderOn.executeAction("CLICK", tuple())

                self.assertEqual(get_state_as_dict(xHeaderOn)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xMoreBtn)["Enabled"], "true")

                with self.ui_test.execute_dialog_through_action(xMoreBtn, "CLICK") as xBorderDlg:


                    #modify any property
                    bottomft = xBorderDlg.getChild("bottommf")
                    bottomft.executeAction("UP", tuple())

                    #it would crash here



# vim: set shiftwidth=4 softtabstop=4 expandtab:
