# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file

# Test pivot chart is pasted correctly to a new document

class tdf107097(UITestCase):

  def test_tdf107097(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf107097.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")

    document = self.ui_test.get_component()

    xFirstMatrix = []
    for row in range(6, 18):
        xRow = []
        for column in range(3, 7):
            xRow.append(get_cell_by_position(document, 1, column, row).getValue())
        xFirstMatrix.append(xRow)

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))

    self.xUITest.executeCommand(".uno:Copy")

    self.xUITest.executeCommand(".uno:CloseDoc")

    self.ui_test.create_doc_in_start_center("calc")
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    self.xUITest.executeCommand(".uno:Paste")

    xData = document.Sheets[0].Charts[0].getEmbeddedObject().Data

    xSecondMatrix = []
    for row in xData.Data:
        xRow = []
        for value in row:
            xRow.append(value)
        xSecondMatrix.append(xRow)

    self.assertEqual(xFirstMatrix, xSecondMatrix)

    aExpectedColumnDescriptions = ('Sum - Sales T1', 'Sum - Sales T2',
            'Sum - Sales T3', 'Sum - Sales T4')
    aExpectedRowDescriptions = ('DE Berlin A', 'DE Berlin B', 'DE Munich A',
            'DE Munich B', 'EN Glasgow A', 'EN Liverpool B', 'EN London A',
            'EN London B', 'FR Nantes A', 'FR Nantes B', 'FR Paris A', 'FR Paris B')

    self.assertEqual(aExpectedColumnDescriptions, xData.ColumnDescriptions)
    self.assertEqual(aExpectedRowDescriptions, xData.RowDescriptions)

    self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
