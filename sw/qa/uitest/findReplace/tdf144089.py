# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text, get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf144089(UITestCase):

    def test_tdf144089(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, "This is a test")
            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "10", "END_POS": "14"}))

            self.assertEqual("test", document.CurrentSelection[0].String)

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                xSearchterm = xDialog.getChild("searchterm")
                xSearchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xSearchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xSearchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"^."}))

                xReplaceterm = xDialog.getChild("replaceterm")
                xReplaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"A"}))

                xSelectionOnly = xDialog.getChild("selection")
                xSelectionOnly.executeAction("CLICK", tuple())

                # Deselect similarity before selecting regex
                xSimilarity = xDialog.getChild("similarity")
                if get_state_as_dict(xSimilarity)['Selected'] == 'true':
                    xSimilarity.executeAction("CLICK", tuple())

                xRegexp = xDialog.getChild("regexp")
                xRegexp.executeAction("CLICK", tuple())
                self.assertEqual("true", get_state_as_dict(xRegexp)['Selected'])

                replaceall = xDialog.getChild("replaceall")
                replaceall.executeAction("CLICK", tuple())

                # Deselect regex button, otherwise it might affect other tests
                xRegexp.executeAction("CLICK", tuple())
                self.assertEqual("false", get_state_as_dict(xRegexp)['Selected'])

            # Without the fix in place, this test would have failed with
            # AssertionError: 'This is a test' != 'This is a AAAA'
            self.assertEqual("This is a test", document.Text.String)

            self.assertEqual("test", document.CurrentSelection[0].String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
