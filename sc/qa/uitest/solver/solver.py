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
                select_by_text(xop2list, "=>")

                self.assertEqual("<=", get_state_as_dict(xop1list)["SelectEntryText"])
                self.assertEqual("=>", get_state_as_dict(xop2list)["SelectEntryText"])

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

# vim: set shiftwidth=4 softtabstop=4 expandtab:
