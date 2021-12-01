# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

class tdf93068(UITestCase):

    def test_tdf93068(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:


            self.xUITest.getTopFocusWindow()

            # tdf#135950: Character dialog crashes if multiple cells in a
            # table are selected
            with self.ui_test.execute_dialog_through_command(".uno:InsertTable"):
                pass

            self.xUITest.executeCommand(".uno:SelectAll")

            # Check the table is selected
            self.assertEqual("SwXTextTableCursor", document.CurrentSelection.getImplementationName())

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog"):
                pass

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog", close_button="cancel"):
                pass

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xFontDlg:
                xDiscardBtn = xFontDlg.getChild("reset")
                xDiscardBtn.executeAction("CLICK", tuple())

            self.xUITest.getTopFocusWindow()

            # Check the table is still selected after closing the dialog
            self.assertEqual("SwXTextTableCursor", document.CurrentSelection.getImplementationName())

            self.xUITest.executeCommand(".uno:GoDown")

            # Check the table is no longer selected
            self.assertNotEqual("SwXTextTableCursor", document.CurrentSelection.getImplementationName())

