# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf136578(UITestCase):

    def test_tdf136578(self):

        self.ui_test.load_file(get_url_for_data_file("tdf136578.odt"))

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        document = self.ui_test.get_component()

        self.assertEqual(document.CurrentController.PageCount, 2)

        self.ui_test.wait_until_child_is_available(xWriterEdit, 'PageBreak')
        xPageBreak = xWriterEdit.getChild('PageBreak')

        xPageBreak.executeAction("DELETE", tuple())

        # Without the fix in place, this test would have failed with
        # AssertionError: 1 != 2
        self.assertEqual(document.CurrentController.PageCount, 1)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
