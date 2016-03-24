import unittest
from org.libreoffice.unotest import UnoInProcess

class CheckChangeColor(unittest.TestCase):
    _uno = None
    _xDoc = None

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xEmptyDoc = cls._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_change_color(self):
        xDoc = CheckChangeColor._uno.openEmptyWriterDoc()
        xPageStyles = xDoc.StyleFamilies["PageStyles"]
        xPageStyle = xPageStyles["Standard"]
        self.assertEqual(xPageStyle.BackColor, -1)
        self.assertEqual(xPageStyle.IsLandscape, False)

        xPageStyle.setPropertyValue("BackColor", 0x000000FF)
        xPageStyle.setPropertyValue("IsLandscape", True)
        self.assertEqual(xPageStyle.BackColor, 0x000000FF)
        self.assertEqual(xPageStyle.IsLandscape, True)

if __name__ == '__main__':
    unittest.main()
