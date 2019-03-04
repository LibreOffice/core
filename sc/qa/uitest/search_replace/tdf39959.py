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
from uitest.debug import sleep
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 39959 - Find-and-replace doesn't search all tables anymore

class tdf39959(UITestCase):
   def test_tdf39959_find_replace_all_sheets(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf39959.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        # 1. Open a new document
        # 2. Enter "asdf" in A1
        # 3. Activate Sheet2
        # 4. Try Find-and-replace (Ctrl+Alt+F) to search for "asdf"
        # Whether the checkbox "in allen Tabellen suchen" is activated or not: LibO Calc never seems to find the text

        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        searchterm = xDialog.getChild("searchterm")
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"asdf"}))
        replaceterm = xDialog.getChild("replaceterm")
        replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"bbb"})) #replace textbox
        allsheets = xDialog.getChild("allsheets")
        allsheets.executeAction("CLICK", tuple())
        replaceall = xDialog.getChild("replaceall")
        replaceall.executeAction("CLICK", tuple())
        # print(xDialog.getChildren())
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        #verify Sheet2.A1 = "bbb"
        self.assertEqual(get_cell_by_position(document, 1, 0, 0).getString(), "bbb ")
        self.assertEqual(get_cell_by_position(document, 1, 0, 2).getString(), "abc")
        #Undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 1, 0, 0).getString(), "asdf ")
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
