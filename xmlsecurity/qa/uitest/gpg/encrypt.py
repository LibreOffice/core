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

    # should this be setUp() or setUpClass()?
    # as setUp(), any test's change to the files should be overwritten before
    # the next test, which could be an advantage.
    def setUp(self):
        testdir = os.getenv("TestUserDir")
        certdir = urljoin(testdir, "signing-keys")
        # this sets GNUPGHOME so do it before starting soffice
        pyuno.private_initTestEnvironmentGPG(certdir)
        # ugly: set var here again because "os.environ" is a copy :(
        os.environ["GNUPGHOME"] = url2pathname(urlparse(certdir).path)
        super().setUp()

    def tearDown(self):
        super().tearDown()
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


# vim: set shiftwidth=4 softtabstop=4 expandtab:
