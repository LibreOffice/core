# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

class tdf121263(UITestCase):

    def test_tdf121263_hide_more_sheets(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xTopWindow = self.xUITest.getTopFocusWindow()
            gridwin = xTopWindow.getChild("grid_window")

            with self.ui_test.execute_dialog_through_command(".uno:Insert"):
                pass
            with self.ui_test.execute_dialog_through_command(".uno:Insert"):
                pass

            gridwin.executeAction("SELECT", mkPropertyValues({"TABLE": "2"}))
            #select previous sheet
            self.xUITest.executeCommand(".uno:JumpToPrevTableSel")

            #hide sheet
            self.xUITest.executeCommand(".uno:Hide")

            #show sheet Dialog
            with self.ui_test.execute_dialog_through_command(".uno:Show", close_button="cancel") as xDialog:
                treeview = xDialog.getChild("treeview")
                self.assertEqual(get_state_as_dict(treeview)["Children"], "2")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
