# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.calc.csv_dialog import load_csv_file

class Tdf96561(UITestCase):

    def test_tdf96561(self):
        with load_csv_file(self, "tdf96561.csv", True):
            pass

        document = self.ui_test.get_component()

        self.assertEqual('Campo con ";" en medio', get_cell_by_position(document, 0, 0, 0).getString())

        # Without the fix in place, this test would have failed with
        # AssertionError: '""; esta al """""principio del Campo' != 'Campo la tiene al final;"'
        self.assertEqual('""; esta al """""principio del Campo', get_cell_by_position(document, 0, 0, 1).getString())
        self.assertEqual('Campo la tiene al final;"', get_cell_by_position(document, 0, 0, 2).getString())

        for i in range(1,5):
            for j in range(0,3):
                self.assertEqual("1", get_cell_by_position(document, 0, i, j).getString())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
