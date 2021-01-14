# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text
from uitest.uihelper.common import change_measurement_unit
from libreoffice.uno.propertyvalue import mkPropertyValues

#Writer Table Properties

class tdf134881(UITestCase):
    def test_tdf134881(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf134881_colProportionalAdjust.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        change_measurement_unit(self, "Centimeter")

        #dialog Table Properties
        self.ui_test.execute_dialog_through_command(".uno:TableDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "2") #columns

        adaptcolumns = xDialog.getChild("adaptcolumns")
        width1 = xDialog.getChild("width1")
        width2 = xDialog.getChild("width2")
        # Get the baseline. The first column is much shorter than the second.
        self.assertEqual((get_state_as_dict(width1)["Text"])[0:3], "3.0")
        self.assertEqual((get_state_as_dict(width2)["Text"])[0:4], "14.0")
        # Set proportional adjust - so all columns are affected.
        adaptcolumns.executeAction("CLICK", tuple())
        # Cut the second column in half.
        width2.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        width2.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        width2.executeAction("TYPE", mkPropertyValues({"TEXT":"7"}))
        width2.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

        # The first column should now be half it's original value, not zero (not 3 - 7)
        self.assertEqual((get_state_as_dict(width1)["Text"])[0:2], "1.") #1.5

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
