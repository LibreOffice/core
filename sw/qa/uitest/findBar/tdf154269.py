# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf154269(UITestCase):

    def test_tdf154269(self):

        with self.ui_test.create_doc_in_start_center("writer"):
            # Open quick search
            self.xUITest.executeCommand("vnd.sun.star.findbar:FocusToFindbar")
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xFind = xWriterDoc.getChild("find")

            # Generate a search history with more than 10 entries (A to Z)
            for searchTerm in map(chr, range(65, 91)):
                # Search twice to generate a search history
                xFind.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
                xFind.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
                xFind.executeAction("TYPE", mkPropertyValues({"TEXT": searchTerm}))
                xFind.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
                xFind.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))
                self.xUITest.executeCommand("vnd.sun.star.findbar:FocusToFindbar")

            # The default value of FindReplaceRememberedSearches has been respected
            self.assertEqual("10", get_state_as_dict(xFind)["EntryCount"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
