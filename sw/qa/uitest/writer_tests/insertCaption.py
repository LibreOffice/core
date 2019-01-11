# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import time
from uitest.debug import sleep
from uitest.uihelper.common import select_pos

class insertCaption(UITestCase):

   def test_insert_caption(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        self.ui_test.execute_dialog_through_command(".uno:InsertFrame")   #  insert frame
        xDialogFr = self.xUITest.getTopFocusWindow()

        xWidth = xDialogFr.getChild("width")
        xWidth.executeAction("UP", tuple())
        xWidth.executeAction("UP", tuple())

        xHeight = xDialogFr.getChild("height")
        xHeight.executeAction("UP", tuple())
        xHeight.executeAction("UP", tuple())

        xOkBtn=xDialogFr.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.assertEqual(document.TextFrames.getCount(), 1)

        self.ui_test.execute_dialog_through_command(".uno:InsertCaptionDialog")   #  caption
        xDialogCaption = self.xUITest.getTopFocusWindow()

        xCapt = xDialogCaption.getChild("caption_edit")
        xCapt.executeAction("TYPE", mkPropertyValues({"TEXT":"Caption"}))

        xOkBtn=xDialogCaption.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        xFrame = document.TextFrames.getByIndex(0)

        self.assertEqual(document.TextFrames.getByIndex(0).Text.String, "\nText 1: Caption")

        self.ui_test.execute_dialog_through_command(".uno:InsertCaptionDialog")   # 2nd caption
        xDialogCaption = self.xUITest.getTopFocusWindow()
        xCapt = xDialogCaption.getChild("caption_edit")
        xCapt.executeAction("TYPE", mkPropertyValues({"TEXT":"Caption2"}))
        xSep = xDialogCaption.getChild("separator_edit")
        xSep.executeAction("TYPE", mkPropertyValues({"TEXT":"-"}))

        xOkBtn=xDialogCaption.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.assertEqual(document.TextFrames.getByIndex(0).Text.String, "\nText 1: Caption\nText 2-: Caption2")

        self.ui_test.execute_dialog_through_command(".uno:InsertCaptionDialog")   # 3. caption
        xDialogCaption = self.xUITest.getTopFocusWindow()
        xCapt = xDialogCaption.getChild("caption_edit")
        xCapt.executeAction("TYPE", mkPropertyValues({"TEXT":"Caption3"}))
        xSep = xDialogCaption.getChild("separator_edit")
        xSep.executeAction("TYPE", mkPropertyValues({"TEXT":"-"}))
        xPos = xDialogCaption.getChild("position")
        select_pos(xPos, "1")

        xOkBtn=xDialogCaption.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.assertEqual(document.TextFrames.getByIndex(0).Text.String, "\nText 1: Caption\nText 2-: Caption2\nText 3--: Caption3")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: