# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import type_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from org.libreoffice.unotest import systemPathToFileUrl
from tempfile import TemporaryDirectory
import os.path

class edit_file_properties_before_saving(UITestCase):

    def change_doc_info_setting(self, enabled):
        with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:

            xPages = xDialog.getChild("pages")
            xLoadSaveEntry = xPages.getChild('1')
            xLoadSaveEntry.executeAction("EXPAND", tuple())
            xGeneralEntry = xLoadSaveEntry.getChild('0')
            xGeneralEntry.executeAction("SELECT", tuple())

            xDocInfo = xDialog.getChild("docinfo")
            if get_state_as_dict(xDocInfo)['Selected'] != enabled:
                xDocInfo.executeAction("CLICK", tuple())
            self.assertEqual(enabled, get_state_as_dict(xDocInfo)['Selected'])

    def test_tdf117895(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "tdf117895-temp.odt")

            with self.ui_test.create_doc_in_start_center("writer"):

                self.change_doc_info_setting("true")

                # Save Copy as
                with self.ui_test.execute_dialog_through_command('.uno:SaveAs', close_button="") as xDialog:
                    xFileName = xDialog.getChild('file_name')
                    xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'CTRL+A'}))
                    xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'BACKSPACE'}))
                    xFileName.executeAction('TYPE', mkPropertyValues({'TEXT': xFilePath}))

                    xOpen = xDialog.getChild("open")
                    with self.ui_test.execute_dialog_through_action(xOpen, "CLICK") as xPropertiesDialog:
                        xReadOnly = xPropertiesDialog.getChild("readonly")
                        xReadOnly.executeAction("CLICK", tuple())
                        self.assertEqual("true", get_state_as_dict(xReadOnly)['Selected'])

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as doc2:

                self.change_doc_info_setting("false")

                # Without the fix in place, this test would have failed here
                self.assertTrue(doc2.isReadonly())

    def test_tdf119206(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "tdf119206-temp.odt")

            with self.ui_test.create_doc_in_start_center("writer"):

                self.change_doc_info_setting("true")

                xWriterDoc = self.xUITest.getTopFocusWindow()
                xWriterEdit = xWriterDoc.getChild("writer_edit")
                type_text(xWriterEdit, "XXXX")

                # Close document and save
                with self.ui_test.execute_dialog_through_command('.uno:CloseDoc', close_button="") as xConfirmationDialog:
                    xSave = xConfirmationDialog.getChild("save")

                    with self.ui_test.execute_dialog_through_action(xSave, "CLICK", close_button="") as xDialog:
                        xFileName = xDialog.getChild('file_name')
                        xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'CTRL+A'}))
                        xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'BACKSPACE'}))
                        xFileName.executeAction('TYPE', mkPropertyValues({'TEXT': xFilePath}))

                        xOpen = xDialog.getChild("open")
                        with self.ui_test.execute_dialog_through_action(xOpen, "CLICK") as xPropertiesDialog:
                            # Without the fix in place, this test would have crashed here
                            xReadOnly = xPropertiesDialog.getChild("readonly")
                            xReadOnly.executeAction("CLICK", tuple())
                            self.assertEqual("true", get_state_as_dict(xReadOnly)['Selected'])

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as doc2:

                self.change_doc_info_setting("false")

                self.assertTrue(doc2.isReadonly())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
