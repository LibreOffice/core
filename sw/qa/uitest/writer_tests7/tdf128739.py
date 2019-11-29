# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
   return get_srcdir_url() + "/sw/qa/uitest/writer_tests7/data/" + file_name

class tdf128739 (UITestCase):

    def test_mark_cut_paste_undo_with_markers_tdf128739 (self):

        self.ui_test.load_file(get_url_for_data_file("tdf128739.docx"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Cut")
        self.xUITest.executeCommand(".uno:Paste")
        self.xUITest.executeCommand(".uno:Undo")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
