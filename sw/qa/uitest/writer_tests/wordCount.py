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
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text

class writerWordCount(UITestCase):

    def test_word_count_dialog(self):

        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, "Test for word count dialog") #type text
            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "0", "END_POS": "8"})) #select two words

            with self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog", close_button="close") as xDialog:

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


    def test_tdf68347(self):
        #Bug 68347 - Incorrect word count in a document with recorded changes
        with self.ui_test.load_file(get_url_for_data_file("tdf68347.odt")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "24", "END_POS": "39"})) #select two words

            with self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog", close_button="close") as xDialog:
                xselectwords = xDialog.getChild("selectwords")
                xdocwords = xDialog.getChild("docwords")
                xselectchars = xDialog.getChild("selectchars")
                xdocchars = xDialog.getChild("docchars")
                xselectcharsnospaces = xDialog.getChild("selectcharsnospaces")
                xselectcjkchars = xDialog.getChild("selectcjkchars")
                xdoccjkchars = xDialog.getChild("doccjkchars")

                self.assertEqual(get_state_as_dict(xselectwords)["Text"], "4")
                self.assertEqual(get_state_as_dict(xdocwords)["Text"], "12")
                self.assertEqual(get_state_as_dict(xselectchars)["Text"], "15")
                self.assertEqual(get_state_as_dict(xdocchars)["Text"], "54")
                self.assertEqual(get_state_as_dict(xselectcharsnospaces)["Text"], "12")
                #Bug 117703 Word Count: Wrong result for "Characters excluding spaces"
                #self.assertEqual(get_state_as_dict(xdoccharsnospaces)["Text"], "44")
                self.assertEqual(get_state_as_dict(xselectcjkchars)["Text"], "0")
                self.assertEqual(get_state_as_dict(xdoccjkchars)["Text"], "0")

    def test_tdf91100(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog", close_button="close"):
                pass

    def test_tdf58050(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf58050.html")):

            with self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog", close_button="close") as xDialog:

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

    def test_word_count_interpunction_counted_tdf56975_a(self):

        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            #Open writer, enter "Testing one two! Test?"
            type_text(xWriterEdit, "Testing one two! Test?")
            #-> LO says: 4 words. SUCCESS! :)
            with self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog", close_button="close") as xDialog:

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


    def test_word_count_interpunction_counted_tdf56975_b(self):

        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            #1. Create a new text document.
            #2. Type-in the words:
            #     This is a test sentence.
            type_text(xWriterEdit, "This is a test sentence.")
            #3. Open the word count dialogue.
            #   Word count in both, dialogue and status line, shows 5 words.
            with self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog", close_button="close") as xDialog:

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
            #4. Select the space between 'a' and 'test'.
            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "9", "END_POS": "10"}))
            #5. Replace selection by a non-breaking space by pressing Shift+Ctrl+Space.  Don't move the cursor.
            self.xUITest.executeCommand(".uno:InsertNonBreakingSpace")
            #   Word count in dialogue shows 4 words, whereas in the status line it shows 5 words.
            with self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog", close_button="close") as xDialog:

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
            #6. Move the cursor by pressing Left.
            self.xUITest.executeCommand(".uno:GoLeft")
            #   Word count in both, dialogue and status line, shows 5 words.
            with self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog", close_button="close") as xDialog:

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

    def test_tdf51816(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf51816.odt")):
            #1. Open attached document
            #2. Tools> Word count
            with self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog", close_button="close") as xDialog:
                xselectwords = xDialog.getChild("selectwords")
                xselectchars = xDialog.getChild("selectchars")
                xselectcharsnospaces = xDialog.getChild("selectcharsnospaces")

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
                self.ui_test.wait_until_property_is_updated(xselectwords, "Text", "1")
                self.assertEqual(get_state_as_dict(xselectwords)["Text"], "1")
                self.assertEqual(get_state_as_dict(xselectchars)["Text"], "4")

                #4. Click after "At nunc" then <Shift><Home>
                self.xUITest.executeCommand(".uno:StartOfParaSel")

                #needs to wait, because Word count dialog is already open and it takes time to refresh the counter
                #Expected result : Words 2 & Characters 7 & excluding space 6  #Actual result : Words 0 & Characters 0
                self.ui_test.wait_until_property_is_updated(xselectwords, "Text", "2")
                self.assertEqual(get_state_as_dict(xselectwords)["Text"], "2")
                self.assertEqual(get_state_as_dict(xselectchars)["Text"], "7")

                self.assertEqual(get_state_as_dict(xselectcharsnospaces)["Text"], "6")


    def test_tdf117703(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf117703.odt")):
            self.xUITest.getTopFocusWindow()

            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog", close_button="close") as xDialog:

                xselectwords = xDialog.getChild("selectwords")
                xdocwords = xDialog.getChild("docwords")
                xselectchars = xDialog.getChild("selectchars")
                xdocchars = xDialog.getChild("docchars")
                xselectcharsnospaces = xDialog.getChild("selectcharsnospaces")
                xdoccharsnospaces = xDialog.getChild("doccharsnospaces")
                xselectcjkchars = xDialog.getChild("selectcjkchars")
                xdoccjkchars = xDialog.getChild("doccjkchars")

                self.assertEqual(get_state_as_dict(xselectwords)["Text"], "12")
                self.assertEqual(get_state_as_dict(xdocwords)["Text"], "12")
                self.assertEqual(get_state_as_dict(xselectchars)["Text"], "54")
                self.assertEqual(get_state_as_dict(xdocchars)["Text"], "54")

                # Without the fix in place it would have failed with: AssertionError: '0' != '44'
                self.assertEqual(get_state_as_dict(xselectcharsnospaces)["Text"], "44")
                self.assertEqual(get_state_as_dict(xdoccharsnospaces)["Text"], "44")
                self.assertEqual(get_state_as_dict(xselectcjkchars)["Text"], "0")
                self.assertEqual(get_state_as_dict(xdoccjkchars)["Text"], "0")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
