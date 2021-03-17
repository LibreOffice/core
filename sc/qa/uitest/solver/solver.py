# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class solver(UITestCase):
    def test_solver(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("solver.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B4"}))
        self.ui_test.execute_modeless_dialog_through_command(".uno:SolverDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xtargetedit = xDialog.getChild("targetedit")
        xvalue = xDialog.getChild("value")
        xvalueedit = xDialog.getChild("valueedit")
        xchangeedit = xDialog.getChild("changeedit")
        xref1edit = xDialog.getChild("ref1edit")
        xval1edit = xDialog.getChild("val1edit")
        xref2edit = xDialog.getChild("ref2edit")
        xval2edit = xDialog.getChild("val2edit")
        xop2list = xDialog.getChild("op2list")

        xvalue.executeAction("CLICK", tuple())
        xvalueedit.executeAction("TYPE", mkPropertyValues({"TEXT":"1000"}))
        xchangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"C2"}))
        xref1edit.executeAction("TYPE", mkPropertyValues({"TEXT":"C2"}))
        xval1edit.executeAction("TYPE", mkPropertyValues({"TEXT":"C4"}))
        xref2edit.executeAction("TYPE", mkPropertyValues({"TEXT":"C4"}))
        select_by_text(xop2list, "=>")

        xval2edit.executeAction("TYPE", mkPropertyValues({"TEXT":"0"}))
        xOKBtn = xDialog.getChild("ok")
        def handle_OK_dlg(dialog):
            #('SolverSuccessDialog', 'cancel', 'dialog-action_area1', 'dialog-vbox1', 'grid1', 'label1', 'label2', 'ok', 'result')
            xYesButn = dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xYesButn)

        self.ui_test.execute_blocking_action(xOKBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_OK_dlg)
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 400)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
