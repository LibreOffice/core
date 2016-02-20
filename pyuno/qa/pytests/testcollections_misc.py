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


# Miscellaneous tests of the behaviour of UNO objects using the new-style
# collection accessors

class TestMisc(CollectionsTestBase):

    # Tests syntax:
    #    for val in obj: ...         # Implicit iterator
    # For:
    #    Invalid type
    def test_misc_IterateInvalidType(self):
        # Given
        doc = self.createBlankTextDocument()

        # When / Then
        with self.assertRaises(TypeError):
            for val in doc.UIConfigurationManager:
                pass

    # Tests syntax:
    #    if val in itr: ...          # Test value presence
    # For:
    #    Invalid type
    def test_misc_InInvalidType(self):
        # Given
        doc = self.createBlankTextDocument()

        # When / Then
        with self.assertRaises(TypeError):
            foo = "bar" in doc.UIConfigurationManager

    # Tests syntax:
    #    num = len(obj)              # Number of elements
    # For:
    #    Invalid type
    def test_misc_LenInvalidType(self):
        # Given
        doc = self.createBlankTextDocument()

        # When / Then
        with self.assertRaises(TypeError):
            len(doc.UIConfigurationManager)

    # Tests syntax:
    #    val = obj[0]                # Access by index
    # For:
    #    Invalid type
    def test_misc_SubscriptInvalidType(self):
        # Given
        doc = self.createBlankTextDocument()

        # When / Then
        with self.assertRaises(TypeError):
            doc.UIConfigurationManager[0]


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
