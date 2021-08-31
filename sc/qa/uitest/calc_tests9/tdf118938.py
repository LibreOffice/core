# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 118938 - FILESAVE to Microsoft Excel 2007-2013 XML (.xlsx) files as read-only
#             with additional password protection for editing not working (Calc)

class tdf118938(UITestCase):
    def test_tdf118938(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf118938.xlsx")):
            #The document was created in Calc after this fix.
            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            incorrectPass = False;

            try:
                self.xUITest.executeDialog(".uno:EditDoc")
                xDialog = self.xUITest.getTopFocusWindow();
                xPassword = xDialog.getChild("newpassEntry")
                xPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "a"}))
                xOKBtn = xDialog.getChild("ok")
                self.ui_test.close_dialog_through_button(xOKBtn)

                try:
                    xWarnDialog = self.xUITest.getTopFocusWindow()
                    xOK = xWarnDialog.getChild("ok")
                    self.ui_test.close_dialog_through_button(xOK)

                    xDialog2 = self.xUITest.getTopFocusWindow();
                    xCancelBtn = xDialog2.getChild("cancel")
                    self.ui_test.close_dialog_through_button(xCancelBtn)

                    incorrectPass = True;
                except:
                    pass
            except:
                assert False, "The password dialog hasn't appeared."

            if incorrectPass:
                assert False, "Incorrect password."

# vim: set shiftwidth=4 softtabstop=4 expandtab:
