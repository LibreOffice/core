# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
import org.libreoffice.unotest
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf142847(UITestCase):
    def test_tdf142847(self):
        # load the sample file
        with self.ui_test.load_file(get_url_for_data_file("tdf142847.fodt")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            document = self.ui_test.get_component()

            # get the shape
            shape = document.DrawPage.getByIndex(0)

            # select the shape.
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            self.ui_test.wait_until_child_is_available('metricfield')

            # set the wrap spacing of the shape
            with self.ui_test.execute_dialog_through_command(".uno:TextWrap") as wrap_dialog:
                for i in range(90):
                    wrap_dialog.getChild('left').executeAction("UP", tuple())

            # get the textbox
            frame = shape.getText()

            # without the fix, this will fail.
            # the textbox has fallen apart.
            self.assertGreater(frame.getPropertyValue("HoriOrientPosition"), shape.getPropertyValue("LeftMargin"))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
