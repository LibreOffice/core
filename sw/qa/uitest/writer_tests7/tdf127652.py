# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.path import get_srcdir_url
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

def get_url_for_data_file(file_name):
   return get_srcdir_url() + "/sw/qa/uitest/writer_tests7/data/" + file_name

class tdf127652 (UITestCase):

    def test_mark_delete_undo_delete_tdf127652 (self):

        self.ui_test.load_file(get_url_for_data_file("tdf127652.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        # go to the start of page 4
        xWriterEdit.executeAction("GOTO", mkPropertyValues({"PAGE": "4"}))
        xWriterEdit.executeAction("CLICK", tuple())

        # mark a section that overlaps multiple pages
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+UP"}))

        # delete the marked section
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DELETE"}))

        # go to the start of page 4
        xWriterEdit.executeAction("GOTO", mkPropertyValues({"PAGE": "4"}))
        xWriterEdit.executeAction("CLICK", tuple())

        # move up to page 3
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))

        # check that we are on the third page
        # in the bug one issue was that the cursor was places in the wrong place
        # moving up to the previous page would not work any more
        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "3")
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
