# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position

class tdf150288(UITestCase):

    def test_tdf150288(self):
        with self.ui_test.set_config('/org.openoffice.Setup/L10N/ooSetupSystemLocale', 'en-US'), \
                self.ui_test.set_config(
                    '/org.openoffice.Setup/L10N/DateAcceptancePatterns', 'D/M/Y;D/M'), \
                self.ui_test.create_doc_in_start_center("calc") as document:

            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(xGridWin, "A1", "31/8/22")
            enter_text_to_cell(xGridWin, "A2", "1/8/22")

            self.assertEqual("08/31/22", get_cell_by_position(document, 0, 0, 0).getString())

            # Without the fix in place, this test would have failed with
            # AssertionError: '08/01/22' != '01/08/22'
            self.assertEqual("08/01/22", get_cell_by_position(document, 0, 0, 1).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
