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
from com.sun.star.drawing.HatchStyle import SINGLE

class tdf137729(UITestCase):

    def test_tdf137729(self):

        with self.ui_test.create_doc_in_start_center("impress") as document:

            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            with self.ui_test.execute_dialog_through_command(".uno:PageSetup") as xPageSetupDlg:

                tabcontrol = xPageSetupDlg.getChild("tabcontrol")
                select_pos(tabcontrol, "1")

                xBtn = xPageSetupDlg.getChild('btnhatch')
                xBtn.executeAction("CLICK", tuple())

                xDistance = xPageSetupDlg.getChild('distancemtr')
                xDistance.executeAction("UP", tuple())


            self.assertEqual(
              document.DrawPages[0].Background.FillHatch.Style, SINGLE )
            self.assertEqual(
              document.DrawPages[0].Background.FillHatch.Color, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillHatch.Distance, 102)
            self.assertEqual(
              document.DrawPages[0].Background.FillHatch.Angle, 0)

            # FillHatchName to match the first(default) preset's name
            self.assertEqual(
              document.DrawPages[0].Background.FillHatchName, 'Black 0 Degrees')


# vim: set shiftwidth=4 softtabstop=4 expandtab:
