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
import random

class XTextTable(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_newTable(self):
        xDoc = XTextTable._uno.openEmptyWriterDoc()
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(4, 3)
        xCursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertTextContent(xCursor, xTable, False)
        xTable.Data = ((1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 11, 12))

        self.checkTable(xTable)

    def test_tableFromOdt(self):
        nestedTableWasFound = False

        xDoc = self._uno.openTemplateFromTDOC("XTextTable.odt")
        itr = iter(xDoc.Text.createEnumeration())
        for element in itr:
            if element.supportsService("com.sun.star.text.TextTable") :
                self.checkTable(element)

                # in second table, inside A2 cell we have nested table
                cellContent = iter(element.getCellByName("B1").Text.createEnumeration())
                for cellElement in cellContent:
                    if cellElement.supportsService("com.sun.star.text.TextTable") :
                        self.checkTable(cellElement)
                        nestedTableWasFound = True

        self.assertTrue(nestedTableWasFound)

    def checkTable(self, xTable):
        # in order
        xNames = xTable.getCellNames();
        for xName in xNames:
            xCell = xTable.getCellByName(xName)
            self.assertIsNotNone(xCell)

        # random access
        xNames = xTable.getCellNames();
        for i in random.sample(range(0,len(xNames)),len(xNames)):
            xName = xNames[i]
            xCell = xTable.getCellByName(xName)
            self.assertIsNotNone(xCell)

        # wrong name
        xCell = xTable.getCellByName('WRONG CELL NAME')
        self.assertIsNone(xCell)


if __name__ == '__main__':
    unittest.main()
