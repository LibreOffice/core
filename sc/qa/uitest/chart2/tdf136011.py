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

from libreoffice.uno.propertyvalue import mkPropertyValues


class tdf136011(UITestCase):

   def test_tdf136011(self):

        xObjectNames = ['Object 1', 'Object 2']
        xCategories = ['Test 1', 'Test 2', 'Test 3']
        xSubCategories = [['A', 'B', 'C'], ['1', '2', '3']]
        xColumnNames = ['A', 'B', 'C']

        # Test both charts
        for i, name in enumerate(xObjectNames):

            with self.ui_test.load_file(get_url_for_data_file("tdf136011.ods")) as calc_doc:
                xCalcDoc = self.xUITest.getTopFocusWindow()
                gridwin = xCalcDoc.getChild("grid_window")

                xExpectedResults = []
                for category in xCategories:
                    for subCategory in xSubCategories[i]:
                        xExpectedResults.append(category + ' ' + subCategory)

                gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": name}))

                self.xUITest.executeCommand(".uno:Copy")

            with self.ui_test.load_empty_file("calc") as calc_document:
                xCalcDoc = self.xUITest.getTopFocusWindow()
                gridwin = xCalcDoc.getChild("grid_window")

                self.xUITest.executeCommand(".uno:Paste")

                xData = calc_document.Sheets[0].Charts[0].getEmbeddedObject().Data

                self.assertEqual(xColumnNames, list(xData.ColumnDescriptions))

                # Without the fix in place, the numbers in the categories in chart
                # 'Object 2' wouldn't have been pasted to the new document
                self.assertEqual(xExpectedResults, list(xData.RowDescriptions))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
