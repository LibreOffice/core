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
#Bug 91726 - AutoFormat: Crash when using an existing name for a new preset

class tdf91726(UITestCase):
    def test_tdf91726_new_style(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select A1:E9
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:E9"}))
            #AutoFormat Styles
            with self.ui_test.execute_dialog_through_command(".uno:AutoFormat", close_button="") as xDialog:
                #add new style "Default"
                xadd = xDialog.getChild("add")

                # Use empty close_button to open consecutive dialogs
                with self.ui_test.execute_blocking_action(
                        xadd.executeAction, args=('CLICK', ()), close_button="") as dialog:
                    nameEntry = dialog.getChild("name_entry")
                    nameEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"Default"}))
                    xOKBtn = dialog.getChild("ok")

                    with self.ui_test.execute_blocking_action(
                            xOKBtn.executeAction, args=('CLICK', ()), close_button="") as dialog2:
                        #Error message: You have entered an invalid name.
                        #AutoFormat could not be created.
                        #Try again using a different name.
                        xOKBtn2 = dialog2.getChild("ok")

                        with self.ui_test.execute_blocking_action(
                                xOKBtn2.executeAction, args=('CLICK', ()), close_button="cancel") as dialog3:
                            nameEntry = dialog3.getChild("name_entry")
                            #back to name dialog, LO should not crash
                            self.assertEqual(get_state_as_dict(nameEntry)["Text"], "Default")

                xCanceltn = xDialog.getChild("cancel")
                self.ui_test.close_dialog_through_button(xCanceltn)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
