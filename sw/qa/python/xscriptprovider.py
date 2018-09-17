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
import unohelper
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.script.provider import ScriptFrameworkErrorException
import uno

class TestXScriptProvider(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_getScriptApplication(self):
        xMasterScriptProviderFactory = self.createUnoService("com.sun.star.script.provider.MasterScriptProviderFactory")
        xScriptProvider = xMasterScriptProviderFactory.createScriptProvider("");
        xScript = xScriptProvider.getScript("vnd.sun.star.script:Tools.Misc.CreateNewDocument?language=Basic&location=application")
        self.assertTrue(xScript, "xScript was not loaded")

    def test_getScriptDocument(self):
        xDoc = self.__class__._uno.openTemplateFromTDOC("xscriptprovider.odt")
        xMasterScriptProviderFactory = self.createUnoService("com.sun.star.script.provider.MasterScriptProviderFactory")
        xScriptProvider = xMasterScriptProviderFactory.createScriptProvider("");
        xScript = xScriptProvider.getScript("vnd.sun.star.script:Standard.Module1.Main?language=Basic&location=document")
        self.assertTrue(xScript, "xScript was not loaded")
        xDoc.close(True)

    def test_getScriptInvalidURI(self):
        xMasterScriptProviderFactory = self.createUnoService("com.sun.star.script.provider.MasterScriptProviderFactory")
        xScriptProvider = xMasterScriptProviderFactory.createScriptProvider("");
        with self.assertRaises(ScriptFrameworkErrorException):
            xScript = xScriptProvider.getScript("invalid URI, isn't it?")

    def test_getScriptNotFound(self):
        xMasterScriptProviderFactory = self.createUnoService("com.sun.star.script.provider.MasterScriptProviderFactory")
        xScriptProvider = xMasterScriptProviderFactory.createScriptProvider("");
        with self.assertRaises(ScriptFrameworkErrorException):
            xScript = xScriptProvider.getScript("vnd.sun.star.script:NotExisting.NotExisting.NotExisting?language=Basic&location=document")

    def createUnoService(self, serviceName):
        sm = uno.getComponentContext().ServiceManager
        return sm.createInstanceWithContext(serviceName, uno.getComponentContext())

if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
