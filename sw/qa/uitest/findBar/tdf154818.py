# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf154818(UITestCase):

    def test_tdf154818_remember_search_item(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            # Search for an entry and check again if it is preselected (A -> B -> A)
            searchTerms = ["A", "B", "A"]
            for searchTerm in searchTerms:
                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+f"}))
                xFind = xWriterDoc.getChild("find")
                xFind.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xFind.executeAction("TYPE", mkPropertyValues({"TEXT":searchTerm}))
                xFind.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))
                xFindBar = xWriterDoc.getChild("FindBar")
                xFindBar.executeAction("CLICK", mkPropertyValues({"POS":"0"}))

            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+f"}))
            xFind = xWriterDoc.getChild("find")
            # Without the fix in place, this test would have failed with
            # AssertionError: 'A' != 'B'
            # i.e., the last search item was not remembered
            self.assertEqual("A", get_state_as_dict(xFind)["Text"])

    def test_tdf154818_search_history(self):
        with self.ui_test.create_doc_in_start_center("writer"):
                xWriterDoc = self.xUITest.getTopFocusWindow()
                xWriterEdit = xWriterDoc.getChild("writer_edit")

                # Search for an entry and check for the search history (A -> B -> C -> B)
                searchTerms = ["A", "B", "C", "B"]
                for searchTerm in searchTerms:
                    xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+f"}))
                    xFind = xWriterDoc.getChild("find")
                    xFind.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFind.executeAction("TYPE", mkPropertyValues({"TEXT":searchTerm}))
                    xFind.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))
                    xFindBar = xWriterDoc.getChild("FindBar")
                    xFindBar.executeAction("CLICK", mkPropertyValues({"POS":"0"}))

                # Check if the search history was respected
                searchTerms = ["B", "C", "A"]
                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+f"}))
                xFind = xWriterDoc.getChild("find")
                for searchTerm in searchTerms:
                    select_pos(xFind, str(searchTerms.index(searchTerm)))
                    # Without the fix in place, this test would have failed with
                    # AssertionError: 'B' != 'C'
                    # i.e., the search history was not respected
                    self.assertEqual(searchTerm, get_state_as_dict(xFind)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
