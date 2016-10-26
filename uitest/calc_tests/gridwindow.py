# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.uihelper.calc import enter_text_to_cell

from uitest.framework import UITestCase

class GridWindowTest(UITestCase):

    def test_input(self):

        self.ui_test.create_doc_in_start_center("calc")
        xTopWindow = self.xUITest.getTopFocusWindow()

        xGridWindow = xTopWindow.getChild("grid_window")

        enter_text_to_cell(xGridWindow, "C3", "=A1")
        enter_text_to_cell(xGridWindow, "A1", "2")

        self.ui_test.close_doc()

    def test_special_keys(self):

        self.ui_test.create_doc_in_start_center("calc")
        xTopWindow = self.xUITest.getTopFocusWindow()

        xGridWindow = xTopWindow.getChild("grid_window")

        selectProps = mkPropertyValues({"CELL": "C3"})
        xGridWindow.executeAction("SELECT", selectProps)

        typeProps = mkPropertyValues({"KEYCODE": "CTRL+DOWN"})
        xGridWindow.executeAction("TYPE", typeProps)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
