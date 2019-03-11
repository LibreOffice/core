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
from com.sun.star.beans import PropertyValue
from com.sun.star.lang import Locale
from com.sun.star.table import TableSortField
from com.sun.star.text import ControlCharacter
from os import linesep
from uno import Any


class XSortable(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_createSortDescriptor(self):
        # Initialization
        xDoc = XSortable._uno.openEmptyWriterDoc()
        xTextTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTextTable.initialize(6, 1)
        xTextCursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertTextContent(xTextCursor, xTextTable, False)

        # xSortDescriptor is a TextSortDercriptor2
        xSortDescriptor = xTextTable.createSortDescriptor()
        self.assertTrue(len(xSortDescriptor) > 0)
        attrList = []

        # A TextSortDercriptor2 is a sequence of PropertyValue objects
        for xPropertyValue in xSortDescriptor:
            attrList.append(xPropertyValue.Name)

            self.assertIsNotNone(xPropertyValue.Name)
            self.assertIsNotNone(xPropertyValue.Handle)
            self.assertIsNotNone(xPropertyValue.Value)
            self.assertIsNotNone(xPropertyValue.State)

            if xPropertyValue.Name == "SortFields":
                xTableSortFields = xPropertyValue.Value

                for xTableSortField in xTableSortFields:
                    self.assertIsNotNone(xTableSortField.Field)
                    self.assertIsNotNone(xTableSortField.IsAscending)
                    self.assertIsNotNone(xTableSortField.IsCaseSensitive)
                    self.assertIsNotNone(xTableSortField.FieldType)
                    self.assertIsNotNone(xTableSortField.CollatorLocale)
                    self.assertIsNotNone(xTableSortField.CollatorAlgorithm)

        # Check that xSortDescriptor contains the correct attributes
        self.assertIn('IsSortInTable', attrList)
        self.assertIn('Delimiter', attrList)
        self.assertIn('IsSortColumns', attrList)
        self.assertIn('MaxSortFieldsCount', attrList)
        self.assertIn('SortFields', attrList)

    def test_sort(self):
        # Initialization
        xDoc = XSortable._uno.openEmptyWriterDoc()
        xTextTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTextTable.initialize(6, 1)
        xTextCursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertTextContent(xTextCursor, xTextTable, False)

        self.prepareToSortXTextTable(xTextTable)
        self.checkSortXTextTable(xTextTable)
        self.prepareToSortXTextCursor(xDoc, xTextCursor)
        self.checkSortXTextCursor(xDoc, xTextCursor)

    def prepareToSortXTextTable(self, xTextTable):
        xTextTable.Data = [[4], [0], [3], [0], [23], [0]]
        xTextTable.getCellByName("A2").setString("b")
        xTextTable.getCellByName("A4").setString("a")
        xTextTable.getCellByName("A6").setString("ab")

    def prepareToSortXTextCursor(self, xDoc, xTextCursor):
        xDoc.Text.insertString(xTextCursor, "4", False)
        xDoc.Text.insertControlCharacter(xTextCursor,
                                         ControlCharacter.PARAGRAPH_BREAK,
                                         False)
        xDoc.Text.insertString(xTextCursor, "b", False)
        xDoc.Text.insertControlCharacter(xTextCursor,
                                         ControlCharacter.PARAGRAPH_BREAK,
                                         False)
        xDoc.Text.insertString(xTextCursor, "3", False)
        xDoc.Text.insertControlCharacter(xTextCursor,
                                         ControlCharacter.PARAGRAPH_BREAK,
                                         False)
        xDoc.Text.insertString(xTextCursor, "a", False)
        xDoc.Text.insertControlCharacter(xTextCursor,
                                         ControlCharacter.PARAGRAPH_BREAK,
                                         False)
        xDoc.Text.insertString(xTextCursor, "23", False)
        xDoc.Text.insertControlCharacter(xTextCursor,
                                         ControlCharacter.PARAGRAPH_BREAK,
                                         False)
        xDoc.Text.insertString(xTextCursor, "ab", False)

# ToDo:
#    def prepareToSortXCellRange():

    def checkSortXTextTable(self, xTextTable):
        print("\n##### checkSortXTextTable #####")
        print("\nBefore sorting:")
        self.print_xTextTable(xTextTable)

        print("\nAlphanumeric, Descending:")
        xSortDescriptor = self.buildSortDescriptor(False, False)
        xTextTable.sort(xSortDescriptor)

        self.print_xTextTable(xTextTable)

        vValues = self.extractValues(xTextTable)
        vExpectedValues = ['b', 'ab', 'a', '4', '3', '23']
        self.assertEqual(vValues, vExpectedValues)

        print("\nAlphanumeric, Ascending:")
        xSortDescriptor = self.buildSortDescriptor(False, True)
        xTextTable.sort(xSortDescriptor)

        self.print_xTextTable(xTextTable)

        vValues = self.extractValues(xTextTable)
        vExpectedValues = ['23', '3', '4', 'a', 'ab', 'b']
        self.assertEqual(vValues, vExpectedValues)

        print("\nNumeric, Descending:")
        xSortDescriptor = self.buildSortDescriptor(True, False)
        xTextTable.sort(xSortDescriptor)

        self.print_xTextTable(xTextTable)

        vValues = self.extractValues(xTextTable)
        vValuesToCompare = vValues[0:3]  # A1, A2, A3
        vExpectedValues = ['23', '4', '3']
        self.assertEqual(vValuesToCompare, vExpectedValues)

        print("\nNumeric, Ascending:")
        xSortDescriptor = self.buildSortDescriptor(True, True)
        xTextTable.sort(xSortDescriptor)

        self.print_xTextTable(xTextTable)

        vValues = self.extractValues(xTextTable)
        vValuesToCompare = vValues[3:6]  # A4, A5, A6
        vExpectedValues = ['3', '4', '23']
        self.assertEqual(vValuesToCompare, vExpectedValues)

    def checkSortXTextCursor(self, xDoc, xTextCursor):
        xTextCursor.gotoStart(True)
        ls = linesep
        print("\n##### checkSortXTextCursor #####")
        print("\nBefore sorting:")
        text = xDoc.Text.getString()

        print("\nAlphanumeric, Descending:")
        xSortDescriptor = self.buildSortDescriptor(False, False)
        xTextCursor.sort(xSortDescriptor)
        text = xDoc.Text.getString()
        print(text)

        self.assertEqual(text, "b"+ls+"ab"+ls+"a"+ls+"4"+ls+"3"+ls+"23")

        print("\nAlphanumeric, Ascending:")
        xSortDescriptor = self.buildSortDescriptor(False, True)
        xTextCursor.sort(xSortDescriptor)
        text = xDoc.Text.getString()
        print(text)

        self.assertEqual(text, "23"+ls+"3"+ls+"4"+ls+"a"+ls+"ab"+ls+"b")

        print("\nNumeric, Descending:")
        xSortDescriptor = self.buildSortDescriptor(True, False)
        xTextCursor.sort(xSortDescriptor)
        text = xDoc.Text.getString()
        print(text)

        self.assertTrue(text.startswith("23"+ls+"4"+ls+"3"+ls))

        print("\nNumeric, Ascending:")
        xSortDescriptor = self.buildSortDescriptor(True, True)
        xTextCursor.sort(xSortDescriptor)
        text = xDoc.Text.getString()
        print(text)

        self.assertTrue(text.endswith(ls+"3"+ls+"4"+ls+"23"))

    def print_xTextTable(self, xTextTable):
        print(xTextTable.getCellByName("A1").getString())
        print(xTextTable.getCellByName("A2").getString())
        print(xTextTable.getCellByName("A3").getString())
        print(xTextTable.getCellByName("A4").getString())
        print(xTextTable.getCellByName("A5").getString())
        print(xTextTable.getCellByName("A6").getString())

    def extractValues(self, xTextTable):
        vValues = []  # Contains the values of cells A1 to A6
        for i in range(0, 6):
            sCellName = "A" + str(i+1)
            vValues.append(xTextTable.getCellByName(sCellName).getString())
        return vValues

    def buildSortDescriptor(self, isNumeric, isAscending):
        if isNumeric:
            fieldType = "NUMERIC"
            collatorAlgorithm = "numeric"
        else:
            fieldType = "ALPHANUMERIC"
            collatorAlgorithm = "alphanumeric"
        tableSortField = TableSortField(Field=1,
                                        IsAscending=isAscending,
                                        FieldType=fieldType,
                                        CollatorAlgorithm=collatorAlgorithm)
        tableSortFields = Any("[]com.sun.star.table.TableSortField",
                              (tableSortField, ))
        sortDescriptor = (PropertyValue(Handle=-1,
                                        Name="IsSortInTable",
                                        Value=True),
                          PropertyValue(Handle=-1,
                                        Name="IsSortColumns",
                                        Value=False),
                          PropertyValue(Handle=-1,
                                        Name="SortFields",
                                        Value=tableSortFields))
        return sortDescriptor


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
