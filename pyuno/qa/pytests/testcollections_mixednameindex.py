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
        text_table = doc.createInstance("com.sun.star.text.TextTable")
        text_table.initialize(2, 2)
        text_table.Name = 'foo'
        cursor = doc.Text.createTextCursor()
        doc.Text.insertTextContent(cursor, text_table, False)

        # When
        table_by_name = doc.TextTables['foo']
        table_by_index = doc.TextTables[0]

        # Then
        self.assertEqual('foo', table_by_name.Name)
        self.assertEqual('foo', table_by_index.Name)
        self.assertEqual(table_by_name, table_by_index)


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
