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

class tdf130199(UITestCase):

    def test_tdf130199(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:

            # Insert an empty para
            self.xUITest.executeCommand(".uno:InsertPara")

            # Insert an empty section
            with self.ui_test.execute_dialog_through_command(".uno:InsertSection"):
                pass

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            # Insert an extra empty para in the section
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            self.xUITest.executeCommand(".uno:InsertPara")

            self.assertEqual(1, len(document.TextSections))
            self.assertTrue(document.TextSections.Section1.IsVisible)

            with self.ui_test.execute_dialog_through_command(".uno:EditRegion") as xDialog:
                xHide = xDialog.getChild('hide')
                self.assertEqual('false', get_state_as_dict(xHide)['Selected'])

                xHide.executeAction('CLICK', tuple())

            self.assertFalse(document.TextSections.Section1.IsVisible)

            # Select everything and do not delete the section
            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.execute_dialog_through_command(".uno:Delete", close_button="no") as xDialog:
                pass

            self.assertEqual(1, len(document.TextSections))
            self.assertFalse(document.TextSections.Section1.IsVisible)

            # Select everything and delete the section
            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.execute_dialog_through_command(".uno:Delete", close_button="yes") as xDialog:
                pass

            self.assertEqual(0, len(document.TextSections))


# vim: set shiftwidth=4 softtabstop=4 expandtab:
