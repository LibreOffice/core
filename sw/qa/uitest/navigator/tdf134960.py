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

    def get_item(self, xTree, name):
        for i in xTree.getChildren():
            xItem = xTree.getChild(i)
            if name == get_state_as_dict(xItem)['Text']:
                return xItem

    def launch_sidebar(self, xWriterEdit):

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
        xHyperlinks = self.get_item(xContentTree, 'Hyperlinks')
        self.assertEqual('Hyperlinks', get_state_as_dict(xHyperlinks)['Text'])

        xHyperlinks.executeAction("EXPAND", tuple())

        expectedHyperlinksOrder = [1, 2, 8, 9, 7, 10, 11, 3, 12, 4, 5, 6]
        for i in range(12):
            self.assertEqual('Hyperlink ' + str(expectedHyperlinksOrder[i]), get_state_as_dict(xHyperlinks.getChild(str(i)))['Text'])

        xHyperlinks.executeAction("COLLAPSE", tuple())

        self.xUITest.executeCommand(".uno:Sidebar")

    def test_tdf134960_hyperlinks(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf134960.odt")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            # Without the fix in place, this test would have failed with
            # AssertionError: 'Hyperlink 2' != 'Hyperlink 6'
            self.launch_sidebar(xWriterEdit)

    def test_tdf134960_hyperlinks_with_multiple_pages(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf134960.odt")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            # Insert a page break so we have different links in different pages
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            self.xUITest.executeCommand(".uno:InsertPagebreak")
            self.assertEqual("3", get_state_as_dict(xWriterEdit)["Pages"])

            # Change view to 20% and 2 columns
            with self.ui_test.execute_dialog_through_command(".uno:Zoom") as xDialog:
                variable = xDialog.getChild("variable")
                variable.executeAction("CLICK", tuple())

                zoomsb = xDialog.getChild("zoomsb")
                zoomsb.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                zoomsb.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                zoomsb.executeAction("TYPE", mkPropertyValues({"TEXT":"20"}))

                columns = xDialog.getChild("columns")
                columns.executeAction("CLICK", tuple())

                columnssb = xDialog.getChild("columnssb")
                columnssb.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                columnssb.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                columnssb.executeAction("TYPE", mkPropertyValues({"TEXT":"3"}))

            # Without the fix in place, this test would have failed with
            # AssertionError: 'Hyperlink 2' != 'Hyperlink 8'
            self.launch_sidebar(xWriterEdit)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
