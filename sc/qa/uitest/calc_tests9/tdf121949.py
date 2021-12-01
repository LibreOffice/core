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

class tdf121949 (UITestCase):

    def test_tdf121949_copy_block_with_single_cell_not_included(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            # mark a range with a single cell left our and copy to clipboard
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A3"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B1:B1", "EXTEND":"1"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B3:B3", "EXTEND":"1"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C1:C3", "EXTEND":"1"}))
            with self.ui_test.execute_dialog_through_command(".uno:Copy"):
                pass


# vim: set shiftwidth=4 softtabstop=4 expandtab:

