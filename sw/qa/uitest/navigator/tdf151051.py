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

class tdf151051(UITestCase):

    def test_tdf151051(self):

        with self.ui_test.load_file(get_url_for_data_file('tdf151051.odt')):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild('writer_edit')

            self.xUITest.executeCommand('.uno:Sidebar')
            xWriterEdit.executeAction('SIDEBAR', mkPropertyValues({'PANEL': 'SwNavigatorPanel'}))

            # wait until the navigator panel is available
            xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

            # See the `m_aUpdateTimer.SetTimeout(200)` (to "avoid flickering of buttons")
            # in the SwChildWinWrapper ctor in sw/source/uibase/fldui/fldwrap.cxx, where that
            # m_aUpdateTimer is started by SwChildWinWrapper::ReInitDlg triggered from the
            # xInsert click above.
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.waitUntilAllIdlesDispatched()

            xContentTree = xNavigatorPanel.getChild("contenttree")

            xHeadings = xContentTree.getChild('0')
            xHeadings.executeAction("EXPAND", tuple())

            xChild = xHeadings.getChild('0')
            self.assertEqual('0', get_state_as_dict(xChild)['Children'])

            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("HeadingsContentFunctionButtonsToolbar")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "0"})) # 'promote' button

            xHeadings = xContentTree.getChild('0')
            xHeadings.executeAction("EXPAND", tuple())

            xChild = xHeadings.getChild('0')
            xChild.executeAction("EXPAND", tuple())

            # Without the fix in place, this test would have failed with
            # AssertionError: '1' != '0'
            self.assertEqual('1', get_state_as_dict(xChild)['Children'])

            self.xUITest.executeCommand('.uno:Sidebar')

# vim: set shiftwidth=4 softtabstop=4 expandtab:
