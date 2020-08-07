#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
import unittest
from org.libreoffice.unotest import UnoInProcess


class CheckRangeProperties(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    # see tdf#127534
    def test_TextRangeProperties(self):
        xDoc = CheckRangeProperties._uno.openEmptyWriterDoc()
        xBodyText = xDoc.getText()
        xCursor = xBodyText.createTextCursor()
        xBodyText.insertString(xCursor, "Hello world", 0)
        xTextRange = list(xBodyText)[0]
        pnames = [p.Name for p in xTextRange.PropertySetInfo.Properties]
        xTextRange.getPropertyValues(pnames)
        xDoc.dispose()

if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
