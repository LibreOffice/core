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
from uitest.uihelper.common import type_text
from libreoffice.calc.document import get_cell_by_position

class tdf117879(UITestCase):

    def test_tdf117879(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:

            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            type_text(gridwin, "=SUM({A1},2,3,4,5}")

            # Without the fix in place, this dialog wouldn't have been displayed
            with self.ui_test.execute_blocking_action(gridwin.executeAction,
                    args=("TYPE", mkPropertyValues({"KEYCODE": "RETURN"})), close_button="yes"):
                pass

            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getFormula(), "=SUM({A1};2;3;4;5)")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            type_text(gridwin, "=SUM({A1},2,3,4,5}")

            with self.ui_test.execute_blocking_action(gridwin.executeAction,
                    args=("TYPE", mkPropertyValues({"KEYCODE": "RETURN"})), close_button="no"):
                pass

            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getFormula(), "=SUM({A1};2;3;4;5})")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
