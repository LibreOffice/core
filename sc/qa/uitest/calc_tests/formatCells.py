# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class formatCell(UITestCase):
    def test_format_cell_numbers_tab(self):
        #numberingformatpage.ui
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")  #tab Numbers
        xliststore1 = xDialog.getChild("categorylb")  #1st list / Category
        xliststore2 = xDialog.getChild("formatlb")  #2nd list / Format
        xdecimalsed = xDialog.getChild("decimalsed")
        xleadzerosed = xDialog.getChild("leadzerosed")
        xnegnumred = xDialog.getChild("negnumred")
        xthousands = xDialog.getChild("thousands")
        xlanguagelb = xDialog.getChild("languagelb")
        xformatted = xDialog.getChild("formatted")
        #language
        select_by_text(xlanguagelb, "English (USA)")
        #other properties
        xdecimalsed.executeAction("UP", tuple())
        xleadzerosed.executeAction("UP", tuple())
        xnegnumred.executeAction("CLICK", tuple())
        xthousands.executeAction("CLICK", tuple())
        #format   #,#00.0;[RED]-#,#00.0
        self.assertEqual(get_state_as_dict(xformatted)["Text"], "#,#00.0;[RED]-#,#00.0")
        #save
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)
        #verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")  #tab Numbers
        xliststore1 = xDialog.getChild("categorylb")  #1st list / Category
        xliststore2 = xDialog.getChild("formatlb")  #2nd list / Format
        xdecimalsed = xDialog.getChild("decimalsed")
        xleadzerosed = xDialog.getChild("leadzerosed")
        xnegnumred = xDialog.getChild("negnumred")
        xthousands = xDialog.getChild("thousands")
        xlanguagelb = xDialog.getChild("languagelb")
        xformatted = xDialog.getChild("formatted")

        self.assertEqual(get_state_as_dict(xliststore1)["SelectEntryText"], "Number")
        self.assertEqual(get_state_as_dict(xlanguagelb)["SelectEntryText"], "English (USA)")
        self.assertEqual(get_state_as_dict(xdecimalsed)["Text"], "1")
        self.assertEqual(get_state_as_dict(xleadzerosed)["Text"], "2")
        self.assertEqual(get_state_as_dict(xnegnumred)["Selected"], "true")
        self.assertEqual(get_state_as_dict(xthousands)["Selected"], "true")
        self.assertEqual(get_state_as_dict(xformatted)["Text"], "#,#00.0;[RED]-#,#00.0")
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        self.ui_test.close_doc()

    def test_format_cell_font_tab(self):
        #numberingformatpage.ui
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")  #tab Font
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

        #Verify - select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")  #tab Font
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

    def test_format_cell_font_effects_tab(self):
        #numberingformatpage.ui
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "2")  #tab Font Effects
        xRelief = xDialog.getChild("relieflb")
        xOverline = xDialog.getChild("overlinelb")
        xStrikeout = xDialog.getChild("strikeoutlb")
        xUnderline = xDialog.getChild("underlinelb")
        xEmphasis = xDialog.getChild("emphasislb")
        xPosition = xDialog.getChild("positionlb")

        select_pos(xRelief, "1")
        select_pos(xOverline, "1")
        select_pos(xStrikeout, "1")
        select_pos(xUnderline, "1")
        select_pos(xEmphasis, "1")
        select_pos(xPosition, "1")

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())
        #Verify- select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "2")

        xRelief = xDialog.getChild("relieflb")
        xOverline = xDialog.getChild("overlinelb")
        xStrikeout = xDialog.getChild("strikeoutlb")
        xUnderline = xDialog.getChild("underlinelb")
        xEmphasis = xDialog.getChild("emphasislb")
        xPosition = xDialog.getChild("positionlb")

        self.assertEqual(get_state_as_dict(xRelief)["SelectEntryText"], "Embossed")
        self.assertEqual(get_state_as_dict(xOverline)["SelectEntryText"], "Single")
        self.assertEqual(get_state_as_dict(xStrikeout)["SelectEntryText"], "Single")
        self.assertEqual(get_state_as_dict(xUnderline)["SelectEntryText"], "Single")
        self.assertEqual(get_state_as_dict(xEmphasis)["SelectEntryText"], "Dot")
        self.assertEqual(get_state_as_dict(xPosition)["SelectEntryText"], "Below text")

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()

    def test_format_cell_alignment_tab(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "3")  #tab Alignment
        comboboxHorzAlign = xDialog.getChild("comboboxHorzAlign")
        xspinIndentFrom = xDialog.getChild("spinIndentFrom")
        xcomboboxVertAlign = xDialog.getChild("comboboxVertAlign")
        xcheckVertStack = xDialog.getChild("checkVertStack")
        xcheckWrapTextAuto = xDialog.getChild("checkWrapTextAuto")
        xcheckHyphActive = xDialog.getChild("checkHyphActive")
        xcomboTextDirBox = xDialog.getChild("comboTextDirBox")

        select_by_text(comboboxHorzAlign, "Left")
        xspinIndentFrom.executeAction("UP", tuple())
        indentVal = get_state_as_dict(xspinIndentFrom)["Text"]
        select_by_text(xcomboboxVertAlign, "Top")
        xcheckVertStack.executeAction("CLICK", tuple())
        xcheckWrapTextAuto.executeAction("CLICK", tuple())
        xcheckHyphActive.executeAction("CLICK", tuple())
        select_by_text(xcomboTextDirBox, "Left-to-right (LTR)")
        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())
        #Verify- select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "3")
        comboboxHorzAlign = xDialog.getChild("comboboxHorzAlign")
        xspinIndentFrom = xDialog.getChild("spinIndentFrom")
        xcomboboxVertAlign = xDialog.getChild("comboboxVertAlign")
        xcheckVertStack = xDialog.getChild("checkVertStack")
        xcheckWrapTextAuto = xDialog.getChild("checkWrapTextAuto")
        xcheckHyphActive = xDialog.getChild("checkHyphActive")
        xcomboTextDirBox = xDialog.getChild("comboTextDirBox")

        self.assertEqual(get_state_as_dict(comboboxHorzAlign)["SelectEntryText"], "Left")
        self.assertEqual(get_state_as_dict(xspinIndentFrom)["Text"] == indentVal, True)
        self.assertEqual(get_state_as_dict(xcomboboxVertAlign)["SelectEntryText"], "Top")
        self.assertEqual(get_state_as_dict(xcheckVertStack)["Selected"], "true")
        self.assertEqual(get_state_as_dict(xcheckWrapTextAuto)["Selected"], "true")
        self.assertEqual(get_state_as_dict(xcheckHyphActive)["Selected"], "true")
        self.assertEqual(get_state_as_dict(xcomboTextDirBox)["SelectEntryText"], "Left-to-right (LTR)")
        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_format_cell_asian_typography_tab(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "4")  #tab Asian typography
        xcheckForbidList = xDialog.getChild("checkForbidList")
        xcheckForbidList.executeAction("CLICK", tuple())
        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())
        #Verify- select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "4")
        xcheckForbidList = xDialog.getChild("checkForbidList")
        self.assertEqual(get_state_as_dict(xcheckForbidList)["Selected"], "true")
        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_minimal_border_width(self):
        #borderpage.ui, tdf#137790
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #set points pt measurement
        change_measurement_unit(self, "Point")

        #select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "5")  #tab Borders

        linewidthmf = xDialog.getChild("linewidthmf")
        xLineSet = xDialog.getChild('lineset')

        # check line-width for default solid line
        self.assertEqual('0', get_state_as_dict(xLineSet)['SelectedItemPos'])
        widthVal = get_state_as_dict(linewidthmf)["Text"]
        self.assertEqual(widthVal, '0.75 pt')

        # set line style to "double" (minimal width is taken)
        xLineSet.executeAction("CHOOSE", mkPropertyValues({"POS": '16'}))
        widthVal = get_state_as_dict(linewidthmf)["Text"]
        self.assertEqual(widthVal, '1.10 pt')

        # set line style to "solid"
        xLineSet.executeAction("CHOOSE", mkPropertyValues({"POS": "1"}))
        widthVal = get_state_as_dict(linewidthmf)["Text"]
        self.assertEqual(widthVal, '0.75 pt')

        # make custom line width
        linewidthmf.executeAction("UP", tuple())
        linewidthmf.executeAction("UP", tuple())
        linewidthmf.executeAction("UP", tuple())
        widthVal = get_state_as_dict(linewidthmf)["Text"]
        self.assertEqual(widthVal, '1.50 pt')

        # set line style to "double" (minimal width is not taken)
        xLineSet.executeAction("CHOOSE", mkPropertyValues({"POS": "8"}))
        widthVal = get_state_as_dict(linewidthmf)["Text"]
        self.assertEqual(widthVal, '1.50 pt')

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

    def test_format_cell_borders_tab(self):
        #borderpage.ui
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #set points pt measurement
        change_measurement_unit(self, "Point")

        #select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "5")  #tab Borders
        xsync = xDialog.getChild("sync")
        xleftmf = xDialog.getChild("leftmf")
        xrightmf = xDialog.getChild("rightmf")
        xtopmf = xDialog.getChild("topmf")
        xbottommf = xDialog.getChild("bottommf")

        xsync.executeAction("CLICK", tuple())  #uncheck Synchronize
        xleftmf.executeAction("UP", tuple())
        xrightmf.executeAction("UP", tuple())
        xrightmf.executeAction("UP", tuple())
        xtopmf.executeAction("UP", tuple())
        xtopmf.executeAction("UP", tuple())
        xtopmf.executeAction("UP", tuple())
        xbottommf.executeAction("UP", tuple())
        xbottommf.executeAction("UP", tuple())
        xbottommf.executeAction("UP", tuple())
        xbottommf.executeAction("UP", tuple())

        leftVal = get_state_as_dict(xleftmf)["Text"]
        rightVal = get_state_as_dict(xrightmf)["Text"]
        topVal = get_state_as_dict(xtopmf)["Text"]
        bottomVal = get_state_as_dict(xbottommf)["Text"]

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Verify select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "5")  #tab Borders
        xsync = xDialog.getChild("sync")
        xleftmf = xDialog.getChild("leftmf")
        xrightmf = xDialog.getChild("rightmf")
        xtopmf = xDialog.getChild("topmf")
        xbottommf = xDialog.getChild("bottommf")

        self.assertEqual(get_state_as_dict(xsync)["Selected"], "false")
        self.assertEqual(get_state_as_dict(xleftmf)["Text"] == leftVal, True)
        self.assertEqual(get_state_as_dict(xrightmf)["Text"] == rightVal, True)
        self.assertEqual(get_state_as_dict(xtopmf)["Text"] == topVal, True)
        self.assertEqual(get_state_as_dict(xbottommf)["Text"] == bottomVal, True)

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

    def test_format_cell_cell_protection_tab(self):
        #cellprotectionpage.ui
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "6")  #tab Cell protection
        xcheckHideFormula = xDialog.getChild("checkHideFormula")
        xcheckHideAll = xDialog.getChild("checkHideAll")
        xcheckHidePrinting = xDialog.getChild("checkHidePrinting")

        xcheckHideFormula.executeAction("CLICK", tuple())
        xcheckHideAll.executeAction("CLICK", tuple())
        xcheckHidePrinting.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Verify select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "6")  #tab Borders
        xcheckHideFormula = xDialog.getChild("checkHideFormula")
        xcheckHideAll = xDialog.getChild("checkHideAll")
        xcheckHidePrinting = xDialog.getChild("checkHidePrinting")

        self.assertEqual(get_state_as_dict(xcheckHideFormula)["Selected"], "false")
        self.assertEqual(get_state_as_dict(xcheckHideAll)["Selected"], "false")
        self.assertEqual(get_state_as_dict(xcheckHidePrinting)["Selected"], "false")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

    def test_tdf130762(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "3")  #tab Alignment
        xspinDegrees = xDialog.getChild("spinDegrees")
        self.assertEqual(get_state_as_dict(xspinDegrees)["Text"].replace('°', ''), "0")
        xspinDegrees.executeAction("DOWN", tuple())
        self.assertEqual(get_state_as_dict(xspinDegrees)["Text"].replace('°', ''), "355")
        xspinDegrees.executeAction("UP", tuple())
        self.assertEqual(get_state_as_dict(xspinDegrees)["Text"].replace('°', ''), "0")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
