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


# Tests behaviour of objects implementing both XIndexAccess and XNameAccess
# using the new-style collection accessors
# The objects chosen have no special meaning, they just happen to implement the
# tested interfaces

class TestMixedNameIndex(CollectionsTestBase):

    # Tests:
    #     Ability to access a dual XName*/XIndex* object by both name and index
    def testWriterTextTableNameAndIndex(self):
        # Given
        doc = self.createBlankTextDocument()
        textTable = doc.createInstance("com.sun.star.text.TextTable")
        textTable.initialize(2, 2)
        textTable.Name = 'foo'
        cursor = doc.Text.createTextCursor()
        doc.Text.insertTextContent(cursor, textTable, False)

        # When
        tableByName = doc.TextTables['foo']
        tableByIndex = doc.TextTables[0]

        # Then
        self.assertEqual('foo', tableByName.Name)
        self.assertEqual('foo', tableByIndex.Name)
        self.assertEqual(tableByName, tableByIndex)


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
