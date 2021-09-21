# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 89383 - Read-only passwords on OOXML files are not working

class tdf89383(UITestCase):
   def test_tdf89383_DOCX(self):
        with self.ui_test.load_file(get_url_for_data_file("writeprotection.docx")):
            document = self.ui_test.get_component()

            # Without the fix in place, this test would have failed with
            # AssertionError: False is not true
            self.assertTrue(document.isReadonly())

            with self.ui_test.execute_dialog_through_command(".uno:EditDoc") as xDialog:
                xPassword = xDialog.getChild("newpassEntry")
                xPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "a"}))

            self.assertFalse(document.isReadonly())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
