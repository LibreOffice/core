# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf134439(UITestCase):

    def test_tdf134439(self):

        self.ui_test.load_file(get_url_for_data_file("tdf134439.odt"))

        document = self.ui_test.get_component()

        self.assertEqual(document.CurrentController.PageCount, 3)

        xCursor = document.CurrentController.ViewCursor
        self.assertEqual("Chap 1", xCursor.PageStyleName)

        xPageBreak = self.ui_test.wait_until_child_is_available('PageBreak')
        self.ui_test.execute_dialog_through_action(xPageBreak, "EDIT")

        xDialog = self.xUITest.getTopFocusWindow()

        self.assertEqual("Page", get_state_as_dict(xDialog.getChild("comboBreakType"))["SelectEntryText"])
        self.assertEqual("Before", get_state_as_dict(xDialog.getChild("comboBreakPosition"))["SelectEntryText"])

        xPageStyle = xDialog.getChild("comboPageStyle")
        self.assertEqual("Chap 2", get_state_as_dict(xPageStyle)["SelectEntryText"])

        xPageStyle.executeAction("SELECT", mkPropertyValues({"TEXT": "Chap 3"}))

        self.assertEqual("Chap 3", get_state_as_dict(xPageStyle)["SelectEntryText"])

        # tdf#116070: Without the fix in place, this test would have crashed here
        okBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(okBtn)

        # Without the fix in place, this test would have failed with
        # AssertionError: 'Chap 1' != 'Chap 3'
        self.assertEqual("Chap 1", xCursor.PageStyleName)

        self.assertEqual(document.CurrentController.PageCount, 3)

        xCursor.jumpToNextPage()
        self.assertEqual("Chap 3", xCursor.PageStyleName)

        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual("Chap 2", xCursor.PageStyleName)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
