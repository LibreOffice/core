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
from com.sun.star.beans import UnknownPropertyException


class TestXControlShape(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        if cls._uno:
            cls._uno.tearDown()

    def test_getAndSetControlShape(self):
        xDoc = self.__class__._uno.openDocFromTDOC("xcontrolshape.odt")
        self.assertIsNotNone(xDoc)

        xDrawPage = xDoc.getDrawPage()
        self.assertIsNotNone(xDrawPage)

        # Date picker has control
        xShapeDatePicker = xDrawPage[0]
        self.assertIsNotNone(xShapeDatePicker)
        self.assertIsNotNone(xShapeDatePicker.getControl())

        # Combobox also has control
        xShapeCombo = xDrawPage[1]
        self.assertIsNotNone(xShapeCombo)
        self.assertIsNotNone(xShapeCombo.getControl())

        # Simple draw shape has no ControlShape
        xShapeSimple = xDrawPage[2]
        self.assertIsNotNone(xShapeSimple)
        # Shape has no XControlShape interface and we get AttributeError exception
        # during getControl() call
        with self.assertRaises(AttributeError):
            self.assertIsNone(xShapeSimple.getControl())

        xOldControlShape = xShapeCombo.getControl()

        # Combo box was a combo box and had no "Date" attribute"
        with self.assertRaises(UnknownPropertyException):
            xShapeCombo.getControl().getPropertyValue("Date")

        # We are setting new Control Shape
        xShapeCombo.setControl(xShapeDatePicker.getControl())

        # And we can get date with some value
        xDate = xShapeCombo.getControl().getPropertyValue("Date")
        self.assertIsNotNone(xDate)
        self.assertTrue(xDate.Day > 0 and xDate.Month > 0 and xDate.Year > 0)

        # Return back original controlshape
        xShapeCombo.setControl(xOldControlShape)
        # ...and ensure that date no longer available
        with self.assertRaises(UnknownPropertyException):
            xShapeCombo.getControl().getPropertyValue("Date")

        xDoc.close(True)


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
