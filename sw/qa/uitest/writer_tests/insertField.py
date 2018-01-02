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

    def test_insert_field(self):

        writer_doc = self.ui_test.load_file(get_url_for_data_file("insertField.odt"))
        document = self.ui_test.get_component()

#fill the Options-> User data

        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")
        xProperties = self.xUITest.getTopFocusWindow()
#('OptUserPage', 'OptionsDialog', 'alignment1', 'alignment2', 'apartnum', 'box', 'box2', 'cancel', 'city', 'cityft', 'company', 'companyft', 'country', 'countryft', 'cryptography', 'cryptographylabel', 'dialog-action_area1', 'dialog-vbox1', 'eastfirstname', 'eastlastname', 'eastnameft', 'eastshortname', 'email', 'encryptionkey', 'encryptionkeylabel', 'encrypttoself', 'fax', 'faxft', 'firstname', 'frame1', 'grid1', 'grid10', 'grid11', 'grid12', 'grid13', 'grid14', 'grid2', 'grid3', 'grid4', 'grid5', 'grid6', 'grid7', 'grid8', 'grid9', 'help', 'home', 'icity', 'icityft', 'izip', 'label1', 'lastname', 'nameft', 'ok', 'pages', 'phoneft', 'position', 'revert', 'rusfathersname', 'rusfirstname', 'ruslastname', 'rusnameft', 'russhortname', 'russtreet', 'russtreetft', 'shortname', 'signingkey', 'signingkeylabel', 'state', 'street', 'streetft', 'title', 'titleft', 'usefordocprop', 'work', 'zip')

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
        xOkBtn.executeAction("CLICK", tuple())   # save the data in properties dialog

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
        xCancBtn.executeAction("CLICK", tuple())   # close the dialog
#        sleep(1)  # need a sleep here??
          # Cross reference: .uno:InsertReferenceField
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
#('FieldDbPage', 'FieldDialog', 'FieldDocInfoPage', 'FieldDocumentPage', 'FieldFuncPage', 'FieldRefPage', 'FieldVarPage', 'add', 'alignment1', 'alignment2', 'alignment3', 'alignment4', 'box1', 'box2', 'box3', 'box4', 'box5', 'browse', 'browseft', 'buttonbox1', 'cancel', 'chapterframe', 'cond1', 'cond1ft', 'cond2', 'cond2ft', 'condgroup', 'condition', 'daysft', 'dialog-action_area1', 'dialog-vbox1', 'down', 'filter', 'fixed', 'format', 'formatframe', 'fromdatabasecb', 'grid1', 'grid2', 'grid3', 'grid4', 'grid5', 'grid6', 'grid7', 'grid8', 'grid9', 'help', 'invisible', 'item', 'itemft', 'label1', 'label2', 'label3', 'label4', 'label5', 'level', 'levelft', 'listgroup', 'listitemft', 'listitems', 'listname', 'listnameft', 'macro', 'minutesft', 'name', 'nameft', 'numformat', 'offset', 'ok', 'recgroup', 'recnumber', 'remove', 'reset', 'select', 'selectframe', 'selecttip', 'separator', 'separatorft', 'standard', 'tabcontrol', 'toolbar', 'type', 'typeframe', 'up', 'userdefinedcb', 'value', 'valueft', 'valuegroup')

#        print(xCrossRef.getChildren())

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog

#Author ->Name
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
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
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
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


#Date
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
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

#File name ->File Name
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
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
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
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
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
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
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
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

#Chapter
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
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

#アｱaAaA1
#Page number -> 1,2,3
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "0")
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "0")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "ア")   # assert the field's text  Page number -> 1,2,3 -> ア
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Page number -> 1,2,3 ｱ
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "0")
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "1")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "ｱ")   # assert the field's text  Page number -> 1,2,3 -> ｱ
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Page number -> a..aa..aaa
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "0")
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "2")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "a")   # assert the field's text  Page number -> a..aa..aaa
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Page number -> A..AA..AAA
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "0")
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "3")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "A")   # assert the field's text  Page number -> A..AA..AAA
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Page number -> a b c
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "0")
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "4")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "a")   # assert the field's text  Page number -> a b c
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Page number -> A B C
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "0")
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "5")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "A")   # assert the field's text  Page number -> A B C
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Page number -> Arabic 1,2,3
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "0")
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "6")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "1")   # assert the field's text  Page number -> Arabic 1,2,3
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Page number -> Roman
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "0")
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "9")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "i")   # assert the field's text  Page number -> Roman i
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> Company
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "O")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:18], "Documentfoundation")   # assert the field's text Sender -> Company
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> firstname
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "1")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:5], "Libre")   # assert the field's text Sender -> firstname
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> lastname
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "2")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:6], "Office")   # assert the field's text Sender -> lastname
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> initials
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "3")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:2], "LO")   # assert the field's text Sender -> initials
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> street
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "4")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:6], "Street")   # assert the field's text Sender -> street
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> Country
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "5")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:7], "Country")   # assert the field's text Sender -> Country
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> Zip
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "6")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:6], "000000")   # assert the field's text Sender -> Zip
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> City
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "7")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:4], "City")   # assert the field's text Sender -> City
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> Title
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "8")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:10], "Title_prop")   # assert the field's text Sender -> Title
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> position
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "9")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:8], "position")   # assert the field's text Sender -> position
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> home tel
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "10")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:4], "0001")   # assert the field's text Sender -> home tel
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text


#Sender -> home work
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "11")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:4], "0002")   # assert the field's text Sender -> home work
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> fax
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "12")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:4], "0003")   # assert the field's text Sender -> fax
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> email
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "13")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:5], "email")   # assert the field's text Sender -> email
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Sender -> state
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "14")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:5], "State")   # assert the field's text Sender -> state
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Statistics -> Pages
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "7")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "0")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "1")   # assert the field's text Sender -> Pages
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Statistics -> Paragraphs
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "7")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "1")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "0")   # assert the field's text Sender -> Paragraphs
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Statistics -> Words
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "7")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "2")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "0")   # assert the field's text Sender -> Words
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Statistics -> Characters
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "7")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "3")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "0")   # assert the field's text Sender -> Characters
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Statistics -> Tables
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "7")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "4")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "0")   # assert the field's text Sender -> Tables
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Statistics -> Images
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "7")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "5")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "0")   # assert the field's text Sender -> Images
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Statistics -> Objects
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "7")
        xSelectListbox = xCrossRef.getChild("select")
        select_pos(xSelectListbox, "6")
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "0")   # assert the field's text Sender -> Objects
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Time
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
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

#Conditional text
#fldfuncpage.ui
#https://help.libreoffice.org/Writer/Defining_Conditions
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "0")  # conditional text
        sleep(1)
        xRefTxt = xCrossRef.getChild("name")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"1 EQ 1"}))  # don't know which textBox is Condition 
        xcond1Txt = xCrossRef.getChild("cond1")
        xcond1Txt.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
        xcond2Txt = xCrossRef.getChild("cond2")
        xcond2Txt.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))
#        sleep(7)
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:1], "2")   # assert the field's text Conditional text
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#input list
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
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

#input field /  GtkTextView not supported + inputfielddialog.ui: it doesnt take this top focus dialog
#        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
#        xCrossRef = self.xUITest.getTopFocusWindow()

#        xTabs = xCrossRef.getChild("tabcontrol")
#        select_pos(xTabs, "2")  # tab Functions
#        xTypeListbox = xCrossRef.getChild("type")
#        select_pos(xTypeListbox, "2")  #  input field
#        xRefTxt = xCrossRef.getChild("value")
#        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"reference"}))
#        xAddBtn = xCrossRef.getChild("ok")
#        xAddBtn.executeAction("CLICK", tuple())   # insert the field
#        xRefDlg = self.xUITest.getTopFocusWindow()    #inputfielddialog.ui; it doesnt take this top focus dialog
#        print(xRefDlg.getChildren())
#        xRefDTxt = xRefDlg.getChild("text")
#        xRefDTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"reference"}))
#        xOKBtn = xCrossRef.getChild("ok")
#        xOKBtn.executeAction("CLICK", tuple())   # 

#        xCrossRef = self.xUITest.getTopFocusWindow()
#        xCloseBtn = xCrossRef.getChild("cancel")
#        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
#        self.assertEqual(document.Text.String[0:9], "reference")   # assert the field's text input field
#        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
#        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#placehoder
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "4")  #  placehoder
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "0")  # format Text
        xNameTxt = xCrossRef.getChild("name")
        xNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"aa"}))
        xRefTxt = xCrossRef.getChild("value")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"reference"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        self.assertEqual(document.Text.String[0:4], "<aa>")   # assert the field's text Placeholder
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#combine characters
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
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


#hidden text
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "6")  #  hidden text
        xRefTxt = xCrossRef.getChild("name")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"1 EQ 1"}))
        xvalTxt = xCrossRef.getChild("value")
        xvalTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"ABC"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#hidden paragraph
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "2")  # tab Functions
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "7")  #  hidden paragraph
        xRefTxt = xCrossRef.getChild("name")
        xRefTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"1 EQ 1"}))
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#DocInformation  flddocinfopage.ui - select_pos doesnt work
#comments
        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()

        xTabs = xCrossRef.getChild("tabcontrol")
        select_pos(xTabs, "3")  # tab DocInformation
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "0")  #  Comments
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Author created - select_pos doesnt work
#        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
#        xCrossRef = self.xUITest.getTopFocusWindow()

#        xTabs = xCrossRef.getChild("tabcontrol")
#        select_pos(xTabs, "3")  # tab DocInformation
#        xTypeListbox = xCrossRef.getChild("type")
##        xTypeListbox = xTypeListbox.getChild('1')
##        xTypeListbox.executeAction("EXPAND", tuple())   # try treeview

#        select_pos(xTypeListbox, "1")  #  Author created
#        xFormatListbox = xCrossRef.getChild("format")
#        select_pos(xFormatListbox, "0")  # format Text
#        sleep(5)
#        xAddBtn = xCrossRef.getChild("ok")
#        xAddBtn.executeAction("CLICK", tuple())   # insert the field
#        xCloseBtn = xCrossRef.getChild("cancel")
#        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
#        self.assertEqual(document.Text.String[0:5], "<AA BB>")   # assert the field's text Author created
#        sleep(5)
#        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
#        self.xUITest.executeCommand(".uno:Cut")   #cut  text

#Variables fldvarpage.ui
#tdf112526  Set variable : Name = var, Value = some text, Format = Text

#        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
#        xCrossRef = self.xUITest.getTopFocusWindow()
#        xTabs = xCrossRef.getChild("tabcontrol")
#        select_pos(xTabs, "4")  # tab Variables
#        xTypeListbox = xCrossRef.getChild("type")
#        select_pos(xTypeListbox, "0")  # set variable
##        xFormatListbox = xCrossRef.getChild("numformat")  #swlo-NumFormatListBox not supported?
##        select_pos(xFormatListbox, "0")  # format Text

#        xFormatListbox = xCrossRef.getChild("numformat")
#        props = {"TEXT": "Text"}
#        actionProps = mkPropertyValues(props)
#        xFormatListbox.executeAction("SELECT", actionProps)


#        xNameTxt = xCrossRef.getChild("name")
#        xNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"var"}))
#        xValueTxt = xCrossRef.getChild("value")
#        xValueTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"some text"}))
#        sleep(5)
#        xAddBtn = xCrossRef.getChild("ok")
#        xAddBtn.executeAction("CLICK", tuple())   # insert the field
#        xCloseBtn = xCrossRef.getChild("cancel")
#        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
#        self.assertEqual(document.Text.String[0:9], "some text")   # assert the field's text tdf112526  Set variable 
#        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
#        self.xUITest.executeCommand(".uno:Cut")   #cut  text
#  Set variable-Standard
#        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
#        xCrossRef = self.xUITest.getTopFocusWindow()
#        xTabs = xCrossRef.getChild("tabcontrol")
#        select_pos(xTabs, "4")  # tab Variables
#        xTypeListbox = xCrossRef.getChild("type")
#        select_pos(xTypeListbox, "0")  # set variable
#        xFormatListbox = xCrossRef.getChild("format")
#        select_pos(xFormatListbox, "0")  # format Standard
#        xNameTxt = xCrossRef.getChild("name")
#        xNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"var2"}))
#        xValueTxt = xCrossRef.getChild("value")
#        xValueTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"some text"}))
#        sleep(5)
#        xAddBtn = xCrossRef.getChild("ok")
#        xAddBtn.executeAction("CLICK", tuple())   # insert the field
#        xCloseBtn = xCrossRef.getChild("cancel")
#        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
#        self.assertEqual(document.Text.String[0:9], "some text")   # assert the field's text   Set variable Standard
#        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
#        self.xUITest.executeCommand(".uno:Cut")   #cut  text
##  Set variable - numeric
#        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField")
#        xCrossRef = self.xUITest.getTopFocusWindow()
#        xTabs = xCrossRef.getChild("tabcontrol")
#        select_pos(xTabs, "4")  # tab Variables
#        xTypeListbox = xCrossRef.getChild("type")
#        select_pos(xTypeListbox, "0")  # set variable
#        xFormatListbox = xCrossRef.getChild("format")
#        select_pos(xFormatListbox, "0")  # format -12345
#        xNameTxt = xCrossRef.getChild("name")
#        xNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"var3"}))
#        xValueTxt = xCrossRef.getChild("value")
#        xValueTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
#        sleep(5)
#        xAddBtn = xCrossRef.getChild("ok")
#        xAddBtn.executeAction("CLICK", tuple())   # insert the field
#        xCloseBtn = xCrossRef.getChild("cancel")
#        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
#        self.assertEqual(document.Text.String[0:1], "1")   # assert the field's text  Set variable numeric
#        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
#        self.xUITest.executeCommand(".uno:Cut")   #cut  text


        self.ui_test.close_doc()

