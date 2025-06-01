# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
import pyuno

from tempfile import TemporaryDirectory
from urllib.parse import urljoin, urlparse
from urllib.request import url2pathname
import os.path


# Test for CertificateChooser using GPG.
#
# Requires the environment variable GNUPGHOME to be set correctly.
# See solenv/gbuild/UITest.mk
class GpgEncryptTest(UITestCase):

    @classmethod
    def setUpClass(cls):
        testdir = os.getenv("TestUserDir")
        certdir = urljoin(testdir, "signing-keys")
        # this sets GNUPGHOME so do it before starting soffice
        pyuno.private_initTestEnvironmentGPG(certdir)
        # ugly: set var here again because "os.environ" is a copy :(
        os.environ["GNUPGHOME"] = url2pathname(urlparse(certdir).path)
        super().setUpClass()

    @classmethod
    def tearDownClass(cls):
        super().tearDownClass()
        pyuno.private_deinitTestEnvironmentGPG()

    def test_gpg_encrypt(self):
        # TODO: Maybe deduplicate with sw/qa/uitest/writer_tests8/save_with_password_test_policy.py
        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "testfile.odt")
            with self.ui_test.create_doc_in_start_center("writer"):
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


    def test_gpg_sign_default(self):
        with self.ui_test.set_config('/org.openoffice.UserProfile/Data/signingkey', r"93F7584031D9B74A57BB89DFC468A04FCA526A9F"):
            with TemporaryDirectory() as tempdir:
                xFilePath = os.path.join(tempdir, "testfile.odt")
                with self.ui_test.create_doc_in_start_center("writer"):
                    with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="") as xSaveDialog:
                        xFileName = xSaveDialog.getChild("file_name")
                        xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                        xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                        xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))

                        gpgsign = xSaveDialog.getChild("gpgsign")
                        # make sure the checkbox is enabled when there's a matching signing key
                        self.assertEqual('true', get_state_as_dict(gpgsign)['Enabled'])
                        gpgsign.executeAction("CLICK",tuple())

                        xOpen = xSaveDialog.getChild("open")
                        self.assertFalse(os.path.isfile(xFilePath))
                        with self.ui_test.execute_dialog_through_action(xOpen, "CLICK", close_button="") as xSignOnConsecutiveSaveDialog:
                            xSignOnConsecutiveSaveDialog.getChild("no").executeAction("CLICK", tuple())

                        # confirm signing was successful
                        self.assertTrue(self.ui_test.get_component().getPropertyValue("HasValidSignatures"))
# vim: set shiftwidth=4 softtabstop=4 expandtab:
