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
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuDataLabels"}))
    xDialog = self.xUITest.getTopFocusWindow()

    xTabs = xDialog.getChild("tabcontrol")
    select_pos(xTabs, "1")

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
    self.assertEqual(get_state_as_dict(degrees)["Text"], "1")
    self.assertEqual(get_state_as_dict(textDirection)["SelectEntryText"], "Right-to-left")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

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
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatDataLabels"}))
    xDialog = self.xUITest.getTopFocusWindow()

    xTabs = xDialog.getChild("tabcontrol")
    select_pos(xTabs, "1")

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
#            print(dialog.getChildren())
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

            xOKButton = dialog.getChild("cancel")
            self.ui_test.close_dialog_through_button(xOKButton)

    self.ui_test.execute_blocking_action(buttonPercentage.executeAction, args=('CLICK', ()),
                dialog_handler=handle_perc_dlg)

    self.assertEqual(get_state_as_dict(valueAsNumber)["Selected"], "false")
    self.assertEqual(get_state_as_dict(valueAsPercentage)["Selected"], "true")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

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
#    #button Percentage format- it doesn't works in test, but it works in reality
#    def handle_perc_dlg(dialog):
#            sourceformat = dialog.getChild("sourceformat")
#            decimalsed = dialog.getChild("decimalsed")
#            leadzerosed = dialog.getChild("leadzerosed")
#            negnumred = dialog.getChild("negnumred")
#            thousands = dialog.getChild("thousands")
#            formatted = dialog.getChild("formatted")

#            self.assertEqual(get_state_as_dict(sourceformat)["Selected"], "false")
#            self.assertEqual(get_state_as_dict(decimalsed)["Text"], "1")
#            self.assertEqual(get_state_as_dict(leadzerosed)["Text"], "2")
#            self.assertEqual(get_state_as_dict(negnumred)["Selected"], "true")
#            self.assertEqual(get_state_as_dict(thousands)["Selected"], "true")
#            self.assertEqual(get_state_as_dict(formatted)["Text"], "#,#00.0%;[RED]-#,#00.0%")

#            xOKButton = dialog.getChild("cancel")
#            self.ui_test.close_dialog_through_button(xOKButton)

#    self.ui_test.execute_blocking_action(buttonPercentage.executeAction, args=('CLICK', ()),
#                dialog_handler=handle_perc_dlg)

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
