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
from libreoffice.calc.document import get_cell_by_position
from libreoffice.calc.csv_dialog import load_csv_file

class Tdf142395(UITestCase):

    def test_tdf142395(self):
        with load_csv_file(self, "tdf142395.csv", True) as xDialog:
            # Remove the text delimiter
            xTextDelimiter = xDialog.getChild("textdelimiter")
            xTextDelimiter.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
            xTextDelimiter.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))

        document = self.ui_test.get_component()

        self.assertEqual("a", get_cell_by_position(document, 0, 0, 0).getString())
        self.assertEqual(" b", get_cell_by_position(document, 0, 1, 0).getString())
        self.assertEqual(" c", get_cell_by_position(document, 0, 2, 0).getString())

        # Without the fix in place, this test would have failed with
        # AssertionError: ' ' != '晦餪'
        self.assertEqual(" ", get_cell_by_position(document, 0, 3, 0).getString())

        self.assertEqual("e", get_cell_by_position(document, 0, 0, 1).getString())
        self.assertEqual(" f", get_cell_by_position(document, 0, 1, 1).getString())
        self.assertEqual(" g", get_cell_by_position(document, 0, 2, 1).getString())
        self.assertEqual(" ", get_cell_by_position(document, 0, 3, 1).getString())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
