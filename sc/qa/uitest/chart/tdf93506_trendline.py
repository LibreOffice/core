# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import change_measurement_unit, type_text
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_by_text, select_pos

from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 93506 - Crash when selecting chart containing a trend line
# test all dialog items - trend Line dialog
class tdf93506(UITestCase):
   def test_tdf93506_chart_trendline_dialog(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf93506.ods")) as calc_doc:
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        change_measurement_unit(self, "Centimeter")

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"})) as xDialog:
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

            xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
            xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

            xWidth.executeAction("UP", tuple())
            xTransparent.executeAction("UP", tuple())
            self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
            self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")


        #reopen and verify
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"})) as xDialog:
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


        #Now change regression Type to Exponential
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"})) as xDialog:
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


        #reopen and verify Exponential
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"})) as xDialog:
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


        #Now change regression Type to Power
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"})) as xDialog:
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


        #reopen and verify Power
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"})) as xDialog:
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


        #Now change regression Type to Polynomial
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"})) as xDialog:
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


        #reopen and verify Polynomial
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"})) as xDialog:
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


        #Now change regression Type to Moving average
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"})) as xDialog:
            #Click on tab "Type".
            tabcontrol = xDialog.getChild("tabcontrol")
            select_pos(tabcontrol, "0")
            logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
            exponential = xDialog.getChild("exponential")  #type regression exponential
            power = xDialog.getChild("exponential")  #type regression power
            movingAverage = xDialog.getChild("movingAverage")  #type regression Moving average
            movingAverageType = xDialog.getChild("combo_moving_type")  #type regression Moving average type
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
            select_by_text(movingAverageType, "Averaged Abscissa")
            period.executeAction("UP", tuple())

            #Click on tab "Line".
            select_pos(tabcontrol, "1")
            xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
            xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

            self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
            self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")


        #reopen and verify Moving average
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"})) as xDialog:
            #Click on tab "Type".
            tabcontrol = xDialog.getChild("tabcontrol")
            select_pos(tabcontrol, "0")

            logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
            exponential = xDialog.getChild("exponential")  #type regression exponential
            power = xDialog.getChild("exponential")  #type regression power
            polynomial = xDialog.getChild("polynomial")  #type regression polynomial
            movingAverage = xDialog.getChild("movingAverage")  #type regression Moving average
            movingAverageType = xDialog.getChild("combo_moving_type")  #type regression Moving average type
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
            self.assertEqual(get_state_as_dict(movingAverageType)["SelectEntryText"], "Averaged Abscissa")
            self.assertEqual(get_state_as_dict(period)["Text"], "3")
            self.assertEqual(get_state_as_dict(xentryname)["Text"], "Name")

            #Click on tab "Line".
            select_pos(tabcontrol, "1")
            xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
            xTransparent = xDialog.getChild("MTR_LINE_TRANSPARENT")

            self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
            self.assertEqual(get_state_as_dict(xTransparent)["Text"], "5%")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
