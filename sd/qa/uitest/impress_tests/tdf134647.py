# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sd/qa/uitest/impress_tests/data/" + file_name

class tdf134647(UITestCase):

    def test_tdf134647(self):
        impress_doc = self.ui_test.load_file(get_url_for_data_file("tdf134647.pptx"))

        #Without the fix in place, it would have crashed here
        self.xUITest.executeCommand(".uno:Presentation")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

