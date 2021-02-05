# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text

class tdf137513(UITestCase):

   def test_tdf137513(self):
        self.ui_test.create_doc_in_start_center("writer")

        self.xUITest.executeCommand(".uno:InsertTable?Columns:short=2&Rows:short=2")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        type_text(xWriterEdit, "test")

        self.xUITest.executeCommand(".uno:SelectAll")

        self.xUITest.executeCommand(".uno:Bold")

        self.xUITest.executeCommand(".uno:Sidebar")
        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "InspectorTextPanel"}))

        xListBox = xWriterEdit.getChild('listbox_fonts')

        # The cursor is on text inside the table with direct formatting
        self.assertEqual(2, len(xListBox.getChild('0').getChildren()))
        self.assertEqual("Default Paragraph Style", get_state_as_dict(xListBox.getChild('0').getChild('0'))['Text'])
        self.assertEqual("Table Contents", get_state_as_dict(xListBox.getChild('0').getChild('1'))['Text'])
        self.assertEqual(136, len(xListBox.getChild('0').getChild('0').getChildren()))

        xTableContent = xListBox.getChild('0').getChild('1')
        self.assertEqual(5, len(xTableContent.getChildren()))
        self.assertEqual("Follow Style\tTable Contents", get_state_as_dict(xTableContent.getChild('0'))['Text'])
        self.assertEqual("Para Line Number Count\tFalse", get_state_as_dict(xTableContent.getChild('1'))['Text'])
        self.assertEqual("Para Line Number Start Value\t0", get_state_as_dict(xTableContent.getChild('2'))['Text'])
        self.assertEqual("Para Orphans\t0", get_state_as_dict(xTableContent.getChild('3'))['Text'])
        self.assertEqual("Para Widows\t0", get_state_as_dict(xTableContent.getChild('4'))['Text'])

        xParDirFormatting = xListBox.getChild('1')

        # Without the fix in place, this test would have failed here with
        # AssertionError: 3 != 0
        self.assertEqual(3, len(xParDirFormatting.getChildren()))
        self.assertEqual("Char Weight\tBold", get_state_as_dict(xParDirFormatting.getChild('0'))['Text'])
        self.assertEqual("Char Weight Asian\tBold", get_state_as_dict(xParDirFormatting.getChild('1'))['Text'])
        self.assertEqual("Char Weight Complex\tBold", get_state_as_dict(xParDirFormatting.getChild('2'))['Text'])
        self.assertEqual(0, len(xListBox.getChild('2').getChildren()))
        self.assertEqual(0, len(xListBox.getChild('3').getChildren()))

        self.xUITest.executeCommand(".uno:Sidebar")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
