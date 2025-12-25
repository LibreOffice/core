# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

class tdf95520(UITestCase):
    def test_tdf95520(self):
        # Reuse existing document
        with self.ui_test.load_file(get_url_for_data_file("tdf144549.ods")) as doc:
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWin = calcDoc.getChild("grid_window")

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xMenu = xFloatWindow.getChild("menu")

            # Sort by Color
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            xSubFloatWindow = self.xUITest.getFloatWindow()
            xSubMenu = xSubFloatWindow.getChild("textcolor")
            self.assertEqual(3, len(xSubMenu.getChildren()))
            self.assertEqual('false', get_state_as_dict(xSubMenu.getChild('0'))['IsChecked'])
            self.assertEqual('#00FF00', get_state_as_dict(xSubMenu.getChild('0'))['Text'])
            self.assertEqual('false', get_state_as_dict(xSubMenu.getChild('1'))['IsChecked'])
            self.assertEqual('false', get_state_as_dict(xSubMenu.getChild('2'))['IsChecked'])

            # Choose Red
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            self.assertEqual("Jan", get_cell_by_position(doc, 0, 0, 1).getString())
            self.assertEqual("Dez", get_cell_by_position(doc, 0, 0, 2).getString())
            self.assertEqual("Aug", get_cell_by_position(doc, 0, 0, 3).getString())
            self.assertEqual("Nov", get_cell_by_position(doc, 0, 0, 4).getString())
            self.assertEqual("Jun", get_cell_by_position(doc, 0, 0, 5).getString())
            self.assertEqual("Apr", get_cell_by_position(doc, 0, 0, 6).getString())
            self.assertEqual("Mai", get_cell_by_position(doc, 0, 0, 7).getString())
            self.assertEqual("Okt", get_cell_by_position(doc, 0, 0, 8).getString())
            self.assertEqual("Feb", get_cell_by_position(doc, 0, 0, 9).getString())
            self.assertEqual("Mär", get_cell_by_position(doc, 0, 0, 10).getString())
            self.assertEqual("Jul", get_cell_by_position(doc, 0, 0, 11).getString())
            self.assertEqual("Sep", get_cell_by_position(doc, 0, 0, 12).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
