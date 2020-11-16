# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf130629(UITestCase):

    def test_tdf130629(self):

        self.ui_test.create_doc_in_start_center("writer")

        document = self.ui_test.get_component()
        self.assertEqual(0, document.DrawPage.getCount())

        # Insert shape while pressing CTRL
        self.xUITest.executeCommandWithParameters(".uno:BasicShapes.diamond", mkPropertyValues({"KeyModifier": 8192}))

        self.assertEqual(1, document.DrawPage.getCount())

        # Undo twice
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(0, document.DrawPage.getCount())

        # Shape toolbar is active, use esc before inserting a new shape
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))

        # Without the fix in place, this test would have crashed here
        self.xUITest.executeCommandWithParameters(".uno:BasicShapes.diamond", mkPropertyValues({"KeyModifier": 8192}))

        self.assertEqual(1, document.DrawPage.getCount())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
