#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This file incorporates work covered by the following license notice:

#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0.

import unittest
import random
import os.path
from hashlib import sha1
from tempfile import TemporaryDirectory
from org.libreoffice.unotest import UnoInProcess, mkPropertyValues, systemPathToFileUrl
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK


class CheckBookmarks(unittest.TestCase):
    expectedHashes = {
        'nSetupHash': 0x8f88ee1a13a55d6024f58f470723b5174dfa21bb,
        'nInsertRandomHash': 0x5f27e87e16d2cb3ff0bcb24237aa30da3b84cf24,
        'nDeleteRandomHash': 0x1bdaa7773cbfc73a4dc0bb3e0f801b98f648e8e7,
        'nLinebreakHash': 0x3f30a35f195efcfe0373a3e439de05087a2ad37c,
        'nOdfReloadHash': 0x3f30a35f195efcfe0373a3e439de05087a2ad37c,
        'nMsWordReloadHash': 0x3f30a35f195efcfe0373a3e439de05087a2ad37c,
        }

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xDoc = cls._uno.openEmptyWriterDoc()
        smgr = cls._uno.xContext.ServiceManager
        cls._desktop = smgr.createInstanceWithContext("com.sun.star.frame.Desktop", cls._uno.xContext)

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_bookmarks(self):
        self.xDoc = self.__class__._xDoc
        self.xText = self.xDoc.getText()
        ## setting and testing bookmarks
        self.setupBookmarks()
        self.assertEqual(self.expectedHashes['nSetupHash'],
                         self.getBookmarksHash(self.xDoc))
        ## modifying bookmarks and testing again
        self.insertRandomParts(200177)
        self.assertEqual(self.expectedHashes['nInsertRandomHash'],
                         self.getBookmarksHash(self.xDoc))
        ## modifying bookmarks and testing again
        self.deleteRandomParts(4711)
        self.assertEqual(self.expectedHashes['nDeleteRandomHash'],
                         self.getBookmarksHash(self.xDoc))
        ## adding line breaks and testing again
        self.insertLinebreaks(7)
        self.assertEqual(self.expectedHashes['nLinebreakHash'],
                         self.getBookmarksHash(self.xDoc))
        ## reloading document and testing again
        with TemporaryDirectory() as tempdir:
            xOdfReloadedDoc = self.reloadFrom(tempdir, "writer8", "odt")
            self.assertEqual(self.expectedHashes['nOdfReloadHash'],
                             self.getBookmarksHash(xOdfReloadedDoc))
            xOdfReloadedDoc.close(True)
        ## reloading document as MS Word 97 doc and testing again
            ## MsWord Hash is unstable over different systems
            # xMsWordReloadedDoc = self.reloadFrom(tempdir, "MS Word 97", "doc")
            # self.assertEqual(self.expectedHashes['nMsWordReloadHash'],
            #                  self.getBookmarksHash(xMsWordReloadedDoc))
            # xMsWordReloadedDoc.close(True)
        print('tests ok')

    def setupBookmarks(self):
        xCursor = self.xText.createTextCursor()
        for nPara in range(10):
            for nBookmark in range(100):
                s = "P{}word{}".format(nPara, nBookmark)
                xCursor.gotoEnd(False)
                xCursor.setString(s)
                xBookmark = self.xDoc.createInstance("com.sun.star.text.Bookmark")
                xBookmark.setName(s)
                self.xText.insertTextContent(xCursor, xBookmark, True)
                xCursor.End.setString(" ")
            self.xText.insertControlCharacter(xCursor.End, PARAGRAPH_BREAK, False)

    def getBookmarksHash(self, doc):
        hash = sha1()
        xBookmarks = doc.getBookmarks()
        for xBookmark in xBookmarks:
            s = '{}:{};'.format(xBookmark.Name,
                        xBookmark.getAnchor().getString().replace("\r\n", "\n"))
            hash.update(str.encode(s))
        return int(hash.hexdigest(), 16)

    def insertRandomParts(self, seed):
        random.seed(seed)
        xCursor = self.xText.createTextCursor()
        for i in range(600):
            xCursor.goRight(random.randrange(100), False)
            xCursor.setString(str(random.getrandbits(64)))

    def deleteRandomParts(self, seed):
        random.seed(seed)
        xCursor = self.xText.createTextCursor()
        for i in range(600):
            xCursor.goRight(random.randrange(100), False)
            xCursor.goRight(random.randrange(20), True)
            xCursor.setString("")

    def insertLinebreaks(self, seed):
        random.seed(seed)
        xCursor = self.xText.createTextCursor()
        for i in range(30):
            xCursor.goRight(random.randrange(300), False)
            self.xText.insertControlCharacter(xCursor, PARAGRAPH_BREAK, False)

    def reloadFrom(self, tempdir, sFilter, sExtension):
        sFileUrl = os.path.join(tempdir, "Bookmarktest.{}".format(sExtension))
        sFileUrl = systemPathToFileUrl(sFileUrl)
        store_props = mkPropertyValues(Override=True, FilterName=sFilter)
        self.xDoc.storeToURL(sFileUrl, store_props)
        desktop = self.__class__._desktop
        load_props = mkPropertyValues(Hidden=True, ReadOnly=False)
        return desktop.loadComponentFromURL(sFileUrl, "_default", 0, load_props)


if __name__ == '__main__':
    unittest.main()
