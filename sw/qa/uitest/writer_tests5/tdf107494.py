# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.debug import sleep
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

#Bug 107494 - CRASH: LibreOffice crashes while deleting the header containing an image

class tdf107494(UITestCase):
    def test_tdf107494_delete_header_with_image(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #insert header
        self.assertEqual(document.StyleFamilies.PageStyles.Standard.HeaderIsOn, False)
        self.xUITest.executeCommand(".uno:InsertPageHeader?PageStyle:string=Default%20Style&On:bool=true")
        self.assertEqual(document.StyleFamilies.PageStyles.Standard.HeaderIsOn, True)
        #insert image
        text = document.getText()
        cursor = text.createTextCursor()
        oStyleFamilies = document.getStyleFamilies()
        #https://forum.openoffice.org/en/forum/viewtopic.php?f=7&t=71227
        obj2 = oStyleFamilies.getByName("PageStyles")
        obj3 = obj2.getByName("Standard")
        oHeaderText = obj3.HeaderText
        oHeaderText.setString("New text for header")  #write text to header
        obj4 = oHeaderText.createTextCursor()
        text = obj4.getText()
        cursor = text.createTextCursor()

        textGraphic = document.createInstance('com.sun.star.text.TextGraphicObject')
        provider = self.xContext.ServiceManager.createInstance('com.sun.star.graphic.GraphicProvider')
        graphic = provider.queryGraphic( mkPropertyValues({"URL": get_url_for_data_file("LibreOffice_external_logo_100px.png")}))
        textGraphic.Graphic = graphic
        text.insertTextContent(cursor, textGraphic, False)
        # Delete the header
        self.ui_test.execute_dialog_through_command(".uno:InsertPageHeader?PageStyle:string=Default%20Style&On:bool=false")
        xDialog = self.xUITest.getTopFocusWindow()  #question dialog
        xOption = xDialog.getChild("yes")
        xOption.executeAction("CLICK", tuple())

        self.assertEqual(document.StyleFamilies.PageStyles.Standard.HeaderIsOn, False)

        self.ui_test.close_doc()

    def test_tdf107494_delete_footer_with_image(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #insert footer
        self.assertEqual(document.StyleFamilies.PageStyles.Standard.FooterIsOn, False)
        self.xUITest.executeCommand(".uno:InsertPageFooter?PageStyle:string=Default%20Style&On:bool=true")
        self.assertEqual(document.StyleFamilies.PageStyles.Standard.FooterIsOn, True)
        #insert image
        text = document.getText()
        cursor = text.createTextCursor()
        oStyleFamilies = document.getStyleFamilies()
        #https://forum.openoffice.org/en/forum/viewtopic.php?f=7&t=71227
        obj2 = oStyleFamilies.getByName("PageStyles")
        obj3 = obj2.getByName("Standard")
        oFooterText = obj3.FooterText
        oFooterText.setString("New text for footer")  #write text to footer
        obj4 = oFooterText.createTextCursor()
        text = obj4.getText()
        cursor = text.createTextCursor()

        textGraphic = document.createInstance('com.sun.star.text.TextGraphicObject')
        provider = self.xContext.ServiceManager.createInstance('com.sun.star.graphic.GraphicProvider')
        graphic = provider.queryGraphic( mkPropertyValues({"URL": get_url_for_data_file("LibreOffice_external_logo_100px.png")}))
        textGraphic.Graphic = graphic
        text.insertTextContent(cursor, textGraphic, False)
        # Delete the footer
        self.ui_test.execute_dialog_through_command(".uno:InsertPageFooter?PageStyle:string=Default%20Style&On:bool=false")
        xDialog = self.xUITest.getTopFocusWindow()  #question dialog
        xOption = xDialog.getChild("yes")
        xOption.executeAction("CLICK", tuple())

        self.assertEqual(document.StyleFamilies.PageStyles.Standard.FooterIsOn, False)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
