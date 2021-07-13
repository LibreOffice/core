# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

class NamedRangesChart(UITestCase):

  def test_chart_series_ranges_with_named_ranges_as_datasource(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf64086.xlsx")) as calc_doc:

        xChart = calc_doc.Sheets[0].Charts[0]
        xDataSeries = xChart.getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries

        self.assertEqual(3, len(xDataSeries))

        test_range1 = calc_doc.Sheets[0].NamedRanges.getByName("Local_sheet_name")
        self.assertEqual(xDataSeries[0].DataSequences[0].Values.SourceRangeRepresentation, test_range1.Name)

        test_range2 = calc_doc.NamedRanges.getByName("global_name")
        self.assertEqual(xDataSeries[1].DataSequences[0].Values.SourceRangeRepresentation, test_range2.Name)

        test_range3 = calc_doc.Sheets[1].NamedRanges.getByName("other_sheet_name")
        test_range3_Name = calc_doc.Sheets[1].Name + '.' + test_range3.Name
        self.assertEqual(xDataSeries[2].DataSequences[0].Values.SourceRangeRepresentation, test_range3_Name)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
