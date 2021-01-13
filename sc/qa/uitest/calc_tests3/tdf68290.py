# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Enhancement: EDITING: cursor moves with Enter key

class tdf68290(UITestCase):

    def test_tdf68290_cursor_moves_with_Enter_key(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf68290.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "2")
        self.assertEqual(gridWinState["CurrentColumn"], "12")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
