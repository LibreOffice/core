# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos

class insertCaption(UITestCase):

   def test_insert_caption(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            with self.ui_test.execute_dialog_through_command(".uno:InsertFrame") as xDialogFr:

                xWidth = xDialogFr.getChild("width")
                xWidth.executeAction("UP", tuple())
                xWidth.executeAction("UP", tuple())

                xHeight = xDialogFr.getChild("height")
                xHeight.executeAction("UP", tuple())
                xHeight.executeAction("UP", tuple())


            self.assertEqual(document.TextFrames.getCount(), 1)

            with self.ui_test.execute_dialog_through_command(".uno:InsertCaptionDialog") as xDialogCaption:

                xCapt = xDialogCaption.getChild("caption_edit")
                xCapt.executeAction("TYPE", mkPropertyValues({"TEXT":"Caption"}))


            xFrame = document.TextFrames[0]

            self.assertEqual(document.TextFrames[0].Text.String.replace('\r\n', '\n'), "\nText 1: Caption")

            with self.ui_test.execute_dialog_through_command(".uno:InsertCaptionDialog") as xDialogCaption:
                xCapt = xDialogCaption.getChild("caption_edit")
                xCapt.executeAction("TYPE", mkPropertyValues({"TEXT":"Caption2"}))
                xSep = xDialogCaption.getChild("separator_edit")
                xSep.executeAction("TYPE", mkPropertyValues({"TEXT":"-"}))


            self.assertEqual(document.TextFrames[0].Text.String.replace('\r\n', '\n'), "\nText 1: Caption\nText 2-: Caption2")

            with self.ui_test.execute_dialog_through_command(".uno:InsertCaptionDialog") as xDialogCaption:
                xCapt = xDialogCaption.getChild("caption_edit")
                xCapt.executeAction("TYPE", mkPropertyValues({"TEXT":"Caption3"}))
                xSep = xDialogCaption.getChild("separator_edit")
                xSep.executeAction("TYPE", mkPropertyValues({"TEXT":"-"}))
                xPos = xDialogCaption.getChild("position")
                select_pos(xPos, "1")


            self.assertEqual(document.TextFrames[0].Text.String.replace('\r\n', '\n'), "\nText 1: Caption\nText 2-: Caption2\nText 3--: Caption3")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
