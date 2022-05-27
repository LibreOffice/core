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


# Bug 76731 - ANOVA: F critical is not shown
class anova(UITestCase):
    def test_tdf76731_anova(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf76731.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #1. Open the attached sample document in Calc.
            #2. Select the data range, i.e. A1:C5.
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C5"}))
            #3. Choose Data - Analysis of Variance (ANOVA).
            #4. Choose an arbitrary output cell and click OK.
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AnalysisOfVarianceDialog") as xDialog:
                xinputrangeedit = xDialog.getChild("input-range-edit")
                xoutputrangeedit = xDialog.getChild("output-range-edit")
                xradiotwofactor = xDialog.getChild("radio-two-factor")
                xradiosinglefactor = xDialog.getChild("radio-single-factor")
                xgroupedbyrowsradio = xDialog.getChild("groupedby-rows-radio")
                xgroupedbycolumnsradio = xDialog.getChild("groupedby-columns-radio")
                xalphaspin = xDialog.getChild("alpha-spin")

                xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$F$1"}))
                xradiosinglefactor.executeAction("CLICK", tuple())
                xalphaspin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xalphaspin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xalphaspin.executeAction("TYPE", mkPropertyValues({"TEXT":"0.05"}))
            #Verify, Table of results is shown, "F critical" cell has "#NAME?" error value.
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 0).getString(), "ANOVA - Single Factor")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 1).getString(), "Alpha")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 3).getString(), "Groups")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 4).getString(), "Column 1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 5).getString(), "Column 2")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 6).getString(), "Column 3")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 8).getString(), "Source of Variation")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 9).getString(), "Between Groups")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 10).getString(), "Within Groups")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 11).getString(), "Total")

            self.assertEqual(get_cell_by_position(calc_doc, 0, 11, 8).getString(), "F critical")
            self.assertEqual(round(get_cell_by_position(calc_doc, 0, 11, 9).getValue(),9), 3.885293835)

            #undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 0).getString(), "")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
