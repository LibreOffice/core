# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file

class tdf153622(UITestCase):

    def test_tdf153622(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf153622.xlsx")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()

            A1DropDown = ['apple', 'banana']
            for i in range(len(A1DropDown)):
                gridwin = xCalcDoc.getChild("grid_window")

                # Without the fix in place, this test would have crashed here
                gridwin.executeAction("LAUNCH", mkPropertyValues({"SELECTMENU": "", "COL": "0", "ROW": "1"}))

                #Select the TreeList UI Object
                xWin = self.xUITest.getTopFocusWindow()
                xlist = xWin.getChild("list")

                xListItem = xlist.getChild( str(i) )
                xListItem.executeAction("DOUBLECLICK" , mkPropertyValues({}) )

                # tdf#151794: Without the fix in place, this test would have failed with
                # AssertionError: '' != 'apple'
                self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getString(), A1DropDown[i])

            A2DropDown = ['', 'apple', 'banana']
            for i in range(len(A2DropDown)):
                gridwin = xCalcDoc.getChild("grid_window")

                gridwin.executeAction("LAUNCH", mkPropertyValues({"SELECTMENU": "", "COL": "0", "ROW": "2"}))

                #Select the TreeList UI Object
                xWin = self.xUITest.getTopFocusWindow()
                xlist = xWin.getChild("list")

                xListItem = xlist.getChild( str(i) )
                xListItem.executeAction("DOUBLECLICK" , mkPropertyValues({}) )

                # tdf#79571: Without the fix in place, this test would have failed with
                # AssertionError: 'banana' != ''
                self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2).getString(), A2DropDown[i])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
