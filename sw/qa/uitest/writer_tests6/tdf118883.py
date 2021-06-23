# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class Tdf118883(UITestCase):

   def test_tdf118883(self):
        self.ui_test.create_doc_in_start_center("writer")

        # Insert shape with Ctrl key
        xArgs = mkPropertyValues({"KeyModifier": 8192})
        self.xUITest.executeCommandWithParameters(".uno:BasicShapes.rectangle", xArgs)

        writer_document = self.ui_test.get_component()
        self.assertEqual(1, writer_document.DrawPage.getCount())

        self.xUITest.executeCommand(".uno:Copy")

        self.ui_test.execute_dialog_through_command(".uno:CloseDoc")
        xDialog = self.xUITest.getTopFocusWindow()
        xDiscardBtn = xDialog.getChild("discard")
        self.ui_test.close_dialog_through_button(xDiscardBtn)

        calc_doc = self.ui_test.create_doc_in_start_center("calc")

        self.xUITest.executeCommand(".uno:Paste")

        calc_document = self.ui_test.get_component()

        # Without the fix in place, this test would have failed with
        # AssertionError: 1 != 0
        self.assertEqual(1, calc_document.DrawPages[0].getCount())

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
