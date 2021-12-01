# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text

class PasteSpecial(UITestCase):

   def test_pasteSpecial(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, "test")

            for i in range(5):
                self.xUITest.executeCommand(".uno:SelectAll")
                self.xUITest.executeCommand(".uno:Copy")

                with self.ui_test.execute_dialog_through_command(".uno:PasteSpecial") as xDialog:

                    xList = xDialog.getChild('list')
                    xChild = xList.getChild(str(i))

                    xChild.executeAction("SELECT", tuple())


                self.xUITest.executeCommand(".uno:Undo")

                self.assertEqual(document.Text.String, "test")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
