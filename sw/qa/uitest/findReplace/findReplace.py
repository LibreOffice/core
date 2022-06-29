# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text

class findReplace(UITestCase):
    def test_find_writer(self):
        with self.ui_test.load_file(get_url_for_data_file("findReplace.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:

                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"second"}))  #2nd page
                xsearch = xDialog.getChild("search")
                xsearch.executeAction("CLICK", tuple())
                #verify
                self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")

                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"third"}))
                xsearch.executeAction("CLICK", tuple())
                #verify
                self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "3")  #3rd page


            #now open dialog and verify find="third" (remember last value); replace value with "First" ( click match case) with word "Replace" - click twice Replace button, check "Replace first first"
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                searchterm = xDialog.getChild("searchterm")
                self.assertEqual(get_state_as_dict(searchterm)["Text"], "third")
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"First"}))
                matchcase = xDialog.getChild("matchcase")
                matchcase.executeAction("CLICK", tuple())  #click match case
                replaceterm = xDialog.getChild("replaceterm")
                replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"Replace"})) #replace textbox
                replace = xDialog.getChild("replace")
                replace.executeAction("CLICK", tuple())
                replace.executeAction("CLICK", tuple())   #click twice Replace button
                #verify
                self.assertEqual(writer_doc.Text.String[0:19], "Replace first first")

                #now replace first (uncheck match case) with word "aaa" - click once Replace All button, check "Replace aaa aaa"
                replaceterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                replaceterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"aaa"}))
                matchcase.executeAction("CLICK", tuple())  #uncheck match case
                replaceall = xDialog.getChild("replaceall")
                replaceall.executeAction("CLICK", tuple())
                #verify
                self.assertEqual(writer_doc.Text.String[0:15], "Replace aaa aaa")

                #now delete strings in textbox Find and textbox Replace, click button Format, select  Size=16 pt, Close OK, Find Next; verify Page=2;
                #click No format button; close dialog
                replaceterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                replaceterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))

                format = xDialog.getChild("format")
                with self.ui_test.execute_blocking_action(format.executeAction, args=('CLICK', ())) as dialog:
                    xTabs = dialog.getChild("tabcontrol")
                    select_pos(xTabs, "0")
                    xSizeFont = dialog.getChild("cbWestSize")
                    xSizeFont.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xSizeFont.executeAction("TYPE", mkPropertyValues({"TEXT":"16"}))    #set font size 16

                # Verify these didn't get set again through SvxSearchController::StateChanged, timer-
                # triggered from SfxBindings::NextJob while executing the Format dialog above:
                self.assertEqual(get_state_as_dict(searchterm)["Text"], "")
                self.assertEqual(get_state_as_dict(replaceterm)["Text"], "")

                xsearch = xDialog.getChild("search")
                xsearch.executeAction("CLICK", tuple())
                #verify
                self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")
                noformat = xDialog.getChild("noformat")
                noformat.executeAction("CLICK", tuple())   #click No format button


        #Bug 39022 - find-replace->$1, not pattern
    def test_tdf39022_replace_regexp(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            type_text(xWriterEdit, "test number1 testnot")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"T(est|other)\\>"}))   #find
                replaceterm = xDialog.getChild("replaceterm")
                replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"replaced$1"})) #replace

                # Deselect similarity before selecting regex
                xSimilarity = xDialog.getChild("similarity")
                if get_state_as_dict(xSimilarity)['Selected'] == 'true':
                    xSimilarity.executeAction("CLICK", tuple())

                regexp = xDialog.getChild("regexp")
                regexp.executeAction("CLICK", tuple())
                self.assertEqual("true", get_state_as_dict(regexp)['Selected'])
                replaceall = xDialog.getChild("replaceall")
                replaceall.executeAction("CLICK", tuple())
                #verify
                self.assertEqual(document.Text.String[0:27], "replacedest number1 testnot")

                # Deselect regex button, otherwise it might affect other tests
                regexp.executeAction("CLICK", tuple())
                self.assertEqual("false", get_state_as_dict(regexp)['Selected'])

        #tdf116242  ţ ț
    def test_tdf116242_replace_t_with_cedilla(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf116242.odt")) as writer_doc:

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"ţ"}))   #find
                replaceterm = xDialog.getChild("replaceterm")
                replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"ț"})) #replace
                includediacritics = xDialog.getChild("includediacritics")
                if (get_state_as_dict(includediacritics)["Selected"]) == "false":
                    includediacritics.executeAction("CLICK", tuple())
                replaceall = xDialog.getChild("replaceall")
                replaceall.executeAction("CLICK", tuple())
                #verify
                self.assertEqual(writer_doc.Text.String[0:13], "țoootdf116242")


        #Bug 98417 - FIND & REPLACE: Add 'Find Previous' button
    def test_tdf98417_find_previous_writer(self):
        with self.ui_test.load_file(get_url_for_data_file("findReplace.odt")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:

                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"third"}))
                xsearch = xDialog.getChild("search")
                xsearch.executeAction("CLICK", tuple())
                #verify
                self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "3")
                xsearch.executeAction("CLICK", tuple())
                #verify
                self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "4")   #page 4
                backsearch = xDialog.getChild("backsearch")
                backsearch.executeAction("CLICK", tuple())
                #verify
                self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "3")

    def test_tdf136577(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, "x")

            self.assertEqual(document.Text.String, "x")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:

                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"x"}))

                replaceall = xDialog.getChild("replaceall")
                replaceall.executeAction("CLICK", tuple())

                self.assertEqual(document.Text.String, "")

                self.xUITest.executeCommand(".uno:Undo")

                # Without the fix in place, this test would have failed with AssertionError: '' != 'x'
                self.assertEqual(document.Text.String, "x")

    def test_tdf143128(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, "ß")

            self.assertEqual(document.Text.String, "ß")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:

                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"ẞ"}))

                replaceterm = xDialog.getChild("replaceterm")
                replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"SS"}))

                replaceall = xDialog.getChild("replaceall")
                replaceall.executeAction("CLICK", tuple())

                # Without the fix in place, this test would have failed with
                # AssertionError: 'ß' != 'SS'
                self.assertEqual(document.Text.String, "SS")

                self.xUITest.executeCommand(".uno:Undo")

                self.assertEqual(document.Text.String, "ß")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
