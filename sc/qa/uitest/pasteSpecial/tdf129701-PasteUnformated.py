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
from uitest.uihelper.common import get_url_for_data_file
from uitest.uihelper.common import get_state_as_dict
from libreoffice.calc.document import get_cell_by_position

class tdf129701(UITestCase):

    def test_tdf129701(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf129701.ods")):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "D21:F25"}))
            self.xUITest.executeCommand(".uno:Copy")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "J4"}))
            with self.ui_test.execute_dialog_through_command(".uno:PasteUnformatted", close_button="ok") as xDialog:

                xSkipEmtyCells   = xDialog.getChild("skipemptycells")
                xSeparatedBy     = xDialog.getChild("toseparatedby")
                xTab             = xDialog.getChild("tab")
                xMergeDelimiters = xDialog.getChild("mergedelimiters")

                xSeparatedBy.executeAction("CLICK", tuple())
                if get_state_as_dict(xTab)['Selected'] == 'false':
                    xTab.executeAction("CLICK", tuple())
                if get_state_as_dict(xMergeDelimiters)['Selected'] == 'true':
                    xMergeDelimiters.executeAction("CLICK", tuple())
                if get_state_as_dict(xSkipEmtyCells)['Selected'] == 'true':
                    xSkipEmtyCells.executeAction("CLICK", tuple())
                # Check wether Skip empty cells is unselected
                self.assertEqual('false', get_state_as_dict(xSkipEmtyCells)['Selected'])

            document = self.ui_test.get_component()
            # Without the fix in place, this test would have failed with
            # non empty cells in column 11

            self.assertEqual( "x1", get_cell_by_position(document, 0, 9, 3).getString())
            self.assertEqual( ""  , get_cell_by_position(document, 0,10, 3).getString())
            self.assertEqual( ""  , get_cell_by_position(document, 0,11, 3).getString())
            self.assertEqual("A16", get_cell_by_position(document, 0,12, 3).getString())
            self.assertEqual( ""  , get_cell_by_position(document, 0, 9, 4).getString())
            self.assertEqual( "x2", get_cell_by_position(document, 0,10, 4).getString())
            self.assertEqual( ""  , get_cell_by_position(document, 0,11, 4).getString())
            self.assertEqual("A17", get_cell_by_position(document, 0,12, 4).getString())
            self.assertEqual( ""  , get_cell_by_position(document, 0, 9, 5).getString())
            self.assertEqual( ""  , get_cell_by_position(document, 0,10, 5).getString())
            self.assertEqual( "x3", get_cell_by_position(document, 0,11, 5).getString())
            self.assertEqual("A18", get_cell_by_position(document, 0,12, 5).getString())
            self.assertEqual( ""  , get_cell_by_position(document, 0, 9, 6).getString())
            self.assertEqual( "x4", get_cell_by_position(document, 0,10, 6).getString())
            self.assertEqual( ""  , get_cell_by_position(document, 0,11, 6).getString())
            self.assertEqual("A19", get_cell_by_position(document, 0,12, 6).getString())
            self.assertEqual( "x5", get_cell_by_position(document, 0, 9, 7).getString())
            self.assertEqual( "x6", get_cell_by_position(document, 0,10, 7).getString())
            self.assertEqual( "x7", get_cell_by_position(document, 0,11, 7).getString())
            self.assertEqual("A20", get_cell_by_position(document, 0,12, 7).getString())

            self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
