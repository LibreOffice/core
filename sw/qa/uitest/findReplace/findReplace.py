# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
import org.libreoffice.unotest
import pathlib
def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Find and replace
#tdf118208/118212 - enabling either CJK or CTL, or both (Tools -> Options -> Language Settings -> Languages: Default Languages for Documents checkboxes)
#eliminates all crashes. Setting back to Western only recreates the crashes.   - DONE
#tdf116242  ţ ț - DONE
#Bug 98417 - FIND & REPLACE: Add 'Find Previous' button - DONE
#Bug 39022 - find-replace->$1, not pattern  - DONE

class findReplace(UITestCase):
    def test_find_writer(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("findReplace.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()

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

        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        #now open dialog and verify find="third" (remember last value); replace value with "First" ( click match case) with word "Replace" - click twice Replace button, check "Replace first first"
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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
        self.assertEqual(document.Text.String[0:19], "Replace first first")

        #now replace first (uncheck match case) with word "aaa" - click once Replace All button, check "Replace aaa aaa"
        replaceterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        replaceterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"aaa"}))
        matchcase.executeAction("CLICK", tuple())  #uncheck match case
        replaceall = xDialog.getChild("replaceall")
        replaceall.executeAction("CLICK", tuple())
        #verify
        self.assertEqual(document.Text.String[0:15], "Replace aaa aaa")

        #now delete strings in textbox Find and textbox Replace, click button Format, select  Size=16 pt, Close OK, Find Next; verify Page=2;
        #click No format button; close dialog
        replaceterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        replaceterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))

        format = xDialog.getChild("format")
        def handle_format_dlg(dialog):
            #print(dialog.getChildren())
            xTabs = dialog.getChild("tabcontrol")
            select_pos(xTabs, "0")
            xSizeFont = dialog.getChild("westsizelb-cjk")
            xSizeFont.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
            xSizeFont.executeAction("BACKSPACE", tuple())
            xSizeFont.executeAction("TYPE", mkPropertyValues({"TEXT":"16"}))    #set font size 16
            xOkBtn = dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.execute_blocking_action(format.executeAction, args=('CLICK', ()),
                dialog_handler=handle_format_dlg)

        xsearch = xDialog.getChild("search")
        xsearch.executeAction("CLICK", tuple())
        #verify
        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")
        noformat = xDialog.getChild("noformat")
        noformat.executeAction("CLICK", tuple())   #click No format button

        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        self.ui_test.close_doc()

        #Bug 39022 - find-replace->$1, not pattern
    def test_tdf39022_replace_regexp(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        type_text(xWriterEdit, "test number1 testnot")

        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        searchterm = xDialog.getChild("searchterm")
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"T(est|other)\>"}))   #find
        replaceterm = xDialog.getChild("replaceterm")
        replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"replaced$1"})) #replace
        regexp = xDialog.getChild("regexp")
        regexp.executeAction("CLICK", tuple())   #regular expressions
        replaceall = xDialog.getChild("replaceall")
        replaceall.executeAction("CLICK", tuple())
        #verify
        self.assertEqual(document.Text.String[0:27], "replacedest number1 testnot")
        regexp.executeAction("CLICK", tuple())

        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        self.ui_test.close_doc()

        #tdf116242  ţ ț
    def test_tdf116242_replace_t_with_cedilla(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf116242.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
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
        self.assertEqual(document.Text.String[0:13], "țoootdf116242")

        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        self.ui_test.close_doc()

        #tdf118208/118212 - enabling either CJK or CTL, or both (Tools -> Options -> Language Settings -> Languages: Default Languages for Documents checkboxes)
        #eliminates all crashes. Launching Character dialog from any module crashes ( with CJK and CTL disabled )
    def test_tdf118208_search_dialog_format_crash(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf118208.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        # 1. Open the attached file.
        # 2. Press ctrl-H to show the search and replace dialog.
        # 3. Press the "Format..."  button.
        #Libreoffice immediately crashed.
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialog = self.xUITest.getTopFocusWindow()

        xPages = xDialog.getChild("pages")
        xLanguageEntry = xPages.getChild('2')                 # Language Settings
        xLanguageEntry.executeAction("EXPAND", tuple())
        xxLanguageEntryGeneralEntry = xLanguageEntry.getChild('0')
        xxLanguageEntryGeneralEntry.executeAction("SELECT", tuple())          #Language

        asianlanguage = xDialog.getChild("asiansupport")
        complexlanguage = xDialog.getChild("ctlsupport")
        if (get_state_as_dict(asianlanguage)["Selected"]) == "true":
            asianlanguage.executeAction("CLICK", tuple())
        if (get_state_as_dict(complexlanguage)["Selected"]) == "true":
            complexlanguage.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        format = xDialog.getChild("format")
        def handle_format_dlg(dialog):
            xOkBtn = dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.execute_blocking_action(format.executeAction, args=('CLICK', ()),
                dialog_handler=handle_format_dlg)

        #verify
        self.assertEqual(document.Text.String[0:14], "Aaaaaaaaaaaaaa")
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        #enable lang support again
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialog = self.xUITest.getTopFocusWindow()

        xPages = xDialog.getChild("pages")
        xLanguageEntry = xPages.getChild('2')                 # Language Settings
        xLanguageEntry.executeAction("EXPAND", tuple())
        xxLanguageEntryGeneralEntry = xLanguageEntry.getChild('0')
        xxLanguageEntryGeneralEntry.executeAction("SELECT", tuple())          #Language

        asianlanguage = xDialog.getChild("asiansupport")
        complexlanguage = xDialog.getChild("ctlsupport")
        if (get_state_as_dict(asianlanguage)["Selected"]) == "false":
            asianlanguage.executeAction("CLICK", tuple())
        if (get_state_as_dict(complexlanguage)["Selected"]) == "false":
            complexlanguage.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

        #Bug 98417 - FIND & REPLACE: Add 'Find Previous' button
    def test_tdf98417_find_previous_writer(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("findReplace.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()

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

        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
