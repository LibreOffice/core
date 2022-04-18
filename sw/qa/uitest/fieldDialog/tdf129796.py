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

class Tdf129796(UITestCase):

    def test_tdf129796(self):

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:

            with self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField", close_button="cancel") as xDialog:
                xTab = xDialog.getChild("tabcontrol")
                select_pos(xTab, "4")

                xType = xDialog.getChild("type-var")
                xType.getChild('8').executeAction("SELECT", tuple())
                self.assertEqual("User Field", get_state_as_dict(xType)['SelectEntryText'])

                xNumFormat = xDialog.getChild("numformat-var")
                xNumFormat.getChild('0').executeAction("SELECT", tuple())
                self.assertEqual("Text", get_state_as_dict(xNumFormat)['SelectEntryText'])

                xName = xDialog.getChild("name-var")
                xName.executeAction("TYPE", mkPropertyValues({"TEXT": "MyField"}))

                xValue = xDialog.getChild("value-var")
                xValue.executeAction("TYPE", mkPropertyValues({"TEXT": "abc"}))

                xApplyBtn = xDialog.getChild("apply")
                xApplyBtn.executeAction("CLICK", ())

                xType.getChild('4').executeAction("SELECT", tuple())
                self.assertEqual("Input field", get_state_as_dict(xType)['SelectEntryText'])

                self.assertEqual("MyField", get_state_as_dict(xDialog.getChild("select-var"))["SelectEntryText"])

                xValue.executeAction("TYPE", mkPropertyValues({"TEXT": "ref"}))

                xOkBtn = xDialog.getChild("ok")
                with self.ui_test.execute_blocking_action(xOkBtn.executeAction, args=('CLICK', ())) as xFieldDialog:
                    xName = xFieldDialog.getChild("name")
                    self.assertEqual("ref", get_state_as_dict(xName)['Text'])
                    xText = xFieldDialog.getChild("text")
                    self.assertEqual("abc", get_state_as_dict(xText)['Text'])

            textfields = writer_doc.getTextFields()
            textfield = textfields.createEnumeration().nextElement()
            self.assertEqual('MyField', textfield.Content)
            self.assertTrue(textfield.supportsService("com.sun.star.text.TextField.InputUser"))

            # Move the cursor to the field
            self.xUITest.executeCommand(".uno:GoLeft")

            with self.ui_test.execute_dialog_through_command(".uno:FieldDialog") as xDialog:
                xType = xDialog.getChild("type-var")
                xNumFormat = xDialog.getChild("numformat-var")
                xValue = xDialog.getChild("value-var")
                xName = xDialog.getChild("name-var")

                # Without the fix in place, this test would have crashed here
                self.assertEqual("ref", get_state_as_dict(xValue)['Text'])
                self.assertEqual("Input field", get_state_as_dict(xType)['SelectEntryText'])
                self.assertEqual("General", get_state_as_dict(xNumFormat)['SelectEntryText'])
                self.assertEqual("MyField", get_state_as_dict(xName)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
