# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

class tdf134243(UITestCase):

    def test_tdf134243(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf134243.odt"))

        # Without the fix in place, it would hung launching the mailmerge wizard
        self.ui_test.execute_dialog_through_command(".uno:MailMergeWizard")

        xDialog = self.xUITest.getTopFocusWindow()

        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

