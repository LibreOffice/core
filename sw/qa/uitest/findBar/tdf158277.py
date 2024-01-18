# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file

class tdf158277(UITestCase):

    def test_tdf158277(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf158277.fodt")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:SelectAll")

            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+f"}))
            xFind = xWriterDoc.getChild("find")

            # Without the fix in place, this test would have failed with
            # AssertionError: 'This is visible' != 'This is not visible'
            self.assertEqual("This is visible", get_state_as_dict(xFind)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
