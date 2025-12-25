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
from com.sun.star.beans import PropertyValue

class tdf144996(UITestCase):

    def test_tdf144996(self):

        # get_url_for_data_file calls makeCopyFromTDOC
        # so using save doesn't affect the original file
        xFilePath = get_url_for_data_file("tdf144996.xlsx")

        with self.ui_test.load_file(xFilePath, [PropertyValue(Name="Silent", Value=True)]) as document:

            self.assertTrue(document.isReadonly())

            with self.ui_test.execute_dialog_through_command(".uno:EditDoc") as xDialog:
                xPassword = xDialog.getChild("newpassEntry")
                xPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "password"}))

            self.assertFalse(document.isReadonly())

            with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="btnYes"):
                # Confirm file format popup
                pass

        with self.ui_test.load_file(xFilePath, [PropertyValue(Name="Silent", Value=True)]) as document:

            self.assertTrue(document.isReadonly())

            # Without the fix in place, this test would have failed here because the saved document
            # doesn't need a password to edit it
            with self.ui_test.execute_dialog_through_command(".uno:EditDoc") as xDialog:
                xPassword = xDialog.getChild("newpassEntry")
                xPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "password"}))

            self.assertFalse(document.isReadonly())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
