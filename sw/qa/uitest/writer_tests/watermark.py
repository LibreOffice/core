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


class watermark(UITestCase):

   def test_insert_watermark(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:Watermark") as xDialog:
                xTextInput = xDialog.getChild("TextInput")
                xAngle = xDialog.getChild("Angle")
                xTransparency = xDialog.getChild("Transparency")

                xTextInput.executeAction("TYPE", mkPropertyValues({"TEXT":"Watermark"}))
                xAngle.executeAction("UP", tuple())
                xTransparency.executeAction("UP", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:Watermark", close_button="cancel") as xDialog:
                xTextInput = xDialog.getChild("TextInput")
                xAngle = xDialog.getChild("Angle")
                xTransparency = xDialog.getChild("Transparency")

                self.assertEqual(get_state_as_dict(xTextInput)["Text"], "Watermark")
                self.assertEqual(get_state_as_dict(xAngle)["Text"], "90°")
                self.assertEqual(get_state_as_dict(xTransparency)["Text"], "51%")


            with self.ui_test.execute_dialog_through_command(".uno:Watermark") as xDialog:
                xTextInput = xDialog.getChild("TextInput")
                xTextInput.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xTextInput.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))

            with self.ui_test.execute_dialog_through_command(".uno:Watermark", close_button="cancel") as xDialog:
                xTextInput = xDialog.getChild("TextInput")

                self.assertEqual(get_state_as_dict(xTextInput)["Text"], "")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
