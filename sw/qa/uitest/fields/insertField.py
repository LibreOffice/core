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
from uitest.framework import UITestCase
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
        sleep(1)
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
        self.xUITest.executeCommand(".uno:Cut")   #cut text

        #Author ->Initials
#        self.xUITest.executeCommand(".uno:InsertField")
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "0")
        sleep(1)
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "1")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:2], "LO")   # assert the field's text Author ->Initials
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut text

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
