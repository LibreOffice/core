# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf138232(UITestCase):

    def test_tdf138232(self):

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "Hello"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "Hello"}))

        self.xUITest.executeCommand("vnd.sun.star.findbar:FocusToFindbar")

        xfind = xWriterDoc.getChild("find")
        xfind.executeAction("TYPE", mkPropertyValues({"TEXT": "Hello"}))
        self.assertEqual("Hello", get_state_as_dict(xfind)['Text'])

        xfind_bar = xWriterDoc.getChild("FindBar")

        # Click on Find All
        xfind_bar.executeAction("CLICK", mkPropertyValues({"POS": "4"}))

        self.assertEqual("HelloHello", get_state_as_dict(xWriterEdit)['SelectedText'])

        xfind.executeAction("TYPE", mkPropertyValues({"TEXT": "World"}))

        # Without the fix in place, this test would have failed with
        # AssertionError: 'HelloWorld' != 'World'
        self.assertEqual("HelloWorld", get_state_as_dict(xfind)['Text'])

        xfind_bar.executeAction("CLICK", mkPropertyValues({"POS": "4"}))

        self.assertEqual("", get_state_as_dict(xWriterEdit)['SelectedText'])

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
