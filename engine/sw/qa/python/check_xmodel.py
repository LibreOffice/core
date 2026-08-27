#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
import unittest
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.lang import IllegalArgumentException
from com.sun.star.beans import PropertyValue


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
        p3 = PropertyValue(Name="LockContentExtraction", Value=True)
        p4 = PropertyValue(Name="LockExport", Value=True)
        p5 = PropertyValue(Name="LockPrint", Value=True)
        p6 = PropertyValue(Name="LockSave", Value=True)
        p7 = PropertyValue(Name="LockEditDoc", Value=True)
        p8 = PropertyValue(Name="Replaceable", Value=True)
        xDoc.setArgs([p1, p2, p3, p4, p5, p6, p7, p8])

        # Make sure that all properties are returned with getArgs(); a media descriptor is
        # only a name and a value, so its PropertyValue's Handle and State mean nothing
        args = xDoc.getArgs()
        self.assertTrue(any(p1.Name == arg.Name and p1.Value == arg.Value for arg in args))
        self.assertTrue(any(p2.Name == arg.Name and p2.Value == arg.Value for arg in args))
        self.assertTrue(any(p3.Name == arg.Name and p3.Value == arg.Value for arg in args))
        self.assertTrue(any(p4.Name == arg.Name and p4.Value == arg.Value for arg in args))
        self.assertTrue(any(p5.Name == arg.Name and p5.Value == arg.Value for arg in args))
        self.assertTrue(any(p6.Name == arg.Name and p6.Value == arg.Value for arg in args))
        self.assertTrue(any(p7.Name == arg.Name and p7.Value == arg.Value for arg in args))
        self.assertTrue(any(p8.Name == arg.Name and p8.Value == arg.Value for arg in args))

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
