#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest
import uno

from org.libreoffice.unotest import UnoInProcess
from com.sun.star.script.provider import ScriptFrameworkErrorException

class TestXScriptProvider(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def setUp(self):
        xMasterScriptProviderFactory = create_master_script_provider_factory()
        self.xScriptProvider = xMasterScriptProviderFactory.createScriptProvider("")

    def tearDown(self):
        del self.xScriptProvider

    def test_get_script_application(self):
        #getScript for built-in StarBasic function
        xScript = self.xScriptProvider.getScript(
            "vnd.sun.star.script:Tools.Misc.CreateNewDocument?language=Basic&"
            "location=application")

        self.assertIsNotNone(xScript, "xScript was not loaded")

    def test_get_script_document(self):
        #getScript for StarBasic function in loaded document
        x_doc = self.__class__._uno.openTemplateFromTDOC("xscriptprovider.odt")

        xMasterScriptProviderFactory = create_master_script_provider_factory()
        xScriptProvider = xMasterScriptProviderFactory.createScriptProvider(x_doc)

        xScript = xScriptProvider.getScript(
            "vnd.sun.star.script:Standard.Module1.Main?language=Basic&"
            "location=document")

        self.assertIsNotNone(xScript, "xScript was not loaded")

        x_doc.close(True)

    def test_get_script_invalid_uri(self):
        # getScript fails with invalid URI
        with self.assertRaises(ScriptFrameworkErrorException):
            self.xScriptProvider.getScript("invalid URI, isn't it?")

    def test_get_script_not_found(self):
        # getScript fails when script not found
        with self.assertRaises(ScriptFrameworkErrorException):
            self.xScriptProvider.getScript(
                "vnd.sun.star.script:NotExisting.NotExisting.NotExisting?"
                "language=Basic&location=document")

def create_master_script_provider_factory():
    xServiceManager = uno.getComponentContext().ServiceManager

    return xServiceManager.createInstanceWithContext(
        "com.sun.star.script.provider.MasterScriptProviderFactory",
        uno.getComponentContext())

if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
