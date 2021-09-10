#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf144089(UITestCase):

    def test_tdf144089(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, "This is a test")
            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "10", "END_POS": "14"}))

            self.assertEqual("test", document.CurrentSelection[0].String)

            with self.ui_test.execute_modeless_dialog_through_command_guarded(".uno:SearchDialog", close_button="close") as xDialog:
                xSearchterm = xDialog.getChild("searchterm")
                xSearchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xSearchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xSearchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"^."}))

                xReplaceterm = xDialog.getChild("replaceterm")
                xReplaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"A"}))

                xSelectionOnly = xDialog.getChild("selection")
                xSelectionOnly.executeAction("CLICK", tuple())

                xRegexp = xDialog.getChild("regexp")
                xRegexp.executeAction("CLICK", tuple())

                replaceall = xDialog.getChild("replaceall")
                replaceall.executeAction("CLICK", tuple())

            # Without the fix in place, this test would have failed with
            # AssertionError: 'This is a test' != 'This is a AAAA'
            self.assertEqual("This is a test", document.Text.String)

            self.assertEqual("test", document.CurrentSelection[0].String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
