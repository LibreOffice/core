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

class Tdf101313(UITestCase):

    def test_tdf101313(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf101313.odt")):

            self.xUITest.executeCommand(".uno:SelectTable")

            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_empty_file("calc") as document:

            self.xUITest.executeCommand(".uno:Paste")

            self.assertEqual("a", get_cell_by_position(document, 0, 0, 0).getString())
            self.assertEqual("e", get_cell_by_position(document, 0, 0, 1).getString())
            self.assertEqual("b", get_cell_by_position(document, 0, 1, 0).getString())
            self.assertEqual("f", get_cell_by_position(document, 0, 1, 1).getString())

            # Without the fix in place, this test would have failed with
            # AssertionError: '' != 'c'
            self.assertEqual("", get_cell_by_position(document, 0, 2, 0).getString())
            self.assertEqual("g", get_cell_by_position(document, 0, 2, 1).getString())
            self.assertEqual("c", get_cell_by_position(document, 0, 3, 0).getString())
            self.assertEqual("h", get_cell_by_position(document, 0, 3, 1).getString())
            self.assertEqual("d", get_cell_by_position(document, 0, 4, 0).getString())
            self.assertEqual("i", get_cell_by_position(document, 0, 4, 1).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
