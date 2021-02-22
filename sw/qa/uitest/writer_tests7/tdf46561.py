# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import type_text
import importlib
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf46561(UITestCase):
    def check_header_texts(self, master="", first="", left="", right=""):
        # Get the current header style and its text contents
        xPageStyle = self.document.getStyleFamilies().getByIndex(2)
        xHeaderText = xPageStyle.getByIndex(0).HeaderText.String
        xHeaderTextFirst = xPageStyle.getByIndex(0).HeaderTextFirst.String
        xHeaderTextLeft = xPageStyle.getByIndex(0).HeaderTextLeft.String
        xHeaderTextRight = xPageStyle.getByIndex(0).HeaderTextRight.String

        # Check the current values
        self.assertEqual(master, xHeaderText)
        self.assertEqual(first, xHeaderTextFirst)
        self.assertEqual(left, xHeaderTextLeft)
        self.assertEqual(right, xHeaderTextRight)

    def test_tdf46561(self):
        self.ui_test.load_file(get_url_for_data_file("tdf46561.odt"))
        self.document = self.ui_test.get_component()
        self.check_header_texts(master="right", first="1st", left="left", right="right")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        xWriterEdit.executeAction("GOTO", mkPropertyValues({"PAGE": "2"}))
        self.xUITest.executeCommand(".uno:JumpToHeader")

        # Switch "same left and right page headers" on and off a few times
        for _ in range(4):
            self.ui_test.execute_dialog_through_command(".uno:PageDialog")
            PageDialog = self.xUITest.getTopFocusWindow();

            xTabs = PageDialog.getChild("tabcontrol")
            select_pos(xTabs, "4")

            Button = xTabs.getChild('checkSameLR')
            Button.executeAction("CLICK",tuple())
            ok = PageDialog.getChild("ok")
            self.ui_test.close_dialog_through_button(ok)

        # We should be back to the starting state after 2*k on/off changes
        self.check_header_texts(master="right", first="1st", left="left", right="right")

        # Enter some additional text in the left page header
        type_text(xWriterEdit, "XXXX")
        self.check_header_texts(master="right", first="1st", left="XXXXleft", right="right")

        # Now go back one change (before entering "XXXX")
        self.xUITest.executeCommand(".uno:Undo")
        self.check_header_texts(master="right", first="1st", left="left", right="right")

        # Undo the fourth change
        self.xUITest.executeCommand(".uno:Undo")
        self.check_header_texts(master="right", first="1st", left="right", right="right")

        # Undo the third change
        self.xUITest.executeCommand(".uno:Undo")
        self.check_header_texts(master="right", first="1st", left="left", right="right")

        # Undo the second change
        self.xUITest.executeCommand(".uno:Undo")
        self.check_header_texts(master="right", first="1st", left="right", right="right")

        # Undo the first change
        self.xUITest.executeCommand(".uno:Undo")
        self.check_header_texts(master="right", first="1st", left="left", right="right")

        # Redo the first change
        self.xUITest.executeCommand(".uno:Redo")
        self.check_header_texts(master="right", first="1st", left="right", right="right")

        # Redo the second change
        self.xUITest.executeCommand(".uno:Redo")
        self.check_header_texts(master="right", first="1st", left="left", right="right")

        # Redo the third change
        self.xUITest.executeCommand(".uno:Redo")
        self.check_header_texts(master="right", first="1st", left="right", right="right")

        # Redo the fourth change
        self.xUITest.executeCommand(".uno:Redo")
        self.check_header_texts(master="right", first="1st", left="left", right="right")

        # Redo the final change
        self.xUITest.executeCommand(".uno:Redo")
        self.check_header_texts(master="right", first="1st", left="XXXXleft", right="right")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
