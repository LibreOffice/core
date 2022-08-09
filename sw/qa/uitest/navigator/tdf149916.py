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

class tdf149916(UITestCase):

    def test_tdf149916(self):

        with self.ui_test.load_file(get_url_for_data_file('tdf149916.odt')):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild('writer_edit')

            self.xUITest.executeCommand('.uno:Sidebar')
            xWriterEdit.executeAction('SIDEBAR', mkPropertyValues({'PANEL': 'SwNavigatorPanel'}))

            # wait until the navigator panel is available
            xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

            xContentTree = xNavigatorPanel.getChild('contenttree')
            xFields = xContentTree.getChild('12')
            self.assertEqual('Fields', get_state_as_dict(xFields)['Text'])

            xFields.executeAction('EXPAND', tuple())

            self.assertEqual('1', get_state_as_dict(xFields)['Children'])

            # Without the fix in place, this test would have failed with
            # AssertionError: 'cross-reference - First - 1' != 'cross-reference -  __RefHeading___Toc45_480138666 - 1'
            self.assertEqual('cross-reference - First - 1', get_state_as_dict(xFields.getChild('0'))['Text'])

            self.xUITest.executeCommand('.uno:Sidebar')

# vim: set shiftwidth=4 softtabstop=4 expandtab:
