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
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position

class tdf152717(UITestCase):

    def test_tdf152717(self):

        with self.ui_test.create_doc_in_start_center("calc") as calc_document:
            calcDoc = self.xUITest.getTopFocusWindow()

            xGridWindow = calcDoc.getChild("grid_window")

            # Use an existing document
            enter_text_to_cell(xGridWindow, "A1", "=DDE(\"soffice\";\"" + get_url_for_data_file("tdf119954.ods") + "\";\"Sheet1.A1\")")

            # Without the fix in place, this test would have failed with
            # AssertionError: 1.0 != 0.0
            self.assertEqual(1.0, get_cell_by_position(calc_document, 0, 0, 0).getValue())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
