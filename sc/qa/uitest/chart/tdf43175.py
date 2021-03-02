# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

class Tdf43175(UITestCase):

    def test_tdf43175(self):
        self.ui_test.load_file(get_url_for_data_file("tdf43175.ods"))

        self.xUITest.executeCommand(".uno:TableSelectAll")

        self.ui_test.execute_dialog_through_command(".uno:Move")
        xDialog = self.xUITest.getTopFocusWindow()
        insertBefore = xDialog.getChild("insertBefore")

        # Select - move to end position -
        xTreeEntry = insertBefore.getChild('2')
        xTreeEntry.executeAction("SELECT", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        document = self.ui_test.get_component()

        aSheetNames = ['Blad1', 'Blad2', 'Blad1_2', 'Blad2_2']

        self.assertEqual(4, document.Sheets.getCount())
        for i in range(4):
            self.assertEqual(aSheetNames[i], document.Sheets[i].Name)

            xChart = document.Sheets[i].Charts[0]
            xDataSeries = xChart.getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
            self.assertEqual(1, len(xDataSeries))

            # Without the fix in place, this test would have failed with
            # AssertionError: '$Blad1_2.$A$1:$A$5' != '$Blad1.$A$1:$A$5'
            aExpectedRangeName = '$' + aSheetNames[i] + '.$A$1:$A$5'
            self.assertEqual(aExpectedRangeName, xDataSeries[0].DataSequences[0].Values.SourceRangeRepresentation)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
