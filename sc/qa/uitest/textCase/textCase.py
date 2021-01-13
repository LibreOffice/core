# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class textCase(UITestCase):
    def test_text_case_switch(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #enter data
        enter_text_to_cell(gridwin, "A1", "hello world")
        enter_text_to_cell(gridwin, "A2", "libre office")
        enter_text_to_cell(gridwin, "A4", "free suite")
        #select
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))
        #Capitalize every word - CommandSent Name:.uno:ChangeCaseToTitleCase
        self.xUITest.executeCommand(".uno:ChangeCaseToTitleCase")
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Hello World")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "Libre Office")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "Free Suite")

        #Sentence case - CommandSent Name:.uno:ChangeCaseToSentenceCase
        self.xUITest.executeCommand(".uno:ChangeCaseToSentenceCase")

        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Hello world")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "Libre office")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "Free suite")

        #toggle case - CommandSent Name:.uno:ChangeCaseToToggleCase
        self.xUITest.executeCommand(".uno:ChangeCaseToToggleCase")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "hELLO WORLD")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "lIBRE OFFICE")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "fREE SUITE")

        #uppercase = CommandSent Name:.uno:ChangeCaseToUpper
        self.xUITest.executeCommand(".uno:ChangeCaseToUpper")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "HELLO WORLD")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "LIBRE OFFICE")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "FREE SUITE")

        #lowercase - CommandSent Name:.uno:ChangeCaseToLower
        self.xUITest.executeCommand(".uno:ChangeCaseToLower")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "hello world")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "libre office")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "free suite")

        #cycle case = CommandSent Name:.uno:ChangeCaseRotateCase
        self.xUITest.executeCommand(".uno:ChangeCaseRotateCase")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Hello World")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "Libre Office")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "Free Suite")

        #select non continuous range
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B1:B2"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A4:A5", "EXTEND":"1"}))

        #lowercase - CommandSent Name:.uno:ChangeCaseToLower
        self.xUITest.executeCommand(".uno:ChangeCaseToLower")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "hello world")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "libre office")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "free suite")

        #Capitalize every word - CommandSent Name:.uno:ChangeCaseToTitleCase
        self.xUITest.executeCommand(".uno:ChangeCaseToTitleCase")
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Hello World")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "Libre Office")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "Free Suite")

        #Sentence case - CommandSent Name:.uno:ChangeCaseToSentenceCase
        self.xUITest.executeCommand(".uno:ChangeCaseToSentenceCase")

        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Hello world")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "Libre office")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "Free suite")

        #toggle case - CommandSent Name:.uno:ChangeCaseToToggleCase
        self.xUITest.executeCommand(".uno:ChangeCaseToToggleCase")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "hELLO WORLD")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "lIBRE OFFICE")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "fREE SUITE")

        #uppercase = CommandSent Name:.uno:ChangeCaseToUpper
        self.xUITest.executeCommand(".uno:ChangeCaseToUpper")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "HELLO WORLD")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "LIBRE OFFICE")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "FREE SUITE")

        #lowercase - CommandSent Name:.uno:ChangeCaseToLower
        self.xUITest.executeCommand(".uno:ChangeCaseToLower")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "hello world")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "libre office")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "free suite")

        #cycle case = CommandSent Name:.uno:ChangeCaseRotateCase
        self.xUITest.executeCommand(".uno:ChangeCaseRotateCase")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Hello world")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "Libre office")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "Free suite")
        self.ui_test.close_doc()

    def test_tdf119155_Capitalize_Every_Word(self):
        #Bug 119155 - Freeze after command format->text->Capitalize Every Word
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf119155.xlsx"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #1. Open attached file
        #2. Select cells from C2 to C14
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C2:C14"}))
        #3. Go to menu: Format->Text->Capitalize Every Word
        self.xUITest.executeCommand(".uno:ChangeCaseToTitleCase")
        #Actual Results:Freezes LibreOffice
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getString(), "Collagene Expert Targeted Wrinkle Corrector Unboxed 10 Ml")
        self.assertEqual(get_cell_by_position(document, 0, 2, 13).getString(), "Vitamina Suractivee Hand Cream 400 Ml")

        self.ui_test.close_doc()

    def test_tdf119162_Cycle_Case(self):
        #Bug 119162 - Format > Text > Cycle Case on attached example file hangs Calc reproducibly
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf119162.xls"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #1.Open the attached .xls file
        #2. Select column A
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #3. Select [Format] > [Text] > [Cycle Case]
        self.xUITest.executeCommand(".uno:ChangeCaseRotateCase")
        self.xUITest.executeCommand(".uno:ChangeCaseToLower")
        #Actual Results:Freezes LibreOffice
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "test\n")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
