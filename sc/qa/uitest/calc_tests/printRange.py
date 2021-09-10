# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class printRange(UITestCase):
    def test_printRange(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:F20"}))
            #Set print range
            self.xUITest.executeCommand(".uno:DefinePrintArea")
            # Print Range dialog
            with self.ui_test.execute_modeless_dialog_through_command_guarded(".uno:EditPrintArea") as xDialog:
                xlbprintarea = xDialog.getChild("lbprintarea")
                xedprintarea = xDialog.getChild("edprintarea")
                #verify range
                self.assertEqual(get_state_as_dict(xlbprintarea)["SelectEntryText"], "- selection -")
                self.assertEqual(get_state_as_dict(xedprintarea)["Text"], "$A$1:$F$20")
                #set Row
                xedrepeatrow = xDialog.getChild("edrepeatrow")
                xedrepeatrow.executeAction("TYPE", mkPropertyValues({"TEXT":"$1"}))
                #set Column
                xedrepeatcol = xDialog.getChild("edrepeatcol")
                xedrepeatcol.executeAction("TYPE", mkPropertyValues({"TEXT":"$A"}))
                # Click Ok

            #Verify Print Range dialog
            with self.ui_test.execute_modeless_dialog_through_command_guarded(".uno:EditPrintArea", close_button="cancel") as xDialog:
                xedprintarea = xDialog.getChild("edprintarea")
                xedrepeatrow = xDialog.getChild("edrepeatrow")
                xedrepeatcol = xDialog.getChild("edrepeatcol")
                self.assertEqual(get_state_as_dict(xedprintarea)["Text"], "$A$1:$F$20")
                self.assertEqual(get_state_as_dict(xedrepeatrow)["Text"], "$1")
                self.assertEqual(get_state_as_dict(xedrepeatcol)["Text"], "$A")

            #delete print ranges
            self.xUITest.executeCommand(".uno:DeletePrintArea")
            #Verify Print Range dialog
            with self.ui_test.execute_modeless_dialog_through_command_guarded(".uno:EditPrintArea") as xDialog:
                xedprintarea = xDialog.getChild("edprintarea")
                xlbprintarea = xDialog.getChild("lbprintarea")
                xedrepeatrow = xDialog.getChild("edrepeatrow")
                xedrepeatcol = xDialog.getChild("edrepeatcol")
                self.assertEqual(get_state_as_dict(xedprintarea)["Text"], "")
                self.assertEqual(get_state_as_dict(xlbprintarea)["SelectEntryText"], "- entire sheet -")
                self.assertEqual(get_state_as_dict(xedrepeatrow)["Text"], "$1")
                self.assertEqual(get_state_as_dict(xedrepeatcol)["Text"], "$A")

    def test_tdf33341_copy_sheet_with_print_range(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:F20"}))
            #Set print range
            self.xUITest.executeCommand(".uno:DefinePrintArea")
            # Print Range dialog
            with self.ui_test.execute_modeless_dialog_through_command_guarded(".uno:EditPrintArea") as xDialog:
                xlbprintarea = xDialog.getChild("lbprintarea")
                xedprintarea = xDialog.getChild("edprintarea")
                #verify range
                self.assertEqual(get_state_as_dict(xlbprintarea)["SelectEntryText"], "- selection -")
                self.assertEqual(get_state_as_dict(xedprintarea)["Text"], "$A$1:$F$20")
                #set Row
                xedrepeatrow = xDialog.getChild("edrepeatrow")
                xedrepeatrow.executeAction("TYPE", mkPropertyValues({"TEXT":"$1"}))
                #set Column
                xedrepeatcol = xDialog.getChild("edrepeatcol")
                xedrepeatcol.executeAction("TYPE", mkPropertyValues({"TEXT":"$A"}))
                # Click Ok

            #Copy sheet
            with self.ui_test.execute_dialog_through_command(".uno:Move"):
                pass
            #Verify Print Range dialog on new sheet
            with self.ui_test.execute_modeless_dialog_through_command_guarded(".uno:EditPrintArea") as xDialog:
                xedprintarea = xDialog.getChild("edprintarea")
                xedrepeatrow = xDialog.getChild("edrepeatrow")
                xedrepeatcol = xDialog.getChild("edrepeatcol")
                self.assertEqual(get_state_as_dict(xedprintarea)["Text"], "$A$1:$F$20")
                self.assertEqual(get_state_as_dict(xedrepeatrow)["Text"], "$1")
                self.assertEqual(get_state_as_dict(xedrepeatcol)["Text"], "$A")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
