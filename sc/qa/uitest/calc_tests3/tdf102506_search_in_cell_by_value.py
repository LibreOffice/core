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
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell

class tdf102506(UITestCase):

    def test_tdf102506(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "=2+3")

            # Go to a different cell
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C10"}))

            self.xUITest.executeCommand("vnd.sun.star.findbar:FocusToFindbar")

            xfind = xCalcDoc.getChild("find")
            xfind.executeAction("TYPE", mkPropertyValues({"TEXT": "5"}))

            self.assertEqual("5", get_state_as_dict(xfind)['Text'])

            xfind_bar = xCalcDoc.getChild("FindBar")

            # Search Next
            xfind_bar.executeAction("CLICK", mkPropertyValues({"POS": "3"}))

            # Without the fix in place, this test would have failed with
            # AssertionError: '0' != '2'
            self.assertEqual("0", get_state_as_dict(gridwin)["CurrentColumn"])
            self.assertEqual("0", get_state_as_dict(gridwin)["CurrentRow"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
