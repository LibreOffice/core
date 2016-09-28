# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

import os
import pathlib

def get_data_dir():
    current_dir = os.path.dirname(os.path.realpath(__file__))
    return os.path.join(current_dir, "data")

def get_url_for_data_file(file_name):
    path = os.path.join(get_data_dir(), file_name)
    return pathlib.Path(path).as_uri()

class ConditionalFormatDlgTest(UITestCase):

    def test_simple_open_manager(self):

        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf96453.ods"))
        print(dir(calc_doc))

        self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog")

        xCondFormatMgr = self.xUITest.getTopFocusWindow()

        xCancelBtn = xCondFormatMgr.getChild("cancel")
        xCancelBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
