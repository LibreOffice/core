# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

#Bug 124675 - CRASH: after moving the content down and undoing

class tdf124675(UITestCase):
   def test_tdf124675_crash_moving_SwTextFrame_previous_page(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf124675.docx")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.assertEqual(writer_doc.CurrentController.PageCount, 2)
            self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "1")

            for i in range(52):
                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

            self.assertEqual(writer_doc.CurrentController.PageCount, 4)
            self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")

            for i in range(52):
                self.xUITest.executeCommand(".uno:Undo")

            self.assertEqual(writer_doc.CurrentController.PageCount, 2)
            self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "1")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
