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


# Bug 117903 - Allow signature lines in Calc
class insertSignatureLineCalc(UITestCase):

   def test_insert_signature_line_calc(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

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
            element = document.Sheets.getByIndex(0).DrawPage.getByIndex(0)
            self.assertEqual(element.SignatureLineSuggestedSignerName, "Name")
            self.assertEqual(element.SignatureLineSuggestedSignerTitle, "Title")
            self.assertEqual(element.SignatureLineSuggestedSignerEmail, "Email")
            self.assertEqual(element.SignatureLineSuggestedSignerTitle, "Title")
            self.assertEqual(element.SignatureLineCanAddComment, False)
            self.assertEqual(element.SignatureLineShowSignDate, True)
            self.assertEqual(element.SignatureLineSigningInstructions, "Instructions")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
