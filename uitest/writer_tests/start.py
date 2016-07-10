# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest_helper import UITest, get_state_as_dict

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase

import time

try:
    import pyuno
    import uno
    import unohelper
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

class SimpleWriterTest(UITestCase):

    def test_start_writer(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()

        xWriterEdit = xWriterDoc.getChild("writer_edit")

        xWriterEdit.executeAction("SET", mkPropertyValues({"ZOOM": "200"}))

        self.ui_test.close_doc()

    def test_type_text(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "This is my first writer text written through the UI testing"}))

        self.ui_test.close_doc()

    def test_goto_first_page(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        state = get_state_as_dict(xWriterEdit)
        while state["CurrentPage"] is "1":
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            state = get_state_as_dict(xWriterEdit)

        xWriterEdit.executeAction("GOTO", mkPropertyValues({"PAGE": "1"}))

        self.ui_test.close_doc()


    def test_select_text(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "This is my first writer text written through the UI testing"}))

        xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "0", "END_POS": "4"}))

        self.ui_test.close_doc()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
