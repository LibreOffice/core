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
from uitest.uihelper.common import get_url_for_data_file

#Bug 116474 - Undo/redo: The redo of adding caption to an image isn't working: no image

class tdf116474(UITestCase):

   def test_tdf116474_insert_caption_undo(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            text = document.getText()
            cursor = text.createTextCursor()
            textGraphic = document.createInstance('com.sun.star.text.TextGraphicObject')
            provider = self.xContext.ServiceManager.createInstance('com.sun.star.graphic.GraphicProvider')
            graphic = provider.queryGraphic( mkPropertyValues({"URL": get_url_for_data_file("LibreOffice.jpg")}))
            textGraphic.Graphic = graphic
            text.insertTextContent(cursor, textGraphic, False)
            #select image
            document.getCurrentController().select(document.getDrawPage()[0])

            with self.ui_test.execute_dialog_through_command(".uno:InsertCaptionDialog") as xDialogCaption:

                xCapt = xDialogCaption.getChild("caption_edit")
                xCapt.executeAction("TYPE", mkPropertyValues({"TEXT":"Caption"}))


            xFrame = document.TextFrames[0]
            self.assertEqual(document.TextFrames[0].Text.String, "Figure 1: Caption")
            self.assertEqual(document.GraphicObjects.getCount(), 1)  #nr. of images
            #Undo, redo
            self.xUITest.executeCommand(".uno:Undo")
            self.xUITest.executeCommand(".uno:Redo")
            #Verify
            self.assertEqual(document.TextFrames[0].Text.String, "Figure 1: Caption")
            self.assertEqual(document.GraphicObjects.getCount(), 1)  #nr. of images

# vim: set shiftwidth=4 softtabstop=4 expandtab:
