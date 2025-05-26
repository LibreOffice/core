# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
import time

from com.sun.star.accessibility.AccessibleStateType import ENABLED

class tdf146145(UITestCase):

    def is_enabled_Accept_Track_Change(self):
        xFrame = self.document.getCurrentController().getFrame()

        for i in xFrame.getPropertyValue("LayoutManager").getElements():
            if i.getPropertyValue('ResourceURL') == 'private:resource/toolbar/changes':
                xToolBox = self.xUITest.getWindow(i.getRealInterface())
                for child_name in xToolBox.getChildren():
                    child = xToolBox.getChild(child_name)
                    states = get_state_as_dict(child)
                    if states['Text'] == 'Accept':
                        return states['Enabled'] == 'true'

        return False

    def test_tdf146145(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as self.document:

            # Check enabling Accept/Reject Track Change icons
            # and Accept Change/Reject Change context menu items
            # on table rows with tracked deletion or insertion

            # enable Track Changes toolbar
            self.xUITest.executeCommand(".uno:AvailableToolbars?Toolbar:string=changes")

            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            # cursor at changed text: Accept Track Change is enabled
            self.assertTrue(self.is_enabled_Accept_Track_Change())

            # cursor in a changed row, but not at changed text: Accept Track Change is enabled now
            self.xUITest.executeCommand(".uno:GoRight")
            xToolkit.processEventsToIdle()
            # This was false
            self.assertTrue(self.is_enabled_Accept_Track_Change())

            # cursor in a not changed row: Accept Track Change is disabled
            self.xUITest.executeCommand(".uno:GoDown")
            xToolkit.processEventsToIdle()
            while self.is_enabled_Accept_Track_Change():
                time.sleep(0.1)
            self.assertFalse(self.is_enabled_Accept_Track_Change())

            # check the fix again to avoid of the asynchron state changes
            self.xUITest.executeCommand(".uno:GoUp")
            xToolkit.processEventsToIdle()
            while not self.is_enabled_Accept_Track_Change():
                time.sleep(0.1)
            self.assertTrue(self.is_enabled_Accept_Track_Change())

            # check 1-click accept of table row deletion (3 redlines in the row)

            # not at changed text, but Accept Track Change removes the whole row now

            tables = self.document.getTextTables()
            self.assertEqual(len(tables[0].getRows()), 4)

            self.xUITest.executeCommand(".uno:AcceptTrackedChange")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getRows()), 3)

            self.xUITest.executeCommand(".uno:Undo")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getRows()), 4)

            self.xUITest.executeCommand(".uno:Redo")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getRows()), 3)

            self.xUITest.executeCommand(".uno:Undo")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getRows()), 4)

            # check 1-click reject of table row insertion (3 redlines in the row)

            for i in range(3):
                self.xUITest.executeCommand(".uno:GoDown")

            # not at changed text (the cursor is there at the end of the text of the table row),
            # but Reject Track Change removes the whole row now

            tables = self.document.getTextTables()
            self.assertEqual(len(tables[0].getRows()), 4)

            self.xUITest.executeCommand(".uno:RejectTrackedChange")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getRows()), 3)

            self.xUITest.executeCommand(".uno:Undo")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getRows()), 4)

            self.xUITest.executeCommand(".uno:Redo")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getRows()), 3)

    def test_Related_tdf147182(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as self.document:

            # Check enabling Accept/Reject Track Change icons
            # and Accept Change/Reject Change context menu items
            # on table rows with tracked deletion or insertion

            # enable Track Changes toolbar
            self.xUITest.executeCommand(".uno:AvailableToolbars?Toolbar:string=changes")

            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            # cursor at changed text: Accept Track Change is enabled
            self.assertTrue(self.is_enabled_Accept_Track_Change())

            # cursor in a changed row, but not at changed text: Accept Track Change is enabled now
            self.xUITest.executeCommand(".uno:GoRight")
            xToolkit.processEventsToIdle()
            # This was false
            self.assertTrue(self.is_enabled_Accept_Track_Change())

            # delete first row
            self.xUITest.executeCommand(".uno:AcceptTrackedChange")
            xToolkit.processEventsToIdle()
            # disabled Accept Track Change
            while self.is_enabled_Accept_Track_Change():
                time.sleep(0.1)
            self.assertFalse(self.is_enabled_Accept_Track_Change())

            # delete first row
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:SelectAll")
            xToolkit.processEventsToIdle()
            # This was false
            while not self.is_enabled_Accept_Track_Change():
                time.sleep(0.1)
            self.assertTrue(self.is_enabled_Accept_Track_Change())

            # delete all changes in the selected table
            self.xUITest.executeCommand(".uno:AcceptTrackedChange")
            xToolkit.processEventsToIdle()
            while self.is_enabled_Accept_Track_Change():
                time.sleep(0.1)
            # disabled Accept Track Change
            self.assertFalse(self.is_enabled_Accept_Track_Change())

    def test_tdf155344(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as self.document:

            # accept all tracked changes
            self.xUITest.executeCommand(".uno:AcceptAllTrackedChanges")
            # delete first table column
            self.xUITest.executeCommand(".uno:DeleteColumns")

            # Check enabling Accept/Reject Track Change icons
            # and Accept Change/Reject Change context menu items
            # on table columns with tracked deletion or insertion

            # enable Track Changes toolbar
            self.xUITest.executeCommand(".uno:AvailableToolbars?Toolbar:string=changes")

            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            # cursor at changed text: Accept Track Change is enabled
            self.assertTrue(self.is_enabled_Accept_Track_Change())

            # cursor in a changed column, but not at changed text: Accept Track Change is enabled now
            self.xUITest.executeCommand(".uno:GoRight")
            xToolkit.processEventsToIdle()
            # This was false
            self.assertTrue(self.is_enabled_Accept_Track_Change())

            # cursor in a not changed column: Accept Track Change is disabled
            self.xUITest.executeCommand(".uno:GoRight")
            xToolkit.processEventsToIdle()
# Disable uitest, it keeps getting stuck
#            while self.is_enabled_Accept_Track_Change():
#                time.sleep(0.1)
#            self.assertFalse(self.is_enabled_Accept_Track_Change())

            # check the fix again to avoid of the asynchron state changes
            self.xUITest.executeCommand(".uno:GoLeft")
            xToolkit.processEventsToIdle()
            while not self.is_enabled_Accept_Track_Change():
                time.sleep(0.1)
            self.assertTrue(self.is_enabled_Accept_Track_Change())

            # check 1-click accept of table column deletion (3 redlines in the column)

            # not at changed text, but Accept Track Change removes the whole column now

            tables = self.document.getTextTables()
            self.assertEqual(len(tables[0].getColumns()), 3)

            self.xUITest.executeCommand(".uno:AcceptTrackedChange")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getColumns()), 2)

            self.xUITest.executeCommand(".uno:Undo")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getColumns()), 3)

            self.xUITest.executeCommand(".uno:Redo")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getColumns()), 2)

            # check 1-click reject of table column insertion (9 redlines in the 3 columns)

            self.xUITest.executeCommand(".uno:InsertColumnsBefore")
            self.xUITest.executeCommand(".uno:InsertColumnsBefore")
            self.xUITest.executeCommand(".uno:InsertColumnsBefore")
            xToolkit.processEventsToIdle()
