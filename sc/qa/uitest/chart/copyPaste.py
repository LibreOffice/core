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


class CopyPaste(UITestCase):

  def test_copy_paste_chart_with_dot_in_sheet_name(self):
    with self.ui_test.load_file(get_url_for_data_file("chartWithDotInSheetName.ods")) as calc_doc:

        xChart = calc_doc.Sheets[0].Charts[0]
        xDataSeries = xChart.getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries

        self.assertEqual(4, len(xDataSeries))

        xOldSheetRanges = []
        for i in range(4):
            xRow = []
            xDS = xDataSeries[i].DataSequences

            self.assertEqual(1, len(xDS))
            xRow.append(xDS[0].Values.SourceRangeRepresentation)
            xOldSheetRanges.append(xRow)

        self.xUITest.executeCommand(".uno:SelectAll")

        self.xUITest.executeCommand(".uno:Copy")

    with self.ui_test.load_empty_file("calc") as calc_document:

        # Rename the sheet to match the same name as the first document
        with self.ui_test.execute_dialog_through_command(".uno:RenameTable") as xDialog:
            xname_entry = xDialog.getChild("name_entry")

            xname_entry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
            xname_entry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
            xname_entry.executeAction("TYPE", mkPropertyValues({"TEXT":"Sheet.1"}))

        self.xUITest.executeCommand(".uno:Paste")

        xChart = calc_document.Sheets[0].Charts[0]
        xDataSeries = xChart.getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries

        self.assertEqual(4, len(xDataSeries))

        xNewSheetRanges = []
        for i in range(4):
            xRow = []
            xDS = xDataSeries[i].DataSequences

            self.assertEqual(1, len(xDS))
            xRow.append(xDS[0].Values.SourceRangeRepresentation)
            xNewSheetRanges.append(xRow)

        # Without the fix in place, this test would have failed with
        # ["$'Sheet.1'.$B$12:$B$18"]
        # ["'file:///home/<user>/Documents/Sheet.1'#$Sheet1.$B$12:$B$18"]
        self.assertEqual(xOldSheetRanges, xNewSheetRanges)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
