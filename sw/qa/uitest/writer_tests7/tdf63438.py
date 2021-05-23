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
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import type_text
import time

class tdf63438(UITestCase):

    def test_tdf63438(self):
        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        type_text(xWriterEdit, "rahul")

        time.sleep(3)

        selection = self.xUITest.executeCommand(".uno:SelectAll")
        xArgs = mkPropertyValues({"BackColor": 16776960, })
        self.xUITest.executeCommandWithParameters(".uno:CharBackgroundExt", xArgs)

        time.sleep(3)

        selection = self.xUITest.executeCommand(".uno:SelectAll")
        xArgs = mkPropertyValues({"BackColor": 16776960, })
        self.xUITest.executeCommandWithParameters(".uno:CharBackgroundExt", xArgs)

        time.sleep(3)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
