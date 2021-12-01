# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file

class tdf62057(UITestCase):

  def test_tdf62057(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf62057.ods")) as calc_doc:
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")


        xDataMatrix = []
        for column in range(1, 3):
            xColumn = []
            for row in range(0, 6):
                xColumn.append(round(get_cell_by_position(calc_doc, 0, column, row).getValue(), 5))
            xDataMatrix.append(xColumn)

        xRowDescriptions = []
        for row in range(0, 6):
            xRowDescriptions.append(get_cell_by_position(calc_doc, 0, 0, row).getString())

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))

        self.xUITest.executeCommand(".uno:Copy")

    with self.ui_test.load_empty_file("calc") as calc_document:

        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        self.xUITest.executeCommand(".uno:Paste")

        xData = calc_document.Sheets[0].Charts[0].getEmbeddedObject().Data

        xNewDataMatrix = []
        xNewDataMatrix.append([round(item[0], 5) for item in xData.Data])
        xNewDataMatrix.append([round(item[1], 5) for item in xData.Data])

        # Without the fix in place, this test would have failed with
        # AssertionError: Lists differ: ['10m', '11v', '12m', '13m', '14m', '15v'] != ['55.3796', '35.0989']
        self.assertEqual(xRowDescriptions, list(xData.RowDescriptions))

        self.assertEqual(xDataMatrix, xNewDataMatrix)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
