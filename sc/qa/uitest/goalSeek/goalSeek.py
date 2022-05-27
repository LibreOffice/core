# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


class goalSeek(UITestCase):
    def test_goalSeek(self):
        with self.ui_test.load_file(get_url_for_data_file("goalSeek.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B4"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:GoalSeekDialog", close_button="") as xDialog:
                xtarget = xDialog.getChild("target")
                xvaredit = xDialog.getChild("varedit")
                xtarget.executeAction("TYPE", mkPropertyValues({"TEXT":"15000"}))
                xvaredit.executeAction("TYPE", mkPropertyValues({"TEXT":"B1"}))
                xOKBtn = xDialog.getChild("ok")

                with self.ui_test.execute_blocking_action(xOKBtn.executeAction, args=('CLICK', ()), close_button="yes"):
                    pass

            #verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getValue(), 200000)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getValue(), 15000)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
