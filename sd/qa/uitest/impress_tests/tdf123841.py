# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from com.sun.star.drawing.FillStyle import NONE

class Tdf123841(UITestCase):

   def test_tdf123841(self):
        self.ui_test.create_doc_in_start_center("impress")
        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        xImpressDoc = self.xUITest.getTopFocusWindow()

        xEditWin = xImpressDoc.getChild("impress_win")
        self.xUITest.executeCommandWithParameters(".uno:Rect_Unfilled", mkPropertyValues({"KeyModifier": 8192}))

        document = self.ui_test.get_component()
        self.assertEqual(3, document.DrawPages[0].getCount())

        # Without the fix in place, this test would have failed with
        # AssertionError: <Enum instance com.sun.star.drawing.FillStyle ('NONE')> !=
        # <Enum instance com.sun.star.drawing.FillStyle ('SOLID')>
        for i in range(3):
            self.assertEqual(NONE, document.DrawPages[0][i].FillStyle)


        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
