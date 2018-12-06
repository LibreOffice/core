# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import time
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

class insertField(UITestCase):

    def test_insert_field_document_author(self):

        writer_doc = self.ui_test.load_file(get_url_for_data_file("insertField.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()

        #fill the Options-> User data
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")
        xProperties = self.xUITest.getTopFocusWindow()

        xCompanyText = xProperties.getChild("company")
        xCompanyText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xCompanyText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xCompanyText.executeAction("TYPE", mkPropertyValues({"TEXT":"Documentfoundation"}))

        xfirstnameText = xProperties.getChild("firstname")
        xfirstnameText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xfirstnameText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xfirstnameText.executeAction("TYPE", mkPropertyValues({"TEXT":"Libre"}))

        xlastnameText = xProperties.getChild("lastname")
        xlastnameText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xlastnameText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xlastnameText.executeAction("TYPE", mkPropertyValues({"TEXT":"Office"}))

        xstreetText = xProperties.getChild("street")
        xstreetText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xstreetText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xstreetText.executeAction("TYPE", mkPropertyValues({"TEXT":"Street"}))

        xCityText = xProperties.getChild("city")
        xCityText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xCityText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xCityText.executeAction("TYPE", mkPropertyValues({"TEXT":"City"}))

        xStateText = xProperties.getChild("state")
        xStateText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xStateText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xStateText.executeAction("TYPE", mkPropertyValues({"TEXT":"State"}))

        xZIPText = xProperties.getChild("zip")
        xZIPText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xZIPText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xZIPText.executeAction("TYPE", mkPropertyValues({"TEXT":"0000000"}))

        xCountryText = xProperties.getChild("country")
        xCountryText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xCountryText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xCountryText.executeAction("TYPE", mkPropertyValues({"TEXT":"Country"}))

        xTitleText = xProperties.getChild("title")
        xTitleText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xTitleText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xTitleText.executeAction("TYPE", mkPropertyValues({"TEXT":"Title_prop"}))

        xPositionText = xProperties.getChild("position")
        xPositionText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xPositionText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xPositionText.executeAction("TYPE", mkPropertyValues({"TEXT":"position"}))

        xHomephoneText = xProperties.getChild("home")
        xHomephoneText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xHomephoneText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xHomephoneText.executeAction("TYPE", mkPropertyValues({"TEXT":"0001"}))

        xWorkphoneText = xProperties.getChild("work")
        xWorkphoneText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xWorkphoneText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xWorkphoneText.executeAction("TYPE", mkPropertyValues({"TEXT":"0002"}))

        xFaxText = xProperties.getChild("fax")
        xFaxText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xFaxText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xFaxText.executeAction("TYPE", mkPropertyValues({"TEXT":"0003"}))

        xEmailText = xProperties.getChild("email")
        xEmailText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xEmailText.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xEmailText.executeAction("TYPE", mkPropertyValues({"TEXT":"email"}))

        xOkBtn = xProperties.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)   # save the data in properties dialog

        #check data saved in properties dialog
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")
        xProperties = self.xUITest.getTopFocusWindow()

        xCompanyText = xProperties.getChild("company")
        xfirstnameText = xProperties.getChild("firstname")
        xlastnameText = xProperties.getChild("lastname")
        xstreetText = xProperties.getChild("street")
        xCityText = xProperties.getChild("city")
        xStateText = xProperties.getChild("state")
        xZIPText = xProperties.getChild("zip")
        xCountryText = xProperties.getChild("country")
        xTitleText = xProperties.getChild("title")
        xPositionText = xProperties.getChild("position")
        xHomephoneText = xProperties.getChild("home")
        xWorkphoneText = xProperties.getChild("work")
        xEmailText = xProperties.getChild("email")
        xFaxText = xProperties.getChild("fax")

        self.assertEqual(get_state_as_dict(xCompanyText)["Text"], "Documentfoundation")
        self.assertEqual(get_state_as_dict(xfirstnameText)["Text"], "Libre")
        self.assertEqual(get_state_as_dict(xlastnameText)["Text"], "Office")
        self.assertEqual(get_state_as_dict(xstreetText)["Text"], "Street")
        self.assertEqual(get_state_as_dict(xCityText)["Text"], "City")
        self.assertEqual(get_state_as_dict(xStateText)["Text"], "State")
        self.assertEqual(get_state_as_dict(xZIPText)["Text"], "0000000")
        self.assertEqual(get_state_as_dict(xTitleText)["Text"], "Title_prop")
        self.assertEqual(get_state_as_dict(xPositionText)["Text"], "position")
        self.assertEqual(get_state_as_dict(xHomephoneText)["Text"], "0001")
        self.assertEqual(get_state_as_dict(xWorkphoneText)["Text"], "0002")
        self.assertEqual(get_state_as_dict(xFaxText)["Text"], "0003")
        self.assertEqual(get_state_as_dict(xEmailText)["Text"], "email")

        xCancBtn = xProperties.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancBtn)   # close the dialog

        # open and close dialog
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xCloseBtn = xCrossRef.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCloseBtn)   # close the dialog
        #Author ->Name
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "0")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "1")
        select_pos(xSelectListbox, "0")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:12], "Libre Office")   # assert the field's text Author ->Name
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        #Author ->Initials
#        self.xUITest.executeCommand(".uno:InsertField")
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "0")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "1")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:2], "LO")   # assert the field's text Author ->Initials
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        self.ui_test.close_doc()

    def test_insert_field_document_date(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #Date
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "1")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "0")
        xOffset = xCrossRef.getChild("offset")
        xOffset.executeAction("UP", tuple())  # offset date
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field Date
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text
        self.ui_test.close_doc()

    def test_insert_field_document_file_name(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("insertField.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        #File name ->File Name
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "0")

        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "2")
        xSelectListbox = xCrossRef.getChild("format")
        select_pos(xSelectListbox, "0")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:15], "insertField.odt")   # assert the field's text  File name ->File Name
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        #File name ->File Name without extension
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "2")
        xSelectListbox = xCrossRef.getChild("format")
        select_pos(xSelectListbox, "1")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:11], "insertField")   # assert the field's text  File name ->File Name without extension
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        #File name ->Path
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "2")
        xSelectListbox = xCrossRef.getChild("format")
        select_pos(xSelectListbox, "2")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        #File name ->Path/file_name
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "2")
        xSelectListbox = xCrossRef.getChild("format")
        select_pos(xSelectListbox, "3")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        self.ui_test.close_doc()

    def test_insert_field_document_chapter(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #Chapter
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "0")

        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "3")
        xSelectListbox = xCrossRef.getChild("format")
        select_pos(xSelectListbox, "3")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text
        self.ui_test.close_doc()

    def test_insert_field_document_time(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("insertField.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        #Time
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "9")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "0")
        xOffset = xCrossRef.getChild("offset")
        xOffset.executeAction("UP", tuple())  # offset time
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field Time
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        self.ui_test.close_doc()

    def test_insert_field_document_functions_conditional_text(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #Conditional text
        #fldfuncpage.ui
        #https://help.libreoffice.org/Writer/Defining_Conditions
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "0")  # conditional text

        xRefTxt = xCrossRef.getChild("condFunction")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"1 EQ 1"}))
        xcond1Txt = xCrossRef.getChild("cond1")
        xcond1Txt.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
        xcond2Txt = xCrossRef.getChild("cond2")
        xcond2Txt.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "1")   # assert the field's text Conditional text
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "0")  # conditional text

        xRefTxt = xCrossRef.getChild("condFunction")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"1 EQ 2"}))
        xcond1Txt = xCrossRef.getChild("cond1")
        xcond1Txt.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
        xcond2Txt = xCrossRef.getChild("cond2")
        xcond2Txt.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))

        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "2")   # assert the field's text Conditional text
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        self.ui_test.close_doc()

    def test_insert_field_document_functions_input_list(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #input list
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "1")  #  input list
        xItemTxt = xCrossRef.getChild("item")
        xItemTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"first"}))
        xAddItemBtn = xCrossRef.getChild("add")
        xAddItemBtn.executeAction("CLICK", tuple())   # add  item
        xItemTxt.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xItemTxt.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xItemTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"second"}))
        xAddItemBtn = xCrossRef.getChild("add")
        xAddItemBtn.executeAction("CLICK", tuple())   # add  item
        xNameTxt = xCrossRef.getChild("listname")
        xNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"listname"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:5], "first")   # assert the field's text input list
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        self.ui_test.close_doc()
    def test_insert_field_document_functions_input_field(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #input field
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "2")  #  input field
        xRefTxt = xCrossRef.getChild("value")

        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"reference"}))
        xOKBtn = xCrossRef.getChild("ok")
        def handle_ok_dlg(dialog):
            xname = dialog.getChild("name")
            xText = dialog.getChild("text")
            xsubDOKBtn = dialog.getChild("ok")
            self.assertEqual(get_state_as_dict(xname)["Text"], "reference")
            xText.executeAction("TYPE", mkPropertyValues({"TEXT":"reftext"}))
            self.ui_test.close_dialog_through_button(xsubDOKBtn)
        self.ui_test.execute_blocking_action(xOKBtn.executeAction, args=('CLICK', ()),
            dialog_handler=handle_ok_dlg)

        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:7], "reftext")   # assert the field's text input field
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        self.ui_test.close_doc()

    def test_insert_field_document_functions_placeholder(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #placehoder
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")  #  placehoder
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "0")  # format Text
        xNameTxt = xCrossRef.getChild("condFunction")
        xNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"AA"}))
        xRefTxt = xCrossRef.getChild("value")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"reference"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:4], "<AA>")   # assert the field's text Placeholder
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        #placehoder->table
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")  #  placehoder
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "1")  # format Table
        xNameTxt = xCrossRef.getChild("condFunction")
        xNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"AA"}))
        xRefTxt = xCrossRef.getChild("value")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"reference"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:4], "<AA>")   # assert the field's text Placeholder
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        #placehoder->frame
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")  #  placehoder
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "2")  # format
        xNameTxt = xCrossRef.getChild("condFunction")
        xNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"AA"}))
        xRefTxt = xCrossRef.getChild("value")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"reference"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:4], "<AA>")   # assert the field's text Placeholder
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        #placehoder->image
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")  #  placehoder
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "3")  # format
        xNameTxt = xCrossRef.getChild("condFunction")
        xNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"AA"}))
        xRefTxt = xCrossRef.getChild("value")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"reference"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:4], "<AA>")   # assert the field's text Placeholder
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        #placehoder->object
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")  #  placehoder
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "4")  # format Object
        xNameTxt = xCrossRef.getChild("condFunction")
        xNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"AA"}))
        xRefTxt = xCrossRef.getChild("value")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"reference"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:4], "<AA>")   # assert the field's text Placeholder
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        self.ui_test.close_doc()

    def test_insert_field_document_functions_combine_characters(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        #combine characters
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "5")  #  combine characters
        xRefTxt = xCrossRef.getChild("name")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"LO"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        self.ui_test.close_doc()

    def test_insert_field_document_functions_hidden_text(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #hidden text
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")  #  hidden text
        xRefTxt = xCrossRef.getChild("condFunction")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"1 EQ 1"}))
        xvalTxt = xCrossRef.getChild("value")
        xvalTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"ABC"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        #hidden text
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")  #  hidden text
        xRefTxt = xCrossRef.getChild("condFunction")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"1 EQ 2"}))
        xvalTxt = xCrossRef.getChild("value")
        xvalTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"ABC"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:3], "ABC")   # text is not hidden, because condition is false
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        self.ui_test.close_doc()

    def test_insert_field_document_functions_hidden_paragraph(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #hidden paragraph
        type_text(xWriterEdit, "Test for hidden paragraph")
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "7")  #  hidden paragraph
        xRefTxt = xCrossRef.getChild("condFunction")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"1 EQ 1"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
