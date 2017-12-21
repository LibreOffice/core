#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
# tests for tracked changes ; tdf912270

from uitest.framework import UITestCase
import time
from uitest.uihelper.common import get_state_as_dict, type_text

class trackedchanges(UITestCase):

    def test_tdf91270(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        type_text(xWriterEdit, "Test")

        self.xUITest.executeCommand(".uno:TrackChanges")

        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_tracked_changes_accept(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        
        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, "Test LibreOffice")
        self.xUITest.executeCommand(".uno:ShowTrackedChanges")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()
#        print(xTrackDlg.getChildren())
        xAccBtn = xTrackDlg.getChild("accept")
        xAccBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_tracked_changes_acceptall(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        
        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, "Test LibreOffice")
        self.xUITest.executeCommand(".uno:ShowTrackedChanges")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()
#        print(xTrackDlg.getChildren())
        xAccBtn = xTrackDlg.getChild("acceptall")
        xAccBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_tracked_changes_reject(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        
        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, "Test LibreOffice")
        self.xUITest.executeCommand(".uno:ShowTrackedChanges")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()
#        print(xTrackDlg.getChildren())
        xRejBtn = xTrackDlg.getChild("reject")
        xRejBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_tracked_changes_rejectall(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        
        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, "Test LibreOffice")
        self.xUITest.executeCommand(".uno:ShowTrackedChanges")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()
#        print(xTrackDlg.getChildren())
        xAccBtn = xTrackDlg.getChild("rejectall")
        xAccBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_tracked_changes_zprev_next(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        
        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, "Test LibreOffice")
        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, " Test2")
        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, " Test3")
        self.xUITest.executeCommand(".uno:PreviousTrackedChange")
        self.xUITest.executeCommand(".uno:PreviousTrackedChange")
        self.xUITest.executeCommand(".uno:NextTrackedChange")
        self.ui_test.close_doc()

