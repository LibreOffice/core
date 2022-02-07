# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

class tdf74577(UITestCase):

    def test_tdf74577(self):

        # Open the HTML in writer
        with self.ui_test.load_file(get_url_for_data_file("tdf74577.html")):

            # Use SelectAll twice to select the table
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:SelectAll")

            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_empty_file("calc") as calc_document:

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            with self.ui_test.execute_dialog_through_command(".uno:PasteSpecial", close_button="") as xDialog:

                xList = xDialog.getChild('list')

                for childName in xList.getChildren():
                    xChild = xList.getChild(childName)
                    if get_state_as_dict(xChild)['Text'] == "HyperText Markup Language (HTML)":
                        break

                xChild.executeAction("SELECT", tuple())
                self.assertEqual(
                        get_state_as_dict(xList)['SelectEntryText'], "HyperText Markup Language (HTML)")

                xOkBtn = xDialog.getChild("ok")

                with self.ui_test.execute_blocking_action(xOkBtn.executeAction, args=('CLICK', ())):
                    pass

            self.assertEqual("Cell1", get_cell_by_position(calc_document, 0, 0, 0).getString())
            self.assertEqual("Cell1", get_cell_by_position(calc_document, 0, 0, 1).getString())
            self.assertEqual("Cell1 + Cell2", get_cell_by_position(calc_document, 0, 0, 2).getString())
            self.assertEqual("Cell1 + Cell2 + Cell3", get_cell_by_position(calc_document, 0, 0, 3).getString())
            self.assertEqual("Cell1 + Cell2", get_cell_by_position(calc_document, 0, 0, 4).getString())

            self.assertEqual("Cell2 + 3", get_cell_by_position(calc_document, 0, 1, 0).getString())
            self.assertEqual("Cell2", get_cell_by_position(calc_document, 0, 1, 1).getString())
            self.assertEqual("Cell3", get_cell_by_position(calc_document, 0, 2, 1).getString())

            # Without the fix in place, this test would have failed with
            # AssertionError: 'Cell3' != ''
            self.assertEqual("Cell3", get_cell_by_position(calc_document, 0, 2, 2).getString())

            self.assertEqual("Cell2 + Cell3", get_cell_by_position(calc_document, 0, 1, 4).getString())
            self.assertEqual("Cell2", get_cell_by_position(calc_document, 0, 1, 5).getString())
            self.assertEqual("Cell3", get_cell_by_position(calc_document, 0, 2, 5).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
