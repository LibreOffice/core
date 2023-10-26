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
from uitest.uihelper.common import get_state_as_dict
import time

class tdf137274(UITestCase):

    def get_item(self, xTree, name):
        for i in xTree.getChildren():
            xItem = xTree.getChild(i)
            if name == get_state_as_dict(xItem)['Text']:
                return xItem

    def test_tdf137274(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            xMainWindow = self.xUITest.getTopFocusWindow()
            xWriterEdit = xMainWindow.getChild("writer_edit")

            xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "Test"}))

            # Insert first comment
            xArgs = mkPropertyValues({"Text": "C1"})
            self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

            # wait until the comment is available
            self.ui_test.wait_until_child_is_available('Comment1')

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
            xComments = self.get_item(xContentTree, 'Comments')
            self.assertEqual('Comments', get_state_as_dict(xComments)['Text'])

            xComments.executeAction("EXPAND", tuple())

            self.assertEqual(1, len(xComments.getChildren()))
            self.assertEqual('C1', get_state_as_dict(xComments.getChild('0'))['Text'])

            xComments.executeAction("COLLAPSE", tuple())

            xArgs = mkPropertyValues({"Text": "C2"})
            self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

            # wait until the second comment is available
            self.ui_test.wait_until_child_is_available('Comment2')

            # xComments needs reassigned after content tree change
            while True:
                xComments = self.get_item(xContentTree, 'Comments')
                if '1' in xComments.getChildren():
                    break
                time.sleep(self.ui_test.get_default_sleep())
            self.assertEqual('Comments', get_state_as_dict(xComments)['Text'])

            xComments.executeAction("EXPAND", tuple())

            # Without the fix in place, this test would have failed with AssertionError: 2 != 0
            self.assertEqual(2, len(xComments.getChildren()))
            self.assertEqual('C1', get_state_as_dict(xComments.getChild('0'))['Text'])

            xComments.executeAction("COLLAPSE", tuple())

            self.xUITest.executeCommand(".uno:Sidebar")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
