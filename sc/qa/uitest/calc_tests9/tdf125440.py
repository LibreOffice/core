# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.common import type_text

class Tdf1254400(UITestCase):

   def test_tdf125440(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        # Disable AutoCorrect 'While typing' so the quotes are not replaced
        self.xUITest.executeCommand(".uno:OnlineAutoFormat")

        # In order to simulate copy&paste from external source, type the
        # string in Writer, copy it and paste it in Calc
        type_text(xWriterEdit, '"Row"')
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        type_text(xWriterEdit, '""')
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        type_text(xWriterEdit, '""')
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        type_text(xWriterEdit, '"50.000"')

        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Copy")

        # Enable it again
        self.xUITest.executeCommand(".uno:OnlineAutoFormat")

        self.ui_test.execute_dialog_through_command(".uno:CloseDoc")
        xDialog = self.xUITest.getTopFocusWindow()
        xDiscardBtn = xDialog.getChild("discard")
        self.ui_test.close_dialog_through_button(xDiscardBtn)

        calc_doc = self.ui_test.create_doc_in_start_center("calc")

        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

        self.xUITest.executeCommand(".uno:PasteUnformatted")

        # Without the fix in place, this test would have failed with
        # AssertionError: 'Row' != '"Row"'
        self.assertEqual("Row", get_cell_by_position(document, 0, 0, 0).getString())
        self.assertEqual("", get_cell_by_position(document, 0, 1, 0).getString())
        self.assertEqual("", get_cell_by_position(document, 0, 2, 0).getString())
        self.assertEqual("50", get_cell_by_position(document, 0, 3, 0).getString())

        self.ui_test.close_doc()
