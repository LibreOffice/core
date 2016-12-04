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

# SheetCellRanges instance factory
def getSheetCellRangesInstance(spr):
    return spr.createInstance("com.sun.star.sheet.SheetCellRanges")


# Tests behaviour of objects implementing XNameContainer using the new-style
# collection accessors
# The objects chosen have no special meaning, they just happen to implement the
# tested interfaces

class TestXNameContainer(CollectionsTestBase):

    # Tests syntax:
    #    obj[key] = val              # Insert by key
    # For:
    #    0->1 element
    def test_XNameContainer_InsertName(self):
        # Given
        spr = self.createBlankSpreadsheet()
        ranges = getSheetCellRangesInstance(spr)
        new_range = spr.Sheets.getByIndex(0).getCellRangeByPosition(1, 2, 1, 2)

        # When
        ranges['foo'] = new_range

        # Then
        self.assertEqual(1, len(ranges.ElementNames))

    # Tests syntax:
    #    obj[key] = val              # Insert by key
    # For:
    #    Invalid key
    def test_XNameContainer_InsertName_Invalid(self):
        # Given
        spr = self.createBlankSpreadsheet()
        ranges = getSheetCellRangesInstance(spr)
        new_range = spr.Sheets.getByIndex(0).getCellRangeByPosition(1, 2, 1, 2)

        # When / Then
        with self.assertRaises(TypeError):
            ranges[12.34] = new_range

    # Tests syntax:
    #    obj[key] = val              # Replace by key
    def test_XNameContainer_ReplaceName(self):
        # Given
        spr = self.createBlankSpreadsheet()
        ranges = getSheetCellRangesInstance(spr)
        new_range1 = spr.Sheets.getByIndex(0).getCellRangeByPosition(1, 2, 1, 2)
        new_range2 = spr.Sheets.getByIndex(0).getCellRangeByPosition(6, 6, 6, 6)

        # When
        ranges['foo'] = new_range1
        ranges['foo'] = new_range2

        # Then
        self.assertEqual(1, len(ranges.ElementNames))
        read_range = ranges['foo']
        self.assertEqual(6, read_range.CellAddress.Column)

    # Tests syntax:
    #    del obj[key]                # Delete by key
    # For:
    #    1/2 elements
    def test_XNameContainer_DelKey(self):
        # Given
        spr = self.createBlankSpreadsheet()
        spr.Sheets.insertNewByName('foo', 1)

        # When
        del spr.Sheets['foo']

        # Then
        self.assertEqual(1, len(spr.Sheets))
        self.assertFalse('foo' in spr.Sheets)

    # Tests syntax:
    #    del obj[key]                # Delete by key
    # For:
    #    Missing key
    def test_XNameContainer_DelKey_Missing(self):
        # Given
        spr = self.createBlankSpreadsheet()

        # When / Then
        with self.assertRaises(KeyError):
            del spr.Sheets['foo']

    # Tests syntax:
    #    del obj[key]                # Delete by key
    # For:
    #    Invalid key (float)
    def test_XNameContainer_DelKey_Invalid(self):
        # Given
        spr = self.createBlankSpreadsheet()

        # When / Then
        with self.assertRaises(TypeError):
            del spr.Sheets[12.34]


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
