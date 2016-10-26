# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase

class GridWinTest(UITestCase):

    def test_select_object(self):

        self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")

        selectProps = mkPropertyValues({"CELL": "B10"})
        xGridWindow.executeAction("SELECT", selectProps)

        xGridWindow.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))

        xGridWindow.executeAction("ACTIVATE", tuple())

        xGridWindow.executeAction("DESELECT", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
