# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

class Tdf145062(UITestCase):

    def test_tdf145062(self):

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:

            with self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField", close_button="") as xDialog:
                xTab = xDialog.getChild("tabcontrol")
                select_pos(xTab, "2")

                xType = xDialog.getChild("type-func")
                xType.getChild('2').executeAction("SELECT", tuple())
                self.assertEqual("Input field", get_state_as_dict(xType)['SelectEntryText'])

                xOkBtn = xDialog.getChild("ok")
                with self.ui_test.execute_blocking_action(xOkBtn.executeAction, args=('CLICK', ())) as xFieldDialog:
                    xText = xFieldDialog.getChild("text")
                    xText.executeAction("TYPE", mkPropertyValues({"TEXT": "AAA"}))

                # move cursor to the middle of the field
                self.xUITest.executeCommand(".uno:GoLeft")
                self.xUITest.executeCommand(".uno:GoLeft")

                xType.getChild('6').executeAction("SELECT", tuple())
                self.assertEqual("Hidden text", get_state_as_dict(xType)['SelectEntryText'])

                # try to insert a hidden text a few times
                # Without the fix in place, this test would have crashed here
                for i in range(10):
                    xOkBtn.executeAction("CLICK", ())

                # close_dialog_through_button should be used here but it hangs and
                # the reason seems to be that interacting with the document while
                # the dialog is open doesn't play well together in the uitests
                # As a workaround, leave the dialog open, close the document,
                # open a new one and then, close the dialog.
                # If the dialog is not closed, it will be displayed in other tests

            textfields = writer_doc.getTextFields()
            textfield = textfields.createEnumeration().nextElement()
            self.assertTrue(textfield.supportsService("com.sun.star.text.TextField.Input"))
            self.assertEqual('AAA', textfield.Content)

        with self.ui_test.load_empty_file("writer"):
            xDialog = self.xUITest.getTopFocusWindow()
            xCancelBtn = xDialog.getChild("cancel")
            self.ui_test.close_dialog_through_button(xCancelBtn)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
