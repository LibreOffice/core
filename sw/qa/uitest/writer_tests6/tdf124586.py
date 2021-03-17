# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text

#Bug 124586 - Crash if switch from user outline numbering to chapter numbering with same paragraph style

class tdf124586(UITestCase):
   def test_tdf124586_crash_switch_outline_numbering(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf124586.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        #Goto Tools > Chapter Numbering.
        self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xstyle = xDialog.getChild("style")
        select_by_text(xstyle, "MyHeading")
        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.assertEqual(document.Text.String[0:8], "Schritte")

        self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xstyle = xDialog.getChild("style")
        self.assertEqual(get_state_as_dict(xstyle)["SelectEntryText"], "MyHeading")
        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
