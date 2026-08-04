# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
import os.path
import xml.etree.ElementTree as ET
import zipfile
from tempfile import TemporaryDirectory

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

from libreoffice.uno.propertyvalue import mkPropertyValues
from org.libreoffice.unotest import systemPathToFileUrl

CX_NS = "{http://schemas.microsoft.com/office/drawing/2014/chartex}"
X_NS = "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}"


class createFunnelChart(UITestCase):
    def test_create_funnel_chart_to_xlsx(self):
        # Create funnel chart through the chart wizard
        with TemporaryDirectory() as tempdir:
            sFilePath = os.path.join(tempdir, "createFunnelChart-tmp.xlsx")

            with self.ui_test.load_file(get_url_for_data_file("funnel_chart_data.fods")) as calc_doc:
                xCalcDoc = self.xUITest.getTopFocusWindow()
                gridwin = xCalcDoc.getChild("grid_window")
                gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B5"}))

                with self.ui_test.execute_dialog_through_command(
                        ".uno:InsertObjectChart", close_button="finish") as xDialog:
                    xChartTypeList = xDialog.getChild("charttype")
                    for nIndex in range(len(xChartTypeList.getChildren())):
                        xEntry = xChartTypeList.getChild(str(nIndex))
                        if get_state_as_dict(xEntry)["Text"] == "Funnel":
                            xEntry.executeAction("SELECT", mkPropertyValues({}))
                            break
                    else:
                        self.fail("no Funnel entry in the chart type list")

                # click outside the chart to leave edit mode
                gridwin.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

                calc_doc.storeToURL(systemPathToFileUrl(sFilePath),
                                    mkPropertyValues({"FilterName": "Calc MS Excel 2007 XML"}))

            with zipfile.ZipFile(sFilePath) as xZip:
                # the chartex data part references the category and value ranges by name
                xChartEx = ET.fromstring(xZip.read("xl/charts/chartEx1.xml"))
                sDataPath = f"./{CX_NS}chartData/{CX_NS}data/"
                xCatFormula = xChartEx.find(sDataPath + f"{CX_NS}strDim[@type='cat']/{CX_NS}f")
                self.assertIsNotNone(xCatFormula)
                sCatName = xCatFormula.text
                self.assertTrue(sCatName.startswith("_xlchart.v2."),
                                "got category reference: " + sCatName)
                xValFormula = xChartEx.find(sDataPath + f"{CX_NS}numDim[@type='val']/{CX_NS}f")
                self.assertIsNotNone(xValFormula)
                sValName = xValFormula.text
                self.assertTrue(sValName.startswith("_xlchart.v2."),
                                "got value reference: " + sValName)
                self.assertNotEqual(sCatName, sValName)

                # each reference has a matching hidden defined name holding the source range
                xWorkbook = ET.fromstring(xZip.read("xl/workbook.xml"))
                aNames = {}
                for xName in xWorkbook.iterfind(f"./{X_NS}definedNames/{X_NS}definedName"):
                    aNames[xName.get("name")] = (xName.get("hidden"), xName.text)
                self.assertEqual(("true", "Sheet1!$A$2:$A$5"), aNames.get(sCatName))
                self.assertEqual(("true", "Sheet1!$B$2:$B$5"), aNames.get(sValName))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
