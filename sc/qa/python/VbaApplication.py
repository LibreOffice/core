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
from com.sun.star.awt import Toolkit


# Tests that we can access ooo.vba.excel.Application from Python. See tdf#173293.
class VbaApplication(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

        cls.xContext = cls._uno.getContext()
        cls.xDoc = cls._uno.openEmptyCalcDoc()
        cls.xSheet = cls.xDoc.getSheets()[0]

        # This is needed for the VBA Application to find the doc
        cls.xDoc.createInstance("ooo.vba.VBAGlobals")

        xServiceManager = cls.xContext.getServiceManager()
        cls.xApplication = xServiceManager.createInstanceWithContext(
            "ooo.vba.excel.Application", cls.xContext)

    @classmethod
    def tearDownClass(cls):
        cls.xApplication = None
        cls.xSheet = None
        cls.xDoc.dispose()
        cls.xDoc = None
        cls.xContext = None
        cls._uno.tearDown()

    def testSelection(self):
        # Tests that we can access the Selection property

        # Select a square of 10×10 cells in the topleft
        xController = self.xDoc.getCurrentController()
        xRange = self.xSheet.getCellRangeByName("A1:J10")
        xController.select(xRange)

        # Use the VBA API to fill in the selection
        self.xApplication.Selection.Value = "x"

        # Check that the values were filled in
        self.assertEqual(xRange.getCellByPosition(0, 0).getString(), "x")
        self.assertEqual(xRange.getCellByPosition(9, 0).getString(), "x")
        self.assertEqual(xRange.getCellByPosition(0, 9).getString(), "x")
        self.assertEqual(xRange.getCellByPosition(9, 9).getString(), "x")

    def testFunction(self):
        # Tests that we can call a function
        self.assertEqual(int(self.xApplication.CentimetersToPoints(1)), 28)

    def testWorksheetFunction(self):
        # Tests that we can call a dynamic function via XInvocation
        self.assertEqual(self.xApplication.Concat("one", " ", "potato"), "one potato")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
