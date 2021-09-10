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
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            type_text(xWriterEdit, 'Lorem')
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"F3"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            #generating a big text with ~60k words and several paragraphs
            for i in range(0,8):
                self.xUITest.executeCommand(".uno:SelectAll")
                self.xUITest.executeCommand(".uno:Copy")
                self.xUITest.executeCommand(".uno:Paste")
                self.xUITest.executeCommand(".uno:Paste")
            xWriterEdit.executeAction("GOTO", mkPropertyValues({"PAGE": "1"}))

            #Format->Text operations on small selections (which would generate <~500 redlines), changetracking still working
            self.xUITest.executeCommand(".uno:TrackChanges")
            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "1", "END_POS": "1000"}))
            self.xUITest.executeCommand(".uno:ChangeCaseToTitleCase")
            self.assertTrue(document.Redlines.createEnumeration().hasMoreElements())

            #Removing all the redlines.
            with self.ui_test.execute_modeless_dialog_through_command_guarded(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                xAccBtn = xTrackDlg.getChild("rejectall")
                xAccBtn.executeAction("CLICK", tuple())

            #Without the fix in place, on big selections writer would freeze. Now it ignores change tracking.
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:ChangeCaseToTitleCase")
            self.assertFalse(document.Redlines.createEnumeration().hasMoreElements())

            #The patch has no effects on the Format->Text operations
            self.assertEqual(document.Text.String[0:26], "Lorem Ipsum Dolor Sit Amet")
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:ChangeCaseToUpper")
            self.assertEqual(document.Text.String[0:26], "LOREM IPSUM DOLOR SIT AMET")

# vim: set shiftwidth=4 softtabstop=4 expandtab:

