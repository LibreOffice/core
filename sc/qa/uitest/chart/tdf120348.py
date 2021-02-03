# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file

class tdf120348(UITestCase):

   def test_tdf120348(self):

    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf120348.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    xFirstMatrix = []
    for row in range(1, 159):
        xRow = []
        for column in range(5, 9):
            xRow.append(round(get_cell_by_position(document, 0, column, row).getValue(), 5))
        xFirstMatrix.append(xRow)

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 2"}))

    self.xUITest.executeCommand(".uno:Copy")

    self.ui_test.close_doc()

    self.ui_test.load_empty_file("calc")
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    self.xUITest.executeCommand(".uno:Paste")

    xData = document.Sheets[0].Charts[0].getEmbeddedObject().Data

    columnNames = ('Finland', 'Sweden', 'Poland', '')
    self.assertEqual(columnNames, xData.ColumnDescriptions)

    xSecondMatrix = []
    for row in xData.Data:
        xRow = []
        for value in row:
            xRow.append(round(value, 5))
        xSecondMatrix.append(xRow)

    # Without the fix in place, this test would have failed with
    # First differing element 51:
    # [3.31618, 3.65089, 3.33626, 0.0]
    # [3.31618, 3.65089, 0.0, 0.0]

    self.assertEqual(xFirstMatrix, xSecondMatrix)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
