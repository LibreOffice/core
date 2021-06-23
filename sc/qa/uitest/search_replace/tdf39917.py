# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep

#Bug 39917 - EDITING Find/Replace modifies formula in R1C1 syntax to invalid lowercase

class tdf39917(UITestCase):
   def test_tdf39917_find_replace_R1C1(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #* Tools --> Options --> Calc --> Formula -->  Syntax = Excel R1C1
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xCalcEntry = xPages.getChild('3')                 # Calc
        xCalcEntry.executeAction("EXPAND", tuple())
        xCalcFormulaEntry = xCalcEntry.getChild('4')
        xCalcFormulaEntry.executeAction("SELECT", tuple())          #Formula

        formulasyntax = xDialogOpt.getChild("formulasyntax")
        #Excel R1C1
        select_by_text(formulasyntax, "Excel R1C1")

        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #1. Create a workbook with 3 sheets: Page1, Page2, Page3.
        # 2. Tools -> Options -> LibreOffice Calc -> Formula: Set syntax to Excel A1
        # 5. Fill fields:

        self.ui_test.execute_dialog_through_command(".uno:Insert")  #insert sheet
        xDialog = self.xUITest.getTopFocusWindow()
        after = xDialog.getChild("after")
        after.executeAction("CLICK", tuple())
        nameed = xDialog.getChild("nameed")
        nameed.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        nameed.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        nameed.executeAction("TYPE", mkPropertyValues({"TEXT":"Page2"}))
        xOKButton = xDialog.getChild("ok")
        xOKButton.executeAction("CLICK", tuple())
        self.ui_test.execute_dialog_through_command(".uno:Insert")  #insert sheet
        xDialog = self.xUITest.getTopFocusWindow()
        after = xDialog.getChild("after")
        after.executeAction("CLICK", tuple())
        nameed = xDialog.getChild("nameed")
        nameed.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        nameed.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        nameed.executeAction("TYPE", mkPropertyValues({"TEXT":"Page3"}))
        xOKButton = xDialog.getChild("ok")
        xOKButton.executeAction("CLICK", tuple())
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RIGHT"}))
        # 3. Type in (for example) B4:  =Page2!B4
        enter_text_to_cell(gridwin, "B4", "=Page2!RC")
        # 4. Edit -> Find and Replace
        # Find: Page2
        # Replace: Page3
        # 6. Press Replace all
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        searchterm = xDialog.getChild("searchterm")
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"Page2"}))
        replaceterm = xDialog.getChild("replaceterm")
        replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"Page3"})) #replace textbox
        replace = xDialog.getChild("replace")
        replace.executeAction("CLICK", tuple())
        replace.executeAction("CLICK", tuple())
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        #verify
        enter_text_to_cell(gridwin, "A1", "=FORMULA(R[3]C[1])")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "=Page3!RC")
        #Give it back Tools --> Options --> Calc --> Formula -->  Syntax = Calc A1
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xCalcEntry = xPages.getChild('3')                 # Calc
        xCalcEntry.executeAction("EXPAND", tuple())
        xCalcFormulaEntry = xCalcEntry.getChild('4')
        xCalcFormulaEntry.executeAction("SELECT", tuple())          #Formula

        formulasyntax = xDialogOpt.getChild("formulasyntax")
        #Excel R1C1
        select_by_text(formulasyntax, "Calc A1")

        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
