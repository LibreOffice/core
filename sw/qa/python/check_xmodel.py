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
from com.sun.star.lang import IllegalArgumentException
from com.sun.star.beans import PropertyValue
import uno


class TestXModel(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_setArgs_valid(self):
        xDoc = self._uno.openEmptyWriterDoc()
        self.assertIsNotNone(xDoc)

        p1 = PropertyValue(Name="SuggestedSaveAsName", Value="prettyFileName")
        p2 = PropertyValue(Name="SuggestedSaveAsDir", Value="/my/dir")
        xDoc.setArgs([p1, p2])

        # Make sure that all properties are returned with getArgs()
        args = xDoc.getArgs()
        self.assertTrue(p1 in args)
        self.assertTrue(p2 in args)

        xDoc.close(True)

    def test_setArgs_invalid(self):
        xDoc = self._uno.openEmptyWriterDoc()
        self.assertIsNotNone(xDoc)

        # IllegalArgumentException should be thrown when setting a non-existing property
        p1 = PropertyValue(Name="PropertyNotExists", Value="doesntmatter")
        with self.assertRaises(IllegalArgumentException):
            xDoc.setArgs([p1])

        xDoc.close(True)


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
