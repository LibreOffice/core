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

class tdf143569(UITestCase):

    def test_Tdf143569(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf143569.odt")):

            xMainWindow = self.xUITest.getTopFocusWindow()
            xWriterEdit = xMainWindow.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:Sidebar")

            xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

            # wait until the navigator panel is available
            xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

            xContentTree = xNavigatorPanel.getChild("contenttree")

            xHeadings = xContentTree.getChild('0')
            self.assertEqual('Headings', get_state_as_dict(xHeadings)['Text'])
            self.assertEqual("1", get_state_as_dict(xHeadings)['Children'])

            xHeadings.executeAction("EXPAND", tuple())

            xChild = xHeadings.getChild('0')

            # Without the fix in place, this test would have failed with
            # AssertionError: 'Introduction' != 'Anchored to Paragraph'
            self.assertEqual('Introduction', get_state_as_dict(xChild)['Text'])
            self.assertEqual("1", get_state_as_dict(xChild)['Children'])

            xChild.executeAction("EXPAND", tuple())

            xChild = xChild.getChild('0')
            self.assertEqual('Problem', get_state_as_dict(xChild)['Text'])
            self.assertEqual("1", get_state_as_dict(xChild)['Children'])

            xChild.executeAction("EXPAND", tuple())

            xChild = xChild.getChild('0')
            self.assertEqual('Discussion Agenda', get_state_as_dict(xChild)['Text'])
            self.assertEqual("2", get_state_as_dict(xChild)['Children'])

            xChild.executeAction("EXPAND", tuple())

            xChild1 = xChild.getChild('0')
            self.assertEqual('Anchored to Paragraph', get_state_as_dict(xChild1)['Text'])
            self.assertEqual("0", get_state_as_dict(xChild1)['Children'])

            xChild2 = xChild.getChild('1')
            self.assertEqual('Anchored as Character', get_state_as_dict(xChild2)['Text'])
            self.assertEqual("0", get_state_as_dict(xChild2)['Children'])

            self.xUITest.executeCommand(".uno:Sidebar")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
