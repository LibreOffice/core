# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_by_text

class PageNumberWizard(UITestCase):

    def make_pages(self, nPages):
        xWriterEdit = self.xUITest.getTopFocusWindow().getChild("writer_edit")
        for _ in range(nPages - 1):
            self.xUITest.executeCommand(".uno:InsertPagebreak")
        self.assertEqual(str(nPages), get_state_as_dict(xWriterEdit)["Pages"])

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

    def test_tdf165852(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            self.xUITest.executeCommand(".uno:InsertPagebreak")
            self.assertEqual("2", get_state_as_dict(xWriterEdit)["Pages"])

            with self.ui_test.execute_dialog_through_command(".uno:PageNumberWizard") as xDialog:
                xPositionCombo = xDialog.getChild("positionCombo")
                self.assertEqual("Bottom of page (Footer)", get_state_as_dict(xPositionCombo)["SelectEntryText"])

                xAlignmentCombo = xDialog.getChild("alignmentCombo")
                select_by_text(xAlignmentCombo, "Right")
                self.assertEqual("Right", get_state_as_dict(xAlignmentCombo)["SelectEntryText"])

                xMirrorCheckbox = xDialog.getChild("mirrorCheckbox")
                self.assertEqual("true", get_state_as_dict(xMirrorCheckbox)["Selected"])

            xStandardStyle = document.StyleFamilies.PageStyles.Standard
            self.assertIsNone(xStandardStyle.HeaderText)

            # Without the fix in place, this test would have failed with
            # AssertionError: '1' != ''
            self.assertEqual("1", xStandardStyle.FooterText.String)
            self.assertEqual("1", xStandardStyle.FooterTextFirst.String)
            self.assertEqual("2", xStandardStyle.FooterTextLeft.String)
            self.assertEqual("1", xStandardStyle.FooterTextRight.String)

    def test_rerun_without_mirror(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            self.make_pages(3)

            # com.sun.star.style.ParagraphAdjust
            PARA_ADJUST_LEFT = 0
            PARA_ADJUST_RIGHT = 1

            # Right-aligned page numbers, with the mirror checkbox left at its default.
            with self.ui_test.execute_dialog_through_command(".uno:PageNumberWizard") as xDialog:
                select_by_text(xDialog.getChild("alignmentCombo"), "Right")

            # The even page has a footer of its own now, holding a left-aligned page number.
            xStandardStyle = document.StyleFamilies.PageStyles.Standard
            self.assertFalse(xStandardStyle.FooterIsShared)
            xEvenFooter = xStandardStyle.FooterTextLeft.createEnumeration().nextElement()
            self.assertEqual(PARA_ADJUST_LEFT, xEvenFooter.ParaAdjust)

            # Run the wizard again from an even page, this time asking for no mirroring.
            document.CurrentController.ViewCursor.jumpToPage(2)
            with self.ui_test.execute_dialog_through_command(".uno:PageNumberWizard") as xDialog:
                select_by_text(xDialog.getChild("alignmentCombo"), "Right")
                xDialog.getChild("mirrorCheckbox").executeAction("CLICK", tuple())

            # Both pages share one footer again, with a single right-aligned page number.
            # Without the fix in place, this test would have failed with
            # AssertionError: False is not true
            self.assertTrue(xStandardStyle.FooterIsShared)
            xFooterParagraphs = [p for p in xStandardStyle.FooterTextLeft]
            self.assertEqual(1, len(xFooterParagraphs))
            self.assertEqual(PARA_ADJUST_RIGHT, xFooterParagraphs[0].ParaAdjust)

    def test_rerun_from_another_page(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            self.make_pages(3)

            # Centered page numbers are never mirrored, so all three pages share one footer.
            xViewCursor = document.CurrentController.ViewCursor
            xViewCursor.jumpToPage(1)
            with self.ui_test.execute_dialog_through_command(".uno:PageNumberWizard") as xDialog:
                select_by_text(xDialog.getChild("alignmentCombo"), "Center")

            # Run the wizard again from another page, asking for the page total this time.
            xViewCursor.jumpToPage(3)
            with self.ui_test.execute_dialog_through_command(".uno:PageNumberWizard") as xDialog:
                select_by_text(xDialog.getChild("alignmentCombo"), "Center")
                xDialog.getChild("pagetotalCheckbox").executeAction("CLICK", tuple())

            # The second run replaces the page number of the first instead of adding another one.
            # Without the fix in place, this test would have failed with
            # AssertionError: 1 != 2
            xFooterParagraphs = [p for p in document.StyleFamilies.PageStyles.Standard.FooterText]
            self.assertEqual(1, len(xFooterParagraphs))
            self.assertEqual("1 / 3", xFooterParagraphs[0].String)

    def test_mirror_on_a_single_page(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            self.make_pages(1)

            # A single page has no even page to mirror onto, but the checkbox is on by default.
            with self.ui_test.execute_dialog_through_command(".uno:PageNumberWizard") as xDialog:
                select_by_text(xDialog.getChild("alignmentCombo"), "Right")
                xMirrorCheckbox = xDialog.getChild("mirrorCheckbox")
                self.assertEqual("true", get_state_as_dict(xMirrorCheckbox)["Selected"])

            # The page margins stay the same on both sides, since nothing was mirrored.
            # Without the fix in place, this test would have failed with
            # AssertionError: 'ALL' != 'MIRRORED'
            xStandardStyle = document.StyleFamilies.PageStyles.Standard
            self.assertEqual("ALL", xStandardStyle.PageStyleLayout.value)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
