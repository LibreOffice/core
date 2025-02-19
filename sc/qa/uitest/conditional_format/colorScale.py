# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import select_by_text, type_text

class colorScale(UITestCase):

    def test_colorScale3Entries(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            cond_formats = document.Sheets[0].ConditionalFormats
            self.assertEqual(0, cond_formats.Length)

            enter_text_to_cell(gridwin, "A1", "1")
            enter_text_to_cell(gridwin, "A2", "2")
            enter_text_to_cell(gridwin, "A3", "3")

            self.xUITest.executeCommand(".uno:SelectColumn")

            with self.ui_test.execute_dialog_through_command(".uno:ColorScaleFormatDialog") as xCondFormat:
                self.assertEqual("Condition 1", get_state_as_dict(xCondFormat.getChild("number"))["Text"])
                self.assertEqual("All Cells", get_state_as_dict(xCondFormat.getChild("type"))["SelectEntryText"])
                self.assertEqual("Color Scale (3 Entries)", get_state_as_dict(xCondFormat.getChild("colorformat"))["SelectEntryText"])
                self.assertEqual("Min", get_state_as_dict(xCondFormat.getChild("colscalemin"))["SelectEntryText"])
                self.assertEqual("Percentile", get_state_as_dict(xCondFormat.getChild("colscalemiddle"))["SelectEntryText"])
                self.assertEqual("Max", get_state_as_dict(xCondFormat.getChild("colscalemax"))["SelectEntryText"])
                self.assertEqual("Red", get_state_as_dict(xCondFormat.getChild("lbcolmin"))["Text"])
                self.assertEqual("Yellow", get_state_as_dict(xCondFormat.getChild("lbcolmiddle"))["Text"])
                self.assertEqual("Green", get_state_as_dict(xCondFormat.getChild("lbcolmax"))["Text"])
                self.assertEqual("", get_state_as_dict(xCondFormat.getChild("edcolscalemin"))["Text"])
                self.assertEqual("50", get_state_as_dict(xCondFormat.getChild("edcolscalemiddle"))["Text"])
                self.assertEqual("", get_state_as_dict(xCondFormat.getChild("edcolscalemax"))["Text"])

            self.assertEqual(1, cond_formats.Length)
            self.assertEqual(3, len(cond_formats.ConditionalFormats[0].getByIndex(0).ColorScaleEntries))
            self.assertEqual(16711680, cond_formats.ConditionalFormats[0].getByIndex(0).ColorScaleEntries[0].Color)
            self.assertEqual(16776960, cond_formats.ConditionalFormats[0].getByIndex(0).ColorScaleEntries[1].Color)
            self.assertEqual(43315, cond_formats.ConditionalFormats[0].getByIndex(0).ColorScaleEntries[2].Color)

    def test_colorScale2Entries(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            cond_formats = document.Sheets[0].ConditionalFormats
            self.assertEqual(0, cond_formats.Length)

            enter_text_to_cell(gridwin, "A1", "1")
            enter_text_to_cell(gridwin, "A2", "2")
            enter_text_to_cell(gridwin, "A3", "3")

            self.xUITest.executeCommand(".uno:SelectColumn")

            with self.ui_test.execute_dialog_through_command(".uno:ColorScaleFormatDialog", close_button="") as xCondFormat:
                self.assertEqual("Condition 1", get_state_as_dict(xCondFormat.getChild("number"))["Text"])
                self.assertEqual("All Cells", get_state_as_dict(xCondFormat.getChild("type"))["SelectEntryText"])

                select_by_text(xCondFormat.getChild("colorformat"), "Color Scale (2 Entries)")

                # we need to get a pointer again after changing the color format
                xCondFormat = self.xUITest.getTopFocusWindow()

                self.assertEqual("Color Scale (2 Entries)", get_state_as_dict(xCondFormat.getChild("colorformat"))["SelectEntryText"])
                self.assertEqual("Min", get_state_as_dict(xCondFormat.getChild("colscalemin"))["SelectEntryText"])
                self.assertEqual("Max", get_state_as_dict(xCondFormat.getChild("colscalemax"))["SelectEntryText"])
                self.assertEqual("Light Yellow 2", get_state_as_dict(xCondFormat.getChild("lbcolmin"))["Text"])
                self.assertEqual("Light Green 2", get_state_as_dict(xCondFormat.getChild("lbcolmax"))["Text"])
                self.assertEqual("", get_state_as_dict(xCondFormat.getChild("edcolscalemin"))["Text"])
                self.assertEqual("", get_state_as_dict(xCondFormat.getChild("edcolscalemax"))["Text"])

                # close the conditional format manager
                xOKBtn = xCondFormat.getChild("ok")
                self.ui_test.close_dialog_through_button(xOKBtn)

            self.assertEqual(1, cond_formats.Length)
            self.assertEqual(2, len(cond_formats.ConditionalFormats[0].getByIndex(0).ColorScaleEntries))
            self.assertEqual(16777069, cond_formats.ConditionalFormats[0].getByIndex(0).ColorScaleEntries[0].Color)
            self.assertEqual(7847013, cond_formats.ConditionalFormats[0].getByIndex(0).ColorScaleEntries[1].Color)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
