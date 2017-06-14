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

from libreoffice.uno.propertyvalue import mkPropertyValues

import unittest

class WriterPageDialog(UITestCase):

    def launch_dialog_and_select_tab(self, tab):
        self.ui_test.execute_dialog_through_command(".uno:PageDialog")

        xDialog = self.xUITest.getTopFocusWindow()
        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, str(tab))

        return xDialog

    def click_button(self, dialog, button):
        xButton = dialog.getChild(button)
        xButton.executeAction("CLICK", tuple())

    def check_default_area(self, btn):
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

    def test_area_tab(self):

        self.ui_test.create_doc_in_start_center("writer")

        buttons = ['btnbitmap', 'btncolor', 'btngradient', 'btnhatch', 'btnpattern']
        for index, button in enumerate(buttons):

            xDialog = self.launch_dialog_and_select_tab(2)

            self.click_button(xDialog, button)

            self.click_button(xDialog, 'ok')

            self.check_default_area(button)

            xDialog = self.launch_dialog_and_select_tab(2)

            self.click_button(xDialog, 'btnnone')

            self.click_button(xDialog, 'ok')

            self.check_default_area('btnnone')

        self.ui_test.close_doc()

    @unittest.skip("Currently broken")
    def test_page_tab(self):

        size_list = [[10500,14801], [14801,21001], [21001,29700], [29700,42000],
            [12501,17600], [17600,25000], [25000,35301], [21590,27940],
            [21590,35560], [21590,33020], [27940,43180], [12801,18200],
            [18200,25700], [25700,36400], [18399,26000], [13000,18399],
            [14000,20301], [14000,20301], [11000,21999], [11400,16200],
            [11400,22901], [16200,22901], [22901,32401], [9208,16510],
            [9843,19050], [9843,22543], [10478,24130], [11430,26353]]

        self.ui_test.create_doc_in_start_center("writer")

        for i in range(28):
            with self.subTest(i=i):
                xDialog = self.launch_dialog_and_select_tab(1)

                xFormatList = xDialog.getChild("comboPageFormat")
                select_pos(xFormatList, str(i))

                self.click_button(xDialog, 'ok')

                document = self.ui_test.get_component()

                self.assertEqual(
                    document.StyleFamilies.PageStyles.Standard.Width, size_list[i][0])
                self.assertEqual(
                    document.StyleFamilies.PageStyles.Standard.Height, size_list[i][1])

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
