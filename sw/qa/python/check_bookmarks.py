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
import unohelper
import os
from org.libreoffice.unotest import UnoInProcess
import hashlib
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK
from random import randint
import random
import sys
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
        buf = ""
        xBookmarks = xDoc.Bookmarks
        for sBookmarkname in xBookmarks.ElementNames:
            xBookmark = xBookmarks.getByName(sBookmarkname)
            buf += sBookmarkname
            buf += ':'
            buf += xBookmark.Anchor.String
            buf += ';'
        s = hashlib.sha1()
        # TODO: fix Python3 unicode complains
        # s.update(buf)
        return s.digest()

class CheckBookmarks(unittest.TestCase):
    _uno = None
    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def get20111110Expectations(self):
        result = BookmarkHashes()
        #TODO: adjust values
        result.m_nSetupHash = int("-4b0706744e8452fe1ae9d5e1c28cf70fb6194795",16)
        result.m_nInsertRandomHash = int("25aa0fad3f4881832dcdfe658ec2efa8a1a02bc5",16)
        result.m_nDeleteRandomHash = int("-3ec87e810b46d734677c351ad893bbbf9ea10f55",16)
        result.m_nLinebreakHash = int("3ae08c284ea0d6e738cb43c0a8105e718a633550",16)
        result.m_nOdfReloadHash = int("3ae08c284ea0d6e738cb43c0a8105e718a633550",16)
        return result

    def test_checkBookmarks(self):
        self.setupDocuments()
        self.m_actualHashes.assertExpectation(self.get20111110Expectations())

    def setupDocuments(self):
        self.m_xDoc = self.__class__._uno.openEmptyWriterDoc()
        self.m_actualHashes = BookmarkHashes()
        self.setupBookmarks()
        self.m_actualHashes.m_nSetupHash = BookmarkHashes.getBookmarksHash(self.m_xDoc)
        self.insertRandomParts(200177)
        self.m_actualHashes.m_nInsertRandomHash = BookmarkHashes.getBookmarksHash(self.m_xDoc)
        self.deleteRandomParts(4711)
        self.m_actualHashes.m_nDeleteRandomHash = BookmarkHashes.getBookmarksHash(self.m_xDoc)
        self.insertLinebreaks(7)
        self.m_actualHashes.m_nLinebreakHash = BookmarkHashes.getBookmarksHash(self.m_xDoc)
        #m_xOdfReloadedDoc = reloadFrom("writer8", "odf")
        #self.m_.actualHashes.m_nOdfReloadHash = BookmarkHashes.getBookmarksHash(m_xOdfReloadedDoc)

    def setupBookmarks(self):
        xText = self.m_xDoc.Text
        for nPara in range(10):
            for nBookmark in range(100):
                self.insertBookmark(
                    xText.createTextCursor(),
                    "P" + str(nPara) + "word" + str(nBookmark),
                    "P" + str(nPara) + "word" + str(nBookmark))
                xWordCrsr = xText.createTextCursor()
                xWordCrsr.String = " "
            xParaCrsr = xText.createTextCursor()
            xText.insertControlCharacter(xParaCrsr, PARAGRAPH_BREAK, False)

    def insertRandomParts(self, seed):
        random.seed(seed)
        xCrsr = self.m_xDoc.getText().createTextCursor()
        for i in range(600):
            rnd=random.randint(0, 100)
            xCrsr.goRight(rnd, False)
            xCrsr.String = random.choice(range(0,10000000))

    def deleteRandomParts(self, seed):
        random.seed(seed)
        xCrsr = self.m_xDoc.getText().createTextCursor()
        for i in range(600):
            rnd=random.randint(0, 100)
            xCrsr.goRight(rnd, False)
            rnd=random.randint(0, 20)
            xCrsr.goRight(rnd, True)
            xCrsr.String = ""

    def insertLinebreaks(self, seed):
        xText = self.m_xDoc.Text
        random.seed(seed)
        xCrsr = self.m_xDoc.getText().createTextCursor()
        for i in range(30):
            rnd=random.randint(0, 300)
            xCrsr.goRight(rnd, False)
            xText.insertControlCharacter(xCrsr, PARAGRAPH_BREAK, False)

    def insertBookmark(self, crsr, name, content):
        xBookmark = self.m_xDoc.createInstance("com.sun.star.text.Bookmark")
        crsr.String = content
        xBookmark.Name = name
        self.m_xDoc.Text.insertTextContent(crsr, xBookmark, True)

if __name__ == "__main__":
    unittest.main()
