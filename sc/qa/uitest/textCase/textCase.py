# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


class textCase(UITestCase):
    def test_text_case_switch(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

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

# vim: set shiftwidth=4 softtabstop=4 expandtab:
