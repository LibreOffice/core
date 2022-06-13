# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.calc.paste_special import reset_default_values

class tdf69450(UITestCase):

    def test_tdf69450(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            #add text to A1 and B1
            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "B1", "B")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            self.xUITest.executeCommand(".uno:Copy")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
            with self.ui_test.execute_dialog_through_command(".uno:PasteSpecial") as xDialog:
                reset_default_values(self, xDialog)

                xtext = xDialog.getChild("text")
                xnumbers = xDialog.getChild("numbers")
                xdatetime = xDialog.getChild("datetime")
                xformats = xDialog.getChild("formats")

                xtext.executeAction("CLICK", tuple())
                xnumbers.executeAction("CLICK", tuple())
                xdatetime.executeAction("CLICK", tuple())
                xformats.executeAction("CLICK", tuple())

            #check B1 text
            self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "B")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
            self.xUITest.executeCommand(".uno:Bold")
            self.xUITest.executeCommand(".uno:Copy")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
            with self.ui_test.execute_dialog_through_command(".uno:PasteSpecial"):
                pass

            #check B1 text
            self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "B")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
