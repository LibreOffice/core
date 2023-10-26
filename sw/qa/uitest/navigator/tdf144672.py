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

class tdf144672(UITestCase):

    def get_item(self, xTree, name):
        for i in xTree.getChildren():
            xItem = xTree.getChild(i)
            if name == get_state_as_dict(xItem)['Text']:
                return xItem

    def test_Tdf144672(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf144672.odt")):

            xMainWindow = self.xUITest.getTopFocusWindow()
            xWriterEdit = xMainWindow.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:Sidebar")

            xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

            # wait until the navigator panel is available
            xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

            # See the `m_aUpdateTimer.SetTimeout(200)` (to "avoid flickering of buttons")
            # in the SwChildWinWrapper ctor in sw/source/uibase/fldui/fldwrap.cxx, where that
            # m_aUpdateTimer is started by SwChildWinWrapper::ReInitDlg triggered from the
            # xInsert click above.
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.waitUntilAllIdlesDispatched()

            xContentTree = xNavigatorPanel.getChild("contenttree")

            xReferences = self.get_item(xContentTree, 'References')
            self.assertEqual('References', get_state_as_dict(xReferences)['Text'])

            # tdf#129625: Without the fix in place, this test would have failed with
            # AssertionError: 'true' != 'false'
            self.assertEqual('true', get_state_as_dict(xReferences)['IsSemiTransparent'])

            xIndexes = self.get_item(xContentTree, 'Indexes')
            self.assertEqual('Indexes', get_state_as_dict(xIndexes)['Text'])
            self.assertEqual('false', get_state_as_dict(xIndexes)['IsSemiTransparent'])

            xIndexes.executeAction("EXPAND", tuple())

            self.assertEqual('3', get_state_as_dict(xIndexes)['Children'])

            for i in range(3):
                # Without the fix in place, this test would have failed here with
                # AssertionError: 'false' != 'true'
                self.assertEqual('false', get_state_as_dict(xIndexes.getChild(i))['IsSemiTransparent'])

            xIndexes.executeAction("COLLAPSE", tuple())

            self.xUITest.executeCommand(".uno:Sidebar")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
