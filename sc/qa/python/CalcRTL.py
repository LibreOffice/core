'''
  This is file is part of the LibreOffice project.

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This file incorporates work covered by the following license notice:

    Licensed to the Apache Software Foundation (ASF) under one or more
    contributor license agreements. See the NOTICE file distributed
    with this work for additional information regarding copyright
    ownership. The ASF licenses this file to you under the Apache
    License, Version 2.0 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.apache.org/licenses/LICENSE-2.0 .
'''

import unittest
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.text.WritingMode2 import RL_TB as __WritingMode2_RL_TB__
from com.sun.star.text.WritingMode2 import LR_TB as __WritingMode2_LR_TB__

class CalcRTL(unittest.TestCase):
    xSheetDoc = None

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def testSpreadsheetProperties(self):
        self.assertTrue(self.openSpreadsheetDocument(), msg="Couldn't open document")

        set= self.getSpreadsheet()
        # Make sure there are at least 2 sheets, otherwise hiding a sheet won't work
        self.xSheetDoc.getSheets().insertNewByName("Some Sheet", 0)

        self._uno.checkProperties(set,{'IsVisible': False}, self)
        self._uno.checkProperties(set,{'IsVisible': True}, self)
        self._uno.checkProperties(set,{'PageStyle': 'Report'}, self)
        self._uno.checkProperties(set,{'PageStyle': 'Default'}, self)
        self._uno.checkProperties(set,{'TableLayout': __WritingMode2_RL_TB__}, self)
        self._uno.checkProperties(set,{'TableLayout': __WritingMode2_LR_TB__}, self)

        self.assertTrue(self.closeSpreadsheetDocument(), msg="Couldn't close document")

    def openSpreadsheetDocument(self):
        worked = True
        print("creating a sheetdocument")
        try:
            self.xSheetDoc = self._uno.openEmptyCalcDoc()
        except Exception:
            worked = False
            raise
        return worked

    def closeSpreadsheetDocument(self):
        worked = True
        print("    disposing xSheetDoc ")
        try:
            self.xSheetDoc.close(0)
        except Exception:
            worked = False
            raise
        return worked

    def getSpreadsheet(self):
        return self.xSheetDoc.getSheets().getByIndex(0)

if __name__ == '__main__':
    unittest.main()
