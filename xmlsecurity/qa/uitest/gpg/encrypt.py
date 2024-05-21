# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

from tempfile import TemporaryDirectory
import os.path


# Test for CertificateChooser using GPG.
#
# Requires the environment variable GNUPGHOME to be set correctly.
# See solenv/gbuild/UITest.mk
class GpgEncryptTest(UITestCase):
    def test_gpg_encrypt(self):
        # TODO: Maybe deduplicate with sw/qa/uitest/writer_tests8/save_with_password_test_policy.py
        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "testfile.odt")
            with self.ui_test.create_doc_in_start_center("writer") as document:
                MainWindow = self.xUITest.getTopFocusWindow()
                with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))
                    gpgencrypt = xSaveDialog.getChild("gpgencrypt")
                    gpgencrypt.executeAction("CLICK",tuple())
                    xOpen = xSaveDialog.getChild("open")
                    self.assertFalse(os.path.isfile(xFilePath))
                    with self.ui_test.execute_dialog_through_action(xOpen, "CLICK", close_button="cancel") as xSelectCertificateDialog:
                        signatures = xSelectCertificateDialog.getChild("signatures")
                        signatures.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
                    self.assertFalse(os.path.isfile(xFilePath))
                with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))
                    gpgencrypt = xSaveDialog.getChild("gpgencrypt")
                    gpgencrypt.executeAction("CLICK",tuple())
                    xOpen = xSaveDialog.getChild("open")
                    self.assertFalse(os.path.isfile(xFilePath))
                    with self.ui_test.execute_dialog_through_action(xOpen, "CLICK", close_button="ok") as xSelectCertificateDialog:
                        signatures = xSelectCertificateDialog.getChild("signatures")
                        signatures.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
                    self.assertTrue(os.path.isfile(xFilePath))


# vim: set shiftwidth=4 softtabstop=4 expandtab:
