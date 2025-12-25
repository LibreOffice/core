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

class tdf133855(UITestCase):

    def test_tdf133855(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf133855.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()

            results = ['A', 'B', 'C', 'D', 'E', 'F']

            # Conditional formatted dropdown list
            for i in range(len(results)):
                gridwin = xCalcDoc.getChild("grid_window")
                gridwin.executeAction("LAUNCH", mkPropertyValues({"SELECTMENU": "", "COL": "6", "ROW": "5"}))

                #Select the TreeList UI Object
                xWin = self.xUITest.getTopFocusWindow()
                xlist = xWin.getChild("list")

                xListItem = xlist.getChild( str(i) )
                xListItem.executeAction("DOUBLECLICK" , mkPropertyValues({}) )
                self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 5).getString(), results[i])

            # normal dropdown list
            for i in range(len(results)):
                gridwin = xCalcDoc.getChild("grid_window")
                gridwin.executeAction("LAUNCH", mkPropertyValues({"SELECTMENU": "", "COL": "6", "ROW": "6"}))

                #Select the TreeList UI Object
                xWin = self.xUITest.getTopFocusWindow()
                xlist = xWin.getChild("list")

                xListItem = xlist.getChild( str(i) )
                xListItem.executeAction("DOUBLECLICK" , mkPropertyValues({}) )
                self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 6).getString(), results[i])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
