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

class renameSlide(UITestCase):

   def test_rename_slide(self):
        with self.ui_test.create_doc_in_start_center("impress"):
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            with self.ui_test.execute_dialog_through_command(".uno:RenamePage") as xDialog:

                name_entry = xDialog.getChild("name_entry")
                name_entry.executeAction("TYPE", mkPropertyValues({"TEXT":"NewName"}))


            #verify
            with self.ui_test.execute_dialog_through_command(".uno:RenamePage") as xDialog:

                name_entry = xDialog.getChild("name_entry")
                self.assertEqual(get_state_as_dict(name_entry)["Text"], "NewName")

            self.xUITest.executeCommand(".uno:InsertPage")

            with self.ui_test.execute_dialog_through_command(".uno:RenamePage", close_button="cancel") as xDialog:

                xOKBtn = xDialog.getChild("ok")
                self.assertEqual("true", get_state_as_dict(xOKBtn)['Enabled'])

                name_entry = xDialog.getChild("name_entry")
                name_entry.executeAction("TYPE", mkPropertyValues({"TEXT":"NewName"}))

                self.assertEqual("false", get_state_as_dict(xOKBtn)['Enabled'])


# vim: set shiftwidth=4 softtabstop=4 expandtab:
