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
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_by_text
from uitest.uihelper.calc import enter_text_to_cell

class tdf65334(UITestCase):

   def test_tdf65334(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A5"}))

            # Insert a comment in a cell without text
            xArgs = mkPropertyValues({"Text": "Comment 1"})
            self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

            # Insert a comment in a cell with text
            enter_text_to_cell(gridwin, "C10", "Cell with text")
            xArgs = mkPropertyValues({"Text": "Comment 2"})
            self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                xExpander = xDialog.getChild("OptionsExpander")

                # tdf#129629: Without the fix in place, this test would have failed with
                # AssertionError: 'false' != 'true'
                self.assertEqual("false", get_state_as_dict(xExpander)['Expanded'])

                xExpander.executeAction("EXPAND", tuple())
                self.assertEqual("true", get_state_as_dict(xExpander)['Expanded'])

                xCalcsearchin = xDialog.getChild("calcsearchin")
                select_by_text(xCalcsearchin, "Comments")

                xSearchterm = xDialog.getChild("searchterm")
                xSearchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"Comment"}))

                xSearch = xDialog.getChild("search")
                xSearch.executeAction("CLICK", tuple())

            self.assertEqual("0", get_state_as_dict(gridwin)["CurrentColumn"])

            # Without the fix in place, this test would have failed with
            # AssertionError: '4' != '0'
            self.assertEqual("4", get_state_as_dict(gridwin)["CurrentRow"])

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                xSearchterm = xDialog.getChild("searchterm")
                xCalcsearchin = xDialog.getChild("calcsearchin")
                xExpander = xDialog.getChild("OptionsExpander")

                self.assertEqual("false", get_state_as_dict(xExpander)['Expanded'])
                self.assertEqual("Comment", get_state_as_dict(xSearchterm)['Text'])
                self.assertEqual("Comments", get_state_as_dict(xCalcsearchin)['SelectEntryText'])

                xSearch = xDialog.getChild("search")
                xSearch.executeAction("CLICK", tuple())

            self.assertEqual("2", get_state_as_dict(gridwin)["CurrentColumn"])
            self.assertEqual("9", get_state_as_dict(gridwin)["CurrentRow"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
