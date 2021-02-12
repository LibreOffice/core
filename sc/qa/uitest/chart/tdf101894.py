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
from uitest.uihelper.common import get_state_as_dict

class tdf101894(UITestCase):

  def test_tdf101894(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf101894.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")

    document = self.ui_test.get_component()

    xChart = document.Sheets[0].Charts[0]
    xDataSeries = xChart.getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries

    self.assertEqual(4, len(xDataSeries))

    xOldSheetRanges = []
    for i in range(4):
        xRow = []
        xDS = xDataSeries[i].DataSequences

        self.assertEqual(2, len(xDS))
        xRow.append(xDS[0].Values.SourceRangeRepresentation)
        xRow.append(xDS[1].Values.SourceRangeRepresentation)
        xOldSheetRanges.append(xRow)

    # Rename the sheet first
    self.ui_test.execute_dialog_through_command(".uno:RenameTable")
    xDialog = self.xUITest.getTopFocusWindow()
    xname_entry = xDialog.getChild("name_entry")

    oldName = get_state_as_dict(xname_entry)['Text']
    xname_entry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
    xname_entry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
    xname_entry.executeAction("TYPE", mkPropertyValues({"TEXT":"NewName"}))
    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    # Copy sheet and use the old name
    self.ui_test.execute_dialog_through_command(".uno:Move")
    xDialog = self.xUITest.getTopFocusWindow()
    newName = xDialog.getChild("newName")
    newName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
    newName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
    newName.executeAction("TYPE", mkPropertyValues({"TEXT": oldName}))
    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    xChart = document.Sheets[0].Charts[0]
    xDataSeries = xChart.getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries

    self.assertEqual(4, len(xDataSeries))

    xNewSheetRanges = []
    for i in range(4):
        xRow = []
        xDS = xDataSeries[i].DataSequences

        self.assertEqual(2, len(xDS))
        xRow.append(xDS[0].Values.SourceRangeRepresentation)
        xRow.append(xDS[1].Values.SourceRangeRepresentation)
        xNewSheetRanges.append(xRow)

    self.assertEqual(xOldSheetRanges, xNewSheetRanges)

    self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
