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
from uitest.uihelper.keyboard import select_all
from uitest.debug import sleep
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 119954 - Using a second defined database range in formula expression switches to first range.

class tdf119954(UITestCase):
   def test_tdf119954_second_db_range(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf119954.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #* new document
        #* in A1 enter 1
        #* in C3 enter 2
        #* on A1 define a database range 'aaa' with $Sheet1.$A$1
        #* on C3 define a database range 'bbb' with $Sheet2.$C$3
        #* in any cell enter formula =bbb
        #  => result is 1 instead of 2
        #* place cell cursor on that formula cell again
        #  => see that the formula is =aaa instead of =bbb

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.ui_test.execute_modeless_dialog_through_command(".uno:DefineDBName")
        xDefineNameDlg = self.xUITest.getTopFocusWindow()

        xEntryBox = xDefineNameDlg.getChild("entry")
        type_text(xEntryBox, "aaa")
        add = xDefineNameDlg.getChild("add")
        assign = xDefineNameDlg.getChild("assign")
        add.executeAction("CLICK", tuple())
        xOkBtn = xDefineNameDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.ui_test.execute_modeless_dialog_through_command(".uno:DefineDBName")
        xDefineNameDlg = self.xUITest.getTopFocusWindow()
        xEntryBox = xDefineNameDlg.getChild("entry")
        assign = xDefineNameDlg.getChild("assign")
        select_all(xEntryBox)
        type_text(xEntryBox, "bbb")
        select_all(assign)
        type_text(assign, "$Sheet2.$C$3")
        add.executeAction("CLICK", tuple())

        xOkBtn = xDefineNameDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        enter_text_to_cell(gridwin, "B2", "=bbb")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 2)

        enter_text_to_cell(gridwin, "C2", "=aaa")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), 1)

        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getFormula(), "")
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getFormula(), "")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
