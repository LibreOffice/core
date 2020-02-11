# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf129346(UITestCase):

   def test_run(self):
        self.ui_test.create_doc_in_start_center("impress")
        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.xUITest.executeCommand(".uno:DiaMode")
        xDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        self.assertEqual(document.CurrentController.getCurrentPage().Number, 1)
        self.xUITest.executeCommand(".uno:InsertPage")

        self.assertEqual(document.CurrentController.getCurrentPage().Number, 2)
        self.xUITest.executeCommand(".uno:Undo")

        #Without the accompanying fix in place, getCurrentPage() is None
        self.assertEqual(document.CurrentController.getCurrentPage().Number, 1)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
