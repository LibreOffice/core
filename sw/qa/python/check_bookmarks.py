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
import os
import hashlib
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK
from random import randint
import random
import sys
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.beans import PropertyValue
from com.sun.star.container import XNamed
from com.sun.star.container import XNameAccess
from com.sun.star.frame import XStorable
from com.sun.star.lang import XMultiServiceFactory
from com.sun.star.text import XBookmarksSupplier
from com.sun.star.text import XSimpleText
from com.sun.star.text import XText
from com.sun.star.text import XTextContent
from com.sun.star.text import XTextCursor
from com.sun.star.text import XTextDocument
from com.sun.star.text import XTextRange

class BookmarkHashes(unittest.TestCase):
    def assertExpectation(self, aExpectation):
        self.assertEqual(aExpectation.m_nSetupHash, self.m_nSetupHash)
        self.assertEqual(aExpectation.m_nInsertRandomHash, self.m_nInsertRandomHash)
        self.assertEqual(aExpectation.m_nDeleteRandomHash, self.m_nDeleteRandomHash)
        self.assertEqual(aExpectation.m_nLinebreakHash, self.m_nLinebreakHash)
        self.assertEqual(aExpectation.m_nOdfReloadHash, self.m_nOdfReloadHash)
        self.assertEqual(aExpectation.m_nMsWordReloadHash, self.m_nMsWordReloadHash)

    def getBookmarksHash(xDoc):
        xBookmarks = xDoc.getBookmarks()
        buf= " "
        for sBookmarkname in xBookmarks.ElementNames:
            xBookmark = xBookmarks.getByName(sBookmarkname)
            xBookmarkAsContent = xBookmark
            buf += sBookmarkname
            buf += ':'
            buf += xBookmarkAsContent.getAnchor().getString().replace("\r\n", "\n")
            buf += ';'

        s = hashlib.sha1()
        #Unicode issue is fixed for Python3
        s.update( buf.encode('utf-8') )
        print ( s.hexdigest())
        return s.hexdigest()

class CheckBookmarks(unittest.TestCase):
    m_actualHashes = BookmarkHashes()

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        xContext = cls._uno.getContext()
        m_xMsf = xContext.ServiceManager
        cls.m_xDoc = cls._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def get20111110Expectations(self):
        result = BookmarkHashes()
        result.m_nSetupHash = "-4b0706744e8452fe1ae9d5e1c28cf70fb6194795"
        result.m_nInsertRandomHash = "25aa0fad3f4881832dcdfe658ec2efa8a1a02bc5"
        result.m_nDeleteRandomHash = "-3ec87e810b46d734677c351ad893bbbf9ea10f55"
        result.m_nLinebreakHash = "3ae08c284ea0d6e738cb43c0a8105e718a633550"
        return result

    def test_checkBookmarks(self):
        self.setupDocuments()
        self.m_actualHashes.assertExpectation(self.get20111110Expectations())

    def setupDocuments(self):
        self.setupBookmarks()
        self.m_actualHashes.m_nSetupHash = BookmarkHashes.getBookmarksHash(self.m_xDoc)
        self.insertRandomParts(200177)
        self.m_actualHashes.m_nInsertRandomHash = BookmarkHashes.getBookmarksHash(self.m_xDoc)
        self.deleteRandomParts(4711)
        self.m_actualHashes.m_nDeleteRandomHash = BookmarkHashes.getBookmarksHash(self.m_xDoc)
        self.insertLinebreaks(7)
        self.m_actualHashes.m_nLinebreakHash = BookmarkHashes.getBookmarksHash(self.m_xDoc)

    def setupBookmarks(self):
        xText = self.m_xDoc.getText()
        for nPara in range(10):
            for nBookmark in range(100):
                self.insertBookmark(
                    xText.createTextCursor(),
                    "P" + str(nPara) + "word" + str(nBookmark),
                    "P" + str(nPara) + "word" + str(nBookmark))
                xWordCursor = xText.createTextCursor()
                xWordCursor.setString(" ")
            xParaCursor = xText.createTextCursor()
            xParaCursorAsRange = xParaCursor
            xText.insertControlCharacter(xParaCursorAsRange, PARAGRAPH_BREAK, False)

    def insertRandomParts(self, seed):
        random.seed(seed)
        xCrsr = self.m_xDoc.getText().createTextCursor()
        for i in range(600):
            rnd=random.randint(0, 100)
            xCrsr.goRight(rnd, True)
            xCrsr.setString(str(random.choice(range(0,10000000))))

    def deleteRandomParts(self, seed):
        random.seed(seed)
        xCrsr = self.m_xDoc.getText().createTextCursor()
        for i in range(600):
            rnd=random.randint(0, 100)
            xCrsr.goRight(rnd, False)
            rnd=random.randint(0, 20)
            xCrsr.goRight(rnd, True) #AttributeError:'str'object has no attribute 'goRight'
            xCrsr.setString("")

    def insertLinebreaks(self, seed):
        xText = self.m_xDoc.getText()
        random.seed(seed)
        xCrsr = self.m_xDoc.getText().createTextCursor()
        for i in range(30):
            rnd=random.randint(0, 300)
            xCrsr.goRight(rnd, False)
            xCursorAsRange = xCrsr
            xText.insertControlCharacter(xCursorAsRange, PARAGRAPH_BREAK, False)

    def insertBookmark(self, crsr, name, content):
        xDocFactory = self.__class__.m_xDoc
        xBookmark = xDocFactory.createInstance("com.sun.star.text.Bookmark")
        xBookmarkAsTextContent = xBookmark
        crsr.setString(content)
        xBookmarkAsNamed = xBookmark
        xBookmarkAsNamed.setName(name)
        self.m_xDoc.getText().insertTextContent(crsr, xBookmarkAsTextContent , True)


if __name__ == '__main__':
    unittest.main()

