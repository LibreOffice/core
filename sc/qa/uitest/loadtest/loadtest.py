# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sc/qa/uitest/loadtest/data/" + file_name

class LoadDocTest(UITestCase):

    def test_tdf111974(self):
        # Will crash without the fix
        # Regular unit tests wont be able to load this file due to
        # void GraphicManager::ImplUnregisterObj(const GraphicObject&): Assertion `mnUsedSize >= rObj.maGraphic.GetSizeBytes()' failed
        # hence had to use a UI test for loading this file.
        doc = self.ui_test.load_file(get_url_for_data_file("tdf111974.xlsm"))
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
