# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 93506 - Crash when selecting chart containing a trend line
# test all dialog items - trend Line dialog

class tdf93506(UITestCase):
   def test_tdf93506_chart_trendline_dialog(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf93506.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog, set centimeters
    xDialog = self.xUITest.getTopFocusWindow()

    xPages = xDialog.getChild("pages")
    xCalcEntry = xPages.getChild('3')                 # calc
    xCalcEntry.executeAction("EXPAND", tuple())
    xCalcGeneralEntry = xCalcEntry.getChild('0')
    xCalcGeneralEntry.executeAction("SELECT", tuple())          #General
    xMetric = xDialog.getChild("unitlb")
    props = {"TEXT": "Centimeter"}
    actionProps = mkPropertyValues(props)
    xMetric.executeAction("SELECT", actionProps)
    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")
    # print(xDialog.getChildren())
    #('AttributeDialog', 'CBX_SYNCHRONIZE', 'CB_SYMBOL_RATIO', 'CTL_PREVIEW', 'FL_EDGE_STYLE', 'FL_LINE', 'FL_LINE_ENDS', 'FL_SYMBOL_FORMAT', 'FT_CAP_STYLE', 'FT_COLOR', 'FT_EDGE_STYLE', 'FT_LINE_ENDS_STYLE', 'FT_LINE_END_WIDTH', 'FT_LINE_START_WIDTH', 'FT_LINE_STYLE', 'FT_LINE_WIDTH', 'FT_SYMBOL_HEIGHT', 'FT_SYMBOL_WIDTH', 'FT_TRANSPARENT', 'LB_CAP_STYLE', 'LB_COLOR', 'LB_EDGE_STYLE', 'LB_END_STYLE', 'LB_LINE_STYLE', 'LB_START_STYLE', 'LineTabPage', 'MB_SYMBOL_BITMAP', 'MF_SYMBOL_HEIGHT', 'MF_SYMBOL_WIDTH', 'MTR_FLD_END_WIDTH', 'MTR_FLD_LINE_WIDTH', 'MTR_FLD_START_WIDTH', 'MTR_LINE_TRANSPARENT', 'TP_TRENDLINE', 'TSB_CENTER_END', 'TSB_CENTER_START', 'alignment1', 'alignment2', 'alignment3', 'alignment4', 'alignment6', 'auto_color_button', 'box1', 'box2', 'box3', 'box4', 'box5', 'boxARROW_STYLES', 'boxCOLOR', 'boxEND', 'boxSTART', 'boxTRANSPARENCY', 'boxWIDTH', 'cancel', 'color_picker_button', 'colorset', 'colorsetwin', 'degree', 'dialog-action_area1', 'dialog-vbox1', 'entry_Xname', 'entry_Yname', 'entry_name', 'exponential', 'extrapolateBackward', 'extrapolateForward', 'frame1', 'frame2', 'grid1', 'grid2', 'grid3', 'grid5', 'gridEDGE_CAPS', 'gridICON_SIZE', 'help', 'imageExponential', 'imageLinear', 'imageLogarithmic', 'imageMovingAverage', 'imagePolynomial', 'imagePower', 'interceptValue', 'label1', 'label2', 'label3', 'label4', 'label5', 'label6', 'label7', 'label8', 'label9', 'linear', 'logarithmic', 'movingAverage', 'none_color_button', 'ok', 'palette_listbox', 'palette_popup_window', 'period', 'polynomial', 'power', 'recent_colorset', 'reset', 'separator1', 'separator3', 'separator4', 'setIntercept', 'showCorrelationCoefficient', 'showEquation', 'tabcontrol')

    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    xentryname = xDialog.getChild("entry_name")   #add name
    extrapolateForward = xDialog.getChild("extrapolateForward")
    extrapolateBackward = xDialog.getChild("extrapolateBackward")
    setIntercept = xDialog.getChild("setIntercept")
    interceptValue = xDialog.getChild("interceptValue")
    showEquation = xDialog.getChild("showEquation")
    showCorrelationCoefficient = xDialog.getChild("showCorrelationCoefficient")
    xVarname = xDialog.getChild("entry_Xname")
    yVarName = xDialog.getChild("entry_Yname")

    logarithmic.executeAction("CLICK", tuple())
    xentryname.executeAction("TYPE", mkPropertyValues({"TEXT":"Name"}))
    extrapolateForward.executeAction("UP", tuple())
    extrapolateBackward.executeAction("UP", tuple())
    showEquation.executeAction("CLICK", tuple())
    showCorrelationCoefficient.executeAction("CLICK", tuple())
    xVarname.executeAction("CLEAR", tuple())
    xVarname.executeAction("TYPE", mkPropertyValues({"TEXT":"a"}))
    yVarName.executeAction("CLEAR", tuple())
    yVarName.executeAction("TYPE", mkPropertyValues({"TEXT":"f(a)"}))

    #Click on tab "Line".
    select_pos(tabcontrol, "1")
    # print(xDialog.getChildren())
    # ('AttributeDialog', 'CBX_SYNCHRONIZE', 'CB_SYMBOL_RATIO', 'CTL_PREVIEW', 'FL_EDGE_STYLE', 'FL_LINE', 'FL_LINE_ENDS', 'FL_SYMBOL_FORMAT', 'FT_CAP_STYLE', 'FT_COLOR', 'FT_EDGE_STYLE', 'FT_LINE_ENDS_STYLE', 'FT_LINE_END_WIDTH', 'FT_LINE_START_WIDTH', 'FT_LINE_STYLE', 'FT_LINE_WIDTH', 'FT_SYMBOL_HEIGHT', 'FT_SYMBOL_WIDTH', 'FT_TRANSPARENT', 'LB_CAP_STYLE', 'LB_COLOR', 'LB_EDGE_STYLE', 'LB_END_STYLE', 'LB_LINE_STYLE', 'LB_START_STYLE', 'LineTabPage', 'MB_SYMBOL_BITMAP', 'MF_SYMBOL_HEIGHT', 'MF_SYMBOL_WIDTH', 'MTR_FLD_END_WIDTH', 'MTR_FLD_LINE_WIDTH', 'MTR_FLD_START_WIDTH', 'MTR_LINE_TRANSPARENT', 'TP_TRENDLINE', 'TSB_CENTER_END', 'TSB_CENTER_START', 'alignment1', 'alignment2', 'alignment3', 'alignment4', 'alignment6', 'auto_color_button', 'box1', 'box2', 'box3', 'box4', 'box5', 'boxARROW_STYLES', 'boxCOLOR', 'boxEND', 'boxSTART', 'boxTRANSPARENCY', 'boxWIDTH', 'cancel', 'color_picker_button', 'colorset', 'colorsetwin', 'degree', 'dialog-action_area1', 'dialog-vbox1', 'entry_Xname', 'entry_Yname', 'entry_name', 'exponential', 'extrapolateBackward', 'extrapolateForward', 'frame1', 'frame2', 'grid1', 'grid2', 'grid3', 'grid5', 'gridEDGE_CAPS', 'gridICON_SIZE', 'help', 'imageExponential', 'imageLinear', 'imageLogarithmic', 'imageMovingAverage', 'imagePolynomial', 'imagePower', 'interceptValue', 'label1', 'label2', 'label3', 'label4', 'label5', 'label6', 'label7', 'label8', 'label9', 'linear', 'logarithmic', 'movingAverage', 'none_color_button', 'ok', 'palette_listbox', 'palette_popup_window', 'period', 'polynomial', 'power', 'recent_colorset', 'reset', 'separator1', 'separator3', 'separator4', 'setIntercept', 'showCorrelationCoefficient', 'showEquation', 'tabcontrol')

    xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
    xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

    xWidth.executeAction("UP", tuple())
    xTransparent.executeAction("UP", tuple())
    self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
    self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #reopen and verify
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")

    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    xentryname = xDialog.getChild("entry_name")   #add name
    extrapolateForward = xDialog.getChild("extrapolateForward")
    extrapolateBackward = xDialog.getChild("extrapolateBackward")
    setIntercept = xDialog.getChild("setIntercept")
    interceptValue = xDialog.getChild("interceptValue")
    showEquation = xDialog.getChild("showEquation")
    showCorrelationCoefficient = xDialog.getChild("showCorrelationCoefficient")
    xVarname = xDialog.getChild("entry_Xname")
    yVarName = xDialog.getChild("entry_Yname")

    self.assertEqual(get_state_as_dict(logarithmic)["Checked"], "true")
    self.assertEqual(get_state_as_dict(xentryname)["Text"], "Name")
    self.assertEqual(get_state_as_dict(extrapolateForward)["Text"], "1")
    self.assertEqual(get_state_as_dict(extrapolateBackward)["Text"], "1")
    self.assertEqual(get_state_as_dict(showEquation)["Selected"], "true")
    self.assertEqual(get_state_as_dict(showCorrelationCoefficient)["Selected"], "true")
    self.assertEqual(get_state_as_dict(xVarname)["Text"], "a")
    self.assertEqual(get_state_as_dict(yVarName)["Text"], "f(a)")

    #Click on tab "Line".
    select_pos(tabcontrol, "1")
    xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
    xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

    self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
    self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #Now change regression Type to Exponential
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")
    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    exponential = xDialog.getChild("exponential")  #type regression exponential
    xentryname = xDialog.getChild("entry_name")   #add name
    extrapolateForward = xDialog.getChild("extrapolateForward")
    extrapolateBackward = xDialog.getChild("extrapolateBackward")
    setIntercept = xDialog.getChild("setIntercept")
    interceptValue = xDialog.getChild("interceptValue")
    showEquation = xDialog.getChild("showEquation")
    showCorrelationCoefficient = xDialog.getChild("showCorrelationCoefficient")
    xVarname = xDialog.getChild("entry_Xname")
    yVarName = xDialog.getChild("entry_Yname")

    exponential.executeAction("CLICK", tuple())   #set exponential
    #set Force intercept
    setIntercept.executeAction("CLICK", tuple())
    interceptValue.executeAction("UP", tuple())

    #Click on tab "Line".
    select_pos(tabcontrol, "1")
    xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
    xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

    self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
    self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #reopen and verify Exponential
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")

    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    exponential = xDialog.getChild("exponential")  #type regression exponential
    xentryname = xDialog.getChild("entry_name")   #add name
    extrapolateForward = xDialog.getChild("extrapolateForward")
    extrapolateBackward = xDialog.getChild("extrapolateBackward")
    setIntercept = xDialog.getChild("setIntercept")
    interceptValue = xDialog.getChild("interceptValue")
    showEquation = xDialog.getChild("showEquation")
    showCorrelationCoefficient = xDialog.getChild("showCorrelationCoefficient")
    xVarname = xDialog.getChild("entry_Xname")
    yVarName = xDialog.getChild("entry_Yname")

    self.assertEqual(get_state_as_dict(exponential)["Checked"], "true")
    self.assertEqual(get_state_as_dict(xentryname)["Text"], "Name")
    self.assertEqual(get_state_as_dict(extrapolateForward)["Text"], "1")
    self.assertEqual(get_state_as_dict(extrapolateBackward)["Text"], "1")
    self.assertEqual(get_state_as_dict(showEquation)["Selected"], "true")
    self.assertEqual(get_state_as_dict(setIntercept)["Selected"], "true")
    self.assertEqual(get_state_as_dict(interceptValue)["Text"], "1")
    self.assertEqual(get_state_as_dict(showCorrelationCoefficient)["Selected"], "true")
    self.assertEqual(get_state_as_dict(xVarname)["Text"], "a")
    self.assertEqual(get_state_as_dict(yVarName)["Text"], "f(a)")

    #Click on tab "Line".
    select_pos(tabcontrol, "1")
    xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
    xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

    self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
    self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #Now change regression Type to Power
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")
    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    exponential = xDialog.getChild("exponential")  #type regression exponential
    power = xDialog.getChild("exponential")  #type regression power
    xentryname = xDialog.getChild("entry_name")   #add name
    extrapolateForward = xDialog.getChild("extrapolateForward")
    extrapolateBackward = xDialog.getChild("extrapolateBackward")
    setIntercept = xDialog.getChild("setIntercept")
    interceptValue = xDialog.getChild("interceptValue")
    showEquation = xDialog.getChild("showEquation")
    showCorrelationCoefficient = xDialog.getChild("showCorrelationCoefficient")
    xVarname = xDialog.getChild("entry_Xname")
    yVarName = xDialog.getChild("entry_Yname")

    power.executeAction("CLICK", tuple())   #set power

    #Click on tab "Line".
    select_pos(tabcontrol, "1")
    xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
    xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

    self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
    self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #reopen and verify Power
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")

    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    exponential = xDialog.getChild("exponential")  #type regression exponential
    power = xDialog.getChild("exponential")  #type regression power
    xentryname = xDialog.getChild("entry_name")   #add name
    extrapolateForward = xDialog.getChild("extrapolateForward")
    extrapolateBackward = xDialog.getChild("extrapolateBackward")
    setIntercept = xDialog.getChild("setIntercept")
    interceptValue = xDialog.getChild("interceptValue")
    showEquation = xDialog.getChild("showEquation")
    showCorrelationCoefficient = xDialog.getChild("showCorrelationCoefficient")
    xVarname = xDialog.getChild("entry_Xname")
    yVarName = xDialog.getChild("entry_Yname")

    self.assertEqual(get_state_as_dict(power)["Checked"], "true")
    self.assertEqual(get_state_as_dict(xentryname)["Text"], "Name")
    self.assertEqual(get_state_as_dict(extrapolateForward)["Text"], "1")
    self.assertEqual(get_state_as_dict(extrapolateBackward)["Text"], "1")
    self.assertEqual(get_state_as_dict(showEquation)["Selected"], "true")
    # self.assertEqual(get_state_as_dict(setIntercept)["Selected"], "true")
    # self.assertEqual(get_state_as_dict(interceptValue)["Text"], "1")
    self.assertEqual(get_state_as_dict(showCorrelationCoefficient)["Selected"], "true")
    self.assertEqual(get_state_as_dict(xVarname)["Text"], "a")
    self.assertEqual(get_state_as_dict(yVarName)["Text"], "f(a)")

    #Click on tab "Line".
    select_pos(tabcontrol, "1")
    xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
    xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

    self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
    self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #Now change regression Type to Polynomial
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")
    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    exponential = xDialog.getChild("exponential")  #type regression exponential
    power = xDialog.getChild("exponential")  #type regression power
    polynomial = xDialog.getChild("polynomial")  #type regression polynomial
    degree = xDialog.getChild("degree")
    xentryname = xDialog.getChild("entry_name")   #add name
    extrapolateForward = xDialog.getChild("extrapolateForward")
    extrapolateBackward = xDialog.getChild("extrapolateBackward")
    setIntercept = xDialog.getChild("setIntercept")
    interceptValue = xDialog.getChild("interceptValue")
    showEquation = xDialog.getChild("showEquation")
    showCorrelationCoefficient = xDialog.getChild("showCorrelationCoefficient")
    xVarname = xDialog.getChild("entry_Xname")
    yVarName = xDialog.getChild("entry_Yname")

    polynomial.executeAction("CLICK", tuple())   #set polynomial
    degree.executeAction("UP", tuple())

    #Click on tab "Line".
    select_pos(tabcontrol, "1")
    xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
    xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

    self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
    self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #reopen and verify Polynomial
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")

    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    exponential = xDialog.getChild("exponential")  #type regression exponential
    power = xDialog.getChild("exponential")  #type regression power
    polynomial = xDialog.getChild("polynomial")  #type regression polynomial
    degree = xDialog.getChild("degree")
    xentryname = xDialog.getChild("entry_name")   #add name
    extrapolateForward = xDialog.getChild("extrapolateForward")
    extrapolateBackward = xDialog.getChild("extrapolateBackward")
    setIntercept = xDialog.getChild("setIntercept")
    interceptValue = xDialog.getChild("interceptValue")
    showEquation = xDialog.getChild("showEquation")
    showCorrelationCoefficient = xDialog.getChild("showCorrelationCoefficient")
    xVarname = xDialog.getChild("entry_Xname")
    yVarName = xDialog.getChild("entry_Yname")

    self.assertEqual(get_state_as_dict(polynomial)["Checked"], "true")
    self.assertEqual(get_state_as_dict(degree)["Text"], "3")
    self.assertEqual(get_state_as_dict(xentryname)["Text"], "Name")
    self.assertEqual(get_state_as_dict(extrapolateForward)["Text"], "1")
    self.assertEqual(get_state_as_dict(extrapolateBackward)["Text"], "1")
    self.assertEqual(get_state_as_dict(showEquation)["Selected"], "true")
    self.assertEqual(get_state_as_dict(setIntercept)["Selected"], "true")
    self.assertEqual(get_state_as_dict(interceptValue)["Text"], "1")
    self.assertEqual(get_state_as_dict(showCorrelationCoefficient)["Selected"], "true")
    self.assertEqual(get_state_as_dict(xVarname)["Text"], "a")
    self.assertEqual(get_state_as_dict(yVarName)["Text"], "f(a)")

    #Click on tab "Line".
    select_pos(tabcontrol, "1")
    xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
    xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

    self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
    self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #Now change regression Type to Moving average
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")
    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    exponential = xDialog.getChild("exponential")  #type regression exponential
    power = xDialog.getChild("exponential")  #type regression power
    movingAverage = xDialog.getChild("movingAverage")  #type regression Moving average
    period = xDialog.getChild("period")
    xentryname = xDialog.getChild("entry_name")   #add name
    extrapolateForward = xDialog.getChild("extrapolateForward")
    extrapolateBackward = xDialog.getChild("extrapolateBackward")
    setIntercept = xDialog.getChild("setIntercept")
    interceptValue = xDialog.getChild("interceptValue")
    showEquation = xDialog.getChild("showEquation")
    showCorrelationCoefficient = xDialog.getChild("showCorrelationCoefficient")
    xVarname = xDialog.getChild("entry_Xname")
    yVarName = xDialog.getChild("entry_Yname")

    movingAverage.executeAction("CLICK", tuple())   #set polynomial
    period.executeAction("UP", tuple())

    #Click on tab "Line".
    select_pos(tabcontrol, "1")
    xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
    xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

    self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
    self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #reopen and verify Moving average
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")

    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    exponential = xDialog.getChild("exponential")  #type regression exponential
    power = xDialog.getChild("exponential")  #type regression power
    polynomial = xDialog.getChild("polynomial")  #type regression polynomial
    movingAverage = xDialog.getChild("movingAverage")  #type regression Moving average
    degree = xDialog.getChild("degree")
    period = xDialog.getChild("period")
    xentryname = xDialog.getChild("entry_name")   #add name
    extrapolateForward = xDialog.getChild("extrapolateForward")
    extrapolateBackward = xDialog.getChild("extrapolateBackward")
    setIntercept = xDialog.getChild("setIntercept")
    interceptValue = xDialog.getChild("interceptValue")
    showEquation = xDialog.getChild("showEquation")
    showCorrelationCoefficient = xDialog.getChild("showCorrelationCoefficient")
    xVarname = xDialog.getChild("entry_Xname")
    yVarName = xDialog.getChild("entry_Yname")

    self.assertEqual(get_state_as_dict(movingAverage)["Checked"], "true")
    self.assertEqual(get_state_as_dict(period)["Text"], "3")
    self.assertEqual(get_state_as_dict(xentryname)["Text"], "Name")

    #Click on tab "Line".
    select_pos(tabcontrol, "1")
    xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
    xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

    self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
    self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
