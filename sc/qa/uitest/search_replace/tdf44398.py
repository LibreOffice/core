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

#Bug 44398 - : Find, replace, regular expression bug

class tdf44398(UITestCase):
   def test_tdf44398_find_replace_regexp(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        # 1. A1 => 123456
        enter_text_to_cell(gridwin, "A1", "123456")
        # 2. ctrl-h, in dialog
        # Search: ([0-9])
        # Replace: $1
        # check regular expression
        # hit replace all

        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        searchterm = xDialog.getChild("searchterm")
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"([0-9])"}))
        replaceterm = xDialog.getChild("replaceterm")
        replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"$1"})) #replace textbox
        regexp = xDialog.getChild("regexp")
        regexp.executeAction("CLICK", tuple())   #regular expressions
        replaceall = xDialog.getChild("replaceall")
        replaceall.executeAction("CLICK", tuple())
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        #verify 3. A1 => 123456
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "123456")

        self.ui_test.close_doc()

   def test_tdf44398_find_replace_regexp_string(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        # 1. A1 => VarNumberA
        enter_text_to_cell(gridwin, "A1", "VarNumberA")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RIGHT"}))
        # 2. ctrl-h, in dialog
        # Search: ([A-Z])
        # Replace: <space>$1
        # check regular expression
        # check case
        # hit replace all

        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        searchterm = xDialog.getChild("searchterm")
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"([A-Z])"}))
        replaceterm = xDialog.getChild("replaceterm")
        replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":" $1"})) #replace textbox
        regexp = xDialog.getChild("regexp")
        if (get_state_as_dict(regexp)["Selected"]) == "false":
            regexp.executeAction("CLICK", tuple())   #regular expressions
        matchcase = xDialog.getChild("matchcase")
        matchcase.executeAction("CLICK", tuple())   #case

        replaceall = xDialog.getChild("replaceall")
        replaceall.executeAction("CLICK", tuple())

        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        #verify A1 => ' Var Number A'
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), " Var Number A")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
