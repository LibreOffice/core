# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from com.sun.star.text.TextContentAnchorType import AT_PAGE, AT_PARAGRAPH

class tdf137802(UITestCase):

    def test_tdf137802(self):

        self.ui_test.load_file(get_url_for_data_file("tdf137802.odt"))

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()

        self.assertEqual(document.DrawPage.getCount(), 2)
        self.assertEqual(AT_PARAGRAPH, document.DrawPage.getByIndex(0).AnchorType)

        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        self.ui_test.wait_until_child_is_available(xWriterEdit, 'metricfield')

        self.ui_test.execute_dialog_through_command(".uno:TransformDialog")

        xDialog = self.xUITest.getTopFocusWindow()

        xDialog.getChild('topage').executeAction("CLICK", tuple())

        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.assertEqual(AT_PAGE, document.DrawPage.getByIndex(0).AnchorType)

        self.assertEqual(document.DrawPage.getCount(), 2)

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        # When shape 1 is selected, esc key doesn't put the focus back to the document
        # because the shape has a textbox. Move the focus to another shape with tab key
        # and then use escape
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))

        # Wait until the shape is deselected and the cursor is on the document
        self.ui_test.wait_until_child_is_available(xWriterEdit, 'FontNameBox')

        # Delete the second paragraph. Shape 2 is anchored to this paragraph
        # so it should be deleted
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))

        self.assertEqual(document.DrawPage.getCount(), 1)

        self.xUITest.executeCommand(".uno:JumpToNextFrame")
        self.xUITest.executeCommand(".uno:Delete")

        self.assertEqual(document.DrawPage.getCount(), 0)

        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(document.DrawPage.getCount(), 1)

        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(document.DrawPage.getCount(), 2)

        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(AT_PARAGRAPH, document.DrawPage.getByIndex(0).AnchorType)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
