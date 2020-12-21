# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf138762(UITestCase):

    def test_tdf138762(self):

        self.ui_test.create_doc_in_start_center("writer")

        # Assuming sidebar is open (decks are closed)

        # Create a chart
        self.xUITest.executeCommand(".uno:InsertObjectChart")

        # Click away, or rather, press Esc twice to achieve the same
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xChartMain.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))
        xChartMain.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))

        # Open sidebar
        xWriterDoc = self.xUITest.getTopFocusWindow()
        button = xWriterDoc.getChild("button")
        # Without the fix in place, this test would have crashed here
        button.executeAction("CLICK", mkPropertyValues({"POS": "1"}))

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: