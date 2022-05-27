# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos

from libreoffice.uno.propertyvalue import mkPropertyValues


class Tdf142763(UITestCase):

    def test_tdf142763_header(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog") as xPageFormatDialog:
                xTabControl = xPageFormatDialog.getChild("tabcontrol")
                select_pos(xTabControl, "4")

                xHeaderOnBtn = xPageFormatDialog.getChild("checkHeaderOn")
                xHeaderOnBtn.executeAction("CLICK", tuple())
                xFirstHeaderSameBtn = xPageFormatDialog.getChild("checkSameFP")
                xFirstHeaderSameBtn.executeAction("CLICK", tuple())

                with self.ui_test.execute_dialog_through_command(".uno:EditHeaderAndFooter") as xHeaderDialog:
                    xLeftTextWindow = xHeaderDialog.getChild("textviewWND_LEFT")
                    xLeftTextWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "first"}))
                    xCenterTextWindow = xHeaderDialog.getChild("textviewWND_CENTER")
                    xCenterTextWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "page | "}))
                    xRightTextWindow = xHeaderDialog.getChild("textviewWND_RIGHT")
                    xRightTextWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "header"}))

                    self.assertEqual("first", get_state_as_dict(xLeftTextWindow)["Text"])
                    self.assertEqual("page | Sheet1", get_state_as_dict(xCenterTextWindow)["Text"])
                    self.assertEqual("header", get_state_as_dict(xRightTextWindow)["Text"])

    def test_tdf142763_footer(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog") as xPageFormatDialog:
                xTabControl = xPageFormatDialog.getChild("tabcontrol")
                select_pos(xTabControl, "5")

                xFooterOnBtn = xPageFormatDialog.getChild("checkFooterOn")
                xFooterOnBtn.executeAction("CLICK", tuple())
                xFirstFooterSameBtn = xPageFormatDialog.getChild("checkSameFP")
                xFirstFooterSameBtn.executeAction("CLICK", tuple())

                with self.ui_test.execute_dialog_through_command(".uno:EditHeaderAndFooter") as xFooterDialog:
                    xLeftTextWindow = xFooterDialog.getChild("textviewWND_LEFT")
                    xLeftTextWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "first"}))
                    xCenterTextWindow = xFooterDialog.getChild("textviewWND_CENTER")
                    xCenterTextWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "page | "}))
                    xRightTextWindow = xFooterDialog.getChild("textviewWND_RIGHT")
                    xRightTextWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "footer"}))

                    self.assertEqual("first", get_state_as_dict(xLeftTextWindow)["Text"])
                    self.assertEqual("page | Sheet1", get_state_as_dict(xCenterTextWindow)["Text"])
                    self.assertEqual("footer", get_state_as_dict(xRightTextWindow)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
