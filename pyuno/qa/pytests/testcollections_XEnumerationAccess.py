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


# Tests behaviour of objects implementing XEnumerationAccess using the new-style
# collection accessors
# The objects chosen have no special meaning, they just happen to implement the
# tested interfaces

class TestXEnumerationAccess(CollectionsTestBase):

    # Tests syntax:
    #    for val in obj: ...         # Implicit iterator
    # For:
    #    1 element
    def test_XEnumerationAccess_ForIn(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        paragraphs = []
        for para in doc.Text:
            paragraphs.append(para)

        # Then
        self.assertEqual(1, len(paragraphs))

    # Tests syntax:
    #    itr = iter(obj)             # Named iterator
    # For:
    #    1 element
    def test_XEnumerationAccess_Iter(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        itr = iter(doc.Text)

        # Then
        self.assertIsNotNone(next(itr))
        with self.assertRaises(StopIteration):
            next(itr)

    # Tests syntax:
    #    if val in obj: ...          # Test value presence
    # For:
    #    Present value
    def test_XEnumerationAccess_IfIn_Present(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        paragraph = doc.Text.createEnumeration().nextElement()
        result = paragraph in doc.Text

        # Then
        self.assertTrue(result)

    # Tests syntax:
    #    if val in obj: ...          # Test value presence
    # For:
    #    Absent value
    def test_XEnumerationAccess_IfIn_Absent(self):
        # Given
        doc1 = self.createBlankTextDocument()
        doc2 = self.createBlankTextDocument()

        # When
        paragraph = doc2.Text.createEnumeration().nextElement()
        result = paragraph in doc1.Text

        # Then
        self.assertFalse(result)

    # Tests syntax:
    #    if val in obj: ...          # Test value presence
    # For:
    #    None
    def test_XEnumerationAccess_IfIn_None(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        result = None in doc.Text

        # Then
        self.assertFalse(result)

    # Tests syntax:
    #    if val in obj: ...          # Test value presence
    # For:
    #    Invalid value (string)
    def test_XEnumerationAccess_IfIn_String(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        result = 'foo' in doc.Text

        # Then
        self.assertFalse(result)

    # Tests syntax:
    #    if val in obj: ...          # Test value presence
    # For:
    #    Invalid value (dict)
    def test_XEnumerationAccess_IfIn_String(self):
        # Given
        doc = self.createBlankTextDocument()

        # When / Then
        with self.assertRaises(TypeError):
            result = {} in doc.Text


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
