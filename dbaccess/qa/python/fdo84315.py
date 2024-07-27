#! /usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
import unittest
from collections import deque
from org.libreoffice.unotest import UnoInProcess

class Fdo84315(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        workdir = os.environ[ "WORKDIR_FOR_BUILD" ]
        cls._xDoc = cls._uno.openDocFromAbsolutePath(workdir + "/CppunitTest/fdo84315.odb")

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def __test_Query(self, column_name, expected_type, xResultset):
        self.assertTrue(xResultset)
        xMeta = xResultset.MetaData
        self.assertEqual(xMeta.ColumnCount, 1)
        self.assertEqual(xResultset.findColumn(column_name), 1)
        self.assertEqual(xMeta.getColumnName(1), column_name)
        self.assertEqual(xMeta.getColumnType(1), expected_type)
        return xMeta

    def __test_ResultSetInteger(self, xResultset, expected_values):
        while xResultset.next():
            self.assertEqual(xResultset.getInt(1), expected_values.popleft())
        self.assertEqual(len(expected_values), 0)

    def __test_ResultSetString(self, xResultset, expected_values):
        while xResultset.next():
            self.assertEqual(xResultset.getString(1), expected_values.popleft())
        self.assertEqual(len(expected_values), 0)

    def test_fdo84315(self):
        xDoc = self.__class__._xDoc
        xDataSource = xDoc.DataSource
        xCon = xDataSource.getConnection('','')
        xStatement = xCon.createStatement()

        NUMERIC = 2
        VAR_CHAR = 12
        INTEGER = 4

        xResultset = xStatement.executeQuery('SELECT "count" FROM "test_table"')
        expected_values = deque([42, 4711])
        self.__test_Query('count', NUMERIC, xResultset)
        self.__test_ResultSetInteger(xResultset, expected_values)

        xResultset = xStatement.executeQuery('SELECT "name" FROM "test_table"')
        expected_values = deque(['foo', 'bar'])
        self.__test_Query('name', VAR_CHAR, xResultset)
        self.__test_ResultSetString(xResultset, expected_values)

        xResultset = xStatement.executeQuery('SELECT "id" FROM "test_table"')
        expected_values = deque([0, 1])
        self.__test_Query('id', INTEGER, xResultset)
        self.__test_ResultSetInteger(xResultset, expected_values)

        xCon.dispose()

if __name__ == '__main__':
    unittest.main()
