# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues


class tdf122011(UITestCase):

    def test_tdf122011(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            # FIXME: tdf#148166
            enter_text_to_cell(gridwin, "A2", "4")

            # use an existing document
            xReference = "'" + get_url_for_data_file("tdf119954.ods") + "'#$Sheet1.$A$1"

            with self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart", close_button="finish") as xChartDlg:
                xNextBtn = xChartDlg.getChild("next")
                xNextBtn.executeAction("CLICK", tuple())

                xRange = xChartDlg.getChild("ED_RANGE")
                xRange.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xRange.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xRange.executeAction("TYPE", mkPropertyValues({"TEXT": xReference}))

                # Without the fix in place, this test would have gotten stack here because the dialog is not closed
                # as the reference to an external file is incorrect

            xChart = document.Sheets[0].Charts[0]
            xDataSeries = xChart.getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries

            self.assertEqual(1, len(xDataSeries))

            xDS = xDataSeries[0].DataSequences

            self.assertEqual(1, len(xDS))
            self.assertEqual(xReference, xDS[0].Values.SourceRangeRepresentation)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
