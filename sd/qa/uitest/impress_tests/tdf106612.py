# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

class Tdf106612(UITestCase):

   def test_tdf106612(self):
        self.ui_test.create_doc_in_start_center("impress")
        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        xImpressDoc = self.xUITest.getTopFocusWindow()

        xEditWin = xImpressDoc.getChild("impress_win")

        self.xUITest.executeCommand(".uno:DuplicatePage")

        self.assertEqual("2", get_state_as_dict(xEditWin)["CurrentSlide"])

        xEditWin.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SdNavigatorPanel"}))

        xTree = xImpressDoc.getChild("tree")
        self.assertEqual(2, len(xTree.getChildren()))

        self.assertEqual("Slide 1", get_state_as_dict(xTree.getChild('0'))['Text'])
        self.assertEqual(2, len(xTree.getChild('0').getChildren()))

        self.assertEqual("Slide 2", get_state_as_dict(xTree.getChild('1'))['Text'])
        self.assertEqual(2, len(xTree.getChild('1').getChildren()))

        xTree.getChild('0').executeAction("DOUBLECLICK", tuple())

        # Without the fix in place, this test would have failed with
        # AssertionError: '1' != '2'
        self.assertEqual("1", get_state_as_dict(xEditWin)["CurrentSlide"])

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
