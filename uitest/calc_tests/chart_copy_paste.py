# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from uitest.path import get_srcdir_url

import time
import unittest
import os

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/uitest/calc_tests/data/" + file_name

class CalcChartCopyPaste(UITestCase):

    def test_copy_paste_crash(self):
        # print( "PYTHONPATH = %s" % os.environ['PYTHONPATH'] )
        # print( "URE_BOOTSTRAP = %s" % os.environ['URE_BOOTSTRAP'] )
        doc = self.ui_test.load_file(get_url_for_data_file("tdf108881.ods"))
        #xGridWindow = self.xUITest.getTopFocusWindow().getChild("grid_window")
        #xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))
        #xGridWindow.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        #print("Selected chart")
        #time.sleep(10)
        #xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+X"}))
        #print("Cut chart")
        #time.sleep(10)
        self.ui_test.close_doc()
        self.ui_test.create_doc_in_start_center("calc")
        xGridWindow = self.xUITest.getTopFocusWindow().getChild("grid_window")
        #xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+V"}))
        #xGridWindow.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        self.ui_test.close_doc()
