# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict


class watermark(UITestCase):

   def test_insert_watermark(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:Watermark")
        xDialog = self.xUITest.getTopFocusWindow()
        xTextInput = xDialog.getChild("TextInput")
        xAngle = xDialog.getChild("Angle")
        xTransparency = xDialog.getChild("Transparency")

        xTextInput.executeAction("TYPE", mkPropertyValues({"TEXT":"Watermark"}))
        xAngle.executeAction("UP", tuple())
        xTransparency.executeAction("UP", tuple())

        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.execute_dialog_through_command(".uno:Watermark")
        xDialog = self.xUITest.getTopFocusWindow()
        xTextInput = xDialog.getChild("TextInput")
        xAngle = xDialog.getChild("Angle")
        xTransparency = xDialog.getChild("Transparency")

        self.assertEqual(get_state_as_dict(xTextInput)["Text"], "Watermark")
        self.assertEqual(get_state_as_dict(xAngle)["Text"], "90°")
        self.assertEqual(get_state_as_dict(xTransparency)["Text"], "51%")

        xCancBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancBtn)

        self.ui_test.execute_dialog_through_command(".uno:Watermark")
        xDialog = self.xUITest.getTopFocusWindow()
        xTextInput = xDialog.getChild("TextInput")
        xTextInput.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xTextInput.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.execute_dialog_through_command(".uno:Watermark")
        xDialog = self.xUITest.getTopFocusWindow()
        xTextInput = xDialog.getChild("TextInput")

        self.assertEqual(get_state_as_dict(xTextInput)["Text"], "")

        xCancBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
