#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/uitest/writer_tests/data/" + file_name

class tdf104649(UITestCase):

    def test_delete_and_undo(self):

        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf104649.docx"))

        document = self.ui_test.get_component()

        self.assertEqual(document.Text.String[0:4], "Test")

        selection = self.xUITest.executeCommand(".uno:SelectAll")

        self.xUITest.executeCommand(".uno:Delete")

        self.assertEqual(document.Text.String, "")

        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(document.Text.String[0:4], "Test")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
