# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import type_text

class tdf115088(UITestCase):

   def test_tdf115088(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        type_text(xWriterEdit, "1")
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        type_text(xWriterEdit, "1")

        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        self.xUITest.executeCommand(".uno:Cut")   #cut  text
        type_text(xWriterEdit, "test")
        self.xUITest.executeCommand(".uno:GoLeft")
        self.xUITest.executeCommand(".uno:GoLeft")
        self.xUITest.executeCommand(".uno:GoLeft")
        self.xUITest.executeCommand(".uno:GoLeft")
        self.xUITest.executeCommand(".uno:PasteUnformatted")
        self.assertEqual(document.Text.String[0:2].replace('\r', '\n'), "1\n")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:












