# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest_helper import UITest, get_state_as_dict

from helper import mkPropertyValues

from uitest.framework import UITestCase

import time

try:
    import pyuno
    import uno
    import unohelper
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

class SimpleImpressTest(UITestCase):
    def test_start_impress(self):

        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("cancel")
        xCancelBtn.executeAction("CLICK", tuple())

        time.sleep(2)

        xImpressDoc = self.xUITest.getTopFocusWindow()

        xEditWin = xImpressDoc.getChild("impress_win")
        xEditWin.executeAction("SET", mkPropertyValues({"ZOOM": "200"}))

        self.ui_test.close_doc()

    def test_select_page(self):

        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("cancel")
        xCancelBtn.executeAction("CLICK", tuple())

        time.sleep(2)

        xImpressDoc = self.xUITest.getTopFocusWindow()
        print(xImpressDoc.getChildren())

        xEditWin = xImpressDoc.getChild("impress_win")

        xEditWin.executeAction("GOTO", mkPropertyValues({"PAGE": "2"}))

        self.ui_test.close_doc()

    def test_select_text(self):

        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("cancel")
        xCancelBtn.executeAction("CLICK", tuple())

        time.sleep(2)

        xImpressDoc = self.xUITest.getTopFocusWindow()
        print(xImpressDoc.getChildren())

        xEditWin = xImpressDoc.getChild("impress_win")

        self.ui_test.close_doc()

    def test_select_object(self):

        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("cancel")
        xCancelBtn.executeAction("CLICK", tuple())

        time.sleep(2)

        xImpressDoc = self.xUITest.getTopFocusWindow()
        print(xImpressDoc.getChildren())

        xEditWin = xImpressDoc.getChild("impress_win")

        xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
        xEditWin.executeAction("DESELECT", tuple())

        self.ui_test.close_doc()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
