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
import time

class tdf151051(UITestCase):

    def test_tdf151051(self):

        with self.ui_test.load_file(get_url_for_data_file('tdf151051.odt')):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild('writer_edit')

            self.xUITest.executeCommand('.uno:Sidebar')
            xWriterEdit.executeAction('SIDEBAR', mkPropertyValues({'PANEL': 'SwNavigatorPanel'}))

            # wait until the navigator panel is available
            xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

            # HACK, see the `m_aUpdTimer.SetTimeout(1000)` in the SwContentTree ctor in
            # sw/source/uibase/utlui/content.cxx, where that m_aUpdTimer is started by
            # SwContentTree::ShowTree triggered from the SIDEBAR action above, and which can
            # invalidate the TreeListEntryUIObjects used by the below code (see
            # 2798430c8a711861fdcdfbf9ac00a0527abd3bfc "Mark the uses of TreeListEntryUIObject as
            # dubious"); lets double that 1000 ms timeout value here to hopefully be on the safe
            # side:
            time.sleep(2)

            xContentTree = xNavigatorPanel.getChild("contenttree")

            xHeadings = xContentTree.getChild('0')
            xHeadings.executeAction("EXPAND", tuple())

            xChild = xHeadings.getChild('0')
            self.assertEqual('0', get_state_as_dict(xChild)['Children'])

            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("content6")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "2"})) # 'promote' button

            xHeadings = xContentTree.getChild('0')
            xHeadings.executeAction("EXPAND", tuple())

            xChild = xHeadings.getChild('0')
            xChild.executeAction("EXPAND", tuple())

            # Without the fix in place, this test would have failed with
            # AssertionError: '1' != '0'
            self.assertEqual('1', get_state_as_dict(xChild)['Children'])

            self.xUITest.executeCommand('.uno:Sidebar')

# vim: set shiftwidth=4 softtabstop=4 expandtab:
