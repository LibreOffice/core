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

        self.checkTable(xTable,4,3)
        xDoc.close(True)

    def test_tableFromOdt(self):
        hasNestedTable = False

        xDoc = self._uno.openTemplateFromTDOC("XTextTable.odt")
        itr = iter(xDoc.Text.createEnumeration())
        for element in itr:
            if element.supportsService("com.sun.star.text.TextTable"):
                self.checkTable(element,2,3)

                # in second table, inside B1 cell we have nested table
                xCellB1 = element.getCellByName("B1")
                self.assertIsNotNone(xCellB1)
                cellContent = iter(xCellB1.Text.createEnumeration())
                for cellElement in cellContent:
                    if cellElement.supportsService("com.sun.star.text.TextTable"):
                        self.checkTable(cellElement,2,3)
                        hasNestedTable = True

        self.assertTrue(hasNestedTable)
        xDoc.close(True)

    def checkTable(self, xTable, nRow, nCol):
        # in order
        xNames = xTable.getCellNames()
        for xName in xNames:
            xCell = xTable.getCellByName(xName)
            xCursor = xTable.createCursorByCellName(xName)
            self.assertIsNotNone(xCell)
            self.assertIsNotNone(xCursor)

        # random access
        xNames = xTable.getCellNames()
        for i in random.sample(range(0, len(xNames)), len(xNames)):
            xName = xNames[i]
            xCell = xTable.getCellByName(xName)
            xCursor = xTable.createCursorByCellName(xName)
            self.assertIsNotNone(xCell)
            self.assertIsNotNone(xCursor)

        # wrong name
        xCell = xTable.getCellByName('WRONG CELL NAME')
        self.assertIsNone(xCell)

        # correct number of rows and columns
        self.assertEqual(xTable.getRows().getCount(), nRow)
        self.assertEqual(xTable.getColumns().getCount(), nCol)

if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
