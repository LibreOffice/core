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
from uitest.uihelper.common import select_by_text

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


class solver(UITestCase):
    def test_solver(self):
        with self.ui_test.load_file(get_url_for_data_file("solver.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B4"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SolverDialog", close_button="") as xDialog:
                xtargetedit = xDialog.getChild("targetedit")
                xmax = xDialog.getChild("max")
                xvalue = xDialog.getChild("value")
                xvalueedit = xDialog.getChild("valueedit")
                xchangeedit = xDialog.getChild("changeedit")
                xref1edit = xDialog.getChild("ref1edit")
                xval1edit = xDialog.getChild("val1edit")
                xop1list = xDialog.getChild("op1list")
                xref2edit = xDialog.getChild("ref2edit")
                xval2edit = xDialog.getChild("val2edit")
                xop2list = xDialog.getChild("op2list")

                self.assertEqual("$B$4", get_state_as_dict(xtargetedit)["Text"])
                self.assertEqual("true", get_state_as_dict(xmax)["Checked"])

                xvalue.executeAction("CLICK", tuple())
                select_by_text(xop2list, "≥")

                self.assertEqual("≤", get_state_as_dict(xop1list)["SelectEntryText"])
                self.assertEqual("≥", get_state_as_dict(xop2list)["SelectEntryText"])

                xvalueedit.executeAction("TYPE", mkPropertyValues({"TEXT":"1000"}))
                xchangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"C2"}))
                xref1edit.executeAction("TYPE", mkPropertyValues({"TEXT":"C2"}))
                xval1edit.executeAction("TYPE", mkPropertyValues({"TEXT":"C4"}))
                xref2edit.executeAction("TYPE", mkPropertyValues({"TEXT":"C4"}))
                xval2edit.executeAction("TYPE", mkPropertyValues({"TEXT":"0"}))

                xOKBtn = xDialog.getChild("ok")

                with self.ui_test.execute_blocking_action(xOKBtn.executeAction, args=('CLICK', ())) as xWarnDialog:
                    self.assertEqual("Solving successfully finished.", get_state_as_dict(xWarnDialog.getChild("label2"))["Text"])
                    self.assertEqual("Result: 1000", get_state_as_dict(xWarnDialog.getChild("result"))["Text"])

            #verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getValue(), 400)


    # Tests the isModified property on a blank Calc file
    def test_tdf160104_blank_file(self):
        with self.ui_test.create_doc_in_start_center("calc") as calc_doc:
            self.assertFalse(calc_doc.isModified())
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SolverDialog", close_button="") as xDialog:
                xCloseBtn = xDialog.getChild("close")
                xCloseBtn.executeAction("CLICK", ())

            # Here isModified needs to be False because the dialog was opened and closed with no changes
            self.assertFalse(calc_doc.isModified())

            # Now open the dialog again and make some changes
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SolverDialog", close_button="") as xDialog:
                xCloseBtn = xDialog.getChild("close")
                xMin = xDialog.getChild("min")
                xChangeEdit = xDialog.getChild("changeedit")

                # Click the Minimize option and change variable cells
                xMin.executeAction("CLICK", ())
                xChangeEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "$A$1:$A$10"}))
                xCloseBtn.executeAction("CLICK", ())

            # Here isModified needs to be True because changes were made to the dialog
            self.assertTrue(calc_doc.isModified())


    # Tests the isModified property on an existing file that contains a solver model
    def test_tdf160104_with_file(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf160104.ods")) as calc_doc:
            self.assertFalse(calc_doc.isModified())
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SolverDialog", close_button="") as xDialog:
                xTargetEdit = xDialog.getChild("targetedit")
                xMax = xDialog.getChild("max")
                xChangeEdit = xDialog.getChild("changeedit")
                xRef1Edit = xDialog.getChild("ref1edit")
                xVal1Edit = xDialog.getChild("val1edit")
                xOp1List = xDialog.getChild("op1list")
                xRef2Edit = xDialog.getChild("ref2edit")
                xVal2Edit = xDialog.getChild("val2edit")
                xOp2List = xDialog.getChild("op2list")

                # Checks whether the solver model was loaded correctly
                self.assertEqual("$F$2", get_state_as_dict(xTargetEdit)["Text"])
                self.assertEqual("true", get_state_as_dict(xMax)["Checked"])
                self.assertEqual("$D$2:$D$11", get_state_as_dict(xChangeEdit)["Text"])
                self.assertEqual("$F$5", get_state_as_dict(xRef1Edit)["Text"])
                self.assertEqual("$F$8", get_state_as_dict(xVal1Edit)["Text"])
                self.assertEqual("≤", get_state_as_dict(xOp1List)["SelectEntryText"])
                self.assertEqual("$D$2:$D$11", get_state_as_dict(xRef2Edit)["Text"])
                self.assertEqual("", get_state_as_dict(xVal2Edit)["Text"])
                self.assertEqual("Binary", get_state_as_dict(xOp2List)["SelectEntryText"])

                # Closes the dialog without making changes
                xCloseBtn = xDialog.getChild("close")
                xCloseBtn.executeAction("CLICK", ())

            # Here isModified needs to be False no changes were made to the solver dialog
            self.assertFalse(calc_doc.isModified())

            # Now open the dialog again and make some changes
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SolverDialog", close_button="") as xDialog:
                xCloseBtn = xDialog.getChild("close")
                xMin = xDialog.getChild("min")
                xChangeEdit = xDialog.getChild("changeedit")

                # Click the Minimize option and change variable cells
                xMin.executeAction("CLICK", ())
                xChangeEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "$E$2:$E$11"}))
                xCloseBtn.executeAction("CLICK", ())

            # Here isModified needs to be True because changes were made to the Solver dialog
            self.assertTrue(calc_doc.isModified())


    # Tests whether all solver named ranges are hidden in the UI
    def test_tdf160064(self):
        # This test uses the same file from bug tdf#160104, so no need to check if the model is correct upon opening
        with self.ui_test.load_file(get_url_for_data_file("tdf160104.ods")) as calc_doc:
            # The Manage Names dialog must not contain any names
            with self.ui_test.execute_dialog_through_command(".uno:DefineName") as xDialog:
                xList = xDialog.getChild("names")
                self.assertEqual('0', get_state_as_dict(xList)['Children'])

            # Makes a small change in the solver dialog by clicking the "Minimize" button
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SolverDialog", close_button="") as xDialog:
                xMin = xDialog.getChild("min")
                xMin.executeAction("CLICK", ())
                # Closes the dialog
                xCloseBtn = xDialog.getChild("close")
                xCloseBtn.executeAction("CLICK", ())

            # Here the file has been modified and needs to be saved and reloaded
            self.assertTrue(calc_doc.isModified())
            self.xUITest.executeCommand('.uno:Save')
            self.xUITest.executeCommand('.uno:Reload')

            # Open the Solver dialog and check whether the model is loaded correctly
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SolverDialog", close_button="") as xDialog:
                xTargetEdit = xDialog.getChild("targetedit")
                xMin = xDialog.getChild("min")
                xChangeEdit = xDialog.getChild("changeedit")
                xRef1Edit = xDialog.getChild("ref1edit")
                xVal1Edit = xDialog.getChild("val1edit")
                xOp1List = xDialog.getChild("op1list")
                xRef2Edit = xDialog.getChild("ref2edit")
                xVal2Edit = xDialog.getChild("val2edit")
                xOp2List = xDialog.getChild("op2list")

                # Checks whether the solver model was loaded correctly
                self.assertEqual("$F$2", get_state_as_dict(xTargetEdit)["Text"])
                self.assertEqual("true", get_state_as_dict(xMin)["Checked"])
                self.assertEqual("$D$2:$D$11", get_state_as_dict(xChangeEdit)["Text"])
                self.assertEqual("$F$5", get_state_as_dict(xRef1Edit)["Text"])
                self.assertEqual("$F$8", get_state_as_dict(xVal1Edit)["Text"])
                self.assertEqual("≤", get_state_as_dict(xOp1List)["SelectEntryText"])
                self.assertEqual("$D$2:$D$11", get_state_as_dict(xRef2Edit)["Text"])
                self.assertEqual("", get_state_as_dict(xVal2Edit)["Text"])
                self.assertEqual("Binary", get_state_as_dict(xOp2List)["SelectEntryText"])

                # Closes the dialog
                xCloseBtn = xDialog.getChild("close")
                xCloseBtn.executeAction("CLICK", ())

            # Open the Manage Names dialog again; it must not contain any names
            with self.ui_test.execute_dialog_through_command(".uno:DefineName") as xDialog:
                xList = xDialog.getChild("names")
                self.assertEqual('0', get_state_as_dict(xList)['Children'])



# vim: set shiftwidth=4 softtabstop=4 expandtab:
