#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest
import uno

from org.libreoffice.unotest import pyuno
from com.sun.star.beans import PropertyValue

testEnvironmentInitialized = False

class CollectionsTestBase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.context = pyuno.getComponentContext()
        global testEnvironmentInitialized
        if not testEnvironmentInitialized:
            pyuno.private_initTestEnvironment()
            testEnvironmentInitialized = True

    def setUp(self):
        self._components = []

    def tearDown(self):
        for component in self._components:
            try:
                component.close(True)
            except Exception:
                pass

    def createHiddenWindow(self, url):
        service_manager = self.context.ServiceManager
        desktop = service_manager.createInstanceWithContext('com.sun.star.frame.Desktop', self.context)
        load_props = (
          PropertyValue(Name='Hidden', Value=True),
          PropertyValue(Name='ReadOnly', Value=False)
        )
        component = desktop.loadComponentFromURL(url, '_blank', 0, load_props)
        return component

    def createBlankTextDocument(self):
        component = self.createHiddenWindow('private:factory/swriter')
        self._components.append(component)
        return component

    def createBlankSpreadsheet(self):
        component = self.createHiddenWindow('private:factory/scalc')
        self._components.append(component)
        return component

    def createBlankDrawing(self):
        component = self.createHiddenWindow('private:factory/sdraw')
        self._components.append(component)
        return component

# vim:set shiftwidth=4 softtabstop=4 expandtab:
