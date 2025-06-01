# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_by_text

class PageNumberWizard(UITestCase):

    def test_insert_page_number(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            with self.ui_test.execute_dialog_through_command(".uno:PageNumberWizard") as xDialog:
                xPositionCombo = xDialog.getChild("positionCombo")
                self.assertEqual("Bottom of page (Footer)", get_state_as_dict(xPositionCombo)["SelectEntryText"])
                xAlignmentCombo = xDialog.getChild("alignmentCombo")
                self.assertEqual("Center", get_state_as_dict(xAlignmentCombo)["SelectEntryText"])
                xMirrorCheckbox = xDialog.getChild("mirrorCheckbox")
                self.assertEqual("true", get_state_as_dict(xMirrorCheckbox)["Selected"])
                self.assertEqual("false", get_state_as_dict(xMirrorCheckbox)["Enabled"])
                xPagetotalCheckbox = xDialog.getChild("pagetotalCheckbox")
                self.assertEqual("false", get_state_as_dict(xPagetotalCheckbox)["Selected"])
                self.assertEqual("true", get_state_as_dict(xPagetotalCheckbox)["Enabled"])
                xPagerangetotalCheckbox = xDialog.getChild("pagerangetotalCheckbox")
                self.assertEqual("false", get_state_as_dict(xPagerangetotalCheckbox)["Selected"])
                self.assertEqual("true", get_state_as_dict(xPagerangetotalCheckbox)["Enabled"])
                xFitintoexistingmarginsCheckbox = xDialog.getChild("fitintoexistingmarginsCheckbox")
                self.assertEqual("false", get_state_as_dict(xFitintoexistingmarginsCheckbox)["Selected"])
                self.assertEqual("true", get_state_as_dict(xFitintoexistingmarginsCheckbox)["Enabled"])
                xNumfmtlb = xDialog.getChild("numfmtlb")
                self.assertEqual("1, 2, 3, ...", get_state_as_dict(xNumfmtlb)["SelectEntryText"])

            self.assertIsNone(document.StyleFamilies.PageStyles.Standard.HeaderText)
            xFooter = document.StyleFamilies.PageStyles.Standard.FooterText.createEnumeration().nextElement()
            self.assertEqual("1", xFooter.String)

            self.xUITest.executeCommand(".uno:Undo")

            self.assertIsNone(document.StyleFamilies.PageStyles.Standard.HeaderText)
            # FIXME: tdf#164033: Undo is disabled to avoid the crash
            self.assertIsNotNone(document.StyleFamilies.PageStyles.Standard.FooterText)

            with self.ui_test.execute_dialog_through_command(".uno:PageNumberWizard") as xDialog:
                xPositionCombo = xDialog.getChild("positionCombo")
                select_by_text(xPositionCombo, "Top of page (Header)")
                xNumfmtlb = xDialog.getChild("numfmtlb")
                select_by_text(xNumfmtlb, "A, B, C, ...")

            xHeader = document.StyleFamilies.PageStyles.Standard.HeaderText.createEnumeration().nextElement()
            self.assertEqual("A", xHeader.String)
            self.assertIsNotNone(document.StyleFamilies.PageStyles.Standard.FooterText)

            self.xUITest.executeCommand(".uno:Undo")

            self.assertIsNotNone(document.StyleFamilies.PageStyles.Standard.HeaderText)
            self.assertIsNotNone(document.StyleFamilies.PageStyles.Standard.FooterText)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
