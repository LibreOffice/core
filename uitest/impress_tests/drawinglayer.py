# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.uihelper.common import get_state_as_dict

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase

from uitest.debug import time

class ImpressDrawinglayerTest(UITestCase):

    def test_move_object(self):
        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        xImpressDoc = self.xUITest.getTopFocusWindow()

        xEditWin = xImpressDoc.getChild("impress_win")

        xDrawinglayerObject = xEditWin.getChild("Unnamed Drawinglayer object 1")
        print(get_state_as_dict(xDrawinglayerObject))
        xDrawinglayerObject.executeAction("MOVE", mkPropertyValues({"X": "1000", "Y":"1000"}))

        time.sleep(10)

        self.ui_test.close_doc()
