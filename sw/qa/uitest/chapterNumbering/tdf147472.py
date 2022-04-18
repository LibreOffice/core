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
from uitest.uihelper.common import get_state_as_dict, select_pos, select_by_text, type_text

class Tdf147472(UITestCase):

    def test_tdf147472(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            with self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog", close_button="ok") as xDialog:
                xTab = xDialog.getChild("tabcontrol")
                select_pos(xTab, "0")

                # Select level "2"
                xLevel = xDialog.getChild("level")
                xLevel2 = xLevel.getChild("1")
                xLevel2.executeAction("SELECT", tuple())
                self.assertEqual("2", get_state_as_dict(xLevel)['SelectEntryText'])

                # Select numbering "1,2,3..."
                select_by_text(xDialog.getChild("numbering"), "1, 2, 3, ...")

                # Dot as suffix
                xDialog.getChild("suffix").executeAction("TYPE", mkPropertyValues({"TEXT":"."}))

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            # Enter some text
            type_text(xWriterEdit, "Outline2")

            # Apply outline level 2 (as a style)
            self.xUITest.executeCommand(".uno:StyleApply?Style:string=Heading%202&FamilyName:string=ParagraphStyles")

            # And also verify label strings in outlines
            ParagraphEnum = document.Text.createEnumeration()
            Para = ParagraphEnum.nextElement()
            self.assertEqual(Para.getPropertyValue("ListLabelString"), "1.")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
