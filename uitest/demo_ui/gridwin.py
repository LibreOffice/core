# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase

class GridWinTest(UITestCase):

    def test_select_cell(self):

        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")

            selectProps = mkPropertyValues({"CELL": "B10"})
            xGridWindow.executeAction("SELECT", selectProps)


    def test_select_range(self):

        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")

            selectProps = mkPropertyValues({"RANGE": "B10:C20"})
            xGridWindow.executeAction("SELECT", selectProps)


    def test_extend_range(self):

        with self.ui_test.create_doc_in_start_center("calc"):
            xTopWindow = self.xUITest.getTopFocusWindow()

            xGridWindow = xTopWindow.getChild("grid_window")

            selectProps = mkPropertyValues({"RANGE": "B10:C20"})
            xGridWindow.executeAction("SELECT", selectProps)

            select2Props = mkPropertyValues({"RANGE": "D3:F5", "EXTEND": "true"})
            xGridWindow.executeAction("SELECT", select2Props)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
