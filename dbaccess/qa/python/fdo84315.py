#! /usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest
from org.libreoffice.unotest import UnoInProcess

class Fdo84315(unittest.TestCase):
    _uno = None
    _xDoc = None

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xDoc = cls._uno.openBaseDoc('fdo84315.odb')

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_fdo84315(self):
        xDoc = self.__class__._xDoc
        xDataSource = xDoc.DataSource
        xCon = xDataSource.getConnection('','')
        xStatement = xCon.createStatement()
        xResultset = xStatement.executeQuery('SELECT "count" FROM "test_table"')
        if xResultset:
            while xResultset.next:
                print(xResultset.getString(1))

if __name__ == '__main__':
    unittest.main()
