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
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

# specialcharacters.ui
class specialCharacter(UITestCase):

    def test_tdf56363(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()

            # Insert a font including a font feature into the font name combobox
            xFontName = xWriterDoc.getChild("fontnamecombobox")
            fontName = get_state_as_dict(xFontName)["Text"]
            xFontName.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
            xFontName.executeAction("TYPE", mkPropertyValues({"TEXT": fontName + ":smcp"}))
            xFontName.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

            # Open special character dialog and check selected font name
            with self.ui_test.execute_dialog_through_command(".uno:InsertSymbol", close_button="cancel") as xDialog:
                xComboFont = xDialog.getChild("fontlb")
                # Without the fix in place, no font would be selected
                self.assertEqual(get_state_as_dict(xComboFont)["Text"], fontName)

    def test_special_character(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:InsertSymbol", close_button="cancel") as xDialog:
                xCharSet = xDialog.getChild("showcharset")  # default charset

                xCharSet.executeAction("SELECT", mkPropertyValues({"COLUMN": "1", "ROW": "4"}))  # digit 4 selected

                xHexText = xDialog.getChild("hexvalue")
                xDecText = xDialog.getChild("decimalvalue")

                self.assertEqual(get_state_as_dict(xHexText)["Text"], "34")  # check the values Hex and decimal
                self.assertEqual(get_state_as_dict(xDecText)["Text"], "52")


            with self.ui_test.execute_dialog_through_command(".uno:InsertSymbol", close_button="cancel") as xDialog:

                xComboFont = xDialog.getChild("fontlb")
                select_pos(xComboFont, "0")  # select font
                xComboFont2 = xDialog.getChild("subsetlb")
                select_pos(xComboFont2, "0")  # select font subset

                xSearchText = xDialog.getChild("search")  # test search textBox
                xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT": "d"}))
                xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT": "i"}))
                xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT": "g"}))
                xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT": "i"}))
                xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT": "t"}))
                xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT": " "}))
                xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT": "f"}))
                xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT": "o"}))
                xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT": "u"}))
                xSearchText.executeAction("TYPE", mkPropertyValues({"TEXT": "r"}))

                # works locally and linux_gcc_release_64, but fails at linux_clang_dbgutil_64.
                # Markus: Actually after a round of debugging I think the problem is actually that the test depends on the used font.
                # Therefore, if the font is not available or not selected by default the test fails.
                #        xCharSet = xDialog.getChild("searchcharset")    #another charset -> search charset
                #        xCharSet.executeAction("SELECT", mkPropertyValues({"COLUMN": "0", "ROW": "0"}))   #digit 4 selected, we have only one result;
                #        sleep(1)                                                  #try sleep here
                #        xCharSet.executeAction("SELECT", mkPropertyValues({"COLUMN": "0", "ROW": "0"}))   #try it twice, because it works at local,but fail on gerrit
                ##gerrit:self.assertEqual(get_state_as_dict(xHexText)["Text"], "34")    # check the values for digit 4; AssertionError: '1' != '34'

                #        xHexText = xDialog.getChild("hexvalue")
                #        xDecText = xDialog.getChild("decimalvalue")
                #        self.assertEqual(get_state_as_dict(xHexText)["Text"], "34")    # check the values for digit 4
                #        self.assertEqual(get_state_as_dict(xDecText)["Text"], "52")

                #        xAddFavBtn = xDialog.getChild("favbtn")
                #        xAddFavBtn.executeAction("CLICK", tuple())     # Add to favorites button

                #        xInsrBtn = xDialog.getChild("insert")
                #        xInsrBtn.executeAction("CLICK", tuple())     # Insert to document

                #        self.assertEqual(document.Text.String[0:1], "4")    # check inserted character

                #        self.xUITest.executeCommand(".uno:Undo")
                #        self.xUITest.executeCommand(".uno:Redo")   #undo, redo

                #        self.assertEqual(document.Text.String[0:1], "4")    # check inserted character after undo, redo


# vim: set shiftwidth=4 softtabstop=4 expandtab:
