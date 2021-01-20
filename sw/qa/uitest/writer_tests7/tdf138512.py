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

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf138512(UITestCase):

    def test_tdf138512(self):

        self.ui_test.load_file(get_url_for_data_file("tdf138512.odt"))

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()

        self.assertEqual( AS_CHARACTER, document.DrawPage.getByIndex(0).AnchorType)

        self.xUITest.executeCommand(".uno:JumpToNextFrame")
        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        self.ui_test.wait_until_child_is_available(xWriterEdit, 'metricfield')

        self.xUITest.executeCommand(".uno:AlignBottom")

        self.xUITest.executeCommand(".uno:AlignTop")

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
