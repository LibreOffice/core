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

class tdf134960_hyperlinks(UITestCase):

    def test_tdf134960_hyperlinks(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf134960.odt")) as writer_doc:

            xMainWindow = self.xUITest.getTopFocusWindow()
            xWriterEdit = xMainWindow.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:Sidebar")

            xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

            # wait until the navigator panel is available
            xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

            xContentTree = xNavigatorPanel.getChild("contenttree")
            xHyperlinks = xContentTree.getChild('7')
            self.assertEqual('Hyperlinks', get_state_as_dict(xHyperlinks)['Text'])

            xHyperlinks.executeAction("EXPAND", tuple())

            expectedHyperlinksOrder = [1, 2, 8, 9, 7, 10, 11, 3, 12, 4, 5, 6]
            for i in range(12):
                self.assertEqual('Hyperlink ' + str(expectedHyperlinksOrder[i]), get_state_as_dict(xHyperlinks.getChild(str(i)))['Text'])

            xHyperlinks.executeAction("COLLAPSE", tuple())

            self.xUITest.executeCommand(".uno:Sidebar")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
