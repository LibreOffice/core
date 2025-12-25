# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase

class tdf137726(UITestCase):

    def test_tdf137726(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            # three dialogs are displayed one after the other, click OK in all of them
            with self.ui_test.execute_dialog_through_command(".uno:DataDataPilotRun"):
                pass

            for i in range(2):
                xDialog = self.xUITest.getTopFocusWindow()
                xOKBtn = xDialog.getChild('ok')
                self.ui_test.close_dialog_through_button(xOKBtn)

            # Without the fix in place, this test would have hung here


# vim: set shiftwidth=4 softtabstop=4 expandtab:
