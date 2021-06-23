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

    with self.ui_test.load_file(get_url_for_data_file("tdf129587.ods")) as calc_doc:
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        xDataSeries = calc_doc.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
        self.assertEqual("$Tabelle1.$F$2:$G$11", xDataSeries[0].ErrorBarY.ErrorBarRangeNegative)

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")

        xSeriesObj =  xChartMain.getChild("CID/MultiClick/D=0:CS=0:CT=0:Series=0:ErrorsY=")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatYErrorBars"})) as xDialog:

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


        #Without the fix in place, it would have crashed here
        xDataSeries = calc_doc.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
        self.assertEqual(1.0, xDataSeries[0].ErrorBarY.PercentageError)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
