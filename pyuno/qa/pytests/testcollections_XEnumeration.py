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


# Tests behaviour of objects implementing XEnumeration using the new-style
# collection accessors
# The objects chosen have no special meaning, they just happen to implement the
# tested interfaces

class TestXEnumeration(CollectionsTestBase):

    # Tests syntax:
    #    for val in itr: ...         # Iteration of named iterator
    # For:
    #    1 element
    def test_XEnumeration_ForIn(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        paragraphs = []
        itr = iter(doc.Text.createEnumeration())
        for para in itr:
            paragraphs.append(para)

        # Then
        self.assertEqual(1, len(paragraphs))

    # Tests syntax:
    #    if val in itr: ...          # Test value presence
    # For:
    #    Present value
    def test_XEnumeration_IfIn_Present(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        paragraph = doc.Text.createEnumeration().nextElement()
        itr = iter(doc.Text.createEnumeration())
        result = paragraph in itr

        # Then
        self.assertTrue(result)

    # Tests syntax:
    #    if val in itr: ...          # Test value presence
    # For:
    #    Absent value
    def test_XEnumeration_IfIn_Absent(self):
        # Given
        doc1 = self.createBlankTextDocument()
        doc2 = self.createBlankTextDocument()

        # When
        paragraph = doc2.Text.createEnumeration().nextElement()
        itr = iter(doc1.Text.createEnumeration())
        result = paragraph in itr

        # Then
        self.assertFalse(result)

    # Tests syntax:
    #    if val in itr: ...          # Test value presence
    # For:
    #    None
    def test_XEnumeration_IfIn_None(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        itr = iter(doc.Text.createEnumeration())
        result = None in itr

        # Then
        self.assertFalse(result)

    # Tests syntax:
    #    if val in itr: ...          # Test value presence
    # For:
    #    Invalid value (string)
    # Note: Ideally this would raise TypeError in the same manner as for
    # XEnumerationAccess, but an XEnumeration doesn't know the type of its
    # values
    def test_XEnumeration_IfIn_String(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        itr = iter(doc.Text.createEnumeration())
        result = 'foo' in itr

        # Then
        self.assertFalse(result)


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
