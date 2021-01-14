# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf137105(UITestCase):

   def test_tdf137105(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        text = document.getText()
        cursor = text.createTextCursor()
        textGraphic = document.createInstance('com.sun.star.text.TextGraphicObject')
        provider = self.xContext.ServiceManager.createInstance('com.sun.star.graphic.GraphicProvider')
        graphic = provider.queryGraphic( mkPropertyValues({"URL": get_url_for_data_file("LibreOffice.jpg")}))
        textGraphic.Graphic = graphic
        text.insertTextContent(cursor, textGraphic, False)
        #select image
        document.getCurrentController().select(document.getDrawPage()[0])

        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:Sidebar")
        # Without the fix in place, this test would have crashed here
        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "InspectorTextPanel"}))

        # if the image is selected, there is nothing in the panel
        self.assertEqual('0', get_state_as_dict(xWriterEdit.getChild('listbox_fonts'))['Children'])

        self.xUITest.executeCommand(".uno:Sidebar")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
