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


# Bug 43693 - Goal Seek: reproducible crash using "target value search"
class tdf43693(UITestCase):
    def test_tdf43693_goalSeek(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf43693.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "K248"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:GoalSeekDialog", close_button="") as xDialog:
                xformulaedit = xDialog.getChild("formulaedit")
                xtarget = xDialog.getChild("target")
                xvaredit = xDialog.getChild("varedit")
                xtarget.executeAction("TYPE", mkPropertyValues({"TEXT":"0"}))
                xvaredit.executeAction("TYPE", mkPropertyValues({"TEXT":"H5"}))
                xOKBtn = xDialog.getChild("ok")

                with self.ui_test.execute_blocking_action(xOKBtn.executeAction, args=('CLICK', ()), close_button="yes"):
                    pass

            #verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 7, 4).getValue(), 0.04)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
