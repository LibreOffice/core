# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

class tdf165815(UITestCase):

    def test_tdf165815(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            self.xUITest.executeCommand(".uno:ViewDataSourceBrowser")

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xTree = xWriterDoc.getChild("treeview")
            self.assertEqual("Bibliography", get_state_as_dict(xTree.getChild(0))['Text'])

            # Without the fix in place, this test would have crashed here
            self.xUITest.executeCommand(".uno:ViewDataSourceBrowser")

            self.assertTrue("treeview" not in xWriterDoc.getChildren())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
