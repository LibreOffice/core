#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest

from inspect import isclass
from testcollections_base import CollectionsTestBase


# Footnote instance factory
def getFootnoteInstance(doc):
    return doc.createInstance("com.sun.star.text.Footnote")

# Tests behaviour of objects implementing XIndexAccess using the new-style
# collection accessors
# The objects chosen have no special meaning, they just happen to implement the
# tested interfaces

class TestXIndexAccess(CollectionsTestBase):

    def insertTestFootnotes(self, doc, count):
        cursor = doc.Text.createTextCursor()
        for i in range(count):
            footnote = getFootnoteInstance(doc)
            footnote.Label = 'n'+str(i)
            doc.Text.insertTextContent(cursor, footnote, 0)

    def readValuesTestFixture(self, doc, count, key, expected):
        # Given
        doc.Text.setString('')
        self.insertTestFootnotes(doc, count)

        # When
        captured = None
        try:
            actual = doc.Footnotes[key]
        except Exception as e:
            captured = e

        # Then
        if isclass(expected) and issubclass(expected, Exception):
            # expected is exception
            self.assertNotEqual(None, captured)
            self.assertEqual(expected.__name__, type(captured).__name__)
        elif type(expected) is tuple:
            # expected is tuple
            self.assertEqual(None, captured)
            self.assertTrue(type(actual) is tuple)
            self.assertEqual(len(expected), len(actual))
            for i in range(len(expected)):
                self.assertEqual('n'+str(expected[i]), actual[i].Label)
        else:
            # expected is value
            self.assertEqual(None, captured)
            self.assertTrue(type(actual) is not tuple)
            self.assertEqual('n'+str(expected), actual.Label)

    # Tests syntax:
    #    num = len(obj)              # Number of elements
    # For:
    #    length = 0
    def test_XIndexAccess_Len_0(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        count = len(doc.Footnotes)

        # Then
        self.assertEqual(0, count)

        doc.close(True)

    # Tests syntax:
    #    num = len(obj)              # Number of elements
    # For:
    #    length = 1
    def test_XIndexAccess_Len_1(self):
        # Given
        doc = self.createBlankTextDocument()
        cursor = doc.Text.createTextCursor()
        footnote = getFootnoteInstance(doc)
        doc.Text.insertTextContent(cursor, footnote, 0)

        # When
        count = len(doc.Footnotes)

        # Then
        self.assertEqual(1, count)

        doc.close(True)

    # Tests syntax:
    #    val = obj[0]                # Access by index
    # For:
    #    Single indices
    def test_XIndexAccess_ReadIndex_Single(self):
        doc = self.createBlankTextDocument()
        self.readValuesTestFixture(doc, 0, -1,         IndexError)
        self.readValuesTestFixture(doc, 0, 0,          IndexError)
        self.readValuesTestFixture(doc, 0, 1,          IndexError)
        self.readValuesTestFixture(doc, 1, -3,         IndexError)
        self.readValuesTestFixture(doc, 1, -2,         IndexError)
        self.readValuesTestFixture(doc, 1, -1,         0)
        self.readValuesTestFixture(doc, 1, 0,          0)
        self.readValuesTestFixture(doc, 1, 1,          IndexError)
        self.readValuesTestFixture(doc, 1, 2,          IndexError)
        self.readValuesTestFixture(doc, 2, -4,         IndexError)
        self.readValuesTestFixture(doc, 2, -3,         IndexError)
        self.readValuesTestFixture(doc, 2, -2,         0)
        self.readValuesTestFixture(doc, 2, -1,         1)
        self.readValuesTestFixture(doc, 2, 0,          0)
        self.readValuesTestFixture(doc, 2, 1,          1)
        self.readValuesTestFixture(doc, 2, 2,          IndexError)
        self.readValuesTestFixture(doc, 2, 3,          IndexError)
        doc.close(True)

    def test_XIndexAccess_ReadIndex_Single_Invalid(self):
        doc = self.createBlankTextDocument()
        self.readValuesTestFixture(doc, 0, None,       TypeError)
        self.readValuesTestFixture(doc, 0, 'foo',      TypeError)
        self.readValuesTestFixture(doc, 0, 12.34,      TypeError)
        self.readValuesTestFixture(doc, 0, (0, 1),     TypeError)
        self.readValuesTestFixture(doc, 0, [0, 1],     TypeError)
        self.readValuesTestFixture(doc, 0, {'a': 'b'}, TypeError)
        doc.close(True)

    # Tests syntax:
    #    val1,val2 = obj[2:4]        # Access by slice
    def test_XIndexAccess_ReadSlice(self):
        doc = self.createBlankTextDocument()
        test_max = 4
        for i in range(test_max):
            t = tuple(range(i))
            for j in [x for x in range(-test_max-2, test_max+3)] + [None]:
                for k in [x for x in range(-test_max-2, test_max+3)] + [None]:
                    key = slice(j, k)
                    expected = t[key]
                    self.readValuesTestFixture(doc, i, key, expected)
        doc.close(True)

    # Tests syntax:
    #    val1,val2 = obj[0:3:2]      # Access by extended slice
    def test_XIndexAccess_ReadExtendedSlice(self):
        doc = self.createBlankTextDocument()
        test_max = 4
        for i in range(test_max):
            t = tuple(range(i))
            for j in [x for x in range(-test_max-2, test_max+3)] + [None]:
                for k in [x for x in range(-test_max-2, test_max+3)] + [None]:
                    for l in [-2, -1, 2]:
                        key = slice(j, k, l)
                        expected = t[key]
                        self.readValuesTestFixture(doc, i, key, expected)
        doc.close(True)

    # Tests syntax:
    #    if val in obj: ...          # Test value presence
    # For:
    #    Present element
    def test_XIndexAccess_In_Present(self):
        # Given
        doc = self.createBlankTextDocument()
        cursor = doc.Text.createTextCursor()
        footnote = getFootnoteInstance(doc)
        footnote.setLabel('foo')
        doc.Text.insertTextContent(cursor, footnote, 0)
        footnote = doc.Footnotes[0]

        # When
        present = footnote in doc.Footnotes

        # Then
        self.assertTrue(present)

        doc.close(True)

    # Tests syntax:
    #    if val in obj: ...          # Test value presence
    # For:
    #    None
    def test_XIndexAccess_In_None(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        present = None in doc.Footnotes

        # Then
        self.assertFalse(present)

        doc.close(True)

    # Tests syntax:
    #    if val in obj: ...          # Test value presence
    # For:
    #    Absent element (string)
    def test_XIndexAccess_In_String(self):
        # Given
        doc = self.createBlankTextDocument()

        # When / Then
        present = "foo" in doc.Footnotes

        # Then
        self.assertFalse(present)

        doc.close(True)

    # Tests syntax:
    #    if val in obj: ...          # Test value presence
    # For:
    #    Absent element (dict)
    def test_XIndexAccess_In_Dict(self):
        # Given
        doc = self.createBlankTextDocument()

        # When / Then
        with self.assertRaises(TypeError):
            _ = {} in doc.Footnotes

        doc.close(True)

    # Tests syntax:
    #    for val in obj: ...         # Implicit iterator (values)
    # For:
    #    0 elements
    def test_XIndexAccess_ForIn_0(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        read_footnotes = []
        for f in doc.Footnotes:
            read_footnotes.append(f)

        # Then
        self.assertEqual(0, len(read_footnotes))

        doc.close(True)

    # Tests syntax:
    #    for val in obj: ...         # Implicit iterator (values)
    # For:
    #    1 element
    def test_XIndexAccess_ForIn_1(self):
        # Given
        doc = self.createBlankTextDocument()
        cursor = doc.Text.createTextCursor()
        footnote = getFootnoteInstance(doc)
        footnote.setLabel('foo')
        doc.Text.insertTextContent(cursor, footnote, 0)

        # When
        read_footnotes = []
        for f in doc.Footnotes:
            read_footnotes.append(f)

        # Then
        self.assertEqual(1, len(read_footnotes))
        self.assertEqual('foo', read_footnotes[0].Label)

        doc.close(True)

    # Tests syntax:
    #    for val in obj: ...         # Implicit iterator (values)
    # For:
    #    2 elements
    def test_XIndexAccess_ForIn_2(self):
        # Given
        doc = self.createBlankTextDocument()
        cursor = doc.Text.createTextCursor()
        footnote1 = getFootnoteInstance(doc)
        footnote2 = getFootnoteInstance(doc)
        footnote1.setLabel('foo')
        footnote2.setLabel('bar')
        doc.Text.insertTextContent(cursor, footnote1, 0)
        doc.Text.insertTextContent(cursor, footnote2, 0)

        # When
        read_footnotes = []
        for f in doc.Footnotes:
            read_footnotes.append(f)

        # Then
        self.assertEqual(2, len(read_footnotes))
        self.assertEqual('foo', read_footnotes[0].Label)
        self.assertEqual('bar', read_footnotes[1].Label)

        doc.close(True)

    # Tests syntax:
    #    itr = iter(obj)             # Named iterator (values)
    # For:
    #    1 element
    def test_XIndexAccess_Iter_0(self):
        # Given
        doc = self.createBlankTextDocument()
        cursor = doc.Text.createTextCursor()
        footnote = getFootnoteInstance(doc)
        footnote.setLabel('foo')
        doc.Text.insertTextContent(cursor, footnote, 0)

        # When
        itr = iter(doc.Footnotes)

        # Then
        self.assertIsNotNone(next(itr))
        with self.assertRaises(StopIteration):
            next(itr)

        doc.close(True)


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
