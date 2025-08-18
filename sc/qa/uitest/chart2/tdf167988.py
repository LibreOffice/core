# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

from libreoffice.uno.propertyvalue import mkPropertyValues


class tdf167988(UITestCase):

    def test_tdf167988(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf167988.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            xChart = calc_doc.Sheets[0].Charts[0]
            xDataSeries = xChart.getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
            self.assertEqual(3, len(xDataSeries))

            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")

            xMean = xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=2:Average=0")
            xMean.executeAction("SELECT", tuple())

            # Without the fix in place, this test would have crashed here
            xChartMain.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DELETE"}))

            xTrend = xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=1:Curve=0")
            xTrend.executeAction("SELECT", tuple())
            xChartMain.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DELETE"}))

            xSeries = xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=2")
            xSeries.executeAction("SELECT", tuple())
            xChartMain.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DELETE"}))

            xDataSeries = xChart.getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
            print(dir(xDataSeries[1]))
            self.assertEqual(2, len(xDataSeries))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
