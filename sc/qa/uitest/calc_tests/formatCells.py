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
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import change_measurement_unit
from libreoffice.uno.propertyvalue import mkPropertyValues

class formatCell(UITestCase):
    def test_format_cell_numbers_tab(self):
        #numberingformatpage.ui
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")  #tab Numbers
                xdecimalsed = xDialog.getChild("decimalsed")
                xleadzerosed = xDialog.getChild("leadzerosed")
                xnegnumred = xDialog.getChild("negnumred")
                xthousands = xDialog.getChild("thousands")
                xlocalelb = xDialog.getChild("localelb")
                xformatted = xDialog.getChild("formatted")
                #language
                select_by_text(xlocalelb, "English (USA)")
                #other properties
                xdecimalsed.executeAction("UP", tuple())
                xleadzerosed.executeAction("UP", tuple())
                xnegnumred.executeAction("CLICK", tuple())
                xthousands.executeAction("CLICK", tuple())
                #format   #,#00.0;[RED]-#,#00.0
                self.assertEqual(get_state_as_dict(xformatted)["Text"], "#,#00.0;[RED]-#,#00.0")
                #save
            #verify
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")  #tab Numbers
                xliststore1 = xDialog.getChild("categorylb")  #1st list / Category
                xdecimalsed = xDialog.getChild("decimalsed")
                xleadzerosed = xDialog.getChild("leadzerosed")
                xnegnumred = xDialog.getChild("negnumred")
                xthousands = xDialog.getChild("thousands")
                xlocalelb = xDialog.getChild("localelb")
                xformatted = xDialog.getChild("formatted")

                self.assertEqual(get_state_as_dict(xliststore1)["SelectEntryText"], "Number")
                self.assertEqual(get_state_as_dict(xlocalelb)["SelectEntryText"], "English (USA)")
                self.assertEqual(get_state_as_dict(xdecimalsed)["Text"], "1")
                self.assertEqual(get_state_as_dict(xleadzerosed)["Text"], "2")
                self.assertEqual(get_state_as_dict(xnegnumred)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xthousands)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xformatted)["Text"], "#,#00.0;[RED]-#,#00.0")


    def test_format_cell_font_tab(self):
        #numberingformatpage.ui
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")  #tab Font

                # xNoteBook = xDialog.getChild("nbWestern") //western notebook is always active
                xSizeFont = xDialog.getChild("cbWestSize")
                xSizeFont.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xSizeFont.executeAction("TYPE", mkPropertyValues({"TEXT":"18"}))    #set font size 18
                xLangFont = xDialog.getChild("cbWestLanguage")
                select_pos(xLangFont, "0")

                xNoteBook = xDialog.getChild("nbCJKCTL")
                select_pos(xNoteBook, "0")
                xSizeFontEast = xDialog.getChild("cbCJKSize")
                xSizeFontEast.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xSizeFontEast.executeAction("TYPE", mkPropertyValues({"TEXT":"18"}))    #set font size 18
                xLangFontEast = xDialog.getChild("cbCJKLanguage")
                select_pos(xLangFontEast, "0")

                select_pos(xNoteBook, "1")
                xSizeFontCTL = xDialog.getChild("cbCTLSize")
                xSizeFontCTL.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xSizeFontCTL.executeAction("TYPE", mkPropertyValues({"TEXT":"18"}))    #set font size 18
                xLangFontCTL = xDialog.getChild("cbCTLLanguage")
                select_pos(xLangFontCTL, "0")

            #Verify - select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")  #tab Font
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
                self.assertEqual(get_state_as_dict(xSizeFontCTL)["Text"], "18 pt") #check font size
                xLangFontCTL = xDialog.getChild("cbCTLLanguage")
                self.assertEqual(get_state_as_dict(xLangFontCTL)["Text"], "[None]")



    def test_format_cell_font_effects_tab(self):
        #numberingformatpage.ui
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
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

            #Verify- select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog", close_button="cancel") as xDialog:
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



    def test_format_cell_alignment_tab(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
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
            #Verify- select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
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


    def test_format_cell_asian_typography_tab(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "4")  #tab Asian typography
                xcheckForbidList = xDialog.getChild("checkForbidList")
                xcheckForbidList.executeAction("CLICK", tuple())
            #Verify- select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "4")
                xcheckForbidList = xDialog.getChild("checkForbidList")
                self.assertEqual(get_state_as_dict(xcheckForbidList)["Selected"], "true")


    def test_minimal_border_width(self):
        #borderpage.ui, tdf#137790
        with self.ui_test.create_doc_in_start_center("calc"):

            #set points pt measurement
            with change_measurement_unit(self, "Point"):

                xCalcDoc = self.xUITest.getTopFocusWindow()
                gridwin = xCalcDoc.getChild("grid_window")

                #select cell A1
                gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
                #format - cell
                with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
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
                    # minimum predefined width is Medium (1.50 pt)
                    self.assertEqual(widthVal, '1.50 pt')

                    # set line style to "solid"
                    xLineSet.executeAction("CHOOSE", mkPropertyValues({"POS": "1"}))
                    widthVal = get_state_as_dict(linewidthmf)["Text"]
                    self.assertEqual(widthVal, '1.50 pt')

                    # make custom line width
                    linewidthmf.executeAction("UP", tuple())
                    linewidthmf.executeAction("UP", tuple())
                    linewidthmf.executeAction("UP", tuple())
                    widthVal = get_state_as_dict(linewidthmf)["Text"]
                    self.assertEqual(widthVal, '2.25 pt')

                    # set line style to "double" (minimal width is not taken)
                    xLineSet.executeAction("CHOOSE", mkPropertyValues({"POS": "8"}))
                    widthVal = get_state_as_dict(linewidthmf)["Text"]
                    self.assertEqual(widthVal, '2.25 pt')



    def test_format_cell_borders_tab(self):
        #borderpage.ui
        with self.ui_test.create_doc_in_start_center("calc"):

            #set points pt measurement
            with change_measurement_unit(self, "Point"):
                xCalcDoc = self.xUITest.getTopFocusWindow()
                gridwin = xCalcDoc.getChild("grid_window")

                #select cell A1
                gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
                #format - cell
                with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
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


                # Verify select cell A1
                gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
                #format - cell
                with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
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



    def test_format_cell_cell_protection_tab(self):
        #cellprotectionpage.ui
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "6")  #tab Cell protection
                xcheckHideFormula = xDialog.getChild("checkHideFormula")
                xcheckHideAll = xDialog.getChild("checkHideAll")
                xcheckHidePrinting = xDialog.getChild("checkHidePrinting")

                xcheckHideFormula.executeAction("CLICK", tuple())
                xcheckHideAll.executeAction("CLICK", tuple())
                xcheckHidePrinting.executeAction("CLICK", tuple())

            # Verify select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "6")  #tab Borders
                xcheckHideFormula = xDialog.getChild("checkHideFormula")
                xcheckHideAll = xDialog.getChild("checkHideAll")
                xcheckHidePrinting = xDialog.getChild("checkHidePrinting")

                self.assertEqual(get_state_as_dict(xcheckHideFormula)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xcheckHideAll)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xcheckHidePrinting)["Selected"], "false")



    def test_tdf130762(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "3")  #tab Alignment
                xspinDegrees = xDialog.getChild("spinDegrees")
                self.assertEqual(get_state_as_dict(xspinDegrees)["Text"].replace('°', ''), "0")
                xspinDegrees.executeAction("DOWN", tuple())
                self.assertEqual(get_state_as_dict(xspinDegrees)["Text"].replace('°', ''), "355")
                xspinDegrees.executeAction("UP", tuple())
                self.assertEqual(get_state_as_dict(xspinDegrees)["Text"].replace('°', ''), "0")

    def test_format_cell_spell_out_numbering(self):
        #numberingformatpage.ui
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")  #tab Numbers

                formatlb = xDialog.getChild("formatlb")
                xformatted = xDialog.getChild("formatted")

                # NatNum12 number formats

                entry = formatlb.getChild("11")
                self.assertEqual(get_state_as_dict(entry)["Text"], "ONE HUNDRED")
                entry.executeAction("SELECT", tuple())
                self.assertEqual(get_state_as_dict(xformatted)["Text"], "[NatNum12 upper cardinal]0")

                entry = formatlb.getChild("10")
                self.assertEqual(get_state_as_dict(entry)["Text"], "One Hundred")
                entry.executeAction("SELECT", tuple())
                self.assertEqual(get_state_as_dict(xformatted)["Text"], "[NatNum12 title cardinal]0")

                entry = formatlb.getChild("9")
                self.assertEqual(get_state_as_dict(entry)["Text"], "One hundred")
                entry.executeAction("SELECT", tuple())
                self.assertEqual(get_state_as_dict(xformatted)["Text"], "[NatNum12 capitalize cardinal]0")

                entry = formatlb.getChild("8")
                self.assertEqual(get_state_as_dict(entry)["Text"], "one hundred")
                entry.executeAction("SELECT", tuple())
                self.assertEqual(get_state_as_dict(xformatted)["Text"], "[NatNum12 cardinal]0")

                # NatNum12 en_US currency formats

                categorylb = xDialog.getChild("categorylb")
                entry = categorylb.getChild("4") # Currency
                entry.executeAction("SELECT", tuple())

                currencies = ["ONE U.S. DOLLAR AND TWENTY CENTS", "ONE U.S. DOLLAR", "One U.S. Dollar and Twenty Cents", "One U.S. Dollar"]
                formats = ["[NatNum12 upper USD]0.00", "[NatNum12 upper USD]0", "[NatNum12 title USD]0.00", "[NatNum12 title USD]0"]

                # handle different order of the items
                numCurrency = 0
                numFormat = 0
                for i in formatlb.getChildren():
                    entry = formatlb.getChild(i)
                    if get_state_as_dict(entry)["Text"] in currencies:
                        numCurrency = numCurrency + 1
                    entry.executeAction("SELECT", tuple())
                    xformatted = xDialog.getChild("formatted")
                    if get_state_as_dict(xformatted)["Text"] in formats:
                        numFormat = numFormat + 1

                self.assertEqual(4, numCurrency)
                self.assertEqual(4, numFormat)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
