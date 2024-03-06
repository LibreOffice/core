# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_by_text, select_pos

from libreoffice.uno.propertyvalue import mkPropertyValues

from com.sun.star.lang import Locale


# Chart Display Data Labels dialog
class chartDataLabels(UITestCase):
   def test_chart_data_labels_dialog(self):
    with self.ui_test.load_file(get_url_for_data_file("chart.ods")) as calc_doc:
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")

        xDataSeries = calc_doc.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
        self.assertFalse(xDataSeries[0].Label.ShowNumber)
        self.assertFalse(xDataSeries[0].Label.ShowCategoryName)
        self.assertFalse(xDataSeries[0].Label.ShowLegendSymbol)
        self.assertEqual(xDataSeries[0].LabelSeparator, " ")

        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuDataLabels"})) as xDialog:

            xTabs = xDialog.getChild("tabcontrol")
            select_pos(xTabs, "1")

            self.assertTrue(xDataSeries[0].Label.ShowNumber)
            self.assertFalse(xDataSeries[0].Label.ShowCategoryName)
            self.assertFalse(xDataSeries[0].Label.ShowLegendSymbol)
            self.assertEqual(xDataSeries[0].LabelSeparator, " ")

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
            select_by_text(separator, "Comma")

            select_by_text(placement, "Below")
            degrees.executeAction("UP", tuple())
            select_by_text(textDirection, "Right-to-left")


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
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuDataLabels"})) as xDialog:

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


        self.assertTrue(xDataSeries[0].Label.ShowNumber)
        self.assertTrue(xDataSeries[0].Label.ShowCategoryName)
        self.assertTrue(xDataSeries[0].Label.ShowLegendSymbol)
        self.assertEqual(xDataSeries[0].LabelSeparator, ", ")

   def test_chart_data_labels_percentage_dialog(self):
    with self.ui_test.load_file(get_url_for_data_file("dataLabels.ods")) as calc_doc:
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")

        xDataSeries = calc_doc.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
        self.assertTrue(xDataSeries[0].Label.ShowNumber)
        self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)
        self.assertIsNone(xDataSeries[0].PercentageNumberFormat)

        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatDataLabels"})) as xDialog:

            xTabs = xDialog.getChild("tabcontrol")
            select_pos(xTabs, "1")

            self.assertTrue(xDataSeries[0].Label.ShowNumber)
            self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)
            self.assertIsNone(xDataSeries[0].PercentageNumberFormat)

            valueAsNumber = xDialog.getChild("CB_VALUE_AS_NUMBER")
            valueAsPercentage = xDialog.getChild("CB_VALUE_AS_PERCENTAGE")
            buttonPercentage = xDialog.getChild("PB_PERCENT_NUMBERFORMAT")

            valueAsNumber.executeAction("CLICK", tuple())
            valueAsPercentage.executeAction("CLICK", tuple())
            #button Percentage format

            with self.ui_test.execute_blocking_action(buttonPercentage.executeAction, args=('CLICK', ())) as dialog:
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

            self.assertEqual(get_state_as_dict(valueAsNumber)["Selected"], "false")
            self.assertEqual(get_state_as_dict(valueAsPercentage)["Selected"], "true")


        xNumberFormats = calc_doc.Sheets[0].Charts[0].getEmbeddedObject().getNumberFormats()
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
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatDataLabels"})) as xDialog:

            xTabs = xDialog.getChild("tabcontrol")
            select_pos(xTabs, "1")

            self.assertFalse(xDataSeries[0].Label.ShowNumber)
            self.assertTrue(xDataSeries[0].Label.ShowNumberInPercent)
            self.assertEqual(xDataSeries[0].PercentageNumberFormat, xFormat)

            valueAsNumber = xDialog.getChild("CB_VALUE_AS_NUMBER")
            valueAsPercentage = xDialog.getChild("CB_VALUE_AS_PERCENTAGE")

            self.assertEqual(get_state_as_dict(valueAsNumber)["Selected"], "false")
            self.assertEqual(get_state_as_dict(valueAsPercentage)["Selected"], "true")

        self.assertFalse(xDataSeries[0].Label.ShowNumber)
        self.assertTrue(xDataSeries[0].Label.ShowNumberInPercent)
        self.assertEqual(xDataSeries[0].PercentageNumberFormat, xFormat)

   def test_tdf131291(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf131291.ods")) as calc_doc:
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")

        xDataSeries = calc_doc.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
        self.assertTrue(xDataSeries[0].Label.ShowNumber)
        self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)

        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatDataLabels"})) as xDialog:

            xTabs = xDialog.getChild("tabcontrol")
            select_pos(xTabs, "1")

            self.assertTrue(xDataSeries[0].Label.ShowNumber)
            self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)

            xNumberFormatBtn = xDialog.getChild("PB_NUMBERFORMAT")

            with self.ui_test.execute_blocking_action(xNumberFormatBtn.executeAction, args=('CLICK', ())) as dialog:
                categoryformat = dialog.getChild("categorylb")
                formatted = dialog.getChild("formatted")
                sourceformat = dialog.getChild("sourceformat")

                # Select currency
                sourceformat.executeAction("CLICK", tuple());
                categoryformat.getChild("4").executeAction("SELECT", tuple());

                self.assertEqual(get_state_as_dict(categoryformat)["SelectEntryText"], "Currency")

                self.assertEqual(get_state_as_dict(formatted)["Text"], "[$$-409]#,##0.00;[RED]-[$$-409]#,##0.00")


        xNumberFormats = calc_doc.Sheets[0].Charts[0].getEmbeddedObject().getNumberFormats()
        xLocale = Locale()
        xFormat = xNumberFormats.queryKey("[$$-409]#,##0.00;[RED]-[$$-409]#,##0.00", xLocale, True)

        self.assertTrue(xDataSeries[0].Label.ShowNumber)
        self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)
        self.assertEqual(xDataSeries[0].NumberFormat, xFormat)

        #reopen and verify the previous changes
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatDataLabels"})) as xDialog:

            self.assertTrue(xDataSeries[0].Label.ShowNumber)
            self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)
            # tdf#131291: it would fail here: AssertionError: 0 != 104
            self.assertEqual(xDataSeries[0].NumberFormat, xFormat)


        self.assertTrue(xDataSeries[0].Label.ShowNumber)
        self.assertFalse(xDataSeries[0].Label.ShowNumberInPercent)
        self.assertEqual(xDataSeries[0].NumberFormat, xFormat)

   def test_tdf136573(self):
    with self.ui_test.load_file(get_url_for_data_file("dataLabels.ods")):

        with change_measurement_unit(self, "Centimeter"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")

            # Select the first label
            xDataLabel = xChartMain.getChild("CID/MultiClick/CID/D=0:CS=0:CT=0:Series=0:DataLabels=:DataLabel=0")
            xDataLabel.executeAction("SELECT", tuple())

            with self.ui_test.execute_dialog_through_action(xDataLabel, "COMMAND", mkPropertyValues({"COMMAND": "TransformDialog"})) as xDialog:

                self.assertEqual("0.74", get_state_as_dict(xDialog.getChild("MTR_FLD_POS_X"))['Value'])
                self.assertEqual("2.82", get_state_as_dict(xDialog.getChild("MTR_FLD_POS_Y"))['Value'])

                # Use OK button in order to test tdf#137165

            xChartMain.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            xChartMain.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))

            with self.ui_test.execute_dialog_through_action(xDataLabel, "COMMAND", mkPropertyValues({"COMMAND": "TransformDialog"})) as xDialog:

                # Check the position has changed after moving the label using the arrows keys
                self.assertEqual("0.64", get_state_as_dict(xDialog.getChild("MTR_FLD_POS_X"))['Value'])
                self.assertEqual("2.72", get_state_as_dict(xDialog.getChild("MTR_FLD_POS_Y"))['Value'])


# vim: set shiftwidth=4 softtabstop=4 expandtab:
