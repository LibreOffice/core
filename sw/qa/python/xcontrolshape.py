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
        cls._uno.tearDown()

    def test_get_and_set_control_shape(self):
        x_doc = self.__class__._uno.openDocFromTDOC("xcontrolshape.odt")
        self.assertIsNotNone(x_doc)

        x_draw_page = x_doc.getDrawPage()
        self.assertIsNotNone(x_draw_page)

        # Date picker has control
        x_shape_date_picker = x_draw_page[0]
        self.assertIsNotNone(x_shape_date_picker)
        self.assertIsNotNone(x_shape_date_picker.getControl())

        # Combobox also has control
        x_shape_combo = x_draw_page[1]
        self.assertIsNotNone(x_shape_combo)
        self.assertIsNotNone(x_shape_combo.getControl())

        # Simple draw shape has no ControlShape
        x_shape_simple = x_draw_page[2]
        self.assertIsNotNone(x_shape_simple)

        # Shape has no XControlShape interface and we get AttributeError exception
        # during getControl() call
        with self.assertRaises(AttributeError):
            self.assertIsNone(x_shape_simple.getControl())

        x_old_control_shape = x_shape_combo.getControl()

        # Combo box was a combo box and had no "Date" attribute"
        with self.assertRaises(UnknownPropertyException):
            x_shape_combo.getControl().getPropertyValue("Date")

        # We are setting new Control Shape
        x_shape_combo.setControl(x_shape_date_picker.getControl())

        # And we can get date with some value
        x_date = x_shape_combo.getControl().getPropertyValue("Date")
        self.assertIsNotNone(x_date)
        self.assertTrue(x_date.Day > 0 and x_date.Month > 0 and x_date.Year > 0)

        # Return back original controlshape
        x_shape_combo.setControl(x_old_control_shape)

        # ...and ensure that date no longer available
        with self.assertRaises(UnknownPropertyException):
            x_shape_combo.getControl().getPropertyValue("Date")

        x_doc.close(True)

if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
