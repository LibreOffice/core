# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import time
from uitest.path import get_srcdir_url
from uitest.debug import sleep
from uitest.uihelper.common import select_pos

#tdf106280
#1. Open Writer
#2. Insert an image (jpg)
#3. Format->Image->Properties (From menu)
#4. Click the "Crop" tab
def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

class tdf106280(UITestCase):
   def test_tdf106280_Crash_open_Image_dialog_click_Crop_tab(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()
        text = document.getText()
        cursor = text.createTextCursor()
        textGraphic = document.createInstance('com.sun.star.text.TextGraphicObject')
        provider = self.xContext.ServiceManager.createInstance('com.sun.star.graphic.GraphicProvider')
        graphic = provider.queryGraphic( mkPropertyValues({"URL": get_url_for_data_file("LibreOffice.jpg")}))
        textGraphic.Graphic = graphic
        text.insertTextContent(cursor, textGraphic, False)
        #select image
        document.getCurrentController().select(document.getDrawPage().getByIndex(0))
        sleep(1)    #need sleep here. Without this test crashes

        self.ui_test.execute_dialog_through_command(".uno:GraphicDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        select_pos(xTabs, "1")
        select_pos(xTabs, "2")
        select_pos(xTabs, "3")
        select_pos(xTabs, "4")
        select_pos(xTabs, "5")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        self.assertEqual(document.GraphicObjects.getCount(), 1)  #nr. of images

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
