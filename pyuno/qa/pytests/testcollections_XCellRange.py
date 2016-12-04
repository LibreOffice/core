#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest
import uno

from testcollections_base import CollectionsTestBase
from com.sun.star.beans import PropertyValue
from com.sun.star.table import CellAddress

# TextTable instance factory
def getTextTableInstance(doc):
    return doc.createInstance('com.sun.star.text.TextTable')

# Tests behaviour of objects implementing XCellRange using the new-style
# collection accessors

class TestXCellRange(CollectionsTestBase):

    # TODO negative indices

    # Tests syntax:
    #    cell = cellrange[0,0]       # Access cell by indices
    # For:
    #    Spreadsheet
    #    Cell at Row 0, Col 0
    def test_XCellRange_Spreadsheet_Cell_00(self):
        # Given
        spr = self.createBlankSpreadsheet()
        sht = spr.Sheets.getByIndex(0)

        # When
        cell = sht[0, 0]

        # Then
        self.assertEqual(0, cell.CellAddress.Sheet)
        self.assertEqual(0, cell.CellAddress.Row)
        self.assertEqual(0, cell.CellAddress.Column)

    # Tests syntax:
    #    cell = cellrange[0,0]       # Access cell by indices
    # For:
    #    Text table
    #    Cell at Row 0, Col 0
    def test_XCellRange_Table_Cell_00(self):
        # Given
        doc = self.createBlankTextDocument()
        text_table = getTextTableInstance(doc)
        text_table.initialize(10, 10)
        cursor = doc.Text.createTextCursor()
        doc.Text.insertTextContent(cursor, text_table, False)
        tbl = doc.TextTables.getByIndex(0)

        # When
        cell = tbl[0, 0]

        # Then
        self.assertEqual('A1', cell.CellName)

    # Tests syntax:
    #    cell = cellrange[0,0]       # Access cell by indices
    # For:
    #    Spreadsheet
    #    Cell at Row 3, Col 7
    def test_XCellRange_Spreadsheet_Cell_37(self):
        # Given
        spr = self.createBlankSpreadsheet()
        sht = spr.Sheets.getByIndex(0)

        # When
        rng = sht[3, 7]

        # Then
        self.assertEqual(0, rng.CellAddress.Sheet)
        self.assertEqual(3, rng.CellAddress.Row)
        self.assertEqual(7, rng.CellAddress.Column)

    # Tests syntax:
    #    cell = cellrange[0,0]       # Access cell by indices
    # For:
    #    Text table
    #    Cell at Row 3, Col 7
    def test_XCellRange_Table_Cell_37(self):
        # Given
        doc = self.createBlankTextDocument()
        text_table = getTextTableInstance(doc)
        text_table.initialize(10, 10)
        cursor = doc.Text.createTextCursor()
        doc.Text.insertTextContent(cursor, text_table, False)
        tbl = doc.TextTables.getByIndex(0)

        # When
        cell = tbl[3, 7]

        # Then
        self.assertEqual('H4', cell.CellName)

    # Tests syntax:
    #    rng = cellrange[0,1:2]      # Access cell range by index,slice
    # For:
    #    Spreadsheet
    def test_XCellRange_Spreadsheet_Range_Index_Slice(self):
        # Given
        spr = self.createBlankSpreadsheet()
        sht = spr.Sheets.getByIndex(0)

        # When
        rng = sht[0, 1:3]

        # Then
        self.assertEqual(0, rng.RangeAddress.Sheet)
        self.assertEqual(0, rng.RangeAddress.StartRow)
        self.assertEqual(1, rng.RangeAddress.StartColumn)
        self.assertEqual(0, rng.RangeAddress.EndRow)
        self.assertEqual(2, rng.RangeAddress.EndColumn)

    # Tests syntax:
    #    rng = cellrange[0,1:2]      # Access cell range by index,slice
    # For:
    #    Text table
    def test_XCellRange_Table_Range_Index_Slice(self):
        # Given
        doc = self.createBlankTextDocument()
        text_table = getTextTableInstance(doc)
        text_table.initialize(10, 10)
        cursor = doc.Text.createTextCursor()
        doc.Text.insertTextContent(cursor, text_table, False)
        tbl = doc.TextTables.getByIndex(0)
        doc.lockControllers()
        tbl.DataArray = tuple(tuple(str(100 + y) for y in range(10*x, 10*x + 10)) for x in range(10))
        doc.unlockControllers()

        # When
        rng = tbl[0, 1:3]

        # Then
        self.assertEqual((('101', '102'),), rng.DataArray)

    # Tests syntax:
    #    rng = cellrange[1:2,0]      # Access cell range by slice,index
    # For:
    #    Spreadsheet
    def test_XCellRange_Spreadsheet_Range_Slice_Index(self):
        # Given
        spr = self.createBlankSpreadsheet()
        sht = spr.Sheets.getByIndex(0)

        # When
        rng = sht[1:3, 0]

        # Then
        self.assertEqual(0, rng.RangeAddress.Sheet)
        self.assertEqual(1, rng.RangeAddress.StartRow)
        self.assertEqual(0, rng.RangeAddress.StartColumn)
        self.assertEqual(2, rng.RangeAddress.EndRow)
        self.assertEqual(0, rng.RangeAddress.EndColumn)

    # Tests syntax:
    #    rng = cellrange[1:2,0]      # Access cell range by index,slice
    # For:
    #    Text table
    def test_XCellRange_Table_Range_Slice_Index(self):
        # Given
        doc = self.createBlankTextDocument()
        text_table = getTextTableInstance(doc)
        text_table.initialize(10, 10)
        cursor = doc.Text.createTextCursor()
        doc.Text.insertTextContent(cursor, text_table, False)
        tbl = doc.TextTables.getByIndex(0)
        doc.lockControllers()
        tbl.DataArray = tuple(tuple(str(100 + y) for y in range(10*x, 10*x + 10)) for x in range(10))
        doc.unlockControllers()

        # When
        rng = tbl[1:3, 0]

        # Then
        self.assertEqual((('110',), ('120',)), rng.DataArray)

    # Tests syntax:
    #    rng = cellrange[0:1,2:3]    # Access cell range by slices
    # For:
    #    Spreadsheet
    def test_XCellRange_Spreadsheet_Range_Slices(self):
        # Given
        spr = self.createBlankSpreadsheet()
        sht = spr.Sheets.getByIndex(0)

        # When
        rng = sht[1:3, 3:5]

        # Then
        self.assertEqual(0, rng.RangeAddress.Sheet)
        self.assertEqual(1, rng.RangeAddress.StartRow)
        self.assertEqual(3, rng.RangeAddress.StartColumn)
        self.assertEqual(2, rng.RangeAddress.EndRow)
        self.assertEqual(4, rng.RangeAddress.EndColumn)

    # Tests syntax:
    #    rng = cellrange[0:1,2:3]    # Access cell range by slices
    # For:
    #    Spreadsheet
    #    Zero rows/columns
    def test_XCellRange_Spreadsheet_Range_Slices_Invalid(self):
        # Given
        spr = self.createBlankSpreadsheet()
        sht = spr.Sheets.getByIndex(0)

        # When / Then
        with self.assertRaises(KeyError):
            rng = sht[1:1, 3:5]
        with self.assertRaises(KeyError):
            rng = sht[1:3, 3:3]

    # Tests syntax:
    #    rng = cellrange[0:1,2:3]    # Access cell range by slices
    # For:
    #    Text table
    def test_XCellRange_Table_Range_Slices(self):
        # Given
        doc = self.createBlankTextDocument()
        text_table = getTextTableInstance(doc)
        text_table.initialize(10, 10)
        cursor = doc.Text.createTextCursor()
        doc.Text.insertTextContent(cursor, text_table, False)
        tbl = doc.TextTables.getByIndex(0)
        doc.lockControllers()
        tbl.DataArray = tuple(tuple(str(100 + y) for y in range(10*x, 10*x + 10)) for x in range(10))
        doc.unlockControllers()

        # When
        rng = tbl[1:3, 3:5]

        # Then
        self.assertEqual((('113', '114'), ('123', '124')), rng.DataArray)

    # Tests syntax:
    #    rng = cellrange['A1:B2']    # Access cell range by descriptor
    # For:
    #    Spreadsheet
    def test_XCellRange_Spreadsheet_Range_Descriptor(self):
        # Given
        spr = self.createBlankSpreadsheet()
        sht = spr.Sheets.getByIndex(0)

        # When
        rng = sht['A3:B4']

        # Then
        self.assertEqual(0, rng.RangeAddress.Sheet)
        self.assertEqual(2, rng.RangeAddress.StartRow)
        self.assertEqual(0, rng.RangeAddress.StartColumn)
        self.assertEqual(3, rng.RangeAddress.EndRow)
        self.assertEqual(1, rng.RangeAddress.EndColumn)

    # Tests syntax:
    #    rng = cellrange['A1:B2']    # Access cell range by descriptor
    # For:
    #    Table
    def test_XCellRange_Table_Range_Descriptor(self):
        # Given
        doc = self.createBlankTextDocument()
        text_table = getTextTableInstance(doc)
        text_table.initialize(10, 10)
        cursor = doc.Text.createTextCursor()
        doc.Text.insertTextContent(cursor, text_table, False)
        tbl = doc.TextTables.getByIndex(0)
        doc.lockControllers()
        tbl.DataArray = tuple(tuple(str(100 + y) for y in range(10*x, 10*x + 10)) for x in range(10))
        doc.unlockControllers()

        # When
        rng = tbl['A3:B4']

        # Then
        self.assertEqual((('120', '121'), ('130', '131')), rng.DataArray)

    # Tests syntax:
    #    rng = cellrange['Name']     # Access cell range by name
    # For:
    #    Spreadsheet
    def test_XCellRange_Spreadsheet_Range_Name(self):
        # Given
        spr = self.createBlankSpreadsheet()
        sht = spr.Sheets.getByIndex(0)
        expr = '$' + sht.Name + '.$C2:F10'
        addr = CellAddress(Sheet=0, Row=1, Column=2)
        sht.NamedRanges.addNewByName('foo', expr, addr, 0)

        # When
        rng = sht['foo']

        # Then
        self.assertEqual(0, rng.RangeAddress.Sheet)
        self.assertEqual(1, rng.RangeAddress.StartRow)
        self.assertEqual(2, rng.RangeAddress.StartColumn)
        self.assertEqual(9, rng.RangeAddress.EndRow)
        self.assertEqual(5, rng.RangeAddress.EndColumn)

    # Tests syntax:
    #    rng = cellrange[0]          # Access cell range by row index
    # For:
    #    Spreadsheet
    def test_XCellRange_Spreadsheet_Range_RowIndex(self):
        # Given
        spr = self.createBlankSpreadsheet()
        sht = spr.Sheets.getByIndex(0)

        # When
        rng = sht[0]

        # Then
        self.assertEqual(0, rng.RangeAddress.Sheet)
        self.assertEqual(0, rng.RangeAddress.StartRow)
        self.assertEqual(0, rng.RangeAddress.StartColumn)
        self.assertEqual(0, rng.RangeAddress.EndRow)
        self.assertEqual(1023, rng.RangeAddress.EndColumn)

    # Tests syntax:
    #    rng = cellrange[0,:]        # Access cell range by row index
    # For:
    #    Spreadsheet
    def test_XCellRange_Spreadsheet_Range_RowIndex_FullSlice(self):
        # Given
        spr = self.createBlankSpreadsheet()
        sht = spr.Sheets.getByIndex(0)

        # When
        rng = sht[0, :]

        # Then
        self.assertEqual(0, rng.RangeAddress.Sheet)
        self.assertEqual(0, rng.RangeAddress.StartRow)
        self.assertEqual(0, rng.RangeAddress.StartColumn)
        self.assertEqual(0, rng.RangeAddress.EndRow)
        self.assertEqual(1023, rng.RangeAddress.EndColumn)

    # Tests syntax:
    #    rng = cellrange[:,0]        # Access cell range by column index
    # For:
    #    Spreadsheet
    def test_XCellRange_Spreadsheet_Range_FullSlice_ColumnIndex(self):
        # Given
        spr = self.createBlankSpreadsheet()
        sht = spr.Sheets.getByIndex(0)

        # When
        rng = sht[:, 0]

        # Then
        self.assertEqual(0, rng.RangeAddress.Sheet)
        self.assertEqual(0, rng.RangeAddress.StartRow)
        self.assertEqual(0, rng.RangeAddress.StartColumn)
        self.assertEqual(1048575, rng.RangeAddress.EndRow)
        self.assertEqual(0, rng.RangeAddress.EndColumn)


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
