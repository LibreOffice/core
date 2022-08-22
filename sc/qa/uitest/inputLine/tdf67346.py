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
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 67346 - EDITING: Undo broken when pasting text that has been copied from the input line

class tdf67346(UITestCase):

    def test_tdf67346_undo_paste_text_input_line(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "Apple")

            # Move focus to input window and copy the text from there
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+SHIFT+F2"}))

            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()
            xInputWin = xCalcDoc.getChild("sc_input_window")
            self.assertEqual('true', get_state_as_dict(xInputWin)["HasFocus"])

            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:Copy")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))

            self.xUITest.executeCommand(".uno:Paste")
            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Apple")
            self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "Apple")
            self.assertEqual(get_state_as_dict(xInputWin)["Text"], "Apple")

            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Apple")
            self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "")
            self.assertEqual(get_state_as_dict(xInputWin)["Text"], "")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
