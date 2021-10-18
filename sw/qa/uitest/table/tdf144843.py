# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf144843(UITestCase):

   def test_tdf144843(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            self.xUITest.executeCommand(".uno:InsertTable?Columns:short=2&Rows:short=2")

            self.xUITest.executeCommand(".uno:SelectAll")

            self.assertEqual(-1, document.TextTables[0].getCellByName("A1").BackColor)

            # Without the fix in place, this test would have crashed here
            with self.ui_test.execute_dialog_through_command(".uno:BackgroundDialog") as xDialog:
                btncolor = xDialog.getChild("btncolor")
                btncolor.executeAction("CLICK", tuple())

                hex_custom = xDialog.getChild("hex_custom")
                hex_custom.executeAction("TYPE", mkPropertyValues({"TEXT":"101010"}))

            self.assertEqual("0x101010", hex(document.TextTables[0].getCellByName("A1").BackColor))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
