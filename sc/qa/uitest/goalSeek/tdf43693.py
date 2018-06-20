# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 43693 - Goal Seek: reproducible crash using "target value search"

class tdf43693(UITestCase):
    def test_tdf43693_goalSeek(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf43693.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "K248"}))
        self.ui_test.execute_modeless_dialog_through_command(".uno:GoalSeekDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xformulaedit = xDialog.getChild("formulaedit")
        xtarget = xDialog.getChild("target")
        xvaredit = xDialog.getChild("varedit")
        xtarget.executeAction("TYPE", mkPropertyValues({"TEXT":"0"}))
        xvaredit.executeAction("TYPE", mkPropertyValues({"TEXT":"H5"}))
        xOKBtn = xDialog.getChild("ok")
        def handle_OK_dlg(dialog):
            xYesButn = dialog.getChild("yes")
            self.ui_test.close_dialog_through_button(xYesButn)

        self.ui_test.execute_blocking_action(xOKBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_OK_dlg)
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 7, 4).getValue(), 0.04)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
