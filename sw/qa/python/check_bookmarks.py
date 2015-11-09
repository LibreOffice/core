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
from com.sun.star.text  import XSimpleText
from com.sun.star.text import XText
from com.sun.star.text import XTextContent
from com.sun.star.text import XTextCursor
from com.sun.star.text import XTextDocument 
from com.sun.star.text import XTextRange 
#from com.sun.star.uno import UnoRuntime 
#from  org.openoffice.test import OfficeConnection

class BookmarkHashes(unittest.TestCase):

    m_nSetupHash=sys.maxsize
    m_nInsertRandomHash=sys.maxsize
    m_nDeleteRandomHash=sys.maxsize
    m_nLinebreakHash=sys.maxsize
    m_nOdfReloadHash=sys.maxsize
    m_nMsWordReloadHash=sys.maxsize

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()


    def setUpDocuments():
        m_xMsf = UnoRuntime.queryInterface( type(XMultiServiceFactory).__name__, connection.getComponentContext().getServiceManager())
        m_xDoc = util.WriterTools.createTextDoc(m_xMsf)
        setupBookmarks()
        actualHashes.m_nSetupHash = BookmarkHashes.getBookmarksHash(m_xDoc)
        insertRandomParts(200177)
        actualHashes.m_nInsertRandomHash = BookmarkHashes.getBookmarksHash(m_xDoc)
        deleteRandomParts(4711)
        actualHashes.m_nDeleteRandomHash = BookmarkHashes.getBookmarksHash(m_xDoc)
        insertLinebreaks(7)
        actualHashes.m_nLinebreakHash = BookmarkHashes.getBookmarksHash(m_xDoc)
        m_xOdfReloadedDoc = reloadFrom("writer8", "odf")
        actualHashes.m_nOdfReloadHash = BookmarkHashes.getBookmarksHash(m_xOdfReloadedDoc)

  
    def assertExpectation(aExpectation):
        self.assertEqual(aExpectation.m_nSetupHash, m_nSetupHash)
        self.assertEqual(aExpectation.m_nInsertRandomHash, m_nInsertRandomHash)
        self.assertEqual(aExpectation.m_nDeleteRandomHash, m_nDeleteRandomHash)
        self.assertEqual(aExpectation.m_nLinebreakHash, m_nLinebreakHash)
        self.assertEqual(aExpectation.m_nOdfReloadHash, m_nOdfReloadHash)
        self.assertEqual(aExpectation.m_nMsWordReloadHash, m_nMsWordReloadHash)


    def getBookmarksHash(xDoc):
        buffer = StringBuffer("")
        xBookmarksSupplier = UnoRuntime.queryInterface(type(XBookmarksSupplier).__name__, xDoc)
        
        xBookmarks = xBookmarksSupplier.getBookmarks()
 
        for sBookmarkname in xBooksmars.getElementNames():
            xBookmark = xBookmarks.getByName(sBookmarkname)
            xBookmarkAsContent = UnoRuntime.queryInterface(type(XTextContent).__name__, xBookmark)
            buffer.append(sBookmarkname)
            buffer.append(':')
            buffer.append(xBookmark.getAnchor().getString())
            buffer.append(';')

        s= hashlib.sha1()
        s.update(buffer) 
        return s.digest()

class CheckBookmarks():
    m_xMsf = None
    m_xDoc = None
    m_xOdfReloadedDoc = None
    actualHashes = BookmarkHashes()

    def get20111110Expectations(self):
        result = BookmarkHashes()
       
        result.m_nSetupHash =int("-4b0706744e8452fe1ae9d5e1c28cf70fb6194795",16)
        result.m_nInsertRandomHash = int("25aa0fad3f4881832dcdfe658ec2efa8a1a02bc5",16)
        result.m_nDeleteRandomHash = int("-3ec87e810b46d734677c351ad893bbbf9ea10f55",16)
        result.m_nLinebreakHash = int("3ae08c284ea0d6e738cb43c0a8105e718a633550",16)
        result.m_nOdfReloadHash = int("3ae08c284ea0d6e738cb43c0a8105e718a633550",16)
        return result

    def checkBookmarks():
         actualHashes.assertExpectation(get20111110Expectations())

    def setupBookmarks():
        xText = m_xDoc.getText()
        UnoRuntime.queryInterface(type(XSimpleText).__name__, xText)

        for nPara in range(10):
            for nBookmark in range(100):
                insertBookmark(
                    xText.createTextCursor(),
                    "P" + nPara + "word" + nBookmark,
                    "P" + nPara + "word" + nBookmark)
                xWordCrsr = xText.createTextCursor()
                xWordCrsr.setString(" ")

            xParaCrsr = xText.createTextCursor()
            xParaCrsrAsRange = UnoRuntime.queryInterface(type(XTextRange).__name__, xParaCrsr)
            xText.insertControlCharacter(xParaCrsrAsRange, PARAGRAPH_BREAK, False)

    def insertRandomParts(seed):
        random.seed(seed)
        xCrsr = m_xDoc.getText().createTextCursor()
        for i in range(600):
            rnd=random.randint(0, 100)
            xCrsr.goRight(rnd , False)
            xCrsr.setString(long(random.choice(range(0,10000000))))
    
    def deleteRandomParts(seed):
        random.seed(seed)
        xCrsr = m_xDoc.getText().createTextCursor()
        for i in range(600):
            rnd=random.randint(0, 100)
            xCrsr.goRight(rnd, False)
            rnd=random.randint(0, 20)
            xCrsr.goRight(rnd, True)
            xCrsr.setString("")
     
    def insertLinebreaks(seed):
        xText = m_xDoc.getText()
        random.seed(seed)
        xCrsr = m_xDoc.getText().createTextCursor()
        for i in range(30):
            rnd=random.randint(0, 300)
            xCrsr.goRight(rnd, False)
            xCrsrAsRange = UnoRuntime.queryInterface(type(XTextRange).__name__, xCrsr)
            xText.insertControlCharacter(xCrsr, PARAGRAPH_BREAK, False)
    
    def insertBookmark( crsr, name,  content):
        xDocFactory = UnoRuntime.queryInterface(type(XMultiServiceFactory).__name__, m_xDoc)
        self.xBookmark = xDocFactory.createInstance("com.sun.star.text.Bookmark")
        xBookmarkAsTextContent = UnoRuntime.queryInterface(type(XTextContent).__name__, xBookmark)
        crsr.setString(content)
        xBookmarkAsNamed = UnoRuntime.queryInterface( type(XNamed).__name__, xBookmark)        
        xBookmarkAsNamed.setName(name)
        m_xDoc.getText().insertTextContent(crsr, xBookmarkAsTextContent, True)

 
if __name__ == "__main__":
    unittest.main()

