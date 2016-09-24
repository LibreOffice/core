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


# Tests behaviour of objects implementing XIndexReplace using the new-style
# collection accessors
# The objects chosen have no special meaning, they just happen to implement the
# tested interfaces

class TestXIndexReplace(CollectionsTestBase):

    def generateTestContentIndex(self, doc):
        index = doc.createInstance("com.sun.star.text.ContentIndex")
        for i in range(10):
            styles = ('n'+str(i),)
            uno.invoke(index.LevelParagraphStyles, "replaceByIndex", (i, uno.Any("[]string", styles)))
        return index

    def generateTestTuple(self, values):
        properties = []
        for i in values:
            properties.append(('n'+str(i),),)
        return tuple(properties)

    def assignValuesTestFixture(self, doc, key, values, expected):
        # Given
        index = self.generateTestContentIndex(doc)
        toAssign = self.generateTestTuple(values)
        if not (isinstance(expected, Exception)):
            toCompare = self.generateTestTuple(expected)

        # When
        captured = None
        try:
            index.LevelParagraphStyles[key] = toAssign
        except Exception as e:
            captured = e

        # Then
        if isinstance(expected, Exception):
            # expected is exception
            self.assertNotEqual(None, captured)
            self.assertEqual(type(expected).__name__, type(captured).__name__)
        else:
            # expected is list
            self.assertEqual(None, captured)
            for i in range(10):
                self.assertEqual(toCompare[i][0], index.LevelParagraphStyles.getByIndex(i)[0])

    # Tests syntax:
    #    obj[0] = val                # Replace by index
    # For:
    #    Cases requiring sequence type coercion
    def test_XIndexReplace_ReplaceIndex(self):
        # Given
        doc = self.createBlankTextDocument()
        index = doc.createInstance("com.sun.star.text.ContentIndex")

        # When
        index.LevelParagraphStyles[0] = ('Caption',)

        # Then
        self.assertEqual(('Caption',), index.LevelParagraphStyles[0])

    # Tests syntax:
    #    obj[0] = val                # Replace by index
    # For:
    #    Invalid value (None)
    def test_XIndexReplace_ReplaceIndex_Invalid_None(self):
        # Given
        doc = self.createBlankTextDocument()
        index = doc.createInstance("com.sun.star.text.ContentIndex")

        # When / Then
        with self.assertRaises(TypeError):
            index.LevelParagraphStyles[0] = None

    # Tests syntax:
    #    obj[0] = val                # Replace by index
    # For:
    #    Invalid value (String)
    def test_XIndexReplace_ReplaceIndex_Invalid_String(self):
        # Given
        doc = self.createBlankTextDocument()
        index = doc.createInstance("com.sun.star.text.ContentIndex")

        # When / Then
        with self.assertRaises(TypeError):
            index.LevelParagraphStyles[0] = 'foo'

    # Tests syntax:
    #    obj[0] = val                # Replace by index
    # For:
    #    Invalid value (Float)
    def test_XIndexReplace_ReplaceIndex_Invalid_Float(self):
        # Given
        doc = self.createBlankTextDocument()
        index = doc.createInstance("com.sun.star.text.ContentIndex")

        # When / Then
        with self.assertRaises(TypeError):
            index.LevelParagraphStyles[0] = 12.34

    # Tests syntax:
    #    obj[0] = val                # Replace by index
    # For:
    #    Invalid value (List)
    def test_XIndexReplace_ReplaceIndex_Invalid_List(self):
        # Given
        doc = self.createBlankTextDocument()
        index = doc.createInstance("com.sun.star.text.ContentIndex")

        # When / Then
        with self.assertRaises(TypeError):
            index.LevelParagraphStyles[0] = [0, 1]

    # Tests syntax:
    #    obj[0] = val                # Replace by index
    # For:
    #    Invalid value (Dict)
    def test_XIndexReplace_ReplaceIndex_Invalid_Dict(self):
        # Given
        doc = self.createBlankTextDocument()
        index = doc.createInstance("com.sun.star.text.ContentIndex")

        # When / Then
        with self.assertRaises(TypeError):
            index.LevelParagraphStyles[0] = {'a': 'b'}

    # Tests syntax:
    #    obj[0] = val                # Replace by index
    # For:
    #    Invalid value (inconsistently typed tuple)
    def test_XIndexReplace_ReplaceIndex_Invalid_InconsistentTuple(self):
        # Given
        doc = self.createBlankTextDocument()
        index = doc.createInstance("com.sun.star.text.ContentIndex")

        # When / Then
        with self.assertRaises(TypeError):
            index.LevelParagraphStyles[0] = ('Caption', ())

    # Tests syntax:
    #    obj[2:4] = val1,val2        # Replace by slice
    # For:
    #    Cases requiring sequence type coercion
    def test_XIndexReplace_ReplaceSlice(self):
        assignMax = 12
        doc = self.createBlankTextDocument()
        t = tuple(range(10))
        for j in [x for x in range(-12, 13)] + [None]:
            for k in [x for x in range(-12, 13)] + [None]:
                key = slice(j, k)
                for l in range(assignMax):
                    assign = [y+100 for y in range(l)]
                    expected = list(range(10))
                    try:
                        expected[key] = assign
                    except Exception as e:
                        expected = e
                    if (len(expected) != 10):
                        expected = ValueError()
                    self.assignValuesTestFixture(doc, key, assign, expected)

    # Tests syntax:
    #    obj[2:4] = val1,val2        # Replace by slice
    # For:
    #    Invalid values (inconsistently value types in tuple)
    def test_XIndexReplace_ReplaceSlice_Invalid_InconsistentTuple(self):
        # Given
        doc = self.createBlankTextDocument()
        index = doc.createInstance("com.sun.star.text.ContentIndex")

        # When / Then
        with self.assertRaises(TypeError):
            index.LevelParagraphStyles[0:2] = (
                ('Caption',),
                12.34
            )

    # Tests syntax:
    #    obj[0:3:2] = val1,val2      # Replace by extended slice
    # For:
    #    Cases requiring sequence type coercion
    def test_XIndexReplace_ReplaceExtendedSlice(self):
        assignMax = 12
        doc = self.createBlankTextDocument()
        t = tuple(range(10))
        for j in [x for x in range(-12, 13)] + [None]:
            for k in [x for x in range(-12, 13)] + [None]:
                for l in [-2, -1, 2]:
                    key = slice(j, k, l)
                    for m in range(assignMax):
                        assign = [y+100 for y in range(m)]
                        expected = list(range(10))
                        try:
                            expected[key] = assign
                        except Exception as e:
                            expected = e
                        self.assignValuesTestFixture(doc, key, assign, expected)


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
