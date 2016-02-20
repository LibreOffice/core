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


# Tests behaviour of objects implementing XNameAccess using the new-style
# collection accessors
# The objects chosen have no special meaning, they just happen to implement the
# tested interfaces

class TestXNameAccess(CollectionsTestBase):

    # Tests syntax:
    #    num = len(obj)              # Number of keys
    # For:
    #    2 elements
    def test_XNameAccess_Len(self):
        # Given
        drw = self.createBlankDrawing()

        # When
        length = len(drw.Links)

        # Then
        self.assertEqual(2, length)

    # Tests syntax:
    #    val = obj[key]              # Access by key
    # For:
    #    1/2 elements
    def test_XNameAccess_ReadKey(self):
        # Given
        drw = self.createBlankDrawing()
        drw.DrawPages.getByIndex(0).Name = 'foo'

        # When
        link = drw.Links['foo']

        # Then
        self.assertEqual('foo', link.getName())

    # Tests syntax:
    #    val = obj[key]              # Access by key
    # For:
    #    Missing key
    def test_XNameAccess_ReadKey_Missing(self):
        # Given
        drw = self.createBlankDrawing()

        # When / Then
        with self.assertRaises(KeyError):
            link = drw.Links['foo']

    # Tests syntax:
    #    val = obj[key]              # Access by key
    # For:
    #    Invalid key type (None)
    def test_XNameAccess_ReadKey_Invalid_None(self):
        # Given
        drw = self.createBlankDrawing()

        # When / Then
        with self.assertRaises(TypeError):
            link = drw.Links[None]

    # Tests syntax:
    #    val = obj[key]              # Access by key
    # For:
    #    Invalid key type (float)
    def test_XNameAccess_ReadKey_Invalid_Float(self):
        # Given
        drw = self.createBlankDrawing()

        # When / Then
        with self.assertRaises(TypeError):
            link = drw.Links[12.34]

    # Tests syntax:
    #    val = obj[key]              # Access by key
    # For:
    #    Invalid key type (tuple)
    def test_XNameAccess_ReadKey_Invalid_Tuple(self):
        # Given
        drw = self.createBlankDrawing()

        # When / Then
        with self.assertRaises(TypeError):
            link = drw.Links[(1, 2)]

    # Tests syntax:
    #    val = obj[key]              # Access by key
    # For:
    #    Invalid key type (list)
    def test_XNameAccess_ReadKey_Invalid_List(self):
        # Given
        drw = self.createBlankDrawing()

        # When / Then
        with self.assertRaises(TypeError):
            link = drw.Links[[1, 2]]

    # Tests syntax:
    #    val = obj[key]              # Access by key
    # For:
    #    Invalid key type (dict)
    def test_XNameAccess_ReadKey_Invalid_Dict(self):
        # Given
        drw = self.createBlankDrawing()

        # When / Then
        with self.assertRaises(TypeError):
            link = drw.Links[{'a': 'b'}]

    # Tests syntax:
    #    if key in obj: ...          # Test key presence
    # For:
    #    1/2 elements
    def test_XNameAccess_In(self):
        # Given
        drw = self.createBlankDrawing()
        drw.DrawPages.getByIndex(0).Name = 'foo'

        # When
        present = 'foo' in drw.Links

        # Then
        self.assertTrue(present)

    # Tests syntax:
    #    for key in obj: ...         # Implicit iterator (keys)
    # For:
    #    2 elements
    def test_XNameAccess_ForIn(self):
        # Given
        drw = self.createBlankDrawing()
        i = 0
        for name in drw.Links.getElementNames():
            drw.Links.getByName(name).Name = 'foo' + str(i)
            i += 1

        # When
        readLinks = []
        for link in drw.Links:
            readLinks.append(link)

        # Then
        self.assertEqual(['foo0', 'foo1'], readLinks)

    # Tests syntax:
    #    itr = iter(obj)             # Named iterator (keys)
    # For:
    #    2 elements
    def test_XNameAccess_Iter(self):
        # Given
        drw = self.createBlankDrawing()

        # When
        itr = iter(drw.Links)

        # Then
        self.assertIsNotNone(next(itr))
        self.assertIsNotNone(next(itr))
        with self.assertRaises(StopIteration):
            next(itr)


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
