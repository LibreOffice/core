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
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos

class tdf49683(UITestCase):

#tdf 49683
   def test_open_documentProperties_tdf49683(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf49683.rtf"))
        self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")     #tab description
        xKeywordsText = xDialog.getChild("keywords")
        self.assertEqual(get_state_as_dict(xKeywordsText)["Text"], "bhavb")

        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
