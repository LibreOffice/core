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
        ranges = spr.createInstance("com.sun.star.sheet.SheetCellRanges")
        newRange = spr.Sheets.getByIndex(0).getCellRangeByPosition(1, 2, 1, 2)

        # When
        ranges['foo'] = newRange

        # Then
        self.assertEqual(1, len(ranges.ElementNames))

    # Tests syntax:
    #    obj[key] = val              # Insert by key
    # For:
    #    Invalid key
    def test_XNameContainer_InsertName_Invalid(self):
        # Given
        spr = self.createBlankSpreadsheet()
        ranges = spr.createInstance("com.sun.star.sheet.SheetCellRanges")
        newRange = spr.Sheets.getByIndex(0).getCellRangeByPosition(1, 2, 1, 2)

        # When / Then
        with self.assertRaises(TypeError):
            ranges[12.34] = newRange

    # Tests syntax:
    #    obj[key] = val              # Replace by key
    def test_XNameContainer_ReplaceName(self):
        # Given
        spr = self.createBlankSpreadsheet()
        ranges = spr.createInstance("com.sun.star.sheet.SheetCellRanges")
        newRange1 = spr.Sheets.getByIndex(0).getCellRangeByPosition(1, 2, 1, 2)
        newRange2 = spr.Sheets.getByIndex(0).getCellRangeByPosition(6, 6, 6, 6)

        # When
        ranges['foo'] = newRange1
        ranges['foo'] = newRange2

        # Then
        self.assertEqual(1, len(ranges.ElementNames))
        readRange = ranges['foo']
        self.assertEqual(6, readRange.CellAddress.Column)

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
