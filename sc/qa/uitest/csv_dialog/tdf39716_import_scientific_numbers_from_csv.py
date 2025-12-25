# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase

from libreoffice.calc.csv_dialog import load_csv_file
from libreoffice.calc.document import get_cell_by_position


class Tdf39716(UITestCase):

    def test_tdf39716(self):

        with load_csv_file(self, "tdf39716.csv", True):
            pass

        document = self.ui_test.get_component()

        # Without the fix in place, this test would have failed with
        # AssertionError: 2.5e-13 != 0.0
        self.assertEqual(2.5e-13, get_cell_by_position(document, 0, 0, 0).getValue())
        self.assertEqual(5e-13, get_cell_by_position(document, 0, 0, 1).getValue())
        self.assertEqual(7.5e-13, get_cell_by_position(document, 0, 0, 2).getValue())
        self.assertEqual(1e-12, get_cell_by_position(document, 0, 0, 3).getValue())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
