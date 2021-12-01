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

class GoToPage_dialog(UITestCase):

   def test_go_to_page(self):
    with self.ui_test.load_file(get_url_for_data_file("3pages.odt")) as writer_doc:
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        with self.ui_test.execute_dialog_through_command(".uno:GotoPage") as xDialog:
            xPageText = xDialog.getChild("page")
            xPageText.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))

        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")

        with self.ui_test.execute_dialog_through_command(".uno:GotoPage") as xDialog:
            xPageText = xDialog.getChild("page")
            xPageText.executeAction("TYPE", mkPropertyValues({"TEXT":"3a"}))

        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "3")

        # check cancel button
        with self.ui_test.execute_dialog_through_command(".uno:GotoPage", close_button="cancel"):
            pass

        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "3")

# vim: set shiftwidth=4 softtabstop=4 expandtab: