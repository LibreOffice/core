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
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf150457(UITestCase):

   def test_delete_footnotes(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, "a")
            self.xUITest.executeCommand(".uno:InsertFootnote")
            type_text(xWriterEdit, "abc")
            self.assertEqual(document.Footnotes[0].String, "abc")
            self.assertEqual(document.Footnotes.getCount(), 1)

            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "PAGEUP"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            type_text(xWriterEdit, "d")

            self.xUITest.executeCommand(".uno:InsertFootnote")
            type_text(xWriterEdit, "def")
            self.assertEqual(document.Footnotes[1].String, "def")
            self.assertEqual(document.Footnotes.getCount(), 2)

            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "HOME"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+DOWN"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DELETE"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "PAGEUP"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DELETE"}))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
