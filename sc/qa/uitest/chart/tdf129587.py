# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf129587(UITestCase):
   def test_tdf129587(self):

    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf129587.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")

    document = self.ui_test.get_component()
    xDataSeries = document.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
    self.assertEqual("$Tabelle1.$F$2:$G$11", xDataSeries[0].ErrorBarY.ErrorBarRangeNegative)

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")

    xSeriesObj =  xChartMain.getChild("CID/MultiClick/D=0:CS=0:CT=0:Series=0:ErrorsY=")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatYErrorBars"}))
    xDialog = self.xUITest.getTopFocusWindow()

    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")

    xPerBtn = xDialog.getChild("RB_PERCENT")
    xPerBtn.executeAction("CLICK", tuple())

    xPosField = xDialog.getChild("MF_POSITIVE")
    xNegField = xDialog.getChild("MF_NEGATIVE")

    self.assertEqual("0%", get_state_as_dict(xPosField)['Text'])
    self.assertEqual("0%", get_state_as_dict(xNegField)['Text'])

    #Increase value by one
    xPosField.executeAction("UP", tuple())

    #Both fields are updated because 'Same value for both' is enabled
    self.assertEqual("1%", get_state_as_dict(xPosField)['Text'])
    self.assertEqual("1%", get_state_as_dict(xNegField)['Text'])

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #Without the fix in place, it would have crashed here
    xDataSeries = document.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
    self.assertEqual(1.0, xDataSeries[0].ErrorBarY.PercentageError)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
