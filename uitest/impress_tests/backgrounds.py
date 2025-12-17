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
from com.sun.star.drawing.HatchStyle import SINGLE
from com.sun.star.drawing.BitmapMode import REPEAT
from com.sun.star.drawing.RectanglePoint import MIDDLE_MIDDLE

class ImpressBackgrounds(UITestCase):

    def checkDefaultBackground(self, tab):
        document = self.ui_test.get_component()
        if tab == 'lbnone':
            self.assertEqual(document.DrawPages[0].Background, None)
        elif tab == 'lbcolor':
            self.assertEqual(
              hex(document.DrawPages[0].Background.FillColor), '0x729fcf')
            self.assertEqual(
              hex(document.DrawPages[0].Background.FillColor), '0x729fcf')
        elif tab == 'lbgradient':
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
            self.assertEqual(
              document.DrawPages[0].Background.FillGradientName, 'Pastel Bouquet')
        elif tab == 'lbhatch':
            self.assertEqual(
              document.DrawPages[0].Background.FillHatch.Style, SINGLE )
            self.assertEqual(
              document.DrawPages[0].Background.FillHatch.Color, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillHatch.Distance, 102)
            self.assertEqual(
              document.DrawPages[0].Background.FillHatch.Angle, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillHatchName, 'Black 0 Degrees')
        elif tab == 'lbbitmap':
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapMode, REPEAT)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapPositionOffsetX, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapPositionOffsetY, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapRectanglePoint, MIDDLE_MIDDLE)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapStretch, False)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapTile, True)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapOffsetX, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapOffsetY, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapLogicalSize, True)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapSizeX, 2540)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapSizeY, 2540)
            self.assertEqual(document.DrawPages[0].Background.FillBitmapName, 'Painted White')
        elif tab == 'lbpattern':
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapMode, REPEAT)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapPositionOffsetX, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapPositionOffsetY, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapRectanglePoint, MIDDLE_MIDDLE)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapStretch, True)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapTile, True)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapOffsetX, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapOffsetY, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapLogicalSize, True)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapSizeX, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapSizeY, 0)
            self.assertEqual(
              document.DrawPages[0].Background.FillBitmapName, '5 Percent')


    def test_background_dialog(self):

        with self.ui_test.create_doc_in_start_center("impress"):

            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            tab_mapping = {
                'lbbitmap' : "4",
                'lbcolor' : "1",
                'lbgradient' : "2",
                'lbhatch' : "3",
                'lbpattern' : "5"
            }
            for tab, index in tab_mapping.items():
                with self.ui_test.execute_dialog_through_command(".uno:PageSetup") as xPageSetupDlg:

                    tabcontrol = xPageSetupDlg.getChild("tabcontrol")
                    select_pos(tabcontrol, "1")

                    xFillTypeTabs = xPageSetupDlg.getChild("nbFillType")
                    select_pos(xFillTypeTabs, index)

                    # tdf#100024: Without the fix in place, this test would have crashed here
                    # changing the background to bitmap

                self.checkDefaultBackground(tab)

                with self.ui_test.execute_dialog_through_command(".uno:PageSetup") as xPageSetupDlg:

                    tabcontrol = xPageSetupDlg.getChild("tabcontrol")
                    select_pos(tabcontrol, "1")

                    xFillTypeTabs = xPageSetupDlg.getChild("nbFillType")
                    select_pos(xFillTypeTabs, "0")


                self.checkDefaultBackground('lbnone')


# vim: set shiftwidth=4 softtabstop=4 expandtab:
