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
from uitest.uihelper.common import select_pos

class tdf115853(UITestCase):

    def test_tdf115853(self):
        self.ui_test.create_doc_in_start_center("writer")
        self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        xTabs = xDialog.getChild("tabcontrol")

        select_pos(xTabs, "2")

        xAddBtn = xDialog.getChild("add")
        xAddBtn.executeAction("CLICK", tuple())

        xNameBox1 = xDialog.getChild("namebox1")
        xNameBox1.executeAction("TYPE", mkPropertyValues({"TEXT":"Text"}))

        xAddBtn = xDialog.getChild("add")
        xAddBtn.executeAction("CLICK", tuple())

        # Without the fix in place, this test would have failed with
        # AssertionError: 'Text' != ''
        self.assertEqual("Text", get_state_as_dict(xNameBox1)['Text'])

        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
