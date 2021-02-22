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

class tdf104795(UITestCase):

   def test_tdf104795(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf104795.odt"))
        self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        sShowSignedText = get_state_as_dict(xDialog.getChild('showsigned'))['Text']

        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        # Without the fix in place, this test would have failed with
        # AssertionError: '12/19/2016, 23:06:31, timur.davletshin' != '12/19/2016, 00:00:00, !!br0ken!!'
        self.assertEqual("12/19/2016, 23:06:31, timur.davletshin", sShowSignedText.split('@')[0])

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
