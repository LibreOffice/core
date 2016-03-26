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
import unohelper
from org.libreoffice.unotest import UnoInProcess

class Fdo84315(unittest.TestCase):
    _uno = None
    _xDoc = None

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        workdir = os.environ[ "WORKDIR_FOR_BUILD" ]
        cls._xDoc = cls._uno.openDoc(workdir + "/CppunitTest/fdo84315.odb")

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_fdo84315(self):
        xDoc = self.__class__._xDoc
        xDataSource = xDoc.DataSource
        xCon = xDataSource.getConnection('','')
        xStatement = xCon.createStatement()

        NUMERIC = 2
        VAR_CHAR = 12
        INTEGER = 4

        def _testQuery(column_name, expected_type):
            self.assertTrue(xResultset)
            xMeta = xResultset.MetaData
            self.assertEqual(xMeta.ColumnCount, 1)
            self.assertEqual(xResultset.findColumn(column_name), 1)
            self.assertEqual(xMeta.getColumnName(1), column_name)
            self.assertEqual(xMeta.getColumnType(1), expected_type)
            return xMeta

        def _testResultSetInteger(xResultSet, expected_values):
            while xResultset.next():
                self.assertEqual(xResultset.getInt(1), expected_values.popleft())
            self.assertEqual(len(expected_values), 0)

        def _testResultSetString(xResultSet, expected_values):
            while xResultset.next():
                self.assertEqual(xResultset.getString(1), expected_values.popleft())
            self.assertEqual(len(expected_values), 0)

        xResultset = xStatement.executeQuery('SELECT "count" FROM "test_table"')
        expected_values = deque([42, 4711])
        xMeta = _testQuery('count', NUMERIC)
        _testResultSetInteger(xResultset, expected_values)

        xResultset = xStatement.executeQuery('SELECT "name" FROM "test_table"')
        expected_values = deque(['foo', 'bar'])
        xMeta = _testQuery('name', VAR_CHAR)
        _testResultSetString(xResultset, expected_values)

        xResultset = xStatement.executeQuery('SELECT "id" FROM "test_table"')
        expected_values = deque([0, 1])
        xMeta = _testQuery('id', INTEGER)
        _testResultSetInteger(xResultset, expected_values)

        xCon.dispose()

if __name__ == '__main__':
    unittest.main()
