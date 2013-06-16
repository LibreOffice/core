import unittest
import unohelper
import os
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK
from com.sun.star.util import XRefreshListener

class RefreshListener(XRefreshListener, unohelper.Base):
 
    def __init__(self):
        self.m_bDisposed = False
        self.m_bRefreshed = False

    def disposing(self, event):
        self.m_bDisposed = True

    def refreshed(self, event):
        self.m_bRefreshed = True
    
    def assertRefreshed(self):
        assert(self.m_bRefreshed)
        self.m_bRefreshed = False
    
class CheckIndex(unittest.TestCase):
    _uno = None
    _xDoc = None

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xDoc = cls._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown() 
   
    def test_check_index(self):
        xDoc = self.__class__._xDoc
        xIndex = xDoc.createInstance("com.sun.star.text.ContentIndex")
        xBodyText = xDoc.getText()
        xCursor = xBodyText.createTextCursor()
        xIndex.setPropertyValue("CreateFromOutline", True)
        xBodyText.insertTextContent(xCursor, xIndex, True)

    # register listener
        listener = RefreshListener()
        xIndex.addRefreshListener(listener)
        self.assertFalse(listener.m_bRefreshed)
        xIndex.refresh()
        listener.assertRefreshed()

    # insert some heading
        xCursor.gotoEnd(False)
        xBodyText.insertControlCharacter(xCursor, PARAGRAPH_BREAK, False)
        xCursor.gotoEnd(False)
        test_string = "a heading"
        xCursor.setString(test_string)
        xCursor.gotoStartOfParagraph(True)
        xCursor.setPropertyValue("ParaStyleName","Heading 1")

    # hope text is in last paragraph...
        xIndex.refresh()
        listener.assertRefreshed()
        xCursor.gotoRange(xIndex.getAnchor().getEnd(), False)
        xCursor.gotoStartOfParagraph(True)
        text = xCursor.getString()
    # check if we got text with 'test_string'
        assert( text.find(test_string) >= 0 )

    # insert some more heading
        xCursor.gotoEnd(False)
        xBodyText.insertControlCharacter(xCursor, PARAGRAPH_BREAK, False)
        xCursor.gotoEnd(False)
        test_string = "yet another heading"
        xCursor.setString(test_string)
        xCursor.gotoStartOfParagraph(True)
        xCursor.setPropertyValue("ParaStyleName","Heading 1")

    # try agian with update
        xIndex.update()
        listener.assertRefreshed()
        xCursor.gotoRange(xIndex.getAnchor().getEnd(), False)
        xCursor.gotoStartOfParagraph(True)
        text = xCursor.getString()
    # check if we got text with 'test_string'
        assert( text.find(test_string) >= 0 )
     
    # dispose must call the listener
        assert(not listener.m_bDisposed)
        xIndex.dispose()
        assert(listener.m_bDisposed)
    
    # close the document
        xDoc.dispose()
if __name__ == "__main__":
    unittest.main()
