# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos, get_state_as_dict
from com.sun.star.awt.GradientStyle import LINEAR
from com.sun.star.drawing.HatchStyle import SINGLE
from com.sun.star.drawing.BitmapMode import REPEAT
from com.sun.star.drawing.RectanglePoint import MIDDLE_MIDDLE


class WriterPageDialog(UITestCase):

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
                hex(document.StyleFamilies.PageStyles.Standard.BackColor), '0x729fcf')
            self.assertEqual(
                hex(document.StyleFamilies.PageStyles.Standard.FillColor), '0x729fcf')
            self.assertEqual(
                hex(document.StyleFamilies.PageStyles.Standard.FillColor), '0x729fcf')
        elif btn == 'btngradient':
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.FillGradient.Style, LINEAR)
            self.assertEqual(
                hex(document.StyleFamilies.PageStyles.Standard.FillGradient.StartColor), '0xdde8cb')
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
                document.StyleFamilies.PageStyles.Standard.FillGradientName, 'Pastel Bouquet')
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
                document.StyleFamilies.PageStyles.Standard.FillBitmapName, 'Painted White')
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

        with self.ui_test.create_doc_in_start_center("writer"):

            buttons = ['btnbitmap', 'btncolor', 'btngradient', 'btnhatch', 'btnpattern']
            for index, button in enumerate(buttons):

                with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                    tabcontrol = xDialog.getChild("tabcontrol")
                    select_pos(tabcontrol, "2")
                    self.click_button(xDialog, button)

                self.check_default_area(button)

                with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                    tabcontrol = xDialog.getChild("tabcontrol")
                    select_pos(tabcontrol, "2")

                    self.click_button(xDialog, 'btnnone')

                self.check_default_area('btnnone')


    def test_paper_format(self):

        lPaperFormat = ["A6", "A5", "A4", "A3", "B6 (ISO)", "B5 (ISO)", "B4 (ISO)", "Letter",
            "Legal", "Long Bond", "Tabloid", "B6 (JIS)", "B5 (JIS)", "B4 (JIS)", "16 Kai",
            "32 Kai", "Big 32 Kai", "User", "DL Envelope", "C6 Envelope", "C6/5 Envelope",
            "C5 Envelope", "C4 Envelope", "#6¾ Envelope", "#7¾ (Monarch) Envelope",
            "#9 Envelope", "#10 Envelope", "#11 Envelope", "#12 Envelope", "Japanese Postcard"]

        with self.ui_test.create_doc_in_start_center("writer"):

            for i in range(30):
                with self.subTest(i=i):
                    with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                        tabcontrol = xDialog.getChild("tabcontrol")
                        select_pos(tabcontrol, "1")
                        xFormatList = xDialog.getChild("comboPageFormat")
                        select_pos(xFormatList, str(i))

                        self.assertEqual(
                            get_state_as_dict(xFormatList)["SelectEntryText"], lPaperFormat[i])


    def test_orientation(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:

            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.IsLandscape, False)

            with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "1")
                self.click_button(xDialog, 'radiobuttonLandscape')

            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.IsLandscape, True)

            with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "1")
                self.click_button(xDialog, 'radiobuttonPortrait')

            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.IsLandscape, False)


    def test_text_direction(self):

        lTextDirection = ['Left-to-right (horizontal)', 'Right-to-left (horizontal)',
            'Right-to-left (vertical)', 'Left-to-right (vertical)']

        with self.ui_test.create_doc_in_start_center("writer") as document:

            for i in range(4):
                with self.subTest(i=i):
                    with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                        tabcontrol = xDialog.getChild("tabcontrol")
                        select_pos(tabcontrol, "1")

                        xTextDirectionList = xDialog.getChild("comboTextFlowBox")
                        select_pos(xTextDirectionList, str(i))

                        self.assertEqual(
                            get_state_as_dict(xTextDirectionList)["SelectEntryText"], lTextDirection[i])

                    self.assertEqual(
                        document.StyleFamilies.PageStyles.Standard.WritingMode, i)


    def test_cancel_button_page_dialog(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:PageDialog", close_button="cancel") as xDialog:
                pass


# vim: set shiftwidth=4 softtabstop=4 expandtab:
