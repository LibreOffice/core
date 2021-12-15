# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict, select_by_text
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf143008(UITestCase):

    def test_tdf143008(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #fill data
            enter_text_to_cell(gridwin, "A1", "22/06/2021 10:02 PM")

            with self.ui_test.execute_dialog_through_command(".uno:TextToColumns") as xDialog:
                xGrid = xDialog.getChild("csvgrid")
                xColumnType = xDialog.getChild("columntype")

                # Use the right arrow to put the focus in the grid
                xGrid.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RIGHT"}))

                self.assertEqual('true', get_state_as_dict(xColumnType)['Enabled'])

                select_by_text(xColumnType, "Date (DMY)")

                self.assertEqual('Date (DMY)', get_state_as_dict(xColumnType)['SelectEntryText'])

            # Without the fix in place, this test would have failed with
            # AssertionError: '06/22/21 10:02 PM' != '06/22/21 10:02 AM'
            self.assertEqual("06/22/21 10:02 PM", get_cell_by_position(document, 0, 0, 0).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
