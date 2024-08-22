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

class Tdf147755(UITestCase):

    def test_tdf147755(self):
        with self.ui_test.create_doc_in_start_center("math"):

            with self.ui_test.execute_dialog_through_command(".uno:SymbolCatalogue", close_button="close") as xDialog:
                xSymbolset = xDialog.getChild("symbolset")
                self.assertEqual("Arabic", get_state_as_dict(xSymbolset)["DisplayText"])

                xOk = xDialog.getChild("ok")
                xOk.executeAction("CLICK", tuple())

            xMathDoc = self.xUITest.getTopFocusWindow()
            xEditView = xMathDoc.getChild("editview")

            # Without the fix in place, this test would have failed with
            # AssertionError: '%arRay ' != ''
            self.assertEqual("%arRay ", get_state_as_dict(xEditView)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
