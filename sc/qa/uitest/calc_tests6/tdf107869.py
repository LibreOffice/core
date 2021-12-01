# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf107869(UITestCase):
    def test_tdf107869_undo_redo_delete_row_with_comment(self):
        with self.ui_test.load_file(get_url_for_data_file("comments.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #- Delete the row with comment.
            #select row 1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            self.xUITest.executeCommand(".uno:SelectRow")
            #delete row 1
            self.xUITest.executeCommand(".uno:DeleteRows")
            #A1 should be "B"
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "B")
            #- Undo-redo deletion a few times.
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "A")
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "B")
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "A")
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "B")
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "A")
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "B")
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "A")
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "B")
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "A")
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "B")
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "A")
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "B")
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "A")

            #select row 1 and 2
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))
            self.xUITest.executeCommand(".uno:SelectRow")
            #delete row 1
            self.xUITest.executeCommand(".uno:DeleteRows")
            #A1 should be ""
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "")
            #- Undo-redo deletion a few times.
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "A")
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "")
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "A")
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "")
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "A")
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "")
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "A")
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "")
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "A")
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
