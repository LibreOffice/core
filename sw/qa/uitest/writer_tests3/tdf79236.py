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

class tdf79236(UITestCase):

    def test_paragraph(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, "Test for tdf79236")


            selection = self.xUITest.executeCommand(".uno:SelectAll")

            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaLeftMargin, 0)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaRightMargin, 0)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaTopMargin, 0)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaBottomMargin, 0)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaFirstLineIndent, 0)

            self.assertEqual(document.CurrentSelection.getByIndex(0).String, "Test for tdf79236")

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xParagraphDlg:



                xLeftSpnBtn = xParagraphDlg.getChild("spinED_LEFTINDENT")
                for _ in range(0,20):
                    xLeftSpnBtn.executeAction("UP", tuple())

                xRightSpnBtn = xParagraphDlg.getChild("spinED_RIGHTINDENT")
                for _ in range(0,20):
                    xRightSpnBtn.executeAction("UP", tuple())


                xLineSpnBtn = xParagraphDlg.getChild("spinED_FLINEINDENT")
                for _ in range(0,20):
                    xLineSpnBtn.executeAction("UP", tuple())


                xBottomSpnBtn = xParagraphDlg.getChild("spinED_BOTTOMDIST")
                for _ in range(0,20):
                    xBottomSpnBtn.executeAction("UP", tuple())

                xTopSpnBtn = xParagraphDlg.getChild("spinED_TOPDIST")
                for _ in range(0,20):
                    xTopSpnBtn.executeAction("UP", tuple())


            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaLeftMargin, 3704)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaRightMargin, 3704)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaTopMargin, 5503)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaBottomMargin, 5503)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaFirstLineIndent, 3704)

            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xParagraphDlg:


                xLeftSpnBtn = xParagraphDlg.getChild("spinED_LEFTINDENT")
                for _ in range(0,20):
                    xLeftSpnBtn.executeAction("DOWN", tuple())

                xRightSpnBtn = xParagraphDlg.getChild("spinED_RIGHTINDENT")
                for _ in range(0,20):
                    xRightSpnBtn.executeAction("DOWN", tuple())


                xLineSpnBtn = xParagraphDlg.getChild("spinED_FLINEINDENT")
                for _ in range(0,20):
                    xLineSpnBtn.executeAction("DOWN", tuple())

                xBottomSpnBtn = xParagraphDlg.getChild("spinED_BOTTOMDIST")
                for _ in range(0,20):
                    xBottomSpnBtn.executeAction("DOWN", tuple())

                xTopSpnBtn = xParagraphDlg.getChild("spinED_TOPDIST")
                for _ in range(0,20):
                    xTopSpnBtn.executeAction("DOWN", tuple())


            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaLeftMargin, 0)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaRightMargin, 0)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaTopMargin, 0)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaBottomMargin, 0)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaFirstLineIndent, 0)

            self.xUITest.executeCommand(".uno:Undo")

            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaLeftMargin, 3704)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaRightMargin, 3704)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaTopMargin, 5503)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaBottomMargin, 5503)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaFirstLineIndent, 3704)

            self.xUITest.executeCommand(".uno:Undo")

            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaLeftMargin, 0)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaRightMargin, 0)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaTopMargin, 0)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaBottomMargin, 0)
            self.assertEqual(document.CurrentSelection.getByIndex(0).ParaFirstLineIndent, 0)

            self.assertEqual(document.CurrentSelection.getByIndex(0).String, "Test for tdf79236")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
