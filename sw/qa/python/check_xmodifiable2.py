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
from org.libreoffice.unotest import UnoInProcess


class XModifiable2(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_XModifiable2(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()

        # perform unit test
        self.assertTrue(xDoc.isSetModifiedEnabled())

        self.assertTrue(xDoc.disableSetModified())
        self.assertFalse(xDoc.isSetModifiedEnabled())

        self.assertFalse(xDoc.enableSetModified())
        self.assertTrue(xDoc.isSetModifiedEnabled())

        # clean up
        xDoc.close(True)

    def test_XModifiable2_Double(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()

        # perform unit test
        self.assertTrue(xDoc.isSetModifiedEnabled())

        # try to disable
        self.assertTrue(xDoc.disableSetModified())
        self.assertFalse(xDoc.isSetModifiedEnabled())

        # try to disable twice
        self.assertFalse(xDoc.disableSetModified())
        self.assertFalse(xDoc.isSetModifiedEnabled())

        # try to disable third time
        self.assertFalse(xDoc.disableSetModified())
        self.assertFalse(xDoc.isSetModifiedEnabled())

        # try to enable
        self.assertFalse(xDoc.enableSetModified())
        self.assertTrue(xDoc.isSetModifiedEnabled())

        # try to enable twice
        self.assertTrue(xDoc.enableSetModified())
        self.assertTrue(xDoc.isSetModifiedEnabled())

        # try to enable third time
        self.assertTrue(xDoc.enableSetModified())
        self.assertTrue(xDoc.isSetModifiedEnabled())

        # clean up
        xDoc.close(True)

    def test_XModifiable2_setModified(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()

        # perform unit test
        # try to set modified flag when modification enabled
        self.assertTrue(xDoc.isSetModifiedEnabled())

        self.assertFalse(xDoc.isModified())
        xDoc.setModified(True)
        self.assertTrue(xDoc.isModified())

        xDoc.setModified(False)
        self.assertFalse(xDoc.isModified())

        # try to set modified flag when modification disabled
        self.assertTrue(xDoc.disableSetModified())
        self.assertFalse(xDoc.isSetModifiedEnabled())

        self.assertFalse(xDoc.isModified())
        xDoc.setModified(True)
        self.assertFalse(xDoc.isModified())

        self.assertFalse(xDoc.enableSetModified())
        self.assertTrue(xDoc.isSetModifiedEnabled())

        # document is still not modified
        self.assertFalse(xDoc.isModified())

        # try to set modified flag when modification enabled
        # and when we have changed the modification possibility
        self.assertTrue(xDoc.isSetModifiedEnabled())

        self.assertFalse(xDoc.isModified())
        xDoc.setModified(True)
        self.assertTrue(xDoc.isModified())

        xDoc.setModified(False)
        self.assertFalse(xDoc.isModified())

        # clean up
        xDoc.close(True)


if __name__ == '__main__':
    unittest.main()
