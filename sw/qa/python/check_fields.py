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
                                "field %s not contained" % readContent)

if __name__ == '__main__':
    unittest.main()
