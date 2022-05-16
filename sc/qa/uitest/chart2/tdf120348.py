# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


class tdf120348(UITestCase):

    def test_tdf120348(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf120348.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            xFirstMatrix = []
            for row in range(1, 159):
                xRow = []
                for column in range(5, 9):
                    xRow.append(round(get_cell_by_position(calc_doc, 0, column, row).getValue(), 5))
                xFirstMatrix.append(xRow)

            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 2"}))

            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_empty_file("calc") as calc_document:

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            self.xUITest.executeCommand(".uno:Paste")

            xData = calc_document.Sheets[0].Charts[0].getEmbeddedObject().Data

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

# vim: set shiftwidth=4 softtabstop=4 expandtab:
