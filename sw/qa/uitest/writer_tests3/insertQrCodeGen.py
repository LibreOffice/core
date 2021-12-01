# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text
from com.sun.star.lang import IndexOutOfBoundsException

class insertQrCode(UITestCase):

   def test_insert_qr_code(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            # cancel the dialog without doing anything
            with self.ui_test.execute_dialog_through_command(".uno:InsertQrCode", close_button="cancel") as xDialog:

                xURL = xDialog.getChild("edit_text")
                type_text(xURL, "www.libreoffice.org")

            with self.assertRaises(IndexOutOfBoundsException):
                document.DrawPage.getByIndex(0)

            # Reopen the dialog box
            with self.ui_test.execute_dialog_through_command(".uno:InsertQrCode") as xDialog:

                # Get elements in the Dialog Box
                xURL = xDialog.getChild("edit_text")
                xECC_Low = xDialog.getChild("button_low") #How radio button input is written in text.
                xBorder = xDialog.getChild("edit_margin")

                type_text(xURL, "www.libreoffice.org") #set the QR code
                xECC_Low.executeAction("CLICK", tuple())
                xBorder.executeAction("UP", tuple())
                xBorder.executeAction("DOWN", tuple())

            # check the QR code in the document
            self.assertEqual(document.DrawPage.getByIndex(0).BarCodeProperties.Payload, "www.libreoffice.org")
            self.assertEqual(document.DrawPage.getByIndex(0).BarCodeProperties.ErrorCorrection, 1)
            self.assertEqual(document.DrawPage.getByIndex(0).BarCodeProperties.Border, 1)


   def test_insert_qr_code_gen2(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_dialog_through_command(".uno:InsertQrCode") as xDialog:

                xURL = xDialog.getChild("edit_text")
                xECC_Low = xDialog.getChild("button_low")
                xBorder = xDialog.getChild("edit_margin")

                type_text(xURL, "www.libreoffice.org") #set the QR code
                xECC_Low.executeAction("CLICK", tuple())
                xBorder.executeAction("UP", tuple())
                xBorder.executeAction("DOWN", tuple())

            #check the QR Code in the document
            self.assertEqual(document.DrawPage.getByIndex(0).BarCodeProperties.Payload, "www.libreoffice.org")
            self.assertEqual(document.DrawPage.getByIndex(0).BarCodeProperties.ErrorCorrection, 1)
            self.assertEqual(document.DrawPage.getByIndex(0).BarCodeProperties.Border, 1)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
