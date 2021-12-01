# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf126577(UITestCase):

    def test_tdf126577(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A20"}))

            with self.ui_test.execute_dialog_through_command(".uno:FillSeries") as xDialog:

                self.assertEqual("true", get_state_as_dict(xDialog.getChild("down"))['Checked'])
                self.assertEqual("true", get_state_as_dict(xDialog.getChild("linear"))['Checked'])

                xStart = xDialog.getChild("startValue")
                xStart.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))

                xEnd = xDialog.getChild("endValue")
                xEnd.executeAction("TYPE", mkPropertyValues({"TEXT":"10"}))


            for i in range(10):
                self.assertEqual(str(i + 1), get_cell_by_position(document, 0, 0, i).getString())

            for i in range(10, 20):
                # Without the fix in place, this test would have failed with
                # AssertionError: '' != '#NUM!'
                self.assertEqual("", get_cell_by_position(document, 0, 0, i).getString())


# vim: set shiftwidth=4 softtabstop=4 expandtab:
