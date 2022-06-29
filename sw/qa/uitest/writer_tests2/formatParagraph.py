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
from uitest.uihelper.common import change_measurement_unit

class formatParagraph(UITestCase):

   def test_format_paragraph_tab_indents_spacing(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")

                xBeforeText = xDialog.getChild("spinED_LEFTINDENT")
                xAfterText = xDialog.getChild("spinED_RIGHTINDENT")
                xFirstLine = xDialog.getChild("spinED_FLINEINDENT")
                xAutomaticChk = xDialog.getChild("checkCB_AUTO")
                xAbovePar = xDialog.getChild("spinED_TOPDIST")
                xBelowPar = xDialog.getChild("spinED_BOTTOMDIST")
                xChkspace = xDialog.getChild("checkCB_CONTEXTUALSPACING")
                xLineSpacing = xDialog.getChild("comboLB_LINEDIST")
                xActivate = xDialog.getChild("checkCB_REGISTER")

                xBeforeText.executeAction("UP", tuple())
                xAfterText.executeAction("UP", tuple())
                xFirstLine.executeAction("UP", tuple())
                xAutomaticChk.executeAction("CLICK", tuple())
                xAbovePar.executeAction("UP", tuple())
                xBelowPar.executeAction("UP", tuple())
                xChkspace.executeAction("CLICK", tuple())
                select_by_text(xLineSpacing, "Double")
                xActivate.executeAction("CLICK", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xBeforeText = xDialog.getChild("spinED_LEFTINDENT")
                xAfterText = xDialog.getChild("spinED_RIGHTINDENT")
                xFirstLine = xDialog.getChild("spinED_FLINEINDENT")
                xAutomaticChk = xDialog.getChild("checkCB_AUTO")
                xAbovePar = xDialog.getChild("spinED_TOPDIST")
                xBelowPar = xDialog.getChild("spinED_BOTTOMDIST")
                xChkspace = xDialog.getChild("checkCB_CONTEXTUALSPACING")
                xLineSpacing = xDialog.getChild("comboLB_LINEDIST")
                xActivate = xDialog.getChild("checkCB_REGISTER")

                self.assertEqual(get_state_as_dict(xBeforeText)["Text"], "0.50 ch")
                self.assertEqual(get_state_as_dict(xAfterText)["Text"], "0.50 ch")
                self.assertEqual(get_state_as_dict(xFirstLine)["Text"], "0.50 ch")
                self.assertEqual(get_state_as_dict(xAutomaticChk)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xAbovePar)["Text"], "0.50 line")
                self.assertEqual(get_state_as_dict(xBelowPar)["Text"], "0.50 line")
                self.assertEqual(get_state_as_dict(xChkspace)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xLineSpacing)["SelectEntryText"], "Double")
                self.assertEqual(get_state_as_dict(xActivate)["Selected"], "true")



   def test_format_paragraph_tab_alignment(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")

                xTextDirection = xDialog.getChild("comboLB_TEXTDIRECTION")
                xAlignment = xDialog.getChild("comboLB_VERTALIGN")
                xSnapToText = xDialog.getChild("checkCB_SNAP")
                xJustified = xDialog.getChild("radioBTN_JUSTIFYALIGN")
                xLastLine = xDialog.getChild("comboLB_LASTLINE")
                xExpandChk = xDialog.getChild("checkCB_EXPAND")

                select_by_text(xTextDirection, "Left-to-right (LTR)")
                select_by_text(xAlignment, "Top")
                xSnapToText.executeAction("CLICK", tuple())
                xJustified.executeAction("CLICK", tuple())
                select_by_text(xLastLine, "Justified")
                xExpandChk.executeAction("CLICK", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")

                xTextDirection = xDialog.getChild("comboLB_TEXTDIRECTION")
                xAlignment = xDialog.getChild("comboLB_VERTALIGN")
                xSnapToText = xDialog.getChild("checkCB_SNAP")
                xJustified = xDialog.getChild("radioBTN_JUSTIFYALIGN")
                xLastLine = xDialog.getChild("comboLB_LASTLINE")
                xExpandChk = xDialog.getChild("checkCB_EXPAND")

                self.assertEqual(get_state_as_dict(xTextDirection)["SelectEntryText"], "Left-to-right (LTR)")
                self.assertEqual(get_state_as_dict(xAlignment)["SelectEntryText"], "Top")
                self.assertEqual(get_state_as_dict(xSnapToText)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xJustified)["Checked"], "true")
                self.assertEqual(get_state_as_dict(xLastLine)["SelectEntryText"], "Justified")
                self.assertEqual(get_state_as_dict(xExpandChk)["Selected"], "true")



   def test_format_paragraph_tab_text_flow(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")

                xAutomaticaly = xDialog.getChild("checkAuto")
                xEnd = xDialog.getChild("spinLineEnd")
                xBegin = xDialog.getChild("spinLineBegin")
                xMax = xDialog.getChild("spinMaxNum")
                xIns = xDialog.getChild("checkInsert")
                xType = xDialog.getChild("comboBreakType")
                xPosition = xDialog.getChild("comboBreakPosition")
                xspinOrphan = xDialog.getChild("spinOrphan")
                xspinWidow = xDialog.getChild("spinWidow")
                xcheckWidow = xDialog.getChild("checkWidow")
                xcheckOrphan = xDialog.getChild("checkOrphan")
                xcheckSplitPara = xDialog.getChild("checkSplitPara")
                xcheckKeepPara = xDialog.getChild("checkKeepPara")

                xAutomaticaly.executeAction("CLICK", tuple())
                xEnd.executeAction("UP", tuple())
                xBegin.executeAction("UP", tuple())
                xMax.executeAction("UP", tuple())
                xIns.executeAction("CLICK", tuple())
                select_by_text(xType, "Column")
                select_by_text(xPosition, "After")
                xspinOrphan.executeAction("UP", tuple())
                xspinWidow.executeAction("UP", tuple())
                xcheckWidow.executeAction("CLICK", tuple())
                xcheckOrphan.executeAction("CLICK", tuple())
                xcheckSplitPara.executeAction("CLICK", tuple())
                xcheckKeepPara.executeAction("CLICK", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")

                xAutomaticaly = xDialog.getChild("checkAuto")
                xEnd = xDialog.getChild("spinLineEnd")
                xBegin = xDialog.getChild("spinLineBegin")
                xMax = xDialog.getChild("spinMaxNum")
                xIns = xDialog.getChild("checkInsert")
                xType = xDialog.getChild("comboBreakType")
                xPosition = xDialog.getChild("comboBreakPosition")
                xspinOrphan = xDialog.getChild("spinOrphan")
                xspinWidow = xDialog.getChild("spinWidow")
                xcheckWidow = xDialog.getChild("checkWidow")
                xcheckOrphan = xDialog.getChild("checkOrphan")
                xcheckSplitPara = xDialog.getChild("checkSplitPara")
                xcheckKeepPara = xDialog.getChild("checkKeepPara")

                self.assertEqual(get_state_as_dict(xAutomaticaly)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xEnd)["Text"], "3")
                self.assertEqual(get_state_as_dict(xBegin)["Text"], "3")
                self.assertEqual(get_state_as_dict(xMax)["Text"], "1")
                self.assertEqual(get_state_as_dict(xIns)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xType)["SelectEntryText"], "Column")
                self.assertEqual(get_state_as_dict(xPosition)["SelectEntryText"], "After")
                self.assertEqual(get_state_as_dict(xspinOrphan)["Text"], "2")
                self.assertEqual(get_state_as_dict(xspinWidow)["Text"], "2")
                self.assertEqual(get_state_as_dict(xcheckWidow)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xcheckOrphan)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xcheckSplitPara)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xcheckKeepPara)["Selected"], "true")



   def test_format_paragraph_tab_asian_typography(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "3")

                xcheckForbidList = xDialog.getChild("checkForbidList")
                xcheckHangPunct = xDialog.getChild("checkHangPunct")
                xcheckApplySpacing = xDialog.getChild("checkApplySpacing")

                xcheckForbidList.executeAction("CLICK", tuple())
                xcheckHangPunct.executeAction("CLICK", tuple())
                xcheckApplySpacing.executeAction("CLICK", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "3")

                xcheckForbidList = xDialog.getChild("checkForbidList")
                xcheckHangPunct = xDialog.getChild("checkHangPunct")
                xcheckApplySpacing = xDialog.getChild("checkApplySpacing")

                self.assertEqual(get_state_as_dict(xcheckForbidList)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xcheckHangPunct)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xcheckApplySpacing)["Selected"], "false")



   def test_format_paragraph_tab_outline_numbering(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "4")

                xOutline = xDialog.getChild("comboLB_OUTLINE_LEVEL")
                xNumbering = xDialog.getChild("comboLB_NUMBER_STYLE")
                xPara = xDialog.getChild("checkCB_RESTART_PARACOUNT")
                xParaSpin = xDialog.getChild("spinNF_RESTART_PARA")

                select_by_text(xOutline, "Level 1")
                select_by_text(xNumbering, "Bullet •")
                xPara.executeAction("CLICK", tuple())
                xParaSpin.executeAction("UP", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "4")

                xOutline = xDialog.getChild("comboLB_OUTLINE_LEVEL")
                xNumbering = xDialog.getChild("comboLB_NUMBER_STYLE")
                xPara = xDialog.getChild("checkCB_RESTART_PARACOUNT")
                xParaSpin = xDialog.getChild("spinNF_RESTART_PARA")

                self.assertEqual(get_state_as_dict(xOutline)["SelectEntryText"], "Level 1")
                self.assertEqual(get_state_as_dict(xNumbering)["SelectEntryText"], "Bullet •")
                self.assertEqual(get_state_as_dict(xPara)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xParaSpin)["Text"], "2")



   def test_format_paragraph_tab_tabs(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            change_measurement_unit(self, "Centimeter")

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")

                xDecimal = xDialog.getChild("radiobuttonBTN_TABTYPE_DECIMAL")
                xDecimalTxt = xDialog.getChild("entryED_TABTYPE_DECCHAR")
                xFill = xDialog.getChild("radiobuttonBTN_FILLCHAR_OTHER")
                xFillTxt = xDialog.getChild("entryED_FILLCHAR_OTHER")
                xNewButtn = xDialog.getChild("buttonBTN_NEW")
                xED_TABPOS = xDialog.getChild("ED_TABPOS")

                xDecimal.executeAction("CLICK", tuple())
                xDecimalTxt.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xDecimalTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"i"}))
                xFill.executeAction("CLICK", tuple())
                xFillTxt.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xFillTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"p"}))
                xED_TABPOS.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xED_TABPOS.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
                xNewButtn.executeAction("CLICK", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")

                xDecimal = xDialog.getChild("radiobuttonBTN_TABTYPE_DECIMAL")
                xDecimalTxt = xDialog.getChild("entryED_TABTYPE_DECCHAR")
                xFill = xDialog.getChild("radiobuttonBTN_FILLCHAR_OTHER")
                xFillTxt = xDialog.getChild("entryED_FILLCHAR_OTHER")
                xNewButtn = xDialog.getChild("buttonBTN_NEW")
                xED_TABPOS = xDialog.getChild("ED_TABPOS")

                self.assertEqual(get_state_as_dict(xDecimal)["Checked"], "true")
                self.assertEqual(get_state_as_dict(xDecimalTxt)["Text"], "i")
                self.assertEqual(get_state_as_dict(xFill)["Checked"], "true")
                self.assertEqual(get_state_as_dict(xFillTxt)["Text"], "p")
                self.assertEqual(get_state_as_dict(xED_TABPOS)["Text"], "1.00 cm")


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")

                xCentered = xDialog.getChild("radiobuttonBTN_TABTYPE_CENTER")
                xUnderscore = xDialog.getChild("radiobuttonBTN_FILLCHAR_UNDERSCORE")
                xNewButtn = xDialog.getChild("buttonBTN_NEW")

                xCentered.executeAction("CLICK", tuple())
                xUnderscore.executeAction("CLICK", tuple())
                xNewButtn.executeAction("CLICK", tuple())

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")

                xCentered = xDialog.getChild("radiobuttonBTN_TABTYPE_CENTER")
                xUnderscore = xDialog.getChild("radiobuttonBTN_FILLCHAR_UNDERSCORE")
                self.assertEqual(get_state_as_dict(xCentered)["Checked"], "true")
                self.assertEqual(get_state_as_dict(xUnderscore)["Checked"], "true")


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")

                xRight = xDialog.getChild("radiobuttonST_RIGHTTAB_ASIAN")
                xDashLine = xDialog.getChild("radiobuttonBTN_FILLCHAR_DASHLINE")
                xNewButtn = xDialog.getChild("buttonBTN_NEW")

                xRight.executeAction("CLICK", tuple())
                xDashLine.executeAction("CLICK", tuple())
                xNewButtn.executeAction("CLICK", tuple())

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")

                xRight = xDialog.getChild("radiobuttonST_RIGHTTAB_ASIAN")
                xDashLine = xDialog.getChild("radiobuttonBTN_FILLCHAR_DASHLINE")
                self.assertEqual(get_state_as_dict(xRight)["Checked"], "true")
                self.assertEqual(get_state_as_dict(xDashLine)["Checked"], "true")


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")

                xLeft = xDialog.getChild("radiobuttonST_LEFTTAB_ASIAN")
                xPointsLine = xDialog.getChild("radiobuttonBTN_FILLCHAR_POINTS")
                xNewButtn = xDialog.getChild("buttonBTN_NEW")

                xLeft.executeAction("CLICK", tuple())
                xPointsLine.executeAction("CLICK", tuple())
                xNewButtn.executeAction("CLICK", tuple())

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")

                xLeft = xDialog.getChild("radiobuttonST_LEFTTAB_ASIAN")
                xPointsLine = xDialog.getChild("radiobuttonBTN_FILLCHAR_POINTS")
                self.assertEqual(get_state_as_dict(xLeft)["Checked"], "true")
                self.assertEqual(get_state_as_dict(xPointsLine)["Checked"], "true")



   def test_format_paragraph_tab_drop_caps(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "6")

                xDisplay = xDialog.getChild("checkCB_SWITCH")
                xWholeWord = xDialog.getChild("checkCB_WORD")
                xLines = xDialog.getChild("spinFLD_LINES")
                xSpaceToText = xDialog.getChild("spinFLD_DISTANCE")
                xText = xDialog.getChild("entryEDT_TEXT")
                xCharStyle = xDialog.getChild("comboBOX_TEMPLATE")

                xDisplay.executeAction("CLICK", tuple())
                xWholeWord.executeAction("CLICK", tuple())
                xLines.executeAction("UP", tuple())
                xSpaceToText.executeAction("UP", tuple())
                xText.executeAction("TYPE", mkPropertyValues({"TEXT":"A"}))
                select_by_text(xCharStyle, "Definition")

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "6")

                xDisplay = xDialog.getChild("checkCB_SWITCH")
                xWholeWord = xDialog.getChild("checkCB_WORD")
                xLines = xDialog.getChild("spinFLD_LINES")
                xSpaceToText = xDialog.getChild("spinFLD_DISTANCE")
                xText = xDialog.getChild("entryEDT_TEXT")
                xCharStyle = xDialog.getChild("comboBOX_TEMPLATE")

                self.assertEqual(get_state_as_dict(xDisplay)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xWholeWord)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xText)["Text"], "A")
                self.assertEqual(get_state_as_dict(xCharStyle)["SelectEntryText"], "Definition")



   def test_format_paragraph_tab_borders(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            change_measurement_unit(self, "Centimeter")

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "7")

                xwidth = xDialog.getChild("linewidthmf")
                xSync = xDialog.getChild("sync")
                xLeft = xDialog.getChild("leftmf")
                xRight = xDialog.getChild("rightmf")
                xTop = xDialog.getChild("topmf")
                xBottom = xDialog.getChild("bottommf")
                xMerge = xDialog.getChild("mergewithnext")

                xwidth.executeAction("UP", tuple())
                xSync.executeAction("CLICK", tuple())
                xLeft.executeAction("UP", tuple())
                xLeft.executeAction("UP", tuple())
                xRight.executeAction("UP", tuple())
                xTop.executeAction("UP", tuple())
                xBottom.executeAction("UP", tuple())
                xMerge.executeAction("CLICK", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "7")

                xwidth = xDialog.getChild("linewidthmf")
                xSync = xDialog.getChild("sync")
                xLeft = xDialog.getChild("leftmf")
                xRight = xDialog.getChild("rightmf")
                xTop = xDialog.getChild("topmf")
                xBottom = xDialog.getChild("bottommf")
                xMerge = xDialog.getChild("mergewithnext")

                self.assertEqual(get_state_as_dict(xSync)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xMerge)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xLeft)["Text"], "0.20 cm")
                self.assertEqual(get_state_as_dict(xRight)["Text"], "0.10 cm")
                self.assertEqual(get_state_as_dict(xTop)["Text"], "0.10 cm")
                self.assertEqual(get_state_as_dict(xBottom)["Text"], "0.10 cm")


   def test_format_paragraph_area(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "8")

                xColor = xDialog.getChild("btncolor")
                xGradient = xDialog.getChild("btngradient")
                xBitmap = xDialog.getChild("btnbitmap")
                xPattern = xDialog.getChild("btnpattern")
                xHatch = xDialog.getChild("btnhatch")

                xColor.executeAction("CLICK", tuple())
                xGradient.executeAction("CLICK", tuple())
                xBitmap.executeAction("CLICK", tuple())
                xPattern.executeAction("CLICK", tuple())
                xHatch.executeAction("CLICK", tuple())

   def test_format_paragraph_transparency(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "9")

                xTran = xDialog.getChild("RBT_TRANS_LINEAR")
                xTranText = xDialog.getChild("MTR_TRANSPARENT")

                xTran.executeAction("CLICK", tuple())
                xTranText.executeAction("UP", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "9")

                xTran = xDialog.getChild("RBT_TRANS_LINEAR")
                xTranText = xDialog.getChild("MTR_TRANSPARENT")

                self.assertEqual(get_state_as_dict(xTran)["Checked"], "true")
                self.assertEqual(get_state_as_dict(xTranText)["Text"], "51%")


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "9")

                xGradient = xDialog.getChild("RBT_TRANS_GRADIENT")
                xType = xDialog.getChild("LB_TRGR_GRADIENT_TYPES")
                xAngle = xDialog.getChild("MTR_TRGR_ANGLE")
                xBorder = xDialog.getChild("MTR_TRGR_BORDER")
                xStart = xDialog.getChild("MTR_TRGR_START_VALUE")
                xEnd = xDialog.getChild("MTR_TRGR_END_VALUE")

                xGradient.executeAction("CLICK", tuple())
                select_by_text(xType, "Axial")
                xAngle.executeAction("UP", tuple())
                xBorder.executeAction("UP", tuple())
                xStart.executeAction("UP", tuple())
                xEnd.executeAction("UP", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "9")

                xGradient = xDialog.getChild("RBT_TRANS_GRADIENT")
                xType = xDialog.getChild("LB_TRGR_GRADIENT_TYPES")
                xAngle = xDialog.getChild("MTR_TRGR_ANGLE")
                xBorder = xDialog.getChild("MTR_TRGR_BORDER")
                xStart = xDialog.getChild("MTR_TRGR_START_VALUE")
                xEnd = xDialog.getChild("MTR_TRGR_END_VALUE")

                self.assertEqual(get_state_as_dict(xGradient)["Checked"], "true")
                self.assertEqual(get_state_as_dict(xType)["SelectEntryText"], "Axial")
                self.assertEqual(get_state_as_dict(xAngle)["Text"], "1°")
                self.assertEqual(get_state_as_dict(xBorder)["Text"], "1%")
                self.assertEqual(get_state_as_dict(xStart)["Text"], "1%")
                self.assertEqual(get_state_as_dict(xEnd)["Text"], "1%")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
