# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf138271(UITestCase):

    def test_tdf138271(self):

        self.ui_test.load_file(get_url_for_data_file("numerical-form.odt"))

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:JumpToNextFrame")
        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties")

        self.ui_test.wait_until_child_in_dialog_is_available('formattedcontrol')
        xDialog = self.xUITest.getTopFocusWindow()
        xFormattedControl = xDialog.getChild("formattedcontrol")
        xTextField = xDialog.getChild("textfield")

        xFormattedControl.executeAction("UP", tuple())

        self.assertEqual('-99', get_state_as_dict(xFormattedControl)['Value'])

        xTextField.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
        xTextField.executeAction("TYPE", mkPropertyValues({"TEXT": "Form 1"}))

        self.assertEqual('Form 1', get_state_as_dict(xTextField)['Text'])

        # Select the other form to refresh the dialog
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

        self.ui_test.wait_until_child_in_dialog_is_available('textview')
        xDialog = self.xUITest.getTopFocusWindow()
        xTextView = xDialog.getChild("textview")
        self.assertEqual('DefaultText', get_state_as_dict(xTextView)['Text'])

        # Select the numerical form again
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

        self.ui_test.wait_until_child_in_dialog_is_available('formattedcontrol')
        xDialog = self.xUITest.getTopFocusWindow()
        xFormattedControl = xDialog.getChild("formattedcontrol")

        # Without the fix in place, this test would have failed with
        # AssertionError: '-99' != '-100'
        self.assertEqual('-99', get_state_as_dict(xFormattedControl)['Value'])

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
