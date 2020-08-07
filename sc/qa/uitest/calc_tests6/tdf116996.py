# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

#Bug 116996 - Crash recover on selecting Tools -> Options -> Advanced: Enable experimental features

class tdf116996(UITestCase):
    def test_tdf116996_enable_experimental_feature(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xLOEntry = xPages.getChild('0')                 # Libreoffice
        xLOEntry.executeAction("EXPAND", tuple())
        xAdvancedEntry = xLOEntry.getChild('10')
        xAdvancedEntry.executeAction("SELECT", tuple())          #Libreoffice / Advanced
        xexperimental = xDialogOpt.getChild("experimental")
        xexperimental.executeAction("CLICK", tuple())          #enable experimental features

        xOKBtn = xDialogOpt.getChild("ok")

        def handle_confirm_dlg(dialog):
            xLaterBtn = dialog.getChild("no")
            self.ui_test.close_dialog_through_button(xLaterBtn)

        self.ui_test.execute_blocking_action(xOKBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_confirm_dlg)

        #reopen options dialog and verify
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()
        xPages = xDialogOpt.getChild("pages")
        xLOEntry = xPages.getChild('0')                 # Libreoffice
        xLOEntry.executeAction("EXPAND", tuple())
        xAdvancedEntry = xLOEntry.getChild('10')
        xAdvancedEntry.executeAction("SELECT", tuple())          #Libreoffice / Advanced
        xexperimental = xDialogOpt.getChild("experimental")

        self.assertEqual(get_state_as_dict(xexperimental)["Selected"], "true")
        xexperimental.executeAction("CLICK", tuple())       #disable experimental features
        xOKBtn = xDialogOpt.getChild("ok")

        def handle_confirm_dlg(dialog):
            xLaterBtn = dialog.getChild("no")
            self.ui_test.close_dialog_through_button(xLaterBtn)

        self.ui_test.execute_blocking_action(xOKBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_confirm_dlg)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
