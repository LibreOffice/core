'''
  This file is part of the LibreOffice project.

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
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK
from com.sun.star.awt.FontUnderline import SINGLE

class CheckChangeColor(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xEmptyDoc = cls._uno.openEmptyWriterDoc()
        cls.RED = 0xFF0000
        cls.BLUE = 0x0000FF
        cls.GREEN = 0x008000

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_change_color(self):
        xDoc = CheckChangeColor._uno.openEmptyWriterDoc()
        xPageStyles = xDoc.StyleFamilies["PageStyles"]
        xPageStyle = xPageStyles["Standard"]

        self.assertEqual(xPageStyle.BackColor, -1)
        self.assertEqual(xPageStyle.IsLandscape, False)

        xPageStyle.BackColor = self.RED
        xPageStyle.IsLandscape = True
        self.assertEqual(xPageStyle.BackColor, self.RED)
        self.assertEqual(xPageStyle.IsLandscape, True)

        xPageStyle.GridColor = self.GREEN
        self.assertEqual(xPageStyle.GridColor, self.GREEN)

        xPageStyle.FootnoteLineColor = self.BLUE
        self.assertEqual(xPageStyle.FootnoteLineColor, self.BLUE)

        xDoc.dispose()

    def test_change_text_color(self):
        xDoc = CheckChangeColor._uno.openEmptyWriterDoc()
        cursor = xDoc.Text.createTextCursor()

        cursor.CharColor = self.RED
        self.assertEqual(cursor.CharColor, self.RED)

        cursor.CharBackColor = self.BLUE
        self.assertEqual(cursor.CharBackColor, self.BLUE)

        self.assertEqual(cursor.CharUnderlineHasColor, False)
        cursor.CharUnderlineHasColor = True
        cursor.CharUnderline = SINGLE
        cursor.CharUnderlineColor = self.GREEN
        self.assertEqual(cursor.CharUnderlineColor, self.GREEN)
        self.assertEqual(cursor.CharUnderline, SINGLE)
        self.assertEqual(cursor.CharUnderlineHasColor, True)

        xDoc.Text.insertString(cursor, "One foo to rule them all", 0)

        self.assertEqual(xDoc.getText().createTextCursor().CharColor, self.RED)
        self.assertEqual(xDoc.getText().createTextCursor().CharBackColor, self.BLUE)
        self.assertEqual(xDoc.getText().createTextCursor().CharUnderlineColor, self.GREEN)

        xDoc.dispose()

    def test_change_paragraph_color(self):
        xDoc = CheckChangeColor._uno.openEmptyWriterDoc()
        cursor = xDoc.Text.createTextCursor()

        cursor.ParaBackColor = self.RED
        self.assertEqual(cursor.ParaBackColor, self.RED)

        xDoc.Text.insertControlCharacter(cursor, PARAGRAPH_BREAK, False)
        xDoc.Text.insertString(cursor, "One foo to find them all", 0)

        self.assertEqual(xDoc.getText().createTextCursor().ParaBackColor, self.RED)

        xDoc.dispose()

if __name__ == '__main__':
    unittest.main()
