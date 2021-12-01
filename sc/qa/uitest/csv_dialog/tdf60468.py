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

class Tdf60468(UITestCase):

    def test_tdf60468(self):

        # Load file from Open dialog
        with load_csv_file(self, "tdf60468.csv", True):
            pass

        document = self.ui_test.get_component()

        # tdf#142040: Without the fix in place, this test would have failed with
        # AssertionError: 'head1' != ''
        for i in range(3):
            self.assertEqual("head" + str(i + 1), get_cell_by_position(document, 0, i, 0).getString())

        self.assertEqual("value1.1\nvalue1.2", get_cell_by_position(document, 0, 0, 1).getString())

        # Without the fix in place, this test would have failed with
        # AssertionError: 'value2.1\n\tvalue2.2\nvalue2.3' != 'value2.1\n\tvalue2.2'
        self.assertEqual("value2.1\n\tvalue2.2\nvalue2.3", get_cell_by_position(document, 0, 1, 1).getString())
        self.assertEqual("value3", get_cell_by_position(document, 0, 2, 1).getString())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
