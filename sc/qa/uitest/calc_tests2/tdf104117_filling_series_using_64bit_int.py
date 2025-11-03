# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_by_text
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 104117 - Crash in Calc when selecting "Uniform Integer" with an max value "100000000000000" in "Random Number Generator
class tdf104117(UITestCase):
    def test_tdf104117(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:RandomNumberGeneratorDialog") as xDialog:
                xcellrangeedit = xDialog.getChild("cell-range-edit")
                xdistributioncombo = xDialog.getChild("distribution-combo")
                xparameter2spin = xDialog.getChild("parameter2-spin")

                xcellrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$A$1:$A$2"}))
                select_by_text(xdistributioncombo, "Uniform Integer")
                xparameter2spin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xparameter2spin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xparameter2spin.executeAction("TYPE", mkPropertyValues({"TEXT":"1000000000000000000000000000000000000000000000"}))

            #Verify
            self.assertEqual(bool(get_cell_by_position(document, 0, 0, 0).getString() ), True)
            self.assertEqual(bool(get_cell_by_position(document, 0, 0, 1).getString() ), True)
            #undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(bool(get_cell_by_position(document, 0, 0, 0).getString() ), False)
            self.assertEqual(bool(get_cell_by_position(document, 0, 0, 1).getString() ), False)
# vim: set shiftwidth=4 softtabstop=4 expandtab:
