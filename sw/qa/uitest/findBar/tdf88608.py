# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf88608(UITestCase):

    def test_tdf88608(self):

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "Hello World"}))

        self.xUITest.executeCommand("vnd.sun.star.findbar:FocusToFindbar")

        # Search a word that doesn't exist
        xfind = xWriterDoc.getChild("find")
        xfind.executeAction("TYPE", mkPropertyValues({"TEXT": "X"}))
        self.assertEqual("X", get_state_as_dict(xfind)['Text'])

        xfind_bar = xWriterDoc.getChild("FindBar")
        self.assertEqual('', get_state_as_dict(xfind_bar.getChild('label'))['Text'])

        # Search Next
        xfind_bar.executeAction("CLICK", mkPropertyValues({"POS": "3"}))

        self.assertEqual('Search key not found', get_state_as_dict(xfind_bar.getChild('label'))['Text'])

        self.assertEqual("", get_state_as_dict(xWriterEdit)['SelectedText'])

        xfind.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
        xfind.executeAction("TYPE", mkPropertyValues({"TEXT": "World"}))
        self.assertEqual("World", get_state_as_dict(xfind)['Text'])

        # Without the fix in place, this test would have failed with
        # AssertionError: '' != 'Search key not found'
        self.assertEqual('', get_state_as_dict(xfind_bar.getChild('label'))['Text'])

        # Search Next
        xfind_bar.executeAction("CLICK", mkPropertyValues({"POS": "3"}))

        self.assertEqual('Reached the end of the document', get_state_as_dict(xfind_bar.getChild('label'))['Text'])

        self.assertEqual("World", get_state_as_dict(xWriterEdit)['SelectedText'])

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
