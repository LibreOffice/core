# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
# tests for tracked changes ; tdf912270

from uitest.framework import UITestCase
from uitest.debug import sleep
from uitest.path import get_srcdir_url
import time
from uitest.uihelper.common import get_state_as_dict, type_text


def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

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
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, "Test LibreOffice")
        self.xUITest.executeCommand(".uno:ShowTrackedChanges")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()
        xAccBtn = xTrackDlg.getChild("accept")
        xAccBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.assertEqual(document.Text.String[0:16], "Test LibreOffice")
        self.ui_test.close_doc()

    def test_tracked_changes_acceptall(self):

        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, "Test LibreOffice")
        self.xUITest.executeCommand(".uno:ShowTrackedChanges")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()

        xAccBtn = xTrackDlg.getChild("acceptall")
        xAccBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())


        self.assertEqual(document.Text.String[0:16], "Test LibreOffice")
        self.ui_test.close_doc()

    def test_tracked_changes_reject(self):

        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, "Test LibreOffice")
        self.xUITest.executeCommand(".uno:ShowTrackedChanges")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()

        xRejBtn = xTrackDlg.getChild("reject")
        xRejBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.assertEqual(document.Text.String[0:1], "")
        self.ui_test.close_doc()

    def test_tracked_changes_rejectall(self):

        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, "Test LibreOffice")
        self.xUITest.executeCommand(".uno:ShowTrackedChanges")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()

        xAccBtn = xTrackDlg.getChild("rejectall")
        xAccBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.assertEqual(document.Text.String[0:1], "")
        self.ui_test.close_doc()

    def test_tracked_changes_zprev_next(self):

        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, "Test LibreOffice")
        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, " Test2")
        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, " Test3")
        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, " Test4")
        self.xUITest.executeCommand(".uno:TrackChanges")
        type_text(xWriterEdit, " Test5")
        self.xUITest.executeCommand(".uno:PreviousTrackedChange")
        self.xUITest.executeCommand(".uno:RejectTrackedChange")
        self.assertEqual(document.Text.String[0:37], "Test LibreOffice Test2 Test3 Test4")

        self.xUITest.executeCommand(".uno:PreviousTrackedChange")
        self.xUITest.executeCommand(".uno:PreviousTrackedChange")
        self.xUITest.executeCommand(".uno:AcceptTrackedChange")
        self.assertEqual(document.Text.String[0:37], "Test LibreOffice Test2 Test3 Test4")

        self.xUITest.executeCommand(".uno:NextTrackedChange")
        self.xUITest.executeCommand(".uno:RejectTrackedChange")
        self.assertEqual(document.Text.String[0:30], "Test LibreOffice Test2 Test4")

        self.ui_test.close_doc()

    def test_list_of_changes(self):
        self.ui_test.load_file(get_url_for_data_file("trackedChanges.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()

        listText = [
                "Unknown Author\t01/24/2020 16:19:32\t",
                "Unknown Author\t01/24/2020 16:19:35\t",
                "Unknown Author\t01/24/2020 16:19:39\t",
                "Unknown Author\t01/24/2020 16:19:39\t",
                "Xisco Fauli\t01/27/2020 17:42:55\t"]

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()
        changesList = xTrackDlg.getChild("writerchanges")

        resultsAccept = [
            "The tennis ball is a small ball. The baskedtball is much bigger.",
            "The tennis ball is a small ball. The baskedtball is much bigger.",
            "The tennis ball is a small ball. The baskedtball is much bigger.",
            "The tennis ball is a small ball. The basketball is much bigger.",
            "The tennis ball is a small ball. The basketball is much bigger.",
            "The tennis ball is a small ball. The basketball is much bigger."]

        for i in range(len(listText)):
            self.assertEqual(document.Text.String.strip(), resultsAccept[i])
            self.assertEqual(get_state_as_dict(changesList.getChild('0'))["Text"], listText[i] )
            xAccBtn = xTrackDlg.getChild("accept")
            xAccBtn.executeAction("CLICK", tuple())

        self.assertEqual(document.Text.String.strip(), resultsAccept[5])
        #List is empty
        self.assertFalse('0' in changesList.getChildren())

        for i in reversed(range(len(listText))):
            xUndoBtn = xTrackDlg.getChild("undo")
            xUndoBtn.executeAction("CLICK", tuple())
            self.assertEqual(document.Text.String.strip(), resultsAccept[i])
            self.assertEqual(get_state_as_dict(changesList.getChild('0'))["Text"], listText[i] )

        resultsReject = [
            "The tennis ball is a small ball. The baskedtball is much bigger.",
            "The tenis ball is a small ball. The baskedtball is much bigger.",
            "The tenis ball is a small bal. The baskedtball is much bigger.",
            "The tenis ball is a small bal. The baskedtball is much bigger.",
            "The tenis ball is a small bal. The baskedball is much bigger.",
            "The tenis ball is a small bal. The baskedball is much biger."]

        for i in range(len(listText)):
            self.assertEqual(document.Text.String.strip(), resultsReject[i])
            self.assertEqual(get_state_as_dict(changesList.getChild('0'))["Text"], listText[i] )
            xAccBtn = xTrackDlg.getChild("reject")
            xAccBtn.executeAction("CLICK", tuple())

        self.assertEqual(document.Text.String.strip(), resultsReject[5])
        #List is empty
        self.assertFalse('0' in changesList.getChildren())

        for i in reversed(range(len(listText))):
            xUndoBtn = xTrackDlg.getChild("undo")
            xUndoBtn.executeAction("CLICK", tuple())
            self.assertEqual(document.Text.String.strip(), resultsReject[i])
            self.assertEqual(get_state_as_dict(changesList.getChild('0'))["Text"], listText[i] )

        xcloseBtn = xTrackDlg.getChild("close")
        xcloseBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_tdf135018(self):
        self.ui_test.load_file(get_url_for_data_file("tdf135018.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()

        self.assertEqual(5, document.CurrentController.PageCount)

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()
        changesList = xTrackDlg.getChild("writerchanges")
        self.assertEqual(147, len(changesList.getChildren()))

        # Without the fix in place, it would have crashed here
        xAccBtn = xTrackDlg.getChild("acceptall")
        xAccBtn.executeAction("CLICK", tuple())

        self.assertEqual(0, len(changesList.getChildren()))

        xUndoBtn = xTrackDlg.getChild("undo")
        xUndoBtn.executeAction("CLICK", tuple())

        self.assertEqual(147, len(changesList.getChildren()))

        xcloseBtn = xTrackDlg.getChild("close")
        xcloseBtn.executeAction("CLICK", tuple())

        self.assertEqual(18, document.CurrentController.PageCount)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
