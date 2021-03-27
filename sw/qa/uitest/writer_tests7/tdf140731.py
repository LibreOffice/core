# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import type_text

class tdf140731(UITestCase):
    def test_tdf140731(self):
        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        type_text(xWriterEdit, 'Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.')
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

        #generating a big text with ~35k words and several paragraphs
        for i in range(0,9):
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+a"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+c"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+v"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+v"}))
        xWriterEdit.executeAction("GOTO", mkPropertyValues({"PAGE": "1"}))
        self.xUITest.executeCommand(".uno:TrackChanges")
        #with relatively small selections, changetracking still working
        xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "1", "END_POS": "1000"}))
        self.xUITest.executeCommand(".uno:ChangeCaseToTitleCase")
        document = self.ui_test.get_component()
        self.assertTrue(document.Redlines.createEnumeration().hasMoreElements())

        #removing all the redlines
        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()
        xAccBtn = xTrackDlg.getChild("rejectall")
        xAccBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        #without the fix in place, on big selections writer would freeze. Now it ignores change tracking
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+a"}))
        import time
        time.sleep(2)
        self.xUITest.executeCommand(".uno:ChangeCaseToTitleCase")
        self.assertFalse(document.Redlines.createEnumeration().hasMoreElements())

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:

