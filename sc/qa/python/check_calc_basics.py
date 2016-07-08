#! /usr/bin/env python
# -*- Mode: python; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
import math
import unittest
from org.libreoffice.unotest import UnoInProcess

class CheckCalcBasics(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()


    def test_open_empty_calc_doc(self):
        xDoc = self._uno.openEmptyCalcDoc()

        firstSheet = xDoc.getSheets()[0]

        with self.assertRaises(IndexError):
            xDoc.getSheets()[1]

        self.assertTrue(firstSheet.IsVisible)

        self.assertEqual("Default", firstSheet.PageStyle)

        self.assertEqual(0, firstSheet.TableLayout)

        self.assertTrue(firstSheet.AutomaticPrintArea)

        xDoc.dispose()
