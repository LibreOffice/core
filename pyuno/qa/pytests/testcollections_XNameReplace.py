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


# Tests behaviour of objects implementing XNameReplace using the new-style
# collection accessors
# The objects chosen have no special meaning, they just happen to implement the
# tested interfaces

class TestXNameReplace(CollectionsTestBase):

    # Tests syntax:
    #    obj[key] = val              # Replace by key
    # For:
    #    1 element
    def test_XNameReplace_ReplaceName(self):
        # Given
        doc = self.createBlankTextDocument()
        scriptName = 'macro://Standard.Module1.MySave()'
        eventProperties = (PropertyValue(Name='Script', Value=scriptName),)

        # When
        doc.Events['OnSave'] = eventProperties

        # Then
        onSave = [p.Value for p in doc.Events['OnSave'] if p.Name == 'Script'][0]
        self.assertEqual(scriptName, onSave)

    # Tests syntax:
    #    obj[key] = val              # Replace by key
    # For:
    #    Invalid key
    def test_XNameReplace_ReplaceName_Invalid(self):
        # Given
        doc = self.createBlankTextDocument()
        scriptName = 'macro://Standard.Module1.MySave()'
        eventProperties = (PropertyValue(Name='Script', Value=scriptName),)

        # When / Then
        with self.assertRaises(KeyError):
            doc.Events['qqqqq'] = eventProperties

    # Tests syntax:
    #    obj[key] = val              # Replace by key
    # For:
    #    Invalid key type
    def test_XNameReplace_ReplaceName_Invalid(self):
        # Given
        doc = self.createBlankTextDocument()
        scriptName = 'macro://Standard.Module1.MySave()'
        eventProperties = (PropertyValue(Name='Script', Value=scriptName),)

        # When / Then
        with self.assertRaises(TypeError):
            doc.Events[12.34] = eventProperties


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
