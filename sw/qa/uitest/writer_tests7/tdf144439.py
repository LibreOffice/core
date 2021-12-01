# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import type_text, select_by_text, select_pos, get_state_as_dict

class tdf144439(UITestCase):

    def test_tdf144439_list(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            # Enter some text
            type_text(xWriterEdit, "List item")
            # Apply numbering
            self.xUITest.executeCommand(".uno:DefaultNumbering")
            # Increase level up to 2
            self.xUITest.executeCommand(".uno:IncrementIndent")

            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog") as xDialog:
                # Select custom tab
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")

                # Select numbering
                xNumFmt = xDialog.getChild("numfmtlb")
                select_by_text(xNumFmt, "1, 2, 3, ...")

                # Increase number of sublevels to show
                xSubLevels = xDialog.getChild("sublevels")
                xSubLevels.executeAction("UP", tuple())

            Paragraphs = document.Text.createEnumeration()
            Para1 = Paragraphs.nextElement()
            self.assertEqual(Para1.String, "List item")
            self.assertEqual(Para1.getPropertyValue("ListLabelString"), "1.1.")

    def test_tdf144439_outline(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            # Enter some text
            type_text(xWriterEdit, "Outline2")
            # Apply outline level 2 (as a style)
            self.xUITest.executeCommand(".uno:StyleApply?Style:string=Heading%202&FamilyName:string=ParagraphStyles")

            with self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog") as xDialog:
                # Select level "2"
                xLevel = xDialog.getChild("level")
                xLevel2 = xLevel.getChild("1")
                xLevel2.executeAction("SELECT", tuple())
                self.assertEqual("2", get_state_as_dict(xLevel)['SelectEntryText'])

                # Select custom tab
                xTab = xDialog.getChild("tabcontrol")
                select_pos(xTab, "0")

                # Select numbering
                xNumFmt = xDialog.getChild("numbering")
                select_by_text(xNumFmt, "1, 2, 3, ...")

                # Increase number of sublevels to show
                xSubLevels = xDialog.getChild("sublevelsnf")
                xSubLevels.executeAction("UP", tuple())

            Paragraphs = document.Text.createEnumeration()
            Para1 = Paragraphs.nextElement()
            self.assertEqual(Para1.String, "Outline2")
            # Well, this looks strange, but we asked to show 1 & 2 levels and 1st is without numbering
            self.assertEqual(Para1.getPropertyValue("ListLabelString"), ".1")

            with self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog") as xDialog:
                # Select level "1"
                xLevel = xDialog.getChild("level")
                xLevel2 = xLevel.getChild("0")
                xLevel2.executeAction("SELECT", tuple())
                self.assertEqual("1", get_state_as_dict(xLevel)['SelectEntryText'])

                # Select custom tab
                xTab = xDialog.getChild("tabcontrol")
                select_pos(xTab, "0")

                # Select numbering
                xNumFmt = xDialog.getChild("numbering")
                select_by_text(xNumFmt, "1, 2, 3, ...")

            self.assertEqual(Para1.String, "Outline2")
            self.assertEqual(Para1.getPropertyValue("ListLabelString"), "1.1")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
