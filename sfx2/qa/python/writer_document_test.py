# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest
import unohelper
import os
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.beans import PropertyValue

class WriterDocumentTest(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xDoc = cls._uno.openEmptyWriterDoc()
        cls.s_blindText = """Lorem ipsum dolor. Sit amet penatibus. A cum turpis. Aenean ac eu. 
        Ligula est urna nulla vestibulum ullamcorper. Nec sit in amet tincidunt mus.
        Tellus sagittis mi. Suscipit cursus in vestibulum in eros ipsum felis cursus lectus
        nunc quis condimentum in risus nec wisi aenean luctus hendrerit magna habitasse commodo orci.
        Nisl etiam quis. Vestibulum justo eleifend aliquet luctus sed turpis volutpat ullamcorper 
        aliquam penatibus sagittis pede tincidunt egestas. Nibh massa lectus. Sem mattis purus morbi
        scelerisque turpis donec urna phasellus. Quis at lacus. Viverra mauris mollis. 
        Dolor tincidunt condimentum.'
        """

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def doSingleModification(self):
        textDoc = self.__class__._xDoc
        docText = textDoc.getText()
        docText.setString( self.s_blindText )

    def test_verifyInitialDocumentState(self):
        textDoc = self.__class__._xDoc
        docText = textDoc.getText()
        self.assertEqual( docText.getString(), "" , "document should be empty")

    def test_verifySingleModificationDocumentState(self):
        textDoc = self.__class__._xDoc
        docText = textDoc.getText()
        self.assertNotEqual(self.s_blindText , docText.getString(), "blind text not found")

    def doMultipleModifications(self):
        textDoc = self.__class__._xDoc
        docText = textDoc.getText()
        expectedUndoActions = 0
        cursor = docText.createTextCursor()
        textTable = textDoc.createInstance("com.sun.star.text.TextTable")
        textTable.initialize( 3, 3 )
        textTable.setPropertyValue( "BackColor", 0xCCFF44 )

        docText.insertTextContent( cursor, textTable, false )
        expectedUndoActions += 1

        centerCell = textTable.getCellByPosition( 1, 1 )
        cellText = centerCell.createTextCursor()
        cellText.setString( "Undo Manager API Test" )
        expectedUndoActions += 1

        cellProps = centerCell.setPropertyValue( "BackColor", 0x44CCFF )
        expectedUndoActions += 1

        return expectedUndoActions

if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
