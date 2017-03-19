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


# @unittest.skip("that seems to work")
class TestSetExpression(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xDoc = cls._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_set_expression(self):
        self.__class__._uno.checkProperties(
            self.__class__._xDoc.createInstance("com.sun.star.text.textfield.SetExpression"),
            {"NumberingType": 0,
             "Content": "foo",
             "CurrentPresentation": "bar",
             "NumberFormat": 0,
             "NumberingType": 0,
             "IsShowFormula": False,
             "IsInput": False,
             "IsVisible": True,
             "SequenceValue": 0,
             "SubType": 0,
             "Value": 1.0,
             "IsFixedLanguage": False
             },
            self
            )

if __name__ == '__main__':
    unittest.main()
