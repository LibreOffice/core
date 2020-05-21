# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.debug import sleep
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

class writerWordCount(UITestCase):

    def test_word_count_dialog(self):

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()

        type_text(xWriterEdit, "Test for word count dialog") #type text
        xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "0", "END_POS": "8"})) #select two words

        self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xselectwords = xDialog.getChild("selectwords")
        xdocwords = xDialog.getChild("docwords")
        xselectchars = xDialog.getChild("selectchars")
        xdocchars = xDialog.getChild("docchars")
        xselectcharsnospaces = xDialog.getChild("selectcharsnospaces")
        xdoccharsnospaces = xDialog.getChild("doccharsnospaces")
        xselectcjkchars = xDialog.getChild("selectcjkchars")
        xdoccjkchars = xDialog.getChild("doccjkchars")

        self.assertEqual(get_state_as_dict(xselectwords)["Text"], "2")
        self.assertEqual(get_state_as_dict(xdocwords)["Text"], "5")
        self.assertEqual(get_state_as_dict(xselectchars)["Text"], "8")
        self.assertEqual(get_state_as_dict(xdocchars)["Text"], "26")
        self.assertEqual(get_state_as_dict(xselectcharsnospaces)["Text"], "7")
        self.assertEqual(get_state_as_dict(xdoccharsnospaces)["Text"], "22")
        self.assertEqual(get_state_as_dict(xselectcjkchars)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdoccjkchars)["Text"], "0")
        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.close_doc()

    def test_tdf68347(self):
        #Bug 68347 - Incorrect word count in a document with recorded changes
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf68347.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()

        xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "24", "END_POS": "39"})) #select two words

        self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xselectwords = xDialog.getChild("selectwords")
        xdocwords = xDialog.getChild("docwords")
        xselectchars = xDialog.getChild("selectchars")
        xdocchars = xDialog.getChild("docchars")
        xselectcharsnospaces = xDialog.getChild("selectcharsnospaces")
        xdoccharsnospaces = xDialog.getChild("doccharsnospaces")
        xselectcjkchars = xDialog.getChild("selectcjkchars")
        xdoccjkchars = xDialog.getChild("doccjkchars")

        self.assertEqual(get_state_as_dict(xselectwords)["Text"], "4")
        self.assertEqual(get_state_as_dict(xdocwords)["Text"], "12")
        self.assertEqual(get_state_as_dict(xselectchars)["Text"], "15")
        self.assertEqual(get_state_as_dict(xdocchars)["Text"], "54")
        self.assertEqual(get_state_as_dict(xselectcharsnospaces)["Text"], "12")
        #bug Bug 117703 Word Count: Wrong result for "Characters excluding spaces"
        #self.assertEqual(get_state_as_dict(xdoccharsnospaces)["Text"], "44")
        self.assertEqual(get_state_as_dict(xselectcjkchars)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdoccjkchars)["Text"], "0")
        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.close_doc()

    def test_tdf91100(self):
        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()

        self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.close_doc()

    def test_tdf58050(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf58050.html"))

        self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xselectwords = xDialog.getChild("selectwords")
        xdocwords = xDialog.getChild("docwords")
        xselectchars = xDialog.getChild("selectchars")
        xdocchars = xDialog.getChild("docchars")
        xselectcharsnospaces = xDialog.getChild("selectcharsnospaces")
        xdoccharsnospaces = xDialog.getChild("doccharsnospaces")
        xselectcjkchars = xDialog.getChild("selectcjkchars")
        xdoccjkchars = xDialog.getChild("doccjkchars")

        self.assertEqual(get_state_as_dict(xselectwords)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdocwords)["Text"], "3")
        self.assertEqual(get_state_as_dict(xselectchars)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdocchars)["Text"], "14")
        self.assertEqual(get_state_as_dict(xselectcharsnospaces)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdoccharsnospaces)["Text"], "12")
        self.assertEqual(get_state_as_dict(xselectcjkchars)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdoccjkchars)["Text"], "0")
        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.close_doc()

    def test_word_count_interpunction_counted_tdf56975_a(self):

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()
        #Open writer, enter "Testing one two! Test?"
        type_text(xWriterEdit, "Testing one two! Test?")
        #-> LO says: 4 words. SUCCESS! :)
        self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xselectwords = xDialog.getChild("selectwords")
        xdocwords = xDialog.getChild("docwords")
        xselectchars = xDialog.getChild("selectchars")
        xdocchars = xDialog.getChild("docchars")
        xselectcharsnospaces = xDialog.getChild("selectcharsnospaces")
        xdoccharsnospaces = xDialog.getChild("doccharsnospaces")
        xselectcjkchars = xDialog.getChild("selectcjkchars")
        xdoccjkchars = xDialog.getChild("doccjkchars")

        self.assertEqual(get_state_as_dict(xselectwords)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdocwords)["Text"], "4")
        self.assertEqual(get_state_as_dict(xselectchars)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdocchars)["Text"], "22")
        self.assertEqual(get_state_as_dict(xselectcharsnospaces)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdoccharsnospaces)["Text"], "19")
        self.assertEqual(get_state_as_dict(xselectcjkchars)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdoccjkchars)["Text"], "0")
        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.close_doc()

    def test_word_count_interpunction_counted_tdf56975_b(self):

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()
        #1. Create a new text document.
        #2. Type-in the words:
        #     This is a test sentence.
        type_text(xWriterEdit, "This is a test sentence.")
        #3. Open the word count dialogue.
        #   Word count in both, dialogue and status line, shows 5 words.
        self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xselectwords = xDialog.getChild("selectwords")
        xdocwords = xDialog.getChild("docwords")
        xselectchars = xDialog.getChild("selectchars")
        xdocchars = xDialog.getChild("docchars")
        xselectcharsnospaces = xDialog.getChild("selectcharsnospaces")
        xdoccharsnospaces = xDialog.getChild("doccharsnospaces")
        xselectcjkchars = xDialog.getChild("selectcjkchars")
        xdoccjkchars = xDialog.getChild("doccjkchars")

        self.assertEqual(get_state_as_dict(xselectwords)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdocwords)["Text"], "5")
        self.assertEqual(get_state_as_dict(xselectchars)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdocchars)["Text"], "24")
        self.assertEqual(get_state_as_dict(xselectcharsnospaces)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdoccharsnospaces)["Text"], "20")
        self.assertEqual(get_state_as_dict(xselectcjkchars)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdoccjkchars)["Text"], "0")
        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)
        #4. Select the space between 'a' and 'test'.
        xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "9", "END_POS": "10"}))
        #5. Replace selection by a non-breaking space by pressing Shift+Ctrl+Space.  Don't move the cursor.
        self.xUITest.executeCommand(".uno:InsertNonBreakingSpace")
        #   Word count in dialogue shows 4 words, whereas in the status line it shows 5 words.
        self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xselectwords = xDialog.getChild("selectwords")
        xdocwords = xDialog.getChild("docwords")
        xselectchars = xDialog.getChild("selectchars")
        xdocchars = xDialog.getChild("docchars")
        xselectcharsnospaces = xDialog.getChild("selectcharsnospaces")
        xdoccharsnospaces = xDialog.getChild("doccharsnospaces")
        xselectcjkchars = xDialog.getChild("selectcjkchars")
        xdoccjkchars = xDialog.getChild("doccjkchars")

        self.assertEqual(get_state_as_dict(xselectwords)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdocwords)["Text"], "5")
        self.assertEqual(get_state_as_dict(xselectchars)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdocchars)["Text"], "24")
        self.assertEqual(get_state_as_dict(xselectcharsnospaces)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdoccharsnospaces)["Text"], "20")
        self.assertEqual(get_state_as_dict(xselectcjkchars)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdoccjkchars)["Text"], "0")
        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)
        #6. Move the cursor by pressing Left.
        self.xUITest.executeCommand(".uno:GoLeft")
        #   Word count in both, dialogue and status line, shows 5 words.
        self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xselectwords = xDialog.getChild("selectwords")
        xdocwords = xDialog.getChild("docwords")
        xselectchars = xDialog.getChild("selectchars")
        xdocchars = xDialog.getChild("docchars")
        xselectcharsnospaces = xDialog.getChild("selectcharsnospaces")
        xdoccharsnospaces = xDialog.getChild("doccharsnospaces")
        xselectcjkchars = xDialog.getChild("selectcjkchars")
        xdoccjkchars = xDialog.getChild("doccjkchars")

        self.assertEqual(get_state_as_dict(xselectwords)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdocwords)["Text"], "5")
        self.assertEqual(get_state_as_dict(xselectchars)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdocchars)["Text"], "24")
        self.assertEqual(get_state_as_dict(xselectcharsnospaces)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdoccharsnospaces)["Text"], "20")
        self.assertEqual(get_state_as_dict(xselectcjkchars)["Text"], "0")
        self.assertEqual(get_state_as_dict(xdoccjkchars)["Text"], "0")
        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)
        self.ui_test.close_doc()

    def test_tdf51816(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf51816.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()
        #1. Open attached document
        #2. Tools> Word count
        self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xselectwords = xDialog.getChild("selectwords")
        xdocwords = xDialog.getChild("docwords")
        xselectchars = xDialog.getChild("selectchars")
        xdocchars = xDialog.getChild("docchars")
        xselectcharsnospaces = xDialog.getChild("selectcharsnospaces")
        xdoccharsnospaces = xDialog.getChild("doccharsnospaces")
        xselectcjkchars = xDialog.getChild("selectcjkchars")
        xdoccjkchars = xDialog.getChild("doccjkchars")

        #3. Click after "At nunc" then <Ctrl><Shift><Left>
        self.xUITest.executeCommand(".uno:GoRight")
        self.xUITest.executeCommand(".uno:GoRight")
        self.xUITest.executeCommand(".uno:GoRight")
        self.xUITest.executeCommand(".uno:GoRight")
        self.xUITest.executeCommand(".uno:GoRight")
        self.xUITest.executeCommand(".uno:GoRight")
        self.xUITest.executeCommand(".uno:GoRight")
        self.xUITest.executeCommand(".uno:WordLeftSel")

        #needs to wait, because Word count dialog is already open and it takes time to refresh the counter
        #Expected result : Words 1 & Characters 4 #Actual result : Words 0 & Characters 0
        self.assertEqualWithTimeout(xselectwords, {'Text': '1'})
        self.assertEqual(get_state_as_dict(xselectchars)["Text"], "4")

        #4. Click after "At nunc" then <Shift><Home>
        self.xUITest.executeCommand(".uno:StartOfParaSel")

        #needs to wait, because Word count dialog is already open and it takes time to refresh the counter
        #Expected result : Words 2 & Characters 7 & excluding space 6  #Actual result : Words 0 & Characters 0
        self.assertEqualWithTimeout(xselectwords, {'Text': '2'})
        self.assertEqual(get_state_as_dict(xselectchars)["Text"], "7")

        self.assertEqual(get_state_as_dict(xselectcharsnospaces)["Text"], "6")

        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
