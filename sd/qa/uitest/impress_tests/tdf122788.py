# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf129346(UITestCase):

   def test_run(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            xDoc = self.xUITest.getTopFocusWindow()
            xEdit = xDoc.getChild("impress_win")
            # Type into the text box
            xEdit.executeAction("TYPE", mkPropertyValues({"TEXT":"First first first"}))

            self.assertEqual("First first first", document.DrawPages[0].getByIndex(0).String)

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"First"}))

                replaceterm = xDialog.getChild("replaceterm")
                replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"Replace"}))

                # hit replace button 2 times
                replace = xDialog.getChild("replace")
                replace.executeAction("CLICK", tuple())
                replace.executeAction("CLICK", tuple())

                self.assertEqual("Replace first first", document.DrawPages[0].getByIndex(0).String)

                replaceall = xDialog.getChild("replaceall")
                replaceall.executeAction("CLICK", tuple())

                # Without the fix in place, this test would have failed with
                # AssertionError: 'Replace Replace Replace' != 'Replace first first'
                self.assertEqual("Replace Replace Replace", document.DrawPages[0].getByIndex(0).String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
