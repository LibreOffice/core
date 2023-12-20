#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues


# Test for SvxTableController.
class SvxTableControllerTest(UITestCase):

    def testOnFormatTable(self):
        # Create an Impress document with a single table in it.
        with self.ui_test.create_doc_in_start_center("impress") as component:
            template = self.xUITest.getTopFocusWindow()
            self.ui_test.close_dialog_through_button(template.getChild("close"))
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:Delete")
            self.xUITest.executeCommand(".uno:InsertTable?Columns:short=2&Rows:short=2")

            # Enable shadow.
            with self.ui_test.execute_dialog_through_command(".uno:TableDialog") as tableDialog:
                tabs = tableDialog.getChild("tabcontrol")
                # Select "shadow".
                select_pos(tabs, "4")
                shadowCheckbox = tableDialog.getChild("TSB_SHOW_SHADOW")
                shadowCheckbox.executeAction("CLICK", tuple())

            # Check if the shadow was enabled.
            shape = component.getDrawPages()[0][0]
            # Without the accompanying fix in place, this test would have failed with:
            # AssertionError: False != True
            # i.e. the table still had no shadow.
            self.assertEqual(shape.Shadow, True)

            # Close the document.

    def testUndoCrash(self):
        # Given an Impress document with a single table in it:
        with self.ui_test.create_doc_in_start_center("impress"):
            template = self.xUITest.getTopFocusWindow()
            self.ui_test.close_dialog_through_button(template.getChild("close"))
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:Delete")
            self.xUITest.executeCommand(".uno:InsertTable?Columns:short=3&Rows:short=3")
            self.xUITest.executeCommand(".uno:SelectAll")

            # When enabling shadow on the shape while text edit is active:
            doc = self.xUITest.getTopFocusWindow()
            impress = doc.getChild("impress_win")
            impress.executeAction("TYPE", mkPropertyValues({"TEXT": "A1"}))
            for i in range(6):
                impress.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+TAB"}))
            impress.executeAction("TYPE", mkPropertyValues({"TEXT": "A3"}))
            self.xUITest.executeCommand(".uno:SelectAll")
            with self.ui_test.execute_dialog_through_command(".uno:TableDialog") as tableDialog:
                tabs = tableDialog.getChild("tabcontrol")
                # Select "shadow".
                select_pos(tabs, "4")
                shadowCheckbox = tableDialog.getChild("TSB_SHOW_SHADOW")
                shadowCheckbox.executeAction("CLICK", tuple())

            # Then make sure we don't crash:
            # Without the accompanying fix in place, this test would have failed crashed due to a
            # use-after-free: text edit ended but an undo action of the text edit remained on the undo
            # stack.
            for i in range(2):
                self.xUITest.executeCommand(".uno:Undo")

            # Close the document.

# vim: set shiftwidth=4 softtabstop=4 expandtab:
