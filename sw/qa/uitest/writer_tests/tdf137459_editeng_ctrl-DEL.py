# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import time
from uitest.uihelper.common import get_state_as_dict, type_text
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf137459(UITestCase):

    def test_tdf137459(self):

        xMainDoc = self.ui_test.create_doc_in_start_center("writer")

        xMainWindow = self.xUITest.getTopFocusWindow()

        xwriter_edit = xMainWindow.getChild("writer_edit")
        # adding new Comment
        self.xUITest.executeCommand(".uno:InsertAnnotation")
        # wait until the comment is available
        self.ui_test.wait_until_child_is_available(xMainWindow, 'Comment1')

        xComment1 = xMainWindow.getChild("Comment1")
        xEditView1 = xComment1.getChild("editview")
        sText = "Ctrl+Del should not delete BACKWARDS"
        xEditView1.executeAction("TYPE", mkPropertyValues({"TEXT": sText}))
        self.assertEqual(get_state_as_dict(xComment1)["Text"], sText )

        xEditView1.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+DELETE"}))
        self.assertEqual(get_state_as_dict(xComment1)["Text"], sText )

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
