#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import time
from uitest.uihelper.common import get_state_as_dict, type_text

class tdf79236(UITestCase):

    def test_paragraph(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        type_text(xWriterEdit, "Test for tdf79236")

        document = self.ui_test.get_component()

        selection = self.xUITest.executeCommand(".uno:SelectAll")

        self.assertEqual(document.CurrentSelection.getByIndex(0).ParaLeftMargin, 0)
        self.assertEqual(document.CurrentSelection.getByIndex(0).ParaRightMargin, 0)
        self.assertEqual(document.CurrentSelection.getByIndex(0).ParaTopMargin, 0)
        self.assertEqual(document.CurrentSelection.getByIndex(0).ParaBottomMargin, 0)
        self.assertEqual(document.CurrentSelection.getByIndex(0).ParaFirstLineIndent, 0)

        self.assertEqual(document.CurrentSelection.getByIndex(0).String, "Test for tdf79236")

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")

        xParagraphDlg = self.xUITest.getTopFocusWindow()


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

        xOkBtn = xParagraphDlg.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.assertEqual(document.CurrentSelection.getByIndex(0).ParaLeftMargin, 3704)
        self.assertEqual(document.CurrentSelection.getByIndex(0).ParaRightMargin, 3704)
        self.assertEqual(document.CurrentSelection.getByIndex(0).ParaTopMargin, 5503)
        self.assertEqual(document.CurrentSelection.getByIndex(0).ParaBottomMargin, 5503)
        self.assertEqual(document.CurrentSelection.getByIndex(0).ParaFirstLineIndent, 3704)

        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")

        xParagraphDlg = self.xUITest.getTopFocusWindow()

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

        xOkBtn = xParagraphDlg.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

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

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
