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

class tdf53023(UITestCase):

    def test_tdf53023_autotext_new_paragraph(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            # Insert a test text and select it
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            type_text(xWriterEdit, "Test")
            self.xUITest.executeCommand(".uno:SelectAll")

            # Create auto text entry from the previously inserted text and insert it
            with self.ui_test.execute_dialog_through_command(".uno:EditGlossary") as xEditGlossaryDlg:
                xCategory = xEditGlossaryDlg.getChild("category")
                xMyAutoText = xCategory.getChild("2")
                xMyAutoText.executeAction("SELECT", tuple())
                xAutoTextName = xEditGlossaryDlg.getChild("name")
                type_text(xAutoTextName, "Test")
                xEditMenu = xEditGlossaryDlg.getChild("autotext")
                xEditMenu.executeAction("OPENFROMLIST", mkPropertyValues({"POS": "1"}))

            # Without the fix in place, this test would have failed with
            # AssertionError: 'TestTest' != 'TestTest\r\n'
            self.assertEqual("TestTest", document.Text.String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
