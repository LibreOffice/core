# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import change_measurement_unit
from libreoffice.uno.propertyvalue import mkPropertyValues
#uitest sw / Columns dialog

class columns(UITestCase):
    def test_columns(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        change_measurement_unit(self, "Centimeter")

        #dialog Columns
        self.ui_test.execute_dialog_through_command(".uno:FormatColumns")
        xDialog = self.xUITest.getTopFocusWindow()

        colsnf = xDialog.getChild("colsnf")
        colsnf.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        colsnf.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        colsnf.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))
        colsnf.executeAction("UP", tuple())
        colsnf.executeAction("DOWN", tuple())
        spacing1mf = xDialog.getChild("spacing1mf")
        spacing1mf.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        spacing1mf.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        spacing1mf.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
        autowidth = xDialog.getChild("autowidth")
        autowidth.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        self.ui_test.execute_dialog_through_command(".uno:FormatColumns")
        xDialog = self.xUITest.getTopFocusWindow()
        colsnf = xDialog.getChild("colsnf")
        spacing1mf = xDialog.getChild("spacing1mf")
        autowidth = xDialog.getChild("autowidth")

        self.assertEqual(get_state_as_dict(colsnf)["Text"], "2")
        self.assertEqual(get_state_as_dict(spacing1mf)["Text"], "1.00 cm")
        self.assertEqual(get_state_as_dict(autowidth)["Selected"], "false")
        xOKBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
