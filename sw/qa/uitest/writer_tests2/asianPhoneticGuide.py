# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_by_text
import time
from uitest.debug import sleep
from uitest.uihelper.common import select_pos

class asianPhoneticGuide(UITestCase):

   def test_asian_phonetic_guide(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_modeless_dialog_through_command(".uno:RubyDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xLeft1ED = xDialog.getChild("Left1ED")
        xRight1ED = xDialog.getChild("Right1ED")
        xadjustlb = xDialog.getChild("adjustlb")
        xpositionlb = xDialog.getChild("positionlb")
        xstylelb = xDialog.getChild("stylelb")

        xLeft1ED.executeAction("TYPE", mkPropertyValues({"TEXT":"a"}))
        xRight1ED.executeAction("TYPE", mkPropertyValues({"TEXT":"w"}))
        select_by_text(xadjustlb, "Right")
        select_by_text(xpositionlb, "Right")
        select_by_text(xstylelb, "Quotation")

        xApplyBtn = xDialog.getChild("ok")
        xApplyBtn.executeAction("CLICK", tuple())

        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.assertEqual(document.Text.String[0:1], "a")
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
