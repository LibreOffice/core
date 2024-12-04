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

class optionsDialog(UITestCase):

    def test_moreIconsDialog(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:

                xPages = xDialog.getChild("pages")
                xLOEntry = xPages.getChild('0')
                xLOEntry.executeAction("EXPAND", tuple())
                xViewEntry = xLOEntry.getChild('2')
                xViewEntry.executeAction("SELECT", tuple())

                xMoreIconsBtn = xDialog.getChild("btnMoreIcons")

                with self.ui_test.execute_blocking_action(xMoreIconsBtn.executeAction, args=('CLICK', ()), close_button="buttonClose") as dialog:
                    # Check it doesn't crash while opening it
                    xCloseBtn = dialog.getChild("buttonClose")
                    self.ui_test.wait_until_property_is_updated(xCloseBtn, "Enabled", "true")



    def test_tdf138596(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:
                xPages = xDialog.getChild("pages")
                xWriterEntry = xPages.getChild('3')
                xWriterEntry.executeAction("EXPAND", tuple())
                xFormattingAidsEntry = xWriterEntry.getChild('2')
                xFormattingAidsEntry.executeAction("SELECT", tuple())

                xApplyBtn = xDialog.getChild("apply")

                # Click apply button twice
                # Without the fix in place, this test would have crashed here
                xApplyBtn.executeAction("CLICK", tuple())
                xApplyBtn.executeAction("CLICK", tuple())


    def test_tdf132274Text(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:
                xPages = xDialog.getChild("pages")
                xWriterEntry = xPages.getChild('3')
                xWriterEntry.executeAction("EXPAND", tuple())
                xContentEntry = xWriterEntry.getChild('1')
                xContentEntry.executeAction("SELECT", tuple())
                xOptimal = xDialog.getChild("zoomoptimal")
                self.assertEqual(get_state_as_dict(xOptimal)['Visible'], "true")
                xApplyBtn = xDialog.getChild("apply")
                self.assertIsNotNone(xApplyBtn)

    def test_tdf132274Web(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:
                xPages = xDialog.getChild("pages")
                xWriterWebEntry = xPages.getChild('4')
                xWriterWebEntry.executeAction("EXPAND", tuple())
                xContentWebEntry = xWriterWebEntry.getChild('1')
                xContentWebEntry.executeAction("SELECT", tuple())
                try:
                    xOptimalWeb = xDialog.getChild("zoomoptimal")
                    raise RuntimeError("Zoom controls visible in Web dialog")
                except Exception:
                    xApplyBtn = xDialog.getChild("apply")



# vim: set shiftwidth=4 softtabstop=4 expandtab:
