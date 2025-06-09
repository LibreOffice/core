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
from uitest.uihelper.common import select_by_text

class tdf145158(UITestCase):

    def test_tdf145158(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            xMainWindow = self.xUITest.getTopFocusWindow()
            xWriterEdit = xMainWindow.getChild("writer_edit")

            xFontsize = xMainWindow.getChild("fontsizecombobox")

            self.ui_test.wait_until_property_is_updated(xFontsize, "Text", "12 pt")
            self.assertEqual("12 pt", get_state_as_dict(xFontsize)['Text'])

            select_by_text(xFontsize, "10.5 pt")

            xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "Test"}))

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                xSizeFont = xDialog.getChild("cbWestSize")

                # Without the fix in place, this test would have failed with
                # AssertionError: '10.5 pt' != '11 pt'
                self.assertEqual("10.5 pt", get_state_as_dict(xSizeFont)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
