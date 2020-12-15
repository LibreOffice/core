#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos


# Test for SvxTableController.
class SvxTableControllerTest(UITestCase):

    def testOnFormatTable(self):
        # Create an Impress document with a single table in it.
        self.ui_test.create_doc_in_start_center("impress")
        template = self.xUITest.getTopFocusWindow()
        self.ui_test.close_dialog_through_button(template.getChild("cancel"))
        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Delete")
        self.xUITest.executeCommand(".uno:InsertTable?Columns:short=2&Rows:short=2")

        # Enable shadow.
        self.ui_test.execute_dialog_through_command(".uno:TableDialog")
        tableDialog = self.xUITest.getTopFocusWindow()
        tabs = tableDialog.getChild("tabcontrol")
        # Select "shadow".
        select_pos(tabs, "4")
        shadowCheckbox = tableDialog.getChild("TSB_SHOW_SHADOW")
        shadowCheckbox.executeAction("CLICK", tuple())
        self.ui_test.close_dialog_through_button(tableDialog.getChild("ok"))

        # Check if the shadow was enabled.
        component = self.ui_test.get_component()
        drawPage = component.getDrawPages().getByIndex(0)
        shape = drawPage.getByIndex(0)
        # Without the accompanying fix in place, this test would have failed with:
        # AssertionError: False != True
        # i.e. the table still had no shadow.
        self.assertEqual(shape.Shadow, True)

        # Close the document.
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
