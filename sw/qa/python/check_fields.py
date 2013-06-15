import unittest
from org.libreoffice.unotest import UnoInProcess

class CheckFields(unittest.TestCase):
    _uno = None
    _xDoc = None

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xDoc = cls._uno.openWriterTemplateDoc("fdo39694.ott")
        cls._xEmptyDoc = cls._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_fdo39694_load(self):
        placeholders = ["<Kadr1>", "<Kadr2>", "<Kadr3>", "<Kadr4>", "<Pnname>", "<Pvname>", "<Pgeboren>"]
        xDoc = self.__class__._xDoc
        xEnumerationAccess = xDoc.getTextFields()
        xFieldEnum = xEnumerationAccess.createEnumeration()
        while xFieldEnum.hasMoreElements():
            xField = xFieldEnum.nextElement()
            if xField.supportsService("com.sun.star.text.TextField.JumpEdit"):
                xAnchor = xField.getAnchor()
                readContent = xAnchor.getString()
                self.assertTrue(readContent in placeholders,
                                "field %s is not contained: " % readContent)

    def test_fdo42073(self):
        xDoc = self.__class__._xEmptyDoc
        xBodyText = xDoc.getText()
        xCursor = xBodyText.createTextCursor()
        xTextField = xDoc.createInstance("com.sun.star.text.TextField.Input")
        xBodyText.insertTextContent(xCursor, xTextField, True)
        readContent = xTextField.getPropertyValue("Content")
        self.assertEqual("", readContent)
        content = "this is not surprising"
        xTextField.setPropertyValue("Content", content)
        readContent = xTextField.getPropertyValue("Content")
        self.assertEqual(content, readContent)

if __name__ == '__main__':
    unittest.main()
