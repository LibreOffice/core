# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase

class SimpleImpressTest(UITestCase):
    def test_start_impress(self):

        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        xImpressDoc = self.xUITest.getTopFocusWindow()

        xEditWin = xImpressDoc.getChild("impress_win")
        xEditWin.executeAction("SET", mkPropertyValues({"ZOOM": "200"}))

        self.assertEqual(get_state_as_dict(xEditWin)["Zoom"], "200")

        self.ui_test.close_doc()

    def test_select_page(self):

        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        xImpressDoc = self.xUITest.getTopFocusWindow()

        xEditWin = xImpressDoc.getChild("impress_win")

        self.assertEqual(get_state_as_dict(xEditWin)["CurrentSlide"], "1")

        self.xUITest.executeCommand(".uno:InsertPage")

        self.assertEqual(get_state_as_dict(xEditWin)["CurrentSlide"], "2")

        xEditWin.executeAction("GOTO", mkPropertyValues({"PAGE": "1"}))

        self.assertEqual(get_state_as_dict(xEditWin)["CurrentSlide"], "1")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
