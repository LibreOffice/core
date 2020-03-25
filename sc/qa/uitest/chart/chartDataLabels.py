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

from com.sun.star.lang import Locale

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Chart Display Data Labels dialog

class chartDataLabels(UITestCase):
   def test_chart_data_labels_dialog(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf98390.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")

    xDataSeries = document.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
    self.assertFalse(xDataSeries[0].Label.ShowNumber)
    self.assertFalse(xDataSeries[0].Label.ShowCategoryName)
    self.assertFalse(xDataSeries[0].Label.ShowLegendSymbol)
    self.assertEqual(xDataSeries[0].LabelSeparator, " ")

    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuDataLabels"}))
    xDialog = self.xUITest.getTopFocusWindow()

    xTabs = xDialog.getChild("tabcontrol")
    select_pos(xTabs, "1")

    self.assertTrue(xDataSeries[0].Label.ShowNumber)
    self.assertFalse(xDataSeries[0].Label.ShowCategoryName)
    self.assertFalse(xDataSeries[0].Label.ShowLegendSymbol)
    self.assertEqual(xDataSeries[0].LabelSeparator, " ")

    valueAsNumber = xDialog.getChild("CB_VALUE_AS_NUMBER")
    category = xDialog.getChild("CB_CATEGORY")
    legend = xDialog.getChild("CB_SYMBOL")
    wrapText = xDialog.getChild("CB_WRAP_TEXT")
    separator = xDialog.getChild("LB_TEXT_SEPARATOR")
    placement = xDialog.getChild("LB_LABEL_PLACEMENT")
    degrees = xDialog.getChild("NF_LABEL_DEGREES")
    textDirection = xDialog.getChild("LB_LABEL_TEXTDIR")

    #valueAsNumber.executeAction("CLICK", tuple())
    category.executeAction("CLICK", tuple())
    legend.executeAction("CLICK", tuple())
    wrapText.executeAction("CLICK", tuple())
    props = {"TEXT": "Comma"}
    actionProps = mkPropertyValues(props)
    separator.executeAction("SELECT", actionProps)

    props2 = {"TEXT": "Below"}
    actionProps2 = mkPropertyValues(props2)
    placement.executeAction("SELECT", actionProps2)
    degrees.executeAction("UP", tuple())
    props3 = {"TEXT": "Right-to-left"}
    actionProps3 = mkPropertyValues(props3)
    textDirection.executeAction("SELECT", actionProps3)

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.assertTrue(xDataSeries[0].Label.ShowNumber)
    self.assertTrue(xDataSeries[0].Label.ShowCategoryName)
    self.assertTrue(xDataSeries[0].Label.ShowLegendSymbol)
    self.assertEqual(xDataSeries[0].LabelSeparator, ", ")

    #reopen and verify InsertMenuDataLabels dialog
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuDataLabels"}))
    xDialog = self.xUITest.getTopFocusWindow()

    xTabs = xDialog.getChild("tabcontrol")
    select_pos(xTabs, "1")

    self.assertTrue(xDataSeries[0].Label.ShowNumber)
    self.assertTrue(xDataSeries[0].Label.ShowCategoryName)
    self.assertTrue(xDataSeries[0].Label.ShowLegendSymbol)
    self.assertEqual(xDataSeries[0].LabelSeparator, ", ")

    valueAsNumber = xDialog.getChild("CB_VALUE_AS_NUMBER")
    category = xDialog.getChild("CB_CATEGORY")
    legend = xDialog.getChild("CB_SYMBOL")
    wrapText = xDialog.getChild("CB_WRAP_TEXT")
    separator = xDialog.getChild("LB_TEXT_SEPARATOR")
    placement = xDialog.getChild("LB_LABEL_PLACEMENT")
    degrees = xDialog.getChild("NF_LABEL_DEGREES")
    textDirection = xDialog.getChild("LB_LABEL_TEXTDIR")

    self.assertEqual(get_state_as_dict(valueAsNumber)["Selected"], "true")
    self.assertEqual(get_state_as_dict(category)["Selected"], "true")
    self.assertEqual(get_state_as_dict(legend)["Selected"], "true")
    self.assertEqual(get_state_as_dict(wrapText)["Selected"], "true")
    self.assertEqual(get_state_as_dict(separator)["SelectEntryText"], "Comma")
    self.assertEqual(get_state_as_dict(placement)["SelectEntryText"], "Below")
    self.assertEqual(get_state_as_dict(degrees)["Text"], "1°")
    self.assertEqual(get_state_as_dict(textDirection)["SelectEntryText"], "Right-to-left")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.assertTrue(xDataSeries[0].Label.ShowNumber)
    self.assertTrue(xDataSeries[0].Label.ShowCategoryName)
    self.assertTrue(xDataSeries[0].Label.ShowLegendSymbol)
    self.assertEqual(xDataSeries[0].LabelSeparator, ", ")

    self.ui_test.close_doc()

   def test_chart_data_labels_percentage_dialog(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("dataLabels.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")

    xDataSeries = document.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
    self.assertTrue(xDataSeries[0].Label.ShowNumber)
    self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)
    self.assertIsNone(xDataSeries[0].PercentageNumberFormat)

    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatDataLabels"}))
    xDialog = self.xUITest.getTopFocusWindow()

    xTabs = xDialog.getChild("tabcontrol")
    select_pos(xTabs, "1")

    self.assertTrue(xDataSeries[0].Label.ShowNumber)
    self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)
    self.assertIsNone(xDataSeries[0].PercentageNumberFormat)

    valueAsNumber = xDialog.getChild("CB_VALUE_AS_NUMBER")
    valueAsPercentage = xDialog.getChild("CB_VALUE_AS_PERCENTAGE")
    category = xDialog.getChild("CB_CATEGORY")
    legend = xDialog.getChild("CB_SYMBOL")
    wrapText = xDialog.getChild("CB_WRAP_TEXT")
    separator = xDialog.getChild("LB_TEXT_SEPARATOR")
    placement = xDialog.getChild("LB_LABEL_PLACEMENT")
    degrees = xDialog.getChild("NF_LABEL_DEGREES")
    textDirection = xDialog.getChild("LB_LABEL_TEXTDIR")
    buttonPercentage = xDialog.getChild("PB_PERCENT_NUMBERFORMAT")

    valueAsNumber.executeAction("CLICK", tuple())
    valueAsPercentage.executeAction("CLICK", tuple())
    #button Percentage format

    def handle_perc_dlg(dialog):
            sourceformat = dialog.getChild("sourceformat")
            decimalsed = dialog.getChild("decimalsed")
            leadzerosed = dialog.getChild("leadzerosed")
            negnumred = dialog.getChild("negnumred")
            thousands = dialog.getChild("thousands")
            formatted = dialog.getChild("formatted")

            sourceformat.executeAction("CLICK", tuple())
            decimalsed.executeAction("UP", tuple())
            leadzerosed.executeAction("UP", tuple())
            negnumred.executeAction("CLICK", tuple())
            thousands.executeAction("CLICK", tuple())
            self.assertEqual(get_state_as_dict(formatted)["Text"], "#,#00.0%;[RED]-#,#00.0%")

            xOKButton = dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOKButton)

    self.ui_test.execute_blocking_action(buttonPercentage.executeAction, args=('CLICK', ()),
                dialog_handler=handle_perc_dlg)

    self.assertEqual(get_state_as_dict(valueAsNumber)["Selected"], "false")
    self.assertEqual(get_state_as_dict(valueAsPercentage)["Selected"], "true")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    xNumberFormats = document.Sheets[0].Charts[0].getEmbeddedObject().getNumberFormats()
    xLocale = Locale('en', 'US', '')
    xFormat = xNumberFormats.queryKey("#,#00.0%;[RED]-#,#00.0%", xLocale, True)

    self.assertFalse(xDataSeries[0].Label.ShowNumber)
    self.assertTrue(xDataSeries[0].Label.ShowNumberInPercent)
    self.assertEqual(xDataSeries[0].PercentageNumberFormat, xFormat)

    #reopen and verify Percentage dialog
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatDataLabels"}))
    xDialog = self.xUITest.getTopFocusWindow()

    xTabs = xDialog.getChild("tabcontrol")
    select_pos(xTabs, "1")

    self.assertFalse(xDataSeries[0].Label.ShowNumber)
    self.assertTrue(xDataSeries[0].Label.ShowNumberInPercent)
    self.assertEqual(xDataSeries[0].PercentageNumberFormat, xFormat)

    valueAsNumber = xDialog.getChild("CB_VALUE_AS_NUMBER")
    valueAsPercentage = xDialog.getChild("CB_VALUE_AS_PERCENTAGE")
    category = xDialog.getChild("CB_CATEGORY")
    legend = xDialog.getChild("CB_SYMBOL")
    wrapText = xDialog.getChild("CB_WRAP_TEXT")
    separator = xDialog.getChild("LB_TEXT_SEPARATOR")
    placement = xDialog.getChild("LB_LABEL_PLACEMENT")
    degrees = xDialog.getChild("NF_LABEL_DEGREES")
    textDirection = xDialog.getChild("LB_LABEL_TEXTDIR")
    buttonPercentage = xDialog.getChild("PB_PERCENT_NUMBERFORMAT")

    self.assertEqual(get_state_as_dict(valueAsNumber)["Selected"], "false")
    self.assertEqual(get_state_as_dict(valueAsPercentage)["Selected"], "true")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.assertFalse(xDataSeries[0].Label.ShowNumber)
    self.assertTrue(xDataSeries[0].Label.ShowNumberInPercent)
    self.assertEqual(xDataSeries[0].PercentageNumberFormat, xFormat)

    self.ui_test.close_doc()

   def test_tdf131291(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf131291.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")

    xDataSeries = document.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
    self.assertTrue(xDataSeries[0].Label.ShowNumber)
    self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)

    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatDataLabels"}))
    xDialog = self.xUITest.getTopFocusWindow()

    xTabs = xDialog.getChild("tabcontrol")
    select_pos(xTabs, "1")

    self.assertTrue(xDataSeries[0].Label.ShowNumber)
    self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)

    xNumberFormatBtn = xDialog.getChild("PB_NUMBERFORMAT")

    def handle_number_dlg(dialog):
        categoryformat = dialog.getChild("categorylb")
        formatted = dialog.getChild("formatted")
        sourceformat = dialog.getChild("sourceformat")

        # Select currency
        sourceformat.executeAction("CLICK", tuple());
        categoryformat.getChild("4").executeAction("SELECT", tuple());

        self.assertEqual(get_state_as_dict(categoryformat)["SelectEntryText"], "Currency")

        self.assertEqual(get_state_as_dict(formatted)["Text"], "[$$-409]#,##0.00;[RED]-[$$-409]#,##0.00")

        xOKButton = dialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKButton)

    self.ui_test.execute_blocking_action(xNumberFormatBtn.executeAction, args=('CLICK', ()),
        dialog_handler=handle_number_dlg)

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    xNumberFormats = document.Sheets[0].Charts[0].getEmbeddedObject().getNumberFormats()
    xLocale = Locale()
    xFormat = xNumberFormats.queryKey("[$$-409]#,##0.00;[RED]-[$$-409]#,##0.00", xLocale, True)

    self.assertTrue(xDataSeries[0].Label.ShowNumber)
    self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)
    self.assertEqual(xDataSeries[0].NumberFormat, xFormat)

    #reopen and verify the previous changes
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatDataLabels"}))
    xDialog = self.xUITest.getTopFocusWindow()

    self.assertTrue(xDataSeries[0].Label.ShowNumber)
    self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)
    # tdf#131291: it would fail here: AssertionError: 0 != 104
    self.assertEqual(xDataSeries[0].NumberFormat, xFormat)

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.assertTrue(xDataSeries[0].Label.ShowNumber)
    self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)
    self.assertEqual(xDataSeries[0].NumberFormat, xFormat)
    self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
