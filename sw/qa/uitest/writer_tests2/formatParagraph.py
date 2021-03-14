# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text

import time
from uitest.debug import sleep
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import change_measurement_unit

class formatParagraph(UITestCase):

   def test_format_paragraph_tab_indents_spacing(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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
        props = {"TEXT": "Double"}
        actionProps = mkPropertyValues(props)
        xLineSpacing.executeAction("SELECT", actionProps)
        xActivate.executeAction("CLICK", tuple())

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()

   def test_format_paragraph_tab_alignment(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")

        xTextDirection = xDialog.getChild("comboLB_TEXTDIRECTION")
        xAlignment = xDialog.getChild("comboLB_VERTALIGN")
        xSnapToText = xDialog.getChild("checkCB_SNAP")
        xJustified = xDialog.getChild("radioBTN_JUSTIFYALIGN")
        xLastLine = xDialog.getChild("comboLB_LASTLINE")
        xExpandChk = xDialog.getChild("checkCB_EXPAND")

        props = {"TEXT": "Left-to-right (LTR)"}
        actionProps = mkPropertyValues(props)
        xTextDirection.executeAction("SELECT", actionProps)
        props2 = {"TEXT": "Top"}
        actionProps2 = mkPropertyValues(props2)
        xAlignment.executeAction("SELECT", actionProps2)
        xSnapToText.executeAction("CLICK", tuple())
        xJustified.executeAction("CLICK", tuple())
        props3 = {"TEXT": "Justified"}
        actionProps3 = mkPropertyValues(props3)
        xLastLine.executeAction("SELECT", actionProps3)
        xExpandChk.executeAction("CLICK", tuple())

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()

   def test_format_paragraph_tab_text_flow(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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
        props = {"TEXT": "Column"}
        actionProps = mkPropertyValues(props)
        xType.executeAction("SELECT", actionProps)
        props2 = {"TEXT": "After"}
        actionProps2 = mkPropertyValues(props2)
        xPosition.executeAction("SELECT", actionProps2)
        xspinOrphan.executeAction("UP", tuple())
        xspinWidow.executeAction("UP", tuple())
        xcheckWidow.executeAction("CLICK", tuple())
        xcheckOrphan.executeAction("CLICK", tuple())
        xcheckSplitPara.executeAction("CLICK", tuple())
        xcheckKeepPara.executeAction("CLICK", tuple())

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()

   def test_format_paragraph_tab_asian_typography(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "3")

        xcheckForbidList = xDialog.getChild("checkForbidList")
        xcheckHangPunct = xDialog.getChild("checkHangPunct")
        xcheckApplySpacing = xDialog.getChild("checkApplySpacing")

        xcheckForbidList.executeAction("CLICK", tuple())
        xcheckHangPunct.executeAction("CLICK", tuple())
        xcheckApplySpacing.executeAction("CLICK", tuple())

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "3")

        xcheckForbidList = xDialog.getChild("checkForbidList")
        xcheckHangPunct = xDialog.getChild("checkHangPunct")
        xcheckApplySpacing = xDialog.getChild("checkApplySpacing")

        self.assertEqual(get_state_as_dict(xcheckForbidList)["Selected"], "false")
        self.assertEqual(get_state_as_dict(xcheckHangPunct)["Selected"], "false")
        self.assertEqual(get_state_as_dict(xcheckApplySpacing)["Selected"], "false")

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()

   def test_format_paragraph_tab_outline_numbering(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "4")

        xOutline = xDialog.getChild("comboLB_OUTLINE_LEVEL")
        xNumbering = xDialog.getChild("comboLB_NUMBER_STYLE")
        xPara = xDialog.getChild("checkCB_RESTART_PARACOUNT")
        xParaSpin = xDialog.getChild("spinNF_RESTART_PARA")

        props = {"TEXT": "Level 1"}
        actionProps = mkPropertyValues(props)
        xOutline.executeAction("SELECT", actionProps)
        props2 = {"TEXT": "Bullet •"}
        actionProps2 = mkPropertyValues(props2)
        xNumbering.executeAction("SELECT", actionProps2)
        xPara.executeAction("CLICK", tuple())
        xParaSpin.executeAction("UP", tuple())

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()

   def test_format_paragraph_tab_tabs(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        change_measurement_unit(self, "Centimeter")

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "5")

        xCentered = xDialog.getChild("radiobuttonBTN_TABTYPE_CENTER")
        xUnderscore = xDialog.getChild("radiobuttonBTN_FILLCHAR_UNDERSCORE")
        xNewButtn = xDialog.getChild("buttonBTN_NEW")

        xCentered.executeAction("CLICK", tuple())
        xUnderscore.executeAction("CLICK", tuple())
        xNewButtn.executeAction("CLICK", tuple())
        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "5")

        xCentered = xDialog.getChild("radiobuttonBTN_TABTYPE_CENTER")
        xUnderscore = xDialog.getChild("radiobuttonBTN_FILLCHAR_UNDERSCORE")
        self.assertEqual(get_state_as_dict(xCentered)["Checked"], "true")
        self.assertEqual(get_state_as_dict(xUnderscore)["Checked"], "true")

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "5")

        xRight = xDialog.getChild("radiobuttonST_RIGHTTAB_ASIAN")
        xDashLine = xDialog.getChild("radiobuttonBTN_FILLCHAR_DASHLINE")
        xNewButtn = xDialog.getChild("buttonBTN_NEW")

        xRight.executeAction("CLICK", tuple())
        xDashLine.executeAction("CLICK", tuple())
        xNewButtn.executeAction("CLICK", tuple())
        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "5")

        xRight = xDialog.getChild("radiobuttonST_RIGHTTAB_ASIAN")
        xDashLine = xDialog.getChild("radiobuttonBTN_FILLCHAR_DASHLINE")
        self.assertEqual(get_state_as_dict(xRight)["Checked"], "true")
        self.assertEqual(get_state_as_dict(xDashLine)["Checked"], "true")

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "5")

        xLeft = xDialog.getChild("radiobuttonST_LEFTTAB_ASIAN")
        xPointsLine = xDialog.getChild("radiobuttonBTN_FILLCHAR_POINTS")
        xNewButtn = xDialog.getChild("buttonBTN_NEW")

        xLeft.executeAction("CLICK", tuple())
        xPointsLine.executeAction("CLICK", tuple())
        xNewButtn.executeAction("CLICK", tuple())
        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "5")

        xLeft = xDialog.getChild("radiobuttonST_LEFTTAB_ASIAN")
        xPointsLine = xDialog.getChild("radiobuttonBTN_FILLCHAR_POINTS")
        self.assertEqual(get_state_as_dict(xLeft)["Checked"], "true")
        self.assertEqual(get_state_as_dict(xPointsLine)["Checked"], "true")

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()

   def test_format_paragraph_tab_drop_caps(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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
        props = {"TEXT": "Definition"}
        actionProps = mkPropertyValues(props)
        xCharStyle.executeAction("SELECT", actionProps)
        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()

   def test_format_paragraph_tab_borders(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        change_measurement_unit(self, "Centimeter")

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "7")

        xStyle = xDialog.getChild("linestylelb")
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

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "7")

        xStyle = xDialog.getChild("linestylelb")
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

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)
        self.ui_test.close_doc()

   def test_format_paragraph_area(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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
        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)
        self.ui_test.close_doc()

   def test_format_paragraph_transparency(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "9")

        xTran = xDialog.getChild("RBT_TRANS_LINEAR")
        xTranText = xDialog.getChild("MTR_TRANSPARENT")

        xTran.executeAction("CLICK", tuple())
        xTranText.executeAction("UP", tuple())

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "9")

        xTran = xDialog.getChild("RBT_TRANS_LINEAR")
        xTranText = xDialog.getChild("MTR_TRANSPARENT")

        self.assertEqual(get_state_as_dict(xTran)["Checked"], "true")
        self.assertEqual(get_state_as_dict(xTranText)["Text"], "51%")

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "9")

        xGradient = xDialog.getChild("RBT_TRANS_GRADIENT")
        xType = xDialog.getChild("LB_TRGR_GRADIENT_TYPES")
        xAngle = xDialog.getChild("MTR_TRGR_ANGLE")
        xBorder = xDialog.getChild("MTR_TRGR_BORDER")
        xStart = xDialog.getChild("MTR_TRGR_START_VALUE")
        xEnd = xDialog.getChild("MTR_TRGR_END_VALUE")

        xGradient.executeAction("CLICK", tuple())
        props = {"TEXT": "Axial"}
        actionProps = mkPropertyValues(props)
        xType.executeAction("SELECT", actionProps)
        xAngle.executeAction("UP", tuple())
        xBorder.executeAction("UP", tuple())
        xStart.executeAction("UP", tuple())
        xEnd.executeAction("UP", tuple())

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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

        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
