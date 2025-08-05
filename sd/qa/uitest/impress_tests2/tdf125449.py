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
from com.sun.star.awt.GradientStyle import LINEAR

class tdf125449(UITestCase):

    def test_tdf125449(self):

        with self.ui_test.create_doc_in_start_center("impress") as document:

            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            with self.ui_test.execute_dialog_through_command(".uno:PageSetup") as xPageSetupDlg:

                tabcontrol = xPageSetupDlg.getChild("tabcontrol")
                select_pos(tabcontrol, "1")

                xBtn = xPageSetupDlg.getChild('btngradient')
                xBtn.executeAction("CLICK", tuple())

                xAngle = xPageSetupDlg.getChild('anglemtr')
                xAngle.executeAction("UP", tuple())


            self.assertEqual(
              document.DrawPages[0].Background.FillGradient.Style, LINEAR)
            self.assertEqual(
              hex(document.DrawPages[0].Background.FillGradient.StartColor), '0xdde8cb')
            self.assertEqual(
              document.DrawPages[0].Background.FillGradient.Angle, 300)
            self.assertEqual(
              document.DrawPages[0].Background.FillGradient.Border, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillGradient.XOffset, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillGradient.YOffset, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillGradient.StartIntensity, 100)
            self.assertEqual(
              document.DrawPages[0].Background.FillGradient.EndIntensity, 100)

            # FillGradientName to match the first(default) preset's name
            self.assertEqual(
              document.DrawPages[0].Background.FillGradientName, 'Pastel Bouquet')


# vim: set shiftwidth=4 softtabstop=4 expandtab:
