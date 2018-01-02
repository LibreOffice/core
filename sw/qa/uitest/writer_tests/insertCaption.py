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
        self.ui_test.execute_dialog_through_command(".uno:InsertFrame")   #  insert frame
        xDialogFr = self.xUITest.getTopFocusWindow()
#        print(xDialog.getChildren())

        xWidth = xDialogFr.getChild("width")
        xWidth.executeAction("UP", tuple())
        xWidth.executeAction("UP", tuple())

        xHeight = xDialogFr.getChild("height")
        xHeight.executeAction("UP", tuple())
        xHeight.executeAction("UP", tuple())

        xOkBtn=xDialogFr.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:InsertCaptionDialog")   #  caption
        xDialogCaption = self.xUITest.getTopFocusWindow()

#        print(xDialogCaption.getChildren())
#insertcaption.ui ('InsertCaptionDialog', 'alignment1', 'alignment2', 'alignment3', 'auto', 'box1', 'cancel', 'caption_edit', 'category', 'comboboxtext-entry', 'dialog-action_area1', 'dialog-vbox1', 'frame1', 'frame2', 'frame3', 'grid1', 'help', 'label1', 'label2', 'label3', 'label4', 'num_separator', 'num_separator_edit', 'numbering', 'numbering_label', 'ok', 'options', 'position', 'position_label', 'preview', 'separator_edit', 'separator_label')

        xCapt = xDialogCaption.getChild("caption_edit")
        xCapt.executeAction("TYPE", mkPropertyValues({"TEXT":"Caption"}))

        xOkBtn=xDialogCaption.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:InsertCaptionDialog")   # 2nd caption
        xDialogCaption = self.xUITest.getTopFocusWindow()
        xCapt = xDialogCaption.getChild("caption_edit")
        xCapt.executeAction("TYPE", mkPropertyValues({"TEXT":"Caption2"}))
        xSep = xDialogCaption.getChild("separator_edit")
        xSep.executeAction("TYPE", mkPropertyValues({"TEXT":"-"}))

        xOkBtn=xDialogCaption.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

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


        self.ui_test.close_doc()













