#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from libreoffice.calc.document import get_column
from uitest.path import get_srcdir_url

import time

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sc/qa/uitest/calc_tests/data/" + file_name 

class tdf81351(UITestCase):

    def test_tdf81351(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf81351.ods"))
        xTopWindow = self.xUITest.getTopFocusWindow()

        gridwin = xTopWindow.getChild("grid_window")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:F5"}))


        self.xUITest.executeCommand(".uno:SortAscending")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B7"}))

        self.xUITest.executeCommand(".uno:Undo")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B7"}))

    def test_tdf81351_comment4(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf81351.ods"))
        xTopWindow = self.xUITest.getTopFocusWindow()

        gridwin = xTopWindow.getChild("grid_window")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:F5"}))


        self.xUITest.executeCommand(".uno:SortAscending")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B7"}))

        self.xUITest.executeCommand(".uno:Undo")

        self.xUITest.executeCommand(".uno:Redo")

    def test_sort_descending(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf81351.ods"))
        xTopWindow = self.xUITest.getTopFocusWindow()

        gridwin = xTopWindow.getChild("grid_window")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:F5"}))


        self.xUITest.executeCommand(".uno:SortDescending")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B7"}))

        self.xUITest.executeCommand(".uno:Undo")

        self.xUITest.executeCommand(".uno:Redo")

        self.ui_test.close_doc()
