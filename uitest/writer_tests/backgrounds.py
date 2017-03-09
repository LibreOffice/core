#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from com.sun.star.uno import RuntimeException
from com.sun.star.awt.GradientStyle import LINEAR
from com.sun.star.drawing.HatchStyle import SINGLE
from com.sun.star.drawing.BitmapMode import REPEAT
from com.sun.star.drawing.RectanglePoint import MIDDLE_MIDDLE

class WriterBackgrounds(UITestCase):

    def checkDefaultBackground(self, btn):
        document = self.ui_test.get_component()
        if btn == 'btnnone':            
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.BackColor, -1)
        elif btn == 'btncolor':
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.BackColor, 7512015)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillColor, 7512015)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillColor, 7512015)
        elif btn == 'btngradient':
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillGradient.Style, LINEAR)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillGradient.StartColor, 9101876)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillGradient.Angle, 300)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillGradient.Border, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillGradient.XOffset, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillGradient.YOffset, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillGradient.StartIntensity, 100)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillGradient.EndIntensity, 100)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillGradientName, 'Tango Green')
        elif btn == 'btnhatch':
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillHatch.Style, SINGLE )
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillHatch.Color, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillHatch.Distance, 102)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillHatch.Angle, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillHatchName, 'Black 0 Degrees')
        elif btn == 'btnbitmap':
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapMode, REPEAT)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapPositionOffsetX, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapPositionOffsetY, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapRectanglePoint, MIDDLE_MIDDLE)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapStretch, False)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapTile, True)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapOffsetX, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapOffsetY, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapLogicalSize, True)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapSizeX, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapSizeY, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapName, 'Sky')
        elif btn == 'btnpattern':
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapMode, REPEAT)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapPositionOffsetX, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapPositionOffsetY, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapRectanglePoint, MIDDLE_MIDDLE)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapStretch, False)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapTile, True)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapOffsetX, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapOffsetY, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapLogicalSize, True)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapSizeX, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapSizeY, 0)
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillBitmapName, '5 Percent')


    def test_background_dialog(self):

        self.ui_test.create_doc_in_start_center("writer")

        buttons = ['btnbitmap', 'btncolor', 'btngradient', 'btnhatch', 'btnpattern']
        for index, button in enumerate(buttons):
            self.ui_test.execute_dialog_through_command(".uno:PageStyleName")

            xPageStyleDlg = self.xUITest.getTopFocusWindow()
            tabcontrol = xPageStyleDlg.getChild("tabcontrol")
            select_pos(tabcontrol, "2")

            xBtn = xPageStyleDlg.getChild(button)
            xBtn.executeAction("CLICK", tuple())

            xOkBtn = xPageStyleDlg.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.checkDefaultBackground(button)
        
            self.ui_test.execute_dialog_through_command(".uno:PageStyleName")

            xPageStyleDlg = self.xUITest.getTopFocusWindow()
            tabcontrol = xPageStyleDlg.getChild("tabcontrol")
            select_pos(tabcontrol, "2")

            xBtn = xPageStyleDlg.getChild('btnnone')
            xBtn.executeAction("CLICK", tuple())

            xOkBtn = xPageStyleDlg.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.checkDefaultBackground('btnnone')
            
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
