# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

class SimpleWriterTest(UITestCase):

    def test_start_writer(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()

        xWriterEdit = xWriterDoc.getChild("writer_edit")

        xWriterEdit.executeAction("SET", mkPropertyValues({"ZOOM": "200"}))

        self.assertEqual(get_state_as_dict(xWriterEdit)["Zoom"], "200")

        self.ui_test.close_doc()

    def test_goto_first_page(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        state = get_state_as_dict(xWriterEdit)
        while state["CurrentPage"] == "1":
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            state = get_state_as_dict(xWriterEdit)

        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")

        xWriterEdit.executeAction("GOTO", mkPropertyValues({"PAGE": "1"}))

        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "1")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
