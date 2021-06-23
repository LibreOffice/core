#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict


# Test for SvPasteObjectDialog.
class Test(UITestCase):

    def testGetFormat(self):
        # Copy a string in Impress.
        self.ui_test.create_doc_in_start_center("impress")
        template = self.xUITest.getTopFocusWindow()
        self.ui_test.close_dialog_through_button(template.getChild("close"))
        doc = self.xUITest.getTopFocusWindow()
        editWin = doc.getChild("impress_win")
        # Select the title shape.
        editWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        editWin.executeAction("TYPE", mkPropertyValues({"TEXT": "t"}))
        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Copy")

        # Now use paste special to see what formats are offered.
        self.ui_test.execute_dialog_through_command(".uno:PasteSpecial")
        pasteSpecial = self.xUITest.getTopFocusWindow()
        formats = pasteSpecial.getChild("list")
        entryCount = int(get_state_as_dict(formats)["Children"])
        items = []
        for index in range(entryCount):
            entry = formats.getChild(str(index))
            entry.executeAction("SELECT", tuple())
            items.append(get_state_as_dict(formats)["SelectEntryText"])

        # Make sure there is no RTF vs Richtext duplication.
        self.assertTrue("Rich text formatting (RTF)" in items)
        self.assertFalse("Rich text formatting (Richtext)" in items)

        # Close the dialog and the document.
        self.ui_test.close_dialog_through_button(pasteSpecial.getChild("cancel"))
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
