# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict

class tdf163325(UITestCase):

    def test_tdf163325_toc_preview_crash(self):
        xFilePath = get_url_for_data_file("tdf163325.odt")
        with self.ui_test.load_file(xFilePath):
            # Open ToC dialog and check index preview
            with self.ui_test.execute_dialog_through_command(".uno:InsertMultiIndex") as xTocDialog:
                xShowExample = xTocDialog.getChild("showexample")
                xShowExample.executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xShowExample)["Selected"], "true")

            # Save, reload and update all indexes in the document
            self.xUITest.executeCommand('.uno:Save')
            self.xUITest.executeCommand('.uno:Reload')
            # Without the fix in place, this test would have crashed here
            self.xUITest.executeCommand(".uno:UpdateAllIndexes")

        with self.ui_test.load_file(xFilePath) as document:
            # Check that the index is present
            xDocumentIndexes = document.DocumentIndexes
            self.assertEqual(len(xDocumentIndexes), 1)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
