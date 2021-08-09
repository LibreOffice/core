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

class tdf143574(UITestCase):
    def test_tdf143574(self):
        # load the sample file
        with self.ui_test.load_file(get_url_for_data_file("tdf143574.odt")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            document = self.ui_test.get_component()

            # check the shape type.
            self.assertEqual("com.sun.star.drawing.GroupShape", document.DrawPage.getByIndex(0).ShapeType)

            # select the shape.
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            self.ui_test.wait_until_child_is_available('metricfield')

            # go inside the group
            self.xUITest.executeCommand(".uno:EnterGroup");

            # selct a shape in the group
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

            # At this point the Writer crashed here before the fix.
            self.xUITest.executeCommand(".uno:AddTextBox");

            self.assertEqual(True, document.DrawPage.getByIndex(0).getByIndex(2).TextBox)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
