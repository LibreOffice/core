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

class tdf132026(UITestCase):

    def test_tdf132026(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf132026.ods")) as calc_doc:
            MainWindow = self.xUITest.getTopFocusWindow()
            grid_window = MainWindow.getChild("grid_window")

            chars=["=","+","-"]
            directions=["UP","DOWN","LEFT","RIGHT"]

            # format general, should select cell
            for c in chars:
                sign=-1 if c=="-" else 1
                for i,direction in enumerate(directions):
                    grid_window.executeAction("SELECT", mkPropertyValues({"CELL": "B2"}))
                    grid_window.executeAction("TYPE", mkPropertyValues({"TEXT": c}))
                    grid_window.executeAction("TYPE", mkPropertyValues({"KEYCODE": direction}))
                    grid_window.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
                    self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getString(), str(sign*(i+1)))

            # format text, shouldn't select cell
            for c in chars:
                for direction in directions:
                    grid_window.executeAction("SELECT", mkPropertyValues({"CELL": "E2"}))
                    grid_window.executeAction("TYPE", mkPropertyValues({"TEXT": c}))
                    grid_window.executeAction("TYPE", mkPropertyValues({"KEYCODE": direction}))
                    grid_window.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
                    self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 1).getString(), c)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
