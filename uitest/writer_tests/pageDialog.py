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
from uitest.uihelper.common import get_state_as_dict

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


    def dtest_area_tab(self):

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

    def test_page_tab(self):

        dict_size = { 0:(9255,9255), 1:(10474,10474), 2:(12232,12232), 3:(16838,16838), 4:(9822,9822),
            5:(11268,11268), 6:(14173,14173), 7:(12399,12399), 8:(12399,12399), 9:(12399,12399),
            10:(15840,15840), 11:(9907,9907), 12:(11438,11438), 13:(14570,14570), 14:(11494,11494),
            15:(9964,9964), 16:(10247,10247), 17:(10247,10247), 18:(9397,9397), 19:(9510,9510),
            20:(9510,9510), 21:(10871,10871), 22:(12983,12983), 23:(8889,8889), 24:(9069,9069),
            25:(9069,9069), 26:(9249,9249), 27:(9519,9519)}

        self.ui_test.create_doc_in_start_center("writer")

        for i in range(28):

            xDialog = self.launch_dialog_and_select_tab(1)

            xFormatList = xDialog.getChild("comboPageFormat")
            select_pos(xFormatList, str(i))

            self.click_button(xDialog, 'ok')

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            state = get_state_as_dict(xWriterEdit)
            print(dict_size[i])
            self.assertEqual(dict_size[i][0], int(state['DocWidth']))
            self.assertEqual(dict_size[i][1], int(state['DocWidth']))
            document = self.ui_test.get_component()

        self.ui_test.close_doc()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
