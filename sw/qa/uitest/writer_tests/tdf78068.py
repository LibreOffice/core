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

from uitest.uihelper.common import select_pos

class tdf78068(UITestCase):

   def test_tdf78068_format_paragraph_crash(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            #- add some text
            type_text(xWriterEdit, "Test")
            #- go to Format > Paragraph
            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")

            self.assertEqual(document.Text.String[0:4], "Test")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
