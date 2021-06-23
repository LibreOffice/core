# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text

class formatCharacter(UITestCase):

   def test_format_character_tab_font(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")

        xSizeFont = xDialog.getChild("westsizelb-cjk")
        xSizeFontEast = xDialog.getChild("eastsizelb")
        xSizeFontCTL = xDialog.getChild("ctlsizelb")
        xLangFont = xDialog.getChild("westlanglb-cjk")
        xLangFontEast = xDialog.getChild("eastlanglb")
        xLangFontCTL = xDialog.getChild("ctllanglb")

        xSizeFont.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xSizeFont.executeAction("TYPE", mkPropertyValues({"TEXT":"18"}))    #set font size 18
        xSizeFontEast.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xSizeFontEast.executeAction("TYPE", mkPropertyValues({"TEXT":"18"}))    #set font size 18
        xSizeFontCTL.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xSizeFontCTL.executeAction("TYPE", mkPropertyValues({"TEXT":"18"}))    #set font size 18
        select_pos(xLangFont, "0")
        select_pos(xLangFontEast, "0")
        select_pos(xLangFontCTL, "0")
        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xSizeFont = xDialog.getChild("westsizelb-cjk")
        xSizeFontEast = xDialog.getChild("eastsizelb")
        xSizeFontCTL = xDialog.getChild("ctlsizelb")
        xLangFont = xDialog.getChild("westlanglb-cjk")
        xLangFontEast = xDialog.getChild("eastlanglb")
        xLangFontCTL = xDialog.getChild("ctllanglb")

        self.assertEqual(get_state_as_dict(xSizeFont)["Text"], "18 pt")
        self.assertEqual(get_state_as_dict(xSizeFontEast)["Text"], "18 pt")
        self.assertEqual(get_state_as_dict(xSizeFontCTL)["Text"], "18 pt") #check font size
        self.assertEqual(get_state_as_dict(xLangFont)["Text"], "[None]")
        self.assertEqual(get_state_as_dict(xLangFontEast)["SelectEntryText"], "[None]")
        self.assertEqual(get_state_as_dict(xLangFontCTL)["SelectEntryText"], "[None]")

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()

   def test_format_character_tab_font_effects(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()

   def test_format_character_tab_hyperlink(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "4")

        xURL = xDialog.getChild("urled")
        xURL.executeAction("TYPE", mkPropertyValues({"TEXT":"libreoffice.org"}))
        xTexted = xDialog.getChild("texted")
        xTexted.executeAction("TYPE", mkPropertyValues({"TEXT":"LibreOffice"}))
        xName = xDialog.getChild("nameed")
        xName.executeAction("TYPE", mkPropertyValues({"TEXT":"hyperlink"}))

        xVisited = xDialog.getChild("visitedlb")
        select_by_text(xVisited, "Bullets")
        xUnVisited = xDialog.getChild("unvisitedlb")
        select_by_text(xUnVisited, "Bullets")

        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.xUITest.executeCommand(".uno:GoLeft")
        self.assertEqual(document.Text.String[0:11], "LibreOffice")

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "4")
        xURL = xDialog.getChild("urled")
        xTexted = xDialog.getChild("texted")
        xName = xDialog.getChild("nameed")
        xVisited = xDialog.getChild("visitedlb")
        xUnVisited = xDialog.getChild("unvisitedlb")

        self.assertEqual(get_state_as_dict(xURL)["Text"], "http://libreoffice.org/")
        self.assertEqual(get_state_as_dict(xTexted)["Text"], "LibreOffice")
        self.assertEqual(get_state_as_dict(xName)["Text"], "hyperlink")
        self.assertEqual(get_state_as_dict(xVisited)["SelectEntryText"], "Bullets")
        self.assertEqual(get_state_as_dict(xUnVisited)["SelectEntryText"], "Bullets")

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)
        self.ui_test.close_doc()

   def test_format_character_tab_asian_layout(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "3")

        xTwolines = xDialog.getChild("twolines")
        xTwolines.executeAction("CLICK", tuple())

        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "3")
        xTwolines = xDialog.getChild("twolines")

        self.assertEqual(get_state_as_dict(xTwolines)["Selected"], "true")
        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)
        self.ui_test.close_doc()

   def test_format_character_tab_position(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "2")

        xSuperscript = xDialog.getChild("superscript")
        xRelFontSize = xDialog.getChild("fontsizesb")
        x90deg = xDialog.getChild("90deg")
        xScalewidth = xDialog.getChild("scalewidthsb")
        xKerning = xDialog.getChild("kerningsb")
        xPairKerning = xDialog.getChild("pairkerning")
        xFitToLine = xDialog.getChild("fittoline")

        xSuperscript.executeAction("CLICK", tuple())
        xRelFontSize.executeAction("UP", tuple())
        x90deg.executeAction("CLICK", tuple())
        xScalewidth.executeAction("UP", tuple())
        xKerning.executeAction("UP", tuple())
        xPairKerning.executeAction("CLICK", tuple())
        xFitToLine.executeAction("CLICK", tuple())

        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "2")
        xSuperscript = xDialog.getChild("superscript")
        xRelFontSize = xDialog.getChild("fontsizesb")
        x90deg = xDialog.getChild("90deg")
        xScalewidth = xDialog.getChild("scalewidthsb")
        xKerning = xDialog.getChild("kerningsb")
        xPairKerning = xDialog.getChild("pairkerning")
        xFitToLine = xDialog.getChild("fittoline")

        self.assertEqual(get_state_as_dict(xSuperscript)["Checked"], "true")
        self.assertEqual(get_state_as_dict(x90deg)["Checked"], "true")
        self.assertEqual(get_state_as_dict(xScalewidth)["Text"], "100%")
        self.assertEqual(get_state_as_dict(xKerning)["Text"], "0.1 pt")
        self.assertEqual(get_state_as_dict(xPairKerning)["Selected"], "false")
        self.assertEqual(get_state_as_dict(xFitToLine)["Selected"], "true")

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()

   def test_format_character_tab_position_scalewidthsb(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "2")

        xScalewidth = xDialog.getChild("scalewidthsb")
        xScalewidth.executeAction("UP", tuple())

        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "2")
        xScalewidth = xDialog.getChild("scalewidthsb")
        self.assertEqual(get_state_as_dict(xScalewidth)["Text"], "101%")

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
