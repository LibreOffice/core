# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict

class Tdf145215(UITestCase):

    def test_tdf145215(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf145215.docx")) as writer_doc:
            with self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog") as xDialog:
                xTab = xDialog.getChild("tabcontrol")

                # Select level "4"
                xLevel = xDialog.getChild("level")
                xLevel2 = xLevel.getChild("3")
                xLevel2.executeAction("SELECT", tuple())
                self.assertEqual("4", get_state_as_dict(xLevel)['SelectEntryText'])

                # Check value for show upper levels
                xSubLevels = xDialog.getChild("sublevelsnf")
                self.assertEqual(get_state_as_dict(xSubLevels)["Text"], "1")

            # Check field value (there is only one field)
            textfields = writer_doc.getTextFields()
            textfields.refresh()
            textfield = textfields.createEnumeration().nextElement()
            self.assertTrue(textfield.supportsService("com.sun.star.text.TextField.GetReference"))
            self.assertEqual(textfield.CurrentPresentation, "1.2.1(i)")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
