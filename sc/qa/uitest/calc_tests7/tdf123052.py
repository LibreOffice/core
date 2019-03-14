# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib

#Bug 123052 - [regression] Shift+Tab not working
def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf123052(UITestCase):
    def test_tdf123052_shit_tab(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf123052.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Holding the Shift key and repeated pressing the Tab key does not correctly cycle though the unprotected cells
        #I'm at 7, hold TAB for cycling
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        # Assert that the correct cell has been selected
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "2")
        self.assertEqual(gridWinState["CurrentColumn"], "5")

        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        # Assert that the correct cell has been selected
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "4")
        self.assertEqual(gridWinState["CurrentColumn"], "3")

        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        # Assert that the correct cell has been selected
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "4")
        self.assertEqual(gridWinState["CurrentColumn"], "4")

        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        # Assert that the correct cell has been selected
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "5")
        self.assertEqual(gridWinState["CurrentColumn"], "5")

        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        # Assert that the correct cell has been selected
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "7")
        self.assertEqual(gridWinState["CurrentColumn"], "0")

        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        # Assert that the correct cell has been selected
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "8")
        self.assertEqual(gridWinState["CurrentColumn"], "4")
        #now shift + tab
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+TAB"}))
        # Assert that the correct cell has been selected
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "7")
        self.assertEqual(gridWinState["CurrentColumn"], "0")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
