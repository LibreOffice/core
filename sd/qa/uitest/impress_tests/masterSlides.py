# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class masterSlides(UITestCase):

    def test_change_master_slide_from_dialog(self):

        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        xImpress = self.xUITest.getTopFocusWindow()
        xMasterSlide = xImpress.getChild("masterslide")
        self.assertEqual("Default", get_state_as_dict(xMasterSlide)['SelectEntryText'])

        xMasterButton = xImpress.getChild("masterslidebutton")
        xMasterButton.executeAction("CLICK", tuple())

        self.xUITest.executeCommand(".uno:InsertMasterPage")

        xCloseMasterButton = xImpress.getChild("closemasterslide")
        xCloseMasterButton.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:PresentationLayout")
        xDialog = self.xUITest.getTopFocusWindow()

        xSelect = xDialog.getChild('select')
        xSelect.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))

        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        xMasterSlide = xImpress.getChild("masterslide")
        self.assertEqual("Default 1", get_state_as_dict(xMasterSlide)['SelectEntryText'])

        self.ui_test.close_doc()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
