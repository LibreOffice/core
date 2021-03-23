# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
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

        self.ui_test.create_doc_in_start_center("calc")

        calcDoc = self.xUITest.getTopFocusWindow()
        gridwin = calcDoc.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        type_text(gridwin, "=SUM({A1},2,3,4,5}")

        # Without the fix in place, this dialog wouldn't have been displayed
        self.ui_test.execute_blocking_action(gridwin.executeAction,
            args=("TYPE", mkPropertyValues({"KEYCODE": "RETURN"})), dialog_element="yes")

        document = self.ui_test.get_component()
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getFormula(), "=SUM({A1};2;3;4;5)")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        type_text(gridwin, "=SUM({A1},2,3,4,5}")

        self.ui_test.execute_blocking_action(gridwin.executeAction,
            args=("TYPE", mkPropertyValues({"KEYCODE": "RETURN"})), dialog_element="no")

        document = self.ui_test.get_component()
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getFormula(), "=SUM({A1};2;3;4;5})")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
