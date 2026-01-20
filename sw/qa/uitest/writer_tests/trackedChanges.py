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
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text, select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from tempfile import TemporaryDirectory
from org.libreoffice.unotest import systemPathToFileUrl
import os.path

class trackedchanges(UITestCase):

    def test_tdf91270(self):

        with self.ui_test.create_doc_in_start_center("writer"):

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
                changesList = xTrackDlg.getChild("writerchanges")
                changesList.getChild(0).executeAction("SELECT", tuple())

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

                changesList.getChild(0).executeAction("SELECT", tuple())

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

                changesList.getChild(0).executeAction("SELECT", tuple())

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

            self.assertEqual(5, document.CurrentController.PageCount)

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")
                self.assertEqual(111, len(changesList.getChildren()))

                # Without the fix in place, it would have crashed here
                xAccBtn = xTrackDlg.getChild("acceptall")
                xAccBtn.executeAction("CLICK", tuple())

                self.assertEqual(0, len(changesList.getChildren()))

                xUndoBtn = xTrackDlg.getChild("undo")
                xUndoBtn.executeAction("CLICK", tuple())

                self.assertEqual(111, len(changesList.getChildren()))


            # Check the changes are shown after opening the Manage Tracked Changes dialog
            self.assertGreater(document.CurrentController.PageCount, 5)

    def test_tdf144270_tracked_table_rows(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as document:

            tables = document.getTextTables()
            self.assertEqual(3, len(tables))

            # Accept
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")

                # This was 14 (every cell is a different change instead of counting rows or tables)
                # Now: 4 changes (2 deleted/inserted rows and 2 deleted/inserted tables)
                self.assertEqual(4, len(changesList.getChildren()))

                changesList.getChild(0).executeAction("SELECT", tuple())

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
            self.xUITest.executeCommand(".uno:GoToStartOfDoc")  # start of cell
            self.xUITest.executeCommand(".uno:GoToStartOfDoc")  # start of table
            self.xUITest.executeCommand(".uno:GoToStartOfDoc")  # start of document

            # Reject
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")

                # This was 14 (every cell is a different change instead of counting rows or tables)
                # Now: 4 changes (2 deleted/inserted rows and 2 deleted/inserted tables)
                self.assertEqual(4, len(changesList.getChildren()))

                changesList.getChild(0).executeAction("SELECT", tuple())

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

    def test_tdf148032(self):

        with self.ui_test.load_file(get_url_for_data_file("trackedChanges.odt")):

            # adding new Comment
            self.xUITest.executeCommand(".uno:InsertAnnotation")

            # wait until the comment is available
            xComment1 = self.ui_test.wait_until_child_is_available('Comment1')

            xEditView1 = xComment1.getChild("editview")
            xEditView1.executeAction("TYPE", mkPropertyValues({"TEXT": "This is the First Comment"}))
            self.assertEqual(get_state_as_dict(xComment1)["Text"], "This is the First Comment" )
            self.assertEqual(get_state_as_dict(xComment1)["Resolved"], "false" )
            self.assertEqual(get_state_as_dict(xComment1)["Author"], "Unknown Author" )
            self.assertEqual(get_state_as_dict(xComment1)["ReadOnly"], "false" )

            xComment1.executeAction("LEAVE", mkPropertyValues({}))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")
                self.assertEqual(6, len(changesList.getChildren()))

                xChild = changesList.getChild(0)
                # This was False (missing comment)
                self.assertEqual(True, get_state_as_dict(xChild)["Text"].endswith('\tComment added'))


    def get_annotation_count(self, document):
            n = 0
            textfields = document.getTextFields()
            for textfield in textfields:
                if textfield.supportsService("com.sun.star.text.TextField.Annotation"):
                    n = n + 1
            return n

    def test_tdf153016_annotation_in_DOC(self):
        # load a test document, and add a tracked comment
        with TemporaryDirectory() as tempdir:
                xFilePath = os.path.join(tempdir, 'temp_drop_down_form_field.doc')

                with self.ui_test.load_file(get_url_for_data_file('drop_down_form_field.doc')) as document:

                    self.xUITest.executeCommand(".uno:TrackChanges")
                    self.xUITest.executeCommand('.uno:SelectAll')

                    self.assertEqual(0, self.get_annotation_count(document))

                    self.xUITest.executeCommand('.uno:InsertAnnotation')

                    self.assertEqual(1, self.get_annotation_count(document))

                    # Save Copy as
                    with self.ui_test.execute_dialog_through_command('.uno:SaveAs', close_button="") as xDialog:
                        xFileName = xDialog.getChild('file_name')
                        xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'CTRL+A'}))
                        xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'BACKSPACE'}))
                        xFileName.executeAction('TYPE', mkPropertyValues({'TEXT': xFilePath}))

                        xOpen = xDialog.getChild("open")
                        # DOC confirmation dialog is displayed
                        with self.ui_test.execute_dialog_through_action(xOpen, "CLICK", close_button="btnYes"):
                            pass

                with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as document:
                    # This was 2
                    self.assertEqual(1, self.get_annotation_count(document))


    def test_tdf147179(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            tables = document.getTextTables()
            self.assertEqual(3, len(tables))

            # Select text of the tracked row, not only text of its first cell
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")

                # select second tracked table row in tree list
                changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
                xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
                while get_state_as_dict(xWriterEdit)["SelectedText"] != 'klj':
                    xToolkit.processEventsToIdle()

                # this was "j" (only text of the first cell was selected, not text of the row)
                self.assertEqual(get_state_as_dict(xWriterEdit)["SelectedText"], "klj" )

                # select first tracked table row in tree list
                changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
                while get_state_as_dict(xWriterEdit)["SelectedText"] != 'bca':
                    xToolkit.processEventsToIdle()

                # this was "a" (only text of the first cell was selected, not text of the row)
                self.assertEqual(get_state_as_dict(xWriterEdit)["SelectedText"], "bca" )

    def test_RedlineSuccessorData(self):
        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "redlinesuccessordata-temp.odt")
            with self.ui_test.load_file(get_url_for_data_file("redlinesuccessordata.docx")) as document:

                # check tracked deletion in tracked insertion
                with self.ui_test.execute_modeless_dialog_through_command('.uno:AcceptTrackedChanges', close_button="close") as xTrackDlg:
                    changesList = xTrackDlg.getChild('writerchanges')
                    # four children, but only three visible
                    state = get_state_as_dict(changesList)
                    self.assertEqual(state['Children'], '4')
                    self.assertEqual(state['VisibleCount'], '3')

                    # select tracked deletion with RedlineSuccessorData in tree list
                    changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
                    state = get_state_as_dict(changesList)
                    self.assertEqual(state['SelectEntryText'], 'Kelemen Gábor 2\t05/19/2021 12:35:00\t')
                    self.assertEqual(get_state_as_dict(changesList.getChild(1))['Children'], '1')
                    self.assertEqual(get_state_as_dict(changesList.getChild(1))['VisibleChildCount'], '0')

                    # open tree node with the tracked insertion: four visible children
                    changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
                    state = get_state_as_dict(changesList)
                    self.assertEqual(state['Children'], '4')
                    self.assertEqual(state['VisibleCount'], '4')
                    self.assertEqual(get_state_as_dict(changesList.getChild(1))['Children'], '1')
                    self.assertEqual(get_state_as_dict(changesList.getChild(1))['VisibleChildCount'], '1')

                    # select tracked insertion in tree list
                    changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
                    state = get_state_as_dict(changesList)
                    self.assertEqual(state['SelectEntryText'], 'First Person\t10/21/2012 23:45:00\t')

                # Save the DOCX document as ODT with a tracked deletion in a tracked insertion
                with self.ui_test.execute_dialog_through_command(".uno:SaveAs", close_button="open") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))
                    xFileTypeCombo = xSaveDialog.getChild("file_type")
                    select_by_text(xFileTypeCombo, "ODF Text Document (.odt)")

            # load the temporary file, and check ODF roundtrip of the tracked deletion in a tracked insertion
            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as document:
                # check tracked deletion in tracked insertion
                with self.ui_test.execute_modeless_dialog_through_command('.uno:AcceptTrackedChanges', close_button="close") as xTrackDlg:
                    changesList = xTrackDlg.getChild('writerchanges')
                    # four children, but only three visible
                    state = get_state_as_dict(changesList)
                    self.assertEqual(state['Children'], '4')
                    self.assertEqual(state['VisibleCount'], '3')

                    # select tracked deletion with RedlineSuccessorData in tree list
                    changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
                    state = get_state_as_dict(changesList)
                    self.assertEqual(state['SelectEntryText'], 'Kelemen Gábor 2\t05/19/2021 12:35:00\t')
                    self.assertEqual(get_state_as_dict(changesList.getChild(1))['Children'], '1')
                    self.assertEqual(get_state_as_dict(changesList.getChild(1))['VisibleChildCount'], '0')

                    # open tree node with the tracked insertion: four visible children
                    changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
                    state = get_state_as_dict(changesList)
                    self.assertEqual(state['Children'], '4')
                    self.assertEqual(state['VisibleCount'], '4')
                    self.assertEqual(get_state_as_dict(changesList.getChild(1))['Children'], '1')
                    self.assertEqual(get_state_as_dict(changesList.getChild(1))['VisibleChildCount'], '1')

                    # select tracked insertion in tree list
                    changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
                    state = get_state_as_dict(changesList)
                    self.assertEqual(state['SelectEntryText'], 'First Person\t10/21/2012 23:45:00\t')

                    # reject all
                    xAccBtn = xTrackDlg.getChild("rejectall")
                    xAccBtn.executeAction("CLICK", tuple())
                    # FIXME why we need double rejectall (dialog-only)?
                    xAccBtn.executeAction("CLICK", tuple())
                    self.assertEqual(0, len(changesList.getChildren()))
                    # This was false, because of not rejected tracked deletion
                    # of the text "inserts": "Document text inserts document"...
                    self.assertTrue(document.getText().getString().startswith('Document text document text'))

    def test_tdf155342_tracked_table_columns(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as document:

            # accept all changes and insert new columns with change tracking
            self.xUITest.executeCommand(".uno:AcceptAllTrackedChanges")
            tables = document.getTextTables()
            self.assertEqual(2, len(tables))
            self.assertEqual(len(tables[0].getColumns()), 3)
            self.xUITest.executeCommand(".uno:InsertColumnsAfter")
            self.xUITest.executeCommand(".uno:InsertColumnsAfter")
            self.assertEqual(len(tables[0].getColumns()), 5)

            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            # check and reject changes
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")
                changesList.getChild(0).executeAction("SELECT", tuple())

                # six changes, but only one visible in the Manage Changes dialog window
                state = get_state_as_dict(changesList)
                self.assertEqual(state['Children'], '6')
                self.assertEqual(state['VisibleCount'], '1')

                # This was 6 (every cell is a different change instead of counting column changes)
                # Now: 1 changes (2 inserted columns)
                self.assertEqual(1, len(changesList.getChildren()))

                # reject column insertion

                xAccBtn = xTrackDlg.getChild("reject")
                xAccBtn.executeAction("CLICK", tuple())

                # all inserted columns are removed

                self.assertEqual(len(tables[0].getColumns()), 3)

                # no changes in the dialog window

                self.assertEqual(0, len(changesList.getChildren()))

    def test_tdf155847_multiple_tracked_columns(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as document:

            # accept all changes and insert new columns with change tracking
            self.xUITest.executeCommand(".uno:AcceptAllTrackedChanges")
            tables = document.getTextTables()
            self.assertEqual(2, len(tables))
            self.assertEqual(len(tables[0].getColumns()), 3)
            self.xUITest.executeCommand(".uno:InsertColumnsAfter")
            self.xUITest.executeCommand(".uno:DeleteColumns")
            self.assertEqual(len(tables[0].getColumns()), 4)

            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            # check and reject changes
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")
                changesList.getChild(0).executeAction("SELECT", tuple())

                # six changes, but only one visible in the Manage Changes dialog window
                state = get_state_as_dict(changesList)
                self.assertEqual(state['Children'], '6')

                # This was 4 (missing handling of multiple different columns)
                self.assertEqual(state['VisibleCount'], '2')
                # Now: 2 changes (deleted and inserted columns)
                self.assertEqual(2, len(changesList.getChildren()))

                # accept column deletion

                xAccBtn = xTrackDlg.getChild("accept")
                xAccBtn.executeAction("CLICK", tuple())

                # deleted column is removed

                self.assertEqual(len(tables[0].getColumns()), 3)

                # single parent left in the dialog window
                self.assertEqual(1, len(changesList.getChildren()))

                # reject column insertion

                xAccBtn = xTrackDlg.getChild("reject")
                xAccBtn.executeAction("CLICK", tuple())

                # inserted column is removed

                self.assertEqual(len(tables[0].getColumns()), 2)

                # no changes in the dialog window

                self.assertEqual(0, len(changesList.getChildren()))

    def test_tdf155847_multiple_tracked_columns_crash(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as document:

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            # accept all changes and insert new columns with change tracking
            self.xUITest.executeCommand(".uno:AcceptAllTrackedChanges")
            tables = document.getTextTables()
            self.assertEqual(2, len(tables))
            self.assertEqual(len(tables[0].getColumns()), 3)
            self.xUITest.executeCommand(".uno:InsertColumnsAfter")
            self.xUITest.executeCommand(".uno:DeleteColumns")
            self.assertEqual(len(tables[0].getColumns()), 4)

            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            # check and reject changes
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")

                # six changes, but only one visible in the Manage Changes dialog window
                state = get_state_as_dict(changesList)
                self.assertEqual(state['Children'], '6')

                # This was 4 (missing handling of multiple different columns)
                self.assertEqual(state['VisibleCount'], '2')
                # Now: 2 changes (deleted and inserted columns)
                self.assertEqual(2, len(changesList.getChildren()))

                # select second tracked table column in tree list
                xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
                changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))

                # while not empty, i.e. starts with CH_TXT_TRACKED_DUMMY_CHAR
                while len(get_state_as_dict(xWriterEdit)["SelectedText"]):
                    xToolkit.processEventsToIdle()

                # reject column insertion

                xAccBtn = xTrackDlg.getChild("reject")
                # Without the fix in place, it would have crashed here
                xAccBtn.executeAction("CLICK", tuple())

                # inserted column is removed

                self.assertEqual(len(tables[0].getColumns()), 3)

                # single parent left in the dialog window
                self.assertEqual(1, len(changesList.getChildren()))

                # accept column deletion

                xAccBtn = xTrackDlg.getChild("accept")
                xAccBtn.executeAction("CLICK", tuple())

                # deleted column is removed

                self.assertEqual(len(tables[0].getColumns()), 2)

                # no changes in the dialog window

                self.assertEqual(0, len(changesList.getChildren()))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
