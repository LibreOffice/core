# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
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

class tdf100517(UITestCase):

    def execute_sort_dialog(self, gridwin, bIncludeNotes):
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B3"}))

        with self.ui_test.execute_dialog_through_command(".uno:DataSort") as xDialog:
            xTabs = xDialog.getChild("tabcontrol")
            select_pos(xTabs, "1")

            xIncludeNotes = xDialog.getChild("includenotes")

            if (get_state_as_dict(xIncludeNotes)["Selected"]) != bIncludeNotes:
                xIncludeNotes.executeAction("CLICK", tuple())


    def test_tdf100517(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "Text 2")
            enter_text_to_cell(gridwin, "A2", "Text 3")
            enter_text_to_cell(gridwin, "A3", "Text 1")

            for i in ['B1', 'B2', 'B3']:
                gridwin.executeAction("SELECT", mkPropertyValues({"CELL": i}))
                xArgs = mkPropertyValues({"Text": i})

                self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

            self.execute_sort_dialog(gridwin, "true")

            self.assertEqual("Text 1", get_cell_by_position(document, 0, 0, 0).getString())
            self.assertEqual("Text 2", get_cell_by_position(document, 0, 0, 1).getString())
            self.assertEqual("Text 3", get_cell_by_position(document, 0, 0, 2).getString())

            self.assertEqual("B3", get_cell_by_position(document, 0, 1, 0).Annotation.String)
            self.assertEqual("B1", get_cell_by_position(document, 0, 1, 1).Annotation.String)
            self.assertEqual("B2", get_cell_by_position(document, 0, 1, 2).Annotation.String)

            self.xUITest.executeCommand(".uno:Undo")

            self.assertEqual("Text 2", get_cell_by_position(document, 0, 0, 0).getString())
            self.assertEqual("Text 3", get_cell_by_position(document, 0, 0, 1).getString())
            self.assertEqual("Text 1", get_cell_by_position(document, 0, 0, 2).getString())

            self.assertEqual("B1", get_cell_by_position(document, 0, 1, 0).Annotation.String)
            self.assertEqual("B2", get_cell_by_position(document, 0, 1, 1).Annotation.String)
            self.assertEqual("B3", get_cell_by_position(document, 0, 1, 2).Annotation.String)

            self.execute_sort_dialog(gridwin, "false")

            self.assertEqual("Text 1", get_cell_by_position(document, 0, 0, 0).getString())
            self.assertEqual("Text 2", get_cell_by_position(document, 0, 0, 1).getString())
            self.assertEqual("Text 3", get_cell_by_position(document, 0, 0, 2).getString())

            self.assertEqual("B1", get_cell_by_position(document, 0, 1, 0).Annotation.String)
            self.assertEqual("B2", get_cell_by_position(document, 0, 1, 1).Annotation.String)
            self.assertEqual("B3", get_cell_by_position(document, 0, 1, 2).Annotation.String)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
