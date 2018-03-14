# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
import time
from uitest.debug import sleep

class insertFootnote(UITestCase):

   def test_insert_footnote(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:InsertFootnote")

        type_text(xWriterEdit, "LibreOffice")
        self.assertEqual(document.Footnotes.getByIndex(0).String, "LibreOffice")
        self.assertEqual(document.Footnotes.getCount(), 1)

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Footnotes.getByIndex(0).String, "")
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Footnotes.getCount(), 0)
        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(document.Footnotes.getByIndex(0).String, "")
        self.assertEqual(document.Footnotes.getCount(), 1)
        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(document.Footnotes.getByIndex(0).String, "LibreOffice")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
