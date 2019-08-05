# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 125261 - Applying autocorrect to text with URLs causes Writer to hang

class tdf125261(UITestCase):
   def test_tdf125261_crash_autocorrect_text_with_URL(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf125261.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        #apply autocorrect StartAutoCorrect
        self.xUITest.executeCommand(".uno:AutoFormatApply")

        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(document.Text.String[0:4], "This")
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
