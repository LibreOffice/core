# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf149268(UITestCase):

    def test_tdf149268(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf149268.odt")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:Sidebar")

            # Without the fix in place, this test would have crashed here
            xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

            # wait until the navigator panel is available
            xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

            xContentTree = xNavigatorPanel.getChild("contenttree")
            xHyperlinks = xContentTree.getChild('7')
            self.assertEqual('Hyperlinks', get_state_as_dict(xHyperlinks)['Text'])

            xHyperlinks.executeAction("EXPAND", tuple())

            self.assertEqual("4", get_state_as_dict(xHyperlinks)['Children'])
            self.assertEqual('Internet', get_state_as_dict(xHyperlinks.getChild('0'))['Text'])
            self.assertEqual('1234567', get_state_as_dict(xHyperlinks.getChild('1'))['Text'])
            self.assertEqual('yyyy', get_state_as_dict(xHyperlinks.getChild('2'))['Text'])
            self.assertEqual('zzz', get_state_as_dict(xHyperlinks.getChild('3'))['Text'])

            self.xUITest.executeCommand(".uno:Sidebar")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
