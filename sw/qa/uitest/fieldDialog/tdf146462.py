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
from uitest.uihelper.common import select_pos

class Tdf146462(UITestCase):

    def test_tdf146462(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, "Header")

            self.xUITest.executeCommand(".uno:StyleApply?Style:string=Heading%201&FamilyName:string=ParagraphStyles")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField", close_button="cancel") as xDialog:
                xTab = xDialog.getChild("tabcontrol")
                select_pos(xTab, "1")

                xType = xDialog.getChild("type-ref")
                xType.getChild('2').executeAction("SELECT", tuple())
                self.assertEqual("Headings", get_state_as_dict(xType)['SelectEntryText'])

                xFormat = xDialog.getChild("format-ref")
                self.assertEqual("Page number (unstyled)", get_state_as_dict(xFormat)['SelectEntryText'])

                xSelect = xDialog.getChild("selecttip")

                # Without the fix in place, this test would have failed with
                # AssertionError: 'Header' != ''
                self.assertEqual("Header", get_state_as_dict(xSelect)['SelectEntryText'])
                self.assertEqual("1", get_state_as_dict(xSelect)['Children'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
