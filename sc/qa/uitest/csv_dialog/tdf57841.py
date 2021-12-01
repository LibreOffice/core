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

class Tdf57841(UITestCase):

    def test_tdf57841(self):
        with load_csv_file(self, "tdf57841.csv", True):
            pass

        document = self.ui_test.get_component()

        # Without the fix in place, this test would have failed with
        # AssertionError: '2' != ''
        for i in range(4):
            self.assertEqual(str(i + 1), get_cell_by_position(document, 0, i, 0).getString())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
