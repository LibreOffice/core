#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from com.sun.star.awt.GradientStyle import LINEAR
from com.sun.star.drawing.HatchStyle import SINGLE
from com.sun.star.drawing.BitmapMode import REPEAT
from com.sun.star.drawing.RectanglePoint import MIDDLE_MIDDLE

class ImpressBackgrounds(UITestCase):

    def checkDefaultBackground(self, btn):
        document = self.ui_test.get_component()
        if btn == 'btnnone':
            self.assertEqual(document.DrawPages.getByIndex(0).Background, None)
        elif btn == 'btncolor':
            self.assertEqual(
              hex(document.DrawPages.getByIndex(0).Background.FillColor), '0x729fcf')
            self.assertEqual(
              hex(document.DrawPages.getByIndex(0).Background.FillColor), '0x729fcf')
        elif btn == 'btngradient':
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillGradient.Style, LINEAR)
            self.assertEqual(
              hex(document.DrawPages.getByIndex(0).Background.FillGradient.StartColor), '0xdde8cb')
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillGradient.Angle, 300)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillGradient.Border, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillGradient.XOffset, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillGradient.YOffset, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillGradient.StartIntensity, 100)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillGradient.EndIntensity, 100)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillGradientName, 'Pastel Bouquet')
        elif btn == 'btnhatch':
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillHatch.Style, SINGLE )
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillHatch.Color, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillHatch.Distance, 102)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillHatch.Angle, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillHatchName, 'Black 0 Degrees')
        elif btn == 'btnbitmap':
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapMode, REPEAT)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapPositionOffsetX, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapPositionOffsetY, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapRectanglePoint, MIDDLE_MIDDLE)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapStretch, False)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapTile, True)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapOffsetX, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapOffsetY, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapLogicalSize, True)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapSizeX, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapSizeY, 0)
            self.assertEqual(document.DrawPages.getByIndex(0).Background.FillBitmapName, 'Painted White')
        elif btn == 'btnpattern':
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapMode, REPEAT)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapPositionOffsetX, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapPositionOffsetY, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapRectanglePoint, MIDDLE_MIDDLE)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapStretch, True)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapTile, True)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapOffsetX, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapOffsetY, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapLogicalSize, True)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapSizeX, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapSizeY, 0)
            self.assertEqual(
              document.DrawPages.getByIndex(0).Background.FillBitmapName, '5 Percent')


    def test_background_dialog(self):

        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        buttons = ['btnbitmap', 'btncolor', 'btngradient', 'btnhatch', 'btnpattern']
        for index, button in enumerate(buttons):
            self.ui_test.execute_dialog_through_command(".uno:PageSetup")

            xPageSetupDlg = self.xUITest.getTopFocusWindow()
            tabcontrol = xPageSetupDlg.getChild("tabcontrol")
            select_pos(tabcontrol, "1")

            xBtn = xPageSetupDlg.getChild(button)
            xBtn.executeAction("CLICK", tuple())

            # tdf#100024: Without the fix in place, this test would have crashed here
            # changing the background to bitmap
            xOkBtn = xPageSetupDlg.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.checkDefaultBackground(button)

            self.ui_test.execute_dialog_through_command(".uno:PageSetup")

            xPageSetupDlg = self.xUITest.getTopFocusWindow()
            tabcontrol = xPageSetupDlg.getChild("tabcontrol")
            select_pos(tabcontrol, "1")

            xBtn = xPageSetupDlg.getChild('btnnone')
            xBtn.executeAction("CLICK", tuple())

            xOkBtn = xPageSetupDlg.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.checkDefaultBackground('btnnone')

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
