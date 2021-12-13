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

   # access to the private:resource changes toolbar via accessibility API
   def is_enabled_Accept_Track_Change(self):
       xFrame = self.document.getCurrentController().getFrame()

       for i in xFrame.getPropertyValue("LayoutManager").getElements():
           if i.getPropertyValue('ResourceURL') == 'private:resource/toolbar/changes':
               return ENABLED in i.getRealInterface().getAccessibleContext().getAccessibleChild(5).getAccessibleStateSet().getStates()

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

# vim: set shiftwidth=4 softtabstop=4 expandtab:
