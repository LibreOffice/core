# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
from com.sun.star.lang import IndexOutOfBoundsException

class insertQrCode(UITestCase):

   def test_insert_qr_code(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        # cancel the dialog without doing anything
        self.ui_test.execute_dialog_through_command(".uno:InsertQrCode")
        xDialog = self.xUITest.getTopFocusWindow()

        xURL = xDialog.getChild("edit_text")
        type_text(xURL, "www.libreoffice.org")

        xCloseBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCloseBtn)
        with self.assertRaises(IndexOutOfBoundsException):
            document.DrawPage.getByIndex(0)

        # Reopen the dialog box
        self.ui_test.execute_dialog_through_command(".uno:InsertQrCode")
        xDialog = self.xUITest.getTopFocusWindow()

        # Get elements in the Dialog Box
        xURL = xDialog.getChild("edit_text")
        xECC_Low = xDialog.getChild("button_low") #How radio button input is written in text.
        xBorder = xDialog.getChild("edit_margin")

        type_text(xURL, "www.libreoffice.org") #set the QR code
        xECC_Low.executeAction("CLICK", tuple())
        xBorder.executeAction("UP", tuple())
        xBorder.executeAction("DOWN", tuple())
        xOKBtn = xDialog.getChild("ok")
        xOKBtn.executeAction("CLICK", tuple())

        # check the QR code in the document
        self.assertEqual(document.DrawPage.getByIndex(0).QRCodeProperties.Payload, "www.libreoffice.org")
        self.assertEqual(document.DrawPage.getByIndex(0).QRCodeProperties.ErrorCorrection, 1)
        self.assertEqual(document.DrawPage.getByIndex(0).QRCodeProperties.Border, 1)

        self.ui_test.close_doc()

   def test_insert_qr_code_gen2(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:InsertQrCode")
        xDialog = self.xUITest.getTopFocusWindow()

        xURL = xDialog.getChild("edit_text")
        xECC_Low = xDialog.getChild("button_low")
        xBorder = xDialog.getChild("edit_margin")

        type_text(xURL, "www.libreoffice.org") #set the QR code
        xECC_Low.executeAction("CLICK", tuple())
        xBorder.executeAction("UP", tuple())
        xBorder.executeAction("DOWN", tuple())
        xOKBtn = xDialog.getChild("ok")
        xOKBtn.executeAction("CLICK", tuple())

        #check the QR Code in the document
        self.assertEqual(document.DrawPage.getByIndex(0).QRCodeProperties.Payload, "www.libreoffice.org")
        self.assertEqual(document.DrawPage.getByIndex(0).QRCodeProperties.ErrorCorrection, 1)
        self.assertEqual(document.DrawPage.getByIndex(0).QRCodeProperties.Border, 1)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
