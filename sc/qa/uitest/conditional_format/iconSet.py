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

class IconSet(UITestCase):

    def test_IconSet(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            cond_formats = document.Sheets[0].ConditionalFormats
            self.assertEqual(0, cond_formats.Length)

            enter_text_to_cell(gridwin, "A1", "1")
            enter_text_to_cell(gridwin, "A2", "2")
            enter_text_to_cell(gridwin, "A3", "3")

            self.xUITest.executeCommand(".uno:SelectColumn")

            with self.ui_test.execute_dialog_through_command(".uno:IconSetFormatDialog", close_button="") as xCondFormat:
                self.assertEqual("Condition 1", get_state_as_dict(xCondFormat.getChild("number"))["Text"])
                self.assertEqual("All Cells", get_state_as_dict(xCondFormat.getChild("type"))["SelectEntryText"])
                self.assertEqual("Icon Set", get_state_as_dict(xCondFormat.getChild("colorformat"))["SelectEntryText"])
                self.assertEqual("3 Arrows", get_state_as_dict(xCondFormat.getChild("iconsettype"))["SelectEntryText"])
                select_by_text(xCondFormat.getChild("iconsettype"), "4 Gray Arrows")

                # we need to get a pointer again after changing the IconSet type
                xCondFormat = self.xUITest.getTopFocusWindow()

                self.assertEqual("Percent", get_state_as_dict(xCondFormat.getChild("listbox1"))["SelectEntryText"])
                self.assertEqual("Percent", get_state_as_dict(xCondFormat.getChild("listbox2"))["SelectEntryText"])
                self.assertEqual("Percent", get_state_as_dict(xCondFormat.getChild("listbox3"))["SelectEntryText"])
                self.assertEqual("", get_state_as_dict(xCondFormat.getChild("entry1"))["Text"])
                self.assertEqual("", get_state_as_dict(xCondFormat.getChild("entry2"))["Text"])
                self.assertEqual("", get_state_as_dict(xCondFormat.getChild("entry3"))["Text"])

                # close the conditional format manager
                xOKBtn = xCondFormat.getChild("ok")
                self.ui_test.close_dialog_through_button(xOKBtn)

            self.assertEqual(1, cond_formats.Length)
            self.assertEqual(11, cond_formats.ConditionalFormats[0].getByIndex(0).Icons) # IconSet_4ArrowsGray
            self.assertEqual(4, len(cond_formats.ConditionalFormats[0].getByIndex(0).IconSetEntries))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
