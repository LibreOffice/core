# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Chart Wall dialog

class chartWall(UITestCase):
   def test_chart_wall_dialog(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf122398.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    change_measurement_unit(self, "Centimeter")

    xWall = document.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().Wall
    self.assertEqual(xWall.LineWidth, 0)
    self.assertEqual(xWall.LineTransparence, 0)
    self.assertEqual(hex(xWall.FillColor), '0xe6e6e6')
    self.assertEqual(xWall.FillTransparence, 0)

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramWall"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Borders".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")

    xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
    transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

    xWidth.executeAction("UP", tuple())
    transparency.executeAction("UP", tuple())

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.assertEqual(xWall.LineWidth, 100)
    self.assertEqual(xWall.LineTransparence, 5)
    self.assertEqual(hex(xWall.FillColor), '0xe6e6e6')
    self.assertEqual(xWall.FillTransparence, 0)

    #reopen and verify tab "Borders".
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramWall"}))
    xDialog = self.xUITest.getTopFocusWindow()

    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")
    xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
    transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

    self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
    self.assertEqual(get_state_as_dict(transparency)["Text"], "5%")

    self.assertEqual(xWall.LineWidth, 100)
    self.assertEqual(xWall.LineTransparence, 5)
    self.assertEqual(hex(xWall.FillColor), '0xe6e6e6')
    self.assertEqual(xWall.FillTransparence, 0)

    #Click on tab "Area"
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "1")

    btncolor = xDialog.getChild("btncolor")
    btncolor.executeAction("CLICK", tuple())

    rCustom = xDialog.getChild("R_custom")
    gCustom = xDialog.getChild("G_custom")
    bCustom = xDialog.getChild("B_custom")

    rCustom.executeAction("CLEAR", tuple())
    rCustom.executeAction("TYPE", mkPropertyValues({"TEXT":"35"}))
    rCustom.executeAction("UP", tuple())
    rCustom.executeAction("DOWN", tuple())  #without this save data doesn't works
    self.assertEqual(get_state_as_dict(rCustom)["Text"], "35")
    gCustom.executeAction("CLEAR", tuple())
    gCustom.executeAction("TYPE", mkPropertyValues({"TEXT":"169"}))
    gCustom.executeAction("UP", tuple())
    gCustom.executeAction("DOWN", tuple())  #without this save data doesn't works
    bCustom.executeAction("CLEAR", tuple())
    bCustom.executeAction("TYPE", mkPropertyValues({"TEXT":"211"}))
    bCustom.executeAction("UP", tuple())
    bCustom.executeAction("DOWN", tuple())  #without this save data doesn't works
    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.assertEqual(xWall.LineWidth, 100)
    self.assertEqual(xWall.LineTransparence, 5)
    self.assertEqual(hex(xWall.FillColor), '0x23a9d3')
    self.assertEqual(xWall.FillTransparence, 0)

    #reopen and verify tab "Area".
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramWall"}))
    xDialog = self.xUITest.getTopFocusWindow()

    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "1")

    rCustom = xDialog.getChild("R_custom")
    gCustom = xDialog.getChild("G_custom")
    bCustom = xDialog.getChild("B_custom")

    self.assertEqual(get_state_as_dict(rCustom)["Text"], "35")
    self.assertEqual(get_state_as_dict(gCustom)["Text"], "169")
    self.assertEqual(get_state_as_dict(bCustom)["Text"], "211")

    self.assertEqual(xWall.LineWidth, 100)
    self.assertEqual(xWall.LineTransparence, 5)
    self.assertEqual(hex(xWall.FillColor), '0x23a9d3')
    self.assertEqual(xWall.FillTransparence, 0)

    #change tab "Transparency"
    select_pos(tabcontrol, "2")
    transparency = xDialog.getChild("RBT_TRANS_LINEAR")
    transparencyPercent = xDialog.getChild("MTR_TRANSPARENT")  #51%

    transparency.executeAction("CLICK", tuple())
    transparencyPercent.executeAction("UP", tuple())

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.assertEqual(xWall.LineWidth, 100)
    self.assertEqual(xWall.LineTransparence, 5)
    self.assertEqual(hex(xWall.FillColor), '0x23a9d3')
    self.assertEqual(xWall.FillTransparence, 51)

    #reopen and verify tab "Transparency"
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramWall"}))
    xDialog = self.xUITest.getTopFocusWindow()

    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "2")

    transparency = xDialog.getChild("RBT_TRANS_LINEAR")
    transparencyPercent = xDialog.getChild("MTR_TRANSPARENT")  #51%

    self.assertEqual(get_state_as_dict(transparency)["Checked"], "true")
    self.assertEqual(get_state_as_dict(transparencyPercent)["Text"], "51%")

    self.assertEqual(xWall.LineWidth, 100)
    self.assertEqual(xWall.LineTransparence, 5)
    self.assertEqual(hex(xWall.FillColor), '0x23a9d3')
    self.assertEqual(xWall.FillTransparence, 51)

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.assertEqual(xWall.LineWidth, 100)
    self.assertEqual(xWall.LineTransparence, 5)
    self.assertEqual(hex(xWall.FillColor), '0x23a9d3')
    self.assertEqual(xWall.FillTransparence, 51)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