# Disabled unreliable test, tends to get stuck here
#            while self.is_enabled_Accept_Track_Change():
#                time.sleep(0.1)
#            self.assertFalse(self.is_enabled_Accept_Track_Change())
#
#            self.assertEqual(len(tables[0].getColumns()), 5)

            # check the fix again to avoid of the asynchron state changes
            self.xUITest.executeCommand(".uno:GoLeft")
            xToolkit.processEventsToIdle()
            while not self.is_enabled_Accept_Track_Change():
                time.sleep(0.1)
            # This was false
            self.assertTrue(self.is_enabled_Accept_Track_Change())

            self.xUITest.executeCommand(".uno:RejectTrackedChange")

            self.assertEqual(len(tables[0].getColumns()), 2)

            self.xUITest.executeCommand(".uno:Undo")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getColumns()), 5)

            self.xUITest.executeCommand(".uno:Redo")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getColumns()), 2)

            self.xUITest.executeCommand(".uno:Undo")
            xToolkit.processEventsToIdle()
            self.assertEqual(len(tables[0].getColumns()), 5)

    def test_crashWithHiddenFirstTableColumn(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as self.document:

            # accept all tracked changes
            self.xUITest.executeCommand(".uno:AcceptAllTrackedChanges")
            # delete first table column
            self.xUITest.executeCommand(".uno:DeleteColumns")

            # Check enabling Accept/Reject Track Change icons
            # and Accept Change/Reject Change context menu items
            # on table columns with tracked deletion or insertion

            # enable Track Changes toolbar
            self.xUITest.executeCommand(".uno:AvailableToolbars?Toolbar:string=changes")

            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            # cursor at changed text: Accept Track Change is enabled
            self.assertTrue(self.is_enabled_Accept_Track_Change())

            # Disable uitest, it keeps getting stuck
            # hide changes
            #self.xUITest.executeCommand(".uno:ShowTrackedChanges")
            #while self.is_enabled_Accept_Track_Change():
            #    time.sleep(0.1)
            #self.assertFalse(self.is_enabled_Accept_Track_Change())

            # Without the fix in place, this test would have crashed here
            #self.xUITest.executeCommand(".uno:DeleteRows")

            #self.xUITest.executeCommand(".uno:ShowTrackedChanges")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
