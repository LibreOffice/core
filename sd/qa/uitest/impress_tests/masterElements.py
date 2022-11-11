# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict


class masterElements(UITestCase):

   def test_master_elements(self):
        with self.ui_test.create_doc_in_start_center("impress"):
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            with self.ui_test.execute_dialog_through_command(".uno:MasterLayouts") as xDialog:

                xpagenumber = xDialog.getChild("pagenumber")
                xfooter = xDialog.getChild("footer")
                datetime = xDialog.getChild("datetime")
                xpagenumber.executeAction("CLICK", tuple())
                xfooter.executeAction("CLICK", tuple())
                datetime.executeAction("CLICK", tuple())


            #verify
            with self.ui_test.execute_dialog_through_command(".uno:MasterLayouts") as xDialog:
                xfooter = xDialog.getChild("footer")
                datetime = xDialog.getChild("datetime")
                xpagenumber = xDialog.getChild("pagenumber")

                self.assertEqual(get_state_as_dict(xfooter)["Selected"], "false")
                self.assertEqual(get_state_as_dict(datetime)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xpagenumber)["Selected"], "false")



# vim: set shiftwidth=4 softtabstop=4 expandtab:
