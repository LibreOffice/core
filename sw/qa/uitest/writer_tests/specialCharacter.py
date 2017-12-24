#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
import time
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep
#specialcharacters.ui
class specialCharacter(UITestCase):

    def test_special_character(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        document = self.ui_test.get_component()

        self.ui_test.execute_dialog_through_command(".uno:InsertSymbol")  #specialCharacter dialog
        xDialog = self.xUITest.getTopFocusWindow()
        xCharSet = xDialog.getChild("showcharset")      #default charset

        xCharSet.executeAction("SELECT", mkPropertyValues({"COLUMN": "1", "ROW": "4"}))   #digit 4 selected

        xHexText = xDialog.getChild("hexvalue")
        xDecText = xDialog.getChild("decimalvalue")

        self.assertEqual(get_state_as_dict(xHexText)["Text"], "34")    # check the values Hex and decimal
        self.assertEqual(get_state_as_dict(xDecText)["Text"], "52")

        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)


        self.ui_test.execute_dialog_through_command(".uno:InsertSymbol")
        xDialog = self.xUITest.getTopFocusWindow()

        xSearchText = xDialog.getChild("search")                             #test search textBox
        xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT":"d"}))
        xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT":"i"}))
        xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT":"g"}))
        xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT":"i"}))
        xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT":"t"}))
        xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT":" "}))
        xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT":"f"}))
        xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT":"o"}))
        xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT":"u"}))
        xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT":"r"}))

        xSearchText.executeAction("RETURN", tuple())
        sleep(2)                                                  #need a sleep here
        xCharSet = xDialog.getChild("searchcharset")    #another charset -> search charset
        xCharSet.executeAction("SELECT", mkPropertyValues({"COLUMN": "0", "ROW": "0"}))   #digit 4 selected, we have only one result;
        sleep(1)                                                  #try sleep here
        xCharSet.executeAction("SELECT", mkPropertyValues({"COLUMN": "0", "ROW": "0"}))   #try it twice, because it works at local,but fail on gerrit
#gerrit:self.assertEqual(get_state_as_dict(xHexText)["Text"], "34")    # check the values for digit 4; AssertionError: '1' != '34'

        xHexText = xDialog.getChild("hexvalue")
        xDecText = xDialog.getChild("decimalvalue")
        self.assertEqual(get_state_as_dict(xHexText)["Text"], "34")    # check the values for digit 4
        self.assertEqual(get_state_as_dict(xDecText)["Text"], "52")

        xAddFavBtn = xDialog.getChild("favbtn")
        xAddFavBtn.executeAction("CLICK", tuple())     # Add to favorites button

        xInsrBtn = xDialog.getChild("insert")
        xInsrBtn.executeAction("CLICK", tuple())     # Insert to document

        self.assertEqual(document.Text.String[0:1], "4")    # check inserted character

        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Redo")   #undo, redo

        self.assertEqual(document.Text.String[0:1], "4")    # check inserted character after undo, redo


        self.ui_test.close_doc()
