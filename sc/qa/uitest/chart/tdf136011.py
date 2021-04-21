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

class tdf136011(UITestCase):

   def test_tdf136011(self):

    xObjectNames = ['Object 1', 'Object 2']
    xCategories = ['Test 1', 'Test 2', 'Test 3']
    xSubCategories = [['A', 'B', 'C'], ['1', '2', '3']]
    xColumnNames = ['A', 'B', 'C']

    # Test both charts
    for i, name in enumerate(xObjectNames):

        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf136011.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        xExpectedResults = []
        for category in xCategories:
            for subCategory in xSubCategories[i]:
                xExpectedResults.append(category + ' ' + subCategory)

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": name}))

        self.xUITest.executeCommand(".uno:Copy")

        self.xUITest.executeCommand(".uno:CloseDoc")

        self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        self.xUITest.executeCommand(".uno:Paste")

        xData = document.Sheets[0].Charts[0].getEmbeddedObject().Data

        self.assertEqual(xColumnNames, list(xData.ColumnDescriptions))

        # Without the fix in place, the numbers in the categories in chart
        # 'Object 2' wouldn't have been pasted to the new document
        self.assertEqual(xExpectedResults, list(xData.RowDescriptions))

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
