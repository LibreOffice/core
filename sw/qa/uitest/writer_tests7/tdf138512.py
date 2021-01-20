# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from com.sun.star.text.TextContentAnchorType import  AS_CHARACTER
import org.libreoffice.unotest
import pathlib
import time


def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf138512(UITestCase):

    def test_tdf138512(self):

        # Open the bugdoc
        self.ui_test.load_file(get_url_for_data_file("tdf138512.odt"))

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()

        # Check that the anchortype is as_char
        self.assertEqual( AS_CHARACTER, document.DrawPage.getByIndex(0).AnchorType)

        # Select the textbox
        self.xUITest.executeCommand(".uno:JumpToNextFrame")
        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        # Align the shape to bottom
        time.sleep(1)
        self.xUITest.executeCommand(".uno:AlignDown")
        time.sleep(1)

        # Align the shape up
        self.xUITest.executeCommand(".uno:AlignUp")
        time.sleep(1)

        # Deselect the shape
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))

        # Without the fix this will crash at this point with gtk vcl backend
        time.sleep(1)
        self.assertEqual( AS_CHARACTER, document.DrawPage.getByIndex(0).AnchorType)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
