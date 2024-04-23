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

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text

class formatCharacter(UITestCase):

   def test_format_character_tab_font(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")

                # xNoteBook = xDialog.getChild("nbWestern") //western notebook is always active
                xSizeFont = xDialog.getChild("cbWestSize")
                xLangFont = xDialog.getChild("cbWestLanguage")
                xSizeFont.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xSizeFont.executeAction("TYPE", mkPropertyValues({"TEXT":"18"}))
                #set font size 18
                select_pos(xLangFont, "0")

                xNoteBook = xDialog.getChild("nbCJKCTL")
                select_pos(xNoteBook, "0")
                xSizeFontEast = xDialog.getChild("cbCJKSize")
                xLangFontEast = xDialog.getChild("cbCJKLanguage")
                xSizeFontEast.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xSizeFontEast.executeAction("TYPE", mkPropertyValues({"TEXT":"18"}))    #set font size 18
                select_pos(xLangFontEast, "0")

                select_pos(xNoteBook, "1")
                xSizeFontCTL = xDialog.getChild("cbCTLSize")
                xLangFontCTL = xDialog.getChild("cbCTLLanguage")
                xSizeFontCTL.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xSizeFontCTL.executeAction("TYPE", mkPropertyValues({"TEXT":"18"}))    #set font size 18
                select_pos(xLangFontCTL, "0")

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog", close_button="cancel") as xDialog:
                xSizeFont = xDialog.getChild("cbWestSize")
                self.assertEqual(get_state_as_dict(xSizeFont)["Text"], "18 pt")
                xLangFont = xDialog.getChild("cbWestLanguage")
                self.assertEqual(get_state_as_dict(xLangFont)["Text"], "[None]")

                xNoteBook = xDialog.getChild("nbCJKCTL")
                select_pos(xNoteBook, "0")
                xSizeFontEast = xDialog.getChild("cbCJKSize")
                self.assertEqual(get_state_as_dict(xSizeFontEast)["Text"], "18 pt")
                xLangFontEast = xDialog.getChild("cbCJKLanguage")
                self.assertEqual(get_state_as_dict(xLangFontEast)["Text"], "[None]")

                select_pos(xNoteBook, "1")
                xSizeFontCTL = xDialog.getChild("cbCTLSize")
                self.assertEqual(get_state_as_dict(xSizeFontCTL)["Text"], "18 pt")
                xLangFontCTL = xDialog.getChild("cbCTLLanguage")
                self.assertEqual(get_state_as_dict(xLangFontCTL)["Text"], "[None]")

   def test_format_character_tab_font_effects(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")

                xEffects = xDialog.getChild("effectslb")
                xRelief = xDialog.getChild("relieflb")
                xHidden = xDialog.getChild("hiddencb")
                xOverline = xDialog.getChild("overlinelb")
                xStrikeout = xDialog.getChild("strikeoutlb")
                xUnderline = xDialog.getChild("underlinelb")
                xEmphasis = xDialog.getChild("emphasislb")
                xPosition = xDialog.getChild("positionlb")

                select_pos(xEffects, "1")
                select_pos(xRelief, "1")
                xHidden.executeAction("CLICK", tuple())
                select_pos(xOverline, "1")
                select_pos(xStrikeout, "1")
                select_pos(xUnderline, "1")
                select_pos(xEmphasis, "1")
                select_pos(xPosition, "1")


            with self.ui_test.execute_dialog_through_command(".uno:FontDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")

                xEffects = xDialog.getChild("effectslb")
                xRelief = xDialog.getChild("relieflb")
                xHidden = xDialog.getChild("hiddencb")
                xOverline = xDialog.getChild("overlinelb")
                xStrikeout = xDialog.getChild("strikeoutlb")
                xUnderline = xDialog.getChild("underlinelb")
                xEmphasis = xDialog.getChild("emphasislb")
                xPosition = xDialog.getChild("positionlb")

                self.assertEqual(get_state_as_dict(xEffects)["SelectEntryText"], "UPPERCASE")
                self.assertEqual(get_state_as_dict(xRelief)["SelectEntryText"], "Embossed")
                self.assertEqual(get_state_as_dict(xHidden)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xOverline)["SelectEntryText"], "Single")
                self.assertEqual(get_state_as_dict(xStrikeout)["SelectEntryText"], "Single")
                self.assertEqual(get_state_as_dict(xUnderline)["SelectEntryText"], "Single")
                self.assertEqual(get_state_as_dict(xEmphasis)["SelectEntryText"], "Dot")
                self.assertEqual(get_state_as_dict(xPosition)["SelectEntryText"], "Below text")

   def test_format_character_tab_asian_layout(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "3")

                xTwolines = xDialog.getChild("twolines")
                xTwolines.executeAction("CLICK", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:FontDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "3")
                xTwolines = xDialog.getChild("twolines")

                self.assertEqual(get_state_as_dict(xTwolines)["Selected"], "true")

   def test_format_character_tab_position(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")

                xSuperscript = xDialog.getChild("superscript")
                xRelFontSize = xDialog.getChild("fontsizesb")
                x90deg = xDialog.getChild("90deg")
                xScalewidth = xDialog.getChild("scalewidthsb")
                xKerning = xDialog.getChild("kerningsb")
                xPairKerning = xDialog.getChild("pairkerning")
                xFitToLine = xDialog.getChild("fittoline")
                xNoHyphenation = xDialog.getChild("nohyphenation")
                self.assertEqual(get_state_as_dict(xNoHyphenation)["Selected"], "false")

                xSuperscript.executeAction("CLICK", tuple())
                xRelFontSize.executeAction("UP", tuple())
                x90deg.executeAction("CLICK", tuple())
                xScalewidth.executeAction("UP", tuple())
                xKerning.executeAction("UP", tuple())
                xPairKerning.executeAction("CLICK", tuple())
                xFitToLine.executeAction("CLICK", tuple())
                xNoHyphenation.executeAction("CLICK", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:FontDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")
                xSuperscript = xDialog.getChild("superscript")
                xRelFontSize = xDialog.getChild("fontsizesb")
                x90deg = xDialog.getChild("90deg")
                xScalewidth = xDialog.getChild("scalewidthsb")
                xKerning = xDialog.getChild("kerningsb")
                xPairKerning = xDialog.getChild("pairkerning")
                xFitToLine = xDialog.getChild("fittoline")
                xNoHyphenation = xDialog.getChild("nohyphenation")

                self.assertEqual(get_state_as_dict(xSuperscript)["Checked"], "true")
                self.assertEqual(get_state_as_dict(x90deg)["Checked"], "true")
                self.assertEqual(get_state_as_dict(xScalewidth)["Text"], "100%")
                self.assertEqual(get_state_as_dict(xKerning)["Text"], "0.1 pt")
                self.assertEqual(get_state_as_dict(xPairKerning)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xFitToLine)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xNoHyphenation)["Selected"], "true")

   def test_format_character_tab_position_scalewidthsb(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")

                xScalewidth = xDialog.getChild("scalewidthsb")
                xScalewidth.executeAction("UP", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:FontDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")
                xScalewidth = xDialog.getChild("scalewidthsb")
                self.assertEqual(get_state_as_dict(xScalewidth)["Text"], "101%")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
