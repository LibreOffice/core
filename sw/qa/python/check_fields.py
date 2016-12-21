import unittest
from org.libreoffice.unotest import UnoInProcess


class CheckFields(unittest.TestCase):

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
                read_content = xAnchor.getString()
                self.assertTrue(read_content in placeholders,
                                "field %s is not contained: " % read_content)

    def test_fdo42073(self):
        xDoc = self.__class__._xEmptyDoc
        xBodyText = xDoc.getText()
        xCursor = xBodyText.createTextCursor()
        xTextField = xDoc.createInstance("com.sun.star.text.TextField.Input")
        xBodyText.insertTextContent(xCursor, xTextField, True)
        read_content = xTextField.getPropertyValue("Content")
        self.assertEqual("", read_content)
        content = "this is not surprising"
        xTextField.setPropertyValue("Content", content)
        read_content = xTextField.getPropertyValue("Content")
        self.assertEqual(content, read_content)

if __name__ == '__main__':
    unittest.main()
