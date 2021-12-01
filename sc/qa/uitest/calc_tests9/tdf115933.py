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
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 115933 - XLSX <fileSharing> password protected with algorithmName, hashValue, saltValue and spinCount

class tdf115933(UITestCase):

    def test_tdf115933(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf115933.xlsx")):
            #The document was created in Excel.
            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            document = self.ui_test.get_component()

            self.assertTrue(document.isReadonly())

            #Without the fix in place, this dialog wouldn't have been displayed
            with self.ui_test.execute_dialog_through_action(gridwin, "TYPE", mkPropertyValues({"KEYCODE": "CTRL+SHIFT+M"})) as xDialog:
                xPassword = xDialog.getChild("newpassEntry")
                xPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "a"}))

            self.assertFalse(document.isReadonly())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
