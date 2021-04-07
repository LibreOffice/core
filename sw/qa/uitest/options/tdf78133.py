# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

#Bug 78133 - LibO crashes when in options dialog
#when i was going through the options dialog, if i click on the 'Colors' entry, LibO will crash.
class tdf78133(UITestCase):

    def test_tdf78133_options_app_colors(self):

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        #New text document
        #open option, go to App colors
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialog = self.xUITest.getTopFocusWindow()

        xPages = xDialog.getChild("pages")
        xLOEntry = xPages.getChild('0')                 # Lo Dev
        xLOEntry.executeAction("EXPAND", tuple())
        xLoAppColorsEntry = xLOEntry.getChild('8')
        xLoAppColorsEntry.executeAction("SELECT", tuple())          #Applications Colors
        #change text boundaries checkbox, save
        docboundaries = xDialog.getChild("docboundaries")
        docboundaries.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify - reopen dialog and check if "text boundaries" is still unchecked
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialog = self.xUITest.getTopFocusWindow()

        xPages = xDialog.getChild("pages")
        xLOEntry = xPages.getChild('0')                 # Lo Dev
        xLOEntry.executeAction("EXPAND", tuple())
        xLoAppColorsEntry = xLOEntry.getChild('8')
        xLoAppColorsEntry.executeAction("SELECT", tuple())          #Applications Colors
        #change text boundaries checkbox, save
        docboundaries = xDialog.getChild("docboundaries")
        self.assertEqual(get_state_as_dict(docboundaries)["Selected"], "false")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
