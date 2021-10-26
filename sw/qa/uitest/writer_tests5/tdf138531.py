# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell

class Tdf138531(UITestCase):

    def test_tdf138531(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #enter data
            enter_text_to_cell(gridwin, "A1", "First")
            enter_text_to_cell(gridwin, "A2", "Second")

            #select A1:A2
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))

            self.xUITest.executeCommand(".uno:Copy")

            # Work with both documents at the same time
            with self.ui_test.load_empty_file("writer") as writer_doc:

                # Paste as DDE
                formatProperty = mkPropertyValues({"SelectedFormat": 59})
                self.xUITest.executeCommandWithParameters(".uno:ClipboardFormatItems", formatProperty)

                self.assertEqual(1, writer_doc.TextTables.getCount())
                table = writer_doc.getTextTables()[0]
                self.assertEqual("First", table.getCellByName("A1").getString())
                self.assertEqual("Second", table.getCellByName("A2").getString())

                frames = self.ui_test.get_frames()
                # switch view to the calc document
                frames[0].activate()
                enter_text_to_cell(gridwin, "A1", "Second")
                enter_text_to_cell(gridwin, "A2", "First")

                # switch view back to the writer document
                frames[1].activate()

                with self.ui_test.execute_dialog_through_command(".uno:LinkDialog", close_button="close") as xDialog:
                    xLinks = xDialog.getChild("TB_LINKS")
                    self.assertEqual(1, len(xLinks.getChildren()))

                    xFileName = xDialog.getChild("FULL_FILE_NAME")
                    self.assertEqual("Untitled 1", get_state_as_dict(xFileName)["Text"])

                    xUpdate = xDialog.getChild("UPDATE_NOW")
                    xUpdate.executeAction("CLICK", tuple())

                # Without the fix in place, this test would have failed with
                # AssertionError: 'Second' != 'First'
                self.assertEqual("Second", table.getCellByName("A1").getString())
                self.assertEqual("First", table.getCellByName("A2").getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
