#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from com.sun.star.drawing.HatchStyle import SINGLE
from uitest.uihelper.common import get_state_as_dict

class tdf137729(UITestCase):

    def test_tdf137729(self):

        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.execute_dialog_through_command(".uno:PageSetup")

        xPageSetupDlg = self.xUITest.getTopFocusWindow()
        tabcontrol = xPageSetupDlg.getChild("tabcontrol")
        select_pos(tabcontrol, "1")

        xBtn = xPageSetupDlg.getChild('btnhatch')
        xBtn.executeAction("CLICK", tuple())

        xDistance = xPageSetupDlg.getChild('distancemtr')
        xDistance.executeAction("UP", tuple())

        xOkBtn = xPageSetupDlg.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        document = self.ui_test.get_component()
        self.assertEqual(
          document.DrawPages.getByIndex(0).Background.FillHatch.Style, SINGLE )
        self.assertEqual(
          document.DrawPages.getByIndex(0).Background.FillHatch.Color, 0)
        self.assertEqual(
          document.DrawPages.getByIndex(0).Background.FillHatch.Distance, 152)
        self.assertEqual(
          document.DrawPages.getByIndex(0).Background.FillHatch.Angle, 0)

        # Without the patch in place, this test would have failed with
        # AssertionError: '' != 'hatch'
        self.assertEqual(
          document.DrawPages.getByIndex(0).Background.FillHatchName, 'hatch')

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
