# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


class protectSheet(UITestCase):
    def test_protect_sheet(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #enter password - lock
            with self.ui_test.execute_dialog_through_command(".uno:Protect") as xDialog:
                xprotect = xDialog.getChild("protect")
                xpassword1 = xDialog.getChild("password1")
                xpassword2 = xDialog.getChild("password2")

                if (get_state_as_dict(xprotect)["Selected"]) == "false":
                    xprotect.executeAction("CLICK", tuple())
                xpassword1.executeAction("TYPE", mkPropertyValues({"TEXT":"aa"}))
                xpassword2.executeAction("TYPE", mkPropertyValues({"TEXT":"aa"}))

            #Unlock

            with self.ui_test.execute_dialog_through_command(".uno:Protect") as xDialog:
                xpass1ed = xDialog.getChild("pass1ed")

                xpass1ed.executeAction("TYPE", mkPropertyValues({"TEXT":"aa"}))

            #Verify - the sheet is unlocked
            enter_text_to_cell(gridwin, "B2", "A")
            self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "A")

            # test cancel button
            with self.ui_test.execute_dialog_through_command(".uno:Protect", close_button="cancel"):
                pass

            enter_text_to_cell(gridwin, "B2", "B")
            self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "B")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
