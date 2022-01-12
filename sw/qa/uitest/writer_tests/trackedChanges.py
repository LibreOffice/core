# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# tests for tracked changes ; tdf912270
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text

class trackedchanges(UITestCase):

    def test_tdf91270(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            type_text(xWriterEdit, "Test")

            self.xUITest.executeCommand(".uno:TrackChanges")

            self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
            self.xUITest.executeCommand(".uno:Cut")   #cut  text

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close"):
                pass

    def test_tracked_changes_accept(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:TrackChanges")
            type_text(xWriterEdit, "Test LibreOffice")
            self.xUITest.executeCommand(".uno:ShowTrackedChanges")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                xAccBtn = xTrackDlg.getChild("accept")
                xAccBtn.executeAction("CLICK", tuple())

            self.assertEqual(document.Text.String[0:16], "Test LibreOffice")

    def test_tracked_changes_acceptall(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:TrackChanges")
            type_text(xWriterEdit, "Test LibreOffice")
            self.xUITest.executeCommand(".uno:ShowTrackedChanges")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:

                xAccBtn = xTrackDlg.getChild("acceptall")
                xAccBtn.executeAction("CLICK", tuple())


            self.assertEqual(document.Text.String[0:16], "Test LibreOffice")

    def test_tracked_changes_reject(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:TrackChanges")
            type_text(xWriterEdit, "Test LibreOffice")
            self.xUITest.executeCommand(".uno:ShowTrackedChanges")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:

                xRejBtn = xTrackDlg.getChild("reject")
                xRejBtn.executeAction("CLICK", tuple())

            self.assertEqual(document.Text.String[0:1], "")

    def test_tracked_changes_rejectall(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:TrackChanges")
            type_text(xWriterEdit, "Test LibreOffice")
            self.xUITest.executeCommand(".uno:ShowTrackedChanges")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:

                xAccBtn = xTrackDlg.getChild("rejectall")
                xAccBtn.executeAction("CLICK", tuple())

            self.assertEqual(document.Text.String[0:1], "")

    def test_tracked_changes_zprev_next(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:
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


    def test_list_of_changes(self):
        with self.ui_test.load_file(get_url_for_data_file("trackedChanges.odt")) as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            listText = [
                    "Unknown Author\t01/24/2020 16:19:32\t",
                    "Unknown Author\t01/24/2020 16:19:35\t",
                    "Unknown Author\t01/24/2020 16:19:39\t",
                    "Unknown Author\t01/24/2020 16:19:39\t",
                    "Xisco Fauli\t01/27/2020 17:42:55\t"]

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
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


    def test_tdf135018(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf135018.odt")) as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.assertEqual(5, document.CurrentController.PageCount)

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")
                self.assertEqual(147, len(changesList.getChildren()))

                # Without the fix in place, it would have crashed here
                xAccBtn = xTrackDlg.getChild("acceptall")
                xAccBtn.executeAction("CLICK", tuple())

                self.assertEqual(0, len(changesList.getChildren()))

                xUndoBtn = xTrackDlg.getChild("undo")
                xUndoBtn.executeAction("CLICK", tuple())

                self.assertEqual(147, len(changesList.getChildren()))


            # Check the changes are shown after opening the Manage Tracked Changes dialog
            self.assertGreater(document.CurrentController.PageCount, 5)

    def test_tdf144270_tracked_table_rows(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            tables = document.getTextTables()
            self.assertEqual(3, len(tables))

            # Accept
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")

                # This was 14 (every cell is a different change instead of counting rows or tables)
                # Now: 4 changes (2 deleted/inserted rows and 2 deleted/inserted tables)
                self.assertEqual(4, len(changesList.getChildren()))

                # Without the fix in place, it would have crashed here
                for i in (3, 2, 1, 0):
                    xAccBtn = xTrackDlg.getChild("accept")
                    xAccBtn.executeAction("CLICK", tuple())
                    self.assertEqual(i, len(changesList.getChildren()))

                tables = document.getTextTables()
                self.assertEqual(2, len(tables))

                for i in range(1, 5):
                    xUndoBtn = xTrackDlg.getChild("undo")
                    xUndoBtn.executeAction("CLICK", tuple())
                    self.assertEqual(i, len(changesList.getChildren()))

            tables = document.getTextTables()
            self.assertEqual(3, len(tables))

            # Accept All
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")

                # This was 14 (every cell is a different change instead of counting rows or tables)
                # Now: 4 changes (2 deleted/inserted rows and 2 deleted/inserted tables)
                self.assertEqual(4, len(changesList.getChildren()))

                xAccBtn = xTrackDlg.getChild("acceptall")
                xAccBtn.executeAction("CLICK", tuple())
                self.assertEqual(0, len(changesList.getChildren()))

                tables = document.getTextTables()
                self.assertEqual(2, len(tables))

                xUndoBtn = xTrackDlg.getChild("undo")
                xUndoBtn.executeAction("CLICK", tuple())
                self.assertEqual(4, len(changesList.getChildren()))

            tables = document.getTextTables()
            self.assertEqual(3, len(tables))

            # goto to the start of the document to reject from the first tracked table row change
            self.xUITest.executeCommand(".uno:GoToStartOfDoc")

            # Reject
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")

                # This was 14 (every cell is a different change instead of counting rows or tables)
                # Now: 4 changes (2 deleted/inserted rows and 2 deleted/inserted tables)
                self.assertEqual(4, len(changesList.getChildren()))

                # Without the fix in place, it would have crashed here
                for i in (3, 2, 1, 0):
                    xAccBtn = xTrackDlg.getChild("reject")
                    xAccBtn.executeAction("CLICK", tuple())
                    self.assertEqual(i, len(changesList.getChildren()))

                tables = document.getTextTables()
                self.assertEqual(2, len(tables))

                for i in range(1, 5):
                    xUndoBtn = xTrackDlg.getChild("undo")
                    xUndoBtn.executeAction("CLICK", tuple())
                    self.assertEqual(i, len(changesList.getChildren()))

            tables = document.getTextTables()
            self.assertEqual(3, len(tables))

            # Reject All
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")

                # This was 14 (every cell is a different change instead of counting rows or tables)
                # Now: 4 changes (2 deleted/inserted rows and 2 deleted/inserted tables)
                self.assertEqual(4, len(changesList.getChildren()))

                xAccBtn = xTrackDlg.getChild("rejectall")
                xAccBtn.executeAction("CLICK", tuple())
                self.assertEqual(0, len(changesList.getChildren()))

                tables = document.getTextTables()
                self.assertEqual(2, len(tables))

                xUndoBtn = xTrackDlg.getChild("undo")
                xUndoBtn.executeAction("CLICK", tuple())
                self.assertEqual(4, len(changesList.getChildren()))

            tables = document.getTextTables()
            self.assertEqual(3, len(tables))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
