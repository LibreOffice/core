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
from libreoffice.uno.propertyvalue import mkPropertyValues

class goToSheet(UITestCase):
    def test_go_to_sheet(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:

            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")

            for i in range(3):
                with self.ui_test.execute_dialog_through_command(".uno:Insert"):
                    pass

            with self.ui_test.execute_dialog_through_command(".uno:RenameTable") as xDialogRename:
                xname_entry = xDialogRename.getChild("name_entry")
                xname_entry.executeAction("TYPE", mkPropertyValues({"TEXT":"NewName"}))

            with self.ui_test.execute_dialog_through_command(".uno:JumpToTable") as xDialogGoToSheet:
                xentry_mask = xDialogGoToSheet.getChild("entry-mask")
                xtree_view = xDialogGoToSheet.getChild("treeview")

                # Empty mask lists all sheets
                self.assertEqual(get_state_as_dict(xtree_view)["Children"], "4")

                # None sheet contains 'E': Search entry is case sensitive
                xentry_mask.executeAction("TYPE", mkPropertyValues({"TEXT":"E"}))
                self.assertEqual(get_state_as_dict(xtree_view)["Children"], "0")

                xentry_mask.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                self.assertEqual(get_state_as_dict(xtree_view)["Children"], "4")

                # All sheets contains 'e'
                xentry_mask.executeAction("TYPE", mkPropertyValues({"TEXT":"e"}))
                self.assertEqual(get_state_as_dict(xtree_view)["Children"], "4")

                # List Sheet3, Sheet2 and Sheet1
                xentry_mask.executeAction("TYPE", mkPropertyValues({"TEXT":"e"}))
                self.assertEqual(get_state_as_dict(xtree_view)["Children"], "3")

            #if no name selected, active sheet remains unchanged
            self.assertEqual(get_state_as_dict(xGridWindow)["SelectedTable"], "0")

            with self.ui_test.execute_dialog_through_command(".uno:JumpToTable") as xDialogGoToSheet:
                xentry_mask = xDialogGoToSheet.getChild("entry-mask")
                xtree_view = xDialogGoToSheet.getChild("treeview")
                # List Sheet3, Sheet2 and Sheet1
                xentry_mask.executeAction("TYPE", mkPropertyValues({"TEXT":"ee"}))
                self.assertEqual(get_state_as_dict(xtree_view)["Children"], "3")
                # Select Sheet2
                xtree_view.getChild('1').executeAction("SELECT", tuple())

            self.assertEqual(get_state_as_dict(xGridWindow)["SelectedTable"], "2")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
