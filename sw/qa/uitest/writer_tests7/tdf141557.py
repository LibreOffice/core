# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from com.sun.star.text.TextContentAnchorType import AS_CHARACTER, AT_PARAGRAPH
import org.libreoffice.unotest
from uitest.uihelper.common import get_url_for_data_file

class tdf141557(UITestCase):
    def test_tdf141557(self):
        # load the sample file
        self.ui_test.load_file(get_url_for_data_file("tdf141557.docx"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()

        self.assertEqual(AS_CHARACTER, document.DrawPage.getByIndex(0).AnchorType)

        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        self.ui_test.wait_until_child_is_available('metricfield')

        self.ui_test.execute_dialog_through_command(".uno:TransformDialog")

        xDialog = self.xUITest.getTopFocusWindow()

        xDialog.getChild('topara').executeAction("CLICK", tuple())

        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        # Without the fix in place, at this point crash occurs.
        self.assertEqual(AT_PARAGRAPH, document.DrawPage.getByIndex(0).AnchorType)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
