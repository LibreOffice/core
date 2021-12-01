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
from uitest.uihelper.common import get_state_as_dict, type_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from com.sun.star.lang import IndexOutOfBoundsException

#Bug 117903 - Allow signature lines in Calc

class insertSignatureLineCalc(UITestCase):

   def test_insert_signature_line_calc(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            # cancel the dialog without doing anything
            with self.ui_test.execute_dialog_through_command(".uno:InsertSignatureLine", close_button="cancel") as xDialog:

                xName = xDialog.getChild("edit_name")
                xName.executeAction("TYPE", mkPropertyValues({"TEXT":"Name"})) #set the signature line

            with self.assertRaises(IndexOutOfBoundsException):
                document.Sheets.getByIndex(0).DrawPage.getByIndex(0)

            # set the signature line
            with self.ui_test.execute_dialog_through_command(".uno:InsertSignatureLine") as xDialog:

                xName = xDialog.getChild("edit_name")
                xTitle = xDialog.getChild("edit_title")
                xEmail = xDialog.getChild("edit_email")
                xComment = xDialog.getChild("checkbox_can_add_comments")
                xInstructions = xDialog.getChild("edit_instructions")

                xName.executeAction("TYPE", mkPropertyValues({"TEXT":"Name"})) #set the signature line
                xTitle.executeAction("TYPE", mkPropertyValues({"TEXT":"Title"}))
                xEmail.executeAction("TYPE", mkPropertyValues({"TEXT":"Email"}))
                xComment.executeAction("CLICK", tuple())
                xInstructions.executeAction("TYPE", mkPropertyValues({"TEXT":"Instructions"}))

            #check the signature Line in the document
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineSuggestedSignerName, "Name")
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineSuggestedSignerTitle, "Title")
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineSuggestedSignerEmail, "Email")
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineSuggestedSignerTitle, "Title")
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineCanAddComment, False)
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineShowSignDate, True)
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineSigningInstructions, "Instructions")


   def test_insert_signature_line2_calc(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            with self.ui_test.execute_dialog_through_command(".uno:InsertSignatureLine") as xDialog:

                xName = xDialog.getChild("edit_name")
                xTitle = xDialog.getChild("edit_title")
                xEmail = xDialog.getChild("edit_email")
                xComment = xDialog.getChild("checkbox_can_add_comments")
                xDate = xDialog.getChild("checkbox_show_sign_date")
                xInstructions = xDialog.getChild("edit_instructions")

                xName.executeAction("TYPE", mkPropertyValues({"TEXT":"Name"})) #set the signature line
                xTitle.executeAction("TYPE", mkPropertyValues({"TEXT":"Title"}))
                xEmail.executeAction("TYPE", mkPropertyValues({"TEXT":"Email"}))
                xDate.executeAction("CLICK", tuple())
                xComment.executeAction("CLICK", tuple())
                xInstructions.executeAction("TYPE", mkPropertyValues({"TEXT":"Instructions"}))

            #check the signature Line in the document
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineSuggestedSignerName, "Name")
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineSuggestedSignerTitle, "Title")
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineSuggestedSignerEmail, "Email")
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineSuggestedSignerTitle, "Title")
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineCanAddComment, False)
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineShowSignDate, False)
            self.assertEqual(document.Sheets.getByIndex(0).DrawPage.getByIndex(0).SignatureLineSigningInstructions, "Instructions")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
